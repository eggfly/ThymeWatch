# MicroPython
# mail: goctaprog@gmail.com
# MIT license

# from sensor_pack_2.bus_service import mpy_bl
from collections import namedtuple
from sensor_pack_2.base_sensor import check_value

# разностный вход (bool, differential_input)
# разрядность в битах (int, resolution)
# опорное напряжение в вольтах (float, rev_voltage)
# количество аналоговых входов (channels)
adc_base_props = namedtuple("adc_props", "ref_voltage resolution channels differential_channels")
# кортеж информации о канале АЦП: number(номер канала):int, is_differential(дифференциальный_режим):bool
adc_channel_info = namedtuple("adc_channel_info", "number is_differential")
# кортеж информации о количестве(!) каналов АЦП
# channels - количество обычных(single ended) каналов
# differential_channels - количество дифференциальных(differential) каналов
adc_channels = namedtuple("adc_channels", "channels differential_channels")
# основные свойства АЦП: опорное напряжение, Вольт; текущее кол-во значащих бит в отсчете,
# предельное кол-во значащих бит в отсчете, текущий номер канала, количество обычных накалов(Vxx..GND),
# предельное кол-во обычных каналов, предельное кол-во дифференциальных каналов,
# текущая частота отсчетов(current sample rate), Гц
adc_general_props = namedtuple("adc_general_props",
                               "ref_voltage resolution max_resolution current_channel channels diff_channels")
# основные 'сырые' настройки, характерные для всех(!) АЦП
adc_general_raw_props = namedtuple("adc_general_raw_props", "sample_rate gain_amplifier single_shot_mode")

# параметры для инициализации АЦП
# reference_voltage - опорное напряжение, Вольт
# max_resolution - предельное кол-во значащих бит в отсчете. Разрешение часто, параметр динамический(!).
# Зависит от частоты преобразования _data_rate (Гц) аналогового сигнала в цифровой.
# channels - количество обычных(single ended) каналов
# differential_channels - количество дифференциальных(differential) каналов
# differential_mode - Если истина, то это дифференциальный АЦП. для метода get_lsb.
adc_init_props = namedtuple("adc_init_props",
                            "reference_voltage max_resolution channels differential_channels differential_mode")
# для метода get_raw_value_ex
# value - значение АЦП, сырое(!)
# если low_limit в Истина, то "стрелка" АЦП на нижнем крае шкалы (underflow)
# если hi_limit в Истина, то "стрелка" АЦП верхнем крае шкалы (overflow)
raw_value_ex = namedtuple("raw_value_ex", "value low_limit hi_limit")

# Типовое содержимое регистра конфигурации (все значения сырые/raw):
# gain; коэффициент усиления для PGA-programmable gain amplifier (усилитель с программируемым усилением)
# channel;      измеряющий канал
# data_rate;    частота отсчетов
# resolution;   количество бит в отсчете    (для некоторых АЦП)
# meas_mode;    режим измерения (single shot or automatic)
# raw_config_reg = namedtuple("raw_config_reg", "gain channel data_rate resolution meas_mode")


# вычисляет 'сырые' предельные значения, считываемые из регистра АЦП
def _get_reg_raw_limits(adc_resolution: int, differential: bool) -> raw_value_ex:
    if differential:
        # для дифференциальных АЦП
        _base = 2 ** (adc_resolution - 1)
        return raw_value_ex(value=0, low_limit=_base, hi_limit=_base - 1)
    # для обычных АЦП
    return raw_value_ex(value=0, low_limit=0, hi_limit=2 ** adc_resolution - 1)


