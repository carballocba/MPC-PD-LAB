#include "engine.h"
#include "z_libpd.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
static void print_hook(const char *s) { fputs(s, stderr); }
int main(int argc, char **argv) {
    if (argc != 3) return 2;
    int direct = strcmp(argv[1], "engine") == 0;
    const mpc_engine_t *engine = mpc_engine();
    libpd_set_printhook(print_hook);
    void *instance;
    if (direct) instance = engine->create(argv[2]);
    else {
        if (libpd_init() || libpd_init_audio(2, 2, 44100)) return 3;
        libpd_add_to_search_path(argv[2]);
        instance = libpd_openfile("libpd_host.pd", argv[2]);
        libpd_start_message(1); libpd_add_float(1); libpd_finish_message("pd", "dsp");
    }
    if (!instance) return 4;
    if (direct && engine->create(argv[2]) != NULL) return 5;
    int16_t input[256] = {0}, output[256];
    if (direct) {
        memset(output, 1, sizeof output);
        engine->process(instance, input, output, 64);
        for (int i=0;i<128;i++) if (output[i] != 0) return 7;
        output[0] = 123;
        engine->process(instance, input, output, -1);
        if (output[0] != 123) return 8;
        engine->process(instance, NULL, output, 128);
        for (int i=0;i<256;i++) if (output[i] != 0) return 9;
    }
    int peaks[2] = {0}, at[2] = {-1,-1};
    for (int block=0; block<350; ++block) {
        memset(input,0,sizeof input);
        if (block == 0) { input[0]=12000; input[1]=-8000; }
        if (direct) engine->process(instance,input,output,128);
        else if (libpd_process_short(2,input,output)) return 6;
        for (int i=0;i<128;++i) for (int c=0;c<2;++c)
            if (abs(output[2*i+c]) > abs(peaks[c])) { peaks[c]=output[2*i+c]; at[c]=block*128+i; }
    }
    printf("%s: peaks=%d,%d frames=%d,%d\n",argv[1],peaks[0],peaks[1],at[0],at[1]);
    if (direct) engine->destroy(instance); else libpd_closefile(instance);
    return !(peaks[0] > 11000 && peaks[1] < -7000 && at[0]>=15300 && at[0]<=15600 && at[1]==at[0]);
}
