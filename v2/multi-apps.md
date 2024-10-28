要在 ESP32-C3 上同时运行 Arduino、PlatformIO 和 MicroPython，需要定制分区表和第二阶段引导加载程序，以选择启动不同的固件。以下是实现该目标的详细步骤，包括分区表布局、代码修改以及引导加载程序如何检测应用程序分区类型。

---

### **一、分区表设置**

首先，定义一个自定义的分区表 `partitions.csv`，为每个固件分配独立的应用程序分区。

**示例 `partitions.csv`：**

```csv
# Name,        Type, SubType, Offset,  Size,       Flags
nvs,           data, nvs,     0x9000,  0x5000,
otadata,       data, ota,     0xe000,  0x2000,
phy_init,      data, phy,     0x10000, 0x1000,
factory,       app,  factory, 0x20000, 0x140000,
arduino_app,   app,  ota_0,   ,        0x140000,
micropython,   app,  ota_1,   ,        0x140000,
spiffs,        data, spiffs,  ,        0x190000,
```

- **factory 分区**：默认的应用程序，可以是 PlatformIO 编译的固件。
- **arduino_app 分区**：用于存储 Arduino 固件。
- **micropython 分区**：用于存储 MicroPython 固件。
- **spiffs 分区**：共享的数据存储区域。

**注意事项：**

- 确保每个应用程序分区的大小合适，根据你的 Flash 大小进行调整。
- `Offset` 留空的分区会自动接续上一个分区的结束位置。

---

### **二、修改引导加载程序**

ESP32 的引导加载程序源码位于 `components/bootloader` 目录中。需要修改引导加载程序，使其能够根据条件（如 GPIO 状态、NVS 标志等）来选择启动哪个应用程序分区。

#### **1. 获取引导加载程序源码**

