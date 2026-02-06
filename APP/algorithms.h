#ifndef RB_ALGORITHMS_H
#define RB_ALGORITHMS_H

#include "osal.h"

/*
 * APP/algorithms module
 * 负责：数据处理、360度匹配算法（KMP 等）、更新 detla_theta1 等算法相关状态
 * 对外暴露：algorithms_init() 用于创建算法任务
 */

void algorithms_init(void);
int algorithms_get_delta_theta(void);

#endif // RB_ALGORITHMS_H
