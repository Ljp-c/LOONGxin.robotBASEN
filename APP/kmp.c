#include "kmp.h"

void kmp_build_lps(const int *pat, int N, int *lps)
{
    int len = 0;
    lps[0] = 0;
    int i = 1;
    while (i < N) {
        if (pat[i] == pat[len]) {
            len++;
            lps[i++] = len;
        } else {
            if (len != 0) len = lps[len - 1];
            else lps[i++] = 0;
        }
    }
}

int kmp_search(const int *text, int M, const int *pat, int N, int *lps)
{
    if (N == 0) return 0;
    int i = 0, j = 0;
    while (i < M) {
        if (pat[j] == text[i]) { i++; j++; }
        if (j == N) return i - j;
        if (i < M && pat[j] != text[i]) {
            if (j != 0) j = lps[j - 1];
            else i++;
        }
    }
    return -1;
}
