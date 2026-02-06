#include "uart_dma.h"
#include "peripherals.h"
#include "ls2k_uart.h"
#include "ls2k_dma.h"
#include "osal.h"
#include <stdio.h>

static void using_uart_digit_task(void *arg)
{
    osal_mq_t q = peripherals_get_redar_to_serial();
    if (!q) return;
    uint8_t angeledistan[3*360] = {0};

    if (osal_mq_receive(q, angeledistan, sizeof(angeledistan), OSAL_WAIT_FOREVER) != 0) return;

    UART_initialize(BSP_USE_UART2, 115200);
    ls2k_uart_open(BSP_USE_UART2,NULL);
    ls2k_uart_ioctl(BSP_USE_UART2,UART_WORK_DMA,NULL);
    ls2k_dma_init(NULL,NULL);

    /* DMA 配置占位：请在工程中根据 DMA API/通道补全 transbytes & 回调 */
    // struct dma_chnl_cfg messageSendingAngle = { ... };
    // ls2k_dma_open(NULL,&messageSendingAngle); dma_start(&messageSendingAngle,DMA_PRIORITY_MID);
}

void uart_dma_init(void)
{
    osal_task_create("uart_digit_task", 4096, 0, 0, using_uart_digit_task, NULL);
}
