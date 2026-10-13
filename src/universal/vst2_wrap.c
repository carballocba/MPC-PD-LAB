/* SPDX-License-Identifier: MIT
 * Loader-only derivative of sd88me/mpc-vst-plugins wrapper/vst2_wrap.c
 * at 208745830fc2ed90b2fd0dc480ef5c554284292f (MIT, copyright upstream).
 * Retains the AEffect ABI, module-relative discovery and int16 stereo bridge.
 * Fixed four-parameter subset deliberately omits generic wrapper locks, menus,
 * MIDI/feedback polling and arbitrary-block buffering. No legacy target uses it.
 */
#define _GNU_SOURCE
#include "universal.h"
#include "plugin_dir.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdatomic.h>
#ifdef __APPLE__
#include <dlfcn.h>
#endif
typedef struct AEffect AEffect;
typedef intptr_t (*master_fn)(AEffect *,int32_t,int32_t,intptr_t,void *,float);
struct AEffect {
    int32_t magic;intptr_t (*dispatcher)(AEffect *,int32_t,int32_t,intptr_t,void *,float);
    void (*process)(AEffect *,float **,float **,int32_t);
    void (*setParameter)(AEffect *,int32_t,float);float (*getParameter)(AEffect *,int32_t);
    int32_t numPrograms,numParams,numInputs,numOutputs,flags;intptr_t resvd1,resvd2;
    int32_t initialDelay,realQualities,offQualities;float ioRatio;void *object,*user;
    int32_t uniqueID,version;void (*processReplacing)(AEffect *,float **,float **,int32_t);
    void (*processDoubleReplacing)(AEffect *,double **,double **,int32_t);char future[56];
};
_Static_assert(sizeof(AEffect)==(sizeof(void*)==4?144:192),"VST ABI");
_Static_assert(offsetof(AEffect,processReplacing)==(sizeof(void*)==4?80:120),"VST processing offset");
typedef struct { double samplePos,sampleRate,nanoSeconds,ppqPos,tempo,barStartPos,cycleStartPos,cycleEndPos;
    int32_t timeSigNumerator,timeSigDenominator,smpteOffset,smpteFrameRate,samplesToNextClock,flags; } time_info;
