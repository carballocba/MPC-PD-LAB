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
#include <dirent.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL engine:%d: %s status=%u phase=%u active=%u\n",__LINE__,#x,ul_status(s),ul_test_phase(s),ul_active_slot(s));abort();}}while(0)
static int16_t in[256],out[256];
static void sleep_ms(void){struct timespec t={0,1000000};nanosleep(&t,NULL);}
#ifdef UL_RT_AUDIT
extern void ul_rt_enter(void),ul_rt_leave(void);
static void block(ul_engine *s){ul_rt_enter();ul_process(s,in,out,128);ul_rt_leave();}
#else
static void block(ul_engine *s){ul_process(s,in,out,128);}
#endif
static int ready(ul_engine *s,unsigned slot){for(int i=0;i<4000;i++){block(s);if(ul_active_slot(s)==slot&&ul_test_phase(s)==1&&!ul_test_pending(s))return 1;sleep_ms();}return 0;}
static void spectral(ul_engine *s){
    const double frequencies[]={500,8000},mixes[]={0,0.5,1};
    for(unsigned f=0;f<2;f++)for(unsigned m=0;m<3;m++){
        ul_set(s,2,0.5f);ul_set(s,3,(float)mixes[m]);double energy_in=0,energy_out=0,w=2*3.141592653589793*frequencies[f]/44100;
        for(int b=0;b<120;b++){for(int k=0;k<128;k++){in[2*k]=(int16_t)(10000*sin((b*128+k)*w));in[2*k+1]=0;}block(s);
            if(b>=20)for(int k=0;k<128;k++){energy_in+=(double)in[2*k]*in[2*k];energy_out+=(double)out[2*k]*out[2*k];CHECK(out[2*k+1]==0);}}
        double a=1200*(2*3.14159)/44100,den=1-2*(1-a)*cos(w)+(1-a)*(1-a);
        double real=a*(1-(1-a)*cos(w))/den,imag=-a*(1-a)*sin(w)/den;
        double expected=0.5*hypot(1-mixes[m]+mixes[m]*real,mixes[m]*imag);
        CHECK(fabs(sqrt(energy_out/energy_in)-expected)<0.006);
    }
    ul_set(s,2,0);for(int i=0;i<20;i++)block(s);for(int k=0;k<256;k++)CHECK(out[k]==0);
    ul_set(s,2,1);ul_set(s,3,1);for(int k=0;k<256;k++)in[k]=8000;
}
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
    ul_transport(s,1,0);CHECK(ready(s,1));CHECK(ul_test_pd_overlap(s)==0);spectral(s);
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
    /* Cancel a project recall after loading: retain old patch AND old values. */
    ul_test_pause(s,0,1);const char *cancelled_recall="UL1 stereo-lowpass 1 3e800000 3f800000";CHECK(ul_restore(s,cancelled_recall,strlen(cancelled_recall)+1));
    for(int i=0;i<4000&&ul_test_phase(s)!=3;i++){block(s);sleep_ms();}CHECK(ul_test_phase(s)==3);ul_transport(s,1,1);ul_test_pause(s,0,0);
    for(int i=0;i<4000&&ul_test_phase(s)!=4;i++){block(s);sleep_ms();}CHECK(ul_test_phase(s)==4);apply(s,0);ul_transport(s,1,0);CHECK(ready(s,0));CHECK(ul_get(s,2)==1);
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
    /* Cancel after exclusive handoff: reopen retained immutable old graph. */
    ul_test_pause(s,0,1);apply(s,1);for(int i=0;i<4000&&ul_test_phase(s)!=3;i++){block(s);sleep_ms();}CHECK(ul_test_phase(s)==3);
    apply(s,0);ul_test_pause(s,0,0);CHECK(ready(s,0));CHECK(ul_status(s)==UL_CANCELLED);
    /* Same binary loads edited folder content; no VST rebuild. */
    path(b,root,"user/patches/lowpass/main.pd");path(a,argv[1],"user/patches/trim/main.pd");copy(a,b);
    apply(s,1);CHECK(ready(s,1));memset(in,0,sizeof in);in[0]=10000;block(s);CHECK(abs(out[0]-10000)<5&&out[1]==0);
    path(a,argv[1],"user/patches/lowpass/main.pd");copy(a,b);apply(s,0);CHECK(ready(s,0));
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
    /* An unavailable recall ID cannot replace the current graph. */
    unsigned retained=ul_active_slot(s);const char *missing="UL1 absent 1 3f800000 3f800000";
    int accepted=0;for(int i=0;i<4000&&!accepted;i++){accepted=ul_restore(s,missing,strlen(missing)+1);block(s);sleep_ms();}CHECK(accepted);for(int i=0;i<100;i++){block(s);sleep_ms();}CHECK(ul_active_slot(s)==retained&&ul_status(s)==UL_MISSING);
    ul_mains(s,0);ul_destroy(s);s=ul_create(root);CHECK(s);ul_transport(s,1,0);CHECK(ready(s,0));
    ul_test_pause(s,0,1);apply(s,1);for(int i=0;i<2000&&ul_test_phase(s)!=3;i++){block(s);sleep_ms();}ul_destroy(s);
    /* Stable IDs survive catalog reorder on reopen, then explicit no-patch recall. */
    path(b,root,"user/catalog.json");f=fopen(b,"wb");assert(f);
    fputs("{\"schema\":1,\"patches\":[{\"id\":\"stereo-lowpass\",\"folder\":\"patches/lowpass\"},{\"id\":\"stereo-trim\",\"folder\":\"patches/trim\"}]}",f);fclose(f);
    s=ul_create(root);CHECK(s);ul_transport(s,1,0);CHECK(ul_restore(s,saved,(size_t)saved_n));CHECK(ready(s,0));char reordered[256];CHECK(ul_save(s,reordered,sizeof reordered)&&strstr(reordered,"stereo-lowpass"));
    CHECK(ul_restore(s,"UL0",4));for(int i=0;i<200;i++){block(s);sleep_ms();}CHECK(ul_active_slot(s)==2);for(int k=0;k<256;k++)CHECK(out[k]==0);
    apply(s,1);CHECK(ready(s,1));
    /* Sabotage only this test's owned snapshot to demonstrate rollback failure. */
    path(b,root,"user/.loader-cache");DIR *cache=opendir(b);assert(cache);struct dirent *entry;char session[1024]={0};
    while((entry=readdir(cache)))if(!strncmp(entry->d_name,"session-",8)){assert(snprintf(session,sizeof session,"%s/%s",b,entry->d_name)<1024);break;}closedir(cache);CHECK(session[0]);
    for(unsigned i=0;i<2;i++){char host[1200];snprintf(host,sizeof host,"%s/snapshot-%u/host.pd",session,i);unlink(host);}
    path(b,root,"user/patches/lowpass/main.pd");f=fopen(b,"ab");assert(f);fputs("#X connect 0 0 99 0;\n",f);fclose(f);
    apply(s,0);for(int i=0;i<300;i++){block(s);sleep_ms();}CHECK(ul_active_slot(s)==2&&ul_status(s)==UL_ROLLBACK);for(int k=0;k<256;k++)CHECK(out[k]==0);
    apply(s,1);CHECK(ready(s,1));ul_destroy(s);
    printf("PASS Universal stereo, controls, transport, cancellation, rollback, recall, %u repeated switches, concurrent publication and shutdown\n",count);
    return 0;
}
