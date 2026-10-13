/* Native host test: exact unchanged reference DSOs remain independent. */
#define _GNU_SOURCE
#include <stdint.h>
#include <dlfcn.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
typedef struct E E;typedef intptr_t(*M)(E*,int32_t,int32_t,intptr_t,void*,float);
struct E{int32_t magic;intptr_t(*dispatch)(E*,int32_t,int32_t,intptr_t,void*,float);void(*process)(E*,float**,float**,int32_t);void(*set)(E*,int32_t,float);float(*get)(E*,int32_t);int32_t programs,params,inputs,outputs,flags;intptr_t r1,r2;int32_t delay,q1,q2;float ratio;void*object,*user;int32_t uid,version;void(*replace)(E*,float**,float**,int32_t);void(*doub)(E*,double**,double**,int32_t);char future[56];};
static intptr_t master(E*e,int32_t op,int32_t i,intptr_t v,void*p,float x){(void)e;(void)i;(void)v;(void)p;(void)x;return op==1?2400:0;}
int main(int n,char**args){assert(n==4);void *libs[3];E *fx[3];int ids[]={0x4d50554c,0x4d50444c,0x4347465a};
 for(int i=0;i<3;i++){libs[i]=dlopen(args[i+1],RTLD_NOW|RTLD_LOCAL);if(!libs[i])fprintf(stderr,"%s\n",dlerror());assert(libs[i]);E*(*entry)(M);void*p=dlsym(libs[i],"VSTPluginMain");memcpy(&entry,&p,sizeof entry);fx[i]=entry(master);assert(fx[i]&&fx[i]->uid==ids[i]);fx[i]->dispatch(fx[i],12,0,1,NULL,0);}
 float l[128]={0},r[128]={0},ol[128],or_[128];float *in[]={l,r},*out[]={ol,or_};
 for(int b=0;b<500;b++){for(int k=0;k<128;k++)l[k]=0.1f*sinf((b*128+k)*0.03f);
  for(int i=0;i<3;i++){fx[i]->replace(fx[i],in,out,128);for(int k=0;k<128;k++)assert(isfinite(ol[k])&&isfinite(or_[k]));}
  struct timespec t={0,100000};nanosleep(&t,NULL);
 }
 for(int i=2;i>=0;i--){fx[i]->dispatch(fx[i],12,0,0,NULL,0);fx[i]->dispatch(fx[i],1,0,0,NULL,0);dlclose(libs[i]);}
 puts("PASS native coexistence with unchanged MPDL Delay and CGFZ Freezer reference binaries");return 0;}
