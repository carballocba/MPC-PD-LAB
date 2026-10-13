# Organelle compatibility and reusable components

Research date: 2026-10-13. Original MPC One, ARMv7 hard-float, Hakai 3.9.1.
Research only: no plugin/DSP/installer/system changes, hardware access or deployment.
Evidence labels: **source-verified**, **manufacturer-documented**, **upstream hardware report**,
**user-reported hardware result**, **inference/proposal**, and **not established** are distinct.
Exact source revisions and access gaps are in [SOURCES.md](SOURCES.md).

## Sources and concrete boundaries

Official OS [fw_dir/mother.pd](https://github.com/critterandguitari/Organelle_OS/blob/1f549fddc2dfd7d6ac1f2310ef01278ddf361856/fw_dir/mother.pd) and
[fw_dir/scripts/start-mother.sh](https://github.com/critterandguitari/Organelle_OS/blob/1f549fddc2dfd7d6ac1f2310ef01278ddf361856/fw_dir/scripts/start-mother.sh) separate hardware coordination from
patch DSP. Patch folders use `main.pd` and local abstractions/assets. This is a useful
convention, not a universal patch metadata/parameter schema. SuperCollider and script-driven
patches also exist: an Organelle folder is not necessarily a standalone Vanilla Pd graph.

| Concern | Existing evidence | Universal reuse/adaptation |
|---|---|---|
| Audio | mother named inL/inR and outL/outR buses | Reuse Phase 4 mother; per-port channel/gain policy |
| Knobs | knob1–4 normalized messages and override/raw variants | Keep receiver mappings, scale only at the adapter boundary |
| Aux/fs/exp | Binary controls, continuous expression; individual patches vary | Manifest says momentary/absolute/rising-edge/latch; never infer one meaning for aux |
| MIDI | notes pitch/velocity lists, raw/gated MIDI and ordinary notein/ctlin | Retain input bridge; add MIDI output hooks and tested backend separately |
| OLED | screenLine messages, LED and graphical OSC APIs | Display text/status slots initially; do not render arbitrary OLED drawings as native MPC widgets |
| Loader | OS scripts manage processes/services and hardware paths | Reuse folder conventions and audit logic, not shell kill/start scripts inside MPC |
| Presets | Patch-specific files/messages plus OS save helpers | Explicit state adapters; no universal serialize-everything assumption |
| Externals | Hardware/OS-specific Pd binaries and dependencies | Rebuild supported source for pinned ARMhf ABI; do not copy foreign .pd_linux blindly |

The official [fw_dir/scripts/killpatch.sh](https://github.com/critterandguitari/Organelle_OS/blob/1f549fddc2dfd7d6ac1f2310ef01278ddf361856/fw_dir/scripts/killpatch.sh) uses OSC quit and process
termination. Copying it into a VST could kill unrelated Pd processes and does not close an
embedded canvas safely. `save-patch.sh`, `save-new-patch.sh` and `patch_loaded.sh` are lifecycle
references; Universal should not execute them or use their absolute hardware paths.

## Reuse already demonstrated

[compat/organelle/mpc_organelle_mother.pd](https://github.com/carballocba/MPC-PD-LAB/blob/879ab14336fd5c4576ab9832127b4047aaa2a2b6/compat/organelle/mpc_organelle_mother.pd),
[ports/granular-freezer/manifest.json](https://github.com/carballocba/MPC-PD-LAB/blob/879ab14336fd5c4576ab9832127b4047aaa2a2b6/ports/granular-freezer/manifest.json), [src/organelle_engine.c](https://github.com/carballocba/MPC-PD-LAB/blob/879ab14336fd5c4576ab9832127b4047aaa2a2b6/src/organelle_engine.c)
and [scripts/prepare_organelle.py](https://github.com/carballocba/MPC-PD-LAB/blob/879ab14336fd5c4576ab9832127b4047aaa2a2b6/scripts/prepare_organelle.py) are the starting point.
C&G Granular Freezer remains byte-identical to its official revision; our mother/host and
six-control mapping are separate. The user now reports granular audio, freeze and all six
Q-Links working on the One/Hakai 3.9.1; UI still black. That does not prove other patches,
GUI rendering, dynamic loading or multiple instances.

Zynthian's alternative mother is embedded in
[presets/puredata/audio-fx/Granular_Freezer/main.pd](https://github.com/zynthian/zynthian-data/blob/4920bea5a6327824eee8500987a1d3dcd936f7fb/presets/puredata/audio-fx/Granular_Freezer/main.pd); supporting audio/MIDI
patches live under `puredata/organelle/`. It maps CCs and initialization, but its input/output
gain and modified freezer behavior differ. Study the code, preserve the original DSP and
avoid copying a whole GPL compatibility layer when the smaller existing Phase 4 boundary
already works. [README.md](https://github.com/gitnob/pd_to_zynthian_oram/blob/0a1f626ec2aafe0b1502f062182f3bb2e91c34b2/README.md) is an hvcc/DPF LV2 workflow, not a ready libpd mother.

## Proposed tier and manifest policy

| Tier | Scope | Acceptance gate |
|---|---|---|
| A | Vanilla, minimal hardware dependencies | Entry/audio/asset audit, successful load and resource cleanup |
| B | Needs Organelle mother/control translation | A + explicit symbols, edges, MIDI and initialization tests |
| C | Supported externals/assets/additional runtime adapters | B + source/ABI/license inventory, load/thread/file-I/O tests |
| D | Platform services, unavailable sources, significant UI/system dependencies | Remain unsupported until a separate demonstrated solution exists |

Manifest records: source URL/revision/file hashes, author/per-file license, entry, audio/rate,
MIDI in/out requirements, search roots, externals/source/build pins, assets/sample hashes,
control symbols/types/ranges/defaults/curve, momentary/edge semantics, displays, state policy,
backend permissions and compatibility tier. Do not let arbitrary patch code select hardware
ports silently. Port manifests are not a security sandbox.

Discovery may suggest a main.pd entry and scan literal object names; dynamic abstractions,
clone/declare paths, externals creating objects and runtime file operations defeat a complete
static dependency proof. Audit unknowns manually and capture runtime errors; do not label a
patch supported solely because no parser warning appeared.

## State and version limits

Original Organelle generations, OS versions and patch authors use different sample paths,
MIDI defaults, OLED protocols and libraries. Preserve per-patch quirks. Scalar receiver state
is not frozen audio, RNG seed, table contents, note latch or external engine state. Do not
promise project recall until every mapped component has an explicit save/restore contract.

BSD root notices do not override third-party GPL imports. Granular Freezer credits GPL
panning adaptations; retain both notices and editable source. Samples and binaries need their
own permissions. Unsupported script/service patches must not be silently rewritten into new
instruments to inflate compatibility counts.

## Follow-up: M/S/S2, PLAY and control patches

[Additional audit](ADDITIONAL_SOURCE_AUDIT.md) inspects the supplied newer manuals,
Organelle_OS loader and actual PLAY/CCquencer2/MIDI Maker downloads.
[MainMenu.cpp](https://github.com/critterandguitari/Organelle_OS/blob/1f549fddc2dfd7d6ac1f2310ef01278ddf361856/MainMenu.cpp) allows `mother.pd`-only entries and process-managed
non-Pd programs; a directory containing `main.pd` is not a Vanilla compatibility test.

**New tier distinction:** inspected PLAY/CZ has an empty `main.pd` and a custom mother
containing the actual instrument graph, eight Faust voices and Jam/Lua sequencing.
Keep this tier separate from the older curated mother/knob subset. It also needs Link,
OSC/display behavior, script/asset/preset management and licensed ABI-compatible externals.
No explicit license was found in Jam's checkout or the downloaded PLAY archive; copying
is blocked until grants are resolved. Do not run its compilation or OS scripts.

CCquencer2 and MIDI Maker already implement useful CC sequencing/audio-to-MIDI behavior.
Prefer their existing graphs over a new sequencer after dependency/license audit.
Both carry nested GPL code despite a BSD root notice. MIDI Maker audio analysis is not
CV input. Original device external binaries are not automatically usable in our hidden
libpd build. The supplied newer manual has OS 5.1/5.0 text inconsistency; record model
and patch/firmware version rather than assuming all Organelle generations are equivalent.