typedef struct { AEffect fx;master_fn master;ul_engine *engine;atomic_uint rate,block;char chunk[256]; } wrap;
static void text(void *p,const char *s,size_t n){if(p&&n){strncpy(p,s,n-1);((char*)p)[n-1]=0;}}
static uint32_t bits(float x){uint32_t n;memcpy(&n,&x,4);return n;}
static void set_parameter(AEffect *fx,int32_t i,float v){if(i>=0&&i<4)ul_set(((wrap*)fx->object)->engine,(unsigned)i,v);}
static float get_parameter(AEffect *fx,int32_t i){return i>=0&&i<4?ul_get(((wrap*)fx->object)->engine,(unsigned)i):0;}
static int16_t f2s(float x){if(!isfinite(x))return 0;if(x>1)x=1;if(x< -1)x= -1;return (int16_t)(x*32767.0f);}
static void run(AEffect *fx,float **in,float **out,int32_t n,int add) {
    if(n<=0||!out||!out[0]||!out[1])return;
    wrap *w=fx->object;
    if(n!=128){for(int32_t k=0;k<n;k++){if(!add)out[0][k]=out[1][k]=0;}ul_config(w->engine,0);return;}
    time_info *ti=w->master?(time_info*)w->master(fx,7,0,0,NULL,0):NULL;
    ul_transport(w->engine,ti!=NULL,ti&&(ti->flags&2));
    int16_t input[256],output[256];
    for(int k=0;k<128;k++){input[2*k]=in&&in[0]?f2s(in[0][k]):0;input[2*k+1]=in&&in[1]?f2s(in[1][k]):0;}
    ul_process(w->engine,input,output,128);
    for(int k=0;k<128;k++){float l=output[2*k]/32768.0f,r=output[2*k+1]/32768.0f;
        if(add){out[0][k]+=l;out[1][k]+=r;}else{out[0][k]=l;out[1][k]=r;}}
}
static void replacing(AEffect *fx,float **in,float **out,int32_t n){run(fx,in,out,n,0);}
static void accumulating(AEffect *fx,float **in,float **out,int32_t n){run(fx,in,out,n,1);}
static intptr_t dispatch(AEffect *fx,int32_t op,int32_t index,intptr_t v,void *p,float opt) {
    wrap *w=fx->object;static const char *names[]={"Patch","Apply","Gain","Mix"};
    switch(op){
    case 0:return 1;
    case 1:ul_destroy(w->engine);free(w);return 1;
    case 2:return v==0;
    case 3:return 0;
    case 5:text(p,"Folder loader",24);return 1;
    case 6:text(p,"",8);return 1;
    case 7:
        if(index==0){char b[24];ul_display(w->engine,b,sizeof b);text(p,b,8);}
        else if(index==1)text(p,get_parameter(fx,1)>=0.5f?"Load":"Ready",8);
        else if(index==2||index==3){char b[24];unsigned z=(unsigned)(get_parameter(fx,index)*1000+0.5f);snprintf(b,sizeof b,"%u.%03u",z/1000,z%1000);text(p,b,8);}
        else return 0;return 1;
    case 8:if(index<0||index>3)return 0;text(p,names[index],8);return 1;
    case 10:atomic_store(&w->rate,bits(opt));ul_config(w->engine,opt==44100.0f&&atomic_load(&w->block)==128);return 1;
    case 11:atomic_store(&w->block,(unsigned)v);ul_config(w->engine,atomic_load(&w->rate)==bits(44100)&&v==128);return 1;
    case 12:ul_mains(w->engine,v!=0);return 1;
    case 23:{int n=ul_save(w->engine,w->chunk,sizeof w->chunk);if(!n||!p)return 0;*(void**)p=w->chunk;return n;}
    case 24:return v>0&&v<=255&&ul_restore(w->engine,p,(size_t)v);
    case 25:return 0; /* MIDI is not part of this effect prototype. */
    case 26:return index==2||index==3;
    case 35:return 1; /* kPlugCategEffect */
    case 45:case 48:text(p,"MPC PD Universal Loader",32);return 1;
    case 47:text(p,"CARBALLO",32);return 1;
    case 49:return 100;
    case 51:return p&&!strcmp(p,"receiveVstTimeInfo")?1:-1;
    case 58:return 2400;
    default:return 0;
    }
}
__attribute__((visibility("default"))) AEffect *VSTPluginMain(master_fn master) {
    char dir[1024],data[1100];
#ifdef __APPLE__
    Dl_info info;if(!dladdr((void*)(uintptr_t)&dispatch,&info)||!mpc_dirname_of(info.dli_fname,dir,sizeof dir))return NULL;
#else
    if(!mpc_plugin_dir(dir,sizeof dir))return NULL;
#endif
    int n=snprintf(data,sizeof data,"%s/data",dir);if(n<0||n>=(int)sizeof data)return NULL;
    wrap *w=calloc(1,sizeof *w);if(!w)return NULL;
    w->engine=ul_create(data);if(!w->engine){free(w);return NULL;}
    w->master=master;atomic_store(&w->rate,bits(44100));atomic_store(&w->block,128);
    AEffect *fx=&w->fx;fx->magic=0x56737450;fx->dispatcher=dispatch;fx->process=accumulating;
    fx->setParameter=set_parameter;fx->getParameter=get_parameter;fx->numPrograms=1;fx->numParams=4;
    fx->numInputs=fx->numOutputs=2;fx->flags=(1<<4)|(1<<5);fx->object=w;fx->uniqueID=0x4d50554c;fx->version=100;fx->processReplacing=replacing;
    return fx;
}
