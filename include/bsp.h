/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * bsp.h
 *
 * created: 2024-6-16
 *  author: Bian
 */

#ifndef _BSP_H
#define _BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_PESUDO
#define BSP_USE_OS			1
#else
#define BSP_USE_OS			0
#endif

//-----------------------------------------------------------------------------
// debug
//-----------------------------------------------------------------------------

#if 0
#define DBG_OUT(...)        printk(__VA_ARGS__)
#else
#define DBG_OUT(...)
#endif

//-----------------------------------------------------------------------------
// bus frequency
//-----------------------------------------------------------------------------

extern unsigned apb_frequency;

//-----------------------------------------------------------------------------
// device used
//-----------------------------------------------------------------------------

/**
 * UART
 */
#define BSP_USE_UART0   1
#define BSP_USE_UART1   0
#define BSP_USE_UART2   0
#define BSP_USE_UART3   0
#define BSP_USE_UART4   1
#define BSP_USE_UART5   1
#define BSP_USE_UART6   1
#define BSP_USE_UART7   0
#define BSP_USE_UART8   0
#define BSP_USE_UART9   0

//---------
// Mdbus
//---------

#define	USE_MODBUS		0

/**
 * SPI
 */
#define BSP_USE_SPI0    1
#define BSP_USE_SPI1    0

#define BSP_USE_SPI     (BSP_USE_SPI0 || BSP_USE_SPI1)

#if BSP_USE_SPI0
#define NORFLASH_DRV    1
#endif

/*
 * SPI New Controller
 */
#define BSP_USE_SPI2    0
#define BSP_USE_SPI3    0

#define BSP_USE_SPIIO   (BSP_USE_SPI2 || BSP_USE_SPI3)

#if BSP_USE_SPI2
#define W25Q_DRV        1
#endif

/**
 * I2C
 */
#define BSP_USE_I2C0    1
#define BSP_USE_I2C1    0
#define BSP_USE_I2C2    0
#define BSP_USE_I2C3    1

#define BSP_USE_I2C     (BSP_USE_I2C0 || BSP_USE_I2C1 || BSP_USE_I2C2 || BSP_USE_I2C3)

#if BSP_USE_I2C0
#define ADS1015_DRV     1
#define AT24C02_DRV     1
#define RC522_DRV       1
#define MCP4725_DRV     1
#endif

/**
 * RTC
 */
#define BSP_USE_RTC     1

/**
 * HPET
 */
#define BSP_USE_HPET0   1
#define BSP_USE_HPET1   1
#define BSP_USE_HPET2   1
#define BSP_USE_HPET3   1

#define BSP_USE_HPET    (BSP_USE_HPET0 || BSP_USE_HPET1 || BSP_USE_HPET2 || BSP_USE_HPET3)

/**
 * CAN
 */
#define BSP_USE_CAN0    1
#define BSP_USE_CAN1    1
#define BSP_USE_CAN2    0
#define BSP_USE_CAN3    0

#define BSP_USE_CAN     (BSP_USE_CAN0 || BSP_USE_CAN1 || BSP_USE_CAN2 || BSP_USE_CAN3)

/**
 * GMAC
 */
#define BSP_USE_GMAC0   1
#define BSP_USE_GMAC1   0

#define BSP_USE_GMAC    (BSP_USE_GMAC0 || BSP_USE_GMAC1)

#if BSP_USE_GMAC
#define USE_LWIP        1
#endif

/**
 * PWM
 */
#define BSP_USE_PWM0    1
#define BSP_USE_PWM1    1
#define BSP_USE_PWM2    1
#define BSP_USE_PWM3    1

#define BSP_USE_PWM     (BSP_USE_PWM0  || BSP_USE_PWM1  || BSP_USE_PWM2  || BSP_USE_PWM3)

/**
 * Display Control
 */
#define BSP_USE_DC      1
#if BSP_USE_DC
#define USE_ST7701S     0
#define USE_LVGL        1
#endif

/**
 * AD Converter
 */
#define BSP_USE_ADC     1

/**
 * I2S
 */
#define BSP_USE_I2S     1

#if BSP_USE_I2S
#define ES8388_DRV      1
#endif

//-----------------------------------------------------------------------------
// filesystem
//-----------------------------------------------------------------------------

#define BSP_USE_FS      1

#if BSP_USE_FS

#define BSP_USE_USB     1

#define BSP_USE_EMMC    1

#endif // #if BSP_USE_FS

/*
 * Shell
 */
#define BSP_USE_SHELL   1

//*****************************************************************************
//-----------------------------------------------------------------------------
// This function print to console directly
//-----------------------------------------------------------------------------

extern void printk(const char *fmt, ...);

extern void print_hex(char *p, int size);

extern void delay_us(int us);
extern void delay_ms(int ms);

extern unsigned long get_clock_ticks(void);

//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif // _BSP_H

//-----------------------------------------------------------------------------
/*
 * @@ END
 */

