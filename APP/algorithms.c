#include "algorithms.h"
#include "kmp.h"
#include "peripherals.h"
#include "osal.h"
#include <stdio.h>
#include <string.h>

static int detla_theta1 = 0;

/* KMP-based matching task：从 queue (redar_to_alogriom) 读取 angle（长度 360）并匹配 */
static void using_READAR_FOR_ROTATE_step2_task(void *arg)
{
    int angle_for_now[360] = {0};
    int angle[360] = {0};
    osal_mq_t q = peripherals_get_redar_to_algorithm();
    if (!q) return;

    /* 等待并接收 angle 数据（360 * sizeof(int)） */
    if (osal_mq_receive(q, angle, sizeof(angle), OSAL_WAIT_FOREVER) != 0) return;

    /* 把接收到的 angle 作为模式，同时把当前扫描数据（此处简化为复制）作为文本 */
    for (int i = 0; i < 360; i++) angle_for_now[i] = angle[i];

    int tem[720];
    const int N = 360;
    const int M = 720;
    for (int i = 0; i < N; i++) tem[i] = angle_for_now[i];
    for (int i = 0; i < N; i++) tem[i+N] = angle_for_now[i];

    int lps[360];
    kmp_build_lps(angle, N, lps);
    int match_start_index = kmp_search(tem, M, angle, N, lps);

    detla_theta1 = (match_start_index != -1) ? match_start_index : -1;
}

void algorithms_init(void)
{
    osal_task_create("redar_for_rotate", 4096, 0, 0, using_READAR_FOR_ROTATE_step2_task, NULL);
}

int algorithms_get_delta_theta(void) { return detla_theta1; }
