# Research conclusions and implementation roadmap

Research date: 2026-10-13. Original MPC One, ARMv7 hard-float, Hakai 3.9.1.
Research only: no plugin/DSP/installer/system changes, hardware access or deployment.
Evidence labels: **source-verified**, **manufacturer-documented**, **upstream hardware report**,
**user-reported hardware result**, **inference/proposal**, and **not established** are distinct.
Exact source revisions and access gaps are in [SOURCES.md](SOURCES.md).

## Current approved scope for design (implementation still awaiting approval)

The latest requested first milestone is the two-folder, Q-Link-controlled Loader prototype,
with GUI and CV kept optional. Its concrete specification supersedes the earlier UI-first
milestone below: [UNIVERSAL_LOADER_PROTOTYPE.md](../design/UNIVERSAL_LOADER_PROTOTYPE.md).
See the linked symbol-boundary and complete 24-failure classification before implementation.
No production code or hardware access was authorized by preparing that design.

## Executive assessment

The existing MPC wrapper, libpd and native skin generator are the strongest foundation.
NAM/Crate Digger provide useful browser/state patterns; Acid demonstrates transport-aware
MIDI sequencing; Glass Spheres is a significant direct-CV precedent on MPC X. Neither
that X result nor desktop/QEMU tests establishes original-One private CV compatibility.
The original One has eight output signals on four TRS CV/Gate sockets, with no documented
CV input. Native CV tracks and existing external converters deserve priority over a new
private hardware driver.

Immediate reuse means source patterns and existing build infrastructure, not automatic
binary compatibility. Patch loading, persistent mappings, native UI and external MIDI
routing need adaptation and hardware evidence. Seamless arbitrary patch switching,
arbitrary externals/desktop editors, direct One CV and sample-accurate CV are unsupported
claims today. Project/patch/vendor license review precedes redistribution.

## Smallest safe next milestone — requires implementation authorization

Create a separately identified Universal prototype with one curated vanilla effect and
one static native control page, using existing packaging and libpd. Demonstrate label,
parameter readback, save/reopen and bypass at 44.1 kHz before adding a browser or CV.
Retain Delay and Freezer unchanged. This isolates the currently black-GUI uncertainty
without changing their DSP or packaging. Acceptance is visible functional UI and stable
controls on the original One in an empty backed-up project, followed by clean rollback.

## Ordered gates

1. **License and baseline:** resolve project license and patch notices; record known
   working binaries, IDs and dependency pins. Reuse Phase 2/3 validation. No merge of
   Phase 4 as part of this research.
2. **Native UI proof:** fixed effect controls, status text and host automation/Q-Link
   readback. Test every bank/page, stop/play, reopen, bypass and multiple instances.
3. **Curated loader:** manifest validation and local browser; stopped reload only;
   missing assets and failed loads preserve a valid graph. Test malformed manifests,
   relative-path escapes, repeated reloads, memory growth and explicit state migrations.
4. **Organelle subset:** knob/aux/notes/display/tempo adapters and audited assets.
   Reject OS scripts and unsupported externals. Test stereo routing, gain and persistence.
5. **MIDI timing bridge:** confirm whether VST event output is ignored on this exact
   host; validate an explicit virtual ALSA route if necessary. Track note-off cleanup,
   CC limits, queue overflows, loop/locate/tempo/stop and subscriptions.
6. **Host CV / external converters:** use empty CV tracks and manufacturer-supported
   interfaces. Scope measured pitch/gate/clock/reset and jitter, with destination voltage
   limits checked first. Keep a single writer per output and master clock.
7. **Optional private CV feasibility:** only after separate authorization, compare
   One controller enumeration/protocol to the X precedent. No hardcoded card number,
   blind writes, pulse-coalescing queue or cross-instance ownership assumptions. Stop
   if public/source evidence cannot support safe access. This is not a required release gate.
8. **Performance and release:** original-One long sessions and CPU/memory/jitter under
   realistic project load; package/state/rollback tests; then a reviewed implementation
   PR with reproducible artifact. Do not merge automatically.

## Verification performed for this research

Pinned primary repository source and manufacturer manuals were inspected; source paths,
license boundaries and hardware evidence are recorded across these documents. The
116-entry catalog was inventoried, with six focused plugin/framework source audits.
Document structure, relative links and whitespace are checked locally. No new plugin was
built and no hardware, clock, voltage, crash or real-time measurements were performed.
Research-only changes do not justify reporting a production test suite as newly passed.

## Risks retained

A plugin runs inside the MPC application and can crash it. QEMU timing does not predict
real-time performance. The existing Delay is fixed at 350 ms and has no custom GUI or
Q-Link mapping; Freezer's six Q-Links/freeze and both effects working are user-reported,
while its GUI remains black. Direct private CV on the One is unverified; no CV input is
assumed. Host firmware feature/licensing availability and routing need device-specific
validation. Electrical calibration and end-to-end jitter remain measurement tasks.

## Follow-up gates from supplied sources

The smallest safe milestone remains a separately identified native-UI proof with one
curated Vanilla patch. Before expanding its library:

- Define a graph entry policy that detects custom mothers; PLAY/CZ demonstrates why
  an empty `main.pd` cannot be treated as a successful instrument load.
- Audit static external registration versus hidden Pd exports; inspect dependency ABI,
  float width and instruction set before loading any existing ARM binary.
- Resolve Jam/PLAY grants and nested GPL provenance in CCquencer2/MIDI Maker before
  redistribution. Prefer a licensed curated control patch over a rewritten sequencer.
- Retain existing MPC tools/CI and pin images/dependencies when reproducibility matters.
  Current catalog ceilings and CPU/hardware tests are separate acceptance gates.
- Verify external MIDI-to-CV with supported USB roles; keep module-specific calibration
  and timers on existing external hardware rather than porting board drivers.

Follow-up validation: 75 upstream offline catalog/build/site tests and 10 skin compatibility
tests passed. The broader synthetic fixture suite had 24 failures out of 125; one store
failure was traced to missing macOS `sha256sum`. Full suite success is not claimed.
No production plugin was built, installed or hardware-tested in this follow-up.