class ADC:
    def __init__(self, init_props: adc_init_props, model: str = None):
        """reference_voltage - опорное напряжение в Вольтах;
        max_resolution - предельное разрешение АЦП в битах;
        channels - кол-во(!) входных аналоговых каналов;
        differential_channels - кол-во(!) входных аналоговых дифференциальных каналов;
        model - модель АЦП в виде строки"""
        self.init_props = init_props
        adc_ip = self.init_props
        if adc_ip.reference_voltage <= 0 or adc_ip.channels < 0 or adc_ip.differential_channels < 0:
            raise ValueError(f"Неверный параметр! Опорное напряжение, В: {adc_ip.reference_voltage}; Кол-во каналов: {adc_ip.channels}/{adc_ip.differential_channels}")
        # текущее количество выполняемых преобразований аналогового сигнала в цифровой! RAW, сырое значение!
        # для записи в регистр
        self._curr_raw_data_rate = None
        # текущее разрешение АЦП в битах
        self._curr_resolution = None
        # текущий номер канала. Диапазон 0..self._channels/self._diff_channels. Проверка на правильность в методе
        # check_channel_number
        self._curr_channel = None
        # если Истина, то self._curr_channel это дифференциальный(!) канал, иначе канал не дифференциальный(!)
        self._is_diff_channel = None
        # текущий коэффициент усиления (raw). Для записи в регистр АЦП
        self._curr_raw_gain = None  # RAW!
        # действительный текущий коэффициент усиления.
        # присвойте его в классе - наследнике путем пересчета из self._curr_gain. Смотри метод Ads1115.get_correct_gain
        self._real_gain = None
        # режим преобразования. если истина, то АЦП выполняет преобразование по запросу,
        # иначе АЦП выполняет преобразования автоматически с определенной частотой (_curr_data_rate)
        self._single_shot_mode = None
        # режим пониженного энергопотребления
        self._low_pwr_mode = None
        # строковое имя модели АЦП
        self._model_name = model

    @property
    def model(self) -> str:
        """Строковое имя модели АЦП"""
        return self._model_name

    def get_general_props(self) -> adc_general_props:
        """Возвращает основные свойства АЦП"""
        ipr = self.init_props
        return adc_general_props(ipr.reference_voltage, self.current_resolution, ipr.max_resolution, self._curr_channel,
                                 ipr.channels, ipr.differential_channels)

    def get_general_raw_props(self) -> adc_general_raw_props:
        """Возвращает основные 'сырые' свойства АЦП, которые считываются из регистра"""
        return adc_general_raw_props(sample_rate=self._curr_raw_data_rate, gain_amplifier=self._curr_raw_gain,
                                     single_shot_mode=self._single_shot_mode)

    def get_specific_props(self):
        """Возвращает характерные для АЦП свойства, желательно в виде именованного кортежа.
        Для переопределения в классе-наследнике"""
        raise NotImplemented

    def check_channel_number(self, value: int, diff: bool) -> int:
        """Проверяет номер входного аналогового канала(value) АЦП на правильность.
        Если diff в Истина, то канал дифференциальный(!).
        value должно быть в диапазоне 0..self._channels/self._diff_channels"""
        ipr = self.init_props
        _max = ipr.differential_channels if diff else ipr.channels
        check_value(value, range(_max),
                    f"Неверный номер канала АЦП: {value}; дифф: {diff}. Допустимый диапазон: 0..{_max - 1}")
        return value

    def check_gain_raw(self, gain_raw: int) -> int:
        """Проверяет сырое усиление на правильность. В случае ошибки выброси исключение!
        Возвращает значение gain_raw в случае успеха! Для переопределения в классе-наследнике."""
        raise NotImplemented

    def check_data_rate_raw(self, data_rate_raw: int) -> int:
        """Проверяет сырое data_rate на правильность. В случае ошибки выброси исключение!
        Возвращает data_rate_raw в случае успеха! Для переопределения в классе-наследнике."""
        raise NotImplemented

    def get_lsb(self) -> float:
        """Возвращает цену младшего разряда в Вольтах в зависимости от текущих настроек АЦП.
        gain - коэффициент усиления/ослабления входного делителя АЦП, должен быть больше нуля!"""
        ipr = self.init_props
        _k = 2 if ipr.differential_mode else 1
        return _k * ipr.reference_voltage / (self.gain * 2 ** self.current_resolution)

    def get_conversion_cycle_time(self) -> int:
        """возвращает время преобразования в [мкc/мс] аналогового значения в цифровое в зависимости от
        текущих настроек АЦП. Переопредели для каждого АЦП!"""
        raise NotImplemented

    @property
    def general_properties(self) -> adc_general_props:
        return self.get_general_props()

    @property
    def value(self) -> float:
        """Возвращает значение текущего канала в Вольтах"""
        return self.get_value(raw=False)

    def get_raw_value(self) -> int:
        """Возвращает 'сырое' значение отсчета АЦП.
        Переопределяется в классах - наследниках!"""
        raise NotImplemented

    def get_raw_value_ex(self, delta: int = 5) -> raw_value_ex:
        """Возвращает 'сырое' значение отсчета АЦП и флаги переполнения.
        Переопределяется в классах - наследниках!
        delta - 'зазор'"""
        raw = self.get_raw_value()
        limits = _get_reg_raw_limits(self.current_resolution, self.init_props.differential_mode)
        return raw_value_ex(value=raw, low_limit=raw in range(limits.low_limit, 1 + delta + limits.low_limit),
                            hi_limit=raw in range(limits.hi_limit - delta, 1 + limits.hi_limit))

    def raw_value_to_real(self, raw_val: int) -> float:
        """Преобразует 'сырое' значение из регистра АЦП в значение в Вольтах"""
        return raw_val * self.get_lsb()

    def gain_raw_to_real(self, raw_gain: int) -> float:
        """Преобразует 'сырое' значение усиления в 'настоящее'.
        Переопределить в классе - наследнике!"""
        raise NotImplemented

    def get_value(self, raw: bool = True) -> float:
        """Возвращает значение текущего канала в Вольтах, если raw в Ложь, в коде, если raw в Истина"""
        val = self.get_raw_value()
        if raw:
            return val
        return self.raw_value_to_real(val)

    def get_resolution(self, raw_data_rate: int) -> int:
        """Возвращает кол-во бит в отсчете АЦП в зависимости от частоты взятия отсчетов (сырое значение!).
        Переопределить в классе - наследнике!"""
        raise NotImplemented

    def get_current_channel(self) -> adc_channel_info:
        """Возвращает информацию о текущем активном канале АЦП"""
        return adc_channel_info(number=self._curr_channel, is_differential=self._is_diff_channel)

    @property
    def channel(self) -> adc_channel_info:
        """Возвращает информацию о текущем канале"""
        return self.get_current_channel()

    def __len__(self) -> int:
        """Возвращает количество аналоговых каналов АЦП в зависимости от типа текущего канала.
        Если текущий канал дифференциальный, то возвращается кол-во дифференциальных каналов, иначе
        возвращается кол-во обычных(single ended) каналов"""
        ipr = self.init_props
        return ipr.differential_channels if self._is_diff_channel else ipr.channels

    def start_measurement(self, single_shot: bool, data_rate_raw: int, gain_raw: int, channel: int,
                          differential_channel: bool):
        """Запуск однократного(single_shot в Истина) или многократного(single_shot в Ложь) измерения.
        data_rate_raw - частота получения выборок АЦП, отсчетов в сек., RAW-параметр, смотри в datasheet битовое поле!
        gain_raw - коэффициент усиления входного аналогового напряжения, RAW-параметр, смотри в datasheet битовое поле!
        channel - номер аналогового входа. От 0 до self._channels/self._diff_channels - 1
        differential_channel - если Истина, то канал с номером channel дифференциальный(!)
        Внимание! Последней строкой этого метода всегда вызывайте метод raw_config_to_adc_properties для
        записи значений в соответствующие поля класса!
        Скорее всего этот метод не потребуется переопределять, в крайнем случае и это можно сделать."""
        self.check_gain_raw(gain_raw=gain_raw)   # проверка на правильность
        self.check_data_rate_raw(data_rate_raw=data_rate_raw)   # проверка на правильность
        self.check_channel_number(channel, differential_channel)  # проверка на правильность
        #
        self._single_shot_mode = single_shot
        self._curr_raw_data_rate = data_rate_raw
        self._curr_raw_gain = gain_raw
        self._curr_channel = channel
        self._curr_resolution = self.get_resolution(data_rate_raw)
        self._is_diff_channel = differential_channel
        # переопределяемые для каждого АЦП, методы
        _raw_cfg = self.adc_properties_to_raw_config()
        self.set_raw_config(_raw_cfg)
        # читаю config АЦП и обновляю поля класса
        _raw_cfg = self.get_raw_config()    # читаю настройки АЦП
        self.raw_config_to_adc_properties(_raw_cfg)     # обновляю поля экземпляра класса
        # пересчет в реальное усиление
        self._real_gain = self.gain_raw_to_real(self._curr_raw_gain)

    def raw_config_to_adc_properties(self, raw_config: int):
        """Возвращает текущие настройки датчика из числа, возвращенного get_raw_config(!), в поля(!) класса.
        raw_config -> adc_properties.
        Переопределить в классе - наследнике!"""
        raise NotImplemented

    def adc_properties_to_raw_config(self) -> int:
        """Преобразует свойства АЦП из полей класса в 'сырую' конфигурацию АЦП.
        adc_properties -> raw_config.
        Переопределить в классе - наследнике!"""
        raise NotImplemented

    def get_raw_config(self) -> int:
        """Возвращает(считывает) текущие настройки датчика из регистров(конфигурации) в виде числа.
        Переопределить в классе - наследнике!"""
        raise NotImplemented

    def set_raw_config(self, value: int):
        """Записывает настройки(value) во внутреннюю память/регистр датчика.
        Переопределить в классе - наследнике!"""
        raise NotImplemented

    def raw_sample_rate_to_real(self, raw_sample_rate: int) -> float:
        """Преобразует сырое значение частоты преобразования в [Гц].
        Переопределить в классе - наследнике!"""
        raise NotImplemented

    @property
    def sample_rate(self) -> float:
        """Возвращает текущее число отсчетов в секунду"""
        return self.raw_sample_rate_to_real(self.current_sample_rate)

    @property
    def current_sample_rate(self) -> int:
        """Возвращает текущее сырое(!) количество отсчетов АЦП"""
        return self._curr_raw_data_rate

    @property
    def current_raw_gain(self) -> int:
        """Возвращает текущий сырой(!) коэффициент усиления АЦП"""
        return self._curr_raw_gain

    @property
    def gain(self) -> float:
        """Возвращает текущий реальный коэффициент усиления АЦП"""
        return self._real_gain

    @property
    def current_resolution(self) -> int:
        """Возвращает текущее(!) кол-во бит в отсчете АЦП"""
        return self._curr_resolution

    @property
    def single_shot_mode(self) -> bool:
        """Возвращает Истина, Если АЦП настроен на однократный(single shot conversion) режим работы,
        иначе на непрерывный (continuous conversion mode)"""
        return self._single_shot_mode
