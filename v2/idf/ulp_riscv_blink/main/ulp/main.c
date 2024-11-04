/*
 * 该代码运行在 ESP32-S3 的 ULP-RISC-V 协处理器上，用于使 GPIO 闪烁
 */

#include "ulp_riscv.h"
#include "ulp_riscv_utils.h"
#include "ulp_riscv_gpio.h"

#define BLINK_GPIO GPIO_NUM_14
#define ULP_FREQ (17500000)

#define MS_TO_CYCLES(ms) (ULP_FREQ / 1000 * ms)

int main(void)
{
    ulp_riscv_gpio_init(BLINK_GPIO);
    ulp_riscv_gpio_output_enable(BLINK_GPIO);

    while (1) {
        // 设置 GPIO48 输出高电平
        ulp_riscv_gpio_output_level(BLINK_GPIO, 1);
        // 延迟一段时间
        ulp_riscv_delay_cycles(MS_TO_CYCLES(500));

        // 设置 GPIO48 输出低电平
        ulp_riscv_gpio_output_level(BLINK_GPIO, 0);
        // 延迟一段时间
        ulp_riscv_delay_cycles(MS_TO_CYCLES(500));
    }

    // 终止程序（ULP-RISC-V 程序在 main 函数结束后会自动进入休眠）
    // 貌似不会休眠，会一直进入main函数
    return 0;
}
