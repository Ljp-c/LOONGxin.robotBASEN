/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * main.c - RobotBaseN 工程入口文件
 *
 * created: 2025-07-16
 *  author: DR.Liu
 *
 * 工程说明:
 *   本工程是基于龙芯 2K300 处理器的嵌入式机器人控制系统。
 *   主要功能包括:
 *   1. 电机运动控制 - 使用 PWM 和 GPIO 实现电机控制
 *   2. IMU 惯性测量 - 使用 MPU6050 传感器获取姿态数据
 *   3. 雷达扫描 - 超声波雷达测距，配合舵机实现 360 度扫描
 *   4. 数据处理 - KMP 算法进行数据匹配
 *   5. 串口通信 - 通过 DMA 将雷达数据传输到上位机
 *
 * 主要模块:
 *   - peripherals: 外设管理，创建消息队列
 *   - algorithms:  算法处理，KMP 匹配
 *   - pesudoos:    轻量级伪操作系统
 */
#include <stdio.h>
#include <stdlib.h>
#include "osal.h"
#include "peripherals.h"
#include "algorithms.h"

/*
 * main - 程序入口
 *
 * 执行流程:
 *   1. 打印欢迎信息
 *   2. 初始化外设模块 (peripherals_init)
 *      - 创建消息队列
 *      - 初始化 GPIO、MPU6050、雷达、串口等外设
 *      - 各外设模块会自动创建任务
 *   3. 初始化算法模块 (algorithms_init)
 *      - 创建算法处理任务
 *   4. 进入主循环，调用 pesudoos_run() 调度任务
 */
int main(void)
{
    /* 打印欢迎信息 */
    printf("Hello world!\r\n");
    printf("Welcome to Loongson 2K300!\r\n");

    /*
     * 步骤 1: 初始化外设模块
     *   - 创建消息队列用于模块间通信
     *   - 初始化 GPIO (电机控制引脚、I2C 引脚等)
     *   - 初始化 MPU6050 (IMU 传感器)
     *   - 初始化 readar (超声波雷达 I2C 读取)
     *   - 初始化 readar_rotate (雷达旋转 PWM 控制)
     *   - 初始化 uart_dma (串口 DMA 发送)
     */
    peripherals_init();

    /*
     * 步骤 2: 初始化算法模块
     *   - 创建算法任务，从消息队列接收雷达数据
     *   - 使用 KMP 算法进行数据匹配
     *   - 计算角度偏移量 delta_theta
     */
    algorithms_init();

    /*
     * 步骤 3: 进入主循环
     *   pesudoos_run() 是伪操作系统的调度函数
     *   它会轮询检查各任务的就绪状态并执行
     */
    for (;;)
    {
        pesudoos_run(0);
        /*
         * 注意: 此处不要使用 pesudoos 提供的函数
         * 主循环应该保持简单，仅做任务调度
         */
    }

    return 0;
}

/*
 * @@ END
 */

