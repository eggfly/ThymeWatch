# SPDX-FileCopyrightText: Copyright (c) 2023 Jose D. Montoya
#
# SPDX-License-Identifier: MIT
"""
`bmi270`
================================================================================

MicroPython Driver for the Bosch BMI270 Accelerometer/Gyro Sensor


* Author(s): Jose D. Montoya


"""

import time
from micropython import const
from micropython_bmi270.i2c_helpers import CBits, RegisterStruct


try:
    from typing import Tuple
except ImportError:
    pass

# pylint: disable=import-outside-toplevel

__version__ = "0.0.0+auto.0"
__repo__ = "https://github.com/jposada202020/MicroPython_BMI270.git"


_REG_WHOAMI = const(0x00)
_ERROR_CODE = const(0x02)
_COMMAND = const(0x7E)
_ACC_RANGE = const(0x41)
_PWR_CTRL = const(0x7D)
_GYRO_RANGE = const(0x43)

_STANDARD_GRAVITY = const(9.80665)

# Acceleration Data
ACC_X_LSB = const(0x0C)
ACC_Y_LSB = const(0x0E)
ACC_Z_LSB = const(0x10)

# Gyro Data
GYRO_X_LSB = const(0x12)
GYRO_Y_LSB = const(0x14)
GYRO_Z_LSB = const(0x16)

# Acceleration Range
ACCEL_RANGE_2G = const(0b00)
ACCEL_RANGE_4G = const(0b01)
ACCEL_RANGE_8G = const(0b10)
ACCEL_RANGE_16G = const(0b11)
acceleration_range_values = (
    ACCEL_RANGE_2G,
    ACCEL_RANGE_4G,
    ACCEL_RANGE_8G,
    ACCEL_RANGE_16G,
)

ACCELERATOR_DISABLED = const(0b0)
ACCELERATOR_ENABLED = const(0b1)
acceleration_operation_mode_values = (ACCELERATOR_DISABLED, ACCELERATOR_ENABLED)

GYRO_RANGE_2000 = const(0b000)
GYRO_RANGE_1000 = const(0b001)
GYRO_RANGE_500 = const(0b010)
GYRO_RANGE_250 = const(0b011)
GYRO_RANGE_125 = const(0b100)
gyro_range_values = (
    GYRO_RANGE_2000,
    GYRO_RANGE_1000,
    GYRO_RANGE_500,
    GYRO_RANGE_250,
    GYRO_RANGE_125,
)

# RESET Command
RESET_COMMAND = const(0xB6)

_PWR_CONF = const(0x7C)
_INIT_CTRL = const(0x59)
_INIT_ADDR_0 = const(0x5B)
_INIT_ADDR_1 = const(0x5C)
_INIT_DATA = const(0x5E)


