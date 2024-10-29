"""Модуль для управления датчиком внешней освещенности OPT3001 от TI."""

# micropython
# MIT license

from sensor_pack_2 import bus_service
from sensor_pack_2.base_sensor import DeviceEx, IBaseSensorEx, Iterator, check_value
from collections import namedtuple
from sensor_pack_2.bitfield import bit_field_info
from sensor_pack_2.bitfield import BitFields

#   описывает состояние выходного регистра ЦАП
#	out_reg  - значение 0..4095 (12 бит), выходной регистр ЦАП
#	power_mode - режим работы (0 - нормальный режим, 1..3 - режим малого потребления мощности)
# mcp4725_data = namedtuple("mcp4725_data", "out_reg power_mode")

#   тип данных, для чтения из ЦАП.
#	data  - это значение типа mcp4725_data
#	eeprom_data  - это значение типа mcp4725_data, хранимое в EEPROM
#	write_status - это значение типа bool, состояние записи EEPROM памяти (EEPROM Write Status)
# mcp4725_status = namedtuple("mcp4725_status", 'data eeprom_data write_status')
opt3001_id = namedtuple("opt3001_id", "manufacturer_id device_id")
opt3001_config = namedtuple("config_opt3001", "RN CT M OVF CRF FH FL L POL ME FC")
# результат измерения освещенности датчиком
# exponent и fractional два поля сырых данных
# lux - освещенность в Люксах (https://en.wikipedia.org/wiki/Lux)
opt3001_meas_data = namedtuple("opt3001_meas_data", "lux full_scale_range")
# 'сырые' измеренные данные
opt3001_meas_raw = namedtuple("opt3001_meas_raw", "exponent fractional")
# FSR - Full Scale Range в Люксах! Предельная освещенность, на которую настроен датчик
# LSB - цена наименее значащего бита в Люксах!
_opt3001_lsb_fsr = namedtuple("_opt3001_lsb_fsr", "LSB FSR")
# готовы ли данные для считывания?
OPT3001_data_status = namedtuple("OPT3001_data_status", "conversion_ready overflow")

