// 文件：src/ulp_spi_example.c  

#include "ulp_riscv.h"  
#include "ulp_riscv_utils.h"  
#include "ulp_riscv_gpio.h"  

#define GPIO_MOSI 2  // 替换为您的实际 GPIO 编号  
#define GPIO_SCLK 3  
#define GPIO_CS   4  

#define SPI_DELAY_CYCLES 20  // 根据需要调整延迟周期  

void ulp_main(void)  
{  
    // 初始化 GPIO 引脚  
    ulp_riscv_gpio_init(GPIO_MOSI);  
    ulp_riscv_gpio_output_enable(GPIO_MOSI);  

    ulp_riscv_gpio_init(GPIO_SCLK);  
    ulp_riscv_gpio_output_enable(GPIO_SCLK);  

    ulp_riscv_gpio_init(GPIO_CS);  
    ulp_riscv_gpio_output_enable(GPIO_CS);  

    // 默认拉高 CS 引脚  
    ulp_riscv_gpio_output_level(GPIO_CS, 1);  

    while (1)  
    {  
        uint8_t data = 0xA5;  // 您要发送的数据  

        // 开始 SPI 通信  
        ulp_riscv_gpio_output_level(GPIO_CS, 0);  
        ulp_riscv_delay_cycles(SPI_DELAY_CYCLES);  

        // 逐位发送数据（从 MSB 到 LSB）  
        for (int i = 7; i >= 0; i--)  
        {  
            // 设置 MOSI 电平  
            uint8_t bit = (data >> i) & 0x01;  
            ulp_riscv_gpio_output_level(GPIO_MOSI, bit);  

            // 产生时钟上升沿  
            ulp_riscv_gpio_output_level(GPIO_SCLK, 1);  
            ulp_riscv_delay_cycles(SPI_DELAY_CYCLES);  

            // 产生时钟下降沿  
            ulp_riscv_gpio_output_level(GPIO_SCLK, 0);  
            ulp_riscv_delay_cycles(SPI_DELAY_CYCLES);  
        }  

        // 结束 SPI 通信  
        ulp_riscv_gpio_output_level(GPIO_CS, 1);  

        // 延时一段时间  
        ulp_riscv_delay_cycles(1000);  
    }  
}
