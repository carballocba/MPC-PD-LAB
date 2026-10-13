# libpd VST2 adapter — experimental

`src/libpd_engine.c` implements the `mpc_engine_t` effect contract from [sd88me/mpc-vst-plugins/wrapper/engine.h](https://github.com/sd88me/mpc-vst-plugins/blob/main/wrapper/engine.h).

**Desktop and ARMv7/QEMU audio tests pass.** The ARMv7 VST2 plugin is compiled and offline-verified; MPC hardware remains unverified. See [build instructions](ARMV7_BUILD.md) and [Phase 2 results](PHASE2_VALIDATION.md).

## Integration prerequisites
- Build libpd for ARMv7 Linux using the same compatible ABI/toolchain as the wrapper.
- Compile/link `src/libpd_engine.c` alongside `wrapper/vst2_wrap.c`, generated `params.h`, and libpd.
- Configure the port as `effect: true` in the wrapper's `vst.json` and package both `patches/libpd_host.pd` and `patches/stereo_delay_smoke.pd` in the plugin's data directory. Verify the wrapper's MODULE_DIR/MODULE_SUBDIR path resolution.
- Test on desktop with a compatible libpd build before running on MPC.
- The example deliberately supports only **one instance** because global libpd state and per-instance patch/audio isolation need further work. The benchmark target of 4 instances is **not yet implemented**.
- The patch is a 350 ms stereo *wet-only* smoke test, not the finished configurable delay.
- libpd's 64-frame DSP ticks require a multiple-of-64 frame count. The wrapper's normal 128-frame effect blocks meet this.
- The adapter currently accepts exactly 128 frames per callback (two 64-frame ticks); other block sizes return silence.

## Next development tasks
1. Complete separate controlled MPC hardware validation of the Phase 2 build.
2. Implement per-instance isolation (libpd multi-instance API if supported by target build).
3. Implement parameters and a controllable delay patch.
4. Extend desktop regression coverage and add device performance measurements.

## Silent-output integration fix (2026-10-13)

Opening `stereo_delay_smoke.pd` directly leaves its `inlet~`/`outlet~` ports
unconnected. `libpd_process_short` exchanges audio through `adc~`/`dac~`, so
successful processing calls returned silence. The adapter now opens
`libpd_host.pd`, which routes channels 1 and 2 through the unchanged delay
abstraction. Pd resolves the abstraction from the host patch's directory.
The audio callback still processes the caller's interleaved int16 buffers
directly, with no added allocations, copies, or conversion buffers. Removed
two unused 256-sample int16 arrays (1024 bytes per instance).

### Published-environment validation

Dependencies: libpd `ba0dc63262901d658af8bbda5e619a60fa975e78`
(Pd 0.56.5), wrapper `208745830fc2ed90b2fd0dc480ef5c554284292f`.
The existing environment smoke harness processes 350 blocks at 44.1 kHz,
128 frames, injecting left +12000 and right -8000 impulses. It also checks
that the adapter rejects a second active instance.

| Test | Before | After |
| --- | --- | --- |
| `audio_smoke engine` | FAIL: peaks 0,0; frames -1,-1 | PASS: peaks 12000,-8000; frames 15435,15435 |
| `audio_smoke hosted` | PASS: peaks 12000,-8000; frames 15435,15435 | PASS: identical output |

Reproduce in the published environment from the repository root:

```sh
mkdir -p work
cc -Wall -Wextra -Werror \
  -I /workspace/.mpc-pd-setup/mpc-vst-plugins/wrapper \
  -I /workspace/.mpc-pd-setup/libpd/libpd_wrapper \
  -I /workspace/.mpc-pd-setup/libpd/pure-data/src \
  src/libpd_engine.c /workspace/.mpc-pd-setup/audio_smoke.c \
  -L /workspace/.mpc-pd-setup/libpd/libs \
  -Wl,-rpath,/workspace/.mpc-pd-setup/libpd/libs -lpd -o work/audio_smoke
work/audio_smoke engine /workspace/MPC-PD-LAB/patches
work/audio_smoke hosted /workspace/.mpc-pd-setup
```

Both tests exited 0 after the fix; the strict compiler build also passed.
Frame 15435 is exactly 350 ms at 44.1 kHz, preserving stereo channel polarity
and delay timing. No MPC deployment, ARMv7 packaging, or hardware benchmark
was performed.
