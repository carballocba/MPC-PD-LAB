#include "abi.h"
#include <assert.h>
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){
 assert(argc==2);void *module=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);if(!module){puts(dlerror());return 1;}
 AEffect *(*entry)(master_fn);*(void **)(&entry)=dlsym(module,"VSTPluginMain");assert(entry);
 AEffect *f=entry(NULL);assert(f&&f->magic==0x56737450&&f->uniqueID==0x50445569&&f->numParams==27);
 assert(f->numInputs==2&&f->numOutputs==2);
 float l[128],r[128],ol[128],orr[128];float *in[]={l,r},*out[]={ol,orr};
 for(int k=0;k<128;k++){l[k]=k/256.0f;r[k]=-l[k];}
 for(int j=0;j<1000;j++){
  f->setParameter(f,0,(j%2)?1:0);f->setParameter(f,1,(j%4)/3.0f);f->setParameter(f,2,(j%2)?1:0);
  f->processReplacing(f,in,out,128);
  assert(!memcmp(l,ol,sizeof l)&&!memcmp(r,orr,sizeof r));
 }
 char buf[16];memset(buf,0x55,sizeof buf);f->dispatcher(f,7,6,0,buf,0);assert(!strcmp(buf,"Alt 1"));
 for(int k=8;k<16;k++)assert(buf[k]==0x55);
 f->setParameter(f,6,1);assert(f->getParameter(f,1)==0);
 f->setParameter(f,9,1);assert(f->getParameter(f,1)==1);
 f->setParameter(f,9,0);assert(f->getParameter(f,1)==1);
 f->setParameter(f,3,1);f->setParameter(f,3,1);f->setParameter(f,4,1);
 f->dispatcher(f,7,5,0,buf,0);assert(!strcmp(buf,"M001R1"));
 f->setParameter(f,3,0);f->setParameter(f,3,1);f->dispatcher(f,7,5,0,buf,0);assert(!strcmp(buf,"M001R2"));
 f->setParameter(f,5,1);f->setParameter(f,26,1);f->setParameter(f,10,NAN);assert(f->getParameter(f,10)==0);
 for(int i=10;i<26;i++){f->setParameter(f,i,(i-10)/15.0f);assert(f->getParameter(f,i)==(i-10)/15.0f);assert(f->dispatcher(f,26,i,0,NULL,0)==1);}
 f->setParameter(f,0,0);for(int i=10;i<26;i++)assert(f->getParameter(f,i)==(i-10)/15.0f);
 f->processReplacing(f,NULL,out,128);for(int i=0;i<128;i++)assert(ol[i]==0&&orr[i]==0);
 for(int i=0;i<27;i++){memset(buf,0x55,sizeof buf);f->dispatcher(f,7,i,0,buf,0);assert(memchr(buf,0,8));for(int j=8;j<16;j++)assert(buf[j]==0x55);}
 f->dispatcher(f,1,0,0,NULL,0);dlclose(module);
 puts("PASS: dynamic binary ABI/UID/count; 1000 mock view transitions with bit-exact stereo pass-through; row selection; bounded text/canaries; mock Apply rearm; refresh; readonly fields; 16 fixed slots; null-input silence. No MPC host test.");
}
