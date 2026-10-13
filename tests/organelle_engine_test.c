#include "engine.h"
#include "z_libpd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#define CHECK(x) do { if(!(x)) {fprintf(stderr,"FAIL engine line %d: %s\n",__LINE__,#x);return 1;} }while(0)
static float observed[6];
static int note,velocity,nnotes;
static void floats(const char *name,float v) {
    const char *keys[]={"knob1","knob2","knob3","knob4","vol","led"};
    for(int i=0;i<6;i++) if(!strcmp(keys[i],name)) observed[i]=v;
}
static void lists(const char *name,int argc,t_atom *argv) {
    if(!strcmp(name,"notes") && argc==2) {note=(int)libpd_get_float(argv);velocity=(int)libpd_get_float(argv+1);nnotes++;}
}
int main(int argc,char **argv) {
    CHECK(argc==2);
    const mpc_engine_t *e=mpc_engine();
    void *s=e->create(argv[1]); CHECK(s); CHECK(!e->create(argv[1]));
    float window[256]; CHECK(!libpd_read_array(window,"hanning",0,256)); CHECK(window[128]>0.99f);
    libpd_set_floathook(floats); libpd_set_listhook(lists);
    void *bind[6]; const char *keys[]={"knob1","knob2","knob3","knob4","vol","notes"};
    for(int i=0;i<6;i++) {bind[i]=libpd_bind(keys[i]);CHECK(bind[i]);}
    e->set_param(s,"knob1","0.37");CHECK(fabsf(observed[0]-0.37f)<1e-5f);
    e->set_param(s,"knob2","0.1");
    e->set_param(s,"knob3","1");
    e->set_param(s,"knob4","0");CHECK(observed[3]==0);
    e->set_param(s,"freeze","1");CHECK(observed[5]==7);
    e->set_param(s,"freeze","1");CHECK(observed[5]==7); /* idempotent absolute state */
    e->set_param(s,"freeze","0");CHECK(observed[5]==3);
    uint8_t on[]={0x90,60,100},off[]={0x80,60,0},cc[]={0xb0,21,127};
    e->midi(s,on,3);CHECK(nnotes==1 && note==60 && velocity==100);
    e->midi(s,off,3);CHECK(nnotes==2 && velocity==0);
    e->midi(s,cc,3);CHECK(observed[0]==1);
    uint8_t foot[]={0xb0,64,127};
    e->midi(s,foot,3);CHECK(observed[5]==7);e->midi(s,foot,3);CHECK(observed[5]==7);
    foot[2]=0;e->midi(s,foot,3);e->set_param(s,"freeze","0");CHECK(observed[5]==3);
    char state[256],value[64];CHECK(e->get_param(s,"state",state,sizeof state)>0);
    e->set_param(s,"knob1","0.2"); e->set_param(s,"state",state);
    CHECK(e->get_param(s,"knob1",value,sizeof value)>0 && atof(value)==1);
    e->set_param(s,"knob1","nan");CHECK(e->get_param(s,"knob1",value,sizeof value)>0 && atof(value)==1);
    e->set_param(s,"state","v1 0 0 0 nan 0 1");CHECK(e->get_param(s,"knob1",value,sizeof value)>0 && atof(value)==1);
    int16_t input[256]={0},output[256];
    memset(output,1,sizeof output);e->process(s,input,output,64);
    for(int i=0;i<128;i++) CHECK(output[i]==0);
    e->process(s,NULL,output,128);for(int i=0;i<256;i++) CHECK(output[i]==0);
    for(int b=0;b<400;b++) e->process(s,input,output,128);
    for(int side=0;side<2;side++) {
        memset(input,0,sizeof input);input[2*32+side]=4000;
        e->process(s,input,output,128);
        CHECK(abs(output[2*96+side]-4000)<4); /* one Pd bus tick */
        CHECK(abs(output[2*96+1-side])<4);
    }
    puts("PASS mother routing, native initialization, MIDI, controls, freeze edges, state, stereo dry channel isolation");
    e->set_param(s,"knob4","1");
    double energy[2]={0};int stereo_diff=0;
    for(int side=0;side<2;side++) {
    energy[0]=energy[1]=0;
    for(int b=0;b<2300;b++) {
        for(int i=0;i<128;i++) {input[2*i+side]=(int16_t)(1200*sin((b*128+i)*0.055));input[2*i+1-side]=0;}
        e->process(s,input,output,128);
        if(b>1800) for(int i=0;i<128;i++) {
            for(int c=0;c<2;c++) energy[c]+=(double)output[2*i+c]*output[2*i+c];
            stereo_diff|=output[2*i]!=output[2*i+1];
        }
    }
    CHECK(energy[0]>100000 && energy[1]>100000 && stereo_diff);
    }
    e->set_param(s,"freeze","1"); memset(input,0,sizeof input); energy[0]=energy[1]=0;
    for(int b=0;b<2300;b++) {e->process(s,input,output,128);if(b>2000) for(int i=0;i<256;i++) energy[i%2]+=(double)output[i]*output[i];}
    CHECK(energy[0]>100000 && energy[1]>100000);
    CHECK(e->get_param(s,"diagnostic",state,sizeof state)>=0 && !strstr(state,"error:"));
    for(int i=0;i<6;i++) libpd_unbind(bind[i]);
    e->destroy(s);
    s=e->create(argv[1]); CHECK(s); e->destroy(s);
    puts("PASS original wet granular stereo energy and frozen-tail retention, close/reopen");
    return 0;
}
