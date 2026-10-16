/* Read-only compatibility check using unchanged current runtime validator. */
#include "manifest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){assert(argc==2);ul_patch_spec s;memset(&s,0,sizeof s);s.count=1;strcpy(s.files[0],"main.pd");
 char name[2048],text[131073];snprintf(name,sizeof name,"%s/main.pd",argv[1]);FILE *f=fopen(name,"rb");assert(f);size_t n=fread(text,1,131072,f);assert(feof(f));fclose(f);text[n]=0;
 assert(ul_audit_pd(text,n,&s));assert(ul_audit_closure(argv[1],&s));puts("PASS unchanged runtime Pd object and closure audit");return 0;}
