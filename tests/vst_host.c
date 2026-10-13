#define _POSIX_C_SOURCE 200809L
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h>
#include <stddef.h>

typedef struct AEffect AEffect;
typedef intptr_t (*Master)(AEffect *,int32_t,int32_t,intptr_t,void *,float);
struct AEffect {
    int32_t magic;
    intptr_t (*dispatcher)(AEffect *,int32_t,int32_t,intptr_t,void *,float);
    void (*process)(AEffect *,float **,float **,int32_t);
    void (*setParameter)(AEffect *,int32_t,float);
    float (*getParameter)(AEffect *,int32_t);
    int32_t numPrograms,numParams,numInputs,numOutputs,flags;
    intptr_t resvd1,resvd2;
    int32_t initialDelay,realQualities,offQualities;
    float ioRatio;
    void *object,*user;
    int32_t uniqueID,version;
    void (*processReplacing)(AEffect *,float **,float **,int32_t);
    void (*processDoubleReplacing)(AEffect *,double **,double **,int32_t);
    char future[56];
};
_Static_assert(sizeof(AEffect) == (sizeof(void *) == 4 ? 144 : 192), "VST ABI size");
_Static_assert(offsetof(AEffect, processReplacing) == (sizeof(void *) == 4 ? 80 : 120), "VST ABI offset");
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#c); return 1; } } while(0)
static intptr_t master(AEffect *e,int32_t op,int32_t i,intptr_t v,void *p,float f) {
    (void)e;(void)i;(void)v;(void)p;(void)f;return op==1 ? 2400 : 0;
}
static double ns(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return (double)t.tv_sec*1e9+t.tv_nsec; }
static int cmp(const void *a,const void *b) { double x=*(const double *)a,y=*(const double *)b;return (x>y)-(x<y); }
int main(int argc,char **argv) {
    CHECK(argc==2 || (argc==3 && !strcmp(argv[2],"--expect-failure")));
    void *lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL); if (!lib) fprintf(stderr,"%s\n",dlerror()); CHECK(lib);
    AEffect *(*entry)(Master); *(void **)(&entry)=dlsym(lib,"VSTPluginMain"); CHECK(entry);
    AEffect *e=entry(master);
    if (argc==3) { CHECK(e==NULL); CHECK(dlclose(lib)==0); puts("PASS incomplete bundle rejected");return 0; }
    CHECK(e);
    CHECK(e->magic==0x56737450 && e->uniqueID==0x4d50444c && e->version==1000);
    CHECK(e->numInputs==2 && e->numOutputs==2 && e->numParams==0);
    CHECK((e->flags&16) && !(e->flags&256));
    CHECK(e->dispatcher(e,35,0,0,NULL,0)==1);
    CHECK(e->dispatcher(e,58,0,0,NULL,0)==2400);
    char name[64]={0};e->dispatcher(e,45,0,0,name,0);CHECK(!strcmp(name,"MPC PD Delay"));
    e->dispatcher(e,47,0,0,name,0);CHECK(!strcmp(name,"MPC-PD-LAB"));
    CHECK(entry(master)==NULL);
    CHECK(e->dispatcher(e,0,0,0,NULL,0)==1);
    e->dispatcher(e,10,0,0,NULL,44100);e->dispatcher(e,11,0,128,NULL,0);e->dispatcher(e,12,0,1,NULL,0);
    float l[128]={0},r[128]={0},ol[128],orr[128];float *in[]={l,r},*out[]={ol,orr};
    int at[2]={-1,-1};float peaks[2]={0,0};
    for(int b=0;b<350;b++) {
        memset(l,0,sizeof l);memset(r,0,sizeof r);
        if(b==0) { l[0]=12000.0f/32768; r[0]=-8000.0f/32768; }
        e->processReplacing(e,in,out,128);
        for(int j=0;j<128;j++) for(int c=0;c<2;c++) {
            CHECK(isfinite(out[c][j]));
            if(fabsf(out[c][j])>fabsf(peaks[c])) { peaks[c]=out[c][j];at[c]=b*128+j; }
        }
    }
    CHECK(at[0]==15435 && at[1]==15435);
    CHECK(fabsf(peaks[0]-12000.0f/32768)<1e-5 && fabsf(peaks[1]+8000.0f/32768)<1e-5);
    printf("PASS VST2 metadata, single instance, stereo impulse: frames %d,%d peaks %.8f,%.8f\n",at[0],at[1],peaks[0],peaks[1]);
    // No input is a scanner/load probe; legacy process must accumulate silence.
    for(int j=0;j<128;j++) ol[j]=orr[j]=0.25f;
    e->process(e,NULL,out,128);
    for(int j=0;j<128;j++) CHECK(ol[j]==0.25f && orr[j]==0.25f);
    double timings[10000];
    memset(l,0,sizeof l);memset(r,0,sizeof r);
    for(int i=0;i<10000;i++) { double t=ns(); e->processReplacing(e,in,out,128); timings[i]=ns()-t; }
    qsort(timings,10000,sizeof timings[0],cmp);
    struct rusage ru;getrusage(RUSAGE_SELF,&ru);
    printf("128-frame callbacks: median %.3f us; p95 %.3f us; max %.3f us; process peak RSS %ld KiB (includes host/runtime)\n",timings[5000]/1000,timings[9500]/1000,timings[9999]/1000,ru.ru_maxrss);
    e->dispatcher(e,12,0,0,NULL,0);e->dispatcher(e,1,0,0,NULL,0);
    e=entry(master);CHECK(e);e->processReplacing(e,NULL,out,128);
    for(int j=0;j<128;j++) CHECK(ol[j]==0 && orr[j]==0);
    at[0]=at[1]=-1;peaks[0]=peaks[1]=0;
    for(int b=0;b<400;b++) {
        memset(l,0,sizeof l);memset(r,0,sizeof r);
        if(b==0) { l[0]=12000.0f/32768;r[0]=-8000.0f/32768; }
        e->processReplacing(e,in,out,64);
        for(int j=0;j<64;j++) for(int c=0;c<2;c++)
            if(fabsf(out[c][j])>fabsf(peaks[c])) { peaks[c]=out[c][j];at[c]=b*64+j; }
    }
    // The preceding empty 128-frame call leaves no partial FIFO state.
    CHECK(at[0]==15563 && at[1]==15563);
    CHECK(peaks[0]==12000.0f/32768 && peaks[1]==-8000.0f/32768);
    printf("PASS 64-frame host FIFO: impulse frames %d,%d (128 added)\n",at[0],at[1]);
    e->dispatcher(e,1,0,0,NULL,0);CHECK(dlclose(lib)==0);
    puts("PASS missing input, legacy accumulation, close/reopen");return 0;
}
