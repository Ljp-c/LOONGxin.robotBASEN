#include "readar.h"
#include "peripherals.h"
#include "ls2k_i2c_bus.h"
#include "bsp.h"
#include "osal.h"
#include <stdio.h>

#define READAR_ADDRESS 0x57
#define READAR_WRITEREADER 0xAE
#define READAR_READREADER 0xAF

static void USE_READAR_task(void *arg)
{
    osal_mq_t q = peripherals_get_supersonic_to_redar();
    if (!q) return;

    int WRitecommmand[2]={READAR_WRITEREADER,0x01};

    while (1)
    {
        uint8_t DATA[3] = {0,0,0};
        I2C_send_start(BSP_USE_I2C1,READAR_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1,READAR_ADDRESS,0);
        I2C_write_bytes(BSP_USE_I2C1,WRitecommmand,2);
        I2C_send_stop(BSP_USE_I2C1,READAR_ADDRESS);

        I2C_send_start(BSP_USE_I2C1,READAR_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1,READAR_ADDRESS,0);
        I2C_write_bytes(BSP_USE_I2C1,READAR_READREADER,1);
      
        I2C_send_start(BSP_USE_I2C1,READAR_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1,READAR_ADDRESS,1);
        I2C_read_bytes(BSP_USE_I2C1,DATA,3);  
        I2C_send_stop(BSP_USE_I2C1,READAR_ADDRESS);
    
        if (osal_mq_send(q, DATA, sizeof(DATA)) != 0)
        {
            printk("Failed to send angle distance data\n");
        }
        delay_ms(10);
    }
}

void readar_init(void)
{
    osal_task_create("READERUSING", 4096, 0, 0, USE_READAR_task, NULL);
}
