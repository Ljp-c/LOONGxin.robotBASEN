/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/* main.c
 *
 * created: 2025-07-16
 *  author:DR.Liu
 */
/*
the basen can use the motor in order to achieve this using pwm and to increase usage present 
by relocate the gpio
1 motor movation 
2 IMU 
3 maybe a camera 

*/
#include <stdio.h>
#include <stdlib.h>
#include "osal.h"
#include "peripherals.h"
#include "algorithms.h"

int main(void)
{
    printf("Hello world!\r\n");
    printf("Welcome to Loongson 2K300!\r\n");

    /* 初始化外设模块（创建队列并启动外设相关任务） */
    peripherals_init();

    /* 初始化算法模块（创建算法任务，依赖 peripherals 提供的队列） */
    algorithms_init();

    /* 进入主循环 */
    for (;;)
    {
        pesudoos_run(0);
        /*
         * If you do more work here, don't use any pesudo-os functions.
         */
    }

    return 0;
}

//-----------------------------------------------------------------------------
/*
 * @@ END
 */
