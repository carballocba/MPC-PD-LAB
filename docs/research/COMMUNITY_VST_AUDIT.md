# Community VST audit

Research date: 2026-10-13. Original MPC One, ARMv7 hard-float, Hakai 3.9.1.
Research only: no plugin/DSP/installer/system changes, hardware access or deployment.
Evidence labels: **source-verified**, **manufacturer-documented**, **upstream hardware report**,
**user-reported hardware result**, **inference/proposal**, and **not established** are distinct.
Exact source revisions and access gaps are in [SOURCES.md](SOURCES.md).

## Scope and evidence

This is a reproducible survey of 116 entries in the pinned sd88me catalog, with focused
source inspection of six high-value implementations below. It is not a claim that every
catalog project was built, electrically tested or audited line by line. Catalog metadata
is discovery evidence only. Activity dates and exact licenses/revisions are in SOURCES.
Maintainer names below identify repository owners, not a promise of ongoing support.

For each focused project, format/build, hardware, UI, mapping, MIDI, state, assets, reuse,
license and limitations are recorded. Public source is available at the pinned links;
binary download availability and all release combinations were not independently tested.

## sd88me/mpc-vst-plugins

- Maintainer/availability/activity: sd88me; public [sd88me/mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins/tree/5e3a96885b6de76a48b498821c379568a05cc006); inspected tip 2026-10-13.
- Format/build/target: community ARMv7 VST2 ecosystem, source build scripts/framework; no binary rebuilt in this audit. Gen2 and desktop support must be checked separately.
- Function/UI/Q-Links/MIDI/state: Reusable ARMv7 VST2 framework and installer; native declarative skin/Q-Link generation, parameter/chunk/MIDI/time adapters. See [wrapper/engine.h](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/wrapper/engine.h) and the repository's parameter/layout/build definitions. MIDI input/output and persistence are feature-specific, not universal guarantees.
- Hardware evidence: Force timing evidence; no blanket certification for every plugin or original One.
- Runtime/dependencies: inspect build scripts, vendored code and assets at the pinned tree; dependencies and engine state are not interchangeable with libpd.
- License: MIT. Preserve notices and audit imported files before copying.
- Reuse/limitations: Reuse MIT infrastructure after pin comparison. Fixed 44.1 kHz/int16/128-frame engine and mutex constrain arbitrary Pd hosting.

## padbangers/glass-spheres

