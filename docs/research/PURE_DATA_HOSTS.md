# Pure Data hosts: runtime loading versus compiled patches

Research date: 2026-10-13. Original MPC One, ARMv7 hard-float, Hakai 3.9.1.
Research only: no plugin/DSP/installer/system changes, hardware access or deployment.
Evidence labels: **source-verified**, **manufacturer-documented**, **upstream hardware report**,
**user-reported hardware result**, **inference/proposal**, and **not established** are distinct.
Exact source revisions and access gaps are in [SOURCES.md](SOURCES.md).

## Comparative result

| System | Inspected implementation | Formats / loading model | ARMv7/MPC reuse decision |
|---|---|---|---|
| libpd | [libpd_wrapper/z_libpd.h](https://github.com/libpd/libpd/blob/ba0dc63262901d658af8bbda5e619a60fa975e78/libpd_wrapper/z_libpd.h); [libpd_wrapper/z_libpd.c](https://github.com/libpd/libpd/blob/ba0dc63262901d658af8bbda5e619a60fa975e78/libpd_wrapper/z_libpd.c) | Runtime open/close/search paths, MIDI/hooks, float/short processing; no GUI or VST wrapper | REUSE DIRECTLY: pinned existing build; add loading policy rather than a new Pd engine |
| Pure Data standalone | [src/g_canvas.c](https://github.com/pure-data/pure-data/blob/f009fd8d7b537e209e09898d487fdf1bf547da2b/src/g_canvas.c), [src/s_loader.c](https://github.com/pure-data/pure-data/blob/f009fd8d7b537e209e09898d487fdf1bf547da2b/src/s_loader.c) | Full Pd process, GUI/system backends and dynamic external loader | Architecture/reference; do not launch a competing ALSA audio process inside MPC |
| Camomile | [Source/PluginProcessor.cpp](https://github.com/pierreguillot/Camomile/blob/03fe38053229e37fb98b89049b0d8bdf35213c14/Source/PluginProcessor.cpp), [Source/Pd/PdInstance.cpp](https://github.com/pierreguillot/Camomile/blob/03fe38053229e37fb98b89049b0d8bdf35213c14/Source/Pd/PdInstance.cpp) | libpd-derived runtime plugin, reload/state/message parameter machinery | ADAPT IDEAS/CODE subject to GPL; current CMake targets AU/VST3, not a supplied ARMv7 MPC VST2 |
| PlugData | [Source/PluginProcessor.cpp](https://github.com/plugdata-team/plugdata/blob/4e6843671e1561d666d9507d9e5e7a7b7dc7836b/Source/PluginProcessor.cpp), [Source/Pd/Instance.cpp](https://github.com/plugdata-team/plugdata/blob/4e6843671e1561d666d9507d9e5e7a7b7dc7836b/Source/Pd/Instance.cpp) | Editable runtime Pd environment, plugin/standalone, large JUCE/Pd ecosystem | Architecture inspiration first; full port carries Pd/JUCE forks, externals and desktop UI cost |
| hvcc / Heavy | [hvcc/generators/c2dpf/c2dpf.py](https://github.com/Wasted-Audio/hvcc/blob/14812fb1bf6ec62a17e46db9835f8bda8fd9dddb/hvcc/generators/c2dpf/c2dpf.py), [hvcc/generators/ir2c/static/HvHeavy.cpp](https://github.com/Wasted-Audio/hvcc/blob/14812fb1bf6ec62a17e46db9835f8bda8fd9dddb/hvcc/generators/ir2c/static/HvHeavy.cpp) | Supported Pd subset compiled into C/C++; DPF generator builds wrappers | Good optional offline optimization later; not a runtime universal patch loader |
| Zynthian Pd/mother | [presets/puredata/audio-fx/Granular_Freezer/main.pd](https://github.com/zynthian/zynthian-data/blob/4920bea5a6327824eee8500987a1d3dcd936f7fb/presets/puredata/audio-fx/Granular_Freezer/main.pd) | Pd process plus hardware-specific MIDI/audio conventions | Compose protocol/patch ideas; do not import Zynthian system/JACK setup into MPC |
| Our Phase 4 | [src/organelle_engine.c](https://github.com/carballocba/MPC-PD-LAB/blob/879ab14336fd5c4576ab9832127b4047aaa2a2b6/src/organelle_engine.c) | One compiled mapping per port, runtime original Pd DSP | Working foundation; dynamic selection/mapping is still new adaptation |

## Actual lifecycle/state patterns inspected

Camomile's `reloadPatch` suspends processing, releases resources, drains queued messages,
captures state, opens the patch, restores state and prepares audio. Its `PdInstance` selects
its Pd instance and closes the prior canvas before creating a new one. This is useful evidence
for ordering, not a worker-thread drop-in: MessageManagerLock and its Pd fork are specific to
that host. The current CMake explicitly lists AU/VST3; a `VST2_CATEGORY` setting does not
mean the build emits VST2. See [CMakeLists.txt](https://github.com/pierreguillot/Camomile/blob/03fe38053229e37fb98b89049b0d8bdf35213c14/CMakeLists.txt) and
[Source/PluginProcessorReceive.cpp](https://github.com/pierreguillot/Camomile/blob/03fe38053229e37fb98b89049b0d8bdf35213c14/Source/PluginProcessorReceive.cpp) for its `param` protocol.

PlugData's processor serializes patch contents/locations and state and uses loading/suspend
paths; Instance opens patch canvases and integrates a rich editor. Its `param-help.pd` describes
explicit named automation parameters, not automatic discovery of every existing patch knob.
See [Resources/Patches/param-help.pd](https://github.com/plugdata-team/plugdata/blob/4e6843671e1561d666d9507d9e5e7a7b7dc7836b/Resources/Patches/param-help.pd), [CMakeLists.txt](https://github.com/plugdata-team/plugdata/blob/4e6843671e1561d666d9507d9e5e7a7b7dc7836b/CMakeLists.txt),
[.gitmodules](https://github.com/plugdata-team/plugdata/blob/4e6843671e1561d666d9507d9e5e7a7b7dc7836b/.gitmodules). Current packaging/install logic includes VST3/LV2/CLAP;
these are not formats the present MPC VST2 host automatically loads. The desktop editor's
X11/desktop assumptions and dependency/submodule footprint need a separate ARM audit.
No RAM/CPU figures on the One were measured; do not invent estimates.

Heavy translates a supported graph into compiled DSP. Object coverage differs from Vanilla;
file/dynamic patch operations and unsupported externals cannot be assumed. Compiler GPL and
ISC-style runtime-file notices differ; inspect generated artifacts/templates individually.
A DPF VST build still needs an MPC-compatible format, ABI, resource packaging and native skin.
Compiling known patches may reduce interpreter work but defeats unrestricted no-rebuild loading.

## Real-time/resource constraints for a reusable host

The libpd API supplies primitives, not a safe reload transaction, dependency resolver or sandbox.
Never call openfile/closefile/search-path updates while another thread is processing that same
Pd instance. Our baseline builds MULTI=false. Enabling MULTI/new instances requires auditing
Pd globals and every external; it does not automatically make two arbitrary patches safe.

Proposed initial policy: one patch active; loading happens only after callback quiescence is
acknowledged, with bounded silence during the transition. Loader work must not run inside
set_param while the inherited wrapper holds its audio-shared mutex. Account for native
loadbangs that allocate/read samples, synchronous soundfiler, stream threads, DSP rebuilds,
status hooks and stale bindings. Capture diagnostics outside audio logging; success from
openfile alone does not prove all objects instantiated.

Full float libpd processing exists and is a better eventual universal audio/CV transport than
int16 conversion. Keep the proven short interface initially; a float conversion is a separate,
measured change, not a cosmetic refactor. Arbitrary host buffers/rates and MIDI offsets need
explicit adaptation, tests and stated fallback behavior.

A malformed Pd graph, external crash or unbounded message loop can take down an in-process
host. Error hooks and a CPU meter cannot catch/preempt every such failure. A separate Pd worker
process could isolate crashes, but adds IPC/audio buffering and recovery complexity and cannot
simply access MPC's privately owned controller fd. This is a future trade-off, not required
for the first curated loader.

## Recommendation

Retain libpd and the MPC wrapper. Borrow scoped lifecycle/state concepts from Camomile/
PlugData only after license and fork/API review; do not transplant their GUI or promise full
external coverage. Use manifests for curated runtime patches, and reserve Heavy for a separate
optional precompiled tier. This achieves the no-per-patch-VST objective with the least host risk.

## Follow-up: Vanilla and Deken

[Additional audit](ADDITIONAL_SOURCE_AUDIT.md) follows puredata.info to newer Vanilla
source and Deken. Keep the working embedded Pd pin; no upgrade was performed.
[src/s_loader.c](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/src/s_loader.c) shows global external loading/caching;
[src/g_readwrite.c](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/src/g_readwrite.c) shows cooperative `[savestate]`, not a VST chunk API.
[developer/src/deken/deken.hy](https://github.com/pure-data/deken/blob/785862399822b6d12f4804cf7b12f5ed55ee10c4/developer/src/deken/deken.hy) and its Tcl GUI distinguish OS/CPU/float
width. Reuse that vocabulary for offline preflight, while still checking hard-float,
instruction set, glibc/GLIBCXX, Pd ABI and imported symbols.

A concrete integration blocker is now recorded: our hidden-symbol/static libpd build
cannot be assumed to satisfy ordinary `.pd_linux` Pd imports. Start with audited static
registration instead of promising arbitrary dynamic externals. Norns is Lua/SuperCollider
and softcut is looping DSP; neither is another dynamic Pd host.
