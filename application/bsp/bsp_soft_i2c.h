/**
  ******************************************************************************
  * @file    bsp_soft_i2c.h
  * @brief   Bit banged I2C on the EI2C pins (PD12 = SCL, PD13 = SDA).
  *          Both lines are open drain with external pull-ups.
  ******************************************************************************
  */

#ifndef __BSP_SOFT_I2C_H
#define __BSP_SOFT_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_def.h"

/* Half period of the bus clock, 5 us -> ~100 kHz. */
#ifndef BSP_SOFT_I2C_DELAY_US
#define BSP_SOFT_I2C_DELAY_US   5U
#endif

void bsp_soft_i2c_init(void);

void bsp_soft_i2c_start(void);
void bsp_soft_i2c_stop(void);

/* Returns BSP_OK when the slave acknowledged. */
uint8_t bsp_soft_i2c_write_byte(uint8_t byte);
/* ack != 0 -> master sends ACK, else NACK. */
uint8_t bsp_soft_i2c_read_byte(uint8_t ack);

/* Recover a stuck bus (slave holding SDA low) by clocking SCL. */
void bsp_soft_i2c_bus_recover(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SOFT_I2C_H */
