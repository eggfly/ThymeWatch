ULP:

* https://blog.csdn.net/Marchtwentytwo/article/details/132307946
* https://github.com/espressif/esp-idf/issues/10813
* https://www.espressif.com/sites/default/files/documentation/esp32-s2_datasheet_cn.pdf
* 

RTC 定时器时钟源
RTC 定时器有以下时钟源：

* 内置 136 kHz RC 振荡器 （默认）：Deep-sleep 模式下电流消耗最低，不依赖任何外部元件。但由于温度波动会影响该时钟源的频率稳定性，在 Deep-sleep 和 Light-sleep 模式下都有可能发生时间偏移。

* 外置 32 kHz 晶振：需要将一个 32 kHz 晶振连接到 XTAL_32K_P 和 XTAL_32K_N 管脚。频率稳定性更高，但在 Deep-sleep 模式下电流消耗略高（比默认模式高 1 μA）。

* 管脚 XTAL_32K_P 外置 32 kHz 振荡器：允许使用由外部电路产生的 32 kHz 时钟。外部时钟信号必须连接到管脚 XTAL_32K_P。正弦波信号的振幅应小于 1.2 V，方波信号的振幅应小于 1 V。正常模式下，电压范围应为 0.1 < Vcm < 0.5 xVamp，其中 Vamp 代表信号振幅。使用此时钟源时，管脚 XTAL_32K_P 无法用作 GPIO 管脚。

* 内置 17.5 MHz 振荡器的 256 分频时钟 (约 68 kHz)：频率稳定性优于 内置 136 kHz RC 振荡器，同样无需外部元件，但 Deep-sleep 模式下电流消耗更高（比默认模式高 5 μA）。


关于 RISC-V ULP
1. **ESP32-S3 的 RISC-V ULP 代码最大是多少？**

   ESP32-S3 的 ULP-RISC-V 协处理器可以使用最多 **8 KB 的 RTC SLOW 内存** 来存储指令和数据。因此，ULP-RISC-V 程序的代码和数据总大小最大为 **8 KB**。

2. **RTC FAST 和 RTC SLOW MEMORY 哪个会被 ULP 代码占用？**

   ULP-RISC-V 使用 **RTC SLOW MEMORY（RTC 慢速内存）** 来存放代码和数据。RTC SLOW MEMORY 的大小为 8 KB，ULP-RISC-V 可以完全使用这块内存。

3. **RISC-V 的 ULP 内存模型是什么，最大能使用多大内存？**

   ULP-RISC-V 的内存模型使用 **RTC SLOW MEMORY 作为指令和数据存储空间**，最大可使用内存为 **8 KB**。ULP-RISC-V 可以在 RTC SLOW MEMORY 中存储程序代码、静态数据和运行时数据。

   在您的示例代码中，可以看到 `RTC_SLOW_MEM` 被用作程序的基地址，这进一步证明了 ULP-RISC-V 使用 RTC SLOW MEMORY 来存储程序代码和数据：

   ```c
   uint8_t* base = (uint8_t*) RTC_SLOW_MEM;

   //Start by clearing memory reserved with zeros, this will also initialize the bss:
   hal_memset(base, 0, CONFIG_ULP_COPROC_RESERVE_MEM);
   hal_memcpy(base, program_binary, program_size_bytes);
   ```

   其中，`CONFIG_ULP_COPROC_RESERVE_MEM` 指定了为 ULP 保留的内存大小，不能超过 8 KB。

**补充说明：**

- **RTC SLOW MEMORY（RTC 慢速内存）** 是主 CPU 和 ULP-RISC-V 协处理器共享的内存区域。主 CPU 可以在进入深度睡眠（Deep-sleep）模式前，将程序代码加载到此内存中，然后启动 ULP-RISC-V 执行。

- **RTC FAST MEMORY（RTC 快速内存）** 主要用于存储在深度睡眠模式下需要快速访问的数据，但不被 ULP-RISC-V 使用。

- ULP-RISC-V **运行在 17.5 MHz 的频率下**，使用 `RTC_FAST_CLK` 时钟。

- ULP-RISC-V **支持使用 C 语言编写程序**，并编译为 RV32IMC 指令集的二进制代码。

- ULP-RISC-V 的程序和数据存储在 RTC SLOW MEMORY 内，地址范围为 **0x5000_0000 到 0x5000_1FFF** 或 **0x6002_1000 到 0x6002_2FFF**。

**结论：**

- **ULP-RISC-V 最大可以使用 8 KB 的 RTC SLOW MEMORY 来存储程序代码和数据。**

- **ULP-RISC-V 占用的是 RTC SLOW MEMORY，而不是 RTC FAST MEMORY。**

- **ULP-RISC-V 内存模型使用 RTC SLOW MEMORY 作为指令和数据存储空间，最大可使用内存为 8 KB。**
