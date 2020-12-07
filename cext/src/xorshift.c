#include <stdio.h>
#include <stdlib.h>
#include "xorshift.h"


static inline unsigned int xorshift(unsigned zero_or_seed) {
    static unsigned int state = 1; //TODO: Seed with time?
    if (zero_or_seed) {
        unsigned int prev_state = state;
        state = zero_or_seed;
        return prev_state;
    }
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

unsigned int xor_rand() {
    //We want 0 to be possible and UINT_MAX to be impossible, but the opposite is normally true for xorshift, so we subtract 1 to make it work.
    return xorshift(0) - 1;
}

unsigned int xor_srand(unsigned int seed) {
    if (!(seed + 1)) {
        fprintf(stderr, "ATTEMPTED TO SEED XORSHIFT WITH %ui!", UINT_MAX);
        fprintf(stdout, "ATTEMPTED TO SEED XORSHIFT WITH %ui!", UINT_MAX);
        fflush(stderr);
        fflush(stdout);
        exit(-1);
        return 0;
    }
    //We want 0 to be possible and UINT_MAX to be impossible, but the opposite is normally true for xorshift, so we subtract 1 to make it work.
    return xorshift(seed + 1) - 1;
}

int xor_rand_signed() {
    unsigned int raw = xorshift(0);
    int out = *(int*)&raw;
    if(out == INT_MAX) return 0;
    return out;
}

int rand_sample(const int arr[], const int arr_len) {
    return arr[xor_rand() % arr_len];
}
