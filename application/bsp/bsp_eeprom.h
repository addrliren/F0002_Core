/**
  ******************************************************************************
  * @file    bsp_eeprom.h
  * @brief   BL24C02F-PARC 2 Kbit (256 x 8) I2C EEPROM on the EI2C bus.
  *          Slave address 1010 000X -> 0xA0 write / 0xA1 read.
  *          Page size 8 bytes, write cycle 5 ms.
  ******************************************************************************
  */

#ifndef __BSP_EEPROM_H
#define __BSP_EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

#define BSP_EEPROM_ADDR         0xA0U
#define BSP_EEPROM_SIZE         256U
#define BSP_EEPROM_PAGE_SIZE    8U
#define BSP_EEPROM_WRITE_MS     6U

/* Read back every written page to validate the content. */
#ifndef BSP_EEPROM_VERIFY
#define BSP_EEPROM_VERIFY       1
#endif

uint8_t bsp_eeprom_init(void);
uint8_t bsp_eeprom_is_present(void);

uint8_t bsp_eeprom_read(uint16_t addr, void *buf, uint16_t len);
uint8_t bsp_eeprom_write(uint16_t addr, const void *buf, uint16_t len);

uint8_t bsp_eeprom_read_byte(uint16_t addr, uint8_t *value);
uint8_t bsp_eeprom_write_byte(uint16_t addr, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_EEPROM_H */
