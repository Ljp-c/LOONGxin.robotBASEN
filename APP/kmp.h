#ifndef RB_KMP_H
#define RB_KMP_H

/* KMP algorithm helpers for integer arrays */
void kmp_build_lps(const int *pat, int N, int *lps);
/* 在 text 长度 M 中查找 pat 长度 N 的第一次出现，返回起始索引或 -1（要求 lps 长度 >= N） */
int kmp_search(const int *text, int M, const int *pat, int N, int *lps);

#endif // RB_KMP_H

