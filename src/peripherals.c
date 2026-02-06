#include "peripherals.h"
#include "osal.h"
#include <stdio.h>

/* 子模块头（各子模块实现各自 init） */
#include "gpio.h"
#include "mpu6050.h"
#include "readar.h"
#include "readar_rotate.h"
#include "uart_dma.h"

/* --- 模块内部静态队列句柄 --- */
static osal_mq_t s_supersonictoredar = NULL;
static osal_mq_t s_redar_to_serial = NULL;
static osal_mq_t s_redar_to_alogriom = NULL;

/* peripherals_init 负责：创建队列 -> 调用子模块 init */
void peripherals_init(void)
{
    /* create queues (根据 original sizes) */
    s_redar_to_serial = osal_mq_create("redar_to_serial", 0, 3*360, 3);
    s_redar_to_alogriom = osal_mq_create("redar_to_alogriom", 0, 3*360, 3);
    s_supersonictoredar = osal_mq_create("supersonictoredar", 0, 24, 10);

    /* 调用各子模块 init（子模块内部会基于 peripherals_get_* 创建任务） */
    gpio_init();
    mpu6050_init();
    readar_init();
    readar_rotate_init();
    uart_dma_init();
}

/* --- getter 接口 --- */
osal_mq_t peripherals_get_supersonic_to_redar(void) { return s_supersonictoredar; }
osal_mq_t peripherals_get_redar_to_serial(void) { return s_redar_to_serial; }
osal_mq_t peripherals_get_redar_to_algorithm(void) { return s_redar_to_alogriom; }
