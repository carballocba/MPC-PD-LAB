# Native MPC UI, Q-Links and universal parameter mapping

Research date: 2026-10-13. Original MPC One, ARMv7 hard-float, Hakai 3.9.1.
Research only: no plugin/DSP/installer/system changes, hardware access or deployment.
Evidence labels: **source-verified**, **manufacturer-documented**, **upstream hardware report**,
**user-reported hardware result**, **inference/proposal**, and **not established** are distinct.
Exact source revisions and access gaps are in [SOURCES.md](SOURCES.md).

## What the source and hardware evidence establish

The community UI is a **native MPC content skin**, not a desktop VST editor painted into the
MPC display. [tools/shadow_skin.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/shadow_skin.py) `write_skin` emits TUI.json, version.xml
and Q-Link maps. [tools/gen_vst.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/gen_vst.py) generates parameter definitions and
registry metadata. MPC binds declared widgets to VST parameter indices. Plugin-meta.xml's
portable library path and VST identity are registration concerns, separate from widget layout.

Our Phase 3/4 skins deliberately contain empty tabs; the user reports black screen but six
working Q-Links on Granular Freezer. **Working Q-Links do not demonstrate a functional TUI**,
and a black page does not demonstrate that Hakai lacks parameter support. This new report
supersedes the earlier phase documents' pending-Q-Link status for that exact six-param plugin.

| Need | Source implementation to reuse | Actual boundary |
|---|---|---|
| Native widgets/pages | [tools/shadow_skin.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/shadow_skin.py), studio/layout tools | Static declarative page definitions; per-widget schema/version matters |
| Param names/values | [wrapper/vst2_wrap.c](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/wrapper/vst2_wrap.c) effGetParamName/Display, `<key>_name`/`<key>_display` | Dynamic text over fixed indices; not proof of host-supported dynamic parameter counts |
| Option lists/popups | gen_vst + wrapper option metadata and open/close parameters | Reuse bounded menus/pages; no arbitrary text-input widget established |
| Browser rows | [vst/nam_vst.cpp](https://github.com/jacob-sabella/mpc-vst-nam/blob/cf2548f796c96b8ca4fdd4582af8514d82a24830/vst/nam_vst.cpp) model slots/page selection and display refresh | Adapt directory/list state; do not copy network downloads as part of a local patch browser |
| Async status/display | [vst/cratedigger_vst.cpp](https://github.com/sd88me/mpc-vst-cratedigger/blob/e8ed11227d36e05e7c5245288745c3634c3adbf8/vst/cratedigger_vst.cpp) engine status + refresh | Separate worker results from callback-time host updates |
| Bank/preset selection | [src/dsp/dx7_plugin.cpp](https://github.com/sd88me/mpc-vst-dx7/blob/400fa20a506c8bc7c72105358ca85f27eefe3315/src/dsp/dx7_plugin.cpp) plus wrapper program support | Adapt selector semantics and stable IDs; sound banks are not Pd patch folders |
| Q-Link maps | shadow_skin outputs Q-Links.json and 8by1 variant | Generate real bindings; verify on One, page selection and value feedback |
| Automation/state | wrapper chunk/state and setter/getter | Fixed index identity; store patch-specific mapping version and values |

No inspected interface allows a VST to arbitrarily replace the host's widget tree while
running. Rewriting TUI files during live processing is not an established dynamic-UI API.
Do not build a new framebuffer/remote desktop or preload drawing subsystem as the first GUI.
MPC OS2 versus OS3 JSON formats differ; target 3.9.1 explicitly and preserve schema versioning.

## Recommended browser and controls

Compose existing mechanisms: one static browser page with bounded row/slot parameters,
page-up/down, refresh and a deliberate load trigger; one fixed performance parameter bank;
status/error text and patch-name readout. Worker scans a permitted patch root and caches a
bounded list. Selection is a request, not a disk load in the parameter callback. Maintain
stable patch IDs instead of persisting mutable row numbers.

A **fixed host parameter pool mapped dynamically to Pd receivers** is the strongest candidate.
Start with a small pool whose layout/Q-Link behavior can be measured; the existing six-param
result is a baseline, not evidence for an arbitrary 128-control pool. Freeze the number/order
of host parameters for a released plugin UID. Change ranges/labels inside the adapter with
normalized host values, and persist mapping schema/version. Update text via existing getters
and display notifications; whether Hakai refreshes every relevant label must be tested.

Arbitrary Pd patches do not expose a common parameter registry. Camomile's `param` message
and PlugData's explicit `[param]` metadata are opt-in host conventions, not discoverable
metadata in every Vanilla patch. Use port manifests/manual mapping first. GUI-object scanning
may suggest receivers but misses expressions, custom abstractions and controls lacking sends.
Never change original DSP merely to force automatic discovery.

## MIDI Learn and semantic mapping

Adapt a bounded controller-to-slot map from the wrapper's CC/NRPN path; inspect range/order
and feedback before expanding. Separate host automation, incoming MIDI, physical Q-Link and
patch-generated parameter changes to avoid feedback loops. Learn mode should record source
port/channel/controller and latch one target, with clear cancel/reset. Enforce affine/log
scaling, limits, quantization and mutually exclusive momentary/edge/latch policies in metadata.

A button intended to emit a single trigger is not an automatable absolute toggle. The working
Freezer adapter explicitly converts absolute Freeze changes into original aux edges; reuse
that distinction for future ports. Do not infer every patch's aux/fs behavior from Freezer.

## UI acceptance tests before claims

Verify widget page and Q-Link bank navigation, fixed indices after reload, label refresh,
readback/automation, MIDI Learn feedback, browser pagination, missing-root recovery, and
save/reopen identity. Repeat with multiple project copies and a patch switch at stop. Keep
all existing Delay/Freezer installations separate. No UI code was changed in this research.

## Follow-up: supplied build/workflow pages and monome controls

[tools/studio.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/studio.py) and [docs/SKIN_STUDIO.md](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/docs/SKIN_STUDIO.md) provide an
existing computer-side skin authoring/preview path. HTML artwork tooling renders static
assets during build; it does not install a browser-based runtime GUI on the MPC.
Retain native TUI/Q-Link generation and fixed host slots. Norns' OLED/encoder/file-selector
code is tied to its own event/menu system and supplies no native MPC UI or Q-Link API.

[lua/core/paramset.lua](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/lua/core/paramset.lua), [lua/core/pmap.lua](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/lua/core/pmap.lua) and
`lua/core/menu/params.lua` are concrete parameter-ID, MIDI Learn, range/conflict and
momentary-state precedents, with GPL and Lua/norns dependencies. Adapt behavior only
within an approved license/integration plan; existing MPC browser patterns cost less.
Organelle CC21–25 can overlap wrapper CC mappings; avoid two adapters acting on one
message and document the selected mapping mode. See [additional audit](ADDITIONAL_SOURCE_AUDIT.md).
