/*
 * osal.h
 *
 * created: 2025-01-11
 *  author: 
 */

/******************************************************************************
 * Should use clock-tick = 1000 per second, so ms equal tick
 */

#ifndef _OSAL_H
#define _OSAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#elif defined(OS_UCOS)
#include "os.h"
#elif defined(OS_PESUDO)
#include "pesudoos.h"
#endif

//-----------------------------------------------------------------------------

extern void printk(const char *fmt, ...);
extern void delay_ms(int ms);
extern uint64_t get_clock_ticks(void);

#define LOG_ERR         printk

//-----------------------------------------------------------------------------

#define OSAL_WAIT_FOREVER       0xFFFFFFFF      /* 永久等待 */

#define OSAL_ERR_OK             0
#define OSAL_ERR_TIMEOUT        1               /* 超时返回 */
#define OSAL_ERR_INVAL          2               /* 参数错位 */

/*
 * Event MQ Mutex Semphore Common Options
 */
#define OSAL_OPT_FIFO           0x0001          /* 先进先出 */
#define OSAL_OPT_LIFO           0x0002          /* 后进先出 */
#define OSAL_OPT_PRIO           0x0004          /* 按照优先级 */
#define OSAL_OPT_ALL            0x0008          /* 分发给全部 */

/*
 * Event Receive Flag
 */
#define OSAL_EVENT_FLAG_AND     0x0001          /* 收到全部事件 */
#define OSAL_EVENT_FLAG_OR      0x0002          /* 收到任一事件 */
#define OSAL_EVENT_FLAG_CLEAR   0x0004          /* 事件接收后清除 */

//-----------------------------------------------------------------------------

typedef void*   osal_task_t;
typedef void*   osal_event_t;
typedef void*   osal_sem_t;
typedef void*   osal_mutex_t;
typedef void*   osal_mq_t;
typedef void*   osal_timer_t;

//-----------------------------------------------------------------------------
// Task
//-----------------------------------------------------------------------------

typedef void (*osal_task_entry_t)(void *arg);

osal_task_t osal_task_create(const char *name,              /* 名称 */
                             uint32_t stack_size,           /* 堆栈大小 */
                             uint32_t prio,                 /* 优先级 */
                             uint32_t slice,                /* 时间片 */
                             osal_task_entry_t entry,       /* 入口函数 */
                             void *args);                   /* 函数参数 */

void osal_task_delete(osal_task_t task);

void osal_task_suspend(osal_task_t task);
void osal_task_resume(osal_task_t task);
void osal_task_sleep(uint32_t ms);
void osal_task_sleep_until(uint32_t *prev_ticks, uint32_t inc_ticks);

//-----------------------------------------------------------------------------
// Event
//-----------------------------------------------------------------------------

osal_event_t osal_event_create(const char *name, uint32_t opt);
void osal_event_delete(osal_event_t event);

int  osal_event_send(osal_event_t event, uint32_t bits);
uint32_t osal_event_receive(osal_event_t event, uint32_t bits,
                            uint32_t flag, uint32_t timeout_ms);
void osal_event_set_bits(osal_event_t event, uint32_t bits);
void osal_event_set_os_opt(osal_event_t event, uint32_t opt);

//-----------------------------------------------------------------------------
// Semphore
//-----------------------------------------------------------------------------

osal_sem_t osal_sem_create(const char *name, uint32_t opt, uint32_t initial_count);
void osal_sem_delete(osal_sem_t sem);

int osal_sem_obtain(osal_sem_t sem, uint32_t timeout);
int osal_sem_release(osal_sem_t sem);
void osal_sem_reset(osal_sem_t sem);
void osal_sem_set_os_opt(osal_sem_t sem, uint32_t opt);

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

osal_mutex_t osal_mutex_create(const char *name, uint32_t opt);
void osal_mutex_delete(osal_mutex_t mutex);
int osal_mutex_obtain(osal_mutex_t mutex, uint32_t timeout_ms);
int osal_mutex_release(osal_mutex_t mutex);
void osal_mutex_set_os_opt(osal_mutex_t mutex, uint32_t opt);

//-----------------------------------------------------------------------------
// Message Queue
//-----------------------------------------------------------------------------

osal_mq_t osal_mq_create(const char *name, uint32_t opt,
                         uint32_t item_size, uint32_t max_msgs);
void osal_mq_delete(osal_mq_t mq);
int osal_mq_send(osal_mq_t mq, const void *msg, int size);
int osal_mq_receive(osal_mq_t mq, void *msg, int size, uint32_t timeout);
void osal_mq_set_os_opt(osal_mq_t mq, uint32_t opt);

int osal_mq_is_full(osal_mq_t mq);
int osal_mq_flush(osal_mq_t mq);

//-----------------------------------------------------------------------------
// Timer
//-----------------------------------------------------------------------------

osal_timer_t osal_timer_create(const char *name,
                               osal_task_entry_t handler,
                               void *argument,
                               uint32_t timeout_ms,
                               bool is_period);

void osal_timer_delete(osal_timer_t timer);

void osal_timer_start(osal_timer_t timer, uint32_t timeout_ms);
void osal_timer_stop(osal_timer_t timer);

//-----------------------------------------------------------------------------
// Other
//-----------------------------------------------------------------------------

int osal_is_osrunning(void);        /* return 1 == running */

size_t osal_enter_critical_section(void);
void osal_leave_critical_section(size_t flag);

void osal_msleep(uint32_t ms);

void *osal_malloc(size_t size);
void osal_free(void *ptr);

//-----------------------------------------------------------------------------
// 字符串常量
//-----------------------------------------------------------------------------

#define STR_OSAL_CREATE_TASK_FAIL   "create osal task %s fail"
#define STR_OSAL_CREATE_EVENT_FAIL  "create osal event %s fail"
#define STR_OSAL_CREATE_SEM_FAIL    "create osal semphore %s fail"
#define STR_OSAL_CREATE_MUTEX_FAIL  "create osal mutex %s fail"
#define STR_OSAL_CREATE_MQ_FAIL     "create osal message queue %s fail"
#define STR_OSAL_CREATE_TIMER_FAIL  "create osal timer %s fail"

#ifdef __cplusplus
}
#endif

#endif // _OSAL_H


