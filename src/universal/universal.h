/* SPDX-License-Identifier: MIT. MPC-PD-LAB Universal-specific adapter. */
#ifndef UL_UNIVERSAL_H
#define UL_UNIVERSAL_H
#include <stdint.h>
#include <stddef.h>
typedef struct ul_engine ul_engine;
enum { UL_INITIALIZING, UL_READY, UL_WAIT_STOP, UL_LOADING, UL_BUSY,
       UL_MISSING, UL_INVALID, UL_OBJECT, UL_CONFIG, UL_ROLLBACK, UL_CANCELLED };
ul_engine *ul_create(const char *data_dir);
void ul_destroy(ul_engine *);
void ul_set(ul_engine *, unsigned index, float value);
float ul_get(ul_engine *, unsigned index);
void ul_transport(ul_engine *, int known, int playing);
void ul_mains(ul_engine *, int active);
void ul_config(ul_engine *, int supported);
void ul_process(ul_engine *, const int16_t *input, int16_t *output, int frames);
int ul_save(ul_engine *, char *output, size_t capacity);
int ul_restore(ul_engine *, const void *data, size_t length);
unsigned ul_status(ul_engine *);
unsigned ul_active_slot(ul_engine *); /* 2 means no committed patch. */
void ul_display(ul_engine *, char *output, size_t capacity);
#ifdef UL_TESTING
/* Test-only worker barriers. No waiting, sleeping or file operations in render. */
void ul_test_pause(ul_engine *, int preflight, int load);
unsigned ul_test_phase(ul_engine *);
unsigned ul_test_pending(ul_engine *);
unsigned ul_test_pd_overlap(ul_engine *);
#endif
#endif
