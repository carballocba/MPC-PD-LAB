# Phase 4 validation — 2026-10-13

No MPC hardware was connected, installed to, configured or restarted.
All Granular Freezer results below are offline. The user reports successful
MPC PD Delay loading/audio on MPC One/Hakai 3.9.1; that is a separate baseline.

## Evidence

[CI run 38374544427](https://github.com/carballocba/MPC-PD-LAB/actions/runs/38374544427),
implementation commit `fc119a788e6e7dca22a0202415cfdc52e3f22ab4`, passed:

| Test | Result |
| --- | --- |
| All retained Phase 1–3 desktop/ARM/QEMU builds, delay audio/VST tests and Phase 3 package tests | PASS; delay ZIP still exactly `a802278ab1ded9ff12cec54aa5543b1095b6907aca7d669d398168a03ef7e0e5` |
| Original patch inventory and exact source hashes | PASS; imported files also compared byte-for-byte against the official upstream checkout |
| Missing source/changed source/unknown external static audit | PASS: rejected |
| Native macOS ARM64 bridge and VST tests | PASS |
| Linux desktop bridge and VST tests | PASS |
| ARMv7/QEMU bridge and VST tests | PASS |
| Real Hanning array loaded from original patch | PASS |
| Mother knob message routing and six mapped values | PASS |
| Freeze absolute-state to aux toggle edges; idempotent repeated values; footswitch edge de-duplication | PASS |
| MIDI note-on/off and controller routing | PASS |
| Scalar state save/restore and malformed-state rejection | PASS |
| Stereo dry-channel isolation, measured 64-frame named-bus latency | PASS |
| Original wet DSP, both left-only and right-only input excite both stereo grain outputs | PASS |
| Frozen audio remains after silence beyond five-second source-buffer duration | PASS |
| Close/reopen; second instance refusal; invalid frames/null input silence | PASS |
| Missing each required runtime file | PASS: VST entry returns NULL |
| Missing external injected into a loadable host canvas | PASS: Pd diagnostic captured, VST entry returns NULL |
| Distinct CGFZ ID, effect classification, stereo I/O, six parameter names/readback, VST chunk restore | PASS |
| ARM ELF32 EABI5 hard-float, ARMv7/VFP argument attributes, only VSTPluginMain export | PASS |
| Dependencies and glibc | PASS: libc/libm/libdl/libpthread, no external libpd; maximum required GLIBC 2.29 |
| Runtime paths/text relocations | PASS: absent |
| Package source/identity/assets/notices/checksums and upstream catalog rules | PASS, review notes below |
| Actual upstream desktop OpenPackage reader | PASS |
| Local temporary install/uninstall with MPDL registration and delay-folder sentinel | PASS: delay and unrelated settings preserved; complete settings backup retained |
| UID collision, existing folder, repeated install, corrupted payload | PASS: refused without target/settings changes |
| Repeated ZIP generation | PASS: byte-identical |
| Hardware discovery, GUI/Q-Link access, MIDI routing into inserts, timing, stability, coexistence | UNTESTED |

The downloaded CI artifact `11800666748` matched its published SHA-256
`eb4a149d9bdd2fd697e37553aebd74f35417823cb3b93f3f8ca6e9f5566d5335`.
The ARM ZIP was subsequently rechecked on macOS with the same package tests and
actual pinned upstream Go ZIP reader. Go 1.27.2 / pyelftools 0.33 were used locally.

Granular Freezer ARM module SHA-256:
`9e736bbb51464e5af7ed15084772839df285248066281b7e00351a573fbff24d`.

Installable ZIP SHA-256:
`fcd26e7d9f801d9029c3c36fb10bee79c5f6181139859237041a0e8663da2164`.

The final follow-up adds this report and an explicit same-toolchain repeated ARM
compile/link comparison, using the same pinned Phase 2 libpd archive. This checks
the final module, not full archive reproducibility across different toolchains
or paths. Final-branch CI is linked from the PR checks. No additional DSP changes
were made after the native/desktop/QEMU results above.

## Review notes and limits

The upstream checker reports undeclared overall project licensing (original
patch and third-party notices/source are included), the deliberate fresh-install
preflight added to its install.sh, and static network/process/dlopen/setuid
capabilities inherited from libpd/Pure Data. The audited delay/granular patches
do not use those objects; no sandbox/capability-removal claim is made.

Only the two retained wrapper unused-function warnings occurred in Linux/ARM
builds; engine/tests compiled with warnings-as-errors. macOS's upstream Pd build
also emitted existing format/pointer warnings. The injected missing-external
`error: ... couldn't create` line is the expected negative-test diagnostic,
not a failed positive load. CI action-runtime deprecation warnings do not change
the test results and are outside this patch-porting scope.

No standalone native MPC page or verified Q-Link mapping exists yet. This port
is fixed to 44.1 kHz with one active module instance, scalar-only state and int16
conversion/clipping. Original wet input is mono-summed with stereo grain outputs.
The process path adds no adapter file I/O/allocation/lifecycle lock, but original
Pd message allocations and the inherited wrapper mutex remain. QEMU tests do
not guarantee real-time performance. A plugin crash may affect MPC itself.

See PHASE4_SOURCE_AUDIT.md, PHASE4_PORTABILITY.md and PHASE4_INSTALL.md for exact
sources, unchanged-DSP adaptations, installation/rollback and Phase 5 roadmap.
