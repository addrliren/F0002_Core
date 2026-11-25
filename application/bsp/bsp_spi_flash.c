/**
  ******************************************************************************
  * @file    bsp_spi_flash.c
  * @brief   W25Q128JVSIQ SPI NOR flash driver.
  ******************************************************************************
  */

#include "bsp_spi_flash.h"
#include "spi.h"
#include "bsp_dwt.h"
#include <string.h>

#define CMD_WRITE_ENABLE    0x06U
#define CMD_READ_STATUS1    0x05U
#define CMD_READ_DATA       0x03U
#define CMD_PAGE_PROGRAM    0x02U
#define CMD_SECTOR_ERASE    0x20U
#define CMD_JEDEC_ID        0x9FU

#define FLASH_SPI_TIMEOUT   100U
#define FLASH_READY_TIMEOUT 1000U
#define STATUS_BUSY         0x01U

static uint8_t s_present = 0U;
static uint32_t s_jedec_id = 0U;

static void flash_cs_low(void)
{
  HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_RESET);
}

static void flash_cs_high(void)
{
  HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_SET);
}

static uint8_t flash_send(const uint8_t *data, uint16_t len)
{
  if (HAL_SPI_Transmit(&hspi2, (uint8_t *)data, len, FLASH_SPI_TIMEOUT) != HAL_OK)
  {
    return BSP_ERROR;
  }

  return BSP_OK;
}

static uint8_t flash_send_cmd_addr(uint8_t cmd, uint32_t addr)
{
  uint8_t frame[4];

  frame[0] = cmd;
  frame[1] = (uint8_t)((addr >> 16) & 0xFFU);
  frame[2] = (uint8_t)((addr >> 8) & 0xFFU);
  frame[3] = (uint8_t)(addr & 0xFFU);

  return flash_send(frame, 4U);
}

static uint8_t flash_receive(uint8_t *dst, uint32_t len)
{
  uint8_t dummy[32];

  memset(dummy, 0xFF, sizeof(dummy));

  while (len > 0U)
  {
    uint16_t chunk = (len > sizeof(dummy)) ? (uint16_t)sizeof(dummy) : (uint16_t)len;

    if (HAL_SPI_TransmitReceive(&hspi2, dummy, dst, chunk, FLASH_SPI_TIMEOUT) != HAL_OK)
    {
      return BSP_ERROR;
    }

    dst += chunk;
    len -= chunk;
  }

  return BSP_OK;
}

static uint8_t flash_write_enable(void)
{
  uint8_t cmd = CMD_WRITE_ENABLE;
  uint8_t status = 0U;
  uint32_t start;

  flash_cs_low();
  if (flash_send(&cmd, 1U) != BSP_OK)
  {
    flash_cs_high();
    return BSP_ERROR;
  }
  flash_cs_high();

  /* The WEL bit must be set before any program/erase operation. */
  start = HAL_GetTick();
  for (;;)
  {
    if (bsp_flash_read_status(&status) != BSP_OK)
    {
      return BSP_ERROR;
    }

    if ((status & 0x02U) != 0U)
    {
      return BSP_OK;
    }

    if ((uint32_t)(HAL_GetTick() - start) >= 10U)
    {
      return BSP_ERROR;
    }
  }
}

uint8_t bsp_flash_read_status(uint8_t *status)
{
  uint8_t cmd = CMD_READ_STATUS1;

  if (status == 0)
  {
    return BSP_ERROR;
  }

  flash_cs_low();
  if ((flash_send(&cmd, 1U) != BSP_OK) || (flash_receive(status, 1U) != BSP_OK))
  {
    flash_cs_high();
    return BSP_ERROR;
  }
  flash_cs_high();

  return BSP_OK;
}

uint8_t bsp_flash_is_busy(void)
{
  uint8_t status = 0U;

  if (bsp_flash_read_status(&status) != BSP_OK)
  {
    return 1U;
  }

  return ((status & STATUS_BUSY) != 0U) ? 1U : 0U;
}

static uint8_t flash_wait_ready(uint32_t timeout_ms)
{
  uint32_t start = HAL_GetTick();

  while (bsp_flash_is_busy() != 0U)
  {
    if ((uint32_t)(HAL_GetTick() - start) >= timeout_ms)
    {
      return BSP_ERROR;
    }

    bsp_dwt_delay_us(100U);
  }

  return BSP_OK;
}

uint32_t bsp_flash_read_id(void)
{
  uint8_t cmd = CMD_JEDEC_ID;
  uint8_t id[3] = { 0U, 0U, 0U };

  flash_cs_low();
  if ((flash_send(&cmd, 1U) != BSP_OK) || (flash_receive(id, 3U) != BSP_OK))
  {
    flash_cs_high();
    return 0U;
  }
  flash_cs_high();

  return ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | (uint32_t)id[2];
}

