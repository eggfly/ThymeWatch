

#include <stdio.h>
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_periph.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
// #include "ulp_riscv.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_periph.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
// #include "ulp_main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hal/misc.h"
#include <string.h>

#include "ulp_riscv_text.h"

#define CONFIG_ULP_COPROC_RESERVE_MEM 4096
#define SOC_RTC_DATA_LOW  0x50000000
#define CONFIG_IDF_TARGET_ESP32S3 1

// extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
// extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

static void init_ulp_program(void);
typedef enum {
    ULP_RISCV_WAKEUP_SOURCE_TIMER,
    ULP_RISCV_WAKEUP_SOURCE_GPIO,
} ulp_riscv_wakeup_source_t;

typedef struct {
    ulp_riscv_wakeup_source_t wakeup_source; /*!< ULP wakeup source */
} ulp_riscv_cfg_t;


void ulp_riscv_timer_stop(void)
{
  CLEAR_PERI_REG_MASK(RTC_CNTL_ULP_CP_TIMER_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
}

void ulp_riscv_halt(void)
{
  ulp_riscv_timer_stop();

  /* suspends the ulp operation*/
  SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_DONE);

  /* Resets the processor */
  SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_SHUT_RESET_EN);
}
void loop(){}
void setup(void)
{
  vTaskDelay(pdMS_TO_TICKS(1000));

  /* Initialize selected GPIO as RTC IO, enable input, disable pullup and pulldown */
  rtc_gpio_init(GPIO_NUM_0);
  rtc_gpio_set_direction(GPIO_NUM_0, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pulldown_dis(GPIO_NUM_0);
  rtc_gpio_pullup_dis(GPIO_NUM_0);
  rtc_gpio_hold_en(GPIO_NUM_0);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  /* not a wakeup from ULP, load the firmware */
  if (cause != ESP_SLEEP_WAKEUP_ULP)
  {
    printf("Not a ULP-RISC-V wakeup, initializing it! \n");
    ulp_riscv_halt();
    init_ulp_program();
  }

  /* ULP Risc-V read and detected a change in GPIO_0, prints */
  if (cause == ESP_SLEEP_WAKEUP_ULP)
  {
    printf("ULP-RISC-V woke up the main CPU! \n");
    // printf("ULP-RISC-V read changes in GPIO_0 current is: %s \n",
    //        (bool)(ulp_gpio_level_previous == 0) ? "Low" : "High");
  }

  /* Go back to sleep, only the ULP Risc-V will run */
  printf("Entering in deep sleep\n\n");

  /* Small delay to ensure the messages are printed */
  vTaskDelay(100);
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    // printf("LOOP %ld\n", ulp_cost_cycles);
  }

  ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
  // esp_deep_sleep_start();
}
static esp_err_t ulp_riscv_config_wakeup_source(ulp_riscv_wakeup_source_t wakeup_source)
{
    esp_err_t ret = ESP_OK;

    switch (wakeup_source) {
    case ULP_RISCV_WAKEUP_SOURCE_TIMER:
        /* start ULP_TIMER */
        CLEAR_PERI_REG_MASK(RTC_CNTL_ULP_CP_CTRL_REG, RTC_CNTL_ULP_CP_FORCE_START_TOP);
        SET_PERI_REG_MASK(RTC_CNTL_ULP_CP_TIMER_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
        break;

    case ULP_RISCV_WAKEUP_SOURCE_GPIO:
        SET_PERI_REG_MASK(RTC_CNTL_ULP_CP_TIMER_REG, RTC_CNTL_ULP_CP_GPIO_WAKEUP_ENA);
        break;

    default:
        ret = ESP_ERR_INVALID_ARG;
    }

    return ret;
}

esp_err_t ulp_riscv_config_and_run(ulp_riscv_cfg_t* cfg)
{
    esp_err_t ret = ESP_OK;

#if CONFIG_IDF_TARGET_ESP32S2
    /* Reset COCPU when power on. */
    SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_SHUT_RESET_EN);

    /* The coprocessor cpu trap signal doesnt have a stable reset value,
      force ULP-RISC-V clock on to stop RTC_COCPU_TRAP_TRIG_EN from waking the CPU*/
    SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_CLK_FO);

    /* Disable ULP timer */
    CLEAR_PERI_REG_MASK(RTC_CNTL_ULP_CP_TIMER_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
    /* wait for at least 1 RTC_SLOW_CLK cycle */
    esp_rom_delay_us(20);
    /* Select RISC-V as the ULP_TIMER trigger target. */
    CLEAR_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_SEL);

    /* Select ULP-RISC-V to send the DONE signal. */
    SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_DONE_FORCE);

    ret = ulp_riscv_config_wakeup_source(cfg->wakeup_source);

