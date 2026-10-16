/* New offline test harness. Runs only the two newly authored fixture graphs.
 * adc~/dac~ are test adapters, not additions to the patch admission profile. */
#include "z_libpd.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SR 44100
#define N 64
static float in[N*2],out[N*2];
static void controls(float gain,float mix){assert(!libpd_float("ul-gain",gain));assert(!libpd_float("ul-mix",mix));memset(in,0,sizeof in);for(int i=0;i<64;i++)assert(!libpd_process_float(1,in,out));}
static double run(float gain,float mix,int filtered,int impulse,int right){
 controls(gain,mix);double last[2]={0,0},err=0;const double a=900.0*2*3.14159/SR;
 for(int block=0;block<180;block++){
  for(int k=0;k<N;k++){int t=block*N+k;in[2*k]=right?0:(impulse?(t==0?0.4f:0):0.2f*sinf((float)(2*3.141592653589793*2300*t/SR)));in[2*k+1]=right?(impulse?(t==0?0.4f:0):0.2f*sinf((float)(2*3.141592653589793*137*t/SR))):0;}
  assert(!libpd_process_float(1,in,out));
  for(int k=0;k<N;k++)for(int c=0;c<2;c++){
   double dry=in[2*k+c];last[c]=a*dry+(1-a)*last[c];double processed=filtered?last[c]:dry;
   double expected=dry+mix*(gain*processed-dry);double e=fabs(out[2*k+c]-expected);if(e>err)err=e;
   assert(isfinite(out[2*k+c]));assert(e<2e-5);
  }
 }
 return err;
}
static double response(double frequency){
 controls(1,1);double energyIn=0,energyOut=0;
 for(int b=0;b<400;b++){
  for(int k=0;k<N;k++){int t=b*N+k;in[2*k]=0.25f*sinf((float)(2*3.141592653589793*frequency*t/SR));in[2*k+1]=0;}
  assert(!libpd_process_float(1,in,out));
  if(b>40)for(int k=0;k<N;k++){energyIn+=in[2*k]*in[2*k];energyOut+=out[2*k]*out[2*k];assert(fabs(out[2*k+1])<1e-6);}
 }
 return sqrt(energyOut/energyIn);
}
int main(int argc,char **argv){
 assert(argc==4);int filtered=atoi(argv[3]);assert(!libpd_init());assert(!libpd_init_audio(2,2,SR));
 libpd_add_to_search_path(argv[1]);void *p=libpd_openfile("fixture-host.pd",argv[2]);assert(p);
 assert(!libpd_start_message(1));libpd_add_float(1);assert(!libpd_finish_message("pd","dsp"));
 double maxerr=0;int count=0;
 for(int g=0;g<3;g++)for(int m=0;m<3;m++)for(int imp=0;imp<2;imp++)for(int right=0;right<2;right++){
  double e=run(g/2.0f,m/2.0f,filtered,imp,right);if(e>maxerr)maxerr=e;count++;
 }
 double bass=response(100),treble=response(9000);
 if(filtered){assert(bass>0.98&&bass<1.01);assert(treble<0.13&&treble>0.09);}else{assert(fabs(bass-1)<1e-5);assert(fabs(treble-1)<1e-5);}
 printf("PASS %s: %d gain/mix/sine/impulse/LR cases; max_abs_error=%.9g; response100Hz=%.6f response9000Hz=%.6f\n",filtered?"Stereo Lowpass":"Stereo Gain/Mix",count,maxerr,bass,treble);
 libpd_closefile(p);return 0;
}
