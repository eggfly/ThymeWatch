The current tools of the ESP32 have no explicit option for that. However, the existing tools can be easily combined to do this.

The partition table is located at `0x8000` (32768) on older, and on `0x9000` (36384) on newer systems. Anyways, its location can be found (and be set) at the CONFIG_PARTITION_TABLE_OFFSET setting in the menuconfig. Its length is always `0xc00` (3072) bytes.

With the `esptool.py`, this can be read out, for example by the command
```
python $(IDF_PATH)/components/esptool_py/esptool/esptool.py read_flash 0x9000 0xc00 ptable.img
```
And then, the `gen_esp32part.py` tool can be used to convert it to csv to the stdout: `gen_esp32part.py ptable.img`.

For some scripting, these tools can be also combined, for example in a Linux
```
(python $(IDF_PATH)/components/esptool_py/esptool/esptool.py \
  read_flash 0x9000 0xc00 /dev/fd/3 >&2) 3>&1|python \
  $(IDF_PATH)/components/partition_table/gen_esp32part.py /dev/fd/0
```
will dump the table to the stdout.