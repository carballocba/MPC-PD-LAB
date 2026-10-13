/* Organelle protocol bridge. Original DSP lives in the upstream port directory.
 * 44.1kHz, 128 frames, one active instance per loaded module.
 * Host serializes engine calls (existing wrapper contract). */
#include "engine.h"
#include "z_libpd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct { const char *key, *receiver; float initial; int toggle; } control_map;
#include "organelle_map.h"
typedef struct {
    void *patch, *bindings[6];
    float values[NCONTROLS];
    int failed, led, fs;
} instance;
static instance *current;
static int initialized;
static pthread_mutex_t lifecycle = PTHREAD_MUTEX_INITIALIZER;
static char diagnostics[8192];
static size_t used;
/* Never print/write files from Pd's callback, including after initialization.
 * Diagnostic capture is bounded and allocation-free; caller can read it later. */
static void pd_print(const char *text) {
    size_t n = strlen(text), room = sizeof diagnostics - used - 1;
    if (n > room) n = room;
    memcpy(diagnostics + used, text, n); used += n; diagnostics[used] = 0;
    if (current && (strstr(diagnostics, "couldn't create") || strstr(diagnostics, "error:") || used == sizeof diagnostics-1))
        current->failed = 1;
}
static void capture_float(const char *receiver, float value) {
    if (current && !strcmp(receiver,"led")) current->led = (int)value;
}
static void release(instance *s) {
    if (s->patch) libpd_closefile(s->patch);
    for (int i=0;i<6;i++) if (s->bindings[i]) libpd_unbind(s->bindings[i]);
    current = NULL;
    free(s);
}
static void apply(instance *s, int i, float v, int force) {
    if (MAP[i].toggle) {
        v = v >= 0.5f;
        if (v != s->values[i]) {
            libpd_float(MAP[i].receiver, 1);
            libpd_float(MAP[i].receiver, 0);
        }
    } else if (force || v != s->values[i]) libpd_float(MAP[i].receiver, v);
    s->values[i] = v;
}
static void *create(const char *dir) {
    pthread_mutex_lock(&lifecycle);
    instance *s=NULL;
    if (current || !dir) goto done;
    used=0; diagnostics[0]=0;
    for (size_t i=0;i<sizeof REQUIRED/sizeof REQUIRED[0];i++) {
        char path[1400]; struct stat st;
        int n=snprintf(path,sizeof path,"%s/%s",dir,REQUIRED[i]);
        if(n<0 || n>=(int)sizeof path || stat(path,&st) || !S_ISREG(st.st_mode) || access(path,R_OK)) {
            snprintf(diagnostics,sizeof diagnostics,"missing runtime file: %s",REQUIRED[i]);
            goto done;
        }
    }
    libpd_set_printhook(pd_print);
    libpd_set_floathook(capture_float);
    if (!initialized) { if(libpd_init()) goto done; initialized=1; }
    if(libpd_blocksize()!=64 || libpd_init_audio(2,2,44100)) goto done;
    s=calloc(1,sizeof *s);
    if(!s) goto done;
    current=s;
    const char *names[]={"screenLine1","screenLine2","screenLine3","screenLine4","screenLine5","led"};
    for(int i=0;i<6;i++) { s->bindings[i]=libpd_bind(names[i]); if(!s->bindings[i]) s->failed=1; }
    libpd_clear_search_path();
    libpd_add_to_search_path(dir);
    s->patch=libpd_openfile("host.pd",dir);
    if(!s->patch || s->failed) { release(s); s=NULL; goto done; }
    /* Native loadbang has already run; now publish host defaults and ready. */
    for(int i=0;i<NCONTROLS;i++) apply(s,i,MAP[i].initial,1);
    libpd_float("mpc-exp",0); libpd_float("mpc-fs",0); libpd_bang("mpc-ready");
    libpd_start_message(1); libpd_add_float(1); libpd_finish_message("pd","dsp");
    if(s->failed) { release(s); s=NULL; }
done:
    if(!s && !current) fprintf(stderr,"Organelle initialization rejected: %s\n",diagnostics);
    pthread_mutex_unlock(&lifecycle);
    return s;
}
static void destroy(void *p) {
    if(!p) return;
    pthread_mutex_lock(&lifecycle);
    release(p);
    pthread_mutex_unlock(&lifecycle);
}
static void set(void *p,const char *key,const char *value) {
    instance *s=p;
    if(!s || !key || !value || s->failed) return;
    if(!strcmp(key,"state")) {
        float v[NCONTROLS]; int n=0;
        if(sscanf(value,"v1 %f %f %f %f %f %f %n",v,v+1,v+2,v+3,v+4,v+5,&n)!=6 || !n || value[n]) return;
        for(int i=0;i<NCONTROLS;i++) if(!isfinite(v[i]) || v[i]<0 || v[i]>1) return;
        for(int i=0;i<NCONTROLS;i++) apply(s,i,v[i],0);
        return;
    }
    char *end; float v=strtof(value,&end);
    if(end==value || *end || !isfinite(v)) return;
    if(v<0) v=0;
    if(v>1) v=1;
    for(int i=0;i<NCONTROLS;i++) if(!strcmp(key,MAP[i].key)) { apply(s,i,v,0); return; }
}
static int get(void *p,const char *key,char *buf,int len) {
    instance *s=p;
    if(!s || !key || !buf || len<=0) return 0;
    if(!strcmp(key,"state")) return snprintf(buf,(size_t)len,"v1 %.9g %.9g %.9g %.9g %.9g %.9g",s->values[0],s->values[1],s->values[2],s->values[3],s->values[4],s->values[5]);
    if(!strcmp(key,"diagnostic")) return snprintf(buf,(size_t)len,"%s",diagnostics);
    if(!strcmp(key,"led")) return snprintf(buf,(size_t)len,"%d",s->led);
    for(int i=0;i<NCONTROLS;i++) if(!strcmp(key,MAP[i].key)) return snprintf(buf,(size_t)len,"%.9g",s->values[i]);
    return 0;
}
static void midi(void *p,const uint8_t *msg,int len) {
    instance *s=p;
    if(!s || s->failed || !msg || len<3 || msg[1]>127 || msg[2]>127) return;
    int type=msg[0]&0xf0, ch=msg[0]&15;
    if(type==0x90 || type==0x80) {
        int vel=type==0x80?0:msg[2];
        libpd_noteon(ch,msg[1],vel);
        libpd_start_message(2); libpd_add_float(msg[1]); libpd_add_float(vel); libpd_finish_list("mpc-notes");
    } else if(type==0xb0) {
        libpd_controlchange(ch,msg[1],msg[2]);
        for(int i=0;i<NCONTROLS;i++) if(msg[1]==MIDI_CC[i]) apply(s,i,msg[2]/127.0f,0);
        if(msg[1]==11) libpd_float("mpc-exp",msg[2]/127.0f);
        if(msg[1]==64) {
            int pressed=msg[2]>=64;
            if(pressed!=s->fs) {
                libpd_float("mpc-fs",pressed);
                if(pressed) for(int i=0;i<NCONTROLS;i++)
                    if(MAP[i].toggle && !strcmp(MAP[i].key,"freeze")) s->values[i]=1-s->values[i];
                s->fs=pressed;
            }
        }
    }
}
static void process(void *p,const int16_t *in,int16_t *out,int frames) {
    if(frames<=0 || !out) return;
    instance *s=p;
    if(!s || s->failed || !in || frames!=128) { memset(out,0,(size_t)frames*2*sizeof *out); return; }
    if(libpd_process_short(2,in,out) || s->failed) memset(out,0,(size_t)frames*2*sizeof *out);
}
static const mpc_engine_t API={create,destroy,midi,set,get,NULL,process,NULL};
const mpc_engine_t *mpc_engine(void) { return &API; }
/* Mach-O needs a definition for the wrapper's optional weak symbol too. */
int mpc_engine_changed(void *p,char *key,int len) { (void)p; (void)key; (void)len; return 0; }
