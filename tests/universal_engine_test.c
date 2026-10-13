#define _GNU_SOURCE
#include "universal.h"
#include <assert.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL engine:%d: %s status=%u phase=%u active=%u\n",__LINE__,#x,ul_status(s),ul_test_phase(s),ul_active_slot(s));abort();}}while(0)
static int16_t in[256],out[256];
static void sleep_ms(void){struct timespec t={0,1000000};nanosleep(&t,NULL);}
#ifdef UL_RT_AUDIT
extern void ul_rt_enter(void),ul_rt_leave(void);
static void block(ul_engine *s){ul_rt_enter();ul_process(s,in,out,128);ul_rt_leave();}
#else
static void block(ul_engine *s){ul_process(s,in,out,128);}
#endif
static int ready(ul_engine *s,unsigned slot){for(int i=0;i<4000;i++){block(s);if(ul_active_slot(s)==slot&&ul_test_phase(s)==1&&ul_status(s)==UL_READY)return 1;sleep_ms();}return 0;}
static void apply(ul_engine *s,unsigned slot){ul_set(s,0,(float)slot);ul_set(s,1,0);ul_set(s,1,1);ul_set(s,1,1);}
static void copy(const char *a,const char *b){FILE *i=fopen(a,"rb"),*o=fopen(b,"wb");assert(i&&o);char buf[4096];size_t n;while((n=fread(buf,1,sizeof buf,i)))assert(fwrite(buf,1,n,o)==n);assert(!fclose(i));assert(!fclose(o));}
static void path(char *out,const char *root,const char *sub){assert(snprintf(out,1024,"%s/%s",root,sub)<1024);}
static atomic_uint producer_stop;
static void *producer(void *arg){ul_engine *s=arg;for(unsigned i=0;!atomic_load(&producer_stop);i++){
    ul_set(s,2,(float)(i%101)/100);ul_set(s,3,(float)(i%97)/96);char chunk[256];int n=ul_save(s,chunk,sizeof chunk);
    if(n&&i%4096==0)ul_restore(s,chunk,(size_t)n);
}return NULL;}
int main(int argc,char **argv){assert(argc==2);char root[]="/tmp/ul-engine-XXXXXX";assert(mkdtemp(root));char a[1024],b[1024];
    const char *dirs[]={"user","user/patches","user/patches/trim","user/patches/lowpass","defaults"};
    for(unsigned i=0;i<sizeof dirs/sizeof *dirs;i++){path(b,root,dirs[i]);assert(!mkdir(b,0700));}
    const char *files[]={"user/catalog.json","defaults/catalog.json","user/patches/trim/main.pd","user/patches/trim/manifest.json","user/patches/lowpass/main.pd","user/patches/lowpass/manifest.json"};
    for(unsigned i=0;i<sizeof files/sizeof *files;i++){path(a,argv[1],files[i]);path(b,root,files[i]);copy(a,b);}
    ul_engine *s=ul_create(root);CHECK(s);CHECK(!ul_create(root));ul_transport(s,1,0);CHECK(ready(s,0));
    in[0]=10000;in[1]=0;block(s);CHECK(abs(out[0]-10000)<5&&out[1]==0);
    memset(in,0,sizeof in);in[1]=10000;block(s);CHECK(abs(out[1]-10000)<5&&out[0]==0);
    for(int k=0;k<256;k++)in[k]=8000;
    ul_set(s,2,0.5f);for(int i=0;i<10;i++)block(s);CHECK(abs(out[200]-4000)<5);
    ul_set(s,2,NAN);CHECK(ul_get(s,2)==0.5f);ul_set(s,2,1);ul_set(s,3,1);
    ul_transport(s,1,1);apply(s,1);for(int i=0;i<30;i++){block(s);sleep_ms();}CHECK(ul_active_slot(s)==0&&ul_test_phase(s)==1);
    ul_transport(s,0,0);for(int i=0;i<10;i++){block(s);sleep_ms();}CHECK(ul_active_slot(s)==0);
    ul_transport(s,1,0);CHECK(ready(s,1));CHECK(ul_test_pd_overlap(s)==0);
    char saved[256];int saved_n=ul_save(s,saved,sizeof saved);CHECK(saved_n>0&&strstr(saved,"stereo-lowpass"));
    apply(s,0);CHECK(ready(s,0));CHECK(ul_restore(s,saved,(size_t)saved_n));CHECK(ready(s,1));
    CHECK(!ul_restore(s,"UL1 x 1 7fc00000 3f800000",sizeof "UL1 x 1 7fc00000 3f800000"));CHECK(!ul_restore(s,"bad",4));
    /* Preflight cancellation leaves old graph audible and unchanged. */
    ul_test_pause(s,1,0);apply(s,0);for(int i=0;i<10;i++){block(s);sleep_ms();}apply(s,1);ul_test_pause(s,0,0);
    for(int i=0;i<30;i++){block(s);sleep_ms();}CHECK(ul_active_slot(s)==1&&ul_test_phase(s)==1);
    /* Playback resumes during fade: old graph returns, request waits for stop. */
    apply(s,0);for(int i=0;i<2000&&ul_test_phase(s)!=2;i++){block(s);sleep_ms();}CHECK(ul_test_phase(s)==2);
    ul_transport(s,1,1);for(int i=0;i<10;i++)block(s);CHECK(ul_active_slot(s)==1&&ul_test_phase(s)==1);
    ul_transport(s,1,0);CHECK(ready(s,0));
    /* Ownership held by stalled worker: initialized silence, no Pd overlap. */
    ul_test_pause(s,0,1);apply(s,1);for(int i=0;i<4000&&ul_test_phase(s)!=3;i++){block(s);sleep_ms();}CHECK(ul_test_phase(s)==3);
    for(int i=0;i<5000;i++){memset(out,1,sizeof out);block(s);for(int k=0;k<256;k++)CHECK(out[k]==0);}
    ul_transport(s,1,1);ul_test_pause(s,0,0);for(int i=0;i<30;i++){block(s);sleep_ms();}CHECK(ul_active_slot(s)==0); /* ready waits for stop */
    ul_transport(s,1,0);CHECK(ready(s,1));
    /* Real invalid Pd connection passes object subset but must fail graph activation. */
    path(b,root,"user/patches/trim/main.pd");FILE *f=fopen(b,"ab");assert(f);fputs("#X connect 0 0 99 0;\n",f);fclose(f);
    apply(s,0);for(int i=0;i<200;i++){block(s);sleep_ms();}CHECK(ul_active_slot(s)==1);CHECK(ul_status(s)==UL_OBJECT);
    path(a,argv[1],"user/patches/trim/main.pd");copy(a,b);apply(s,0);CHECK(ready(s,0));
    /* Missing candidate is rejected before closing current graph. */
    path(b,root,"user/patches/lowpass/main.pd");char backup[1100];snprintf(backup,sizeof backup,"%s.saved",b);assert(!rename(b,backup));
    apply(s,1);for(int i=0;i<50;i++){block(s);sleep_ms();}CHECK(ul_active_slot(s)==0&&ul_status(s)==UL_MISSING);assert(!rename(backup,b));
    /* Mains-off allows safe handoff when there are no audio callbacks. */
    ul_mains(s,0);apply(s,1);for(int i=0;i<2000&&ul_active_slot(s)!=1;i++)sleep_ms();CHECK(ul_active_slot(s)==1);
    ul_mains(s,1);CHECK(ready(s,1));
    unsigned count=1000;char *env=getenv("UL_SWITCH_COUNT");if(env)count=(unsigned)strtoul(env,NULL,10);
    for(unsigned i=0;i<count;i++){unsigned slot=i&1;apply(s,slot);CHECK(ready(s,slot));}CHECK(ul_test_pd_overlap(s)==0);
    pthread_t writer;assert(!pthread_create(&writer,NULL,producer,s));
    for(int i=0;i<50000;i++){block(s);if(i%256==0)sleep_ms();}
    atomic_store(&producer_stop,1);assert(!pthread_join(writer,NULL));CHECK(ul_test_pd_overlap(s)==0);
    /* Unsupported config never leaks stale output; recovery is explicit. */
    ul_config(s,0);block(s);for(int k=0;k<256;k++)CHECK(out[k]==0);ul_config(s,1);
    ul_mains(s,0);ul_destroy(s);s=ul_create(root);CHECK(s);ul_transport(s,1,0);CHECK(ready(s,0));
    ul_test_pause(s,0,1);apply(s,1);for(int i=0;i<2000&&ul_test_phase(s)!=3;i++){block(s);sleep_ms();}ul_destroy(s);
    printf("PASS Universal stereo, controls, transport, cancellation, rollback, recall, %u repeated switches, concurrent publication and shutdown\n",count);
    return 0;
}
