#ifndef RB_SRC_PERIPHERALS_H
#define RB_SRC_PERIPHERALS_H

#include "osal.h"
#include <stdint.h>

/*
 * src/peripherals module
 * 负责：创建共享消息队列（雷达、串口等），为子模块提供队列 getter
 */

/* 初始化外设汇总（创建队列并调用各子模块 init） */
void peripherals_init(void);

/* 队列访问接口（返回 osal_mq_t） */
osal_mq_t peripherals_get_supersonic_to_redar(void);
osal_mq_t peripherals_get_redar_to_serial(void);
osal_mq_t peripherals_get_redar_to_algorithm(void);

#endif // RB_SRC_PERIPHERALS_H
