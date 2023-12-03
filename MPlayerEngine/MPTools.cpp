#include "MPTools.h"
#include "fft.h"
#include <math.h>


void calc_freq(unsigned char *dest, unsigned char *src) {
    static fft_state *state = nullptr;
    float tmp_out[257];
    int i;

    if(!state) {
        state = fft_init();
    }

    fft_perform((char*)src,tmp_out,state);

    for(i = 0; i < 256; i++) {
        dest[i] = (unsigned char)(((int)sqrt(tmp_out[i + 1])) >> 4);
    }
}
