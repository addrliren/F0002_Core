/**
  ******************************************************************************
  * @file    bsp_eeprom.c
  * @brief   BL24C02F-PARC EEPROM driver.
  ******************************************************************************
  */

#include "bsp_eeprom.h"
#include "bsp_soft_i2c.h"
#include "bsp_dwt.h"
#include <string.h>

static uint8_t s_present = 0U;

/* Acknowledge polling: the device does not answer while its write cycle is
   still running. */
static uint8_t eeprom_wait_ready(uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();

  for (;;)
  {
    uint8_t acked;

    bsp_soft_i2c_start();
    acked = bsp_soft_i2c_write_byte(BSP_EEPROM_ADDR);
    bsp_soft_i2c_stop();

    if (acked == BSP_OK)
    {
      return BSP_OK;
    }

    if ((uint32_t)(HAL_GetTick() - start) >= timeout_ms)
    {
      return BSP_ERROR;
    }

    bsp_dwt_delay_us(200U);
  }
}

uint8_t bsp_eeprom_init(void)
{
  bsp_soft_i2c_init();

  s_present = (eeprom_wait_ready(20U) == BSP_OK) ? 1U : 0U;

  return (s_present != 0U) ? BSP_OK : BSP_ERROR;
}

uint8_t bsp_eeprom_is_present(void)
{
  return s_present;
}

uint8_t bsp_eeprom_read(uint16_t addr, void *buf, uint16_t len)
{
  uint8_t *dst = (uint8_t *)buf;
  uint16_t i;

  if ((dst == 0) || (len == 0U) || ((uint32_t)addr + (uint32_t)len > BSP_EEPROM_SIZE))
  {
    return BSP_ERROR;
  }

  bsp_soft_i2c_start();
  if (bsp_soft_i2c_write_byte(BSP_EEPROM_ADDR) != BSP_OK)
  {
    bsp_soft_i2c_stop();
    return BSP_ERROR;
  }

  if (bsp_soft_i2c_write_byte((uint8_t)(addr & 0xFFU)) != BSP_OK)
  {
    bsp_soft_i2c_stop();
    return BSP_ERROR;
  }

  bsp_soft_i2c_start();
  if (bsp_soft_i2c_write_byte(BSP_EEPROM_ADDR | 0x01U) != BSP_OK)
  {
    bsp_soft_i2c_stop();
    return BSP_ERROR;
  }

  for (i = 0U; i < len; i++)
  {
    dst[i] = bsp_soft_i2c_read_byte((i + 1U < len) ? 1U : 0U);
  }

  bsp_soft_i2c_stop();

  return BSP_OK;
}

uint8_t bsp_eeprom_write(uint16_t addr, const void *buf, uint16_t len)
{
  const uint8_t *src = (const uint8_t *)buf;
  uint16_t written = 0U;

  if ((src == 0) || (len == 0U) || ((uint32_t)addr + (uint32_t)len > BSP_EEPROM_SIZE))
  {
    return BSP_ERROR;
  }

  while (written < len)
  {
    uint16_t page_free = (uint16_t)(BSP_EEPROM_PAGE_SIZE -
                                    ((addr + written) % BSP_EEPROM_PAGE_SIZE));
    uint16_t chunk = (uint16_t)(len - written);
    uint16_t i;

    if (chunk > page_free)
    {
      chunk = page_free;
    }

    if (eeprom_wait_ready(20U) != BSP_OK)
    {
      return BSP_ERROR;
    }

    bsp_soft_i2c_start();
    if (bsp_soft_i2c_write_byte(BSP_EEPROM_ADDR) != BSP_OK)
    {
      bsp_soft_i2c_stop();
      return BSP_ERROR;
    }

    if (bsp_soft_i2c_write_byte((uint8_t)((addr + written) & 0xFFU)) != BSP_OK)
    {
      bsp_soft_i2c_stop();
      return BSP_ERROR;
    }

    for (i = 0U; i < chunk; i++)
    {
      if (bsp_soft_i2c_write_byte(src[written + i]) != BSP_OK)
      {
        bsp_soft_i2c_stop();
        return BSP_ERROR;
      }
    }

    bsp_soft_i2c_stop();
    bsp_dwt_delay_ms(BSP_EEPROM_WRITE_MS);

    written = (uint16_t)(written + chunk);
  }

  if (eeprom_wait_ready(20U) != BSP_OK)
  {
    return BSP_ERROR;
  }

#if (BSP_EEPROM_VERIFY != 0)
  {
    uint8_t verify[BSP_EEPROM_PAGE_SIZE];
    uint16_t offset = 0U;

    while (offset < len)
    {
      uint16_t chunk = (uint16_t)(len - offset);

      if (chunk > BSP_EEPROM_PAGE_SIZE)
      {
        chunk = BSP_EEPROM_PAGE_SIZE;
      }

      if (bsp_eeprom_read((uint16_t)(addr + offset), verify, chunk) != BSP_OK)
      {
        return BSP_ERROR;
      }

      if (memcmp(verify, &src[offset], chunk) != 0)
      {
        return BSP_ERROR;
      }

      offset = (uint16_t)(offset + chunk);
    }
  }
#endif

  return BSP_OK;
}

uint8_t bsp_eeprom_read_byte(uint16_t addr, uint8_t *value)
{
  return bsp_eeprom_read(addr, value, 1U);
}

uint8_t bsp_eeprom_write_byte(uint16_t addr, uint8_t value)
{
  return bsp_eeprom_write(addr, &value, 1U);
}
