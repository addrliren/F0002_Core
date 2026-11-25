/**
  ******************************************************************************
  * @file    app_main.h
  * @brief   Application entry points called from main().
  ******************************************************************************
  */

#ifndef __APP_MAIN_H
#define __APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

/* Initialises the BSP, the services and the cooperative scheduler. */
void app_init(void);

/* Runs one scheduler pass and services the watchdog. Call in an endless loop. */
void app_loop(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_MAIN_H */
