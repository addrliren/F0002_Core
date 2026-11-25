/**
  ******************************************************************************
  * @file    app_main.c
  * @brief   Application entry points: task registration and main loop.
  *
  *          Task periods:
  *            10  ms : LED, keys, buzzer, switch command, UART recovery
  *            20  ms : HMI frame parser
  *            100 ms : TF card detect, ADJ voltage/current
  *            500 ms : VDDA, MCU temperature, DS18B20 conversion
  *
  *          The watchdog is fed from the main loop, not from an interrupt, so a
  *          blocked or crashed task lets the IWDG reset the box.
  ******************************************************************************
  */

#include "app_main.h"
#include "app_sched.h"
#include "app_log.h"
#include "app_led.h"
#include "app_switch.h"
#include "app_hmi.h"
#include "app_board.h"

static uint32_t s_last_feed_ms = 0U;

#if (APP_SWITCH_KEEP_LED != 0)
static uint32_t s_hmi_led_ms = 0U;
#endif

static void app_watchdog_service(void)
{
  uint32_t now = HAL_GetTick();

  if ((uint32_t)(now - s_last_feed_ms) >= BSP_WDG_FEED_PERIOD_MS)
  {
    s_last_feed_ms = now;
    bsp_wdg_feed();
  }
}

static void app_log_identity(void)
{
  APP_LOGI("APP", "%s / %s", BSP_MCU_NAME, BSP_BOARD_NAME);

  if (bsp_wdg_caused_reset() != 0U)
  {
    APP_LOGW("APP", "previous reset caused by the independent watchdog");
  }
  else
  {
    APP_LOGI("APP", "power on reset");
  }

  APP_LOGI("APP", "dwt=%u eeprom=%u flash=0x%06lX",
           (unsigned)bsp_dwt_is_ready(),
           (unsigned)bsp_eeprom_is_present(),
           (unsigned long)bsp_flash_read_id());
}

void app_init(void)
{
  s_last_feed_ms = HAL_GetTick();

  app_sched_init();

  /* Board drivers first: DWT, rails, UARTs, EEPROM, flash, 1-Wire, ADC. */
  bsp_init();

  app_log_init();
  app_log_identity();

  app_led_init();
  app_switch_init();
  app_hmi_init();
  app_board_init();

  (void)app_sched_add("led",        app_led_task,          10U);
  (void)app_sched_add("switch",     app_switch_poll,       10U);
  (void)app_sched_add("board-fast", app_board_task_fast,   10U);
  (void)app_sched_add("uart",       bsp_uart_poll,         10U);
  (void)app_sched_add("hmi",        app_hmi_poll,          20U);
  (void)app_sched_add("board-med",  app_board_task_medium, 100U);
  (void)app_sched_add("board-slow", app_board_task_slow,   500U);

  APP_LOGI("APP", "%u tasks registered", (unsigned)app_sched_get_task_count());
}

void app_loop(void)
{
  /* After the USB-UART has been routed to the screen every task is stopped on
     purpose: only the watchdog (and the run indicator) stay alive. */
  if (app_switch_is_hmi_mode() != 0U)
  {
    app_watchdog_service();

#if (APP_SWITCH_KEEP_LED != 0)
    /* The scheduler is stopped, so the run indicator is driven from here. */
    if ((uint32_t)(HAL_GetTick() - s_hmi_led_ms) >= 300U)
    {
      s_hmi_led_ms = HAL_GetTick();
      bsp_led1_toggle();
    }
#endif

    bsp_dwt_delay_ms(1U);
    return;
  }

  (void)app_sched_run();
  app_watchdog_service();
}
