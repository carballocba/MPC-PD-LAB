# PD LAB Stage 2A — Native UI Capability Probe

2026-10-16. Local branch `development/pdlab-v0.3-stage2a-ui-probe`, based on Stage 1 `c565aaf1423635c71a2c4aa1f5f902bc33c19d5e` / Draft PR #7. No push, merge, install or hardware access. No Stage 2B engine. All new source is isolated in `probes/pdlab-stage2a/`.

## What was produced

Native MPC OS3 TUI.json, PNG backgrounds/buttons/knob filmstrips, Q-Links.json, Q-Links - 8by1.json and version.xml using the already cached sd88me tools. Header is **PD LAB** only. Technical folder is `CARBALLO - VST - PD LAB UI Probe`, distinct from any working plugin. This is not an installable package: no registration metadata, installer or MPC.settings fragment is supplied.

The probe offers two predeclared visibility trees, Library and Performance, selected by a native enum. Library has four reusable 598×72 px rows, bounded labels, separate selection and explicit Apply/rearm. Labels come from two in-memory mock sets (Trim/Lowpass/Mock 3/Mock 4 versus Alt 1–4). No Stage 1 manifest/catalog is loaded. Apply increments a mock request counter, not an active patch or success flag. Performance displays 16 fixed normalized controls. Returning to Library changes only UI state. No patch loading, transport handling, DSP engine, file scanning or project recall exists.

Native binary: **macOS arm64 Mach-O dylib**, built and exercised through a small dynamic-loading C harness. It is not an MPC ARMv7 ELF. An ARMv7 hard-float-targeted freestanding **relocatable object** compiles AEffect size/offset assertions (144-byte layout, processReplacing offset 80); it is not a complete plugin and was not executed. No ARM Linux linking toolchain/runtime was used or installed in this stage. Delivering an MPC executable remains a separate build/validation task; this report does not claim it is available.

Probe identity `PDUi` / `0x50445569`, version 1, 27 experimental parameters. This new experimental identity is not MPUL, MPDL or CGFZ and does not modify their parameter counts or UIDs. No global UID registry collision guarantee is claimed. **These indices and the 27-parameter count are not the final PD LAB ABI.** The binary advertises no desktop editor or state chunks; closing/reopening resets mock data. Multiple instances have independent allocations; no libpd symbols are involved.

## Evidence and reuse

