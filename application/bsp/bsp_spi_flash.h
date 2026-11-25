/**
  ******************************************************************************
  * @file    bsp_spi_flash.h
  * @brief   W25Q128JVSIQ (128 Mbit / 16 MByte) SPI NOR flash on SPI2.
  *          Chip select: FLASH_CS (PB12), active low.
  ******************************************************************************
  */

#ifndef __BSP_SPI_FLASH_H
#define __BSP_SPI_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

#define BSP_FLASH_SIZE          0x01000000UL   /* 16 MByte */
#define BSP_FLASH_PAGE_SIZE     256U
#define BSP_FLASH_SECTOR_SIZE   4096U

#define BSP_FLASH_JEDEC_W25Q128    0x00EF4018UL
#define BSP_FLASH_JEDEC_MFR_WINBOND 0xEFU
#define BSP_FLASH_CAP_128MBIT       0x18U   /* 16 MByte capacity code */

/* Detects the device, returns BSP_OK when a W25Q128 answered. */
uint8_t bsp_flash_init(void);
uint8_t bsp_flash_is_present(void);
uint32_t bsp_flash_read_id(void);

uint8_t bsp_flash_read(uint32_t addr, void *buf, uint32_t len);

/* Page program. The target sectors must be erased beforehand. */
uint8_t bsp_flash_write(uint32_t addr, const void *buf, uint32_t len);

uint8_t bsp_flash_erase_sector(uint32_t addr);
uint8_t bsp_flash_erase_range(uint32_t addr, uint32_t len);

uint8_t bsp_flash_read_status(uint8_t *status);
uint8_t bsp_flash_is_busy(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SPI_FLASH_H */
