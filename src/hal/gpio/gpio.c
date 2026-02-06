#include "gpio.h"
#include "ls2k_gpio.h"
#include "osal.h"
#include <stdio.h>

static void useGPIOactivate_task(void *arg)
{
    gpio_enable(64 ,0);
    gpio_enable(65 ,0);
    gpio_enable(87 ,0);
    gpio_enable(86 ,0);
    
    gpio_mux(64,PAD_AS_MUX1);
    gpio_mux(65,PAD_AS_MUX1);
    gpio_mux(86,PAD_AS_MUX2);
    gpio_mux(87,PAD_AS_MUX2);

    gpio_enable( 50,0);
    gpio_enable( 51,0);
    gpio_mux(50,PAD_AS_MASTER);
    gpio_mux(51,PAD_AS_MASTER);

    gpio_enable( 44,0);
    gpio_enable( 45,~0);
    gpio_mux(44,PAD_AS_MASTER);
    gpio_mux(45,PAD_AS_MASTER);
}

void gpio_init(void)
{
    osal_task_create("gpioactivation", 4096, 0, 0, useGPIOactivate_task, NULL);
}
