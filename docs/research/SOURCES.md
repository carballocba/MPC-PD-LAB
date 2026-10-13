# Primary sources and reproducibility

Research date: 2026-10-13. Original MPC One, ARMv7 hard-float, Hakai 3.9.1.
Research only: no plugin/DSP/installer/system changes, hardware access or deployment.
Evidence labels: **source-verified**, **manufacturer-documented**, **upstream hardware report**,
**user-reported hardware result**, **inference/proposal**, and **not established** are distinct.
Exact source revisions and access gaps are in [SOURCES.md](SOURCES.md).

## Repository revisions inspected

| Key / repository | Exact revision / latest commit date at inspection | License scope |
|---|---|---|
| framework: [sd88me/mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins/tree/5e3a96885b6de76a48b498821c379568a05cc006) | `5e3a96885b6de76a48b498821c379568a05cc006` / 2026-10-13 | MIT |
| glass: [padbangers/glass-spheres](https://github.com/padbangers/glass-spheres/tree/1fe3b0a17b14dc07d327e5408d7aaf438715ce44) | `1fe3b0a17b14dc07d327e5408d7aaf438715ce44` / 2026-10-13 | MIT; inspect vendored notices |
| acid: [sd88me/mpc-vst-acid](https://github.com/sd88me/mpc-vst-acid/tree/d4bb7c23c3fb6b8203350c9f34b06d6c8ea7f50b) | `d4bb7c23c3fb6b8203350c9f34b06d6c8ea7f50b` / 2026-10-13 | GPL-3.0-only (catalog; verify each imported file) |
| crate: [sd88me/mpc-vst-cratedigger](https://github.com/sd88me/mpc-vst-cratedigger/tree/e8ed11227d36e05e7c5245288745c3634c3adbf8) | `e8ed11227d36e05e7c5245288745c3634c3adbf8` / 2026-10-11 | MIT; third-party notices separate |
| dx7: [sd88me/mpc-vst-dx7](https://github.com/sd88me/mpc-vst-dx7/tree/400fa20a506c8bc7c72105358ca85f27eefe3315) | `400fa20a506c8bc7c72105358ca85f27eefe3315` / 2026-10-10 | GPL-3.0-only (catalog; third-party notices separate) |
| nam: [jacob-sabella/mpc-vst-nam](https://github.com/jacob-sabella/mpc-vst-nam/tree/cf2548f796c96b8ca4fdd4582af8514d82a24830) | `cf2548f796c96b8ca4fdd4582af8514d82a24830` / 2026-10-10 | MIT; model/data licenses separate |
| camomile: [pierreguillot/Camomile](https://github.com/pierreguillot/Camomile/tree/03fe38053229e37fb98b89049b0d8bdf35213c14) | `03fe38053229e37fb98b89049b0d8bdf35213c14` / 2024-02-21 | GPL-3.0 root license; JUCE/Pd dependencies separate |
| plugdata: [plugdata-team/plugdata](https://github.com/plugdata-team/plugdata/tree/4e6843671e1561d666d9507d9e5e7a7b7dc7836b) | `4e6843671e1561d666d9507d9e5e7a7b7dc7836b` / 2026-10-12 | GPL-3.0 root license; numerous separate dependencies |
| hvcc: [Wasted-Audio/hvcc](https://github.com/Wasted-Audio/hvcc/tree/14812fb1bf6ec62a17e46db9835f8bda8fd9dddb) | `14812fb1bf6ec62a17e46db9835f8bda8fd9dddb` / 2026-10-09 | GPL-3.0 compiler; ISC-style generated runtime files |
| organelle: [critterandguitari/Organelle_OS](https://github.com/critterandguitari/Organelle_OS/tree/1f549fddc2dfd7d6ac1f2310ef01278ddf361856) | `1f549fddc2dfd7d6ac1f2310ef01278ddf361856` / 2026-09-05 | BSD-3-Clause except attributed imports |
| patches: [critterandguitari/Organelle_Patches](https://github.com/critterandguitari/Organelle_Patches/tree/72d19ed847ee40a89e6f40bcebce56229be1ef0b) | `72d19ed847ee40a89e6f40bcebce56229be1ef0b` / 2026-04-28 | Per patch; Granular Freezer BSD plus GPL panning adaptations |
| zynthian: [zynthian/zynthian-data](https://github.com/zynthian/zynthian-data/tree/4920bea5a6327824eee8500987a1d3dcd936f7fb) | `4920bea5a6327824eee8500987a1d3dcd936f7fb` / 2026-09-18 | GPL-3.0 root; nested patch notices separate |
| gitnob: [gitnob/pd_to_zynthian_oram](https://github.com/gitnob/pd_to_zynthian_oram/tree/0a1f626ec2aafe0b1502f062182f3bb2e91c34b2) | `0a1f626ec2aafe0b1502f062182f3bb2e91c34b2` / 2024-09-15 | GPL-3.0 |
| phazerville: [djphazer/O_C-Phazerville](https://github.com/djphazer/O_C-Phazerville/tree/195932ab6f1d24813b317f9e5f76dc4957ee6ef0) | `195932ab6f1d24813b317f9e5f76dc4957ee6ef0` / 2026-08-10 | Per-file; CaptainMIDI/hMIDIIn MIT headers, imported code separate |
| crow: [monome/crow](https://github.com/monome/crow/tree/b340579d94e57e2b43336611b2c4037cff74bb80) | `b340579d94e57e2b43336611b2c4037cff74bb80` / 2026-04-18 | GPL-3.0 root; submodules separate |
| link: [Ableton/link](https://github.com/Ableton/link/tree/9c9091275e707ab09d09a5a608fcdb84bf0dec85) | `9c9091275e707ab09d09a5a608fcdb84bf0dec85` / 2026-09-23 | GPL-2.0-or-later; alternative licensing offered by Ableton |
| explore: [TheKikGen/MPC-LiveXplore](https://github.com/TheKikGen/MPC-LiveXplore/tree/9be3e63bf03d3467cf06478b48671894dcc634f4) | `9be3e63bf03d3467cf06478b48671894dcc634f4` / 2025-03-03 | Mixed/verify per file; no blanket reuse approval |
| libpd: [libpd/libpd](https://github.com/libpd/libpd/tree/ba0dc63262901d658af8bbda5e619a60fa975e78) | `ba0dc63262901d658af8bbda5e619a60fa975e78` / 2026-06-21 | BSD-style LICENSE.txt and separately licensed imports |
| pd: [pure-data/pure-data](https://github.com/pure-data/pure-data/tree/f009fd8d7b537e209e09898d487fdf1bf547da2b) | `f009fd8d7b537e209e09898d487fdf1bf547da2b` / 2026-06-17 | Standard Improved BSD except attributed imports |
| ours: [carballocba/MPC-PD-LAB](https://github.com/carballocba/MPC-PD-LAB/tree/879ab14336fd5c4576ab9832127b4047aaa2a2b6) | `879ab14336fd5c4576ab9832127b4047aaa2a2b6` / 2026-10-13 | Project-wide license undeclared; upstream notices retained |

Dates above describe checked-out branch-tip commits, not a guarantee of each project's latest
release. Shallow research clones were inspected without installing them as project dependencies.
No submodule builds or new plugin framework integrations were performed.

## Manufacturer and standards evidence

- [Akai MPC One FAQ](https://support.akaipro.com/en/support/solutions/articles/69000816149-akai-pro-mpc-one-frequently-asked-questions).
- [Akai Standalone OS 3.5 manual](https://cdn.inmusicbrands.com/Software/SME0-3.5.0/Public/MPC%20Standalone%20OS%20-%20User%20Guide%20-%20v3.5.pdf): PDF pages 51, 119, 344, 357 and hardware specifications. Original One connector/breakout and CV-track routing evidence.
- [Akai Standalone OS 3.6 manual](https://cdn.inmusicbrands.com/Software/36/MPC%20Standalone%20OS%20-%20User%20Guide%20-%20v3.6.pdf): pages 472–478, Advanced CV Track Edit and Q-Link modulation. Version 3.6 documentation is not an exact Hakai 3.9.1 implementation audit.
- [Akai CV/Gate troubleshooting](https://support.akaipro.com/en/support/solutions/articles/69000858173-akai-pro-troubleshooting-cv-gate-outputs) and [Akai program overview](https://support.akaipro.com/ja-JP/support/solutions/articles/69000804211): nominal 5V gate and 0–5V V/oct pitch evidence.
- [Akai Pro Pack support](https://support.akaipro.com/en/support/solutions/articles/69000873513-akai-pro-mpc-series-mpc-pro-pack): CV playground is an additional licensed feature, not a public VST API.
- [ALM Pamela's NEW Workout manual](https://assets.busycircuits.com/docs/alm017-manual.pdf): manufacturer's current asset URL discovered from its product page; manual v0.42, firmware 199/200, 28 pages. Pages 15–17 for CV/clock/run configuration.
- [Intellijel MIDI 1U manual](https://intellijel.com/downloads/manuals/midi-1u_manual_v1.2_2026.01.06.pdf): pitch, gate, clock and reset/run routes.
- [Phazerville MIDI Input](https://firmware.phazerville.com/MIDI-Input), [Captain MIDI](https://firmware.phazerville.com/Captain-MIDI): consult pinned source too; hardware variants differ.
- [Norns clock/control](https://monome.org/docs/norns/control-clock/), [crow specifications](https://monome.org/docs/crow/), [crow FAQ](https://monome.org/docs/crow/faq/): crow is serial USB, not a generic USB-MIDI converter.
- [Expert Sleepers FH-2](https://www.expert-sleepers.co.uk/fh2.html), [ES-9](https://www.expert-sleepers.co.uk/es9.html): documented external MIDI/CV and DC-coupled USB audio alternatives. No MPC-specific acceptance test performed.
- [Ableton Link concepts](https://ableton.github.io/link/) and [start/stop FAQ](https://help.ableton.com/hc/en-us/articles/209776125-Link-features-and-functions-FAQ).
- [Steinberg VST2 discontinuation](https://helpcenter.steinberg.de/hc/en-us/articles/4409561018258-VST-2-Discontinued), [VST licensing portal](https://steinbergmedia.github.io/vst3_dev_portal/pages/VST%2B3%2BLicensing/Which%2Bfiles%2Bfall%2Bunder%2Bwhich%2Blicense.html). Do not mistake VST3's newer MIT licensing for a grant covering old VST2 SDK material.

## Firmware/community gaps and unsuccessful searches

The historical RedHate/Hakai-MPC URL returned 404 in Phase 3. The surviving
[henning/Hakai-MPC fork at 0c7008c7a5cd62cd2dc331e7b657bdcee19b4625](https://github.com/henning/Hakai-MPC/tree/0c7008c7a5cd62cd2dc331e7b657bdcee19b4625)
contains an older launcher/content-mount implementation, not current Hakai 3.9.1 host source
or a CV API. MPC-LiveXplore supplies controller/modding research, not proof of the One's CV protocol.
No official MPC One DAC schematic, DAC part identification, calibrated transfer curve,
CV refresh/settling specification, or open Hakai 3.9.1 CV implementation was located.
Absence in this search does not prove none exists privately.

Akai's modern manuals/Pro Pack landing pages returned only a client-rendered shell to the
text browser; downloaded primary PDF manuals and support articles supply the usable evidence.
The old busycircuits.com/docs/alm017-manual.pdf URL returned 404; its current assets subdomain works.
The initial 3.6 PDF extraction needed an AES-capable parser dependency; extraction then succeeded.
No bypass of access controls was attempted. Forum/Reddit results were discovery leads, not electrical specifications.
The guessed pd-externals/pan repository from the prior audit was unavailable; the actual
[Debian pan 0.1.2 source archive](https://deb.debian.org/debian/pool/main/p/pd-pan/pd-pan_0.1.2.orig.tar.gz)
provides the panning-library notice. Its SHA256 is `594ae9dd2c3aa22942ee53c7a7ff8ddaa9dbb6db0944efb55185d63ee9d18ff9`.

## Evidence limits

The user now reports Delay and Granular Freezer working on the original One/Hakai 3.9.1,
including freeze and all six Q-Links; no functional GUI. These are accepted user reports,
not newly executed tests. Phase 4's older documents describe the state before that report.
[PR 4](https://github.com/carballocba/MPC-PD-LAB/pull/4) is inspected as a separate branch;
research main remains at ae21b99. No merge was requested or performed.

This is a broad, reproducible audit of the enumerated sources, not a claim to have found
all private community code or inspected every catalog plugin's implementation. Catalog-only
entries are explicitly triage records. Untested branches are not hardware certifications.

## Downloaded primary-document fingerprints

- `mpc35.pdf`: SHA256 `8f34852e41ae2da40ff3e600b4c1b5eae976a228de2dc2a5002091aa0ce345cd`. Research copy only; manual not redistributed.
- `mpc36.pdf`: SHA256 `7dfae219d5b203092f550fd5a945f0b243d67f5b599c4e18734df1e5a08faee6`. Research copy only; manual not redistributed.
- `pam.pdf`: SHA256 `c3b96b49fde3c1f2f6fc5a672f63bfe6cf2cc54264498bd5d792d1e6021bd121`. Research copy only; manual not redistributed.

## Follow-up search and coverage limits

Follow-up broad GitHub searches on 2026-10-13 included `MPC VST Hakai Pure Data
plugin skin Q Links`, `sd88me mpc-vst-plugins issues skin MIDI`, and
`libpd embedded dynamic patch loader Organelle`.
Additional discovery leads include [gmorb's Dragonfly port](https://github.com/gmorb/mpc-vst-dragonfly),
[KoreTeknology's MPC resources](https://github.com/KoreTeknology/Free-Resources-for-MPC-Software),
and the [Organelle manual repository](https://github.com/critterandguitari/Organelle_Manual).
These follow-up leads have not received a pinned implementation audit here and are not
promoted to verified reuse recommendations. Search snippets are discovery leads only.

This report is a substantial initial audit, not a completed exhaustive audit of every
public fork, issue, PR, binary-only plugin or project branch. The 116-entry catalog
inventory must not be read as 116 full source audits. Six focused MPC implementation
cards plus the listed host/Organelle/CV sources were inspected. An exhaustive per-project
16-field source audit, systematic issue/PR history survey, complete all-branch own-code
review and dependency-by-dependency legal clearance remain open coverage items.
No unavailable firmware implementation or hardware behavior is inferred to fill them.

## Additional supplied-source audit (2026-10-13)

See [ADDITIONAL_SOURCE_AUDIT.md](ADDITIONAL_SOURCE_AUDIT.md) for the six entry URLs,
source paths, architecture/dependency/licensing findings and evidence limits. Follow-up
inspections do not silently replace the original pinned libpd/Pd/framework versions.

| Repository | Exact revision / date | License boundary |
|---|---|---|
| [pure-data/pure-data](https://github.com/pure-data/pure-data/tree/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9) | `f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9` / 2026-10-05 | Standard Improved BSD except imports |
| [critterandguitari/cg-docs](https://github.com/critterandguitari/cg-docs/tree/b6f2633e7c39f78f7d924f9bf6084eaa0144bbff) | `b6f2633e7c39f78f7d924f9bf6084eaa0144bbff` / 2026-10-02 | No repository-wide license found; documentation is not a patch license |
| [owenosborn/jampd](https://github.com/owenosborn/jampd/tree/1c061e8efa423399d4045958fb4e9d3c8fb294d2) | `1c061e8efa423399d4045958fb4e9d3c8fb294d2` / 2026-04-12 | No explicit license found in checkout |
| [monome/norns](https://github.com/monome/norns/tree/6302a0d4dcb85ab3b701a4d7268863092b693caf) | `6302a0d4dcb85ab3b701a4d7268863092b693caf` / 2026-10-11 | GPL-3.0 root; dependencies separately licensed |
| [monome/softcut-lib](https://github.com/monome/softcut-lib/tree/14241f2a94a7cd2e6c91b9bdc4e500d68abef907) | `14241f2a94a7cd2e6c91b9bdc4e500d68abef907` / 2024-12-31 | GPL-3.0 root |
| [monome/libmonome](https://github.com/monome/libmonome/tree/9055ea22fd824839be821c10f6e9b13e9d2aa3cd) | `9055ea22fd824839be821c10f6e9b13e9d2aa3cd` / 2026-08-27 | ISC-style COPYRIGHT |
| [monome/serialosc](https://github.com/monome/serialosc/tree/c96ea389dbf82c84d17f6f7adddaf311aed49438) | `c96ea389dbf82c84d17f6f7adddaf311aed49438` / 2026-08-27 | ISC-style core; static dependencies include LGPL-2.1-or-later liblo |
| [monome/docs](https://github.com/monome/docs/tree/de746029ef57244036df2a206e5b894a29b5e0f6) | `de746029ef57244036df2a206e5b894a29b5e0f6` / 2026-10-01 | CC-BY-4.0 LICENSE; MIT LICENSE.txt is attributed to theme author |
| [monome/teletype](https://github.com/monome/teletype/tree/e390bacdf024f2234e2ade6a4821773b9a80a322) | `e390bacdf024f2234e2ade6a4821773b9a80a322` / 2026-06-24 | GPL-2.0 root; per-file/vendor review required |
| [monome/ansible](https://github.com/monome/ansible/tree/9796656cbbe9eb0227d380d555e31192f0cf32bf) | `9796656cbbe9eb0227d380d555e31192f0cf32bf` / 2025-09-05 | GPL-2.0 root; per-file/vendor review required |
| [pure-data/deken](https://github.com/pure-data/deken/tree/785862399822b6d12f4804cf7b12f5ed55ee10c4) | `785862399822b6d12f4804cf7b12f5ed55ee10c4` / 2026-08-28 | Standard Improved BSD except imports |

Archive fingerprints (downloaded for inspection, not redistributed):

| Asset | SHA-256 |
|---|---|
| `grid.zip` | `d822624dbedea605ef9038db2081710593272cf38b79e525c37b735aa23a7b01` |
| `play.zop` | `97cfed75acce27271f21ca413b988d4888832114782c191f53ea3119521fd3bc` |
| `ccquencer2.zip` | `61703b2e500e0d44051a777228ced5c7e6a555e39784342b54cf52927ac1f8df` |
| `midi-maker.zip` | `53968c4e6d5fef25a8a3f38f244f1e6f9c0af062f019755a09d640d047df3fc1` |

The monome grid archive is also tracked as source under the pinned monome/docs
`grid/studies/pd/files/` tree. PatchStorage archives were linked by pinned cg-docs patch
pages; their hashes, internal paths and static observations are recorded in the additional
audit. Their download permissions do not establish blanket redistribution permission.

License files inspected: [pure-data/pure-data: LICENSE.txt](https://github.com/pure-data/pure-data/blob/f3497d41dd28d4ec1906a0601ff4c0a661c3e5d9/LICENSE.txt), [pure-data/deken: LICENSE.txt](https://github.com/pure-data/deken/blob/785862399822b6d12f4804cf7b12f5ed55ee10c4/LICENSE.txt), [monome/norns: LICENSE.txt](https://github.com/monome/norns/blob/6302a0d4dcb85ab3b701a4d7268863092b693caf/LICENSE.txt), [monome/softcut-lib: LICENSE.txt](https://github.com/monome/softcut-lib/blob/14241f2a94a7cd2e6c91b9bdc4e500d68abef907/LICENSE.txt), [monome/libmonome: COPYRIGHT](https://github.com/monome/libmonome/blob/9055ea22fd824839be821c10f6e9b13e9d2aa3cd/COPYRIGHT), [monome/serialosc: COPYRIGHT](https://github.com/monome/serialosc/blob/c96ea389dbf82c84d17f6f7adddaf311aed49438/COPYRIGHT), [monome/docs: LICENSE](https://github.com/monome/docs/blob/de746029ef57244036df2a206e5b894a29b5e0f6/LICENSE), [monome/teletype: LICENSE](https://github.com/monome/teletype/blob/e390bacdf024f2234e2ade6a4821773b9a80a322/LICENSE), [monome/ansible: LICENSE](https://github.com/monome/ansible/blob/9796656cbbe9eb0227d380d555e31192f0cf32bf/LICENSE).

No license file or explicit source grant was located in the inspected cg-docs/Jam trees.

## Loader design diagnostic sources (2026-10-13)

- Runtime wrapper actually pinned by the working bridge: [208745830fc2ed90b2fd0dc480ef5c554284292f, wrapper/vst2_wrap.c](https://github.com/sd88me/mpc-vst-plugins/blob/208745830fc2ed90b2fd0dc480ef5c554284292f/wrapper/vst2_wrap.c).
- Proposed new tokenizer, audited but not integrated: [zserge/jsmn at 25647e692c7906b96ffd2b05ca54c097948e879c](https://github.com/zserge/jsmn/tree/25647e692c7906b96ffd2b05ca54c097948e879c), [jsmn.h](https://github.com/zserge/jsmn/blob/25647e692c7906b96ffd2b05ca54c097948e879c/jsmn.h), [MIT LICENSE](https://github.com/zserge/jsmn/blob/25647e692c7906b96ffd2b05ca54c097948e879c/LICENSE). Tip date 2021-10-14; small C tokenizer, no runtime-library dependency; schema validation remains ours. Four native test configurations passed 16 checks each.
- [Hidden-symbol investigation](../design/PD_SYMBOL_BOUNDARY.md): local Phase 4 CI ELF export/local-symbol observations and pinned loader/header/build sources.
- [Full failure classification](../design/UPSTREAM_TEST_FAILURE_CLASSIFICATION.md): original suite reproduced, controlled checksum/sed/wc diagnostic tool compatibility, exact 24-case dispositions. No real MPC settings/services were used.