uint8_t bsp_flash_init(void)
{
  /* Make sure the bus is idle before the first transaction. */
  HAL_GPIO_WritePin(FLSH_CS_GPIO_Port, FLSH_CS_Pin, GPIO_PIN_SET);
  bsp_dwt_delay_us(50U);

  s_jedec_id = bsp_flash_read_id();

  if (s_jedec_id == BSP_FLASH_JEDEC_W25Q128)
  {
    s_present = 1U;
  }
  else
  {
    /* W25Q128 sub-series differ in the memory-type byte (0x40 / 0x70), so
       accept the Winbond manufacturer id together with the 0x18 capacity code. */
    s_present = ((((s_jedec_id >> 16) & 0xFFU) == BSP_FLASH_JEDEC_MFR_WINBOND) &&
                 ((s_jedec_id & 0xFFU) == BSP_FLASH_CAP_128MBIT)) ? 1U : 0U;
  }

  return (s_present != 0U) ? BSP_OK : BSP_ERROR;
}

uint8_t bsp_flash_is_present(void)
{
  return s_present;
}

uint8_t bsp_flash_read(uint32_t addr, void *buf, uint32_t len)
{
  uint8_t *dst = (uint8_t *)buf;

  if ((dst == 0) || (len == 0U) || ((addr + len) > BSP_FLASH_SIZE))
  {
    return BSP_ERROR;
  }

  if (flash_wait_ready(FLASH_READY_TIMEOUT) != BSP_OK)
  {
    return BSP_ERROR;
  }

  flash_cs_low();
  if ((flash_send_cmd_addr(CMD_READ_DATA, addr) != BSP_OK) ||
      (flash_receive(dst, len) != BSP_OK))
  {
    flash_cs_high();
    return BSP_ERROR;
  }
  flash_cs_high();

  return BSP_OK;
}

uint8_t bsp_flash_write(uint32_t addr, const void *buf, uint32_t len)
{
  const uint8_t *src = (const uint8_t *)buf;
  uint32_t written = 0U;

  if ((src == 0) || (len == 0U) || ((addr + len) > BSP_FLASH_SIZE))
  {
    return BSP_ERROR;
  }

  while (written < len)
  {
    uint32_t page_left = BSP_FLASH_PAGE_SIZE - ((addr + written) % BSP_FLASH_PAGE_SIZE);
    uint32_t chunk = len - written;

    if (chunk > page_left)
    {
      chunk = page_left;
    }

    if (flash_wait_ready(FLASH_READY_TIMEOUT) != BSP_OK)
    {
      return BSP_ERROR;
    }

    if (flash_write_enable() != BSP_OK)
    {
      return BSP_ERROR;
    }

    flash_cs_low();
    if ((flash_send_cmd_addr(CMD_PAGE_PROGRAM, addr + written) != BSP_OK) ||
        (flash_send(&src[written], (uint16_t)chunk) != BSP_OK))
    {
      flash_cs_high();
      return BSP_ERROR;
    }
    flash_cs_high();

    if (flash_wait_ready(FLASH_READY_TIMEOUT) != BSP_OK)
    {
      return BSP_ERROR;
    }

    written += chunk;
  }

  return BSP_OK;
}

uint8_t bsp_flash_erase_sector(uint32_t addr)
{
  if (addr >= BSP_FLASH_SIZE)
  {
    return BSP_ERROR;
  }

  if (flash_wait_ready(FLASH_READY_TIMEOUT) != BSP_OK)
  {
    return BSP_ERROR;
  }

  if (flash_write_enable() != BSP_OK)
  {
    return BSP_ERROR;
  }

  flash_cs_low();
  if (flash_send_cmd_addr(CMD_SECTOR_ERASE, addr & ~(uint32_t)(BSP_FLASH_SECTOR_SIZE - 1U)) != BSP_OK)
  {
    flash_cs_high();
    return BSP_ERROR;
  }
  flash_cs_high();

  return flash_wait_ready(FLASH_READY_TIMEOUT);
}

uint8_t bsp_flash_erase_range(uint32_t addr, uint32_t len)
{
  uint32_t sector;

  if ((len == 0U) || ((addr + len) > BSP_FLASH_SIZE))
  {
    return BSP_ERROR;
  }

  sector = addr & ~(uint32_t)(BSP_FLASH_SECTOR_SIZE - 1U);

  while (sector < (addr + len))
  {
    if (bsp_flash_erase_sector(sector) != BSP_OK)
    {
      return BSP_ERROR;
    }

    sector += BSP_FLASH_SECTOR_SIZE;
  }

  return BSP_OK;
}
