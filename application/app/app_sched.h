/**
  ******************************************************************************
  * @file    app_sched.h
  * @brief   Cooperative bare metal task scheduler.
  *
  *          Tasks are plain functions registered with a period. Nothing is
  *          allocated at run time and every task must return quickly: the
  *          watchdog is fed from a scheduled task, so a task that blocks for
  *          more than BSP_WDG_TIMEOUT_MS resets the board by design.
  ******************************************************************************
  */

#ifndef __APP_SCHED_H
#define __APP_SCHED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

#define APP_SCHED_MAX_TASKS     16
#define APP_SCHED_INVALID_ID    0xFFU

typedef void (*app_task_fn_t)(void);

typedef struct
{
  const char     *name;
  app_task_fn_t   fn;
  uint32_t        period_ms;
  uint32_t        last_run_ms;
  uint32_t        run_count;
  uint32_t        overrun_count;
  uint8_t         enabled;
} app_task_t;

void app_sched_init(void);

/* Returns the task id, APP_SCHED_INVALID_ID when the table is full. */
uint8_t app_sched_add(const char *name, app_task_fn_t fn, uint32_t period_ms);

void app_sched_enable(uint8_t id, uint8_t enable);
void app_sched_stop_all(void);

/* Runs the tasks that are due. Call it in a tight main loop. */
uint8_t app_sched_run(void);

uint8_t  app_sched_get_task_count(void);
const app_task_t *app_sched_get_task(uint8_t id);

#ifdef __cplusplus
}
#endif

#endif /* __APP_SCHED_H */
