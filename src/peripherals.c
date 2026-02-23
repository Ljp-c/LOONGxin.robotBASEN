/*
 * peripherals.c - 外设管理模块
 *
 * 功能说明:
 *   本模块是机器人控制系统的外设管理层，负责:
 *   1. 集中管理所有外设模块的初始化
 *   2. 创建消息队列，实现模块间数据传递
 *   3. 为各子模块提供队列访问接口
 *
 * 模块架构:
 *   peripherals 作为外设管理中枢，调用各子模块的 init 函数
 *   各子模块内部会创建独立的任务，实现具体功能
 *
 * 消息队列说明:
 *   supersonic_to_redar:  超声波传感器 -> 雷达模块 (3字节 x 10条)
 *   redar_to_serial:      雷达 -> 串口输出 (1080字节 = 360 x 3)
 *   redar_to_algorithm:   雷达 -> 算法处理 (1440字节 = 360 x 4)
 */

#include "peripherals.h"
#include "osal.h"
#include <stdio.h>

/*
 * 包含各子模块的头文件
 * 子模块实现各自的初始化和任务创建
 */
#include "gpio.h"           /* GPIO 硬件抽象层 */
#include "mpu6050.h"        /* IMU 传感器驱动 */
#include "readar.h"         /* 超声波雷达读取 */
#include "readar_rotate.h"  /* 雷达旋转控制 */
#include "uart_dma.h"        /* 串口 DMA 通信 */

/*
 * 模块内部静态队列句柄
 * 这些队列用于各模块之间的数据传递
 */
static osal_mq_t s_supersonictoredar = NULL;   /* 超声波到雷达队列 */
static osal_mq_t s_redar_to_serial = NULL;     /* 雷达到串口队列 */
static osal_mq_t s_redar_to_alogriom = NULL;   /* 雷达到算法队列 */

/*
 * peripherals_init - 外设模块初始化入口
 *
 * 执行流程:
 *   1. 创建消息队列
 *      - redar_to_serial: 雷达数据发送到串口 (大小: 3*360 字节, 缓冲: 3 条)
 *      - redar_to_alogriom: 雷达数据发送到算法 (大小: 3*360 字节, 缓冲: 3 条)
 *      - supersonictoredar: 超声波数据传送到雷达 (大小: 24 字节, 缓冲: 10 条)
 *
 *   2. 调用各子模块的初始化函数
 *      - gpio_init(): 初始化 GPIO 引脚
 *      - mpu6050_init(): 初始化 IMU 传感器
 *      - readar_init(): 初始化雷达读取任务
 *      - readar_rotate_init(): 初始化雷达旋转控制任务
 *      - uart_dma_init(): 初始化串口 DMA 发送任务
 *
 * 注意: 子模块的 init 函数内部会创建任务，无需外部干预
 */
void peripherals_init(void)
{
    /*
     * 创建消息队列
     * 参数说明:
     *   队列名称, 消息大小(0表示可变), 队列总容量, 消息条数
     */
    /* 雷达到串口的队列: 1080 字节 (360个角度 x 3字节), 缓冲 3 条 */
    s_redar_to_serial = osal_mq_create("redar_to_serial", 0, 3*360, 3);

    /* 雷达到算法的队列: 1080 字节 (360个角度 x 3字节), 缓冲 3 条 */
    s_redar_to_alogriom = osal_mq_create("redar_to_alogriom", 0, 3*360, 3);

    /* 超声波到雷达的队列: 24 字节 (3字节 x 8个方向?), 缓冲 10 条 */
    s_supersonictoredar = osal_mq_create("supersonictoredar", 0, 24, 10);

    /*
     * 调用各子模块初始化
     * 各子模块内部会基于 peripherals_get_* 获取队列句柄
     * 并创建相应的任务来处理数据
     */
    gpio_init();           /* GPIO 初始化 */
    mpu6050_init();        /* MPU6050 IMU 初始化 */
    readar_init();         /* 雷达读取任务初始化 */
    readar_rotate_init();  /* 雷达旋转控制初始化 */
    uart_dma_init();       /* 串口 DMA 初始化 */
}

/*
 * peripherals_get_supersonic_to_redar - 获取超声波到雷达队列句柄
 * 返回值: 消息队列句柄，供其他模块发送/接收数据
 */
osal_mq_t peripherals_get_supersonic_to_redar(void) { return s_supersonictoredar; }

/*
 * peripherals_get_redar_to_serial - 获取雷达到串口队列句柄
 * 返回值: 消息队列句柄，供其他模块发送/接收数据
 */
osal_mq_t peripherals_get_redar_to_serial(void) { return s_redar_to_serial; }

/*
 * peripherals_get_redar_to_algorithm - 获取雷达到算法队列句柄
 * 返回值: 消息队列句柄，供其他模块发送/接收数据
 */
osal_mq_t peripherals_get_redar_to_algorithm(void) { return s_redar_to_alogriom; }

