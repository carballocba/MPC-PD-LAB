/* SPDX-License-Identifier: MIT
 * Reuses the pinned Phase 2 private-libpd stereo processing contract. Graph and
 * filesystem work belong to the worker. This is a curated Vanilla host, not a sandbox.
 */
#define _GNU_SOURCE
#include "universal.h"
#include "manifest.h"
#include "z_libpd.h"
#include <stdatomic.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>

_Static_assert(ATOMIC_INT_LOCK_FREE==2,"32-bit publication must be lock-free");
enum { P_INIT, P_RUN, P_OUT, P_WORK, P_READY, P_IN, P_FAIL };
typedef struct { char id[UL_ID], revision[UL_REV]; uint32_t values[2]; int none; } saved_state;
typedef struct { atomic_uint state; saved_state value; unsigned sequence; } restore_slot;
typedef struct { ul_patch_spec spec; char path[1024]; unsigned slot,cache_index; } snapshot;
struct ul_engine {
    char root[1024],user[1024],cache[1024],session[1024]; pthread_t worker;
    atomic_uint phase,guard,mains,known,playing,config,closing,pending,prepared;
    atomic_uint serial,token,target,cancel,committed,rollback,status,active,selected,apply;
    atomic_uint values[2],record_seq,record[24];
    restore_slot restores[4];
    unsigned last_restore_sequence;
    ul_catalog_entry catalog[2]; /* immutable after catalog_ready publication */
    atomic_uint catalog_ready;
    snapshot *current,*candidate;
    void *graph;
    uint32_t applied_values[2]; /* exclusive Pd owner only */
    unsigned ready_slot,ready_token; char ready_id[UL_ID],ready_rev[UL_REV];
    float fade; unsigned fade_left; /* accessed only while guard is owned */
    char diagnostic[8192]; size_t diagnostic_used; int load_error;
    atomic_uint render_fault,pd_calls,overlap;
#ifdef UL_TESTING
    atomic_uint pause_preflight,pause_load;
#endif
};
static atomic_uint module_active;
static int initialized; /* only one module worker can exist; survives graph closes */
static ul_engine *hook_engine;
static uint32_t bits(float v){uint32_t n;memcpy(&n,&v,4);return n;}
static float value(uint32_t n){float v;memcpy(&v,&n,4);return v;}
static void nap(void){struct timespec t={0,2000000};nanosleep(&t,NULL);}
static void pd_enter(ul_engine *s){if(atomic_fetch_add(&s->pd_calls,1))atomic_fetch_add(&s->overlap,1);}
static void pd_leave(ul_engine *s){atomic_fetch_sub(&s->pd_calls,1);}
static int claim(ul_engine *s){unsigned zero=0;return atomic_compare_exchange_strong(&s->guard,&zero,1);}
static void unclaim(ul_engine *s){atomic_store(&s->guard,0);}
static void print_hook(const char *text) {
    ul_engine *s=hook_engine;if(!s)return;
    unsigned p=atomic_load(&s->phase);
    size_t n=strnlen(text,257);
    if(p==P_WORK||p==P_INIT){
        size_t room=sizeof s->diagnostic-1-s->diagnostic_used;
        if(n>256||n>room){s->load_error=1;n=n>room?room:n;if(n>256)n=256;}
        memcpy(s->diagnostic+s->diagnostic_used,text,n);s->diagnostic_used+=n;s->diagnostic[s->diagnostic_used]=0;
        if(strstr(s->diagnostic,"couldn't create")||strstr(s->diagnostic,"error:")||
           strstr(s->diagnostic,"connection failed")||strstr(s->diagnostic,"DSP loop"))s->load_error=1;
    }else atomic_store(&s->render_fault,1); /* no logging, allocation or shared text on audio */
}
static void log_status(ul_engine *s,unsigned code) {
    atomic_store(&s->status,code);
    char path[1200];snprintf(path,sizeof path,"%s/loader.log",s->user);
    struct stat st;if(!lstat(path,&st)&&S_ISLNK(st.st_mode))return;
    if(!lstat(path,&st)&&st.st_size>65536){char old[1220];snprintf(old,sizeof old,"%s.previous",path);rename(path,old);}
    int fd=open(path,O_WRONLY|O_CREAT|O_APPEND|O_NOFOLLOW,0600);if(fd<0)return;
    char b[96];int n=snprintf(b,sizeof b,"status=%u active=%u\n",code,atomic_load(&s->active));
    if(n>0)(void)write(fd,b,(size_t)n);close(fd);
}
static int make_dir(const char *p) {
    struct stat st;if(!lstat(p,&st))return S_ISDIR(st.st_mode)&&!S_ISLNK(st.st_mode);
    return errno==ENOENT&&!mkdir(p,0700);
}
static int parents(char *p,size_t start) {
    for(size_t k=start;p[k];k++)if(p[k]=='/'){p[k]=0;int ok=make_dir(p);p[k]='/';if(!ok)return 0;}return 1;
}
static int remove_tree(const char *p) {
    /* Only called for mkdtemp-owned snapshots, never patch folders or arbitrary paths. */
    DIR *d=opendir(p);if(!d)return 0;struct dirent *e;int ok=1;
    while((e=readdir(d)))if(strcmp(e->d_name,".")&&strcmp(e->d_name,"..")){
        char q[1600];struct stat st;int n=snprintf(q,sizeof q,"%s/%s",p,e->d_name);
        if(n<0||n>=(int)sizeof q||lstat(q,&st)){ok=0;continue;}
        if(S_ISDIR(st.st_mode)&&!S_ISLNK(st.st_mode))ok&=remove_tree(q);else ok&=!unlink(q);
    }closedir(d);return !rmdir(p)&&ok;
}
static void discard(snapshot *p){if(p){if(p->path[0])remove_tree(p->path);free(p);}}
static unsigned prepare(ul_engine *s,unsigned slot,const saved_state *restore,snapshot **out) {
    if(slot>1||!atomic_load(&s->catalog_ready))return UL_MISSING;
    snapshot *p=calloc(1,sizeof *p);if(!p)return UL_INVALID;p->slot=slot;
    char folder[1024],manifest[1024];
    int n=snprintf(folder,sizeof folder,"%s/%s",s->user,s->catalog[slot].folder);
    if(n<0||n>=(int)sizeof folder){discard(p);return UL_INVALID;}
    /* Check every parent with the same no-symlink policy as required assets. */
    char rel[UL_PATH];n=snprintf(rel,sizeof rel,"%s/manifest.json",s->catalog[slot].folder);
    if(n<0||n>=(int)sizeof rel||!ul_regular_path(s->user,rel,manifest)){discard(p);return UL_MISSING;}
    if(!ul_manifest_read(manifest,s->catalog[slot].id,&p->spec)){discard(p);return UL_INVALID;}
    if(restore&&strcmp(p->spec.revision,restore->revision)){discard(p);return UL_INVALID;}
    p->cache_index=s->current?1-s->current->cache_index:0;
    n=snprintf(p->path,sizeof p->path,"%s/snapshot-%u",s->session,p->cache_index);
    if(n<0||n>=(int)sizeof p->path||mkdir(p->path,0700)){p->path[0]=0;discard(p);return UL_INVALID;}
    size_t total=0;
    for(unsigned k=0;k<p->spec.count;k++){
        char source[1024],dest[1600];struct stat before,after;
        if(!ul_regular_path(folder,p->spec.files[k],source)){discard(p);return UL_MISSING;}
        int in=open(source,O_RDONLY|O_NOFOLLOW);if(in<0){discard(p);return UL_MISSING;}
        if(fstat(in,&before)||!S_ISREG(before.st_mode)||before.st_size<=0||before.st_size>131072||
           (total+=(size_t)before.st_size)>UL_TEXT_LIMIT){close(in);discard(p);return UL_INVALID;}
        char *text=malloc((size_t)before.st_size+1);if(!text){close(in);discard(p);return UL_INVALID;}
        size_t used=0;while(used<(size_t)before.st_size){ssize_t got=read(in,text+used,(size_t)before.st_size-used);if(got<=0)break;used+=(size_t)got;}
        int valid=!fstat(in,&after)&&used==(size_t)before.st_size&&before.st_size==after.st_size&&before.st_mtime==after.st_mtime;
        close(in);text[used]=0;valid&=!memchr(text,0,used)&&ul_audit_pd(text,used,&p->spec);
        if(!valid){free(text);discard(p);return UL_OBJECT;}
        n=snprintf(dest,sizeof dest,"%s/%s",p->path,p->spec.files[k]);
        if(n<0||n>=(int)sizeof dest||!parents(dest,strlen(p->path)+1)){free(text);discard(p);return UL_INVALID;}
        int fd=open(dest,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600);size_t written=0;
        if(fd>=0)while(written<used){ssize_t z=write(fd,text+written,used-written);if(z<=0)break;written+=(size_t)z;}
        free(text);if(fd>=0)close(fd);if(fd<0||written!=used){discard(p);return UL_INVALID;}
    }
    if(!ul_audit_closure(p->path,&p->spec)){discard(p);return UL_OBJECT;}
    char host[1200];snprintf(host,sizeof host,"%s/host.pd",p->path);
    FILE *f=fopen(host,"wb");if(!f){discard(p);return UL_INVALID;}
    const char *canvas="#N canvas 0 0 400 300 10;\n#X obj 20 20 adc~ 1 2;\n#X obj 20 70 main;\n#X obj 20 120 dac~ 1 2;\n#X connect 0 0 1 0;\n#X connect 0 1 1 1;\n#X connect 1 0 2 0;\n#X connect 1 1 2 1;\n";
    int ok=fwrite(canvas,1,strlen(canvas),f)==strlen(canvas);ok&=!fclose(f);
    if(!ok){discard(p);return UL_INVALID;}*out=p;return UL_READY;
}
static void close_graph(ul_engine *s){if(s->graph){pd_enter(s);libpd_closefile(s->graph);s->graph=NULL;pd_leave(s);}}
static int open_graph(ul_engine *s,snapshot *p) {
    s->diagnostic_used=0;s->diagnostic[0]=0;s->load_error=0;atomic_store(&s->render_fault,0);
    pd_enter(s);libpd_clear_search_path();libpd_add_to_search_path(p->path);
    s->graph=libpd_openfile("host.pd",p->path);
    if(s->graph&&!s->load_error){
        s->applied_values[0]=atomic_load(&s->values[0]);s->applied_values[1]=atomic_load(&s->values[1]);
        libpd_float("ul-gain",value(s->applied_values[0]));
        libpd_float("ul-mix",value(s->applied_values[1]));
        libpd_start_message(1);libpd_add_float(1);libpd_finish_message("pd","dsp");
        int16_t in[256]={0},out[256];for(int i=0;i<4;i++)if(libpd_process_short(2,in,out))s->load_error=1;
    }
    int ok=s->graph&&!s->load_error;pd_leave(s);if(!ok)close_graph(s);return ok;
}
static void record(ul_engine *s,unsigned slot,const char *id,const char *rev) {
    char b[96]={0};if(id)memcpy(b,id,strlen(id));if(rev)memcpy(b+64,rev,strlen(rev));
    atomic_fetch_add(&s->record_seq,1);
    for(unsigned k=0;k<24;k++){uint32_t v;memcpy(&v,b+4*k,4);atomic_store(&s->record[k],v);}
    atomic_store(&s->active,slot);atomic_fetch_add(&s->record_seq,1);
}
static void activate(ul_engine *s) {
    record(s,s->ready_slot,s->ready_id,s->ready_rev);
    s->fade=0;s->fade_left=512;
    atomic_store(&s->phase,s->ready_slot<2?P_IN:P_FAIL);
    atomic_store(&s->committed,s->ready_token);
}
static int has_restore(ul_engine *s){for(int i=0;i<4;i++)if(atomic_load(&s->restores[i].state)==2)return 1;return 0;}
static int take_restore(ul_engine *s,saved_state *out) {
    int chosen=-1;unsigned seq=0;
    for(int i=0;i<4;i++)if(atomic_load(&s->restores[i].state)==2&&s->restores[i].sequence>s->last_restore_sequence&&s->restores[i].sequence>=seq){chosen=i;seq=s->restores[i].sequence;}
    if(chosen<0){for(int i=0;i<4;i++)if(atomic_load(&s->restores[i].state)==2&&s->restores[i].sequence<=s->last_restore_sequence)atomic_store(&s->restores[i].state,0);return 0;}
    *out=s->restores[chosen].value;s->last_restore_sequence=seq;
    for(int i=0;i<4;i++)if(atomic_load(&s->restores[i].state)==2&&s->restores[i].sequence<=seq)atomic_store(&s->restores[i].state,0);
    return 1;
}
static unsigned resolve(ul_engine *s,const char *id){for(unsigned i=0;i<2;i++)if(!strcmp(s->catalog[i].id,id))return i;return 2;}
static int cancelled(ul_engine *s,unsigned token){return atomic_load(&s->cancel)==token;}
#ifdef UL_TESTING
static void pause_worker(ul_engine *s,atomic_uint *flag,unsigned token){while(atomic_load(flag)&&!atomic_load(&s->closing)&&!cancelled(s,token))nap();}
#endif
static void transaction(ul_engine *s,unsigned slot,const saved_state *restore,int initial) {
    unsigned token=atomic_load(&s->token),code=UL_READY;
    snapshot *next=NULL,*previous=s->current;
#ifdef UL_TESTING
    pause_worker(s,&s->pause_preflight,token);
#endif
    if(cancelled(s,token))goto preflight_cancel;
    if(slot<2){code=prepare(s,slot,restore,&next);if(code!=UL_READY){log_status(s,code);goto preflight_fail;}}
    else if(!restore||!restore->none){log_status(s,UL_MISSING);goto preflight_fail;}
    atomic_store(&s->prepared,token);log_status(s,UL_WAIT_STOP);
    for(;;){
        if(atomic_load(&s->closing)){discard(next);return;}
        if(atomic_load(&s->phase)==P_WORK)break;
        if(cancelled(s,token)&&atomic_load(&s->phase)!=P_OUT)goto preflight_cancel;
        if((initial||!previous||!atomic_load(&s->mains))&&claim(s)){
            if(initial||!previous||!atomic_load(&s->mains))atomic_store(&s->phase,P_WORK);
            unclaim(s);
        }
        nap();
    }
    atomic_store(&s->prepared,0);log_status(s,UL_LOADING);
#ifdef UL_TESTING
    pause_worker(s,&s->pause_load,token);
#endif
    close_graph(s);
    if(restore&&!restore->none){atomic_store(&s->values[0],restore->values[0]);atomic_store(&s->values[1],restore->values[1]);atomic_store(&s->selected,slot);}
    if(cancelled(s,token)){code=UL_CANCELLED;goto rollback;}
    if(next&&!open_graph(s,next)){code=UL_OBJECT;goto rollback;}
    if(cancelled(s,token)){code=UL_CANCELLED;goto rollback;}
    s->candidate=next;
    goto publish;
rollback:
    close_graph(s);discard(next);next=NULL;
    if(previous&&open_graph(s,previous)){next=previous;code=code==UL_CANCELLED?UL_CANCELLED:UL_OBJECT;}
    else if(previous){code=UL_ROLLBACK;discard(previous);previous=NULL;s->current=NULL;}
    s->candidate=next;
publish:
    s->ready_slot=next?next->slot:2;s->ready_token=token;
    memset(s->ready_id,0,sizeof s->ready_id);memset(s->ready_rev,0,sizeof s->ready_rev);
    if(next){memcpy(s->ready_id,next->spec.id,UL_ID);memcpy(s->ready_rev,next->spec.revision,UL_REV);}
    atomic_store(&s->rollback,0);log_status(s,code);atomic_store(&s->phase,P_READY);
    for(;;){
        if(atomic_load(&s->closing))return;
        if(initial&&has_restore(s))atomic_store(&s->cancel,token);
        if(atomic_load(&s->phase)==P_WORK&&atomic_exchange(&s->rollback,0)){
            if(next!=previous){close_graph(s);discard(next);next=NULL;s->candidate=NULL;code=UL_CANCELLED;
                if(previous&&open_graph(s,previous))next=previous;
                else if(previous){discard(previous);previous=NULL;s->current=NULL;code=UL_ROLLBACK;}
                goto publish;}
        }
        if(atomic_load(&s->committed)==token)break;
        /* Deactivation is an explicit, verified quiescent lifecycle boundary. */
        if((!atomic_load(&s->mains)||(!previous&&!next))&&claim(s)){
            if(atomic_load(&s->phase)==P_READY&&(!atomic_load(&s->mains)||(!previous&&!next)))activate(s);
            unclaim(s);
        }
        nap();
    }
    if(previous&&previous!=next)discard(previous);
    s->current=next;s->candidate=NULL;atomic_store(&s->pending,0);log_status(s,code);return;
preflight_cancel:
    log_status(s,UL_CANCELLED);
preflight_fail:
    discard(next);atomic_store(&s->prepared,0);
    if(initial)atomic_store(&s->phase,P_FAIL);
    atomic_store(&s->pending,0);
}
static void *worker(void *data) {
    ul_engine *s=data;hook_engine=s;
    char catalog[1200];snprintf(catalog,sizeof catalog,"%s/catalog.json",s->user);
    int ok=make_dir(s->user)&&make_dir(s->cache);
    struct stat st;
    if(ok&&lstat(catalog,&st)&&errno==ENOENT){
        char defaults[1200];snprintf(defaults,sizeof defaults,"%s/defaults/catalog.json",s->root);
        FILE *in=fopen(defaults,"rb");int fd=open(catalog,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600);
        if(in&&fd>=0){char b[16385];size_t n=fread(b,1,sizeof b,in);if(n<=16384)(void)write(fd,b,n);}
        if(in)fclose(in);if(fd>=0)close(fd);
    }
    ok=ok&&!lstat(catalog,&st)&&S_ISREG(st.st_mode)&&!S_ISLNK(st.st_mode)&&ul_catalog_read(catalog,s->catalog);
    atomic_store(&s->catalog_ready,ok);
    if(ok){int n=snprintf(s->session,sizeof s->session,"%s/session-XXXXXX",s->cache);ok=n>0&&n<(int)sizeof s->session&&mkdtemp(s->session)!=NULL;if(!ok)s->session[0]=0;}
    if(ok){
        pd_enter(s);if(!initialized){ok=!libpd_init();if(ok)initialized=1;}
        libpd_set_printhook(print_hook);ok=ok&&libpd_blocksize()==64&&!libpd_init_audio(2,2,44100);pd_leave(s);
    }
    if(!ok){log_status(s,UL_MISSING);atomic_store(&s->phase,P_FAIL);atomic_store(&s->pending,0);}
    else {
        saved_state state;int restore=take_restore(s,&state);unsigned target=restore?(state.none?2:resolve(s,state.id)):0;
        atomic_store(&s->target,target);atomic_store(&s->token,atomic_fetch_add(&s->serial,1)+1);atomic_store(&s->pending,1);
        transaction(s,target,restore?&state:NULL,1);
    }
    int reported_fault=0;
    while(!atomic_load(&s->closing)){
        if(atomic_load(&s->render_fault)&&!reported_fault){log_status(s,UL_OBJECT);reported_fault=1;}
        if(!atomic_load(&s->render_fault))reported_fault=0;
        unsigned zero=0;
        if(has_restore(s)&&atomic_compare_exchange_strong(&s->pending,&zero,2)){
            saved_state state;
            if(take_restore(s,&state)){unsigned target=state.none?2:resolve(s,state.id);
                atomic_store(&s->target,target);atomic_store(&s->token,atomic_fetch_add(&s->serial,1)+1);atomic_store(&s->pending,1);
                transaction(s,target,&state,0);
            }else atomic_store(&s->pending,0);
        }else if(atomic_load(&s->pending)==1)transaction(s,atomic_load(&s->target),NULL,0);
        nap();
    }
    while(!claim(s))nap();atomic_store(&s->phase,P_WORK);close_graph(s);unclaim(s);
    if(s->candidate&&s->candidate!=s->current)discard(s->candidate);
    discard(s->current);s->current=s->candidate=NULL;if(s->session[0])remove_tree(s->session);hook_engine=NULL;return NULL;
}
ul_engine *ul_create(const char *root) {
    unsigned zero=0;if(!root||strlen(root)>900||!atomic_compare_exchange_strong(&module_active,&zero,1))return NULL;
    ul_engine *s=calloc(1,sizeof *s);if(!s){atomic_store(&module_active,0);return NULL;}
    snprintf(s->root,sizeof s->root,"%s",root);snprintf(s->user,sizeof s->user,"%s/user",root);snprintf(s->cache,sizeof s->cache,"%s/user/.loader-cache",root);
    atomic_store(&s->active,2);atomic_store(&s->config,1);atomic_store(&s->mains,1);atomic_store(&s->pending,2);
    atomic_store(&s->values[0],bits(1));atomic_store(&s->values[1],bits(1));
    if(pthread_create(&s->worker,NULL,worker,s)){free(s);atomic_store(&module_active,0);return NULL;}return s;
}
void ul_destroy(ul_engine *s){if(!s)return;atomic_store(&s->mains,0);atomic_store(&s->closing,1);pthread_join(s->worker,NULL);free(s);atomic_store(&module_active,0);}
void ul_set(ul_engine *s,unsigned index,float v) {
    if(!s||index>3||!isfinite(v))return;if(v<0)v=0;if(v>1)v=1;
    if(index>=2){atomic_store(&s->values[index-2],bits(v));return;}
    if(!index){atomic_store(&s->selected,v>=0.5f);return;}
    unsigned high=v>=0.5f,old=atomic_exchange(&s->apply,high);if(!high||old)return;
    unsigned target=atomic_load(&s->selected);
    if(atomic_load(&s->pending)){
        if(target==atomic_load(&s->active))atomic_store(&s->cancel,atomic_load(&s->token));
        else atomic_store(&s->status,UL_BUSY);return;
    }
    if(target==atomic_load(&s->active)){atomic_store(&s->status,UL_READY);return;}
    unsigned zero=0;if(!atomic_compare_exchange_strong(&s->pending,&zero,2)){atomic_store(&s->status,UL_BUSY);return;}
    atomic_store(&s->target,target);atomic_store(&s->token,atomic_fetch_add(&s->serial,1)+1);atomic_store(&s->pending,1);
}
float ul_get(ul_engine *s,unsigned i){if(!s||i>3)return 0;if(i>=2)return value(atomic_load(&s->values[i-2]));return i?(float)atomic_load(&s->apply):(float)atomic_load(&s->selected);}
void ul_transport(ul_engine *s,int known,int playing){if(s){atomic_store(&s->playing,!!playing);atomic_store(&s->known,!!known);}}
void ul_mains(ul_engine *s,int active){if(s)atomic_store(&s->mains,!!active);}
void ul_config(ul_engine *s,int ok){if(s){atomic_store(&s->config,!!ok);if(!ok)atomic_store(&s->status,UL_CONFIG);}}
void ul_process(ul_engine *s,const int16_t *in,int16_t *out,int frames) {
    if(!out||frames<=0)return;
    memset(out,0,(size_t)frames*2*sizeof *out);
    if(!s||frames!=128||!in)return;
    if(!claim(s))return;
    if(!atomic_load(&s->mains)||!atomic_load(&s->config)||atomic_load(&s->closing)){unclaim(s);return;}
    unsigned p=atomic_load(&s->phase),token=atomic_load(&s->token);
    int stopped=atomic_load(&s->known)&&!atomic_load(&s->playing);
    if(p==P_READY){
        if(cancelled(s,s->ready_token)&&s->ready_slot!=atomic_load(&s->active)){
            atomic_store(&s->rollback,1);atomic_store(&s->phase,P_WORK);unclaim(s);return;
        }
        if(atomic_load(&s->active)==2||stopped){activate(s);p=atomic_load(&s->phase);}
        else {atomic_store(&s->status,UL_WAIT_STOP);unclaim(s);return;}
    }
    if(p==P_FAIL){if(atomic_load(&s->prepared)&&stopped)atomic_store(&s->phase,P_WORK);unclaim(s);return;}
    if(p!=P_RUN&&p!=P_OUT&&p!=P_IN){unclaim(s);return;}
    if(p==P_RUN&&atomic_load(&s->prepared)==token&&stopped&&!cancelled(s,token)){
        p=P_OUT;s->fade_left=512;atomic_store(&s->phase,p);
    }
    if(p==P_OUT&&(!stopped||cancelled(s,token))){p=P_IN;s->fade_left=512;atomic_store(&s->phase,p);}
    pd_enter(s);
    for(unsigned k=0;k<2;k++){uint32_t v=atomic_load(&s->values[k]);if(v!=s->applied_values[k]){libpd_float(k?"ul-mix":"ul-gain",value(v));s->applied_values[k]=v;}}
    int error=libpd_process_short(2,in,out);pd_leave(s);
    if(error||atomic_load(&s->render_fault)){memset(out,0,256*sizeof *out);atomic_store(&s->status,UL_OBJECT);}
    if(p==P_OUT||p==P_IN){
        float target=p==P_OUT?0.0f:1.0f;
        for(int k=0;k<128;k++){if(s->fade_left){s->fade+=(target-s->fade)/(float)s->fade_left;--s->fade_left;}
            out[2*k]=(int16_t)(out[2*k]*s->fade);out[2*k+1]=(int16_t)(out[2*k+1]*s->fade);}
        if(!s->fade_left){if(p==P_OUT)atomic_store(&s->phase,P_WORK);else atomic_store(&s->phase,P_RUN);}
    }
    unclaim(s);
}
unsigned ul_status(ul_engine *s){if(!s)return UL_INVALID;unsigned status=atomic_load(&s->status);return status==UL_READY&&atomic_load(&s->pending)?UL_LOADING:status;}
unsigned ul_active_slot(ul_engine *s){return s?atomic_load(&s->active):2;}
void ul_display(ul_engine *s,char *out,size_t cap){static const char *names[]={"INIT","READY","WAIT STOP","LOADING","BUSY","E MISSING","E INVALID","E OBJECT","E CONFIG","E ROLLBACK","CANCELLED"};
    unsigned c=ul_status(s);if(c>UL_CANCELLED)c=UL_INVALID;snprintf(out,cap,"%s %c>%c",names[c],(char)(ul_active_slot(s)<2?'A'+ul_active_slot(s):'-'),(char)('A'+(s?atomic_load(&s->selected):0)));}
