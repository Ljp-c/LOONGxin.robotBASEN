#include "mpu6050.h"
#include "mpu6050REG.h"
#include "ls2k_i2c_bus.h"
#include "bsp.h"
#include "osal.h"
#include <stdio.h>

static void USEMPU6050_task(void *arg)
{
    static int detla_time=10;//10ms
    int accleX,accleY,GYROZ,speedX=0,speedY=0,omiga=0,distanceX=0,distanceY=0,AngeleZ=0;
    int WRITE_register_value1[5]={MPU6050_SMPLRT_DIV,0x09,0x06,0x18,0x18};
    int WRITE_register_value2[5]={MPU6050_PWR_MGMT_1,0x01,0x00};
    int READ_register_value1XYlinar[5]={MPU6050_ACCEL_XOUT_H};
    int READ_register_value1ZGYRO[3]={MPU6050_GYRO_ZOUT_H};

    I2C_initialize(BSP_USE_I2C1);  
    if (~ I2C_initialize(BSP_USE_I2C1))
    {
        I2C_send_start(BSP_USE_I2C1,MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1,MPU6050_ADDRESS,0);
        I2C_write_bytes(BSP_USE_I2C1,WRITE_register_value1,5);
        I2C_send_stop(BSP_USE_I2C1,MPU6050_ADDRESS);

        I2C_send_start(BSP_USE_I2C1,MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1,MPU6050_ADDRESS,0);
        I2C_write_bytes(BSP_USE_I2C1,WRITE_register_value2,3);
        I2C_send_stop(BSP_USE_I2C1,MPU6050_ADDRESS);

        I2C_send_start(BSP_USE_I2C1,MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1,MPU6050_ADDRESS,0);
        I2C_write_bytes(BSP_USE_I2C1,READ_register_value1XYlinar[0],1);
        I2C_send_stop(BSP_USE_I2C1,MPU6050_ADDRESS);

        I2C_send_start(BSP_USE_I2C1,MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1,MPU6050_ADDRESS,1);
        I2C_read_bytes(BSP_USE_I2C1,READ_register_value1XYlinar+1,4);
        I2C_send_stop(BSP_USE_I2C1,MPU6050_ADDRESS);

        I2C_send_start(BSP_USE_I2C1,MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1,MPU6050_ADDRESS,0);
        I2C_write_bytes(BSP_USE_I2C1,READ_register_value1ZGYRO[0],1);
        I2C_send_stop(BSP_USE_I2C1,MPU6050_ADDRESS);

        I2C_send_start(BSP_USE_I2C1,MPU6050_ADDRESS);
        I2C_send_addr(BSP_USE_I2C1,MPU6050_ADDRESS,1);
        I2C_read_bytes(BSP_USE_I2C1,READ_register_value1ZGYRO+1,2);
        I2C_send_stop(BSP_USE_I2C1,MPU6050_ADDRESS);

        accleX=(READ_register_value1XYlinar[1] << 8) | READ_register_value1XYlinar[2];
        accleY=(READ_register_value1XYlinar[3] << 8) | READ_register_value1XYlinar[4];
        GYROZ=(READ_register_value1ZGYRO[1] << 8) | READ_register_value1ZGYRO[2];

        speedX+=accleX*detla_time; speedY+=accleY*detla_time; omiga+=GYROZ*detla_time;
        distanceX+=speedX*detla_time; distanceY+=speedY*detla_time; AngeleZ+=omiga*detla_time;
    }
}

void mpu6050_init(void)
{
    osal_task_create("USEMPU6050_task", 4096, 0, 0, USEMPU6050_task, NULL);
}