Pinned [sd88me/mpc-vst-plugins 5e3a96885b6de76a48b498821c379568a05cc006](https://github.com/sd88me/mpc-vst-plugins/tree/5e3a96885b6de76a48b498821c379568a05cc006):

- `tools/shadow_skin.py`: write_skin, list row binding, enum actions, IndexedEnabling, program_qlinks and qlink_for_slot.
- `tools/skin_check.py`: native touch/edge/mapping checks.
- `tools/studio.py`: native asset compositor, including visibility variants; dynamic Value labels deliberately remain placeholders.
- `tools/shadow_art.c` and force-shadow renderer: same one-call static TTF rendering adaptation already used by v0.2; no change to upstream checkout.
- `docs/NOTES.md`: Force observations about conditional visibility, dynamic strings and host notification/echo problems; not MPC One results. In particular later notes qualify the initial claim that labels refresh unconditionally.
- [v0.2 wrapper at 062ac5c](https://github.com/carballocba/MPC-PD-LAB/blob/062ac5cec83b8ab469ade5f1df059a97cb1d2634/src/universal/vst2_wrap.c): reuse only the AEffect declaration/size/offset contract, not its engine. `abi.h` records this derivation.

MIT notices for sd88me/force-shadow and Titillium OFL are included. Runtime ABI declarations derive from the existing bridge, not a newly downloaded SDK. This does not settle all future VST2 distribution licensing questions. Stage 1's inherited DSP MIT authorship concern is unchanged; no new DSP code or patches are distributed in this probe.

## Experimental parameter contract and Q-Links

| Indices | Purpose | Behavior |
|---|---|---|
| 0 | View | Library=0, Performance=1; static visibility |
| 1 | Row select | Four normalized discrete choices; Q-Link target |
| 2 | Mock set | Two choices, changes cached label source |
| 3 | Apply | Rising edge counts request; explicit low rearm; never loads |
| 4 | Text probe | Rising edge increments mock display tick; rearm required |
| 5 | Status | Read-only `M###R#` (mock ticks/request counter), never READY/LOADED |
| 6–9 | Row display/touch | Bounded display label; high touch selects row, low ignored |
| 10–25 | Controls 0–15 | Fixed 1:1 normalized slots; only these marked automatable |
| 26 | Chosen label | Read-only selected mock label |

Native VST names/displays are bounded to seven characters plus NUL. Long patch names and arbitrary enums are deliberately not claimed. All row labels are mock strings ≤7 chars. Setter accepts only finite values, clamps to [0,1]; telemetry writes are ignored. Native list highlighting may be stale until the host re-queries values: it is an explicit probe question, not a solved feature.

Screen-mode Library mapping: Q13→select(1), Q9→apply(3), Q5→mockset(2), Q1→view(0). Screen-mode Performance and fixed Program map: Q13/Q9/Q5/Q1→10/11/12/13; Q14/Q10/Q6/Q2→14/15/16/17; Q15/Q11/Q7/Q3→18/19/20/21; Q16/Q12/Q8/Q4→22/23/24/25. `Q-Links - 8by1.json` uses the same generated map as the normal file, as upstream does; physical 8by1 operation is unverified.

There are two native Q-Link subpages named LIBRARY/PERFORMANCE and an independent View parameter. **Setting View does not select the native Q-Link subpage or physical bank.** The asset generator repeats the layout for these subpages; it does not supply a verified bidirectional binding. A future hardware test must manually distinguish View, native subpage and physical bank. This limitation is the reason the final ABI cannot be frozen from this demonstration.

## Refresh experiments and callback boundary

1. Change Mock A/B: all four row texts and selected label should be re-read by the host.
2. Select by touch and then by Q-Link: compare chosen text and row highlight, including selection above the current row and toggling the already selected row.
3. Press Text Probe high/rearm/press again: status changes M000→M001→M002 in the plugin's getter. Check whether visible text changes without an unrelated touch.
4. Return between views: test IndexedEnabling invalidation, hidden hit regions and value retention.

The getter is proven to return updated bounded strings in the native harness. **Visible native refresh is not proven.** There is no thread/timer, host callback or `audioMasterUpdateDisplay` in the probe. It intentionally tests the passive path first; it does not establish a supported non-RT notification dispatcher. There is no autonomous background telemetry timer. A host idle/GUI-thread callback experiment requires a documented context and separate review; do not copy upstream processReplacing housekeeping as a workaround.

Audio is bit-exact float stereo passthrough or silence for missing inputs, independent of every parameter. Callback source audit finds no allocation/free, locks, atomics, filesystem, logging, waits or host calls in render. Allocation/free occur at instance construction/destruction only; setters/getters use lock-free integer atomics. Source audit is narrow, not a formal whole-program RT proof. Native ABI loading and parameter getter tests do not exercise MPC's audio worker scheduling or host reentrancy.

## Validation actually performed

| Check | Result | What it establishes |
|---|---|---|
| Native generation / skin_check | PASS | Structurally valid upstream-generated skin and local bounds checks |
| 15 PNG assets | PASS | Decode/verify and ≤16384 dimensions; no on-device GPU proof |
| Touch regions | PASS | 29 visible interactive regions across two modes, each ≥64 px; 3 passive readout regions; no flagged overlaps/edges |
| Native mapping contract | PASS | 27 mock params; two screen maps; exactly slots 10–25 for Performance/Program; identical 8by1 file |
| Visibility trees | PASS | Expected IndexedEnabling handles; four rows in Library, zero in Performance |
| Dynamic binary harness | PASS | AEffect entry/UID/count, 1,000 view changes with exact L/R samples, mock rows, rearm, bounded buffers/canaries, fixed control retention and null-input silence |
| ARM ABI compile | PASS | 32-bit declaration size/offset only; not linked, loaded or run |
| Stage 1 regression | 43/43 PASS | Offline library behavior remains intact |
| Preservation | PASS | All 106 tracked files from Stage 1 byte-identical, including stable sources/assets/configuration |
| Visual review | DONE offline | Rendered both views; adjusted first knob row to avoid decorative title overlap not caught by automated checks |
| MPC One / Hakai 3.9.1 | NOT TESTED | No installation, host registration, UI/audio or real-time evidence |

Raw `native-assets-*.png` previews contain placeholder boxes for native live values. `review-library.png` and `review-performance.png` overlay explicit illustrative mock text and add **RENDERED MOCK DATA / NOT MPC HARDWARE**. They are design previews, not screenshots or a simulation of successful loading. Native knob positions shown at frame zero agree with illustrative default values. Host fonts/selection borders may differ. The review legend is outside the 1280×628 native content area, not a plugin subtitle.

## Recommendation: 16 slots versus fallback

The asset format and fixed maps can express 16 distinct musical slots. Maintain this as the **preferred eventual capacity**, with permanent indices and four physical Q-Link banks. Never repurpose the same four automation parameters when changing UI page.

For the first real loader integration, choose **four permanent musical slots and one Performance page** unless hardware validates the 16-slot workflow and view/bank coordination first. Four are an ergonomic fallback, not proof that the host only supports four parameters. Navigation still needs its own native page/map; four slots do not automatically solve contextual switching. If native subpage/context switching is unreliable, keep a static Program map and touch navigation, explicitly revise the Q-Link navigation requirement before ABI freeze. No final count or reserved-index budget is set here.

Required gates for 16: all 16 parameter values independently editable/retained/automated; physical bank order matches labels; no hidden control accidentally operated after view change; native screen and program mappings consistent; text/highlights refresh with bounded calls in a supported non-RT context; no frame drops/audio glitches during navigation. Only after these gates should pages of four replace the dense 16-knob diagnostic grid.

## Hardware-only questions and future protocol (not executed)

Hardware access and installation require separate approval. There is currently no MPC-executable probe artifact, so these are prerequisites for future work, not instructions to install this ZIP.

- Build a separate ARMv7 ELF probe with correct hard-float ABI, exports/DT_NEEDED/GLIBC validated and dedicated registration metadata. Confirm experimental UID/technical name isolation before any install.
- In an approved empty test project at 44.1 kHz, verify the 1280×628 offset, text legibility, touch hitboxes and no hidden Library rows intercepting Performance taps.
- Compare passive label redraw, numeric-value invalidation and selection borders while changing mock set through touch/Q-Link. Record stale fields separately; do not infer success from the getter log.
- Determine how native subpage selection changes Q-Link maps and whether View can follow it through a supported API. Test all four One physical banks, then Program versus Screen mode. Do not assume IndexedEnabling changes Q-Link binding.
- Test Apply high/rearm, repeated selection and fast gestures. Confirm that mock count increments only as specified; **no patch should ever load**.
- Record continuous L/R loopback through navigation/text probes; measure dropouts and UI load under repeated gestures. Local bit-exact tests do not guarantee host real-time stability.
- Confirm project reopen behavior is intentionally reset (no chunks), multiple instances and unload/close behavior; do not use working projects.

No pd files, unknown externals or scripts are loaded. Stage 1 library, Delay, Granular Freezer, v0.1/v0.2 remain untouched. This stage stops at assets, standalone probe and report; no Stage 2B implementation is authorized by this result.

## Reproduce offline

Use the already available Python environment containing Pillow; no install is required or performed. From repository root:

```sh
../gui-tools/bin/python probes/pdlab-stage2a/run_checks.py --upstream ../upstream --out ../new-fresh-probe-output
```

Requires cached upstream at the exact pin, native cc/clang, and a fresh output path. run_checks builds only the new probe, verifies original files against the Stage 1 commit, runs native harness and Stage 1 tests, compiles the ARM ABI-only object, validates generated assets/maps and emits logs/JSON. Native binary naming reports the actual platform; tests are intended for macOS/Linux, but this stage ran on macOS arm64 only. No dependencies are fetched and no installers are run. This is review tooling, not a production framework.
