#define _GNU_SOURCE
#include "manifest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc,char **argv){assert(argc==2);char file[1200];ul_patch_spec spec;ul_catalog_entry c[2];
 snprintf(file,sizeof file,"%s/user/catalog.json",argv[1]);assert(ul_catalog_read(file,c));assert(strcmp(c[0].id,c[1].id));
 snprintf(file,sizeof file,"%s/user/patches/trim/manifest.json",argv[1]);assert(ul_manifest_read(file,"stereo-trim",&spec));assert(!ul_manifest_read(file,"wrong",&spec));
 assert(!ul_relative("../main.pd"));assert(!ul_relative("/main.pd"));assert(!ul_relative("a//b"));assert(!ul_relative("a/./b"));assert(!ul_relative("a/"));assert(ul_relative("a/main.pd"));
 char source[16385];FILE *f=fopen(file,"rb");assert(f);size_t n=fread(source,1,16384,f);fclose(f);source[n]=0;
 char temp[]="/tmp/ul-manifest-XXXXXX";int fd=mkstemp(temp);assert(fd>=0);close(fd);
 const char *bad[]={"{\"schema\":1,\"schema\":1}","{} {}","{\"schema\":nan}","{\"schema\":01}","{\"patches\":[]}","{\"schema\":1,\"patches\":[{},{}]}","{\"schema\":1,\"patches\":[]}"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){f=fopen(temp,"wb");assert(f);fputs(bad[i],f);fclose(f);assert(!ul_catalog_read(temp,c));assert(!ul_manifest_read(temp,"stereo-trim",&spec));}
 const char *badpd="#N canvas 0 0 1 1 10;\n#X obj 0 0 until;\n";assert(!ul_audit_pd(badpd,strlen(badpd),&spec));
 unlink(temp);puts("PASS manifest/catalog bounds, schema, duplicate/malformed input and traversal rejection");return 0;}
