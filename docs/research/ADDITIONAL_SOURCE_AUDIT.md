# Additional source audit: Pd, Organelle, MPC tooling and monome

Research date: 2026-10-13. Research only; no production changes, dependency integration,
hardware access or deployment. The six supplied links are documentation/organization
entry points, not six interchangeable plugin repositories. This report follows their
relevant implementation sources. Source inspection and static ELF analysis do not prove
runtime compatibility. Refer to SOURCES.md for baseline and follow-up coverage limits.

## Entry points and relationship to earlier research

| Supplied URL | Implementation sources followed | Change to previous conclusions |
|---|---|---|
| [puredata.info](https://puredata.info/) | Pd Vanilla, existing libpd checkout, Deken | Retain libpd; extend external audit with CPU/float/ABI checks, not a desktop installer |
| [Organelle M/S/S2 manual](https://critterandguitari.github.io/cg-docs/Organelle/og_sms2/) | cg-docs, Organelle_OS menu/loader/mother | Distinguish OS generations, custom mothers, program-change favourites and whole-process reload |
| [Organelle patch catalog](https://critterandguitari.github.io/cg-docs/Organelle/patches/) | Catalog generator; downloaded PLAY, CCquencer2, MIDI Maker source assets; jampd | PLAY is Faust/Lua plus Pd, not automatically Vanilla; existing MIDI/CC generators deserve curated ports |
| [sd88me build](https://sd88me.github.io/mpc-vst-plugins/build.html) | build_port, gen_vst, wrapper, skin generator/Studio, host_test | Reuse MPC tools rather than creating another GUI or build framework |
| [sd88me workflow](https://sd88me.github.io/mpc-vst-plugins/workflow.html) | reusable CI, release, catalog checker, installer fixtures | Separate desktop tests, catalog acceptance, draft artifacts and real hardware acceptance |
| [monome](https://monome.org/) | norns, crow (previous audit), softcut, serialosc, libmonome, Pd grid studies, Ansible, Teletype | Valuable external control ecosystem and patterns; no ready-made MPC Pd host or native MPC CV driver |

## Exact revisions and license boundaries

Tip dates are activity observations, not maintenance guarantees or release certification.
Source is publicly available for all listed repositories; documentation assets and
third-party code may carry different grants. No dependency was added to the plugin.

| Repository / maintainer | Inspected revision | Tip date | License evidence / caveat |
|---|---|---|---|
| [pure-data/pure-data](https://github.com/pure-data/pure-data/tree/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9) / pure-data | `f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9` | 2026-10-05 | Standard Improved BSD except imports |
| [critterandguitari/cg-docs](https://github.com/critterandguitari/cg-docs/tree/b6f2633e7c39f78f7d924f9bf6084eaa0144bbff) / critterandguitari | `b6f2633e7c39f78f7d924f9bf6084eaa0144bbff` | 2026-10-02 | No repository-wide license found; documentation is not a patch license |
| [owenosborn/jampd](https://github.com/owenosborn/jampd/tree/1c061e8efa423399d4045958fb4e9d3c8fb294d2) / owenosborn | `1c061e8efa423399d4045958fb4e9d3c8fb294d2` | 2026-04-12 | No explicit license found in checkout |
| [pure-data/deken](https://github.com/pure-data/deken/tree/785862399822b6d12f4804cf7b12f5ed55ee10c4) / pure-data | `785862399822b6d12f4804cf7b12f5ed55ee10c4` | 2026-08-28 | Standard Improved BSD except imports |
| [monome/norns](https://github.com/monome/norns/tree/6302a0d4dcb85ab3b701a4d7268863092b693caf) / monome | `6302a0d4dcb85ab3b701a4d7268863092b693caf` | 2026-10-11 | GPL-3.0 root; dependencies separately licensed |
| [monome/softcut-lib](https://github.com/monome/softcut-lib/tree/14241f2a94a7cd2e6c91b9bdc4e500d68abef907) / monome | `14241f2a94a7cd2e6c91b9bdc4e500d68abef907` | 2024-12-31 | GPL-3.0 root |
| [monome/libmonome](https://github.com/monome/libmonome/tree/9055ea22fd824839be821c10f6e9b13e9d2aa3cd) / monome | `9055ea22fd824839be821c10f6e9b13e9d2aa3cd` | 2026-08-27 | ISC-style COPYRIGHT |
| [monome/serialosc](https://github.com/monome/serialosc/tree/c96ea389dbf82c84d17f6f7adddaf311aed49438) / monome | `c96ea389dbf82c84d17f6f7adddaf311aed49438` | 2026-08-27 | ISC-style core; static dependencies include LGPL-2.1-or-later liblo |
| [monome/docs](https://github.com/monome/docs/tree/de746029ef57244036df2a206e5b894a29b5e0f6) / monome | `de746029ef57244036df2a206e5b894a29b5e0f6` | 2026-10-01 | CC-BY-4.0 LICENSE; MIT LICENSE.txt is attributed to theme author |
| [monome/teletype](https://github.com/monome/teletype/tree/e390bacdf024f2234e2ade6a4821773b9a80a322) / monome | `e390bacdf024f2234e2ade6a4821773b9a80a322` | 2026-06-24 | GPL-2.0 root; per-file/vendor review required |
| [monome/ansible](https://github.com/monome/ansible/tree/9796656cbbe9eb0227d380d555e31192f0cf32bf) / monome | `9796656cbbe9eb0227d380d555e31192f0cf32bf` | 2025-09-05 | GPL-2.0 root; per-file/vendor review required |

## Pure Data portal → Vanilla and libpd

**Architecture/source:** Pd Vanilla separates DSP/message graph, platform I/O and its
Tcl/Tk GUI. Inspect [src/m_class.c](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/src/m_class.c),
[src/g_canvas.c](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/src/g_canvas.c), [src/g_readwrite.c](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/src/g_readwrite.c),
[src/s_loader.c](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/src/s_loader.c) and [INSTALL.txt](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/INSTALL.txt).
The newly inspected tip says 0.57-0; our libpd embeds an older pinned Pd revision.
Do not silently upgrade the working engine merely because a newer tip exists.

**Direct reuse:** keep existing libpd processing, open/close graph APIs, explicit search
paths and MIDI hooks. Vanilla abstractions/assets can remain unchanged when their
receivers, objects and license are compatible. Native `[savestate]` handles cooperative
abstraction data during Pd patch save/restore; it does not automatically serialize a
VST project's graph, samples or arbitrary Lua state. A non-null opened graph does not
prove every object instantiated: [src/g_text.c](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/src/g_text.c) reports creation errors.
Capture diagnostics and use a declared object/dependency audit plus functional tests.

**External loading:** `s_loader.c` uses `dlopen(...RTLD_NOW | RTLD_GLOBAL)`, caches loaded
module names and calls setup functions. Closing a graph is not a guarantee of unloading
its external libraries or resetting their static globals. Header [src/m_pd.h](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/src/m_pd.h)
includes `PD_FLOATSIZE` and `PDINSTANCE` distinctions. Validate ARM little-endian ELF32,
hard-float convention, instruction set, exact Pd ABI/float width, C++/glibc requirements,
exported/imported symbols and instance safety. `.pd_linux` only identifies a naming
convention. GUI-dependent externals may require Tcl/Tk or unsupported host behavior.

Our [scripts/build.sh](https://github.com/carballocba/MPC-PD-LAB/blob/879ab14336fd5c4576ab9832127b4047aaa2a2b6/scripts/build.sh) builds `EXTRA=false`, `MULTI=false`, with hidden Pd
symbols and `--exclude-libs,ALL`/`-Bsymbolic`. Ordinary dynamically loaded externals need
Pd symbols such as `class_new`/`gensym`; that symbol policy is an integration blocker,
even for an otherwise matching ARM binary. Prefer a small audited set registered into
the same engine first. Any future exported API/shared-runtime design needs explicit
isolation tests; globally exposing Pd in the MPC process could conflict across plugins.

**GUI/Q-Link/MIDI/CV:** Pd GUI controls are not native MPC widgets or automatically
VST parameters. Use the existing native skin and manifest mapping. libpd MIDI hooks
supply messages, not physical MIDI routing or CV voltages. No supported MPC CV API was
found in these Pd sources. Standalone Pd's platform audio/MIDI backends must not compete
with MPC's ownership when embedded.

### Deken: reuse package knowledge, not its desktop installation UI

Inspect [deken-plugin.tcl](https://github.com/pure-data/deken/blob/785862399822b6d12f4804cf7b12f5ed55ee10c4/deken-plugin.tcl) and
[developer/src/deken/deken.hy](https://github.com/pure-data/deken/blob/785862399822b6d12f4804cf7b12f5ed55ee10c4/developer/src/deken/deken.hy): compatibility includes OS, CPU aliases and
float width, with ELF/Mach-O/PE inspection in developer tooling. This supports an offline
preflight vocabulary; it is not proof that any `Linux-armv7-32` package loads in our
hidden-symbol libpd. Deken does not certify external real-time behavior or license all
packages under its own BSD license.

The GUI plugin depends on Pd/Tcl integration. Its developer client uses Python/Hy and
packages including pyelftools, keyring, WebDAV, macholib, pefile and GnuPG integration
([developer/requirements.txt](https://github.com/pure-data/deken/blob/785862399822b6d12f4804cf7b12f5ed55ee10c4/developer/requirements.txt)); the launcher can self-install dependencies.
None was executed or added. Use existing tooling on a development computer after a
separate dependency audit, or adapt bounded metadata checks; no on-MPC package manager
is needed for the first milestone. Portal-listed alternative flavours/Gem/Cyclone/zexy
are discovery leads, not fully audited additions in this follow-up.

## Organelle documentation → runtime and patch implementations

The M/S/S2 manual identifies the newer platform, while its header says OS 5.1 and one
introductory sentence still says 5.0. Treat this as a documentation inconsistency,
not proof of exact firmware compatibility. The original Organelle model is separately
documented. The programming and patch documents are available in [critterandguitari/cg-docs](https://github.com/critterandguitari/cg-docs/tree/b6f2633e7c39f78f7d924f9bf6084eaa0144bbff); no
repository-wide license was found, so do not copy its text, imagery or generator code
on the assumption the Organelle_OS BSD license covers it.

### Menu, loader and hardware boundary

[MainMenu.cpp](https://github.com/critterandguitari/Organelle_OS/blob/1f549fddc2dfd7d6ac1f2310ef01278ddf361856/MainMenu.cpp) scans directories and accepts `main.pd`, `mother.pd`,
`main.scd`, `main.py` or `run.sh`; Pd entries take precedence. It finds a mother from the
patch/system/firmware paths, reads `pd-opts.txt`, kills the previous process, changes
`/tmp/patch` links, and starts `/usr/bin/pd` in that working directory. Some platform
builds rename bundled `abl_link~.pd_linux` to avoid incompatible versions. These are
whole-machine policies, unsuitable for blind incorporation into an in-process VST.

Reuse the **selection conventions** and explicit control messages; adapt enumeration,
favourites and program-change lookup to an instance-owned root and state. Never use
process-wide `chdir`, process killing, global symlinks, arbitrary command-line options
or shell execution inside the plugin. A custom mother cannot simply be replaced with
our generic one: inspect whether it contains the actual DSP/control graph.

[main.cpp](https://github.com/critterandguitari/Organelle_OS/blob/1f549fddc2dfd7d6ac1f2310ef01278ddf361856/main.cpp) routes OSC/display and program-change messages;
[Makefile](https://github.com/critterandguitari/Organelle_OS/blob/1f549fddc2dfd7d6ac1f2310ef01278ddf361856/Makefile) has CM3/CM4 hardware and SDL desktop targets. GPIO/wiringPi,
OLED drivers, hardware power/menu services and desktop X/Tk behavior do not transfer to
MPC touchscreen/Q-Links. Their Linux/ARM ancestry is not binary compatibility evidence.
`fw_dir/scripts/info.py` uses psutil, blocking sampling and Raspberry Pi `vcgencmd`;
this is a reference for monitoring, not a reusable MPC audio-thread monitor.

Organelle default knob CCs 21–24 and aux CC25 overlap the community wrapper's CC20–35
mapping range. Avoid duplicate delivery: explicitly select native Organelle MIDI mapping
or VST parameter mapping and persist that choice. Patch-specific MIDI handling may
bypass the defaults. Favourites program changes need stable, project-owned IDs rather
than alphabetical rescans. These are adapters, not automatic MIDI Learn.

### Actual patch archives inspected

These are **source downloads for inspection**, not installations. Hashes identify the
exact bytes; no archive binaries were loaded. Published filenames can change, so retain
these fingerprints in any later acceptance record. Do not redistribute archives as
part of this documentation bundle.

| Asset | Files in archive | Source characteristics | License result |
|---|---|---|---|
| [PLAY](https://patchstorage.com/wp-content/uploads/2026/04/PLAY-69e4b9e28475a.zop) | 297 | Eight instrument folders, Pd/custom mother, Faust DSP, Jam/Lua, patterns, compiled externals | No license file found in archive; redistribution/reuse clearance unresolved |
| [CCquencer2](https://patchstorage.com/wp-content/uploads/2021/04/CCquencer2.zip) | 85 | Sequencing/modulation graph, knob5, note/CC output, waveform and knob save files | BSD root; `cg-speedlim.pd` explicitly GPL-3.0-or-later; audit nested imports |
| [MIDI Maker](https://patchstorage.com/wp-content/uploads/2023/03/MIDI-Maker-641a20c653824.zip) | 103 | Audio-to-MIDI, six cloned voices, `sigmund~`, sequencer/metronome abstractions and MIDI output | BSD root; nested GPL speed limiter/folder_list provenance requires review |

CCquencer2 `main.pd` uses `ctlout` and `noteout`; MIDI Maker `main.pd`, `a2midi-voice.pd`
and `a_2_midi.pd` reuse Pd pitch analysis and MIDI generation. These are good curated
control/sequencing candidates after license/dependency audit. Neither contains a native
MPC CV output implementation. Route generated messages through a separately validated
host/ALSA output and CV track or external converter. MIDI Maker is audio-to-MIDI, **not
CV input**; detection and analysis latency must be measured.

PLAY/CZ `main.pd` is an empty placeholder; `mother.pd` declares `lib`, creates
`[jam 360 120]` and eight `[synthvoice]` clones, and includes `abl_link~`, MIDI clock and
Organelle OSC/OLED behavior. `fsynth.dsp` imports `stdfaust.lib`; `compile.sh` invokes
`faust2puredata`. Preserve source and use offline audited compilation if this tier is
ever authorized. Do not run patch compile scripts on the MPC or equate Faust source
availability with complete generated-runtime licensing. Other seven PLAY instruments
were structurally inventoried; CZ's control/loader path received the focused read.

### Jam source: useful control engine, presently blocked for copying

[jam.c](https://github.com/owenosborn/jampd/blob/1c061e8efa423399d4045958fb4e9d3c8fb294d2/jam.c) has a Lua state per object, tick-driven events, pending note-offs,
active-note tracking, `flushnotes`, and close/free behavior. `jam.lua` and example scripts
cover periodic/probabilistic patterns and child jams. It outputs control messages;
there is no audio renderer, VST2 wrapper, MPC skin/Q-Link adapter or direct CV API.

The normal Makefile builds a macOS bundle against a hardcoded Pd app include path.
`Makefile.static` offers Linux shared output/static Lua discovery but is not an ARMv7
cross-toolchain. A port needs matching Pd headers, hard-float Lua/PIC library, symbol
visibility, callback timing and instance tests. `luaL_openlibs` exposes normal Lua
libraries; script loading executes files and callbacks can allocate/run without bounded
execution. Note bookkeeping also uses Pd allocation. Neither Lua isolation nor a fixed
pending-note bound guarantees real-time safety or recovery from an infinite script.

No explicit license was found in the inspected Jam repository or PLAY archive. **Do not
copy/integrate/distribute this code until a grant and generated/runtime dependency
licenses are clarified.** The existence of a public ARM binary does not remove that
blocker. There is no need to add Lua to the initial curated Vanilla host.

### Static ARM observations — necessary, insufficient

| Exact file within archive | ELF/header result | Highest required glibc | DT_NEEDED |
|---|---|---|---|
| PLAY/CZ/lib/fsynth~.pd_linux | ARM ELF32 EABI5, hard-float header flag | 2.27 | libstdc++, libm, libgcc_s, libc |
| PLAY/CZ/lib/jam.pd_linux | ARM ELF32 EABI5, hard-float header flag | 2.34 | libm, libc, ld-linux-armhf.so.3 |
| CCquencer2/cg-pd-library-local/externals/clocker.pd_linux | ARM ELF32 EABI5, hard-float header flag | 2.4 | libc |

These three samples have Pd undefined imports. glibc numbers are parsed from ELF
version-needed sections using upstream `catalog_check.max_glibc`, not guessed from
strings. Header checks do not certify all required instructions, C++ symbol versions,
Pd ABI or every binary in the archives. User-reported device glibc 2.39 alone cannot
resolve missing Pd exports or establish successful loading.

## sd88me build and workflow: strongest direct reuse

The supplied pages are generated from [catalog/pages/build.md](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/pages/build.md) and
[catalog/pages/workflow.md](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/pages/workflow.md). Browser fetches failed in this session;
these pinned source files and the implementations below were inspected instead.

- [tools/build_port.sh](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/build_port.sh): parameter/header, skin and registry generation;
  separate C/C++ compilation; hidden symbols; `--no-undefined`; ARMv7 defaults to
  `arm32v7/gcc:11-bullseye`, with separate AArch64 target support. Build dependencies
  include Docker, emulation as needed, Python/Pillow; HTML artwork uses a build-time
  Chromium container. Mutable image tags and pip dependencies mean a future reproducible
  release should record image digests and dependency versions, not only Git commits.
- [tools/gen_vst.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/gen_vst.py) and [wrapper/engine.h](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/wrapper/engine.h):
  generate fixed host parameters and bridge the 44.1 kHz/128-frame engine contract.
  Preserve shipped parameter order and identity; a Universal plugin needs its own ID.
- [tools/shadow_skin.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/shadow_skin.py), [tools/studio.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/studio.py) and
  [docs/SKIN_STUDIO.md](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/docs/SKIN_STUDIO.md): native TUI/Q-Link pages, widgets, displays,
  art and offline previews. Skin Studio is a computer-side authoring tool. Build-time
  HTML artwork is a rendered asset, **not an embedded Chromium GUI on the MPC**.
- [tools/test_port.sh](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/test_port.sh), [tools/host_test.c](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/host_test.c):
  native sanitizer test path; optional AArch64 emulation path uses UBSan without ASan.
  This does not certify original-One performance, GUI or arbitrary Pd graphs.
- [.github/workflows/vst-release.yml](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/.github/workflows/vst-release.yml):
  pin called workflow and `tools_ref` to the same commit; build/test/preview/package,
  attach to a draft, retain human hardware acceptance. Missing committed bench output
  is reported as not measured; CI does not invent a benchmark or perform hardware tests.
- [tools/release.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release.py) and [tools/catalog_check.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/catalog_check.py):
  package assets/checksums/manifest/standard scripts; enforce ARM and glibc constraints.
  The pinned ARMv7 catalog ceiling is glibc 2.36, although this device reportedly has
  2.39. This is an ecosystem acceptance rule, distinct from the system loader's ability.
  GLIBCXX requirements and hard-float/instruction attributes still need dedicated checks.

Reuse these existing tools and known packaging rather than creating another installer,
UI framework or CI pipeline. Reconcile pins with our working baseline before adopting
new wrapper behavior. Package `data`/patch/sample/notices with explicit `--extra` inputs;
mark writable user data where supported. A standard installer can replace matching IDs
or filenames, so Universal must preserve the established Delay/Freezer identities and
use a separate folder. None of the real plugin install/bench paths was invoked.

## monome: control ecosystem rather than a replacement Pd host

### norns

[wscript](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/wscript), [norns/wscript](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/norns/wscript) and
[readme-setup.md](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/readme-setup.md) show a Linux application, Lua control layer, JACK audio,
SuperCollider engines and many platform dependencies: ALSA, libudev/evdev/gpiod, liblo,
Cairo, Lua5.3, Avahi, sndfile, nng/libmonome, Link and softcut. Setup documentation still
mentions older separate executables; current build defines a combined `norns` target.
Build source is authoritative. `--release` adds Cortex-A53/ARMv8-related FPU flags;
these must not be copied to the original One. Omitting them is only a prerequisite,
not a demonstrated ARMv7 VST2 port. Do not install norns services or take over JACK/audio.

High-value implementations:

- [lua/core/paramset.lua](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/lua/core/paramset.lua) and [lua/core/pmap.lua](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/lua/core/pmap.lua):
  stable IDs, control/option/binary/trigger/file parameter types, CC device/channel/value
  ranges, assignment conflict handling, persistence and optional echo. `.pset` excludes
  triggers/separators; this is a useful momentary-state precedent. Mapping read uses Lua
  evaluation, so it is not a safe generic manifest parser. These modules depend on
  norns globals/utilities/menu/MIDI; they cannot be dropped into a C/libpd wrapper.
- [lua/lib/fileselect.lua](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/lua/lib/fileselect.lua): file browsing with callback and preview debounce,
  tied to norns keys/encoders/screen/menu and preview clocks. Native MPC browser patterns
  remain the lower-porting-cost option. Norns OLED output is not a TUI skin or Q-Link map.
- [matron/src/clocks/clock_midi.cc](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/matron/src/clocks/clock_midi.cc): 24-PPQN timing smoothing, start/stop,
  global reference state. Its dispatcher has no explicit MIDI Continue handler.
  [tests/matron/clocks/clock_midi/test_clock_midi.cpp](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/tests/matron/clocks/clock_midi/test_clock_midi.cpp) records that caveat.
  [matron/src/clocks/clock_scheduler.cc](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/matron/src/clocks/clock_scheduler.cc) uses a mutex, event posting and
  `usleep(1000)`; this is control scheduling, not proof of sample-accurate VST/CV events.
- [matron/src/device/device_midi.cc](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/matron/src/device/device_midi.cc) uses ALSA raw MIDI, while community
  MPC generators use virtual sequencer ports to avoid depending on ignored host output.
  Reusing raw physical-port ownership would require a conflict/routing design.
- [matron/src/device/device_crow.cc](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/matron/src/device/device_crow.cc) and [lua/core/crow.lua](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/lua/core/crow.lua)
  demonstrate serial crow control. They do not address MPC's internal DAC. Prefer
  norns/crow externally; a future MPC bridge needs USB role, CDC/TTY, ownership, bounded
  worker I/O, disconnect/reconnect, calibrated voltages and licensing acceptance.

Root GPL-3.0 plus individually licensed dependencies apply. Copying implementation
requires a compatible project license/source distribution plan; studying behavior does
not require porting the entire norns stack. No norns source was built or executed.

### softcut-lib

[softcut-lib/include/softcut/Softcut.h](https://github.com/monome/softcut-lib/blob/14241f2a94a7cd2e6c91b9bdc4e500d68abef907/softcut-lib/include/softcut/Softcut.h) exposes templated voice/block APIs,
loop/cut/rate/filter/record controls and sample-rate configuration;
[softcut-lib/CMakeLists.txt](https://github.com/monome/softcut-lib/blob/14241f2a94a7cd2e6c91b9bdc4e500d68abef907/softcut-lib/CMakeLists.txt) builds a C++14 static library. The top-level
project also builds a JACK/OSC client; that client's dependencies are not all intrinsic
to the DSP core. Core buffers/voice routing and crossfades need integration and memory
budgeting. This is GPL-3.0 looping DSP, not a dynamic Pd loader, MIDI router, GUI, Q-Link
system or CV implementation. Adding it would be a new optional DSP feature, outside the
minimum Universal scope; do not replace the working Granular Freezer with it.

### Grid/arc: serialosc and libmonome

[src/serialosc-device/server.c](https://github.com/monome/serialosc/blob/c96ea389dbf82c84d17f6f7adddaf311aed49438/src/serialosc-device/server.c), [CMakeLists.txt](https://github.com/monome/serialosc/blob/c96ea389dbf82c84d17f6f7adddaf311aed49438/CMakeLists.txt) and
[COPYRIGHT](https://github.com/monome/serialosc/blob/c96ea389dbf82c84d17f6f7adddaf311aed49438/COPYRIGHT) show USB discovery/serial-to-OSC infrastructure with separate
processes, libuv, libmonome, liblo and configuration dependencies. Current CMake requires
3.27; its working-poll probe uses `try_run`, requiring explicit cross-compilation handling.
liblo carries LGPL-2.1-or-later, including static-link relinking obligations when applicable.
Do not describe the whole dependency stack as ISC-only.

[public/monome.h](https://github.com/monome/libmonome/blob/9055ea22fd824839be821c10f6e9b13e9d2aa3cd/public/monome.h), [src/platform/posix.c](https://github.com/monome/libmonome/blob/9055ea22fd824839be821c10f6e9b13e9d2aa3cd/src/platform/posix.c),
[CMakeLists.txt](https://github.com/monome/libmonome/blob/9055ea22fd824839be821c10f6e9b13e9d2aa3cd/CMakeLists.txt) and [COPYRIGHT](https://github.com/monome/libmonome/blob/9055ea22fd824839be821c10f6e9b13e9d2aa3cd/COPYRIGHT) provide ISC-style serial
protocol, grid LEDs/key events and arc rings. Linux requires libudev; serial reads and
polling belong outside the audio callback. C portability is plausible; device access,
driver/library availability and host behavior on the One are untested. These controllers
are not MIDI by default, provide no CV output and do not expose MPC Q-Links.

Actual Pd abstraction [grid/studies/pd/files/monome-device/monome-device.pd](https://github.com/monome/docs/blob/de746029ef57244036df2a206e5b894a29b5e0f6/grid/studies/pd/files/monome-device/monome-device.pd)
uses OSC externals (`packOSC`, `unpackOSC`, `routeOSC`, `pipelist`) plus `prepend`, UDP
`netsend`/`netreceive` and serialosc discovery. It assumes localhost, takes the first
reported device and derives a listen port from `$0`; this needs instance/network
arbitration rather than blind copying. The sequencer examples use Cyclone `counter`/`uzi`.
These dependencies are not Vanilla. Their libraries have not received full per-file
licensing/ABI audit here. Documentation `LICENSE` is CC-BY-4.0; the separate MIT
`LICENSE.txt` credits the site theme author and is not blanket permission for all examples.
Preserve attribution and establish example scope before bundling.

Prefer an existing computer/norns serialosc bridge first, with an explicit OSC transport
adapter if later authorized. Vanilla `oscparse`/`oscformat` are available alternatives
for an adapter; replacing the existing abstraction is still development, not automatic
compatibility. UDP timing, polling within libpd processing, disconnects, LED traffic and
multiple instances require tests. No daemon or new external was installed on the MPC.

### Ansible and Teletype: proven external synthesis workflows

[src/ansible_midi.c](https://github.com/monome/ansible/blob/9796656cbbe9eb0227d380d555e31192f0cf32bf/src/ansible_midi.c) contains mono/poly/multi allocation, pitch bend,
velocity/CC conversion, arpeggiation, clock division, panic and DAC updates. This is a
valuable MIDI-to-CV implementation to study rather than rewriting voice allocation.
[src/config.mk](https://github.com/monome/ansible/blob/9796656cbbe9eb0227d380d555e31192f0cf32bf/src/config.mk) targets AVR32 UC3B0512, with USB host, DAC/GPIO/timer/ii
libraries, not Linux ARMv7. Its timing comments (including forced DAC updates) describe
Ansible hardware only. Root GPL-2.0 plus vendor headers need audit before extracting code.

[src/ops/hardware.c](https://github.com/monome/teletype/blob/e390bacdf024f2234e2ade6a4821773b9a80a322/src/ops/hardware.c) implements CV/slew/calibration and gate/pulse commands,
[module/config.mk](https://github.com/monome/teletype/blob/e390bacdf024f2234e2ade6a4821773b9a80a322/module/config.mk) targets AVR32, and its tests/simulator cover language
and scene behavior. It is a separate scripted Eurorack processor, not a Pd interpreter
or an available MPC I2C API. Prefer keeping its algorithms on the external module.
Root GPL-2.0 and imported hardware support licenses are distinct from Pd's BSD license.

For MPC→Ansible, Ansible's USB host expects a USB MIDI **device**. The MPC's USB host
port cannot be directly connected host-to-host. A suitable device-side MIDI interface
or properly configured intermediary is required; no direct route was hardware-tested.
For Teletype, existing MPC gate/clock output can trigger external scripts, subject to
manufacturer voltage/threshold checks. Neither module's CV inputs establish CV input
on the One. Do not wire MPC to an internal ii/I2C bus without an appropriate supported
bridge and electrical interface.

## Priority and decisions

1. **Reuse now as foundation:** existing libpd API/core and Vanilla-compatible licensed
   abstractions; existing MPC build/package/checker/native-skin infrastructure. These
   are already relevant to the target and avoid new runtime systems.
2. **Adapt next:** fixed parameter manifests/state, local browser/status, stopped loading;
   curate CCquencer2/MIDI Maker control graphs after nested license/external audit.
3. **Compose externally:** existing norns/crow or Ansible/Teletype functions for modular
   control; verify explicit MIDI/OSC/CV routing and timing on the intended devices.
4. **Defer/block:** copying Jam/PLAY without license clearance; wholesale norns port;
   arbitrary ARM externals; seamless live switching; native MPC CV claims inferred
   from external module firmware. None solves the current black MPC GUI automatically.

## Validation and access limitations

- Static source/manifest/archive reads only; three ELF samples checked without loading.
- Upstream offline catalog/build/site tests selected without installer/store fixtures:
  **75 passed**. Upstream skin-compatibility tests: **10 passed**.
- A broader upstream test attempt used synthetic packages/settings and mocked service
  roots only: **125 tests, 24 failures**. A representative store failure was reproduced:
  missing `sha256sum` on this macOS host. Do not claim all 24 share that cause or that the
  complete suite passed. No real MPC settings or installed plugins were involved.
- puredata.info returned 403 to the browser fetch, but normal TLS-verified curl obtained
  its portal page (HTTP 200). Python urllib archive downloads failed local CA verification;
  system curl succeeded with TLS verification enabled. No certificate checks were disabled.
- Clone/submodule declarations were inspected, but full dependency builds and licenses
  of every transitive package remain outside this follow-up. No ARM runtime/performance,
  electrical, GUI or hardware tests were performed. Link/path checks are local source
  existence checks, not proof every remote link will remain reachable.
