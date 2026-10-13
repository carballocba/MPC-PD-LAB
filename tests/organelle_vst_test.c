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
int main(int argc,char **argv) {
    CHECK(argc==2 || argc==3);
    void *lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);if(!lib) fprintf(stderr,"%s\n",dlerror());CHECK(lib);
    AEffect *(*entry)(Master);*(void **)(&entry)=dlsym(lib,"VSTPluginMain");CHECK(entry);
    AEffect *e=entry(master);
    if(argc==3) {CHECK(!e); puts("PASS missing file/external rejected by VST entry");dlclose(lib);return 0;}
    CHECK(e && e->uniqueID==0x4347465a && e->uniqueID!=0x4d50444c);
    CHECK(e->numParams==6 && e->numInputs==2 && e->numOutputs==2 && !(e->flags&256));
    char name[64]={0}; e->dispatcher(e,45,0,0,name,0);CHECK(!strcmp(name,"Organelle Granular Freezer"));
    const char *names[]={"Grain Size","Spread","Density","Wet Dry","Freeze","Volume"};
    for(int i=0;i<6;i++) {e->dispatcher(e,8,i,0,name,0);CHECK(!strcmp(name,names[i]));e->setParameter(e,i,0.75f);CHECK(fabsf(e->getParameter(e,i)-(i==4?1.0f:0.75f))<0.001f);}
    e->setParameter(e,4,0); e->setParameter(e,3,0);e->setParameter(e,5,1);
    void *chunk=NULL;intptr_t len=e->dispatcher(e,23,0,0,&chunk,0);CHECK(len>0 && len<256);
    char saved[256];memcpy(saved,chunk,(size_t)len);
    e->setParameter(e,1,0.1f);CHECK(e->dispatcher(e,24,0,len,saved,0)==1);CHECK(fabsf(e->getParameter(e,1)-0.75f)<0.001f);
    float l[128]={0},r[128]={0},ol[128],orr[128];float *in[]={l,r},*out[]={ol,orr};
    for(int b=0;b<400;b++) e->processReplacing(e,in,out,128);
    l[32]=0.1f;r[32]=-0.05f;e->processReplacing(e,in,out,128);
    CHECK(fabsf(ol[96]-0.1f)<0.0002f && fabsf(orr[96]+0.05f)<0.0002f);
    CHECK(!entry(master));e->dispatcher(e,1,0,0,NULL,0);CHECK(dlclose(lib)==0);
    puts("PASS VST identity, six parameters/readback, chunk restore and stereo audio");return 0;
}
