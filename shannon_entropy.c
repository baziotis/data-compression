#include <math.h>
#include <stdio.h>
#include <string.h>

// Look at the README for a short explanation on entropy.

#define MAXLEN 2048

int construct_histogram(char *s, int hist[], int len) {
    int hist_len = 0;
    int char_map[256];
    for(size_t i = 0; i != 256; ++i) char_map[i] = -1;
    for(size_t i = 0; i != len; ++i) {
        int hist_pos = char_map[s[i]];  // position assigned for this number in
                                        // the histogram.
        if(char_map[s[i]] == -1) {      // new symbol found, create new position
            char_map[s[i]] = hist_pos = hist_len;
            ++hist_len;
        }
        hist[hist_pos]++;
    }
    return hist_len;
}

double compute_entropy(int hist[], int hist_len, int str_len) {
    double H = 0.0;
    for(int i = 0; i != hist_len; ++i) {
        double pi = (double) hist[i] / (double) str_len;
        H += pi * log2(pi);
    }
    return -H;
}

int main(void) {
    int hist[MAXLEN];
    char s[MAXLEN];
    strcpy(s, "ABBCCCDDDD");
    int str_len = strlen(s);
    int hist_len = construct_histogram(s, hist, str_len);
    double H = compute_entropy(hist, hist_len, str_len);
    printf("H = %lf\n", H);
}
