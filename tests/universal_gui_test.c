/* Exercise native skin parameter contracts using the same VST host ABI. */
#define main legacy_suite_main
#include "universal_vst_test.c"
#undef main
static void display(AEffect *e,int i,char b[8]){memset(b,0,8);CHECK(e->dispatcher(e,7,i,0,b,0));CHECK(memchr(b,0,8));}
static void assert_text(AEffect *e,int i,const char *expected){char b[8];display(e,i,b);if(strcmp(b,expected))fprintf(stderr,"GUI %d got '%s', expected '%s'\n",i,b,expected);CHECK(!strcmp(b,expected));}
int main(int argc,char **argv){CHECK(argc==2);void *lib=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL
#ifdef __APPLE__
 |RTLD_NODELETE
#endif
 );CHECK(lib);AEffect*(*entry)(Master);void *sym=dlsym(lib,"VSTPluginMain");memcpy(&entry,&sym,sizeof entry);known=1;ti.flags=0;AEffect *e=entry(master);CHECK(e&&e->numParams==9);
 for(int k=0;k<128;k++)left[k]=0.25f;CHECK(wait_patch(e,"stereo-trim"));assert_text(e,4,"Trim");assert_text(e,5,"READY");assert_text(e,6,"stereo-");assert_text(e,7,"trim");assert_text(e,8,"1");
 for(int i=4;i<9;i++){CHECK(!e->dispatcher(e,26,i,0,NULL,0));float v=e->getParameter(e,i);e->setParameter(e,i,1);CHECK(e->getParameter(e,i)==v);}
 /* Browser highlights and Q-Links share the original normalized parameter. */
 e->setParameter(e,0,1);CHECK(e->getParameter(e,0)==1);assert_text(e,4,"Trim");
 ti.flags=2;e->setParameter(e,1,1);for(int i=0;i<60;i++){tick(e);nap();}assert_text(e,5,"WAIT");assert_text(e,4,"Trim");
 ti.flags=0;CHECK(wait_patch(e,"stereo-lowpass"));assert_text(e,4,"Lowpass");assert_text(e,7,"lowpass");
 e->setParameter(e,2,0.42f);e->setParameter(e,3,0.25f);char state[256];int n=chunk(e,state);CHECK(n>0);
 /* A held Apply remains edge-triggered; REARM is explicitly required. */
 e->setParameter(e,0,0);e->setParameter(e,1,1);for(int i=0;i<30;i++){tick(e);nap();}assert_text(e,4,"Lowpass");
 e->setParameter(e,1,0);e->setParameter(e,1,1);CHECK(wait_patch(e,"stereo-trim"));CHECK(e->dispatcher(e,24,0,n,state,0));CHECK(wait_patch(e,"stereo-lowpass"));assert_text(e,4,"Lowpass");CHECK(e->getParameter(e,2)==0.42f&&e->getParameter(e,3)==0.25f);
 char file[1600],saved[1620];CHECK(strlen(argv[1])<1400);strcpy(file,argv[1]);char *slash=strrchr(file,'/');CHECK(slash);*slash=0;strcat(file,"/data/user/patches/trim/main.pd");snprintf(saved,sizeof saved,"%s.gui-test-saved",file);CHECK(!rename(file,saved));
 select_patch(e,0);for(int i=0;i<100;i++){tick(e);nap();}assert_text(e,5,"MISSING");assert_text(e,4,"Lowpass");CHECK(!rename(saved,file));select_patch(e,0);CHECK(wait_patch(e,"stereo-trim"));assert_text(e,5,"READY");
 /* Invalid Pd activation rolls back; GUI must report the retained graph. */
 select_patch(e,1);CHECK(wait_patch(e,"stereo-lowpass"));CHECK(!rename(file,saved));
 FILE *original=fopen(saved,"rb"),*broken=fopen(file,"wb");CHECK(original&&broken);char bytes[4096];size_t count;while((count=fread(bytes,1,sizeof bytes,original)))CHECK(fwrite(bytes,1,count,broken)==count);fputs("#X connect 0 0 99 0;\n",broken);fclose(original);fclose(broken);
 select_patch(e,0);for(int i=0;i<150;i++){tick(e);nap();}assert_text(e,5,"BAD OBJ");assert_text(e,4,"Lowpass");CHECK(!remove(file));CHECK(!rename(saved,file));select_patch(e,0);CHECK(wait_patch(e,"stereo-trim"));
 e->dispatcher(e,10,0,0,NULL,48000);assert_text(e,5,"CONFIG");tick(e);CHECK(ol[100]==0);e->dispatcher(e,10,0,0,NULL,44100);select_patch(e,0);CHECK(wait_patch(e,"stereo-trim"));
 e->dispatcher(e,1,0,0,NULL,0);dlclose(lib);puts("PASS native GUI contract: selected/active split, readonly telemetry, Q-Link readback, Apply/rearm, transport, project recall and error recovery");return 0;}
