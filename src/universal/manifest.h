/* SPDX-License-Identifier: MIT */
#ifndef UL_MANIFEST_H
#define UL_MANIFEST_H
#include <stddef.h>
#define UL_PATH 512
#define UL_FILES 64
#define UL_ID 64
#define UL_REV 32
#define UL_TEXT_LIMIT (1024u * 1024u)
typedef struct { char id[UL_ID], folder[UL_PATH]; } ul_catalog_entry;
typedef struct {
    char id[UL_ID], revision[UL_REV];
    char files[UL_FILES][UL_PATH];
    unsigned count;
} ul_patch_spec;
int ul_identifier(const char *, size_t);
int ul_relative(const char *);
int ul_catalog_read(const char *filename, ul_catalog_entry entries[2]);
int ul_manifest_read(const char *filename, const char *expected_id, ul_patch_spec *);
int ul_regular_path(const char *root, const char *relative, char output[1024]);
int ul_audit_closure(const char *root,const ul_patch_spec *spec);
int ul_audit_pd(const char *text, size_t length, const ul_patch_spec *spec);
#endif
