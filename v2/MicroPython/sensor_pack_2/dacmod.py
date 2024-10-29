from sensor_pack_2.base_sensor import check_value

class DAC:
    """Интерфейс ЦАП.
    Пока ничего нет!"""
    def __init__(self, resolution: int, unipolar: bool = True):
        """resolution - резрешение ЦАП в битах.
        Если unipolar is True, то выходное напряжение изменяется в диапазоне 0..Vопорное,
        иначе: -Vопорное/2 ... Vопорное/2"""
        check_value(resolution, range(8, 25), f"Неверное значение разрешения ЦАП: {resolution}!")
        self._resolution = resolution
        self._unipolar = unipolar

    def get_out_range(self) -> range:
        """возвращает 'сырой' диапазон значений выходного регистра"""
        if self.unipolar:
            return range(2 ** self.resolution)
        _mx = 2 ** (self.resolution - 1)
        return range(-1 * _mx, -1 + _mx)

    def get_raw(self, percent: float) -> int:
        """Преобразует значение из процентов (0..100) в сырое значение для регистра выходного значения ЦАП."""
        if percent < 0.0 or percent > 100.0:
            raise ValueError(f"Неверное значение в процентах: {percent}")
        return int(0.01 * (2 ** self.resolution) * percent)

    @property
    def resolution(self) -> int:
        """resolution - резрешение ЦАП в битах"""
        return self._resolution

    @property
    def unipolar(self) -> bool:
        """Если unipolar is True, то выходное напряжение изменяется в диапазоне 0..Vопорное,
        иначе: -Vопорное/2 ... Vопорное/2"""
        return self._unipolar