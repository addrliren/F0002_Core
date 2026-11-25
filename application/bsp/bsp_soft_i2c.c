/**
  ******************************************************************************
  * @file    bsp_soft_i2c.c
  * @brief   Bit banged I2C master on the EI2C pins.
  ******************************************************************************
  */

#include "bsp_soft_i2c.h"
#include "bsp_dwt.h"

#define SCL_H()     HAL_GPIO_WritePin(EI2C_SCL_GPIO_Port, EI2C_SCL_Pin, GPIO_PIN_SET)
#define SCL_L()     HAL_GPIO_WritePin(EI2C_SCL_GPIO_Port, EI2C_SCL_Pin, GPIO_PIN_RESET)
#define SDA_H()     HAL_GPIO_WritePin(EI2C_SDA_GPIO_Port, EI2C_SDA_Pin, GPIO_PIN_SET)
#define SDA_L()     HAL_GPIO_WritePin(EI2C_SDA_GPIO_Port, EI2C_SDA_Pin, GPIO_PIN_RESET)
#define SDA_READ()  HAL_GPIO_ReadPin(EI2C_SDA_GPIO_Port, EI2C_SDA_Pin)

#define I2C_DELAY() bsp_dwt_delay_us(BSP_SOFT_I2C_DELAY_US)

void bsp_soft_i2c_init(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();

  gpio.Pin   = EI2C_SCL_Pin | EI2C_SDA_Pin;
  gpio.Mode  = GPIO_MODE_OUTPUT_OD;
  gpio.Pull  = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &gpio);

  SCL_H();
  SDA_H();
  bsp_dwt_delay_us(100U);
}

void bsp_soft_i2c_start(void)
{
  SDA_H();
  SCL_H();
  I2C_DELAY();
  SDA_L();
  I2C_DELAY();
  SCL_L();
  I2C_DELAY();
}

void bsp_soft_i2c_stop(void)
{
  SCL_L();
  SDA_L();
  I2C_DELAY();
  SCL_H();
  I2C_DELAY();
  SDA_H();
  I2C_DELAY();
}

uint8_t bsp_soft_i2c_write_byte(uint8_t byte)
{
  uint8_t i;
  uint8_t nack;

  for (i = 0U; i < 8U; i++)
  {
    if ((byte & 0x80U) != 0U)
    {
      SDA_H();
    }
    else
    {
      SDA_L();
    }

    byte <<= 1;
    I2C_DELAY();
    SCL_H();
    I2C_DELAY();
    SCL_L();
    I2C_DELAY();
  }

  /* Acknowledge phase: release SDA and sample it. */
  SDA_H();
  I2C_DELAY();
  SCL_H();
  I2C_DELAY();
  nack = (SDA_READ() == GPIO_PIN_SET) ? 1U : 0U;
  SCL_L();
  I2C_DELAY();

  return (nack != 0U) ? BSP_ERROR : BSP_OK;
}

uint8_t bsp_soft_i2c_read_byte(uint8_t ack)
{
  uint8_t i;
  uint8_t value = 0U;

  SDA_H();

  for (i = 0U; i < 8U; i++)
  {
    value <<= 1;
    I2C_DELAY();
    SCL_H();
    I2C_DELAY();

    if (SDA_READ() != GPIO_PIN_RESET)
    {
      value |= 0x01U;
    }

    SCL_L();
    I2C_DELAY();
  }

  if (ack != 0U)
  {
    SDA_L();
  }
  else
  {
    SDA_H();
  }

  I2C_DELAY();
  SCL_H();
  I2C_DELAY();
  SCL_L();
  SDA_H();
  I2C_DELAY();

  return value;
}

void bsp_soft_i2c_bus_recover(void)
{
  uint8_t i;

  SDA_H();

  for (i = 0U; i < 9U; i++)
  {
    SCL_L();
    I2C_DELAY();
    SCL_H();
    I2C_DELAY();

    if (SDA_READ() != GPIO_PIN_RESET)
    {
      break;
    }
  }

  bsp_soft_i2c_stop();
}