class OPT3001(DeviceEx, IBaseSensorEx, Iterator):
    """Представление OPT3001, ALS от именитого производителя."""

    _config_reg_opt3001 = (bit_field_info(name='RN', position=range(12, 16), valid_values=range(13), description="Range number field."),  # Reset Bit
                          bit_field_info(name='CT', position=range(11, 12), valid_values=None, description="Conversion time field."),
                          bit_field_info(name='M', position=range(9, 11), valid_values=range(4), description="Mode of conversion operation field."),
                          bit_field_info(name='OVF', position=range(8, 9), valid_values=None, description="Overflow flag."),
                          bit_field_info(name='CRF', position=range(7, 8), valid_values=None, description="Conversion ready."),
                          bit_field_info(name='FH', position=range(6, 7), valid_values=None, description='Flag high'),
                          bit_field_info(name='FL', position=range(5, 6), valid_values=None, description='Flag low'),
                          bit_field_info(name='L', position=range(4, 5), valid_values=None, description='Latch field'),
                          bit_field_info(name='POL', position=range(3, 4), valid_values=None, description='Polarity'),
                          bit_field_info(name='ME', position=range(2, 3), valid_values=None, description='Mask exponent'),
                          bit_field_info(name='FC', position=range(2), valid_values=None, description='Fault count'),
                          )

    def __init__(self, adapter: bus_service.BusAdapter, address=0x44):
        check_value(address, range(0x44, 0x48), f"Неверное значение адреса I2C устройства: 0x{address:x}")
        DeviceEx.__init__(self, adapter, address, True)
        # для удобства работы с настройками
        # информация о полях регистра конфигурации устройства = OPT3001._config_reg_opt3001
        self._bit_fields = BitFields(fields_info=OPT3001._config_reg_opt3001)

    def _get_config_field(self, field_name: [str, None] = None) -> [int, bool]:
        """Возвращает значение поля по его имени, field_name, из сохраненной конфигурации.
        Если field_name is None, будут возвращены все поля конфигурации в виде int"""
        bf = self._bit_fields
        if field_name is None:
            return bf.source
        return bf[field_name]

    def _set_config_field(self, value: int, field_name: [str, None] = None):
        """Устанавливает значение поля, value, по его имени, field_name, в сохраненной конфигурации.
        Если field_name is None, будут установлены значения всех полей конфигурации."""
        bf = self._bit_fields
        if field_name is None:
            bf.source = value
            return
        bf[field_name] = value

    @staticmethod
    def _get_lsb_fsr(exp_raw: int) -> _opt3001_lsb_fsr:
        # check_value(exp_raw, range(12), f"Неверное значение exp_raw: {exp_raw}")
        _lsb, _fsr = None, None
        if exp_raw < 0 or exp_raw > 11:
            return _opt3001_lsb_fsr(_lsb, _fsr) # ошибка
        _lsb = 0.01 * 2 ** exp_raw
        _fsr = 40.95 * 2 ** exp_raw
        return _opt3001_lsb_fsr(_lsb, _fsr)


    def get_cfg_reg(self) -> int:
        """Возвращает 'сырую' конфигурацию из регистра. Get raw configuration from register"""
        return self.read_reg_16(0x01)

    def set_cfg_reg(self, value: int) -> int:
        """Установить сырую конфигурацию в регистре. Set raw configuration in register."""
        return self.write_reg_16(0x01, value)

    def get_config_hr(self) -> opt3001_config:
        """"Возвращает настройки датчика"""
        return opt3001_config(RN=self.lux_range_index, CT=self.long_conversion_time, M=self.mode, OVF=self.overflow,
                              CRF=self.conversion_ready, FH=self.flag_high, FL=self.flag_low, L=self.latch,
                              POL=self.polarity, ME=self.mask_exponent, FC=self.fault_count)

    def read_config_from_sensor(self, return_value: bool = False) -> [None, opt3001_config]:
        """Возврат текущей конфигурации датчика в виде кортежа.
        Вызовите этот метод, когда считаете, что нужно обновить конфигурацию в полях класса!!!"""
        raw = self.get_cfg_reg()
        self._set_config_field(value=raw, field_name=None)
        if return_value:
            return self.get_config_hr()

    def write_config_to_sensor(self) -> int:
        """Настраивает датчик в соответствии с настройками в полях класса.
        Возвращает значение настроек в сыром(!) виде"""
        _cfg = self._get_config_field(field_name=None)
        self.set_cfg_reg(_cfg)
        return _cfg

    # BaseSensorEx
    def get_id(self) -> opt3001_id:
        man_id, dev_id = self.read_reg_16(0x7E), self.read_reg_16(0x7F)
        return opt3001_id(manufacturer_id=man_id, device_id=dev_id)

    @property
    def lux_range_index(self) -> int:
        """Возвращает измеряемый диапазон освещенности в 'сыром/raw' виде:
        index       lux         lux_per_LSB
        0           40.95       0.01
        1           81.90       0.02
        2           163.80      0.04
        3           327.60      0.08
        4           655.20      0.16
        5           1310.40     0.32
        6           2620.80     0.64
        7           5241.60     1.28
        8           10483.20    2.56
        9           20966.40    5.12
        10          41932.80    10.24
        11          83865.60    20.48
        12          automatic full-scale range
        """
        return self._get_config_field('RN')

    @lux_range_index.setter
    def lux_range_index(self, value: int):
        """Устанавливает измеряемый диапазон освещенности"""
        self._set_config_field(value, 'RN')

    @property
    def long_conversion_time(self) -> bool:
        """Возвращает время преобразования датчиком кол-ва света в цифровой код. False - 100 мс. True - 800 мс."""
        return self._get_config_field('CT')

    @long_conversion_time.setter
    def long_conversion_time(self, value: bool):
        """Устанавливает время преобразования датчиком кол-ва света в цифровой код. False - 100 мс. True - 800 мс."""
        self._set_config_field(value, 'CT')

    @property
    def mode(self) -> int:
        """Возвращает режим работы датчика:
        * 0 - режим экономии энергии датчиком
        * 1 - одиночный режим работы
        * 2, 3 - автоматический режим работы. после завершения одного измерения, начинается другое!"""
        return self._get_config_field('M')

    @mode.setter
    def mode(self, value: int):
        """Устанавливает режим работы датчика"""
        self._set_config_field(value, 'M')

    @property
    def overflow(self) -> bool:
        """Флаг переполнения указывает на возникновение состояния переполнения в процессе преобразования данных,
        обычно из-за того, что свет, освещающий устройство, превышает запрограммированный полный диапазон шкалы устройства."""
        return self._get_config_field('OVF')

    @property
    def conversion_ready(self) -> bool:
        """Указывает, когда завершено преобразование освещенности в цифровой код."""
        return self._get_config_field('CRF')

    @property
    def flag_high(self) -> bool:
        """Возвращает флаг сравнения.
        Этот флаг указывает на то, что результат преобразования превышает указанный уровень интереса."""
        return self._get_config_field('FH')

    @property
    def flag_low(self) -> bool:
        """Возвращает флаг сравнения.
        Этот флаг указывает на то, что результат преобразования превышает указанный уровень интереса."""
        return self._get_config_field('FL')

    @property
    def latch(self) -> bool:
        """Поле защелки управляет функциональностью механизмов отчетности прерываний: вывод INT, высокое поле флага (FH)
        и низкое поле флага (FL). Этот бит выбирает стиль отчетности между сравнением в стиле защелкнутого окна и
        сравнением в стиле прозрачного гистерезиса."""
        return self._get_config_field('L')

    @latch.setter
    def latch(self, value: bool):
        """."""
        self._set_config_field(value, 'L')

    @property
    def polarity(self) -> bool:
        """Поле защелки управляет функциональностью механизмов отчетности прерываний: вывод INT, высокое поле флага (FH)
        и низкое поле флага (FL). Этот бит выбирает стиль отчетности между сравнением в стиле защелкнутого окна и
        сравнением в стиле прозрачного гистерезиса."""
        return self._get_config_field('POL')

    @polarity.setter
    def polarity(self, value: bool):
        """."""
        self._set_config_field(value, 'POL')

    @property
    def mask_exponent(self) -> bool:
        """Поле защелки управляет функциональностью механизмов отчетности прерываний: вывод INT, высокое поле флага (FH)
        и низкое поле флага (FL). Этот бит выбирает стиль отчетности между сравнением в стиле защелкнутого окна и
        сравнением в стиле прозрачного гистерезиса."""
        return self._get_config_field('ME')

    @mask_exponent.setter
    def mask_exponent(self, value: bool):
        """."""
        self._set_config_field(value, 'ME')

    @property
    def fault_count(self) -> int:
        """."""
        return self._get_config_field('FC')

    @fault_count.setter
    def fault_count(self, value: bool):
        """."""
        self._set_config_field(value, 'FC')

    # IBaseSensorEx

    def get_conversion_cycle_time(self) -> int:
        """Возвращает время в мс преобразования сигнала в цифровой код и готовности его для чтения по шине!
        Для текущих настроек датчика. При изменении настроек следует заново вызвать этот метод!"""
        if self.long_conversion_time:
            return 800
        return 100

    def start_measurement(self, continuously: bool = True, lx_range_index: int = 12, refresh: bool = False):
        """Настраивает параметры датчика и запускает процесс измерения.
        Если refresh is True, то после настройки датчика, обновляется содержимое полей, хранящих настройки датчика!
        lx_range_index - индекс диапазона освещенности, 12 - автоматический(!) выбор. Смотри lux_range_index,
        читай 7.4.1 Automatic Full-Scale Setting Mode"""
        self.mode = 3 if continuously else 1
        self.lux_range_index = lx_range_index
        #
        self.write_config_to_sensor()
        if refresh:
            self.read_config_from_sensor(return_value=False)

    def get_measurement_value(self, value_index: int = 0) -> [opt3001_meas_raw, opt3001_meas_data]:
        """Возвращает измеренное датчиком значение(значения) по его индексу/номеру."""
        raw = self.read_reg_16(0, signed=False)
        _exponent, _fractional = (raw & 0xF000) >> 12, raw & 0x0FFF
        if 0 == value_index:
            return opt3001_meas_raw(exponent=_exponent, fractional=_fractional)
        if 1 == value_index:
            # обработанные данные в Люксах
            _data = OPT3001._get_lsb_fsr(_exponent)
            _lux = _data.LSB * _fractional
            return opt3001_meas_data(lux=_lux, full_scale_range=_data.FSR)

    def get_data_status(self) -> OPT3001_data_status:
        self.read_config_from_sensor(return_value=False)
        return OPT3001_data_status(conversion_ready=self.conversion_ready, overflow=self.overflow)

    def is_single_shot_mode(self) -> bool:
        """Возвращает Истина, когда датчик находится в режиме однократных измерений,
        каждое из которых запускается методом start_measurement"""
        return 1 == self.mode

    def is_continuously_mode(self) -> bool:
        """Возвращает Истина, когда датчик находится в режиме многократных измерений,
        производимых автоматически. Процесс запускается методом start_measurement"""
        return 2 == self.mode or 3 == self.mode

#   Iterator
    def __iter__(self):
        return self

    def __next__(self) -> opt3001_meas_data:
        """Возвращает измеренные значения. кортеж, число."""
        ds = self.get_data_status() # обновляю содержимое полей экземпляра класса
        if self.is_continuously_mode():
            if ds.conversion_ready:
                return self.get_measurement_value(0)