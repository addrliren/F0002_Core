/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY_C_Pin GPIO_PIN_2
#define KEY_C_GPIO_Port GPIOE
#define KEY_A_Pin GPIO_PIN_3
#define KEY_A_GPIO_Port GPIOE
#define OUT_12V_EN_Pin GPIO_PIN_6
#define OUT_12V_EN_GPIO_Port GPIOE
#define LED1_EN_Pin GPIO_PIN_15
#define LED1_EN_GPIO_Port GPIOC
#define OUT_FAN_EN_Pin GPIO_PIN_2
#define OUT_FAN_EN_GPIO_Port GPIOB
#define OUT_24V_EN_Pin GPIO_PIN_11
#define OUT_24V_EN_GPIO_Port GPIOF
#define OUT_ADJ_EN2_Pin GPIO_PIN_14
#define OUT_ADJ_EN2_GPIO_Port GPIOF
#define OUT_5V0_EN_Pin GPIO_PIN_9
#define OUT_5V0_EN_GPIO_Port GPIOE
#define FLSH_CS_Pin GPIO_PIN_12
#define FLSH_CS_GPIO_Port GPIOB
#define IN_SD_STA_Pin GPIO_PIN_9
#define IN_SD_STA_GPIO_Port GPIOD
#define EI2C_SCL_Pin GPIO_PIN_12
#define EI2C_SCL_GPIO_Port GPIOD
#define EI2C_SDA_Pin GPIO_PIN_13
#define EI2C_SDA_GPIO_Port GPIOD
#define OUT_3V3_EN_Pin GPIO_PIN_14
#define OUT_3V3_EN_GPIO_Port GPIOD
#define OUT_ADJ_EN1_Pin GPIO_PIN_4
#define OUT_ADJ_EN1_GPIO_Port GPIOG
#define SWITCH_EN_Pin GPIO_PIN_5
#define SWITCH_EN_GPIO_Port GPIOG
#define OUT_SD_CH_EN_Pin GPIO_PIN_6
#define OUT_SD_CH_EN_GPIO_Port GPIOG
#define OUT_TFT_EN_Pin GPIO_PIN_7
#define OUT_TFT_EN_GPIO_Port GPIOG
#define OUT_SDCARD_EN_Pin GPIO_PIN_8
#define OUT_SDCARD_EN_GPIO_Port GPIOG
#define TEMP_BOARD_Pin GPIO_PIN_6
#define TEMP_BOARD_GPIO_Port GPIOC
#define BUZZER1_EN_Pin GPIO_PIN_3
#define BUZZER1_EN_GPIO_Port GPIOD
#define KEY_D_Pin GPIO_PIN_0
#define KEY_D_GPIO_Port GPIOE
#define KEY_B_Pin GPIO_PIN_1
#define KEY_B_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
