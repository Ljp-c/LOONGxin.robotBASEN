/*
 * peripherals.h - 外设管理模块头文件
 *
 * 功能说明:
 *   本模块是机器人控制系统的外设管理层头文件
 *   定义了外设模块的初始化函数和消息队列访问接口
 *
 * 主要功能:
 *   1. 统一管理所有外设模块的初始化
 *   2. 创建共享消息队列，实现模块间解耦通信
 *   3. 为各子模块提供队列 getter 接口
 *
 * 使用方法:
 *   在 main.c 中调用 peripherals_init() 初始化所有外设
 *   其他模块通过 peripherals_get_*() 获取队列句柄进行数据交换
 */

#ifndef RB_SRC_PERIPHERALS_H
#define RB_SRC_PERIPHERALS_H

#include "osal.h"
#include <stdint.h>

/*
 * peripherals_init - 外设模块初始化函数
 *
 * 功能:
 *   初始化所有外设模块，包括:
 *   - GPIO 硬件抽象层
 *   - MPU6050 IMU 传感器
 *   - 超声波雷达读取
 *   - 雷达旋转 PWM 控制
 *   - 串口 DMA 通信
 *
 * 注意:
 *   此函数必须在 main() 中首先调用
 *   它会创建消息队列和启动各外设任务
 */
void peripherals_init(void);

/*
 * peripherals_get_supersonic_to_redar - 获取超声波到雷达队列句柄
 *
 * 功能:
 *   返回 "supersonic_to_redar" 消息队列的句柄
 *   该队列用于从超声波传感器接收数据，传给雷达处理模块
 *
 * 队列规格:
 *   - 消息大小: 24 字节 (3字节 x 8方向?)
 *   - 缓冲消息数: 10 条
 *
 * 返回值:
 *   osal_mq_t: 消息队列句柄，NULL 表示队列未创建
 */
osal_mq_t peripherals_get_supersonic_to_redar(void);

/*
 * peripherals_get_redar_to_serial - 获取雷达到串口队列句柄
 *
 * 功能:
 *   返回 "redar_to_serial" 消息队列的句柄
 *   该队列用于将雷达扫描数据发送到串口 DMA 模块
 *
 * 队列规格:
 *   - 消息大小: 1080 字节 (360个角度 x 3字节)
 *   - 缓冲消息数: 3 条
 *
 * 返回值:
 *   osal_mq_t: 消息队列句柄，NULL 表示队列未创建
 */
osal_mq_t peripherals_get_redar_to_serial(void);

/*
 * peripherals_get_redar_to_algorithm - 获取雷达到算法队列句柄
 *
 * 功能:
 *   返回 "redar_to_algorithm" 消息队列的句柄
 *   该队列用于将雷达扫描数据发送到算法处理模块
 *
 * 队列规格:
 *   - 消息大小: 1440 字节 (360个角度 x 4字节)
 *   - 缓冲消息数: 3 条
 *
 * 返回值:
 *   osal_mq_t: 消息队列句柄，NULL 表示队列未创建
 */
osal_mq_t peripherals_get_redar_to_algorithm(void);

#endif /* RB_SRC_PERIPHERALS_H */

