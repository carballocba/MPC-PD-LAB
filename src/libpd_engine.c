/* MPC PD LAB: libpd effect engine adapter (experimental).
 * Requires libpd's z_libpd.h and libpd linked into the .so.
 * Host: 44100 Hz, interleaved int16 stereo; Pd: 64-sample ticks.
 * No allocation in the audio callback. This adapter is NOT hardware-tested.
 */
#include "engine.h"
#include "z_libpd.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    void *patch;
} pd_instance;

/* libpd initialization and patch lifecycle must be serialized if multiple
 * instances are supported. v0.1 intentionally supports one instance only. */
static int initialized = 0;
static int active = 0;
/* Only lifecycle calls take this lock; the audio callback never takes it.
 * The host must stop audio before destroying the active instance. */
static pthread_mutex_t lifecycle_lock = PTHREAD_MUTEX_INITIALIZER;

static void *pd_create_locked(const char *data_dir) {
    if (active) return NULL; /* multi-instance is a future milestone */
    if (!initialized) {
        if (libpd_init() != 0) return NULL;
        initialized = 1;
    }
    if (libpd_blocksize() != 64 || libpd_init_audio(2, 2, 44100) != 0) return NULL;
    const char *dir = data_dir && data_dir[0] ? data_dir : ".";
    /* Reject incomplete bundles instead of loading a silent host canvas. */
    const char *files[] = {"libpd_host.pd", "stereo_delay_smoke.pd"};
    for (unsigned i = 0; i < sizeof files / sizeof files[0]; ++i) {
        char path[1200];
        int n = snprintf(path, sizeof path, "%s/%s", dir, files[i]);
        if (n < 0 || n >= (int)sizeof path || access(path, R_OK) != 0) return NULL;
    }
    /* The delay is an abstraction with inlet~/outlet~ ports. The host patch
     * connects those ports to libpd's input/output channels via adc~/dac~. */
    void *patch = libpd_openfile("libpd_host.pd", dir);
    if (!patch) return NULL;
    pd_instance *s = calloc(1, sizeof *s);
    if (!s) { libpd_closefile(patch); return NULL; }
    s->patch = patch;
    libpd_start_message(1);
    libpd_add_float(1);
    libpd_finish_message("pd", "dsp");
    active = 1;
    return s;
}
static void *pd_create(const char *data_dir) {
    pthread_mutex_lock(&lifecycle_lock);
    void *s = pd_create_locked(data_dir);
    pthread_mutex_unlock(&lifecycle_lock);
    return s;
}
static void pd_destroy(void *inst) {
    pd_instance *s = inst;
    if (!s) return;
    pthread_mutex_lock(&lifecycle_lock);
    libpd_closefile(s->patch);
    free(s);
    active = 0;
    pthread_mutex_unlock(&lifecycle_lock);
}
static void pd_midi(void *inst, const uint8_t *msg, int len) {
    (void)inst; (void)msg; (void)len;
}
static void pd_set(void *inst, const char *key, const char *val) {
    (void)inst; (void)key; (void)val;
}
static int pd_get(void *inst, const char *key, char *buf, int len) {
    (void)inst; (void)key; (void)buf; (void)len;
    return 0;
}
static void pd_process(void *inst, const int16_t *in_lr, int16_t *out_lr, int frames) {
    if (frames <= 0) return;
    pd_instance *s = inst;
    if (!s || !in_lr || !out_lr) {
        if (out_lr) memset(out_lr, 0, (size_t)frames * 2 * sizeof(int16_t));
        return;
    }
    /* Pd processes 64 frames per tick. Wrapper normally requests 128.
       Fail safely for unsupported frame counts in this first prototype. */
    if (frames != 128) {
        memset(out_lr, 0, (size_t)frames * 2 * sizeof(int16_t));
        return;
    }
    /* libpd_process_short accepts interleaved stereo int16 samples. */
    if (libpd_process_short(2, in_lr, out_lr) != 0)
        memset(out_lr, 0, (size_t)frames * 2 * sizeof(int16_t));
}
static const mpc_engine_t ENGINE = {
    pd_create, pd_destroy, pd_midi, pd_set, pd_get,
    NULL, pd_process, NULL
};
const mpc_engine_t *mpc_engine(void) { return &ENGINE; }
