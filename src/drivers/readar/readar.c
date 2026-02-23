/*
 * readar.c - 超声波雷达传感器驱动模块
 *
 * 功能说明:
 *   本模块负责通过 I2C 接口读取超声波雷达传感器的距离数据
 *   配合舵机可以实现 360 度全方位扫描
 *
 * 硬件连接:
 *   - I2C 总线: BSP_USE_I2C1
 *   - 传感器地址: 0x57 (7位地址)
 *   - 写命令: 0xAE
 *   - 读命令: 0xAF
 *
 * 数据流程:
 *   1. 通过 I2C 向传感器发送读取命令
 *   2. 接收 3 字节的距离/角度数据
 *   3. 将数据发送到 supersonic_to_redar 消息队列
 *   4. 延迟 10ms 后重复采集
 */

#include "readar.h"
#include "peripherals.h"
#include "ls2k_i2c_bus.h"
#include "bsp.h"
#include "osal.h"
#include <stdio.h>

/*
 * 雷达传感器 I2C 地址和命令定义
 */
#define READAR_ADDRESS      0x57    /* 传感器 I2C 7位地址 */
#define READAR_WRITEREADER  0xAE    /* 写命令寄存器地址 */
#define READAR_READREADER   0xAF    /* 读命令寄存器地址 */

/*
 * USE_READAR_task - 雷达数据读取任务
 *
 * 功能:
 *   这是一个无限循环的任务，持续从超声波雷达传感器读取数据
 *
 * 执行流程:
 *   1. 获取消息队列句柄 (supersonic_to_redar)
 *   2. 循环执行:
 *      a. 通过 I2C 发送启动信号和写地址
 *      b. 发送写命令 (0xAE, 0x01) 触发传感器测量
 *      c. 发送停止信号
 *      d. 重新启动 I2C，发送读地址
 *      e. 读取 3 字节数据 (DATA[0], DATA[1], DATA[2])
 *      f. 将数据发送到消息队列
 *      g. 延迟 10ms
 *
 * I2C 通信时序:
 *   - START -> WR_ADDR(0xAE) -> DATA[0x01] -> STOP
 *   - START -> WR_ADDR(0xAF) -> RESTART -> RD_ADDR -> READ 3B -> STOP
 */
static void USE_READAR_task(void *arg)
{
    /* 获取消息队列句柄，用于发送雷达数据 */
    osal_mq_t q = peripherals_get_supersonic_to_redar();
    if (!q) return;

    /* 写命令: 0xAE 是寄存器地址, 0x01 是触发测量的命令 */
    int WRitecommmand[2] = {READAR_WRITEREADER, 0x01};

    /* 无限循环，持续采集雷达数据 */
    while (1)
    {
        /* 数据缓冲区: 3 字节，包含距离和角度信息 */
        uint8_t DATA[3] = {0, 0, 0};

        /*
         * I2C 写操作: 发送测量命令
         */
        I2C_send_start(BSP_USE_I2C1, READAR_ADDRESS);    /* 发送 START 信号 */
        I2C_send_addr(BSP_USE_I2C1, READAR_ADDRESS, 0);  /* 发送写地址 (最低位=0) */
        I2C_write_bytes(BSP_USE_I2C1, WRitecommmand, 2); /* 发送 2 字节命令 */
        I2C_send_stop(BSP_USE_I2C1, READAR_ADDRESS);     /* 发送 STOP 信号 */

        /*
         * I2C 读操作: 读取测量结果
         */
        /* 第一步: 发送读命令地址 */
        I2C_send_start(BSP_USE_I2C1, READAR_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1, READAR_ADDRESS, 0);
        I2C_write_bytes(BSP_USE_I2C1, READAR_READREADER, 1);

        /* 第二步: 重新启动并读取数据 */
        I2C_send_start(BSP_USE_I2C1, READAR_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1, READAR_ADDRESS, 1);  /* 发送读地址 (最低位=1) */
        I2C_read_bytes(BSP_USE_I2C1, DATA, 3);           /* 读取 3 字节数据 */
        I2C_send_stop(BSP_USE_I2C1, READAR_ADDRESS);

        /*
         * 将数据发送到消息队列
         * 队列名: supersonic_to_redar
         * 接收者: readar_rotate 模块
         */
        if (osal_mq_send(q, DATA, sizeof(DATA)) != 0)
        {
            printk("Failed to send angle distance data\n");
        }

        /* 采样间隔: 10ms */
        delay_ms(10);
    }
}

/*
 * readar_init - 雷达驱动初始化
 *
 * 功能:
 *   创建雷达数据读取任务
 *
 * 任务参数:
 *   - 任务名: "READERUSING"
 *   - 栈大小: 4096 字节
 *   - 优先级: 0 (最高)
 *   - 入口函数: USE_READAR_task
 */
void readar_init(void)
{
    osal_task_create("READERUSING", 4096, 0, 0, USE_READAR_task, NULL);
}

