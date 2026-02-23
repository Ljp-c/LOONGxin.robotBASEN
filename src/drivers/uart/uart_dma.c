/*
 * uart_dma.c - 串口 DMA 通信模块
 *
 * 功能说明:
 *   本模块负责通过 DMA 方式将雷达扫描数据发送到上位机
 *   使用 DMA 可以大大减轻 CPU 负担，提高数据传输效率
 *
 * 硬件连接:
 *   - UART 接口: BSP_USE_UART2
 *   - 波特率: 115200
 *   - DMA 通道 4: 发送 (UART2 TX)
 *   - DMA 通道 5: 接收 (UART2 RX)
 *
 * 数据流程:
 *   1. 从 redar_to_serial 队列接收雷达数据 (1080 字节)
 *   2. 初始化 UART2 为 DMA 模式
 *   3. 初始化 DMA 控制器
 *   4. 配置 DMA 发送通道 (通道 4)
 *   5. 启动 DMA 传输，将数据发送到串口
 *   6. 同时配置 DMA 接收通道 (通道 5)，准备接收上位机命令
 */

#include "uart_dma.h"
#include "peripherals.h"
#include "ls2k_uart.h"
#include "ls2k_dma.h"
#include "osal.h"
#include <stdio.h>

/*
 * ANGleforEVEDIS - 接收缓冲区
 *
 * 大小: 1080 字节 (360 x 3)
 * 用途: 存储从上位机通过 UART 接收的命令数据
 */
static uint8_t ANGleforEVEDIS[3*360] = {0};

/*
 * using_uart_digit_task - 串口 DMA 发送任务
 *
 * 功能:
 *   接收雷达数据并通过 DMA 发送到串口
 *
 * 执行流程:
 *   1. 获取消息队列句柄 (redar_to_serial)
 *   2. 等待并接收 1080 字节的雷达数据
 *   3. 初始化 UART2:
 *      - 设置波特率 115200
 *      - 打开 UART
 *      - 配置为 DMA 模式
 *   4. 初始化 DMA 控制器
 *   5. 打开 DMA 通道 0 和 1
 *   6. 检查通道 4 是否空闲:
 *      - 如果空闲，配置发送参数
 *      - 打开通道 4 并启动 DMA 传输
 *   7. 检查通道 5 是否空闲:
 *      - 如果空闲，配置接收参数
 *      - 打开通道 5 准备接收
 *
 * DMA 配置说明:
 *   - 发送通道 (通道 4):
 *     .cb = NULL: 无回调函数
 *     .ccr32 = 0x00001093: 控制寄存器 (启用中断)
 *     .chNum = DMA_Channel_4: 通道号
 *     .device = UART2_BASE: UART2 基地址
 *     .devNum = DMA_UART2: DMA 设备号
 *     .memAddr = angeledistan: 源地址 (雷达数据)
 *     .transbytes = 1080: 传输字节数
 *
 *   - 接收通道 (通道 5):
 *     .cb = NULL: 无回调函数
 *     .ccr32 = 0x00001083: 控制寄存器
 *     .chNum = DMA_Channel_5: 通道号
 *     .device = UART2_BASE: UART2 基地址
 *     .devNum = DMA_UART2: DMA 设备号
 *     .memAddr = ANGleforEVEDIS: 目的地址 (接收缓冲区)
 *     .transbytes = 1080: 传输字节数
 */
static void using_uart_digit_task(void *arg)
{
    /* 获取消息队列句柄，用于接收雷达数据 */
    osal_mq_t q = peripherals_get_redar_to_serial();
    if (!q) return;

    /* 雷达数据缓冲区: 1080 字节 (360 个角度 x 3 字节) */
    uint8_t angeledistan[3*360] = {0};

    /* 从队列接收数据，超时等待 */
    if (osal_mq_receive(q, angeledistan, sizeof(angeledistan), OSAL_WAIT_FOREVER) != 0) return;

    /*
     * UART2 初始化
     */
    UART_initialize(BSP_USE_UART2, 115200);          /* 初始化 UART2，波特率 115200 */
    ls2k_uart_open(BSP_USE_UART2, NULL);             /* 打开 UART2 */
    ls2k_uart_ioctl(BSP_USE_UART2, UART_WORK_DMA, NULL);  /* 配置为 DMA 模式 */

    /*
     * DMA 控制器初始化
     */
    ls2k_dma_init(NULL, NULL);                       /* 初始化 DMA 控制器 */

    /* 打开 DMA 通道 0 和 1 (预留) */
    ls2k_dma_open(DMA_Channel_1, NULL);
    ls2k_dma_open(DMA_Channel_0, NULL);

    /*
     * 发送通道配置 (DMA 通道 4)
     * 检查通道是否空闲，如果空闲则配置并启动传输
     */
    if (dma_get_idle_channel(DMA_UART2, 4) == 0)
    {
        /* DMA 发送配置结构体 */
        struct dma_chnl_cfg messageSendingAngle = {
            .cb        = NULL,                    /* 无回调函数 */
            .ccr32     = 0x00001093,             /* 控制寄存器: 启用中断 */
            .chNum     = DMA_Channel_4,          /* 通道号: 4 */
            .device    = UART2_BASE,              /* 外设基地址: UART2 */
            .devNum    = DMA_UART2,               /* DMA 设备号: UART2 */
            .memAddr   = (uint32_t)angeledistan, /* 源地址: 雷达数据缓冲区 */
            .transbytes = 360*3                   /* 传输字节数: 1080 */
        };

        /* 打开 DMA 通道 4 并启动传输 */
        ls2k_dma_open(DMA_Channel_4, &messageSendingAngle);
        dma_start(DMA_Channel_4, DMA_PRIORITY_MID);  /* 中等优先级启动 */
    }

    /*
     * 接收通道配置 (DMA 通道 5)
     * 检查通道是否空闲，如果空闲则配置接收通道
     */
    if (dma_get_idle_channel(DMA_UART2, 5) == 0)
    {
        /* DMA 接收配置结构体 */
        struct dma_chnl_cfg messageRECiveingmessages = {
            .cb        = NULL,                        /* 无回调函数 */
            .ccr32     = 0x00001083,                 /* 控制寄存器 */
            .chNum     = DMA_Channel_5,              /* 通道号: 5 */
            .device    = UART2_BASE,                  /* 外设基地址: UART2 */
            .devNum    = DMA_UART2,                   /* DMA 设备号: UART2 */
            .memAddr   = (uint32_t)ANGleforEVEDIS,   /* 目的地址: 接收缓冲区 */
            .transbytes = 360*3                       /* 传输字节数: 1080 */
        };

        /* 打开 DMA 通道 5，准备接收 */
        ls2k_dma_open(DMA_Channel_5, &messageRECiveingmessages);
    }
}

/*
 * uart_dma_init - 串口 DMA 模块初始化
 *
 * 功能:
 *   创建串口 DMA 发送任务
 *
 * 任务参数:
 *   - 任务名: "uart_digit_task"
 *   - 栈大小: 4096 字节
 *   - 优先级: 0 (最高)
 *   - 入口函数: using_uart_digit_task
 */
void uart_dma_init(void)
{
    osal_task_create("uart_digit_task", 4096, 0, 0, using_uart_digit_task, NULL);
}

