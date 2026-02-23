/*
 * gpio.c - GPIO 硬件抽象层模块
 *
 * 功能说明:
 *   本模块负责初始化龙芯 2K300 的 GPIO 引脚
 *   配置电机控制引脚、I2C 总线引脚等功能
 *
 * GPIO 引脚分配:
 *   - GPIO 64, 65: 电机控制 (PWM 输出)
 *   - GPIO 86, 87: 备用功能
 *   - GPIO 50, 51: I2C0 总线 (主设备模式)
 *   - GPIO 44, 45: I2C1 总线 (主设备模式)
 *
 * 功能说明:
 *   gpio_enable: 使能 GPIO 引脚
 *   gpio_mux:    配置引脚复用功能
 *     - PAD_AS_MUX1: 复用功能 1
 *     - PAD_AS_MUX2: 复用功能 2
 *     - PAD_AS_MASTER: 主设备模式 (用于 I2C)
 */

#include "gpio.h"
#include "ls2k_gpio.h"
#include "osal.h"
#include <stdio.h>

/*
 * useGPIOactivate_task - GPIO 初始化任务
 *
 * 功能:
 *   初始化所有 GPIO 引脚，配置其复用功能
 *
 * 执行流程:
 *   1. 使能并配置电机控制引脚 (GPIO 64, 65, 86, 87)
 *   2. 使能并配置 I2C0 引脚 (GPIO 50, 51)
 *   3. 使能并配置 I2C1 引脚 (GPIO 44, 45)
 *
 * 引脚配置说明:
 *   - GPIO 64, 65: 复用为 MUX1 功能 (可能是 PWM 输出)
 *   - GPIO 86, 87: 复用为 MUX2 功能
 *   - GPIO 50, 51: 配置为 I2C0 主设备模式
 *   - GPIO 44, 45: 配置为 I2C1 主设备模式
 *
 * 参数:
 *   arg: 任务参数 (未使用)
 */
static void useGPIOactivate_task(void *arg)
{
    /*
     * 电机控制引脚配置
     * GPIO 64, 65: 复用为 MUX1
     * GPIO 86, 87: 复用为 MUX2
     */
    gpio_enable(64, 0);   /* 使能 GPIO 64，参数 0 表示默认配置 */
    gpio_enable(65, 0);   /* 使能 GPIO 65 */
    gpio_enable(87, 0);   /* 使能 GPIO 87 */
    gpio_enable(86, 0);   /* 使能 GPIO 86 */

    /* 配置复用功能 */
    gpio_mux(64, PAD_AS_MUX1);  /* GPIO 64 -> 复用功能 1 */
    gpio_mux(65, PAD_AS_MUX1);  /* GPIO 65 -> 复用功能 1 */
    gpio_mux(86, PAD_AS_MUX2);  /* GPIO 86 -> 复用功能 2 */
    gpio_mux(87, PAD_AS_MUX2);  /* GPIO 87 -> 复用功能 2 */

    /*
     * I2C0 总线引脚配置
     * GPIO 50, 51: 配置为主设备模式
     */
    gpio_enable(50, 0);   /* 使能 GPIO 50 */
    gpio_enable(51, 0);   /* 使能 GPIO 51 */
    gpio_mux(50, PAD_AS_MASTER);  /* GPIO 50 -> 主设备模式 */
    gpio_mux(51, PAD_AS_MASTER);  /* GPIO 51 -> 主设备模式 */

    /*
     * I2C1 总线引脚配置
     * GPIO 44, 45: 配置为主设备模式
     * 注意: GPIO 45 使用 ~0 作为参数，可能表示特殊配置
     */
    gpio_enable(44, 0);           /* 使能 GPIO 44 */
    gpio_enable(45, ~0);          /* 使能 GPIO 45，参数 ~0 可能表示反向或特殊功能 */
    gpio_mux(44, PAD_AS_MASTER);  /* GPIO 44 -> 主设备模式 */
    gpio_mux(45, PAD_AS_MASTER);  /* GPIO 45 -> 主设备模式 */
}

/*
 * gpio_init - GPIO 模块初始化
 *
 * 功能:
 *   创建 GPIO 初始化任务
 *
 * 任务参数:
 *   - 任务名: "gpioactivation"
 *   - 栈大小: 4096 字节
 *   - 优先级: 0 (最高)
 *   - 入口函数: useGPIOactivate_task
 */
void gpio_init(void)
{
    osal_task_create("gpioactivation", 4096, 0, 0, useGPIOactivate_task, NULL);
}