#elif CONFIG_IDF_TARGET_ESP32S3
    /* Reset COCPU when power on. */
    SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_SHUT_RESET_EN);

    /* The coprocessor cpu trap signal doesnt have a stable reset value,
      force ULP-RISC-V clock on to stop RTC_COCPU_TRAP_TRIG_EN from waking the CPU*/
    SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_CLK_FO);

    /* Disable ULP timer */
    CLEAR_PERI_REG_MASK(RTC_CNTL_ULP_CP_TIMER_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN);
    /* wait for at least 1 RTC_SLOW_CLK cycle */
    esp_rom_delay_us(20);

    /* We do not select RISC-V as the Coprocessor here as this could lead to a hang
     * in the main CPU. Instead, we reset RTC_CNTL_COCPU_SEL after we have enabled the ULP timer.
     *
     * IDF-4510
     */
    //CLEAR_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_SEL);

    /* Select ULP-RISC-V to send the DONE signal */
    SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_DONE_FORCE);

    /* Set the CLKGATE_EN signal */
    SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_CLKGATE_EN);

    ret = ulp_riscv_config_wakeup_source(cfg->wakeup_source);

    /* Select RISC-V as the ULP_TIMER trigger target
     * Selecting the RISC-V as the Coprocessor at the end is a workaround
     * for the hang issue recorded in IDF-4510.
     */
    CLEAR_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_SEL);

    /* Clear any spurious wakeup trigger interrupts upon ULP startup */
    esp_rom_delay_us(20);
    REG_WRITE(RTC_CNTL_INT_CLR_REG, RTC_CNTL_COCPU_INT_CLR | RTC_CNTL_COCPU_TRAP_INT_CLR | RTC_CNTL_ULP_CP_INT_CLR);

#endif

    return ret;
}

#define ULP_RISCV_DEFAULT_CONFIG()                      \
    {                                                   \
        .wakeup_source = ULP_RISCV_WAKEUP_SOURCE_TIMER, \
    }

esp_err_t ulp_riscv_run(void)
{
    ulp_riscv_cfg_t cfg = ULP_RISCV_DEFAULT_CONFIG();
    return ulp_riscv_config_and_run(&cfg);
}


esp_err_t my_ulp_riscv_load_binary(const uint8_t *program_binary, size_t program_size_bytes)
{
  if (program_binary == NULL)
  {
    return ESP_ERR_INVALID_ARG;
  }
  if (program_size_bytes > CONFIG_ULP_COPROC_RESERVE_MEM)
  {
    return ESP_ERR_INVALID_SIZE;
  }

  uint8_t *base = (uint8_t *)SOC_RTC_DATA_LOW;

  // Start by clearing memory reserved with zeros, this will also will initialize the bss:
  memset(base, 0, CONFIG_ULP_COPROC_RESERVE_MEM); // 4KB
  // hal_memset(base, 0, 8192 * 10); // 4KB
  memcpy(base, program_binary, program_size_bytes);
  printf("program_size_bytes: %d\n", program_size_bytes);
  return ESP_OK;
}

static void init_ulp_program(void)
{
  esp_err_t err = my_ulp_riscv_load_binary(ulp_main_bin, ulp_main_bin_length);
  ESP_ERROR_CHECK(err);

  /* The first argument is the period index, which is not used by the ULP-RISC-V timer
   * The second argument is the period in microseconds, which gives a wakeup time period of: 20ms
   */
  // ulp_set_wakeup_period(0, 20000);

  /* Start the program */
  err = ulp_riscv_run();
  ESP_ERROR_CHECK(err);
}

