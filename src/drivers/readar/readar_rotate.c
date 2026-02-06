#include "readar_rotate.h"
#include "peripherals.h"
#include "ls2k_pwm.h"
#include "osal.h"
#include <stdio.h>

static uint8_t ANGleforEVEDIS[360*3] = {0};

static void using_READAR_FOR_ROTATE_step1_task(void *arg)
{
    osal_mq_t q_in = peripherals_get_supersonic_to_redar();
    osal_mq_t q_serial = peripherals_get_redar_to_serial();
    osal_mq_t q_algo = peripherals_get_redar_to_algorithm();
    if (!q_in || !q_serial || !q_algo) return;

    int ANGLE[360] = {0};
    for (int t = 0; t < 360; t++)
    {
        uint8_t DAta1[3] = {0};
        if (osal_mq_receive(q_in, DAta1, sizeof(DAta1), OSAL_WAIT_FOREVER) == 0) {
            float theta = (float)t;
            pwm_cfg_t pwm_cfg1;
            pwm_cfg1.mode = PWM_CONTINUE_PULSE;
            pwm_cfg1.hi_ns = 500 + 2000 * (theta/360.0f);
            pwm_cfg1.lo_ns = 19500 - 2000 * (theta/360.0f);

            ls2k_pwm_pulse_start(devPWM0, &pwm_cfg1);
            delay_ms(50);
            ANGleforEVEDIS[3*t]   = DAta1[0];
            ANGleforEVEDIS[3*t+1] = DAta1[1];
            ANGleforEVEDIS[3*t+2] = DAta1[2];
            ANGLE[t] = (DAta1[0]<<16)|(DAta1[1]<<8)|DAta1[2];
            ls2k_pwm_pulse_stop(devPWM0);
        }
    }

    if (osal_mq_send(q_serial, ANGleforEVEDIS, sizeof(ANGleforEVEDIS)) != 0) {
        printk("Failed to send angle distance data to serial\n");
    }

    if (osal_mq_send(q_algo, ANGLE, sizeof(ANGLE)) != 0) {
        printk("Failed to send angle distance data to algorithm\n");
    }
}

void readar_rotate_init(void)
{
    osal_task_create("rotationFradar", 4096, 0, 0, using_READAR_FOR_ROTATE_step1_task, NULL);
}
