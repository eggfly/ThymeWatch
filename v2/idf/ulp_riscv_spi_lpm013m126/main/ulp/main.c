
/*
 * 该代码运行在 ESP32-S3 的 ULP-RISC-V 协处理器上，用于通过 SPI 接口刷新屏幕，显示单一颜色
 */

#include "ulp_riscv.h"
#include "ulp_riscv_gpio.h"
#include "ulp_riscv_utils.h"

#define GPIO_SCK GPIO_NUM_6  // SPI 时钟引脚
#define GPIO_MOSI GPIO_NUM_7 // SPI 数据引脚
#define GPIO_SS GPIO_NUM_8   // SPI 片选引脚

#define BLINK_GPIO GPIO_NUM_14

#define ULP_FREQ 17500000UL // 17.5 MHz
// #define DELAY_CYCLES(ns) ((ULP_FREQ / 1000000UL) * (ns) / 1000UL)

#define SCREEN_WIDTH 176  // 屏幕宽度
#define SCREEN_HEIGHT 176 // 屏幕高度

/* this variable will be exported as a public symbol, visible from main CPU: */
// bool gpio_level_previous = false;
uint32_t cost_cycles = 0;


uint8_t screen_buffer[SCREEN_WIDTH * 16]; // SPI 数据缓冲区
// 32 不行了
// 16 可以

/*                                   R, G, B     */
#define LCD_COLOR_BLACK (0x00)   /*  0  0  0  0  */
#define LCD_COLOR_BLUE (0x02)    /*  0  0  1  0  */
#define LCD_COLOR_GREEN (0x04)   /*  0  1  0  0  */
#define LCD_COLOR_CYAN (0x06)    /*  0  1  1  0  */
#define LCD_COLOR_RED (0x08)     /*  1  0  0  0  */
#define LCD_COLOR_MAGENTA (0x0a) /*  1  0  1  0  */
#define LCD_COLOR_YELLOW (0x0c)  /*  1  1  0  0  */
#define LCD_COLOR_WHITE (0x0e)   /*  1  1  1  0  */

// SPI 发送一个字节（软件实现的 SPI）
inline __attribute__((always_inline)) static void spi_write_byte(uint8_t data)
{
    for (int i = 0; i < 8; i++)
    {
        // 设置 MOSI 电平
        ulp_riscv_gpio_output_level(GPIO_MOSI, (data & 0x80) ? 1 : 0);

        // 拉高 SCK
        ulp_riscv_gpio_output_level(GPIO_SCK, 1);
        // 等待一段时间，控制时钟频率
        // ulp_riscv_delay_cycles(DELAY_CYCLES(1000)); // 调整延迟以满足时序要求

        // 拉低 SCK
        ulp_riscv_gpio_output_level(GPIO_SCK, 0);
        // ulp_riscv_delay_cycles(DELAY_CYCLES(1000)); // 调整延迟以满足时序要求

        // 左移数据
        data <<= 1;
    }
}

// 将屏幕刷新为指定颜色，data_byte 为要写入的字节，例如全红色为 0x88（RGB111 格式）
static void spi_screen_refresh(uint8_t data_byte)
{
    cost_cycles++;
    // uint8_t line_data[SCREEN_WIDTH / 2];

    // 准备一行的数据（假设屏幕为 RGB111 格式，每字节两个像素）
    for (int i = 0; i < SCREEN_WIDTH / 2; i++)
    {
        screen_buffer[i] = data_byte; // 设置为指定颜色
    }

    // CS 低电平
    ulp_riscv_gpio_output_level(GPIO_SS, 0);

    // 开始传输，拉高 CS
    ulp_riscv_gpio_output_level(GPIO_SS, 1);

    // 逐行刷新屏幕
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        spi_write_byte(0x90);
        // 发送行地址（假设屏幕接受行地址）
        spi_write_byte((uint8_t)(y + 1));

        // 发送行数据
        for (int x = 0; x < SCREEN_WIDTH / 2; x++)
        {
            spi_write_byte(screen_buffer[x]);
        }

        // 发送行结束字节（假设需要）
        // spi_write_byte(0x00); // 不需要了
        // 结束传输
        // ulp_riscv_delay_cycles(DELAY_CYCLES(1000)); // 调整延迟以满足时序要求
    }
    ulp_riscv_gpio_output_level(GPIO_SS, 0);
}

int main(void)
{
    // 初始化引脚
    ulp_riscv_gpio_init(GPIO_SCK);
    ulp_riscv_gpio_output_enable(GPIO_SCK);
    ulp_riscv_gpio_set_output_mode(GPIO_SCK, RTCIO_MODE_OUTPUT);

    ulp_riscv_gpio_init(GPIO_MOSI);
    ulp_riscv_gpio_output_enable(GPIO_MOSI);
    ulp_riscv_gpio_set_output_mode(GPIO_MOSI, RTCIO_MODE_OUTPUT);

    ulp_riscv_gpio_init(GPIO_SS);
    ulp_riscv_gpio_output_enable(GPIO_SS);
    ulp_riscv_gpio_set_output_mode(GPIO_SS, RTCIO_MODE_OUTPUT);

    ulp_riscv_gpio_init(BLINK_GPIO);
    ulp_riscv_gpio_output_enable(BLINK_GPIO);

    // 设置 背光 输出高电平
    ulp_riscv_gpio_output_level(BLINK_GPIO, 1);
    while (1)
    {
        // 11 frames cost 18 seconds
        spi_screen_refresh(LCD_COLOR_BLACK | LCD_COLOR_BLACK << 4);
        spi_screen_refresh(LCD_COLOR_BLUE | LCD_COLOR_BLUE << 4);
        spi_screen_refresh(LCD_COLOR_GREEN | LCD_COLOR_GREEN << 4);
        spi_screen_refresh(LCD_COLOR_CYAN | LCD_COLOR_CYAN << 4);
        spi_screen_refresh(LCD_COLOR_RED | LCD_COLOR_RED << 4);
        spi_screen_refresh(LCD_COLOR_MAGENTA | LCD_COLOR_MAGENTA << 4);
        spi_screen_refresh(LCD_COLOR_YELLOW | LCD_COLOR_YELLOW << 4);
        spi_screen_refresh(LCD_COLOR_WHITE | LCD_COLOR_WHITE << 4);
    }

    // 进入休眠或停止程序
    // ulp_riscv_halt();

    return 0;
}
