# micropython
# MIT license
# Copyright (c) 2024 Roman Shevchik   goctaprog@gmail.com
"""представление аппаратного регистра устройства"""

# from sensor_pack_2 import bus_service
from sensor_pack_2.base_sensor import DeviceEx, get_error_str, check_value
from sensor_pack_2.bitfield import BitFields

# 24.04.2024 было-> address: int; стало-> address: [int, None]. Смотри def __init__(...


class BaseRegistry:
    """Представление аппаратного регистра. Базовый класс."""

    def _get_width(self) -> int:
        """Возвращает разрядность регистра по информация из параметра типа BitFields в байтах!"""
        mx = max(map(lambda val: val.position.stop, self._fields))
        # print(f"DBG: _get_width: {mx}")
        return 1 + int((mx - 1)/8)

    def __init__(self, device: [DeviceEx, None], address: [int, None], fields: BitFields, byte_len: [int, None] = None):
        """device - устройство, которому принадлежит регистр.
        address - адрес регистра в памяти устройства.
        fields - битовые поля регистра.
        byte_len - разрядность регистра в байтах!"""
        check_value(byte_len, range(1, 3), get_error_str('byte_len', byte_len, range(1, 3)))
        self._device = device
        self._address = address
        self._fields = fields
        # один или два байта!!! Если больше, придется изменить метод read!!!
        self._byte_len = byte_len if byte_len else self._get_width()
        # проверка битового диапазона поля
        # str_err = f"Неверный параметр битового поля!"
        _k = 8 * self._byte_len
        for field in fields:
            check_value(field.position.start, range(_k),
                        get_error_str('field.position.start', field.position.start, range(_k)))
            check_value(field.position.stop - 1, range(_k),
                        get_error_str('field.position.stop', field.position.stop, range(_k)))
            check_value(field.position.step, range(1, 2),
                        get_error_str('field.position.step', field.position.step, range(1, 2)))  # шаг только единица!
        #
        self._value = 0  # значение, считанное из регистра

    def _rw_enabled(self) -> bool:
        """Возвращает Истина, когда возможна запись в регистр по шине"""
        return self._device is not None and self._address is not None

    def __len__(self) -> int:
        return len(self._fields)

    def __getitem__(self, key: str) -> int:
        """Возвращает значение битового поля в виде числа или bool по его имени в виде строки!"""
        lnk = self._fields
        lnk.field_name = key
        # установлю значение, иначе правильной работы не будет!!!
        lnk.source = self.value
        # возврат
        return lnk.get_field_value(validate=False)

    def __setitem__(self, key: str, value: int) -> int:
        """Устанавливает значение битового поля в виде числа или bool по его имени в виде строки!"""
        lnk = self._fields
        lnk.field_name = key
        # установлю значение, иначе правильной работы не будет!!!
        lnk.source = self.value
        # установка
        _tmp = lnk.set_field_value(value=value)
        self._value = _tmp
        return _tmp

    @property
    def value(self) -> int:
        """Возвращает значение, считанное из регистра. Из этого значения будут извлекаться значения битовых полей."""
        return self._value

    @value.setter
    def value(self, new_val: int):
        """Иногда требуется явно присвоить значение полю экземпляра класса. Для удобства использования."""
        self._value = new_val

    @property
    def byte_len(self) -> int:
        """Возвращает разрядность регистра в байтах"""
        return self._byte_len


class RegistryRO(BaseRegistry):
    """Представление аппаратного регистра. Только для чтения"""

    def read(self) -> [int, None]:
        """Чтение значения из регистра устройства и запись его в поле класса"""
        if not self._rw_enabled():
            return
        bl = self._byte_len
        by = self._device.read_reg(self._address, bl)
        fmt = "B" if 1 == bl else "H"
        self._value = self._device.unpack(fmt, by)[0]
        return self._value

    def __int__(self) -> int:
        return self.read()


class RegistryRW(RegistryRO):
    """Представление аппаратного регистра. Чтение и запись."""

    def write(self, value: [int, None] = None):
        """Запись значения в регистр устройства.
        Если value в None, то метод запишет в регистр значение поля self.value"""
        if self._rw_enabled():
            val = value if value else self.value
            self._device.write_reg(self._address, val, self._byte_len)
