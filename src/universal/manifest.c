/* SPDX-License-Identifier: MIT. Bounded, worker-only schema validation around jsmn. */
#define _POSIX_C_SOURCE 200809L
#include "manifest.h"
#define JSMN_STATIC
#define JSMN_STRICT
#include "../../vendor/universal/jsmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

typedef struct { char text[16385]; jsmntok_t tokens[512]; int count; } document;
static int end_token(const document *d, int i) {
    int e=d->tokens[i].end; ++i;
    while(i<d->count && d->tokens[i].start<e) ++i;
    return i;
}
static int equal(const document *d,int i,const char *s) {
    return i>=0 && d->tokens[i].type==JSMN_STRING &&
        (int)strlen(s)==d->tokens[i].end-d->tokens[i].start &&
        !memcmp(d->text+d->tokens[i].start,s,strlen(s));
}
static int string(const document *d,int i,char *out,size_t cap) {
    if(i<0 || d->tokens[i].type!=JSMN_STRING) return 0;
    int n=d->tokens[i].end-d->tokens[i].start;
    if(n<1 || (size_t)n>=cap) return 0;
    for(int k=0;k<n;k++) { unsigned char c=(unsigned char)d->text[d->tokens[i].start+k];
        if(c<32 || c>126 || c=='\\') return 0; }
    memcpy(out,d->text+d->tokens[i].start,(size_t)n);out[n]=0;return 1;
}
static int number(const document *d,int i,double *value) {
    if(i<0 || d->tokens[i].type!=JSMN_PRIMITIVE) return 0;
    int n=d->tokens[i].end-d->tokens[i].start;char b[64];
    if(n<1 || n>40) return 0;
    memcpy(b,d->text+d->tokens[i].start,(size_t)n);b[n]=0;
    /* Deliberately small decimal grammar; no hex, booleans, NaN or locale escape. */
    int k=0;if(b[k]=='-')++k;
    if(!isdigit((unsigned char)b[k]))return 0;
    if(b[k]=='0' && isdigit((unsigned char)b[k+1]))return 0;
    while(isdigit((unsigned char)b[k]))++k;
    if(b[k]=='.'){++k;if(!isdigit((unsigned char)b[k]))return 0;while(isdigit((unsigned char)b[k]))++k;}
    if(b[k])return 0;
    char *e;double v=strtod(b,&e);if(*e || !isfinite(v))return 0;*value=v;return 1;
}
static int exact(const document *d,int i,double v){double x;return number(d,i,&x)&&x==v;}
static int object_valid(const document *d,int i,const char *const *keys,unsigned count) {
    if(i<0 || d->tokens[i].type!=JSMN_OBJECT)return 0;
    unsigned seen=0;int j=i+1,limit=end_token(d,i);
    while(j<limit) {
        if(d->tokens[j].type!=JSMN_STRING || j+1>=limit)return 0;
        unsigned k;for(k=0;k<count;k++)if(equal(d,j,keys[k]))break;
        if(k==count || (seen&(1u<<k)))return 0;
        seen|=1u<<k;j=end_token(d,j+1);
    }
    return seen==((1u<<count)-1u);
}
static int member(const document *d,int i,const char *key) {
    if(i<0)return -1;int limit=end_token(d,i);
    for(int j=i+1;j<limit;j=end_token(d,j+1))if(equal(d,j,key))return j+1;
    return -1;
}
static int read_document(const char *path,document *d) {
    FILE *f=fopen(path,"rb");if(!f)return 0;
    size_t n=fread(d->text,1,sizeof d->text,f);int ok=!ferror(f)&&feof(f)&&n<=16384;
    fclose(f);if(!ok || !n || memchr(d->text,0,n))return 0;
    d->text[n]=0;jsmn_parser parser;jsmn_init(&parser);
    d->count=jsmn_parse(&parser,d->text,n,d->tokens,512);
    if(d->count<=0 || d->tokens[0].type!=JSMN_OBJECT || end_token(d,0)!=d->count)return 0;
    for(int k=0;k<d->tokens[0].start;k++)if(!isspace((unsigned char)d->text[k]))return 0;
    for(size_t k=(size_t)d->tokens[0].end;k<n;k++)if(!isspace((unsigned char)d->text[k]))return 0;
    return 1;
}
static int ascii_alnum(unsigned char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9');}
int ul_identifier(const char *s,size_t max) {
    size_t n=strnlen(s,max);if(!n||n>=max)return 0;
    for(size_t k=0;k<n;k++)if(!(ascii_alnum((unsigned char)s[k])||s[k]=='-'||s[k]=='_'))return 0;
    return 1;
}
int ul_relative(const char *s) {
    if(!s||!*s||*s=='/'||strlen(s)>=UL_PATH)return 0;
    const char *p=s;
    while(*p){const char *q=strchr(p,'/');size_t n=q?(size_t)(q-p):strlen(p);
        if(!n || (n==1&&p[0]=='.') || (n==2&&p[0]=='.'&&p[1]=='.'))return 0;
        for(size_t k=0;k<n;k++)if(!(ascii_alnum((unsigned char)p[k])||p[k]=='-'||p[k]=='_'||p[k]=='.'))return 0;
        if(!q)break;p=q+1;if(!*p)return 0;
    }return 1;
}
int ul_regular_path(const char *root,const char *relative,char out[1024]) {
    if(!ul_relative(relative))return 0;
    int n=snprintf(out,1024,"%s/%s",root,relative);if(n<0||n>=1024)return 0;
    size_t start=strlen(root)+1;struct stat st;
    for(size_t k=start;;k++)if(out[k]=='/'||out[k]==0){char c=out[k];out[k]=0;
        int ok=!lstat(out,&st)&&!S_ISLNK(st.st_mode)&&(c?S_ISDIR(st.st_mode):S_ISREG(st.st_mode));
        out[k]=c;if(!ok)return 0;if(!c)break;
    }return 1;
}
int ul_catalog_read(const char *path,ul_catalog_entry entries[2]) {
    document d;const char *keys[]={"schema","patches"};const char *ek[]={"id","folder"};
    if(!read_document(path,&d)||!object_valid(&d,0,keys,2)||!exact(&d,member(&d,0,"schema"),1))return 0;
    int a=member(&d,0,"patches");if(a<0||d.tokens[a].type!=JSMN_ARRAY||d.tokens[a].size!=2)return 0;
    int j=a+1;for(int k=0;k<2;k++){if(!object_valid(&d,j,ek,2)||!string(&d,member(&d,j,"id"),entries[k].id,UL_ID)||
        !string(&d,member(&d,j,"folder"),entries[k].folder,UL_PATH)||!ul_identifier(entries[k].id,UL_ID)||!ul_relative(entries[k].folder))return 0;
        j=end_token(&d,j);}
    return strcmp(entries[0].id,entries[1].id)!=0 && strcmp(entries[0].folder,entries[1].folder)!=0;
}
int ul_manifest_read(const char *path,const char *expected,ul_patch_spec *s) {
    document d;const char *keys[]={"schema","id","revision","entry","sample_rate","inputs","outputs","required_files","externals","controls"};
    if(!read_document(path,&d)||!object_valid(&d,0,keys,10))return 0;
    memset(s,0,sizeof *s);
    if(!exact(&d,member(&d,0,"schema"),1)||!exact(&d,member(&d,0,"sample_rate"),44100)||
       !exact(&d,member(&d,0,"inputs"),2)||!exact(&d,member(&d,0,"outputs"),2)||
       !equal(&d,member(&d,0,"entry"),"main.pd")||
       !string(&d,member(&d,0,"id"),s->id,UL_ID)||strcmp(s->id,expected)||!ul_identifier(s->id,UL_ID)||
       !string(&d,member(&d,0,"revision"),s->revision,UL_REV)||!ul_identifier(s->revision,UL_REV))return 0;
    int ex=member(&d,0,"externals");if(ex<0||d.tokens[ex].type!=JSMN_ARRAY||d.tokens[ex].size)return 0;
    int a=member(&d,0,"required_files");if(a<0||d.tokens[a].type!=JSMN_ARRAY||d.tokens[a].size<1||d.tokens[a].size>UL_FILES)return 0;
    int main=0,j=a+1;s->count=(unsigned)d.tokens[a].size;
    for(unsigned k=0;k<s->count;k++,j=end_token(&d,j)){
        if(!string(&d,j,s->files[k],UL_PATH)||!ul_relative(s->files[k]))return 0;
        size_t n=strlen(s->files[k]);if(n<4||strcmp(s->files[k]+n-3,".pd")||strchr(s->files[k],'/')||!strcmp(s->files[k],"host.pd"))return 0;
        main|=!strcmp(s->files[k],"main.pd");
        for(unsigned q=0;q<k;q++)if(!strcmp(s->files[q],s->files[k]))return 0;
    }if(!main)return 0;
    a=member(&d,0,"controls");if(a<0||d.tokens[a].type!=JSMN_ARRAY||d.tokens[a].size!=2)return 0;
    const char *ck[]={"slot","key","receiver","min","max","default"};unsigned seen=0;j=a+1;
    for(int k=0;k<2;k++,j=end_token(&d,j)){
        double slot;if(!object_valid(&d,j,ck,6)||!number(&d,member(&d,j,"slot"),&slot)||(slot!=0&&slot!=1))return 0;
        int x=(int)slot;if(seen&(1u<<x))return 0;seen|=1u<<x;
        if(!equal(&d,member(&d,j,"key"),x?"mix":"gain")||!equal(&d,member(&d,j,"receiver"),x?"ul-mix":"ul-gain")||
           !exact(&d,member(&d,j,"min"),0)||!exact(&d,member(&d,j,"max"),1)||!exact(&d,member(&d,j,"default"),1))return 0;
    }return seen==3;
}
int ul_audit_pd(const char *text,size_t n,const ul_patch_spec *s) {
    /* Trusted, intentionally small signal graph subset. No message boxes, network,
       runtime graph edits, file I/O or control-rate loops. Not an untrusted sandbox. */
    const char *allowed[]={"inlet~","outlet~","r","pack","line~","*~","+~","-~","sig~","lop~"};
    size_t start=0;
    for(size_t i=0;i<n;i++)if(text[i]==';'&&(i==0||text[i-1]!='\\')){
        size_t len=i-start;const char *p=text+start;while(len&&isspace((unsigned char)*p)){++p;--len;}
        if(len>=6&&!memcmp(p,"#X obj",6)){
            char obj[UL_ID],arg[UL_ID];obj[0]=arg[0]=0;
            if(sscanf(p,"#X obj %*s %*s %63s %63s",obj,arg)<1)return 0;
            obj[strcspn(obj,",;")]=0;arg[strcspn(arg,",;")]=0;
            int ok=0;for(unsigned k=0;k<sizeof allowed/sizeof *allowed;k++)ok|=!strcmp(obj,allowed[k]);
            if(!ok)for(unsigned k=0;k<s->count;k++){const char *b=strrchr(s->files[k],'/');b=b?b+1:s->files[k];
                size_t z=strlen(b)-3;if(strlen(obj)==z&&!memcmp(obj,b,z))ok=1;}
            if(!ok)return 0;
            if(!strcmp(obj,"r")&&strcmp(arg,"ul-gain")&&strcmp(arg,"ul-mix"))return 0;
        }else if(!((len>=9&&!memcmp(p,"#N canvas",9))||(len>=7&&!memcmp(p,"#X text",7))||
                   (len>=10&&!memcmp(p,"#X connect",10))||!len))return 0;
        start=i+1;
    }
    for(size_t k=start;k<n;k++)if(!isspace((unsigned char)text[k]))return 0;
    return n>0;
}
/* Snapshot files are immutable and small. Reject recursive abstraction creation
   before Pd can recurse on the worker's C stack. No filesystem work on render. */
static int visit(unsigned node,const uint64_t edges[UL_FILES],unsigned char color[UL_FILES],unsigned count) {
    if(color[node]==1)return 0;if(color[node]==2)return 1;color[node]=1;
    for(unsigned k=0;k<count;k++)if((edges[node]&(UINT64_C(1)<<k))&&!visit(k,edges,color,count))return 0;
    color[node]=2;return 1;
}
int ul_audit_closure(const char *root,const ul_patch_spec *s) {
    uint64_t edges[UL_FILES]={0};unsigned char color[UL_FILES]={0};
    for(unsigned k=0;k<s->count;k++){
        char path[1024];if(!ul_regular_path(root,s->files[k],path))return 0;
        FILE *f=fopen(path,"rb");if(!f)return 0;char *b=malloc(131073);if(!b){fclose(f);return 0;}
        size_t n=fread(b,1,131073,f);int ok=!ferror(f)&&feof(f)&&n<=131072;fclose(f);if(!ok){free(b);return 0;}b[n]=0;
        size_t start=0;
        for(size_t i=0;i<n;i++)if(b[i]==';'&&(i==0||b[i-1]!='\\')){
            const char *p=b+start;while(*p&&isspace((unsigned char)*p))++p;
            if(!strncmp(p,"#X obj",6)){char object[64]={0};if(sscanf(p,"#X obj %*s %*s %63s",object)==1){object[strcspn(object,",;")]=0;
                for(unsigned q=0;q<s->count;q++){size_t z=strlen(s->files[q])-3;if(strlen(object)==z&&!memcmp(object,s->files[q],z))edges[k]|=UINT64_C(1)<<q;}}}
            start=i+1;
        }free(b);
    }
    for(unsigned k=0;k<s->count;k++)if(!visit(k,edges,color,s->count))return 0;return 1;
}