int ul_save(ul_engine *s,char *out,size_t cap) {
    if(!s||!out||cap<5)return 0;
    for(int tries=0;tries<2;tries++){
        unsigned seq=atomic_load(&s->record_seq);if(seq&1)continue;
        unsigned slot=atomic_load(&s->active);char b[96];for(unsigned k=0;k<24;k++){uint32_t v=atomic_load(&s->record[k]);memcpy(b+4*k,&v,4);}
        uint32_t gain=atomic_load(&s->values[0]),mix=atomic_load(&s->values[1]);
        if(seq!=atomic_load(&s->record_seq))continue;
        int n=slot>1?snprintf(out,cap,"UL0"):snprintf(out,cap,"UL1 %s %s %08x %08x",b,b+64,gain,mix);
        return n>0&&(size_t)n<cap?n+1:0;
    }return 0;
}
static int hexword(const char *s,uint32_t *v){uint32_t n=0;for(int k=0;k<8;k++){unsigned char c=(unsigned char)s[k];unsigned z=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:16;if(z==16)return 0;n=(n<<4)|z;}*v=n;return 1;}
static int parse_state(const void *data,size_t n,saved_state *state) {
    if(!data||n<4||n>255)return 0;const char *b=data;if(b[n-1]||memchr(b,0,n-1))return 0;
    memset(state,0,sizeof *state);if(n==4&&!memcmp(b,"UL0",4)){state->none=1;return 1;}
    if(n<25||memcmp(b,"UL1 ",4))return 0;
    const char *a=b+4,*p=memchr(a,' ',n-4);if(!p||(size_t)(p-a)>=UL_ID)return 0;memcpy(state->id,a,(size_t)(p-a));
    a=p+1;p=memchr(a,' ',(size_t)(b+n-a));if(!p||(size_t)(p-a)>=UL_REV)return 0;memcpy(state->revision,a,(size_t)(p-a));
    a=p+1;if(b+n-1-a!=17||a[8]!=' '||!hexword(a,&state->values[0])||!hexword(a+9,&state->values[1]))return 0;
    if(!ul_identifier(state->id,UL_ID)||!ul_identifier(state->revision,UL_REV))return 0;
    for(int k=0;k<2;k++){float v=value(state->values[k]);if(!isfinite(v)||v<0||v>1)return 0;}return 1;
}
int ul_restore(ul_engine *s,const void *data,size_t n) {
    saved_state state;if(!s||!parse_state(data,n,&state))return 0;
    for(int i=0;i<4;i++){unsigned zero=0;if(atomic_compare_exchange_strong(&s->restores[i].state,&zero,1)){
        s->restores[i].value=state;s->restores[i].sequence=atomic_fetch_add(&s->serial,1)+1;atomic_store(&s->restores[i].state,2);
        if(atomic_load(&s->pending)==1)atomic_store(&s->cancel,atomic_load(&s->token));return 1;}}
    return 0;
}
#ifdef UL_TESTING
void ul_test_pause(ul_engine *s,int pre,int load){atomic_store(&s->pause_preflight,!!pre);atomic_store(&s->pause_load,!!load);}
unsigned ul_test_phase(ul_engine *s){return atomic_load(&s->phase);}
unsigned ul_test_pd_overlap(ul_engine *s){return atomic_load(&s->overlap);}
#endif
