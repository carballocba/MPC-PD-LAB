/* Isolated native UI experiment. No Pd, file access, transport, state or loader.
 * ABI declarations derive from audited v0.2 sd88me-based bridge (see REPORT).
 */
#include "abi.h"
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <stdio.h>
#include <math.h>
_Static_assert(sizeof(float)==4 && sizeof(unsigned)==4,"32-bit normalized control representation");
_Static_assert(ATOMIC_INT_LOCK_FREE==2,"probe controls require lock-free uint atomics");
enum { VIEW,SELECT,MOCKPAGE,APPLY,REFRESH,STATUS,ROW0,ROW1,ROW2,ROW3,CONTROL0,SELECTED=26,COUNT=27 };
#define PROBE_UID 0x50445569 /* PDUi: experiment only, not PD LAB product identity */
typedef struct { AEffect fx; atomic_uint values[COUNT],tick,requests; } Probe;
static unsigned encode(float f){unsigned u;memcpy(&u,&f,4);return u;}
static float decode(unsigned u){float f;memcpy(&f,&u,4);return f;}
static unsigned index_value(float f,unsigned n){return (unsigned)(f*(float)(n-1)+0.5f);}
static float get(AEffect *f,int32_t i){
 Probe *p=f->object;if(i<0||i>=COUNT)return 0;
 if(i>=ROW0&&i<=ROW3)return index_value(decode(atomic_load(&p->values[SELECT])),4)==(unsigned)(i-ROW0)?1:0;
 if(i==STATUS||i==SELECTED)return 0;
 return decode(atomic_load(&p->values[i]));
}
static void set(AEffect *f,int32_t i,float v){
 Probe *p=f->object;if(i<0||i>=COUNT||i==STATUS||i==SELECTED||!isfinite(v))return;
 if(v<0)v=0;if(v>1)v=1;
 if(i>=ROW0&&i<=ROW3){if(v>=0.5f)atomic_store(&p->values[SELECT],encode((i-ROW0)/3.0f));return;}
 float old=decode(atomic_exchange(&p->values[i],encode(v)));
 if(old<0.5f&&v>=0.5f){if(i==APPLY)atomic_fetch_add(&p->requests,1);if(i==REFRESH)atomic_fetch_add(&p->tick,1);}
}
static void copy8(void *out,const char *s){if(out){size_t n=strlen(s);if(n>7)n=7;memcpy(out,s,n);((char*)out)[n]=0;}}
static const char *label(Probe *p,unsigned row){static const char *labels[2][4]={{"Trim","Lowpass","Mock 3","Mock 4"},{"Alt 1","Alt 2","Alt 3","Alt 4"}};return labels[index_value(decode(atomic_load(&p->values[MOCKPAGE])),2)][row];}
static intptr_t dispatch(AEffect *f,int32_t op,int32_t i,intptr_t v,void *out,float opt){
 Probe *p=f->object;char text[32];(void)v;(void)opt;
 switch(op){
 case 0:return 1;case 1:free(p);return 1;
 case 7:
  if(i<0||i>=COUNT)return 0;
  if(i==STATUS)snprintf(text,sizeof text,"M%03uR%u",atomic_load(&p->tick)%1000,atomic_load(&p->requests)%10);
  else if(i>=ROW0&&i<=ROW3)snprintf(text,sizeof text,"%s",label(p,i-ROW0));
  else if(i==SELECTED)snprintf(text,sizeof text,"%s",label(p,index_value(get(f,SELECT),4)));
  else if(i==VIEW)snprintf(text,sizeof text,"%s",get(f,VIEW)<0.5f?"Library":"Perform");
  else if(i==APPLY||i==REFRESH)snprintf(text,sizeof text,"%s",get(f,i)<0.5f?"Ready":"Rearm");
  else snprintf(text,sizeof text,"%.3f",get(f,i));copy8(out,text);return 1;
 case 8:if(i<0||i>=COUNT)return 0;snprintf(text,sizeof text,"P%02d",i);copy8(out,text);return 1;
 case 6:copy8(out,"");return 1;
 case 26:return i>=CONTROL0&&i<SELECTED;
 case 35:return 1;case 45:case 48:if(out)strcpy(out,"PD LAB");return 1;
 case 47:if(out)strcpy(out,"CARBALLO");return 1;
 case 49:return 1;case 58:return 2400;
 default:return 0;
 }
}
/* Callback intentionally has no atomics, host calls, UI polling, allocation,
 * locks, logging or filesystem. View navigation cannot change these samples. */
static void render(AEffect *fx,float **in,float **out,int32_t n){(void)fx;
 if(n<=0||!out)return;
 for(int c=0;c<2;c++)if(out[c])for(int k=0;k<n;k++)out[c][k]=in&&in[c]?in[c][k]:0;
}
__attribute__((visibility("default"))) AEffect *VSTPluginMain(master_fn host){
 (void)host;Probe *p=calloc(1,sizeof *p);if(!p)return NULL;
 for(int i=0;i<COUNT;i++)atomic_init(&p->values[i],encode(0));atomic_init(&p->tick,0);atomic_init(&p->requests,0);
 AEffect *f=&p->fx;f->magic=0x56737450;f->dispatcher=dispatch;f->setParameter=set;f->getParameter=get;
 f->numPrograms=1;f->numParams=COUNT;f->numInputs=f->numOutputs=2;f->flags=1<<4;
 f->uniqueID=PROBE_UID;f->version=1;f->processReplacing=render;f->object=p;return f;
}
