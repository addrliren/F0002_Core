/**
  ******************************************************************************
  * @file    bsp_def.h
  * @brief   Common definitions shared by all BSP drivers.
  ******************************************************************************
  */

#ifndef __BSP_DEF_H
#define __BSP_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* All drivers return BSP_OK on success and BSP_ERROR on failure. */
#define BSP_OK              0U
#define BSP_ERROR           1U

#define BSP_UNUSED(x)       ((void)(x))

/* Board identification ------------------------------------------------------*/
#define BSP_MCU_NAME        "STM32F103ZET6"
#define BSP_BOARD_NAME      "Industrial Control Box"

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DEF_H */
