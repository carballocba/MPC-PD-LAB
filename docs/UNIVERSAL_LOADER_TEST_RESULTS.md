# Universal Loader v0.1 — validation record

Date: 2026-10-14 (Australia/Brisbane). No physical MPC was contacted, no installer
was executed, no MPC.settings was changed, and no PR was merged.

## Reproducible Linux/ARM evidence

The complete validation baseline is commit
`a97475dd73871d1b7c57c60aaf8f8a16dc8d43e5`,
[Actions run 38511396245](https://github.com/carballocba/MPC-PD-LAB/actions/runs/38511396245).
Both jobs passed. Subsequent changes add a regression/fix for late recall
cancellation, large-file open interposition and package provenance; use the
latest successful **Universal Loader offline validation** run for the exact
reviewed branch commit and artifact. The downloaded ZIP's PROVENANCE.json
records its source commit and binary hash. Do not mix artifacts from older runs.

| Gate | Verified baseline result / coverage |
|---|---|
| Linux engine | PASS: 1,000 alternating folder loads; concurrent parameter/state producer; zero Pd ownership overlap |
| Functional regressions | PASS: stereo, missing/malformed patches, retained snapshots, rollback failure → silence → recovery, edited Pd text without rebuilding the VST, catalog reorder, UL0, shutdown and mains-off handoff |
| Controls/DSP | PASS: Gain 0/0.5/1, Mix 0/0.5/1, 500/8000 Hz low-pass response against analytic transfer magnitude (0.006 tolerance), right-channel isolation |
| Linux ASan/UBSan/LSan | PASS: engine, manifest, VST ABI/state/reopen/unload/reload; 100 switches in sanitizer run; leak detection enabled |
| Linux TSan | PASS: same suites, 100 switches; no reported race; ephemeral runner mmap randomization set to 28 bits for GCC TSan compatibility |
| Callback interposition | PASS on Linux and ARM/QEMU: aborts on allocation/free, mutex locking, file opens, reads/writes/closes, sleeping or join during rendering; tests also stall worker and require initialized silence |
| Upstream Linux tooling | 75 catalog/tool tests + 10 skin compatibility tests PASS at sd88me `5e3a96885b6de76a48b498821c379568a05cc006` |
| ARMv7 | PASS: pinned cross toolchain, 100 QEMU switches, all three Loader suites, repeated link byte comparison |
| ELF | ELF32 little-endian ARMv7 EABI5 hard-float; only VSTPluginMain exported; Pd private; no RPATH/RUNPATH/TEXTREL |
| Dynamic dependencies | libm.so.6, libpthread.so.0, libdl.so.2, libc.so.6; maximum required glibc 2.29; no libpd shared dependency |
| Package | Upstream catalog validation, metadata/UID/effect classification, four Q-Link mappings, runtime files/licenses/checksums, shell syntax and fresh-only guard ordering PASS; repeated ZIP bytes identical |
| Legacy code | All 19 pre-existing main source/patch/packaging/build/test files unchanged from `ae21b99c7b31e9b722755b6d7852214a39d42c3e` |
| Legacy behavior | Delay's existing desktop/ARM tests PASS. Freezer is built/tested from unchanged `879ab14336fd5c4576ab9832127b4047aaa2a2b6`; original source hashes and frozen-tail tests PASS. Loader/Delay/Freezer coexistence PASS; reference checkout stays clean |

The final callback audit additionally wraps open64/fopen64, because
_FILE_OFFSET_BITS=64 redirects those calls on Linux. This is instrumentation of
the exercised curated graphs, not a proof about arbitrary Pd objects or future
patches. Worker I/O is intentionally allowed. Host lifecycle callbacks may
allocate or join; the audio callback may not.

The new late-cancellation test reproduces cancelling project recall after the
worker opens the candidate but before activation. Rollback must restore the old
patch and its controls while preserving any newer explicit control edits. It
failed before the targeted fix; the final CI run must include that regression.

## Failures investigated and repaired

- Worker writes: GCC fortified headers exposed unchecked return values; worker
  handles failure instead of suppressing warnings.
- Cross archive: pinned libpd Makefile hardcodes `ar`, ignoring AR. Loader now
  uses the existing Phase 2 pattern: make PIC objects, then invoke the selected
  native/cross archiver explicitly.
- Pd locking: MULTI=false does not disable PDTHREADS's default. A separate Loader
  archive uses PDTHREADS=0 and atomic exclusive ownership. Minimal no-thread
  branch compile repairs are recorded in `vendor/universal/pd-no-threads.patch`.
- Reblocker lifetime: Linux LSan reported 47,808 bytes/664 signal inlet/outlet
  allocations. The private copied Pd destructors now free their enclosing x_rb
  arrays after the nested buffers. No DSP algorithm or legacy archive changed.
- DSO unload: libpd default-instance classes/symbols have no full teardown API.
  Linux/ARM ELF NODELETE retains one initialized private module until process
  exit; graph/worker resources still close normally. No LSan suppression. The macOS development host explicitly uses RTLD_NODELETE
  to mirror the target lifetime and avoid dyld/ASan registering reloaded image
  globals twice; Linux tests use normal dlopen and verify the ELF behavior.
- VST text bounds: names/displays fit an 8-byte host buffer; canary tests protect
  adjacent memory. Explicit copy avoids GCC's truncation/overread warnings.
- Stress test synchronization: next Apply waits for transaction acknowledgement;
  a full bounded restore mailbox returns failure rather than blocking. Test
  producers retry after making progress, not by weakening production bounds.
- Late recall cancellation: both rollback paths now undo the recalled controls
  when those values have not been superseded by a newer explicit edit.

The earlier 24 upstream macOS failures are classified in
`docs/design/UPSTREAM_TEST_FAILURE_CLASSIFICATION.md`. GNU/BSD sha256sum/sed/wc
assumptions and unrelated addin/store/installer fixtures do not block this
Loader's Linux non-installer gates. **50 upstream installer-related tests are
not run**, to honor the prohibition on executing installers. Installation
integration is therefore unverified; syntax/catalog checks are not an install.

## Package review notes and limitations

Upstream correctly warns that install.sh differs from its template: the only
addition is the fresh-install collision guard before any staging/settings/service
mutation. Review its exact diff. It also flags libpd's linked network/process/
dlopen/setuid imports. Those facilities are present in the linked Pd runtime,
although the curated object allowlist excludes patch access to them; this is not
a sandbox. Do not treat the absence of allowed network objects as removal of
those native capabilities from the binary.

Only two trusted flat Vanilla folder patches, stereo 44.1 kHz/128 frames, four
fixed controls and one Loader instance per loaded module are supported. Arbitrary
Vanilla/Organelle patches, externals, MIDI, CV/Gate, GUI/browser and multiple
instances are outside v0.1. Hidden Pd symbols intentionally prevent ordinary
ARM .pd_linux externals from resolving the embedded API. Linux retains class/
module memory until the MPC application exits. A native crash can affect the MPC
application. QEMU timing does not establish real-time deadline performance.

## Hardware acceptance still pending

Explicit approval is required before hardware access or installation. Follow
UNIVERSAL_LOADER_INSTALL.md and UNIVERSAL_LOADER_VALIDATION.md: backup/recovery,
/storage/Synths registration and effect discovery, actual host block size and
transport validity, four physical Q-Links and Apply rearming, stereo/filter/fades,
resume/cancel/failure behavior, project save/reopen and reordered/missing IDs,
long-duration CPU/memory/xrun/deadline measurement, coexistence with the installed
stable plugins, and safe uninstall/rollback. No hardware compatibility claim is
made by these automated results.
