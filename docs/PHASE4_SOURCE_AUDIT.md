# Phase 4 source audit — 2026-10-13

## Provenance and redistribution

The authentic patch is in Critter & Guitari's official
[Organelle_Patches](https://github.com/critterandguitari/Organelle_Patches/tree/72d19ed847ee40a89e6f40bcebce56229be1ef0b/Effects/Granular%20Freezer),
revision `72d19ed847ee40a89e6f40bcebce56229be1ef0b`, `Effects/Granular Freezer`.
The original author/copyright holder is Critter & Guitari (2016), with
Hans-Christoph Steiner (2004) credited for crossfade adaptations in main.pd.
The directory LICENSE.txt grants BSD-3-Clause terms except for attributed imports.

Those two crossfades explicitly identify `equal_power_pan~` and `pan_core~`.
The reference `pan` library 0.1.2 identifies GNU GPL v2 or later in `pan-meta.pd`;
its source and LICENSE.txt are available from the
[Debian source archive](https://deb.debian.org/debian/pool/main/p/pd-pan/).
Consequently this port records **BSD-3-Clause AND GPL-2.0-or-later** for the patch,
retains both notices, and ships the complete editable Pd sources. It does not
strip the imported-code attribution or present the whole patch as BSD-only.
`PAN-GPL-2.0.txt` is the reference archive's LICENSE.txt. The reference library
archive SHA-256 is `594ae9dd2c3aa22942ee53c7a7ff8ddaa9dbb6db0944efb55185d63ee9d18ff9`.
The reference library itself is not a runtime dependency: the adapted crossfade subpatches are already
inside main.pd.

The four Pd files and original LICENSE.txt were imported **byte for byte**.
`ports/granular-freezer/upstream.sha256.json` records every file's SHA-256, checked
by source-audit and package tests. No upstream DSP, connections, arrays,
initial note, voice count, randomization or freeze logic was edited. The original
Hanning window is embedded Pd array data, not an external asset.

| Complete original directory | Role |
| --- | --- |
| `main.pd` | Original entry, freezer/dry mix, note allocator, 16 grainvoice objects, control scaling, display/LED messages |
| `grainvoice.pd` | Two grains per voice, stereo throws, note transposition and gain ramps |
| `grain.pd` | Delay readout, random placement/density, Hanning window and granular DSP |
| `latch-notes.pd` | Latched MIDI note selection and allocation input |
| `LICENSE.txt` | C&G BSD-3-Clause notice and attributed-import exception |

No sample files, compiled objects, libraries, presets or other directory entries
exist in this revision's patch directory. The runtime assets are the four Pd
files, original notice, generated host patch and reusable mother. No Organelle
Linux binary is copied or assumed ABI-compatible with the MPC.

## Dependency and communication audit

`prepare_organelle.py` parses multiline Pd object records and checks the exact
object set against an explicit Vanilla allowlist plus the three local
abstractions. The port needs **no external libraries**. `expr`/`expr~`, GUI
`tgl`/`bng`, arrays, lists, bag, delay objects and all remaining objects are
provided by the pinned libpd/Pure Data build. Tests additionally instantiate the
actual patch: a missing class can leave libpd_openfile returning a non-null
canvas, so the engine captures Pd creation errors and rejects that canvas.
This whitelist is specific to this audited patch, not a universal Vanilla or
security validator.

| Interface used by original patch | Behavior retained |
| --- | --- |
| `r~ inL`, `r~ inR` | Original wet freezer sums both input channels; not two independent stereo freeze buffers |
| `throw~ outL`, `throw~ outR` | Original stereo grain output; dry paths retain channel separation |
| `knob1` | Normalized 0–1; displayed grain size `1 + 500*x` ms, with original grain clipping/randomization |
| `knob2` | Normalized 0–1; spread `4000*x` ms |
| `knob3` | Normalized 0–1; original density offsets remain inside the patch |
| `knob4` | Normalized wet/dry; original equal-power crossfade |
| `aux`, `fs` | A received value 1 triggers the original freeze toggle; 0 is release |
| `notes` | Two-atom pitch/velocity list; original loadbang sends note 72/velocity 100 |
| `screenLine1`…`screenLine5`, `led` | Text/LED status output, consumed by host bindings; no OLED implementation |
| `grain-size`, `loc`, `density`, `wetdry`, `grainoutL/R`, `alloff`, `allon` | Internal globals remain unchanged; this is one-instance-per-module |

The selected patch does not consume `exp`, MIDI controllers directly, hardware
OSC, storage paths, preset files, shell commands, networking, OLED graphics,
clock externals or filesystem writes. Its native Pd loadbangs run on creation.
It does not itself open mother.pd: the original hardware loads mother separately.

The official [Organelle_OS mother.pd](https://github.com/critterandguitari/Organelle_OS/blob/1f549fddc2dfd7d6ac1f2310ef01278ddf361856/fw_dir/mother.pd),
revision `1f549fddc2dfd7d6ac1f2310ef01278ddf361856` (BSD-3-Clause root notice),
was inspected for normalized controls, pitch/velocity lists, audio named buses,
button edges, MIDI gates and output volume. The new host uses those conventions,
including squared volume smoothed with lop~ 5 followed by output clipping. It
does not import the hardware OSC, shutdown, patch-loader or system-control parts.

## Zynthian comparison

Inspected [zynthian-data](https://github.com/zynthian/zynthian-data/tree/4920bea5a6327824eee8500987a1d3dcd936f7fb),
revision `4920bea5a6327824eee8500987a1d3dcd936f7fb`:

- `presets/puredata/audio-fx/Granular_Freezer/main.pd` embeds the alternative
  mother/control routing, instantiates `GranularFreezer/main`, maps CC 21–24 to
  knobs, 25/27 to aux/fs, 26 to expression and 7 to volume, and sends initialization
  values after a delay. `zynconfig.yml` exposes those controls.
- `puredata/organelle/DACADC.pd`, `WCTL.pd`, `midi-in.pd`, `OSCbuttons.pd` and
  `clockfix.pd` are additional compatibility references. DACADC's input gain and
  output scaling differ from the official Organelle mother; they were not copied.
- Its bundled GranularFreezer files differ from the original (including modified
  freeze control behavior). We use C&G's exact original instead.
- Repository root LICENSE is GPL-3.0; the nested C&G patch carries its own BSD
  notice with imported-code attribution. **No Zynthian code was copied into the
  new bridge.** The implementation is based on the documented protocol boundary.

The [community discussion](https://discourse.zynthian.org/t/puredata-organelle-patches-in-zynthian/10995)
provides context for gitnob's alternative mother. Its forum ZIP is not used as an
unversioned build dependency. Simple single-screen patches are the initial scope;
aux-plus-key multi-page selection, OLED/OSC feedback, platform externals and MIDI
backends need individual adaptation. The audited repository code demonstrates
actual mappings, not universal patch compatibility.

Also inspected [gitnob/pd_to_zynthian_oram](https://github.com/gitnob/pd_to_zynthian_oram/tree/0a1f626ec2aafe0b1502f062182f3bb2e91c34b2)
revision `0a1f626ec2aafe0b1502f062182f3bb2e91c34b2` (GPL-3.0): this is an hvcc/DPF
LV2 development workflow, not an embedded libpd Organelle mother implementation.
Its compiler restrictions are not treated as libpd limitations.

## Adaptations made here

1. Added `host.pd`, containing separate mother and original entry abstractions.
2. Added `mpc_organelle_mother.pd` for adc~/dac~, named buses, normalized control
   forwarding, notes/notesRaw, host-ready and output-volume conventions.
3. Added a C protocol adapter driven by generated manifest mappings. Absolute
   VST Freeze changes pulse aux exactly once per state transition; CC64 forwards
   footswitch edges and synchronizes the tracked freeze state. No original
   freeze logic was changed.
4. Added a six-parameter VST definition with distinct identity CGFZ / 4347465a,
   name `Organelle Granular Freezer`, vendor CARBALLO and separate installation
   directory. Original delay identity MPDL and source/build/package stay intact.
5. Added source hashes, loading/error checks, editable sources and all notices
   to the package. No source file path points to an Organelle filesystem.

The overall MPC-PD-LAB project license remains undeclared; as in Phase 3, catalog
submission needs that decision. This audit identifies upstream terms and keeps
notices/source; it does not assert a firmware endorsement or hardware test.
