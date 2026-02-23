/*
 * mpu6050.c - MPU6050 IMU (惯性测量单元) 传感器驱动模块
 *
 * 功能说明:
 *   本模块负责通过 I2C 接口读取 MPU6050 传感器的数据
 *   MPU6050 集成了三轴加速度计和三轴陀螺仪
 *
 * 硬件连接:
 *   - I2C 总线: BSP_USE_I2C1
 *   - 传感器地址: 0x68 (7位地址)
 *
 * 数据输出:
 *   - 加速度: accleX, accleY (X/Y 轴线性加速度)
 *   - 陀螺仪: GYROZ (Z 轴角速度)
 *   - 积分计算: 速度、位移、角度
 *
 * 积分计算:
 *   - 加速度 -> 速度: speed += acceleration * delta_time
 *   - 速度 -> 位移: distance += speed * delta_time
 *   - 角速度 -> 角度: angle += omega * delta_time
 */

#include "mpu6050.h"
#include "mpu6050REG.h"
#include "ls2k_i2c_bus.h"
#include "bsp.h"
#include "osal.h"
#include <stdio.h>

/*
 * USEMPU6050_task - MPU6050 数据读取任务
 *
 * 功能:
 *   初始化 MPU6050 并持续读取加速度和陀螺仪数据
 *   通过积分计算速度和位移
 *
 * 执行流程:
 *   1. 初始化 I2C1 总线
 *   2. 配置 MPU6050 寄存器:
 *      - SMPLRT_DIV: 采样率分频
 *      - CONFIG: 陀螺仪滤波配置
 *      - GYRO_CONFIG: 陀螺仪量程
 *      - ACCEL_CONFIG: 加速度计量程
 *      - PWR_MGMT_1: 电源管理 (退出   3.睡眠模式)
 * 循环读取传感器数据:
 *      - 读取加速度 X, Y (高/低字节)
 *      - 读取陀螺仪 Z (高/低字节)
 *      - 数据拼接 (高字节 << 8 | 低字节)
 *      - 积分计算速度和位移
 *
 * 寄存器说明 (MPU6050):
 *   - SMPLRT_DIV: 采样率 = 8kHz / (1 + SMPLRT_DIV)
 *   - CONFIG: 数字低通滤波器配置
 *   - GYRO_CONFIG: 陀螺仪自检和量程 (250, 500, 1000, 2000 deg/s)
 *   - ACCEL_CONFIG: 加速度自检和量程 (2, 4, 8, 16 g)
 *   - PWR_MGMT_1: 电源管理，0x01 表示使用 X 轴 gyroscope 作为时钟源
 */