- Maintainer/availability/activity: Padbangers; public [padbangers/glass-spheres](https://github.com/padbangers/glass-spheres/tree/1fe3b0a17b14dc07d327e5408d7aaf438715ce44); inspected tip 2026-10-13.
- Format/build/target: community ARMv7 VST2 ecosystem, source build scripts/framework; no binary rebuilt in this audit. Gen2 and desktop support must be checked separately.
- Function/UI/Q-Links/MIDI/state: Generative Marbles-derived instrument/control source; native skin, parameters, MIDI output and experimental direct CV. See [src/cv_out.cc](https://github.com/padbangers/glass-spheres/blob/1fe3b0a17b14dc07d327e5408d7aaf438715ce44/src/cv_out.cc) and the repository's parameter/layout/build definitions. MIDI input/output and persistence are feature-specific, not universal guarantees.
- Hardware evidence: tested.json and CV capture report MPC X OS 3.9.1; original One protocol unverified.
- Runtime/dependencies: inspect build scripts, vendored code and assets at the pinned tree; dependencies and engine state are not interchangeable with libpd.
- License: MIT; inspect vendored notices. Preserve notices and audit imported files before copying.
- Reuse/limitations: Highest-value CV precedent. Private ALSA controller access, latest-value queue and ownership limitations prevent safe drop-in reuse.

## sd88me/mpc-vst-acid

- Maintainer/availability/activity: sd88me; public [sd88me/mpc-vst-acid](https://github.com/sd88me/mpc-vst-acid/tree/d4bb7c23c3fb6b8203350c9f34b06d6c8ea7f50b); inspected tip 2026-10-13.
- Format/build/target: community ARMv7 VST2 ecosystem, source build scripts/framework; no binary rebuilt in this audit. Gen2 and desktop support must be checked separately.
- Function/UI/Q-Links/MIDI/state: VST2 MIDI sequencer; touchscreen/Q-Links, PPQ scheduling, virtual ALSA output rather than relying on host VST MIDI forwarding. See [vst/acid_vst.cpp](https://github.com/sd88me/mpc-vst-acid/blob/d4bb7c23c3fb6b8203350c9f34b06d6c8ea7f50b/vst/acid_vst.cpp) and the repository's parameter/layout/build definitions. MIDI input/output and persistence are feature-specific, not universal guarantees.
- Hardware evidence: Force OS 3.9.1 upstream tested.json; not original One evidence.
- Runtime/dependencies: inspect build scripts, vendored code and assets at the pinned tree; dependencies and engine state are not interchangeable with libpd.
- License: GPL-3.0-only (catalog; verify each imported file). Preserve notices and audit imported files before copying.
- Reuse/limitations: Reuse scheduling design; copying GPL code requires compatible distribution. Handle discontinuities, loops, note-offs and subscriptions.

## sd88me/mpc-vst-cratedigger

- Maintainer/availability/activity: sd88me; public [sd88me/mpc-vst-cratedigger](https://github.com/sd88me/mpc-vst-cratedigger/tree/e8ed11227d36e05e7c5245288745c3634c3adbf8); inspected tip 2026-10-11.
- Format/build/target: community ARMv7 VST2 ecosystem, source build scripts/framework; no binary rebuilt in this audit. Gen2 and desktop support must be checked separately.
- Function/UI/Q-Links/MIDI/state: VST2 sample discovery instrument; browser/status parameters, background work and engine API bridge. See [vst/cratedigger_vst.cpp](https://github.com/sd88me/mpc-vst-cratedigger/blob/e8ed11227d36e05e7c5245288745c3634c3adbf8/vst/cratedigger_vst.cpp) and the repository's parameter/layout/build definitions. MIDI input/output and persistence are feature-specific, not universal guarantees.
- Hardware evidence: Force OS 3.9.1 report; Live III OS 5.0.18 registration/UI partial success with search failure.
- Runtime/dependencies: inspect build scripts, vendored code and assets at the pinned tree; dependencies and engine state are not interchangeable with libpd.
- License: MIT; third-party notices separate. Preserve notices and audit imported files before copying.
- Reuse/limitations: MIT browser/worker patterns useful; downloading and external tools are not needed for a local Pd browser.

## sd88me/mpc-vst-dx7

- Maintainer/availability/activity: sd88me; public [sd88me/mpc-vst-dx7](https://github.com/sd88me/mpc-vst-dx7/tree/400fa20a506c8bc7c72105358ca85f27eefe3315); inspected tip 2026-10-10.
- Format/build/target: community ARMv7 VST2 ecosystem, source build scripts/framework; no binary rebuilt in this audit. Gen2 and desktop support must be checked separately.
- Function/UI/Q-Links/MIDI/state: Dexed-derived VST2 instrument with preset/bank controls, native skin and parameter handling. See [src/dsp/dx7_plugin.cpp](https://github.com/sd88me/mpc-vst-dx7/blob/400fa20a506c8bc7c72105358ca85f27eefe3315/src/dsp/dx7_plugin.cpp) and the repository's parameter/layout/build definitions. MIDI input/output and persistence are feature-specific, not universal guarantees.
- Hardware evidence: Force OS 3.9.1 upstream record; original One not established.
- Runtime/dependencies: inspect build scripts, vendored code and assets at the pinned tree; dependencies and engine state are not interchangeable with libpd.
- License: GPL-3.0-only (catalog; third-party notices separate). Preserve notices and audit imported files before copying.
- Reuse/limitations: Study bank enumeration/state; GPL obligations and synth-specific DSP make whole-plugin reuse unnecessary.

## jacob-sabella/mpc-vst-nam

- Maintainer/availability/activity: Jacob Sabella; public [jacob-sabella/mpc-vst-nam](https://github.com/jacob-sabella/mpc-vst-nam/tree/cf2548f796c96b8ca4fdd4582af8514d82a24830); inspected tip 2026-10-10.
- Format/build/target: community ARMv7 VST2 ecosystem, source build scripts/framework; no binary rebuilt in this audit. Gen2 and desktop support must be checked separately.
- Function/UI/Q-Links/MIDI/state: VST2 Neural Amp Modeler effect; fixed browser parameters, model/status display, native skin generation and selective state persistence. See [vst/nam_vst.cpp](https://github.com/jacob-sabella/mpc-vst-nam/blob/cf2548f796c96b8ca4fdd4582af8514d82a24830/vst/nam_vst.cpp) and the repository's parameter/layout/build definitions. MIDI input/output and persistence are feature-specific, not universal guarantees.
- Hardware evidence: No tested.json found in inspected checkout; hardware compatibility not independently established.
- Runtime/dependencies: inspect build scripts, vendored code and assets at the pinned tree; dependencies and engine state are not interchangeable with libpd.
- License: MIT; model/data licenses separate. Preserve notices and audit imported files before copying.
- Reuse/limitations: MIT browser/state patterns are strong candidates; models have separate licenses and loading must stay off audio callback.

## Shared API observations

The wrapper exposes parameters, display/name strings, state chunks, audio/MIDI input,
and optional song position. It does not provide a supported CV API. Native skin data
is host-rendered; an embedded desktop editor does not establish an MPC touchscreen UI.
Existing six-Q-Link hardware behavior in Granular Freezer is user-reported and useful,
but its black GUI means the full UI path remains unverified.

## Complete catalog discovery inventory

All rows below are **catalog-triaged only**, unless covered by a focused card above.
Their hardware, release/build status, GUI, Q-Link detail, MIDI behavior, state semantics,
external dependencies, electrical behavior and legal provenance remain unverified in
this audit. License values are catalog declarations, not an independent legal clearance.
The source of each row is its JSON under the pinned catalog directory.

| Plugin | Author | Repository | Kind / role | Declared license | Catalog evidence |
|---|---|---|---|---|---|
| Acid | sd88me | [sd88me/mpc-vst-acid](https://github.com/sd88me/mpc-vst-acid) | instrument / midi | GPL-3.0-only | [acid.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/acid.json) |
| Airwindows Air3 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-air3.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-air3.json) |
| Airwindows ButterComp2 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-buttercomp2.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-buttercomp2.json) |
| Airwindows Chamber2 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-chamber2.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-chamber2.json) |
| Airwindows ChorusEnsemble | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-chorusensemble.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-chorusensemble.json) |
| Airwindows Console7Channel | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-console7channel.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-console7channel.json) |
| Airwindows Density2 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-density2.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-density2.json) |
| Airwindows Galactic | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-galactic.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-galactic.json) |
| Airwindows Galactic2 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-galactic2.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-galactic2.json) |
| Airwindows Galactic3 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-galactic3.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-galactic3.json) |
| Airwindows GalacticVibe | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-galacticvibe.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-galacticvibe.json) |
| Airwindows Infinity2 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-infinity2.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-infinity2.json) |
| Airwindows kAlienSpaceship | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-kalienspaceship.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-kalienspaceship.json) |
| Airwindows kCathedral5 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-kcathedral5.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-kcathedral5.json) |
| Airwindows kCosmos | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-kcosmos.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-kcosmos.json) |
| Airwindows Mackity | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-mackity.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-mackity.json) |
| Airwindows MatrixVerb | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-matrixverb.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-matrixverb.json) |
| Airwindows PitchDelay | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-pitchdelay.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-pitchdelay.json) |
| Airwindows TapeDelay2 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-tapedelay2.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-tapedelay2.json) |
| Airwindows ToVinyl4 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-tovinyl4.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-tovinyl4.json) |
| Airwindows Verbity2 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-verbity2.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-verbity2.json) |
| Airwindows ZAcidLowpass | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-zacidlowpass.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-zacidlowpass.json) |
| Airwindows ZLowpass2 | mmiroshnikov | [mmiroshnikov/mpc-airwindows](https://github.com/mmiroshnikov/mpc-airwindows) | effect / unknown | MIT | [airwindows-zlowpass2.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/airwindows-zlowpass2.json) |
| Aphex | saustin2010 | [saustin2010/mpc-vst-aphex](https://github.com/saustin2010/mpc-vst-aphex) | instrument / unknown | MIT | [aphex.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/aphex.json) |
| Boris Granular | sd88me | [sd88me/mpc-vst-boris](https://github.com/sd88me/mpc-vst-boris) | effect / unknown | GPL-3.0-only | [boris-granular.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/boris-granular.json) |
| Macro Osc | saustin2010 | [saustin2010/mpc-vst-macro-osc](https://github.com/saustin2010/mpc-vst-macro-osc) | instrument / unknown | MIT | [braids.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/braids.json) |
| Chordism | saustin2010 | [saustin2010/mpc-vst-chordism](https://github.com/saustin2010/mpc-vst-chordism) | instrument / unknown | MIT | [chordism.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/chordism.json) |
| Chordsmith | jacob-sabella | [jacob-sabella/mpc-vst-chordsmith](https://github.com/jacob-sabella/mpc-vst-chordsmith) | instrument / midi | MIT | [chordsmith.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/chordsmith.json) |
| Clementine-XT | sd88me | [sd88me/mpc-vst-clementineXT](https://github.com/sd88me/mpc-vst-clementineXT) | instrument / unknown | GPL-3.0-only | [clementine-xt.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/clementine-xt.json) |
| MPC Commander | jacob-sabella | [jacob-sabella/mpc-addin-commander](https://github.com/jacob-sabella/mpc-addin-commander) | addin / unknown | MIT | [commander.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/commander.json) |
| Crate Digger | sd88me | [sd88me/mpc-vst-cratedigger](https://github.com/sd88me/mpc-vst-cratedigger) | instrument / unknown | MIT | [crate-digger.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/crate-digger.json) |
| Denis | saustin2010 | [saustin2010/mpc-vst-denis](https://github.com/saustin2010/mpc-vst-denis) | instrument / unknown | MIT | [denis.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/denis.json) |
| Dexed | sd88me | [sd88me/mpc-vst-dx7](https://github.com/sd88me/mpc-vst-dx7) | instrument / unknown | GPL-3.0-only | [dexed-dx7.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/dexed-dx7.json) |
| Dragonfly Early Reflections | gmorb | [gmorb/mpc-vst-dragonfly-early-reflections](https://github.com/gmorb/mpc-vst-dragonfly-early-reflections) | effect / unknown | GPL-3.0-only | [dragonfly-early-reflections.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/dragonfly-early-reflections.json) |
| Dragonfly Hall | gmorb | [gmorb/mpc-vst-dragonfly-hall](https://github.com/gmorb/mpc-vst-dragonfly-hall) | effect / unknown | GPL-3.0-only | [dragonfly-hall.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/dragonfly-hall.json) |
| Dragonfly Plate | gmorb | [gmorb/mpc-vst-dragonfly-plate](https://github.com/gmorb/mpc-vst-dragonfly-plate) | effect / unknown | GPL-3.0-only | [dragonfly-plate.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/dragonfly-plate.json) |
| Dragonfly Room | gmorb | [gmorb/mpc-vst-dragonfly-room](https://github.com/gmorb/mpc-vst-dragonfly-room) | effect / unknown | GPL-3.0-only | [dragonfly-room.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/dragonfly-room.json) |
| Drips | Padbangers | [padbangers/drips](https://github.com/padbangers/drips) | effect / unknown | MIT | [drips.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/drips.json) |
| Dub Force Siren | no3z | [no3z/mpc-dubsiren](https://github.com/no3z/mpc-dubsiren) | instrument / unknown | MIT | [dub-force-siren.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/dub-force-siren.json) |
| Dub Station | nachtaktiv303 | [nachtaktiv303/mpc-vst-dub-station](https://github.com/nachtaktiv303/mpc-vst-dub-station) | effect / unknown | MIT | [dub-station.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/dub-station.json) |
| EffectForce | Devko | [Devko/EffectForce](https://github.com/Devko/EffectForce) | effect / unknown | MIT | [effectforce.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/effectforce.json) |
| Modal Voice | saustin2010 | [saustin2010/mpc-vst-modal-voice](https://github.com/saustin2010/mpc-vst-modal-voice) | instrument / unknown | MIT | [elements.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/elements.json) |
| Eucalypso | saustin2010 | [saustin2010/mpc-vst-eucalypso](https://github.com/saustin2010/mpc-vst-eucalypso) | instrument / midi | MIT | [eucalypso.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/eucalypso.json) |
| Euclidier | sd88me | [sd88me/mpc-vst-euclidier](https://github.com/sd88me/mpc-vst-euclidier) | instrument / midi | MIT | [euclidier.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/euclidier.json) |
| Fizzik | saustin2010 | [saustin2010/mpc-vst-fizzik](https://github.com/saustin2010/mpc-vst-fizzik) | instrument / unknown | MIT | [fizzik.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/fizzik.json) |
| Glass Spheres | Padbangers | [FullPace/glass-spheres](https://github.com/FullPace/glass-spheres) | instrument / midi | MIT | [glass-spheres.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/glass-spheres.json) |
| Drum Map | saustin2010 | [saustin2010/mpc-vst-drum-map](https://github.com/saustin2010/mpc-vst-drum-map) | instrument / midi | GPL-3.0-only | [grids.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/grids.json) |
| Groove Bank | saustin2010 | [saustin2010/mpc-vst-groovebank](https://github.com/saustin2010/mpc-vst-groovebank) | instrument / midi | MIT | [groove-bank.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/groove-bank.json) |
| Hank | saustin2010 | [saustin2010/mpc-vst-hank](https://github.com/saustin2010/mpc-vst-hank) | instrument / unknown | MIT | [hank.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/hank.json) |
| Helm | Lewinator56 | [Lewinator56/mpc-vst-helm](https://github.com/Lewinator56/mpc-vst-helm) | instrument / unknown | GPL-3.0-only | [helm.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/helm.json) |
| Hera | saustin2010 | [saustin2010/mpc-vst-hera](https://github.com/saustin2010/mpc-vst-hera) | instrument / unknown | GPL-3.0-only | [hera.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/hera.json) |
| InterzoneXXL | L'Cronx | [sunskiefer/InterzoneXXL](https://github.com/sunskiefer/InterzoneXXL) | instrument / unknown | GPL-3.0-or-later | [interzonexxl.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/interzonexxl.json) |
| Mini-JV | sd88me | [sd88me/mpc-vst-jv880](https://github.com/sd88me/mpc-vst-jv880) | instrument / unknown | MAME license | [jv-880.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/jv-880.json) |
| Keyscope | jacob-sabella | [jacob-sabella/mpc-vst-keyscope](https://github.com/jacob-sabella/mpc-vst-keyscope) | effect / unknown | MIT | [keyscope.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/keyscope.json) |
| Liminal Amp | gmorb | [gmorb/mpc-vst-liminal-amp](https://github.com/gmorb/mpc-vst-liminal-amp) | effect / unknown | MIT | [liminal-amp.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/liminal-amp.json) |
| Liminal Hz | gmorb | [gmorb/mpc-vst-liminal-hz](https://github.com/gmorb/mpc-vst-liminal-hz) | effect / unknown | MIT | [liminal-hz.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/liminal-hz.json) |
| Lucky Dip | sd88me | [sd88me/mpc-vst-luckydip](https://github.com/sd88me/mpc-vst-luckydip) | instrument / unknown | MIT | [lucky-dip.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/lucky-dip.json) |
| LXRXXL BAZZLINE | L'Cronx | [sunskiefer/LXRXXL-BAZZLINE](https://github.com/sunskiefer/LXRXXL-BAZZLINE) | instrument / unknown | LicenseRef-LXR02-Open | [lxrxxl-bazzline.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/lxrxxl-bazzline.json) |
| LXRXXL | L'Cronx | [sunskiefer/LXR02XXL](https://github.com/sunskiefer/LXR02XXL) | instrument / unknown | LicenseRef-LXR02-Open | [lxrxxl.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/lxrxxl.json) |
| Machinemodule | sd88me | [sd88me/mpc-vst-machinedrum](https://github.com/sd88me/mpc-vst-machinedrum) | instrument / unknown | AGPL-3.0-only | [machinemodule.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/machinemodule.json) |
| Maschine Group | mmiroshnikov | [mmiroshnikov/mpc-maschine-group](https://github.com/mmiroshnikov/mpc-maschine-group) | instrument / unknown | MIT | [maschine-group.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/maschine-group.json) |
| Maze Sequencer | sd88me | [sd88me/mpc-vst-maze](https://github.com/sd88me/mpc-vst-maze) | instrument / midi | MIT | [maze-sequencer.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/maze-sequencer.json) |
| Maze Voice | sd88me | [sd88me/mpc-vst-maze](https://github.com/sd88me/mpc-vst-maze) | instrument / unknown | MIT | [maze-voice.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/maze-voice.json) |
| MeowSynth | mcardonaserrano | [mcardonaserrano/meowsynth-mpc](https://github.com/mcardonaserrano/meowsynth-mpc) | instrument / unknown | MIT | [meowsynth.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/meowsynth.json) |
| MIDI Player | saustin2010 | [saustin2010/mpc-vst-midiplayer](https://github.com/saustin2010/mpc-vst-midiplayer) | instrument / midi | MIT | [midi-player.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/midi-player.json) |
| MonkSynth | saustin2010 | [saustin2010/mpc-vst-monksynth](https://github.com/saustin2010/mpc-vst-monksynth) | instrument / unknown | MIT | [monksynth.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/monksynth.json) |
| Mono Voice | saustin2010 | [saustin2010/mpc-vst-monovoice](https://github.com/saustin2010/mpc-vst-monovoice) | instrument / unknown | MIT | [mono-voice.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/mono-voice.json) |
| Monomodule One + FX | sd88me | [sd88me/mpc-vst-monomodule](https://github.com/sd88me/mpc-vst-monomodule) | instrument / unknown | AGPL-3.0-only | [monomodule.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/monomodule.json) |
| Morpho-PE | sd88me | [sd88me/mpc-vst-morpho-PE](https://github.com/sd88me/mpc-vst-morpho-PE) | instrument / unknown | MIT | [morpho-pe.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/morpho-pe.json) |
| MPC Plaits | poloq | [poloq-instruments/mpc-vst-plaits](https://github.com/poloq-instruments/mpc-vst-plaits) | instrument / unknown | MIT | [mpc-plaits.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/mpc-plaits.json) |
| Mr Drums | saustin2010 | [saustin2010/mpc-vst-mrdrums](https://github.com/saustin2010/mpc-vst-mrdrums) | instrument / unknown | MIT | [mr-drums.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/mr-drums.json) |
| Mr Hyde | saustin2010 | [saustin2010/mpc-vst-mrhyde](https://github.com/saustin2010/mpc-vst-mrhyde) | instrument / unknown | MIT | [mr-hyde.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/mr-hyde.json) |
| Mutable Vibe MPC | nachtaktiv303 | [nachtaktiv303/mpc-vst-mutable-vibe](https://github.com/nachtaktiv303/mpc-vst-mutable-vibe) | instrument / unknown | MIT | [mutable-vibe.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/mutable-vibe.json) |
| NAM | jacob-sabella | [jacob-sabella/mpc-vst-nam](https://github.com/jacob-sabella/mpc-vst-nam) | effect / unknown | MIT | [nam.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/nam.json) |
| Noisemaker | saustin2010 | [saustin2010/mpc-vst-noisemaker](https://github.com/saustin2010/mpc-vst-noisemaker) | instrument / unknown | GPL-2.0-only | [noisemaker.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/noisemaker.json) |
| NuSaw | saustin2010 | [saustin2010/mpc-vst-nusaw](https://github.com/saustin2010/mpc-vst-nusaw) | instrument / unknown | MIT | [nusaw.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/nusaw.json) |
| OB-Xd | saustin2010 | [saustin2010/mpc-vst-obxd](https://github.com/saustin2010/mpc-vst-obxd) | instrument / unknown | GPL-3.0-only | [obxd.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/obxd.json) |
| Omni Sampler | MrHighMan | [B0ss-M/MrHighMan](https://github.com/B0ss-M/MrHighMan) | instrument / unknown | LGPL-3.0-or-later | [omni-sampler.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/omni-sampler.json) |
| 303 | saustin2010 | [saustin2010/mpc-vst-303](https://github.com/saustin2010/mpc-vst-303) | instrument / unknown | GPL-3.0-only | [open303.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/open303.json) |
| OTTmpc | sd88me | [sd88me/mpc-vst-OTT](https://github.com/sd88me/mpc-vst-OTT) | effect / unknown | GPL-3.0-only | [ottmpc.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/ottmpc.json) |
| Overcast | Padbangers | [FullPace/overcast](https://github.com/FullPace/overcast) | effect / unknown | MIT | [overcast.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/overcast.json) |
| Percolator OUT | saustin2010 | [saustin2010/mpc-vst-percolator](https://github.com/saustin2010/mpc-vst-percolator) | effect / unknown | MIT | [percolator-out.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/percolator-out.json) |
| Percolator | saustin2010 | [saustin2010/mpc-vst-percolator](https://github.com/saustin2010/mpc-vst-percolator) | instrument / unknown | MIT | [percolator.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/percolator.json) |
| Pixel Walkers | saustin2010 | [saustin2010/mpc-vst-pixelwalkers](https://github.com/saustin2010/mpc-vst-pixelwalkers) | instrument / midi | MIT | [pixel-walkers.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/pixel-walkers.json) |
| PlateauXXL | L'Cronx | [sunskiefer/PlateauXXL](https://github.com/sunskiefer/PlateauXXL) | effect / unknown | GPL-3.0-or-later | [plateauxxl.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/plateauxxl.json) |
| Plugin Manager | poloq | [poloq-instruments/mpc-vst-manager](https://github.com/poloq-instruments/mpc-vst-manager) | instrument / unknown | MIT | [plugin-manager.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/plugin-manager.json) |
| PolyForce | Devko | [Devko/PolyForce](https://github.com/Devko/PolyForce) | instrument / unknown | MIT | [polyforce.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/polyforce.json) |
| Profit-08 | sd88me | [sd88me/mpc-vst-profit8](https://github.com/sd88me/mpc-vst-profit8) | instrument / unknown | MIT | [profit-08.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/profit-08.json) |
| PulyTek | Alberto Barrera (ABSounds); Force/MPC port by MagicStino | [MagicStino/pulytek](https://github.com/MagicStino/pulytek) | effect / unknown | GPL-3.0-only | [pulytek.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/pulytek.json) |
| RaffoSynth | saustin2010 | [saustin2010/mpc-vst-raffosynth](https://github.com/saustin2010/mpc-vst-raffosynth) | instrument / unknown | MIT | [raffosynth.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/raffosynth.json) |
| Rampage | saustin2010 | [saustin2010/mpc-vst-rampage](https://github.com/saustin2010/mpc-vst-rampage) | instrument / midi | GPL-3.0-or-later | [rampage.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/rampage.json) |
| MPC Remote | jacob-sabella | [jacob-sabella/mpc-addin-remote](https://github.com/jacob-sabella/mpc-addin-remote) | addin / unknown | MIT | [remote.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/remote.json) |
| Resonator FX | saustin2010 | [saustin2010/mpc-vst-resonator-fx](https://github.com/saustin2010/mpc-vst-resonator-fx) | effect / unknown | MIT | [rings-fx.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/rings-fx.json) |
| Resonator | saustin2010 | [saustin2010/mpc-vst-resonator](https://github.com/saustin2010/mpc-vst-resonator) | instrument / unknown | MIT | [rings.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/rings.json) |
| RMXXXL | L'Cronx | [sunskiefer/RMXXXL](https://github.com/sunskiefer/RMXXXL) | effect / unknown | GPL-3.0-or-later | [rmxxxl.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/rmxxxl.json) |
| MPC Skipback | sd88me | [sd88me/mpc-addin-skipback](https://github.com/sd88me/mpc-addin-skipback) | addin / unknown | MIT | [skipback.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/skipback.json) |
| StepRunner-8 | djokenzi | [djokenzi/steprunner-8](https://github.com/djokenzi/steprunner-8) | instrument / unknown | MIT | [steprunner-8.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/steprunner-8.json) |
| Stevequencer 16 | saustin2010 | [saustin2010/mpc-vst-stevequencer16](https://github.com/saustin2010/mpc-vst-stevequencer16) | instrument / midi | MIT | [stevequencer-16.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/stevequencer-16.json) |
| Stevequencer | saustin2010 | [saustin2010/mpc-vst-stevequencer](https://github.com/saustin2010/mpc-vst-stevequencer) | instrument / midi | MIT | [stevequencer.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/stevequencer.json) |
| Stream | johnny | [stampman3000-jpg/mpc-stream](https://github.com/stampman3000-jpg/mpc-stream) | effect / unknown | MIT | [stream.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/stream.json) |
| Sturm-TP | sd88me | [sd88me/mpc-vst-sturm](https://github.com/sd88me/mpc-vst-sturm) | instrument / unknown | MIT | [sturm-tp.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/sturm-tp.json) |
| SubForce | Devko | [Devko/SubForce](https://github.com/Devko/SubForce) | instrument / unknown | MIT | [subforce.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/subforce.json) |
| Super Arp | saustin2010 | [saustin2010/mpc-vst-superarp](https://github.com/saustin2010/mpc-vst-superarp) | instrument / midi | MIT | [super-arp.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/super-arp.json) |
| Tablor | saustin2010 | [saustin2010/mpc-vst-tablor](https://github.com/saustin2010/mpc-vst-tablor) | instrument / unknown | BSD-3-Clause | [tablor.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/tablor.json) |
| 6W6 | athousanddetails | [sd88me/mpc-vst-TR-MPC](https://github.com/sd88me/mpc-vst-TR-MPC) | instrument / unknown | GPL-3.0-only | [tr-6w6.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/tr-6w6.json) |
| 8W8 | athousanddetails | [sd88me/mpc-vst-TR-MPC](https://github.com/sd88me/mpc-vst-TR-MPC) | instrument / unknown | GPL-3.0-only | [tr-8w8.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/tr-8w8.json) |
| 9W9 | athousanddetails | [sd88me/mpc-vst-TR-MPC](https://github.com/sd88me/mpc-vst-TR-MPC) | instrument / unknown | GPL-3.0-only | [tr-9w9.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/tr-9w9.json) |
| CW-78 | athousanddetails | [sd88me/mpc-vst-TR-MPC](https://github.com/sd88me/mpc-vst-TR-MPC) | instrument / unknown | GPL-3.0-only | [tr-cw78.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/tr-cw78.json) |
| TR-MPC Tap FX | sd88me | [sd88me/mpc-vst-TR-MPC](https://github.com/sd88me/mpc-vst-TR-MPC) | effect / unknown | GPL-3.0-only | [tr-mpc-tap-fx.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/tr-mpc-tap-fx.json) |
| TR-MPC | sd88me | [sd88me/mpc-vst-TR-MPC](https://github.com/sd88me/mpc-vst-TR-MPC) | instrument / unknown | GPL-3.0-only | [tr-mpc.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/tr-mpc.json) |
| Tributary | johnny | [stampman3000-jpg/mpc-tributary](https://github.com/stampman3000-jpg/mpc-tributary) | instrument / unknown | GPL-3.0-only | [tributary.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/tributary.json) |
| MPC USB audio | jacob-sabella | [jacob-sabella/mpc-addin-usb-audio](https://github.com/jacob-sabella/mpc-addin-usb-audio) | addin / unknown | MIT | [usb-audio.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/usb-audio.json) |
| Verglas | saustin2010 | [saustin2010/mpc-vst-verglas](https://github.com/saustin2010/mpc-vst-verglas) | effect / unknown | MIT | [verglas.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/verglas.json) |
| Vibe FX | nachtaktiv303 | [nachtaktiv303/mpc-vst-vibe-fx](https://github.com/nachtaktiv303/mpc-vst-vibe-fx) | effect / unknown | MIT | [vibe-fx.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/vibe-fx.json) |
| Meta Mod | saustin2010 | [saustin2010/mpc-vst-meta-mod](https://github.com/saustin2010/mpc-vst-meta-mod) | effect / unknown | MIT | [warps.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/warps.json) |
| Wurl | saustin2010 | [saustin2010/mpc-vst-wurl](https://github.com/saustin2010/mpc-vst-wurl) | instrument / unknown | GPL-3.0-only | [wurl.json](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/catalog/plugins/wurl.json) |

## Follow-up: supplied portals and implementation sources

[ADDITIONAL_SOURCE_AUDIT.md](ADDITIONAL_SOURCE_AUDIT.md) adds pinned source audits of
Pd/Deken, cg-docs/Jam, norns/softcut, serialosc/libmonome and Ansible/Teletype, plus four
inspected patch/example archives. These are hosts/tools/patches or external firmware,
not additional verified MPC VST2 plugins. It also inspects the actual sd88me build,
skin/Studio, test, release and CI sources behind the supplied build/workflow pages.

No new native MPC Q-Link or CV API was discovered in those external projects. Strongest
direct tool reuse remains the MIT MPC framework; new host behavior needs adaptation.
The 116 catalog rows remain discovery-level records; this follow-up does not turn them
into complete per-plugin source audits or cover every project on either portal.
