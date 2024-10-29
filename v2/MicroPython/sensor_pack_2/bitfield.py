# micropython
# MIT license
# Copyright (c) 2024 Roman Shevchik   goctaprog@gmail.com
"""Представление битового поля"""
from collections import namedtuple
from sensor_pack_2.base_sensor import check_value, get_error_str

# информация о битовом поле в виде именованного кортежа
# name: str  - имя
# position: range - место в номерах битах. position.start = первый бит, position.stop-1 - последний бит
# valid_values: [range, tuple] - диапазон допустимых значений, если проверка не требуется, следует передать None
# description: str - читаемое описание значения, хранимого в битовом поле, если описания не требуется, следует передать None
bit_field_info = namedtuple("bit_field_info", "name position valid_values description")


def _bitmask(bit_rng: range) -> int:
    """возвращает битовую маску по занимаемым битам"""
    # if bit_rng.step < 0 or bit_rng.start <= bit_rng.stop:
    #    raise ValueError(f"_bitmask: {bit_rng.start}; {bit_rng.stop}; {bit_rng.step}")
    return sum(map(lambda x: 2 ** x, bit_rng))


class BitFields:
    """Хранилище информации о битовых полях с доступом по индексу.
    _source - кортеж именованных кортежей, описывающих битовые поля;"""
    @staticmethod
    def _check(fields_info: tuple[bit_field_info, ...]):
        """Проверки на правильность информации!"""
        for field_info in fields_info:
            if 0 == len(field_info.name):
                raise ValueError(f"Нулевая длина строки имени битового поля!; position: {field_info.position}")
            if 0 == len(field_info.position):
                raise ValueError(f"Нулевая длина ('в битах') битового поля!; name: {field_info.name}")

    def __init__(self, fields_info: tuple[bit_field_info, ...]):
        BitFields._check(fields_info)
        self._fields_info = fields_info
        self._idx = 0
        # имя битового поля, которое будет параметром у методов get_value/set_value
        self._active_field_name = fields_info[0].name
        # значение, из которого будут извлекаться битовые поля
        self._source_val = 0

    def _by_name(self, name: str) -> [bit_field_info, None]:
        """возвращает информацию о битовом поле по его имени (поле name именованного кортежа) или None"""
        items = self._fields_info
        for item in items:
            if name == item.name:
                return item

    def _get_field(self, key: [str, int, None]) -> [bit_field_info, None]:
        """для внутреннего использования"""
        fi = self._fields_info
        _itm = None
        if key is None:
            _itm = self._by_name(self.field_name)
        if isinstance(key, int):
            _itm = fi[key]
        if isinstance(key, str):
            _itm = self._by_name(key)
        return _itm

    def get_field_value(self, field_name: str = None, validate: bool = False) -> [int, bool]:
        """возвращает значение битового поля, по его имени(self.field_name), из self.source."""
        item = self._get_field(field_name)
        if item is None:
            raise ValueError(f"get_field_value. Поле с именем {field_name} не существует!")
        pos = item.position
        bitmask = _bitmask(pos)
        val = (self.source & bitmask) >> pos.start  # выделение маской битового диапазона и его сдвиг вправо
        if item.valid_values and validate:
            raise NotImplemented("Если вы решили проверить значение поля при его возвращении, то делайте это самостоятельно!!!")
        if 1 == len(pos):
            return 0 != val     # bool
        return val              # int

    def set_field_value(self, value: int, source: [int, None] = None, field: [str, int, None] = None,
                        validate: bool = True) -> int:
        """Записывает value в битовый диапазон, определяемый параметром field, в source.
        Возвращает значение с измененным битовым полем.
        Если field is None, то имя поля берется из свойства self._active_field_name.
        Если source is None, то значение поля, подлежащее изменению, изменяется в свойстве self._source_val"""
        item = self._get_field(key=field)     #   *
        rng = item.valid_values
        if rng and validate:
            check_value(value, rng, get_error_str(self.field_name, value, rng))
        pos = item.position
        bitmask = _bitmask(pos)
        src = self._get_source(source) & ~bitmask  # чистка битового диапазона
        src |= (value << pos.start) & bitmask  # установка битов в заданном диапазоне
        if source is None:
            self._source_val = src
        return src

    def __getitem__(self, key: [int, str]) -> [int, bool]:
        """возвращает значение битового поля из значения в self.source по его имени/индексу"""
        _bfi = self._get_field(key)
        return self.get_field_value(_bfi.name)

    def __setitem__(self, field_name: str, value: [int, bool]):
        """Волшебный метод, вызывает set_field_value.
        До его вызова нужно установить свойства BitField source"""
        self.set_field_value(value=value, source=None, field=field_name, validate=True)     #   *

    def _get_source(self, source: [int, None]) -> int:
        return source if source else self._source_val

    @property
    def source(self) -> int:
        """значение, из которого будут извлекаться/в котором будут изменятся битовые поля"""
        return self._source_val

    @source.setter
    def source(self, value):
        """значение, из которого будут извлекаться/изменятся битовые поля"""
        self._source_val = value

    @property
    def field_name(self) -> str:
        """имя битового поля, значение которого извлекается/изменяется методами get_value/set_value, если их
        параметр field is None"""
        return self._active_field_name

    @field_name.setter
    def field_name(self, value):
        """имя битового поля, значение которого извлекается/изменяется методами get_value/set_value, если их
        параметр field is None"""
        self._active_field_name = value

    def __len__(self) -> int:
        return len(self._fields_info)

    # протокол итератора
    def __iter__(self):
        return self

    def __next__(self) -> bit_field_info:
        ss = self._fields_info
        try:
            self._idx += 1
            return ss[self._idx - 1]
        except IndexError:
            self._idx = 0   # для возможности выполнения повторной итерации!
            raise StopIteration