static void USEMPU6050_task(void *arg)
{
    /* 采样间隔: 10ms */
    static int detla_time = 10;

    /*
     * 变量说明:
     *   - accleX, accleY: 加速度原始值
     *   - GYROZ: 陀螺仪原始值
     *   - speedX, speedY: 速度 (加速度积分)
     *   - omiga: 角速度积分
     *   - distanceX, distanceY: 位移 (速度积分)
     *   - AngeleZ: 角度 (角速度积分)
     */
    int accleX, accleY, GYROZ;
    int speedX = 0, speedY = 0, omiga = 0;
    int distanceX = 0, distanceY = 0, AngeleZ = 0;

    /*
     * MPU6050 寄存器配置数组
     * 写寄存器 1: 配置采样率和滤波器
     *   - 0x19: SMPLRT_DIV (采样率分频)
     *   - 0x09: 配置值 (示例值)
     *   - 0x06: CONFIG 寄存器 (滤波配置)
     *   - 0x18: GYRO_CONFIG (陀螺仪量程)
     *   - 0x18: ACCEL_CONFIG (加速度量程)
     */
    int WRITE_register_value1[5] = {
        MPU6050_SMPLRT_DIV,   /* 寄存器地址: 0x19 */
        0x09,                  /* 配置值 */
        0x06,                  /* 滤波器配置 */
        0x18,                  /* 陀螺仪量程配置 */
        0x18                   /* 加速度量程配置 */
    };

    /*
     * 写寄存器 2: 电源管理
     *   - 0x6B: PWR_MGMT_1
     *   - 0x01: 使用 X 轴 gyroscope 时钟，退出睡眠模式
     *   - 0x00: 保留
     */
    int WRITE_register_value2[5] = {
        MPU6050_PWR_MGMT_1,   /* 寄存器地址: 0x6B */
        0x01,                  /* 配置值: 唤醒 */
        0x00                   /* 保留 */
    };

    /*
     * 读寄存器: 加速度 X, Y
     *   - 0x3B: ACCEL_XOUT_H (加速度 X 高字节)
     */
    int READ_register_value1XYlinar[5] = {MPU6050_ACCEL_XOUT_H};

    /*
     * 读寄存器: 陀螺仪 Z
     *   - 0x47: GYRO_ZOUT_H (陀螺仪 Z 高字节)
     */
    int READ_register_value1ZGYRO[3] = {MPU6050_GYRO_ZOUT_H};

    /* 初始化 I2C1 总线 */
    I2C_initialize(BSP_USE_I2C1);

    /*
     * 检查 I2C 初始化是否成功
     * ~ 表示按位取反，如果初始化失败则执行后续代码
     */
    if (~I2C_initialize(BSP_USE_I2C1))
    {
        /*
         * 配置 MPU6050 寄存器 (第一组)
         */
        I2C_send_start(BSP_USE_I2C1, MPU6050_ADDRESS);      /* 发送 START，地址 */
        I2C_send_addr(BSP_USE_I2C1, MPU6050_ADDRESS, 0);    /* 发送写地址 */
        I2C_write_bytes(BSP_USE_I2C1, WRITE_register_value1, 5);  /* 写 5 字节配置 */
        I2C_send_stop(BSP_USE_I2C1, MPU6050_ADDRESS);       /* 发送 STOP */

        /*
         * 配置 MPU6050 寄存器 (第二组 - 电源管理)
         */
        I2C_send_start(BSP_USE_I2C1, MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1, MPU6050_ADDRESS, 0);
        I2C_write_bytes(BSP_USE_I2C1, WRITE_register_value2, 3);
        I2C_send_stop(BSP_USE_I2C1, MPU6050_ADDRESS);

        /*
         * 读取加速度数据 (X, Y 轴)
         */
        /* 发送读寄存器地址 */
        I2C_send_start(BSP_USE_I2C1, MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1, MPU6050_ADDRESS, 0);
        I2C_write_bytes(BSP_USE_I2C1, READ_register_value1XYlinar[0], 1);
        I2C_send_stop(BSP_USE_I2C1, MPU6050_ADDRESS);

        /* 读取 4 字节数据: ACCEL_XOUT_H, ACCEL_XOUT_L, ACCEL_YOUT_H, ACCEL_YOUT_L */
        I2C_send_start(BSP_USE_I2C1, MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1, MPU6050_ADDRESS, 1);   /* 读模式 */
        I2C_read_bytes(BSP_USE_I2C1, READ_register_value1XYlinar + 1, 4);
        I2C_send_stop(BSP_USE_I2C1, MPU6050_ADDRESS);

        /*
         * 读取陀螺仪数据 (Z 轴)
         */
        /* 发送读寄存器地址 */
        I2C_send_start(BSP_USE_I2C1, MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1, MPU6050_ADDRESS, 0);
        I2C_write_bytes(BSP_USE_I2C1, READ_register_value1ZGYRO[0], 1);
        I2C_send_stop(BSP_USE_I2C1, MPU6050_ADDRESS);

        /* 读取 2 字节数据: GYRO_ZOUT_H, GYRO_ZOUT_L */
        I2C_send_start(BSP_USE_I2C1, MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1, MPU6050_ADDRESS, 1);
        I2C_read_bytes(BSP_USE_I2C1, READ_register_value1ZGYRO + 1, 2);
        I2C_send_stop(BSP_USE_I2C1, MPU6050_ADDRESS);

        /*
         * 数据拼接 (高字节 << 8 | 低字节)
         * 传感器数据为有符号整数，高字节在前
         */
        accleX = (READ_register_value1XYlinar[1] << 8) | READ_register_value1XYlinar[2];
        accleY = (READ_register_value1XYlinar[3] << 8) | READ_register_value1XYlinar[4];
        GYROZ = (READ_register_value1ZGYRO[1] << 8) | READ_register_value1ZGYRO[2];

        /*
         * 积分计算
         * 公式: value += raw_value * delta_time
         */
        /* 加速度积分 -> 速度 */
        speedX += accleX * detla_time;
        speedY += accleY * detla_time;

        /* 角速度积分 -> 角度 */
        omiga += GYROZ * detla_time;

        /* 速度积分 -> 位移 */
        distanceX += speedX * detla_time;
        distanceY += speedY * detla_time;

        /* 角度积分 */
        AngeleZ += omiga * detla_time;
    }
}

/*
 * mpu6050_init - MPU6050 驱动初始化
 *
 * 功能:
 *   创建 MPU6050 数据读取任务
 *
 * 任务参数:
 *   - 任务名: "USEMPU6050_task"
 *   - 栈大小: 4096 字节
 *   - 优先级: 0 (最高)
 *   - 入口函数: USEMPU6050_task
 */
void mpu6050_init(void)
{
    osal_task_create("USEMPU6050_task", 4096, 0, 0, USEMPU6050_task, NULL);
}

