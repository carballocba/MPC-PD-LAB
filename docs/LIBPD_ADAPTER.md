# libpd VST2 adapter — experimental

`src/libpd_engine.c` implements the `mpc_engine_t` effect contract from [sd88me/mpc-vst-plugins/wrapper/engine.h](https://github.com/sd88me/mpc-vst-plugins/blob/main/wrapper/engine.h).

**Not a compiled or verified plugin.** This is an integration starting point.

## Integration prerequisites
- Build libpd for ARMv7 Linux using the same compatible ABI/toolchain as the wrapper.
- Compile/link `src/libpd_engine.c` alongside `wrapper/vst2_wrap.c`, generated `params.h`, and libpd.
- Configure the port as `effect: true` in the wrapper's `vst.json` and package `patches/stereo_delay_smoke.pd` in the plugin's data directory. Verify the wrapper's MODULE_DIR/MODULE_SUBDIR path resolution.
- Test on desktop with a compatible libpd build before running on MPC.
- The example deliberately supports only **one instance** because global libpd state and per-instance patch/audio isolation need further work. The benchmark target of 4 instances is **not yet implemented**.
- The patch is a 350 ms stereo *wet-only* smoke test, not the finished configurable delay.
- libpd's 64-frame DSP ticks require a multiple-of-64 frame count. The wrapper's normal 128-frame effect blocks meet this.
- Review the installed libpd API version and linking behavior before compiling; no build has been attempted yet.

## Next development tasks
1. Add a reproducible ARMv7 build manifest and build script.
2. Implement per-instance isolation (libpd multi-instance API if supported by target build).
3. Implement parameters and a controllable delay patch.
4. Add desktop audio test and device performance measurements.