- 如果你使用的是 ESP-IDF 环境，直接在项目中修改即可。
- 如果没有，可以从 [ESP-IDF 的 GitHub 仓库](https://github.com/espressif/esp-idf)中获取对应版本的引导加载程序源码。

#### **2. 修改引导选择逻辑**

在 `components/bootloader/subproject/main/bootloader_start.c` 中，修改启动分区的选择逻辑。

**示例：根据 GPIO 引脚状态选择启动分区**

```c
#include "bootloader_common.h"
#include "bootloader_flash.h"
#include "bootloader_spi.h"
#include "esp_log.h"
#include "esp_image_format.h"
#include "esp_partition.h"
#include "hal/gpio_hal.h"

static const char *TAG = "bootloader";

void bootloader_main(void)
{
    // 初始化硬件
    bootloader_init();

    // 配置 GPIO 引脚作为输入
    gpio_hal_set_direction(GPIO_HAL_GET_HW(GPIO_NUM_0), GPIO_MODE_INPUT);
    gpio_hal_input_enable(GPIO_HAL_GET_HW(GPIO_NUM_0));

    // 读取 GPIO0 的电平
    int gpio_level = gpio_hal_get_level(GPIO_HAL_GET_HW(GPIO_NUM_0));

    esp_partition_subtype_t subtype_to_boot;

    if (gpio_level == 0) {
        ESP_LOGI(TAG, "GPIO0 is LOW, booting Arduino app");
        subtype_to_boot = ESP_PARTITION_SUBTYPE_APP_OTA_0; // arduino_app 分区
    } else {
        ESP_LOGI(TAG, "GPIO0 is HIGH, booting MicroPython app");
        subtype_to_boot = ESP_PARTITION_SUBTYPE_APP_OTA_1; // micropython 分区
    }

    // 查找要启动的分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP,
        subtype_to_boot,
        NULL);

    if (partition == NULL) {
        ESP_LOGE(TAG, "Partition not found!");
        // 处理错误，例如启动 factory 分区
        partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP,
            ESP_PARTITION_SUBTYPE_APP_FACTORY,
            NULL);
        if (partition == NULL) {
            ESP_LOGE(TAG, "Factory partition not found!");
            return;
        }
    }

    // 启动应用程序
    bootloader_utility_load_image(partition);
}
```

**说明：**

- **GPIO0**：在上电时读取 GPIO0 的电平，低电平启动 Arduino 应用，高电平启动 MicroPython。
- **错误处理**：如果找不到指��的分区，默认启动 factory 分区。
- **日志输出**：通过串口日志可以监控引导流程，方便调试。

#### **3. 编译自定义引导加载程序**

- 在项目目录中，编译引导加载程序：

  ```bash
  idf.py bootloader
  ```

- 生成的引导加载程序位于 `build/bootloader/bootloader.bin`。

---

### **三、烧录固件**

#### **1. 烧录引导加载程序和分区表**

```bash
esptool.py --chip esp32c3 --port /dev/ttyUSB0 write_flash 0x0 build/bootloader/bootloader.bin
esptool.py --chip esp32c3 --port /dev/ttyUSB0 write_flash 0x8000 build/partition_table/partition-table.bin
```

#### **2. 烧录应用程序固件**

- **烧录 factory 分区（PlatformIO 编译的固件）**

  ```bash
  esptool.py --chip esp32c3 --port /dev/ttyUSB0 write_flash 0x20000 factory_firmware.bin
  ```

- **烧录 Arduino 应用到 `arduino_app` 分区**

  ```bash
  esptool.py --chip esp32c3 --port /dev/ttyUSB0 write_flash 0x360000 arduino_firmware.bin
  ```

- **烧录 MicroPython 到 `micropython` 分区**

  ```bash
  esptool.py --chip esp32c3 --port /dev/ttyUSB0 write_flash 0x4A0000 micropython_firmware.bin
  ```

**注意：**

- 偏移地址应与分区表中的 `Offset` 对应。如果 `Offset` 留空，需手动计算正确的地址。
- 可以使用 `idf.py partition_table` 生成 `partition-table.bin`，并使用 `esptool.py` 烧录。

---

### **四、引导加载程序如何检测应用程序分区类型**

引导加载程序通过读取 Flash 中的分区表，获取每个分区的信息，包括类型 (`Type`) 和子类型 (`SubType`)。

- **Type：**
  - `0x00`：应用程序(`ESP_PARTITION_TYPE_APP`)
  - `0x01`：数据分区(`ESP_PARTITION_TYPE_DATA`)

- **SubType：**
  - 应用程序分区的子类型用于区分不同的应用程序，如：
    - `0x00`：factory（工厂分区）
    - `0x10`：ota_0
    - `0x11`：ota_1
    - 依次类推

引导加载程序使用 `esp_partition_find_first()` 函数，根据类型和子类型找到对应的分区，然后加载并运行。

---

### **五、示例：使用 NVS 存储启动选项**

如果希望在运行时灵活地选择启动分区，可以在应用程序中使用 NVS 存储标志位，引导加载程序读取该标志位决定启动哪个应用程序。

#### **1. 在应用程序中设置启动标志**

**示例代码：**

```c
#include "nvs_flash.h"
#include "nvs.h"

void set_boot_partition(int partition)
{
    // 初始化 NVS
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        // 处理错误
    }

    // 打开 NVS 命名空间
    nvs_handle_t nvs_handle;
    err = nvs_open("boot", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        // 处理错误
    }

    // 设置启动分区标志
    err = nvs_set_i32(nvs_handle, "boot_part", partition);
    if (err != ESP_OK) {
        // 处理错误
    }

    // 提交更改
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        // 处理错误
    }

    // 关闭 NVS
    nvs_close(nvs_handle);

    // 重启设备
    esp_restart();
}
```

- **说明：** `partition` 参数为整数，例如 `0` 表示 `arduino_app` 分区，`1` 表示 `micropython` 分区。

#### **2. 在引导加载程序中读取启动标志**

**示例代码修改（在 `bootloader_start.c` 中）：**

```c
#include "nvs_flash.h"
#include "nvs.h"

void bootloader_main(void)
{
    // 初始化硬件
    bootloader_init();

    // 初始化 NVS
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init NVS");
    }

    // 打开 NVS 命名空间
    nvs_handle_t nvs_handle;
    int32_t boot_partition = -1; // 默认值
    err = nvs_open("boot", NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        // 读取启动分区标志
        err = nvs_get_i32(nvs_handle, "boot_part", &boot_partition);
        nvs_close(nvs_handle);
    } else {
        ESP_LOGE(TAG, "Failed to open NVS");
    }

    esp_partition_subtype_t subtype_to_boot;

    if (boot_partition == 0) {
        ESP_LOGI(TAG, "Booting Arduino app from NVS setting");
        subtype_to_boot = ESP_PARTITION_SUBTYPE_APP_OTA_0;
    } else if (boot_partition == 1) {
        ESP_LOGI(TAG, "Booting MicroPython app from NVS setting");
        subtype_to_boot = ESP_PARTITION_SUBTYPE_APP_OTA_1;
    } else {
        ESP_LOGI(TAG, "Booting factory app as default");
        subtype_to_boot = ESP_PARTITION_SUBTYPE_APP_FACTORY;
    }

    // 查找要启动的分区
    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP,
        subtype_to_boot,
        NULL);

    if (partition == NULL) {
        ESP_LOGE(TAG, "Partition not found!");
        return;
    }

    // 启动应用程序
    bootloader_utility_load_image(partition);
}
```

**说明：**

- 引导加载程序初始化 NVS 并读取 `boot_part` 标志。
- 根据读取的值决定启动哪个分区。
- 如果未设置标志或读取失败，默认启动 factory 分区。

#### **3. 处理引导次数失败（可选）**

为了增加可靠性，可以在引导加载程序中添加逻辑，记录连续失败的引导次数，超过阈值后恢复到 factory 分区。

---

### **六、完整性和注意事项**

- **分区大小和地址**：确保分区表中的大小和偏移地址正确，避免覆盖其他分区。
- **备份重要数据**：在进行固件烧录和分区操作前，备份重要数据。
- **调试日志**：通过串口输出日志，方便调试引导流程。
- **引导加载程序大小限制**：引导加载程序的大小有限制，添加过多代码可能导致编译失败，需要优化代码大小。
- **NVS 初始化**：在引导加载程序中使用 NVS，需要使用最小化的 NVS 实现，确保引导加载程序的大小和功能。

---

### **七、总结**

通过自定义分区表和修改引导加载程序，可以在 ESP32-C3 上实现多固件的共存，并根据需要选择启动不同的应用程序。无论是通过 GPIO 引脚、电平检测还是 NVS 存储标志，都可以灵活地控制设备启动。

这种方式可以在开发过程中方便地切换不同的固件环境，例如在 Arduino、PlatformIO 和 MicroPython 之间切换，满足不同的开发需求。

---

希望以上内容能够帮助你实现 ESP32-C3 的多固件引导。如有其他问题，欢迎继续提问！
