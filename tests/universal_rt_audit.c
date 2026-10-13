/* Linux link-time interposition: callback and setters must not invoke these. */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <pthread.h>
static _Thread_local int in_callback;
static void forbidden(const char *name){if(in_callback){fprintf(stderr,"REALTIME VIOLATION: %s\n",name);abort();}}
void ul_rt_enter(void){in_callback=1;}
void ul_rt_leave(void){in_callback=0;}
void *__real_malloc(size_t);void *__real_calloc(size_t,size_t);void *__real_realloc(void*,size_t);void __real_free(void*);
void *__wrap_malloc(size_t n){forbidden("malloc");return __real_malloc(n);}
void *__wrap_calloc(size_t n,size_t z){forbidden("calloc");return __real_calloc(n,z);}
void *__wrap_realloc(void *p,size_t n){forbidden("realloc");return __real_realloc(p,n);}
void __wrap_free(void *p){forbidden("free");__real_free(p);}
int __real_open(const char*,int,...);int __wrap_open(const char *p,int f,...){forbidden("open");if(f&O_CREAT){va_list a;va_start(a,f);int m=va_arg(a,int);va_end(a);return __real_open(p,f,m);}return __real_open(p,f);}
FILE *__real_fopen(const char*,const char*);FILE *__wrap_fopen(const char*p,const char*m){forbidden("fopen");return __real_fopen(p,m);}
int __real_pthread_mutex_lock(pthread_mutex_t*);int __wrap_pthread_mutex_lock(pthread_mutex_t*p){forbidden("mutex lock");return __real_pthread_mutex_lock(p);}
int __real_pthread_join(pthread_t,void**);int __wrap_pthread_join(pthread_t p,void**r){forbidden("join");return __real_pthread_join(p,r);}
