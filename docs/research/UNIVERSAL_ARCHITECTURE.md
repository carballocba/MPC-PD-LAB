# Universal host architecture recommendation

Research date: 2026-10-13. Original MPC One, ARMv7 hard-float, Hakai 3.9.1.
Research only: no plugin/DSP/installer/system changes, hardware access or deployment.
Evidence labels: **source-verified**, **manufacturer-documented**, **upstream hardware report**,
**user-reported hardware result**, **inference/proposal**, and **not established** are distinct.
Exact source revisions and access gaps are in [SOURCES.md](SOURCES.md).

## Decision

Recommend a hybrid: keep the proven MPC ARMv7/libpd wrapper and native skin generator;
add an explicit patch manifest, fixed parameter pool and controlled loader. Reuse MIT
browser patterns and Organelle control conventions. Study desktop hosts for lifecycle
and state design without assuming their GUI or build targets transfer to Hakai.

| Dimension | A — Extend current bridge | B — Port established host | C — Hybrid (recommended) |
|---|---|---|---|
| Code reuse | Own libpd/VST wrapper and build | Camomile or PlugData host code | Own bridge + native skin + MIT browser patterns + Organelle conventions |
| Complexity / effort | Moderate, but loader/state designed locally | Highest: formats, dependencies, GUI and host adaptation | Moderate: smaller targeted adapters with explicit contracts |
| MPC compatibility | Closest to working binaries | ARMv7 VST2/Hakai port unproven | Preserves validated MPC boundary; new features still require tests |
| CPU / memory | Existing baseline plus patch and loader | Larger dependency/editor surface; no One benchmarks | Bounded workers/queues proposed; no performance claim before measurement |
| GUI | Native skin work needed | Desktop GUI does not establish native MPC UI | Existing skin generator plus fixed browser/control slots |
| MIDI | Add libpd translation/output routing | Mature host handling; MPC output route still unresolved | Reuse host input boundary and community ALSA timing patterns |
| Patch compatibility | Vanilla first, explicit externals | Broader desktop hosts, different forks/objects | Curated Vanilla/Organelle tiers, declared externals |
| Maintainability | Small codebase, more locally owned behavior | Large upstream/fork integration burden | Explicit adapters, pinned dependencies and conservative scope |
| Licensing | Own license unresolved; libpd/import review | GPL/JUCE/dependency review required | Prefer MIT/BSD reuse; GPL copying separately decided |
| Main risk | Reinventing lifecycle and incomplete state | Port cost without hardware/UI evidence | Boundary mismatches and overclaiming dynamic support |
| Assessment | Viable fallback with more new local work | Architectural reference, not first implementation | Best balance of proven MPC foundation and selective reuse |

These are qualitative engineering estimates, not measured CPU/memory results or delivery
dates. Source evidence is detailed in [PURE_DATA_HOSTS.md](PURE_DATA_HOSTS.md),
[MPC_GUI_AND_QLINKS.md](MPC_GUI_AND_QLINKS.md) and [REUSE_MATRIX.md](REUSE_MATRIX.md).
hvcc is a separate optional compiled-patch optimization lane, not a replacement for any
of these runtime-loading options: supported patches must be compiled ahead of time.

## Boundaries and data flow

Patch directory → offline dependency/license audit → manifest → stopped-state loader →
libpd graph. MPC parameter events → fixed mapping slots → bounded Pd messages. Pd display
messages → bounded cache → native labels. Pd MIDI events → timestamped output adapter →
explicit MPC MIDI input/CV track or external MIDI-to-CV. Audio remains stereo until a
separate host-routing test proves more. Private direct CV is a deferred experimental
adapter, disabled by default, not part of the initial Universal contract.

A manifest declares stable patch ID/version, entry patch, relative assets/search paths,
required externals, audio channel/sample-rate expectations, control receivers and ranges,
parameter indices, momentary semantics, displays, state schema, license notices and any
MIDI intent. Never execute patch-provided shell scripts. Reject paths escaping the patch
root, missing/unsupported externals and duplicate IDs with an actionable status message.
This is a proposed schema, not an existing upstream standard.

## Loader and audio safety proposal

Use explicit states: running → request stop/silence → load/validate off audio callback →
restore compatible state → activate → running. Failure returns to the previous valid
patch or silence with a visible error. A real-time callback must never wait for directory
scans, allocation-heavy graph loading, subprocesses or downloads. A fully constructed
second Pd instance and atomic handoff is only possible after MULTI support and externals
are proven isolated. Until then require transport stopped and a deliberate reload; do not
promise seamless live switching. Keep queues bounded and publish overflow counters.

## Parameter, browser and state contract

Reserve stable host indices for browser selection/load/status and a fixed pool of patch
controls. Give each pool slot a consistent host identity; update only labels/ranges within
what the host supports. Changing slot meaning can make old automation dangerous, so store
patch ID and mapping version and reject incompatible recall. MIDI Learn is a separate
explicit mapping with conflict/readback tests, not a property of every Pd knob.

Store relative patch identity, schema version and declared control values in the VST
chunk; distinguish transient buttons from persistent values. File/sample/table state needs
an explicit serialization policy and size limits. Preserve old projects when assets are
missing; never silently substitute a different patch. Implement instrument and effect
roles deliberately with separate identities if the host classification requires them.

## Clock and CV contract

MPC transport is the default authority. Feed host tempo/PPQ/play state into Pd with
explicit loop/locate/stop handling. Generate events against a bounded timeline rather
than assuming accumulated clocks cannot drift. Host MIDI forwarding, ALSA routing and
CV-track response each require an acceptance test. Use external module-local LFOs and
clock division for dense modulation. Do not map a Pd audio signal to a hardware CV port
merely because both are arrays of samples. See the CV report for private protocol limits,
electrical safety and timing measurements.

## Compatibility promise

Start with curated vanilla-Pd patches and declared Organelle subsets. No promise of
arbitrary Organelle scripts, desktop GUIs, binary externals, CV input, sample-accurate CV,
automatic universal parameter discovery or crash isolation inside the MPC process.
Preserve Delay and Granular Freezer; use a new Universal identity and folder only after
a separate implementation approval. No production implementation is included here.

## Follow-up decision: preserve C, narrow externals and patch tiers

The additional source audits strengthen Option C. Keep MPC build/skin/packaging and
libpd; compose curated, licensed Pd abstractions/control graphs and existing external
MIDI-to-CV hardware. Do not add the norns system, SuperCollider/JACK, a new graphical
framework or a Deken runtime package manager to Universal.

Before external support, resolve the current hidden Pd export policy. Static registration
of a small audited set minimizes new dependency and symbol-ownership work; a broader
shared/exported runtime is a separate isolation design. Model custom-mother PLAY patches
as a distinct tier with their own entry graph, Faust/Jam/Link and state contracts; do not
treat `main.pd` detection as sufficient. License blockers prevent integrating Jam/PLAY
at present. These changes refine scope rather than requiring a wholesale engine rewrite.
See [ADDITIONAL_SOURCE_AUDIT.md](ADDITIONAL_SOURCE_AUDIT.md).
