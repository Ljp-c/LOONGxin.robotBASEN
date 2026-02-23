/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * bsp_start_hook.c
 *
 * created: 2024-11-13
 *  author: 
 */

#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "bsp.h"
#include "ls2k_uart.h"

//-----------------------------------------------------------------------------
// 系统使用的变量
//-----------------------------------------------------------------------------

/**
 * DC 显示模式
 */
#if BSP_USE_DC
char LCD_display_mode[] = "1024x600-16@60";
#endif

/**
 * 串口控制台端口
 */
void *ConsolePort = NULL;

//-----------------------------------------------------------------------------
// 在 heap 初始化完成、控制台初始化前执行
//-----------------------------------------------------------------------------

int bsp_start_hook1(void)
{
	/**
	 * LS2K300 龙芯派使用的是 UART6;
	 *
	 * 其它板卡默认用的是 UART0;
	 *
	 */
    ConsolePort = (void *)devUART0;

    /**
     * 文件系统
     */
	#if BSP_USE_FS
	{
		extern void filesystem_initialize(void);
		extern void register_all_devices(void);

		filesystem_initialize();		/* should be initialized first */

		register_all_devices();			/* register all devices as fs */
	}
	#endif

    /*
     * do more...
     */

	return 0;
}

//-----------------------------------------------------------------------------
// 在跳转 main() 之前执行
//-----------------------------------------------------------------------------

int bsp_start_hook2(void)
{
	/******************************************************
	 * avoid linker error!!!
	 */
	#ifdef LIB_BSP
	{
		(void)fstat(-1, NULL);
		(void)isatty(-1);
		(void)realloc(NULL, 0);
	}
	#endif

    /**
     * EMMC 设备
     */
    #if BSP_USE_EMMC
    {
        extern int emmc_initialize(void);

        emmc_initialize();
    }
    #endif

    /**
     * USB 设备
     */
    #if BSP_USE_USB
    {
        extern int usbh_init(void);

        if (usbh_init() == 0)
        {
            printk("USB stack initialized successful.\r\n");
        }
        else
        {
            printk("USB stack initialized fail.\r\n");
        }
    }
    #endif

    /**
     * SHELL
     */
    #if BSP_USE_SHELL
    {
        extern void shell_task_start(const void *pUART);

        shell_task_start(NULL);
    }
    #endif

    /*
     * do more...
     */

    return 0;
}

/**
 * rt_components_board_init() 执行
 */
#ifdef OS_RTTHREAD
#include "rtdef.h"
INIT_EXPORT(bsp_start_hook2, "6");
#endif

//-----------------------------------------------------------------------------
/*
 * @@ END
 */


