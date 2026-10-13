# Phase 2 offline validation — 2026-10-13

Based on latest main `69ca33aa11ac4f3b4f3c44c7b8135d3fef1608c5`.
Branch: `phase2/armv7-vst2`. No MPC hardware was accessed or modified.

## Results

| Check | Result |
| --- | --- |
| Existing published-environment `audio_smoke engine` | PASS, +12000/-8000 at frame 15435; second instance rejected |
| Existing published-environment `audio_smoke hosted` | PASS, identical peaks and timing |
| Original smoke harness compiled with `-Wall -Wextra -Werror` | PASS |
| Portable engine and hosted smoke modes, desktop and ARM/QEMU | PASS; engine also checks unsupported/negative frame counts and null input |
| Desktop and ARM/QEMU VST host | PASS: magic, UID MPDL, version 1000, stereo I/O, effect flags/category, VST 2.4, name/vendor |
| VST plugin stereo impulse | PASS, +12000/-8000 int16 equivalent at frame 15435, exactly 350 ms |
| Second VST instance, missing input, legacy accumulation, close/reopen | PASS |
| 64-frame host calls through reference wrapper FIFO | PASS, impulse at 15563 = delay +128 buffering frames |
| Missing host patch or missing delay abstraction | PASS, VST entry rejects incomplete bundles |
| ARM ELF and dependency checks | PASS, ELF32 little-endian ARMv7 EABI5 hard-float, VFP register arguments |
| Exported defined dynamic symbols | Exactly `VSTPluginMain` |
| Dynamic dependencies | `libm.so.6`, `libdl.so.2`, `libpthread.so.0`, `libc.so.6`; libpd embedded |
| glibc version requirements | 2.4, 2.7, 2.27, 2.28, 2.29 (maximum 2.29) |
| RPATH/RUNPATH/text relocations | None |
| Repeated clean ARM source/object build | Byte-identical `.so`, SHA-256 below |
| Shell syntax, Python compilation, Git whitespace checks | PASS |
| MPC One/Hakai host discovery, UI, realtime performance, stability | UNTESTED |

Final ARM plugin: 1,065,080 bytes (unstripped).
SHA-256: `76a7340039def8494f9d78acb4ad23ac75f8b02b889557f47eb84dc26930e0df`.

The ELF checker enforces the architecture, hard-float ABI, export list,
allowlisted dependencies, maximum glibc symbol version and absence of embedded
runtime paths or text relocations. QEMU dynamically loads the actual plugin
with the ARM sysroot and its real neighboring packaged Pd resources. Repeat
build equality was checked for the plugin binary; this does not establish
identical binaries across different compiler versions, build paths, or CPUs.

## CPU and memory observations

The host times 10,000 silent, 128-frame `processReplacing` calls after the stereo
impulse test. Times include wrapper conversion, engine-call mutex and libpd DSP;
they exclude plugin creation and loading. The clock instrumentation and Linux
scheduling contribute to the measurements.

| Runtime | Median | p95 | Maximum | Whole-process peak RSS |
| --- | --- | --- | --- | --- |
| Native x86_64 desktop, GCC 14 | 1.543 us | 1.572 us | 100.732 us | 3,460 KiB |
| ARMv7 under QEMU 5.2 on x86_64 | 34.372 us | 49.514 us | 4,100.732 us | 22,928 KiB |

These are build-host measurements, not MPC benchmarks. QEMU RSS includes its
runtime; desktop RSS includes the host, loaded plugin, Pd globals and buffers.
Neither isolates per-instance allocation. QEMU's maximum exceeded the 2.902 ms
128-frame deadline, so these results do not demonstrate realtime safety.
No hardware CPU percentage, xruns, or four-instance performance is claimed.

The adapter callback performs two 64-frame libpd ticks directly on caller-owned
interleaved int16 buffers. It adds no allocations or copies. The reference
wrapper owns fixed buffers and converts float audio; it retains a per-instance
mutex around engine calls. The added lifecycle lock runs only during create
and destroy. Allocation behavior was reviewed in source, not measured with an
allocation tracer. Delay storage is fixed at two seconds per channel in the
unchanged Pd abstraction; this phase does not resize it in the callback.

## Warnings and investigated failures

- Two retained reference-wrapper warnings on both builds: unused `cc_set`
  (CC/NRPN disabled), and unused `render_frames` (instrument path in effect
  build), at pinned wrapper lines 612 and 417. No adapter or test warnings;
  no warnings emitted by libpd's upstream Makefile build. Wrapper warnings are
  reported rather than hidden by `-Wno-unused-function`.
- Initial sandbox commands could not reach the configured proxy or Docker
  socket. Approved command access resolved this without changing network policy.
- Bullseye security indices referenced package URLs returning 404. The final
  build-only toolchain uses available official Bullseye main packages, with all
  added packages pinned in `toolchain-packages.lock`; TLS and apt signature
  verification remain enabled. This older toolchain container is for local,
  offline compilation/testing, not a deployed service. The target binary still
  links against the pinned ARM glibc 2.31 sysroot.
- The first HTTPS retry failed because apt's downloader could not read the
  root-only CA secret. Its mount is now read-only mode 0444 for that build step.
- The minimal cross image lacked Git and native `readelf`. Source pins are now
  checked by Python (with Git cleanliness checks when Git is available), and
  the ELF checker selects `arm-linux-gnueabihf-readelf`. Existing desktop cache
  refs are accepted only when their resolved commit exactly matches the pins.
- The first ARM smoke test was silent despite the host patch opening. Syscall
  tracing showed the abstraction opened then rejected. A standalone ARM probe
  reproduced `fstat=-1, errno=75` on the mounted filesystem; compiling with
  `_FILE_OFFSET_BITS=64` changed it to success. The final flag is applied to all
  build sources and both ARM smoke tests pass. This fixes file metadata overflow
  on large-inode filesystems, not only an emulator-specific audio symptom.

## Remaining compatibility limits

Single active instance per loaded module; fixed 44.1 kHz; wrapper int16
quantization/clipping; no configurable delay controls, custom touchscreen skin,
project-state chunks, or saved delay tails. A second instance fails explicitly.
Other sample rates are acknowledged by the reference wrapper but unsupported
by the fixed-rate engine. Non-128 host buffers add FIFO latency. Host discovery,
firmware library availability, `/proc/self/maps` lookup on Hakai, native effect
presentation, and insertion/reload behavior require controlled hardware tests.
See `ARMV7_BUILD.md` for prerequisites, packaging, and rollback.
