# Universal Loader v0.2 native GUI architecture

Base: exact PR #5 head `2b7d25b4738145c279dd4ab511d73464976b3ebf`.
Branch: development/universal-loader-gui-v0.2. This is a real native skin and VST
parameter integration, not a desktop editor, external browser or hardware result.

## Evidence and selected integration

All community-tool references below are pinned to sd88me/mpc-vst-plugins commit
[5e3a96885b6de76a48b498821c379568a05cc006](https://github.com/sd88me/mpc-vst-plugins/tree/5e3a96885b6de76a48b498821c379568a05cc006).
The actual implementations inspected are:

- [tools/shadow_skin.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/shadow_skin.py): write_skin, _placed, _button, _action, enum groups, readout Label Value, knob filmstrips and qlink_for_slot. Native TUI components bind Data handles to `Parameter N`.
- [poc/textprobe.c](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/poc/textprobe.c) and [docs/NOTES.md](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/docs/NOTES.md): upstream Force hardware report says MPC polls effGetParamDisplay for native Label Value. Cross-parameter degenerate readouts may need UpdateDisplay; names need refresh. These are upstream reports, not our MPC One validation.
- [wrapper/vst2_wrap.c](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/wrapper/vst2_wrap.c): normalized parameters, string displays, deferred UpdateDisplay and momentary handling. Its general mutex bridge is not imported into our render path.
- [tools/gen_vst.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/gen_vst.py), [tools/release.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release.py) and tools/release/install.sh: effect registration, portable folder/skin layout and supported installation operation. No installer is executed here.
- [tools/skin_check.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/skin_check.py), tools/studio.py preview and docs/SKIN_STUDIO.md: native resources, actual asset previews, touch overlap/edge/Q-Link validation.
- [jacob-sabella/mpc-vst-nam](https://github.com/jacob-sabella/mpc-vst-nam/tree/cf2548f796c96b8ca4fdd4582af8514d82a24830) vst/nam_vst.cpp and [sd88me/mpc-vst-cratedigger](https://github.com/sd88me/mpc-vst-cratedigger/tree/e8ed11227d36e05e7c5245288745c3634c3adbf8) vst/cratedigger_vst.cpp: cached model/browser/status parameter displays in real community ports. Network/download code is not reused.

The accessible [henning/Hakai-MPC](https://github.com/henning/Hakai-MPC/tree/0c7008c7a5cd62cd2dc331e7b657bdcee19b4625)
README and az01-launch-MPC describe old Hakai 2.9 product-code spoofing, content
mounting and MPC launch. They do not publish the modern 3.9/3.91 host GUI source.
That gap remains explicit. No Hakai-specific drawing API or undocumented desktop
editor embedding is invented. Native MPC content skins are the strongest
source-backed community route; Hakai 3.9/3.91 behavior needs hardware validation.

## Native page and data boundary

The generator emits a real nonempty MPC OS3 TUI.json, PNG assets, version.xml,
Q-Links.json and 8by1 map for the 1280×628 content area (Y offset 86 in the
1280×800 display). It imports the established Akai generic overlays. There is no
runtime web page, framebuffer, remote display, VST editor or new GUI service.

| Index | Purpose | Write behavior |
|---|---|---|
| 0 | Patch selection: Trim/Lowpass | Unchanged engine setter |
| 1 | Apply | Unchanged rising-edge setter; explicitly rearm low |
| 2 / 3 | Gain / Mix | Unchanged, automatable, Q-Link compatible |
| 4 | Active patch name | Read-only; committed ID → Trim/Lowpass |
| 5 | Engine status | Read-only; actual engine status, no estimated progress |
| 6 / 7 | ID head/tail | Read-only; two seven-character segments |
| 8 | Revision | Read-only; seven-character display, LONGREV for longer values |

The first four indices, MPUL identity and UL1 project chunk stay stable. Five
read-only display slots are appended; **numParams changes from 4 to 9**. This
intentional wrapper ABI extension needs old-project recall tests on hardware.
It does not claim arbitrary host upgrade compatibility. UI telemetry is excluded
from automation and state chunks; writes to it do nothing.

Telemetry getters read existing atomics. Display text uses the bounded committed
chunk snapshot; there is no Pd call or file scan. VST display/name writes remain
within eight-byte buffers, unlike community ports that rely on larger MPC
buffers. Curated IDs fit two segments; LONG ID/LONGREV expose longer metadata
rather than pretending it was fully displayed. No dynamic parameter names or
cross-parameter get= readouts are used, and no audioMasterUpdateDisplay callback
is added. Whether One refreshes all readouts as the upstream Force does is a
hardware gate; a timer/reentrant callback is not invented as a fallback.

## Touch semantics and limitations

Trim/Lowpass are native radio buttons bound directly to Patch. Previous/Next are
another radio group targeting the same parameter: with exactly two approved
patches, Previous selects Trim and Next selects Lowpass. Neither loads a graph.
Both controls share host data binding, including Q-Link changes.

The supported native button action is Mouse Down → Toggle Switch. Apply first
sets 1 and requests the existing engine transaction. Its lit caption becomes
REARM; the next tap sets 0, then Apply can request again. This explicit rearm
preserves v0.1's edge semantics, including held-high protection and cancellation
by selecting/applying the active patch. It is the reliable native alternative to
an unverified Mouse Up/momentary handler. It is deliberately not an auto-trigger
on every setParameter call.

Knobs are native MPC Knob widgets and retain Q13=Patch, Q9=Apply, Q5=Gain,
Q1=Mix in both maps. A future approved catalog can reuse the same enum/list/popup
primitives; expanding beyond two patches still requires extending the audited
engine/catalog contract and the static approved UI options. No arbitrary browser
or filesystem scan is introduced. Normalized parameters and stable IDs remain
the separation between browser presentation and loading.

No meters or waveforms are added: the engine has no validated level telemetry,
and upstream notes report the native Meter component can break a whole screen.
WAIT/LOAD/error/rollback codes and the separately committed active name are real
state, not fake progress. Long errors remain in loader.log. DSP, ownership,
stopped-only switching, cancellation, rollback and snapshot rules are unchanged.
