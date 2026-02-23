/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_devices_init_hook.c
 *
 * created: 2024-10-30
 *  author: 
 */

/*
 * 片上设备初始化时调用, 用于实现引脚复用等的设置
 */

#include <stdint.h>
#include <stddef.h>

#include "bsp.h"

#include "ls2k_gpio.h"
#include "ls2k_can.h"
#include "ls2k_pwm.h"
#include "ls2k_uart.h"
#include "ls2k_gmac.h"

#include "ls2k_spi_bus.h"
#include "spi/norflash.h"

#include "ls2k_spiio_bus.h"

#include "ls2k_i2c_bus.h"
#include "i2c/at24c02.h"

//-----------------------------------------------------------------------------

/******************************************************************************
 * UART is always use
 */
int ls2k_uart_init_hook(const void *dev)
{
#if BSP_USE_UART0
    // if (dev == devUART0)
#endif
#if BSP_USE_UART1
    // if (dev == devUART1)
#endif
#if BSP_USE_UART2
    // if (dev == devUART2)
#endif
#if BSP_USE_UART3
    // if (dev == devUART3)
#endif
#if BSP_USE_UART4
    // if (dev == devUART4)
#endif
#if BSP_USE_UART5
    // if (dev == devUART5)
#endif
#if BSP_USE_UART6
    // if (dev == devUART6)
#endif
#if BSP_USE_UART7
    // if (dev == devUART7)
#endif
#if BSP_USE_UART8
    // if (dev == devUART8)
#endif
#if BSP_USE_UART9
    // if (dev == devUART9)
#endif

    return 0;
}

/******************************************************************************
 * SPI bus master
 */
#if BSP_USE_SPI
int ls2k_spi_init_hook(const void *bus)
{
#if BSP_USE_SPI0
    // if (bus == busSPI0)
#endif
#if BSP_USE_SPI1
    // if (bus == busSPI1)
#endif

    return 0;
}
#endif // #if BSP_USE_SPI

/******************************************************************************
 * SPIIO bus master
 */
#if BSP_USE_SPIIO
int ls2k_spiio_init_hook(const void *bus)
{
#if BSP_USE_SPI2
    // if (bus == busSPI2)
#endif
#if BSP_USE_SPI3
    // if (bus == busSPI3)
#endif

    return 0;
}
#endif // #if BSP_USE_SPIIO

/******************************************************************************
 * I2C bus master
 */
#if BSP_USE_I2C
int ls2k_i2c_init_hook(const void *bus)
{
#if BSP_USE_I2C0
    // if (bus == busI2C0)
#endif
#if BSP_USE_I2C1
    // if (bus == busI2C1)
#endif
#if BSP_USE_I2C2
    // if (bus == busI2C2)
#endif
#if BSP_USE_I2C3
    // if (bus == busI2C3)
#endif

    return 0;
}
#endif // #if BSP_USE_I2C

/******************************************************************************
 * CAN
 */
#if BSP_USE_CAN
int ls2k_can_init_hook(const void *dev)
{
#if BSP_USE_CAN0
    // if (dev == devCAN0)
#endif
#if BSP_USE_CAN1
    // if (dev == devCAN1)
#endif
#if BSP_USE_CAN2
    // if (dev == devCAN2)
#endif
#if BSP_USE_CAN3
    // if (dev == devCAN3)
#endif

    return 0;
}
#endif // #if BSP_USE_CAN

/******************************************************************************
 * PWM
 */
#if BSP_USE_PWM
int ls2k_pwm_init_hook(const void *dev)
{
#if BSP_USE_PWM0
    // if (dev == devPWM0)
#endif
#if BSP_USE_PWM1
    // if (dev == devPWM1)
#endif
#if BSP_USE_PWM2
    // if (dev == devPWM2)
#endif
#if BSP_USE_PWM3
    // if (dev == devPWM3)
#endif

    return 0;
}
#endif // #if BSP_USE_PWM

/******************************************************************************
 * Display Control
 */
#if BSP_USE_DC
int ls2k_dc_init_hook(const void *dev)
{
#if 1
    /*
     * Device 16bits RGB565:
     */
    gpio_mux(0,		3);		// pad: LCD_CLK as LCD_CLK
    gpio_mux(1,		3);		// pad: LCD_VSYNC as LCD_VSYNC
    gpio_mux(2,		3);		// pad: LCD_HSYNC as LCD_HSYNC
    gpio_mux(3,		3);		// pad: LCD_EN as LCD_EN

    gpio_enable(4, DIR_IN);
    gpio_enable(5, DIR_IN);
    gpio_enable(6, DIR_IN);

    gpio_mux(7,		3);		// pad: LCD_B3 as LCD_B3
    gpio_mux(8,		3);		// pad: LCD_B4 as LCD_B4
    gpio_mux(9,		3);		// pad: LCD_B5 as LCD_B5
    gpio_mux(10,	3);		// pad: LCD_B6 as LCD_B6
    gpio_mux(11,	3);		// pad: LCD_B7 as LCD_B7

    gpio_enable(11, DIR_IN);
    gpio_enable(12, DIR_IN);

    gpio_mux(14,	3);		// pad: LCD_G2 as LCD_G2
    gpio_mux(15,	3);		// pad: LCD_G3 as LCD_G3
    gpio_mux(16,	3);		// pad: LCD_G4 as LCD_G4
    gpio_mux(17,	3);		// pad: LCD_G5 as LCD_G5
    gpio_mux(18,	3);		// pad: LCD_G6 as LCD_G6
    gpio_mux(19,	3);		// pad: LCD_G7 as LCD_G7

    gpio_enable(20, DIR_IN);
    gpio_enable(21, DIR_IN);
    gpio_enable(22, DIR_IN);

    gpio_mux(23,	3);		// pad: LCD_R3 as LCD_R3
    gpio_mux(24,	3);		// pad: LCD_R4 as LCD_R4
    gpio_mux(25,	3);		// pad: LCD_R5 as LCD_R5
    gpio_mux(26,	3);		// pad: LCD_R6 as LCD_R6
    gpio_mux(27,	3);		// pad: LCD_R7 as LCD_R7
#endif

    return 0;
}
#endif

/******************************************************************************
 * I2S
 */
#if BSP_USE_I2S
int ls2k_i2s_init_hook(const void *dev)
{
    return 0;
}
#endif

/******************************************************************************
 * GMAC
 */
#if BSP_USE_GMAC
int ls2k_gmac_init_hook(const void *dev)
{
#if BSP_USE_GMAC0
    // extern void *devGMAC0;
    // if (dev == devGMAC0)
#endif
#if BSP_USE_GMAC1
    // extern void *devGMAC1;
    // if (dev == devGMAC1)
#endif

    return 0;
}
#endif // #if BSP_USE_GMAC

/******************************************************************************
 * SDIO     emmc & sdcard
 *
 *  dev     arg is device physical address
 *
 */
#if BSP_USE_EMMC || BSP_USE_SDCARD
int ls2k_sdio_init_hook(const void *dev)
{
#if BSP_USE_EMMC
    // if ((unsigned)(uintptr_t)dev == 0x16140000)    // SDIO0
#endif

#if BSP_USE_SDCARD
    // if ((unsigned)(uintptr_t)dev == 0x16148000)    // SDIO1
#endif

    return 0;
}
#endif // #if BSP_USE_EMMC || BSP_USE_SDCARD

//-----------------------------------------------------------------------------
/*
 * @@ END
 */