class BMI270:
    """Driver for the BMI270 Sensor connected over I2C.

    :param ~machine.I2C i2c: The I2C bus the BMI270 is connected to.
    :param int address: The I2C device address. Defaults to :const:`0x69`

    :raises RuntimeError: if the sensor is not found

    **Quickstart: Importing and using the device**

    Here is an example of using the :class:`BMI270` class.
    First you will need to import the libraries to use the sensor

    .. code-block:: python

        from machine import Pin, I2C
        from micropython_bmi270 import bmi270

    Once this is done you can define your `machine.I2C` object and define your sensor object

    .. code-block:: python

        i2c = I2C(1, sda=Pin(2), scl=Pin(3))
        bmi270 = bmi270.BMI270(i2c)

    Now you have access to the attributes

    .. code-block:: python

        acc_x, acc_y, acc_z = bmi270.acceleration

    """

    _device_id = RegisterStruct(_REG_WHOAMI, "B")
    _error_code = RegisterStruct(_ERROR_CODE, "B")
    _soft_reset = RegisterStruct(_COMMAND, "B")
    _read = RegisterStruct(_COMMAND, "B")

    power_control = RegisterStruct(_PWR_CTRL, "B")
    power_config = RegisterStruct(0x7C, "B")

    _acc_data_x = RegisterStruct(ACC_X_LSB, "<h")
    _acc_data_y = RegisterStruct(ACC_Y_LSB, "<h")
    _acc_data_z = RegisterStruct(ACC_Z_LSB, "<h")

    # Gyro Data
    _gyro_data_x = RegisterStruct(GYRO_X_LSB, "<h")
    _gyro_data_y = RegisterStruct(GYRO_Y_LSB, "<h")
    _gyro_data_z = RegisterStruct(GYRO_Z_LSB, "<h")

    # GYRO_RANGE Register (0x43)
    _gyro_range = CBits(3, _GYRO_RANGE, 0)
    gyro_scale = (16.4, 32.8, 65.6, 131.2, 262.4)

    # ACC_RANGE Register (0x41)
    # The register allows the selection of the accelerometer g-range
    _acceleration_range = CBits(2, _ACC_RANGE, 0)
    acceleration_scale = (16384, 8192, 4096, 2048)

    _acceleration_operation_mode = CBits(1, _PWR_CTRL, 2)

    _power_configuration = RegisterStruct(_PWR_CONF, "B")

    internal_status = RegisterStruct(0x21, "B")

    _init_control = RegisterStruct(_INIT_CTRL, "B")

    _init_address_0 = RegisterStruct(_INIT_ADDR_0, "B")
    _init_address_1 = RegisterStruct(_INIT_ADDR_1, "B")
    _init_data = RegisterStruct(_INIT_DATA, ">HHHHHHHHHHHHHHHH")

    def __init__(self, i2c, address: int = 0x69) -> None:
        self._i2c = i2c
        self._address = address

        if self._device_id != 0x24:
            raise RuntimeError("Failed to find BMI270")

        # self.soft_reset()

        self.load_config_file()
        self.power_control = 0x0E
        time.sleep(0.1)
        self.power_config = 0x00
        time.sleep(0.1)
        self.acceleration_range = ACCEL_RANGE_2G
        self.gyro_range = GYRO_RANGE_250

    def error_code(self) -> None:
        """
        The register is meant for debug purposes, not for regular verification
        if an operation completed successfully.

        Fatal Error: Error during bootup. This flag will not be cleared after
        reading the register.The only way to clear the flag is a POR.


        """

        errors = self._error_code
        i2c_err = (errors & 0x80) >> 7
        fifo_err = (errors & 0x40) >> 6
        internal_error = (errors & 0x1E) >> 1
        fatal_error = errors & 0x01
        if i2c_err:
            print("Error in I2C-Master detected. This flag will be reset when read.")
        if fifo_err:
            print(
                "Error when a frame is read in streaming mode (so skipping is not possible) \n"
                + "and fifo is overfilled (with virtual and/or regular frames). This flag will\n"
                + "be reset when read."
            )
        if internal_error != 0:
            print("Internal Sensor Error")
        if fatal_error:
            print("Fatal Error. This flag will be reset when read")

    def soft_reset(self) -> None:
        """
        Performs a Soft Reset

        :return: None

        """
        self._soft_reset = RESET_COMMAND
        time.sleep(0.015)

    @property
    def acceleration(self) -> Tuple[float, float, float]:
        """
        Sensor Acceleration in :math:`m/s^2`
        """

        x = self._acc_data_x / self._acceleration_factor_cached * _STANDARD_GRAVITY
        y = self._acc_data_y / self._acceleration_factor_cached * _STANDARD_GRAVITY
        z = self._acc_data_z / self._acceleration_factor_cached * _STANDARD_GRAVITY

        return x, y, z

    @property
    def acceleration_range(self) -> str:
        """
        Sensor acceleration_range

        +------------------------------------+------------------+
        | Mode                               | Value            |
        +====================================+==================+
        | :py:const:`bmi270.ACCEL_RANGE_2G`  | :py:const:`0b00` |
        +------------------------------------+------------------+
        | :py:const:`bmi270.ACCEL_RANGE_4G`  | :py:const:`0b01` |
        +------------------------------------+------------------+
        | :py:const:`bmi270.ACCEL_RANGE_8G`  | :py:const:`0b10` |
        +------------------------------------+------------------+
        | :py:const:`bmi270.ACCEL_RANGE_16G` | :py:const:`0b11` |
        +------------------------------------+------------------+
        """
        values = (
            "ACCEL_RANGE_2G",
            "ACCEL_RANGE_4G",
            "ACCEL_RANGE_8G",
            "ACCEL_RANGE_16G",
        )
        return values[self._acceleration_range]

    @acceleration_range.setter
    def acceleration_range(self, value: int) -> None:
        if value not in acceleration_range_values:
            raise ValueError("Value must be a valid acceleration_range setting")
        self._acceleration_range = value
        self._acceleration_factor_cached = self.acceleration_scale[value]

    @property
    def acceleration_operation_mode(self) -> str:
        """
        Sensor acceleration_operation_mode

        +-----------------------------------------+-----------------+
        | Mode                                    | Value           |
        +=========================================+=================+
        | :py:const:`bmi270.ACCELERATOR_DISABLED` | :py:const:`0b0` |
        +-----------------------------------------+-----------------+
        | :py:const:`bmi270.ACCELERATOR_ENABLED`  | :py:const:`0b1` |
        +-----------------------------------------+-----------------+
        """
        values = ("ACCELERATOR_DISABLED", "ACCELERATOR_ENABLED")
        return values[self._acceleration_operation_mode]

    @acceleration_operation_mode.setter
    def acceleration_operation_mode(self, value: int) -> None:
        if value not in acceleration_operation_mode_values:
            raise ValueError(
                "Value must be a valid acceleration_operation_mode setting"
            )
        self._acceleration_operation_mode = value

    def load_config_file(self) -> None:
        """
        Load configuration file. This is necessary to use the sensor.
        Script adapted to use with MicroPython from:
        https://github.com/CoRoLab-Berlin/bmi270_python
        (c) 2023 MIT License Kevin Sommler
        """
        if self.internal_status == 0x01:
            print(hex(self._address), " --> Initialization already done")
        else:
            from micropython_bmi270.config_file import bmi270_config_file

            print(hex(self._address), " --> Initializing...")
            self._power_configuration = 0x00
            time.sleep(0.00045)
            self._init_control = 0x00
            for i in range(256):
                self._init_address_0 = 0x00
                self._init_address_1 = i
                time.sleep(0.03)
                self._i2c.writeto_mem(
                    self._address,
                    0x5E,
                    bytes(bmi270_config_file[i * 32 : (i + 1) * 32]),
                )
                time.sleep(0.000020)
            self._init_control = 0x01
            time.sleep(0.02)
            print(
                hex(self._address),
                " --> Initialization status: "
                + "{:08b}".format(self.internal_status)
                + "\t(00000001 --> OK)",
            )

    @property
    def gyro_range(self) -> str:
        """
        Sensor gyro_range

        +------------------------------------+-------------------+
        | Mode                               | Value             |
        +====================================+===================+
        | :py:const:`bmi270.GYRO_RANGE_2000` | :py:const:`0b000` |
        +------------------------------------+-------------------+
        | :py:const:`bmi270.GYRO_RANGE_1000` | :py:const:`0b001` |
        +------------------------------------+-------------------+
        | :py:const:`bmi270.GYRO_RANGE_500`  | :py:const:`0b010` |
        +------------------------------------+-------------------+
        | :py:const:`bmi270.GYRO_RANGE_250`  | :py:const:`0b011` |
        +------------------------------------+-------------------+
        | :py:const:`bmi270.GYRO_RANGE_125`  | :py:const:`0b100` |
        +------------------------------------+-------------------+
        """
        values = (
            "GYRO_RANGE_2000",
            "GYRO_RANGE_1000",
            "GYRO_RANGE_500",
            "GYRO_RANGE_250",
            "GYRO_RANGE_125",
        )
        return values[self._gyro_range]

    @gyro_range.setter
    def gyro_range(self, value: int) -> None:
        if value not in gyro_range_values:
            raise ValueError("Value must be a valid gyro_range setting")
        self._gyro_range = value
        self._gyro_factor_cached = self.gyro_scale[value]

    @property
    def gyro(self) -> Tuple[float, float, float]:
        """
        Gyro values
        """

        x = self._gyro_data_x / self._gyro_factor_cached
        y = self._gyro_data_y / self._gyro_factor_cached
        z = self._gyro_data_z / self._gyro_factor_cached
        return x, y, z
