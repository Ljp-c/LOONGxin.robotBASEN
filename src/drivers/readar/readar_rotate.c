/*
 * readar_rotate.c - 雷达旋转控制模块
 *
 * 功能说明:
 *   本模块负责控制雷达舵机进行 360 度旋转扫描
 *   同时接收来自 readar 模块的测距数据，并进行数据格式化
 *
 * 硬件连接:
 *   - PWM 输出: devPWM0 (用于控制舵机角度)
 *   - 队列输入: supersonic_to_redar (接收雷达数据)
 *   - 队列输出: redar_to_serial (发送到串口)
 *   - 队列输出: redar_to_algorithm (发送到算法)
 *
 * 数据流程:
 *   1. 从 supersonic_to_redar 队列接收 3 字节测距数据
 *   2. 根据当前角度控制 PWM 输出，驱动舵机转动
 *   3. 等待 50ms 使舵机稳定
 *   4. 存储数据到 ANGleforEVEDIS 缓冲区
 *   5. 将完整的一圈数据 (360 个角度) 发送到输出队列
 */

#include "readar_rotate.h"
#include "peripherals.h"
#include "ls2k_pwm.h"
#include "osal.h"
#include <stdio.h>

/*
 * ANGleforEVEDIS - 雷达角度距离数据缓冲区
 *
 * 大小: 360 x 3 = 1080 字节
 * 存储格式: [字节0][字节1][字节2] x 360 个角度
 * 用途: 保存一圈扫描的完整数据，发送到串口和算法模块
 */
static uint8_t ANGleforEVEDIS[360*3] = {0};

/*
 * using_READAR_FOR_ROTATE_step1_task - 雷达旋转扫描任务
 *
 * 功能:
 *   控制雷达舵机旋转，同时采集各角度的距离数据
 *
 * 执行流程:
 *   1. 获取三个消息队列的句柄
 *   2. 循环 360 次 (每度采集一次):
 *      a. 从输入队列接收 3 字节测距数据
 *      b. 配置 PWM 参数，使能对应角度
 *      c. 等待 50ms 让舵机稳定
 *      d. 将数据存入缓冲区
 *      e. 停止 PWM
 *   3. 将完整数据发送到串口队列
 *   4. 将完整数据发送到算法队列
 *
 * PWM 控制说明:
 *   - mode: PWM_CONTINUE_PULSE (连续脉冲模式)
 *   - hi_ns: 500 + 2000 * (theta/360) 纳秒 (高电平时间)
 *   - lo_ns: 19500 - 2000 * (theta/360) 纳秒 (低电平时间)
 *   - 这是一个简化的 PWM 控制，角度范围 0-360 度
 */
static void using_READAR_FOR_ROTATE_step1_task(void *arg)
{
    /* 获取三个消息队列的句柄 */
    osal_mq_t q_in = peripherals_get_supersonic_to_redar();      /* 输入队列 */
    osal_mq_t q_serial = peripherals_get_redar_to_serial();       /* 输出到串口 */
    osal_mq_t q_algo = peripherals_get_redar_to_algorithm();      /* 输出到算法 */
    if (!q_in || !q_serial || !q_algo) return;

    /*
     * ANGLE 数组: 用于算法处理的 32 位整数格式
     * 每个角度对应一个 32 位整数 (4 字节)
     * 大小: 360 x 4 = 1440 字节
     */
    int ANGLE[360] = {0};

    /* 循环扫描 360 度 */
    for (int t = 0; t < 360; t++)
    {
        /* 接收 3 字节测距数据 */
        uint8_t DAta1[3] = {0};
        if (osal_mq_receive(q_in, DAta1, sizeof(DAta1), OSAL_WAIT_FOREVER) == 0)
        {
            /* 计算当前角度的 PWM 参数 */
            float theta = (float)t;
            pwm_cfg_t pwm_cfg1;

            /* 配置 PWM: 连续脉冲模式 */
            pwm_cfg1.mode = PWM_CONTINUE_PULSE;

            /*
             * PWM 占空比计算:
             * 周期 = 20ms (20000ns)
             * 高电平范围: 500ns ~ 2500ns (对应 0.5ms ~ 2.5ms)
             * 低电平范围: 19500ns ~ 17500ns
             */
            pwm_cfg1.hi_ns = 500 + 2000 * (theta/360.0f);   /* 高电平时间 */
            pwm_cfg1.lo_ns = 19500 - 2000 * (theta/360.0f);  /* 低电平时间 */

            /* 启动 PWM，控制舵机转到指定角度 */
            ls2k_pwm_pulse_start(devPWM0, &pwm_cfg1);

            /* 等待舵机稳定 */
            delay_ms(50);

            /*
             * 存储数据到缓冲区 (两种格式)
             */
            /* 格式 1: 原始 3 字节 (用于串口输出) */
            ANGleforEVEDIS[3*t]   = DAta1[0];
            ANGleforEVEDIS[3*t+1] = DAta1[1];
            ANGleforEVEDIS[3*t+2] = DAta1[2];

            /* 格式 2: 32 位整数 (用于算法处理) */
            /* 高字节在前: DATA[0] << 16 | DATA[1] << 8 | DATA[2] */
            ANGLE[t] = (DAta1[0] << 16) | (DAta1[1] << 8) | DAta1[2];

            /* 停止 PWM */
            ls2k_pwm_pulse_stop(devPWM0);
        }
    }

    /*
     * 一圈扫描完成，发送数据到两个输出队列
     */
    /* 发送 1080 字节到串口队列 */
    if (osal_mq_send(q_serial, ANGleforEVEDIS, sizeof(ANGleforEVEDIS)) != 0)
    {
        printk("Failed to send angle distance data to serial\n");
    }

    /* 发送 1440 字节到算法队列 */
    if (osal_mq_send(q_algo, ANGLE, sizeof(ANGLE)) != 0)
    {
        printk("Failed to send angle distance data to algorithm\n");
    }
}

/*
 * readar_rotate_init - 雷达旋转控制初始化
 *
 * 功能:
 *   创建雷达旋转扫描任务
 *
 * 任务参数:
 *   - 任务名: "rotationFradar"
 *   - 栈大小: 4096 字节
 *   - 优先级: 0 (最高)
 *   - 入口函数: using_READAR_FOR_ROTATE_step1_task
 */
void readar_rotate_init(void)
{
    osal_task_create("rotationFradar", 4096, 0, 0, using_READAR_FOR_ROTATE_step1_task, NULL);
}

