/**
  ******************************************************************************
  * @file    app_sched.c
  * @brief   Cooperative bare metal task scheduler.
  ******************************************************************************
  */

#include "app_sched.h"
#include <string.h>

static app_task_t s_tasks[APP_SCHED_MAX_TASKS];
static uint8_t s_task_count = 0U;
static uint8_t s_stopped = 0U;

void app_sched_init(void)
{
  memset(s_tasks, 0, sizeof(s_tasks));
  s_task_count = 0U;
  s_stopped = 0U;
}

uint8_t app_sched_add(const char *name, app_task_fn_t fn, uint32_t period_ms)
{
  uint8_t id;

  if (fn == 0)
  {
    return APP_SCHED_INVALID_ID;
  }

  if (s_task_count >= APP_SCHED_MAX_TASKS)
  {
    return APP_SCHED_INVALID_ID;
  }

  id = s_task_count++;

  s_tasks[id].name          = (name != 0) ? name : "task";
  s_tasks[id].fn            = fn;
  s_tasks[id].period_ms     = (period_ms == 0U) ? 1U : period_ms;
  s_tasks[id].last_run_ms   = HAL_GetTick();
  s_tasks[id].run_count     = 0U;
  s_tasks[id].overrun_count = 0U;
  s_tasks[id].enabled       = 1U;

  return id;
}

void app_sched_enable(uint8_t id, uint8_t enable)
{
  if (id >= s_task_count)
  {
    return;
  }

  s_tasks[id].enabled = (enable != 0U) ? 1U : 0U;

  if (enable != 0U)
  {
    s_tasks[id].last_run_ms = HAL_GetTick();
  }
}

void app_sched_stop_all(void)
{
  uint8_t i;

  for (i = 0U; i < s_task_count; i++)
  {
    s_tasks[i].enabled = 0U;
  }

  s_stopped = 1U;
}

uint8_t app_sched_run(void)
{
  uint32_t now;
  uint8_t i;
  uint8_t executed = 0U;

  if (s_stopped != 0U)
  {
    return 0U;
  }

  now = HAL_GetTick();

  for (i = 0U; i < s_task_count; i++)
  {
    app_task_t *task = &s_tasks[i];

    if ((task->enabled == 0U) || (task->fn == 0))
    {
      continue;
    }

    if ((uint32_t)(now - task->last_run_ms) < task->period_ms)
    {
      continue;
    }

    task->last_run_ms += task->period_ms;

    /* The loop came back too late: count it once and re-synchronise so the
       task cannot be executed several times in a row. */
    if ((uint32_t)(now - task->last_run_ms) >= task->period_ms)
    {
      task->overrun_count++;
      task->last_run_ms = now;
    }

    task->run_count++;
    executed++;

    task->fn();
  }

  return executed;
}

uint8_t app_sched_get_task_count(void)
{
  return s_task_count;
}

const app_task_t *app_sched_get_task(uint8_t id)
{
  if (id >= s_task_count)
  {
    return 0;
  }

  return &s_tasks[id];
}
