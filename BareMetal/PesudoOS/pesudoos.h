/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 */
/*
 * pesudoos.h
 *
 * created: 2025-01-06
 *  author: 
 */

#ifndef _PESUDOOS_H
#define _PESUDOOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * version note:
 *
 *  0.1: a task is blocked with one object, signal from isr process immediately.
 *  0.2: a task is blocked with one object, signal from isr process later.
 *  0.3: blocked task is as list of object, signal from isr process later.
 *
 */
#define PESUDO_OS_MAJOR     0
#define PESUDO_OS_MINOR     2

#define PESUDO_OS_VER       (PESUDO_OS_MAJOR * 100 + PESUDO_OS_MINOR)

//-----------------------------------------------------------------------------
// extern function
//-----------------------------------------------------------------------------

extern void printk(const char *fmt, ...);
extern void delay_ms(int ms);
extern uint64_t get_clock_ticks(void);

//-----------------------------------------------------------------------------
// PesudoOS Defination
//-----------------------------------------------------------------------------

#define PESUDO_NAME_MAX         16          /* Object 命名的字符数 */

#define PESUDO_TIMER_MS_MIN     10          /* 定时器最小定时时间 */

/*
 * Event MQ Mutex Semphore Common Options
 */
#define PESUDO_OPT_NONE         0x0000
#define PESUDO_OPT_FIFO         0x0001      /* FIFO  给1个阻塞对象赋值 */
#define PESUDO_OPT_LIFO         0x0002      /* LIFO  给1个阻塞对象赋值 */
#define PESUDO_OPT_MASK         0x00FF

#define PESUDO_OPT_DEFAULT      PESUDO_OPT_FIFO     /* 默认 */

/*
 * Event Receive Flag
 */
#define EVENT_FLAG_AND          0x0001      /* 多个事件全部符合 */
#define EVENT_FLAG_OR           0x0002      /* 多个事件符合一个就返回 */
#define EVENT_FLAG_CLEAR        0x0004      /* 获取事件后清除 */
#define EVENT_FLAG_MASK         0x00FF

//-----------------------------------------------------------------------------

typedef void (*pesudo_task_entry_t)(void *arg);

struct pesudo_task;
struct pesudo_event;
struct pesudo_mq;
struct pesudo_mutex;
struct pesudo_sem;
struct pesudo_timer;

//-----------------------------------------------------------------------------
// Event Functions
//-----------------------------------------------------------------------------

struct pesudo_event *pesudo_event_create(const char *name, uint32_t opt);
void pesudo_event_delete(struct pesudo_event *event);

int  pesudo_event_send(struct pesudo_event *event, uint32_t bits);
uint32_t pesudo_event_receive(struct pesudo_event *event, uint32_t bits,
                              uint32_t flag, uint32_t timeout_ms);
void pesudo_event_set(struct pesudo_event *event, uint32_t bits);

//-----------------------------------------------------------------------------
// MQ Functions
//-----------------------------------------------------------------------------

struct pesudo_mq *pesudo_mq_create(const char *name, uint32_t opt,
                                   uint32_t item_size, uint32_t max_msgs);
void pesudo_mq_delete(struct pesudo_mq *mq);

int pesudo_mq_send(struct pesudo_mq *mq, const void *msg);
int pesudo_mq_receive(struct pesudo_mq *mq, void *msg, uint32_t timeout_ms);

int pesudo_mq_is_full(struct pesudo_mq *mq);
int pesudo_mq_flush(struct pesudo_mq *mq);

//-----------------------------------------------------------------------------
// Mutex Functions
//-----------------------------------------------------------------------------

struct pesudo_mutex *pesudo_mutex_create(const char *name, uint32_t opt);
void pesudo_mutex_delete(struct pesudo_mutex *mutex);

int pesudo_mutex_obtain(struct pesudo_mutex *mutex, uint32_t timeout_ms);
int pesudo_mutex_release(struct pesudo_mutex *mutex);

//-----------------------------------------------------------------------------
// Semphore Functions
//-----------------------------------------------------------------------------

struct pesudo_sem *pesudo_sem_create(const char *name, uint32_t opt,
                                     uint32_t initial_count);
void pesudo_sem_delete(struct pesudo_sem *sem);

int pesudo_sem_obtain(struct pesudo_sem *sem, uint32_t timeout_ms);
int pesudo_sem_release(struct pesudo_sem *sem);
void pesudo_sem_clear(struct pesudo_sem *sem);

//-----------------------------------------------------------------------------
// Timer Functions
//-----------------------------------------------------------------------------

struct pesudo_timer *pesudo_timer_create(const char *name,
                                         pesudo_task_entry_t entry,
                                         void *argument,
                                         uint32_t timeout_ms,
                                         bool is_period);
void pesudo_timer_delete(struct pesudo_timer *tmr);

int pesudo_timer_start(struct pesudo_timer *tmr, uint32_t timeout_ms);
int pesudo_timer_stop(struct pesudo_timer *tmr);

//-----------------------------------------------------------------------------
// Task Functions
//-----------------------------------------------------------------------------

struct pesudo_task *pesudo_task_create(const char *name,            /* 名称 */
                                       uint32_t stack_size,         /* 堆栈大小 */
                                       uint32_t run_after_ms,       /* 延迟触发初次运行 */
                                       pesudo_task_entry_t entry,   /* 入口函数 */
                                       void *args);                 /* 函数参数 */

void pesudo_task_delete(struct pesudo_task *task);
void pesudo_task_suspend(struct pesudo_task *task);
void pesudo_task_resume(struct pesudo_task *task);
void pesudo_task_sleep(uint32_t ms);
void pesudo_task_sleep_until(uint32_t *prev_ticks, uint32_t inc_ticks);

//-----------------------------------------------------------------------------
// PesudoOS
//-----------------------------------------------------------------------------

/*
 * Current Running Task
 */
extern struct pesudo_task *current_pesudo_task;

struct pesudo_task *pesudoos_task_list_first(void);
struct pesudo_task *pesudoos_task_list_next(struct pesudo_task *task);

/*
 * Main Loop. Run all pesudoos component in here.
 *
 * if deadloop == 0 then run once, this must be calling in user loop.
 */
void pesudoos_run(int deadloop);

int pesudoos_is_running(void);

//-----------------------------------------------------------------------------
// Debug
//-----------------------------------------------------------------------------

#define PESUDOOS_DBG_EVENT  0x01
#define PESUDOOS_DBG_MQ     0x02
#define PESUDOOS_DBG_MUTEX  0x04
#define PESUDOOS_DBG_SEM    0x08
#define PESUDOOS_DBG_TASK   0x10
#define PESUDOOS_DBG_TIMER  0x20

void pesudoos_set_dbg_mask(unsigned int mask);
void pesudoos_dbg(unsigned int mask, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // _PESUDOOS_H


