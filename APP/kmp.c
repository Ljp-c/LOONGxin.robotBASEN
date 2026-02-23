/*
 * kmp.c - KMP (Knuth-Morris-Pratt) 字符串匹配算法实现
 *
 * 功能说明:
 *   本模块实现了 KMP 字符串匹配算法
 *   用于在文本中快速查找模式字符串的位置
 *
 * 算法原理:
 *   KMP 算法的核心是利用已经匹配的信息，避免重复比较
 *   通过预处理模式串，构建 LPS (Longest Prefix Suffix) 数组
 *   使得在匹配失败时，能够快速跳过已匹配的部分
 *
 * 时间复杂度:
 *   - 预处理: O(M)，M 为模式串长度
 *   - 搜索: O(N)，N 为文本串长度
 *   - 总计: O(N + M)
 *
 * 应用场景:
 *   在雷达扫描数据匹配中，将雷达角度数据作为模式
 *   在连续扫描结果中查找相同模式的位置，确定角度偏移
 */

#include "kmp.h"

/*
 * kmp_build_lps - 构建 LPS (Longest Prefix Suffix) 数组
 *
 * 功能:
 *   预处理模式串，计算每个位置的最长相等前后缀长度
 *
 * 参数:
 *   pat: 模式串数组
 *   N:   模式串长度
 *   lps: 输出数组，存储 LPS 值
 *
 * 算法:
 *   lps[i] 表示 pat[0..i] 的最长相等前后缀长度
 *   例如: "AAA" -> lps = [0, 1, 2]
 *         "ABAB" -> lps = [0, 0, 1, 2]
 *         "ABC" -> lps = [0, 0, 0]
 *
 * 示例:
 *   模式: "AABAACAABAA"
 *   lps:  [0, 1, 0, 1, 2, 0, 1, 2, 3, 4, 5]
 */
void kmp_build_lps(const int *pat, int N, int *lps)
{
    /* lps[0] 总是 0，因为单个字符没有前后缀 */
    int len = 0;
    lps[0] = 0;

    /* i 从 1 开始，因为 lps[0] 已经设置 */
    int i = 1;

    /* 计算 lps[i] */
    while (i < N)
    {
        /*
         * 如果当前字符匹配成功
         * 则 lps[i] = len + 1，并移动到下一个位置
         */
        if (pat[i] == pat[len])
        {
            len++;
            lps[i] = len;
            i++;
        }
        /*
         * 如果匹配失败
         */
        else
        {
            /*
              * 如果 len > 0，说明有部分匹配
              * 回退到上一个匹配位置 (len = lps[len - 1])
              * 继续尝试匹配
              */
            if (len != 0)
            {
                len = lps[len - 1];
            }
            /*
             * 如果 len == 0，说明没有匹配
             * lps[i] = 0，移动到下一个位置
             */
            else
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

/*
 * kmp_search - KMP 模式搜索
 *
 * 功能:
 *   在文本串中查找模式串的位置
 *
 * 参数:
 *   text: 文本串数组
 *   M:    文本串长度
 *   pat:  模式串数组
 *   N:    模式串长度
 *   lps:  LPS 数组 (由 kmp_build_lps 构建)
 *
 * 返回值:
 *   匹配成功: 返回模式在文本中的起始位置
 *   匹配失败: 返回 -1
 *
 * 算法:
 *   使用双指针 i (文本) 和 j (模式)
 *   当匹配成功时两者同时前进
 *   当匹配失败时，根据 lps 数组回退 j
 *   不需要回退 i，这是 KMP 的优势
 *
 * 示例:
 *   文本: "ABABDABACDABABCABAB"
 *   模式: "ABABCABAB"
 *   结果: 匹配位置 15
 */
int kmp_search(const int *text, int M, const int *pat, int N, int *lps)
{
    /* 空模式返回 0 */
    if (N == 0) return 0;

    /* 文本指针 i，模式指针 j */
    int i = 0;  /* text 索引 */
    int j = 0;  /* pat 索引 */

    /* 遍历文本 */
    while (i < M)
    {
        /*
         * 当前字符匹配成功
         * 同时移动 i 和 j
         */
        if (pat[j] == text[i])
        {
            i++;
            j++;
        }

        /*
         * 匹配成功
         * j == N 表示模式串全部匹配
         * 返回匹配起始位置: i - j
         */
        if (j == N)
        {
            return i - j;
        }

        /*
         * 匹配失败
         */
        if (i < M && pat[j] != text[i])
        {
            /*
             * 如果 j > 0，回退到 lps[j-1]
             * 继续匹配
             */
            if (j != 0)
            {
                j = lps[j - 1];
            }
            /*
             * 如果 j == 0，说明模式第一个字符就不匹配
             * 移动 i 到下一个位置
             */
            else
            {
                i++;
            }
        }
    }

    /* 遍历完文本没有找到匹配 */
    return -1;
}

