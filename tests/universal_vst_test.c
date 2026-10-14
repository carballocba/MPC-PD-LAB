#define _GNU_SOURCE
#include <stdint.h>
#include <stddef.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
typedef struct AEffect AEffect;typedef intptr_t (*Master)(AEffect *,int32_t,int32_t,intptr_t,void *,float);
struct AEffect {int32_t magic;intptr_t(*dispatcher)(AEffect*,int32_t,int32_t,intptr_t,void*,float);void(*process)(AEffect*,float**,float**,int32_t);void(*setParameter)(AEffect*,int32_t,float);float(*getParameter)(AEffect*,int32_t);int32_t numPrograms,numParams,numInputs,numOutputs,flags;intptr_t resvd1,resvd2;int32_t initialDelay,realQualities,offQualities;float ioRatio;void*object,*user;int32_t uniqueID,version;void(*processReplacing)(AEffect*,float**,float**,int32_t);void(*processDoubleReplacing)(AEffect*,double**,double**,int32_t);char future[56];};
_Static_assert(sizeof(AEffect)==(sizeof(void*)==4?144:192),"ABI size");
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL VST:%d %s\n",__LINE__,#x);abort();}}while(0)
static struct {double samplePos,sampleRate,nanoSeconds,ppqPos,tempo,barStartPos,cycleStartPos,cycleEndPos;int32_t numerator,denominator,smpteOffset,smpteRate,nextClock,flags;} ti;
static int known=1;
static intptr_t master(AEffect *e,int32_t op,int32_t i,intptr_t v,void*p,float f){(void)e;(void)i;(void)v;(void)p;(void)f;return op==7?(known?(intptr_t)&ti:0):op==1?2400:0;}
static float left[128],right[128],ol[128],or_[128];static float *in[]={left,right},*out[]={ol,or_};
static void tick(AEffect*e){e->processReplacing(e,in,out,128);}
static void nap(void){struct timespec t={0,1000000};nanosleep(&t,NULL);}
static int chunk(AEffect *e,char *b){void*p=NULL;int n=(int)e->dispatcher(e,23,0,0,&p,0);if(n>0&&n<=255)memcpy(b,p,(size_t)n);return n;}
static int wait_patch(AEffect*e,const char *id){char b[256];for(int i=0;i<4000;i++){tick(e);int n=chunk(e,b);char state[24];e->dispatcher(e,7,0,0,state,0);if(n&&strstr(b,id)&&!strncmp(state,"READY",5)&&fabsf(ol[100])>0.01f)return 1;nap();}char display[24];e->dispatcher(e,7,0,0,display,0);fprintf(stderr,"WAIT failed %s chunk=%s display=%s output=%g\n",id,b,display,ol[100]);return 0;}
static void select_patch(AEffect*e,int slot){e->setParameter(e,0,(float)slot);e->setParameter(e,1,0);e->setParameter(e,1,1);}
int main(int argc,char**argv){CHECK(argc==2);int load_flags=RTLD_NOW|RTLD_LOCAL;
#ifdef __APPLE__
 /* Development host mirrors the ELF module lifetime; dyld/ASan reload otherwise
    registers the same image globals twice. Linux tests exercise the ELF flag. */
 load_flags|=RTLD_NODELETE;
#endif
 void *lib=dlopen(argv[1],load_flags);CHECK(lib);
 AEffect*(*entry)(Master);void*symbol=dlsym(lib,"VSTPluginMain");memcpy(&entry,&symbol,sizeof entry);CHECK(entry);AEffect*e=entry(master);CHECK(e);CHECK(!entry(master));
 CHECK(e->magic==0x56737450&&e->uniqueID==0x4d50554c&&e->numParams==4&&e->numInputs==2&&e->numOutputs==2&&!(e->flags&(1<<8)));
 CHECK(e->dispatcher(e,35,0,0,NULL,0)==1);CHECK(!e->dispatcher(e,26,0,0,NULL,0)&&e->dispatcher(e,26,2,0,NULL,0));
 e->dispatcher(e,10,0,0,NULL,44100);e->dispatcher(e,11,0,128,NULL,0);e->dispatcher(e,12,0,1,NULL,0);
 for(int i=0;i<128;i++)left[i]=0.25f;
 CHECK(wait_patch(e,"stereo-trim"));for(int k=0;k<10;k++)tick(e);CHECK(fabsf(ol[100]-0.25f)<0.0002f&&or_[100]==0);
 for(int op=7;op<=8;op++)for(int i=0;i<4;i++){struct{char text[8];unsigned char guard[16];} bounded;memset(&bounded,0xa5,sizeof bounded);CHECK(e->dispatcher(e,op,i,0,bounded.text,0));CHECK(memchr(bounded.text,0,8));for(unsigned k=0;k<16;k++)CHECK(bounded.guard[k]==0xa5);}
 char names[4][24];for(int i=0;i<4;i++)CHECK(e->dispatcher(e,8,i,0,names[i],0));CHECK(!strcmp(names[0],"Patch")&&!strcmp(names[1],"Apply")&&!strcmp(names[2],"Gain")&&!strcmp(names[3],"Mix"));
 e->setParameter(e,2,0.37f);e->setParameter(e,3,0.4f);for(int i=0;i<10;i++)tick(e);CHECK(fabsf(ol[100]-0.25f*0.37f)<0.0003f);
 char saved[256],b[256];int n=chunk(e,saved);CHECK(n>0);
 ti.flags=2;select_patch(e,1);for(int i=0;i<30;i++){tick(e);nap();}CHECK(chunk(e,b)&&strstr(b,"stereo-trim"));
 known=0;ti.flags=0;for(int i=0;i<20;i++){tick(e);nap();}CHECK(chunk(e,b)&&strstr(b,"stereo-trim"));
 known=1;CHECK(wait_patch(e,"stereo-lowpass"));CHECK(e->dispatcher(e,24,0,n,saved,0));CHECK(wait_patch(e,"stereo-trim"));
 CHECK(e->getParameter(e,2)==0.37f&&e->getParameter(e,3)==0.4f);CHECK(!e->dispatcher(e,24,0,4,"bad",0));
 e->dispatcher(e,10,0,0,NULL,48000);tick(e);for(int i=0;i<128;i++)CHECK(ol[i]==0&&or_[i]==0);
 e->dispatcher(e,10,0,0,NULL,44100);left[0]=NAN;left[1]=INFINITY;tick(e);CHECK(ol[0]==0&&ol[1]==0);
 memset(ol,1,sizeof ol);e->processReplacing(e,in,out,64);for(int i=0;i<64;i++)CHECK(ol[i]==0&&or_[i]==0);e->dispatcher(e,11,0,128,NULL,0);
 e->dispatcher(e,12,0,0,NULL,0);e->dispatcher(e,1,0,0,NULL,0);e=entry(master);CHECK(e);CHECK(e->dispatcher(e,24,0,n,saved,0));e->dispatcher(e,12,0,1,NULL,0);CHECK(wait_patch(e,"stereo-trim"));e->dispatcher(e,1,0,0,NULL,0);dlclose(lib);
 /* Linux NODELETE retains Pd class/symbol globals across host DSO unload/reload. */
 lib=dlopen(argv[1],load_flags);CHECK(lib);symbol=dlsym(lib,"VSTPluginMain");memcpy(&entry,&symbol,sizeof entry);e=entry(master);CHECK(e);CHECK(e->dispatcher(e,24,0,n,saved,0));CHECK(wait_patch(e,"stereo-trim"));e->dispatcher(e,1,0,0,NULL,0);dlclose(lib);
 puts("PASS VST ABI/effect identity, four Q-Links, transport, exact control/project recall, singleton, invalid input/config and reopen");return 0;}
