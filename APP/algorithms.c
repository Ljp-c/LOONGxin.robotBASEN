/*
 * algorithms.c - 算法处理模块
 *
 * 功能说明:
 *   本模块负责对雷达扫描数据进行处理
 *   使用 KMP 算法进行数据匹配，计算角度偏移量
 *
 * 数据流程:
 *   1. 从 redar_to_algorithm 队列接收 360 个角度的雷达数据
 *   2. 使用 KMP 算法进行模式匹配
 *   3. 计算匹配位置，得到角度偏移量 delta_theta
 *   4. 提供接口供其他模块获取偏移量
 *
 * KMP 算法应用:
 *   - 模式: 当前扫描的角度数据 (360 个元素)
 *   - 文本: 模式数据的双倍拼接 (720 个元素)
 *   - 目的: 在连续扫描数据中找到匹配位置，确定相对角度偏移
 */

#include "algorithms.h"
#include "kmp.h"
#include "peripherals.h"
#include "osal.h"
#include <stdio.h>
#include <string.h>

/*
 * detla_theta1 - 角度偏移量
 *
 * 存储 KMP 匹配计算得到的结果
 * 正数表示匹配到的起始位置，-1 表示未匹配
 */
static int detla_theta1 = 0;

/*
 * using_READAR_FOR_ROTATE_step2_task - 雷达数据算法处理任务
 *
 * 功能:
 *   接收雷达扫描数据，使用 KMP 算法进行匹配
 *
 * 执行流程:
 *   1. 获取消息队列句柄 (redar_to_algorithm)
 *   2. 接收 360 个角度的雷达数据 (每个角度 4 字节，合计 1440 字节)
 *   3. 复制数据作为当前扫描结果
 *   4. 构建双倍文本 (用于循环匹配):
 *      - tem[0~359] = angle_for_now[0~359]
 *      - tem[360~719] = angle_for_now[0~359]
 *   5. 构建 LPS 数组 (KMP 前缀表)
 *   6. 执行 KMP 搜索
 *   7. 保存匹配结果到 detla_theta1
 *
 * KMP 匹配原理:
 *   将雷达数据复制一份接在后面，形成 720 个元素
 *   这样可以处理角度的循环情况 (例如 359 度后面是 0 度)
 *   匹配成功后的索引就是角度偏移量
 */
static void using_READAR_FOR_ROTATE_step2_task(void *arg)
{
    /* 当前扫描的角度数据 */
    int angle_for_now[360] = {0};

    /* 接收缓冲区 */
    int angle[360] = {0};

    /* 获取消息队列句柄 */
    osal_mq_t q = peripherals_get_redar_to_algorithm();
    if (!q) return;

    /*
     * 从队列接收雷达数据
     * 数据格式: 360 个 32 位整数，合计 1440 字节
     * 每个整数代表一个角度的测量值
     */
    if (osal_mq_receive(q, angle, sizeof(angle), OSAL_WAIT_FOREVER) != 0) return;

    /*
     * 复制数据作为当前扫描结果
     * angle_for_now 用于后续匹配
     */
    for (int i = 0; i < 360; i++)
    {
        angle_for_now[i] = angle[i];
    }

    /*
     * 构建双倍文本用于循环匹配
     * tem 数组大小: 720 (360 * 2)
     * 这样可以处理角度 359 -> 0 的循环情况
     */
    int tem[720];
    const int N = 360;  /* 模式/文本长度 */
    const int M = 720;  /* 双倍文本长度 */

    /* 复制第一份 */
    for (int i = 0; i < N; i++)
    {
        tem[i] = angle_for_now[i];
    }

    /* 复制第二份 */
    for (int i = 0; i < N; i++)
    {
        tem[i+N] = angle_for_now[i];
    }

    /*
     * KMP 匹配算法
     * 1. 构建 LPS 数组 (Longest Prefix Suffix)
     * 2. 在双倍文本中搜索模式
     */
    int lps[360];

    /* 构建前缀表 */
    kmp_build_lps(angle, N, lps);

    /* 执行搜索，返回匹配位置 */
    int match_start_index = kmp_search(tem, M, angle, N, lps);

    /*
     * 保存匹配结果
     * - 匹配成功: detla_theta1 = 匹配起始索引
     * - 匹配失败: detla_theta1 = -1
     */
    detla_theta1 = (match_start_index != -1) ? match_start_index : -1;
}

/*
 * algorithms_init - 算法模块初始化
 *
 * 功能:
 *   创建算法处理任务
 *
 * 任务参数:
 *   - 任务名: "redar_for_rotate"
 *   - 栈大小: 4096 字节
 *   - 优先级: 0 (最高)
 *   - 入口函数: using_READAR_FOR_ROTATE_step2_task
 */
void algorithms_init(void)
{
    osal_task_create("redar_for_rotate", 4096, 0, 0, using_READAR_FOR_ROTATE_step2_task, NULL);
}

/*
 * algorithms_get_delta_theta - 获取角度偏移量
 *
 * 功能:
 *   返回最近一次 KMP 匹配计算得到的角度偏移量
 *
 * 返回值:
 *   int: 角度偏移量 (0-359 表示有效值，-1 表示未匹配)
 */
int algorithms_get_delta_theta(void) { return detla_theta1; }

