/**
  ******************************************************************************
  * @file    bsp_ds18b20.c
  * @brief   DS18B20 1-Wire driver on TEMP_BOARD (PC6).
  *
  *          The pin is switched between push-pull output (drive low) and
  *          input with pull-up (release the line) because the sensor is the
  *          only device on that net and no open drain driver is required.
  ******************************************************************************
  */

#include "bsp_ds18b20.h"
#include "bsp_dwt.h"
#include <string.h>

#define OW_T_RESET_US       750U
#define OW_T_PRESENCE_US    70U
#define OW_T_RESET_TAIL_US  410U
#define OW_T_SLOT1_US       60U
#define OW_T_SLOT0_US       60U
#define OW_T_RECOVERY_US    10U
#define OW_T_READ_SAMPLE_US 10U
#define OW_T_READ_TAIL_US   45U

static uint8_t s_present = 0U;

static void ow_low(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin   = TEMP_BOARD_Pin;
  gpio.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(TEMP_BOARD_GPIO_Port, &gpio);

  HAL_GPIO_WritePin(TEMP_BOARD_GPIO_Port, TEMP_BOARD_Pin, GPIO_PIN_RESET);
}

static void ow_release(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin  = TEMP_BOARD_Pin;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(TEMP_BOARD_GPIO_Port, &gpio);
}

static uint8_t ow_level(void)
{
  return (HAL_GPIO_ReadPin(TEMP_BOARD_GPIO_Port, TEMP_BOARD_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

static void ow_write_bit(uint8_t bit)
{
  ow_low();
  bsp_dwt_delay_us(bit != 0U ? 2U : OW_T_SLOT0_US);
  ow_release();
  bsp_dwt_delay_us(bit != 0U ? OW_T_SLOT1_US : OW_T_RECOVERY_US);
}

static uint8_t ow_read_bit(void)
{
  uint8_t bit;

  ow_low();
  bsp_dwt_delay_us(2U);
  ow_release();
  bsp_dwt_delay_us(OW_T_READ_SAMPLE_US);
  bit = ow_level();
  bsp_dwt_delay_us(OW_T_READ_TAIL_US);

  return bit;
}

static void ow_write_byte(uint8_t byte)
{
  uint8_t i;

  for (i = 0U; i < 8U; i++)
  {
    ow_write_bit((uint8_t)(byte & 0x01U));
    byte >>= 1;
  }
}

static uint8_t ow_read_byte(void)
{
  uint8_t i;
  uint8_t value = 0U;

  for (i = 0U; i < 8U; i++)
  {
    value >>= 1;
    if (ow_read_bit() != 0U)
    {
      value |= 0x80U;
    }
  }

  return value;
}

static uint8_t ow_crc8(const uint8_t *data, uint8_t len)
{
  uint8_t crc = 0U;
  uint8_t i;
  uint8_t bit;

  for (i = 0U; i < len; i++)
  {
    uint8_t byte = data[i];

    for (bit = 0U; bit < 8U; bit++)
    {
      uint8_t mix = (uint8_t)((crc ^ byte) & 0x01U);

      crc >>= 1;
      if (mix != 0U)
      {
        crc ^= 0x8CU;
      }
      byte >>= 1;
    }
  }

  return crc;
}

void bsp_ds18b20_init(void)
{
  __HAL_RCC_GPIOC_CLK_ENABLE();

  ow_release();
  bsp_dwt_delay_us(10U);
}

uint8_t bsp_ds18b20_reset(void)
{
  uint8_t presence;

  ow_low();
  bsp_dwt_delay_us(OW_T_RESET_US);
  ow_release();
  bsp_dwt_delay_us(OW_T_PRESENCE_US);

  presence = (ow_level() == 0U) ? 1U : 0U;

  bsp_dwt_delay_us(OW_T_RESET_TAIL_US);

  return presence;
}

uint8_t bsp_ds18b20_is_present(void)
{
  return s_present;
}

uint8_t bsp_ds18b20_start_conversion(void)
{
  if (bsp_ds18b20_reset() == 0U)
  {
    s_present = 0U;
    return BSP_ERROR;
  }

  s_present = 1U;

  ow_write_byte(BSP_DS18B20_ROM_SKIP);
  ow_write_byte(BSP_DS18B20_CONVERT);

  return BSP_OK;
}

uint8_t bsp_ds18b20_conversion_done(void)
{
  /* The sensor holds the line low during the conversion. */
  return ow_level();
}

uint8_t bsp_ds18b20_read_raw(int16_t *raw)
{
  uint8_t scratchpad[9];

  if (raw == 0)
  {
    return BSP_ERROR;
  }

  if (bsp_ds18b20_reset() == 0U)
  {
    s_present = 0U;
    return BSP_ERROR;
  }

  s_present = 1U;

  ow_write_byte(BSP_DS18B20_ROM_SKIP);
  ow_write_byte(BSP_DS18B20_READ_SCRATCH);

  for (uint8_t i = 0U; i < 9U; i++)
  {
    scratchpad[i] = ow_read_byte();
  }

  if (ow_crc8(scratchpad, 8U) != scratchpad[8])
  {
    return BSP_ERROR;
  }

  if (scratchpad[0] == 0x00U && scratchpad[1] == 0x00U)
  {
    /* All zero means no valid conversion result yet. */
    return BSP_ERROR;
  }

  *raw = (int16_t)(((uint16_t)scratchpad[1] << 8) | (uint16_t)scratchpad[0]);

  return BSP_OK;
}

uint8_t bsp_ds18b20_read_celsius(float *celsius)
{
  int16_t raw = 0;

  if (celsius == 0)
  {
    return BSP_ERROR;
  }

  if (bsp_ds18b20_read_raw(&raw) != BSP_OK)
  {
    return BSP_ERROR;
  }

  *celsius = (float)raw * 0.0625f;

  return BSP_OK;
}

uint8_t bsp_ds18b20_read_blocking(float *celsius, uint32_t timeout_ms)
{
  uint32_t start;

  if (bsp_ds18b20_start_conversion() != BSP_OK)
  {
    return BSP_ERROR;
  }

  start = HAL_GetTick();

  while (bsp_ds18b20_conversion_done() == 0U)
  {
    if ((uint32_t)(HAL_GetTick() - start) >= timeout_ms)
    {
      return BSP_ERROR;
    }

    bsp_dwt_delay_us(1000U);
  }

  return bsp_ds18b20_read_celsius(celsius);
}
