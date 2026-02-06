/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 */
/*
 * pesudo_task.h
 *
 * created: 2025-01-06
 *  author: 
 */

#ifndef _PESUDO_TASK_H
#define _PESUDO_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <setjmp.h>
#include <sys/queue.h>

//-------------------------------------------------------------------------------------

#define PESUDO_TASK_MAX         16          /* 未使用 */

#define PESUDO_STACK_MIN        0x1000      /* 4K */
#define PESUDO_STACK_MAGIC      0xdeadbeaf5555aaaaULL

//-------------------------------------------------------------------------------------
// PesudoOS Task
//-------------------------------------------------------------------------------------

/**
 * task function
 */
typedef void (*pesudo_task_entry_t)(void *arg);

/**
 * task state
 */
/*
 * 任务正常状态
 */
#define PT_STATE_NONE       0x00000000
#define PT_STATE_IDLE       0x00000001          /* 刚创建时空闲 */
#define PT_STATE_READY      0x00000002          /* 就绪 */
#define PT_STATE_RUNNING    0x00000004          /* 运行 */
#define PT_STATE_SUSPEND    0x00000008          /* 挂起 */

#define PT_STATE_SLEEP      0x00000010          /* 休眠 */

/*
 * 任务阻塞
 */
#define PT_BLOCKED_MUTEX    0x00000100          /* 互斥量阻塞 */
#define PT_BLOCKED_SEM      0x00000200          /* 信号灯阻塞 */
#define PT_BLOCKED_EVENT    0x00000400          /* 事件阻塞 */
#define PT_BLOCKED_MQ       0x00000800          /* 消息阻塞 */

#define IS_BLOCKED(sr)      ((sr) & 0x0F10)     /* 是否阻塞 task->state */

/*
 * 阻塞后, 超时或者获取
 */
#define PT_OBTAIN_MUTEX     0x00001000          /* 获取互斥量 */
#define PT_OBTAIN_SEM       0x00002000          /* 获取信号灯 */
#define PT_RECV_EVENT       0x00004000          /* 获取事件 */
#define PT_RECV_MQ          0x00008000          /* 获取消息 */

/*
 * 任务执行完后的操作 (Timer)
 */
#define PT_WANT_SUSPEND     0x00010000          /* 等待挂起 */
#define PT_WANT_DELETE      0x00020000          /* 等待删除 */

/**
 * task error
 */
#define PT_FATAL_STACK_OV   0x0001          /* 严重错位: 堆栈越界 */

#define PT_TIMEOUT_MUTEX    0x0100          /* 互斥量超时 */
#define PT_TIMEOUT_SEM      0x0200          /* 信号灯超时 */
#define PT_TIMEOUT_EVENT    0x0400          /* 事件超时 */
#define PT_TIMEOUT_MQ       0x0800          /* 消息超时 */

//-----------------------------------------------------------------------------

/*
 * task struct
 */
struct pesudo_task
{
    uint32_t  ID;
    char      task_name[PESUDO_NAME_MAX];       /* 名称 */
    pesudo_task_entry_t handler;                /* 任务入口 */
    void     *arg;                              /* 任务参数 */
    uint32_t  first_run_until;                  /* 初次运行 ticks */

    uint32_t  stack_size;                       /* 堆栈大小 */
    size_t   *stack_base;                       /* 堆栈基址 */
    size_t    stack_cur_top;                    /* 当前栈顶*/

    volatile uint32_t state;                    /* 状态 */
    volatile uint32_t error;                    /* 出错 */

    volatile size_t   sleep_until;              /* 睡眠终止 ticks */
    volatile size_t   wakeup_ticks;

    volatile size_t   block_when;               /* 阻塞开始时间 */
    volatile size_t   block_until;              /* 阻塞超时 ticks */

    /*
     * 任务中 mutex、sem、event、mq、sleep 阻塞的跳出位置
     */
    jmp_buf  func_exit_pos;

    /*
     * 事件专用
     */
    volatile uint32_t wait_event_bits;
    volatile uint32_t wait_event_flag;

    /*
     * 定时器专用
     */
    struct pesudo_timer *p_timer;
    pesudo_task_entry_t timer_callback;         /* Timer 回调函数, 参数 task 自身 */

#if PESUDO_OS_VER <= 2
    volatile void *block_obj;                   /* Only one object will blocked at one time */
#else
    TAILQ_ENTRY(pesudo_task) list4event;        /* 给 Event 组成链表 */
    TAILQ_ENTRY(pesudo_task) list4sem;          /* 给 Sem   组成链表 */
    TAILQ_ENTRY(pesudo_task) list4mutex;        /* 给 Mutex 组成链表 */
    TAILQ_ENTRY(pesudo_task) list4mq;           /* 给 MQ    组成链表 */
#endif

    void *user_data;                            /* 用户自定义数据 */

    /*
     * 统计数
     */
    uint64_t run_begintick;                     /* 本次运行调用时间 */
    uint32_t run_count;                         /* 调用次数 */
    uint32_t run_ticks;                         /* 使用tick总数 */

    TAILQ_ENTRY(pesudo_task) list;              /* 任务链表 */

};

//-----------------------------------------------------------------------------

/**
 * 主循环进入任务的位置
 */
extern jmp_buf  __mainloop_jmp_pos;

/**
 * 当前正在执行的任务
 */
extern struct pesudo_task *current_pesudo_task;

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


#ifdef __cplusplus
}
#endif

#endif // _PESUDO_TASK_H

