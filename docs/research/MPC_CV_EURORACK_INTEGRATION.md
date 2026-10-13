# MPC One CV/Gate, Eurorack and modular integration

Research date: 2026-10-13. Original MPC One, ARMv7 hard-float, Hakai 3.9.1.
Research only: no plugin/DSP/installer/system changes, hardware access or deployment.
Evidence labels: **source-verified**, **manufacturer-documented**, **upstream hardware report**,
**user-reported hardware result**, **inference/proposal**, and **not established** are distinct.
Exact source revisions and access gaps are in [SOURCES.md](SOURCES.md).

## Decision

Use MPC's own CV tracks/playground and established external MIDI-to-CV hardware first.
For Universal, prioritize Pd MIDI generation through the proven community virtual-MIDI
route, then a separately validated MPC One direct-CV backend. **A working direct-CV source
precedent exists on MPC X; direct output from our plugin on the original One is not verified.**
Do not identify VST audio samples, host parameter automation and voltage output as the same API.

## Verified hardware and unknown architecture

Manufacturer-documented: original MPC One has four 3.5mm CV/Gate connectors carrying
up to eight signals via TRS-to-dual-TS breakouts; a TS lead accesses one signal per connector.
These are two independent mono controls, not balanced audio. There are no documented
CV inputs. Ordinary audio inputs are not a substitute for calibrated DC CV inputs.
See [Akai 3.5 manual, original One rear panel, p344](https://cdn.inmusicbrands.com/Software/SME0-3.5.0/Public/MPC%20Standalone%20OS%20-%20User%20Guide%20-%20v3.5.pdf#page=344).

Akai documents approximately +5V gates and 0–5V, 1V/oct pitch operation in its
[troubleshooting](https://support.akaipro.com/en/support/solutions/articles/69000858173-akai-pro-troubleshooting-cv-gate-outputs)
and [program overview](https://support.akaipro.com/ja-JP/support/solutions/articles/69000804211).
Treat outputs as unipolar; negative voltages, 8–10V envelopes, Hz/V and S-trigger operation
are not established. Verify physical tip/ring-to-logical-channel mapping on the actual One,
using the rear-panel pair labels and measured isolated outputs; do not copy MPC X jack ordering.

The public documentation does not establish the One's DAC chip, physical bit depth,
output impedance/current limit, exact maximum voltage, factory calibration storage,
settling time, controller scan rate or guaranteed jitter. These must remain unknown.
Pitch calibration of a receiving oscillator is separate from calibrating the MPC output DAC.
A panel's voltage knob is not a calibration traceability guarantee.

**Functional architecture inferred from X source:** MPC application → internal controller
raw-MIDI messages → controller/DAC/output circuitry. This is evidence against treating
CV as ordinary PCM audio, but the original One's implementation still needs confirmation.
Its ARM CPU and similar software alone do not prove an identical controller protocol.

## Host-mediated CV and APIs

Akai documents melodic CV tracks with pitch, gate, mod-wheel and velocity destinations,
plus note priority; drum CV tracks allow per-pad output/data-type assignment. CV tracks
have MIDI input-port/channel and monitoring controls. Therefore incoming note/CC data can
be interpreted by a CV track; the exact channel, monitoring and mod-wheel behavior must
be tested on Hakai 3.9.1. Host sequencing and recording keep the host as output owner.
[Akai 3.5, CV Tracks pp51/119](https://cdn.inmusicbrands.com/Software/SME0-3.5.0/Public/MPC%20Standalone%20OS%20-%20User%20Guide%20-%20v3.5.pdf#page=51).

The licensed Pro Pack adds four envelopes, four LFOs, a stepper, ramps/offsets,
clock division and a modulation matrix; routing-page changes replace main CV settings.
It also describes Q-Link modulation sources/targets. These are strong existing solutions
for modulation, but availability/activation on this user's Hakai installation is unknown.
No reviewed public API lets a VST set arbitrary CV playground internals or assign their
routing programmatically. Do not assume audio inserts can route directly into those CVs.
[Akai 3.6, pp472–478](https://cdn.inmusicbrands.com/Software/36/MPC%20Standalone%20OS%20-%20User%20Guide%20-%20v3.6.pdf#page=472).

| Interface | Evidence / availability |
|---|---|
| VST audio output buffers | Existing stereo wrapper; values are normalized audio, not calibrated CV commands |
| VST parameter enumeration/automation | Available; controls the plugin's parameters, not an arbitrary host CV port |
| VST tempo/PPQ/transport | [wrapper/vst2_wrap.c](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/wrapper/vst2_wrap.c), [docs/MIDI_TIMING.md](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/docs/MIDI_TIMING.md); Force hardware reports; map explicitly into Pd |
| Standard VST MIDI output callback | [poc/midiout.c](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/poc/midiout.c) and upstream notes report MPC ignores it; do not rely on callback success as proof of routing |
| ALSA sequencer virtual MIDI port | [vst/acid_vst.cpp](https://github.com/sd88me/mpc-vst-acid/blob/d4bb7c23c3fb6b8203350c9f34b06d6c8ea7f50b/vst/acid_vst.cpp), [src/midi_out.cc](https://github.com/padbangers/glass-spheres/blob/1fe3b0a17b14dc07d327e5408d7aaf438715ce44/src/midi_out.cc); community hardware precedent, One routing requires test |
| Official/public CV-specific VST opcode | None located in reviewed source/documentation |
| Private controller raw-MIDI write | Concrete X implementation below; unsupported private interface, One unverified |
| ALSA PCM / extra VST outputs | [poc/multiout.c](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/poc/multiout.c) is an audio-output experiment, not a discovered CV API |

## Direct CV precedent: Glass Spheres

Inspect [cv/README.md](https://github.com/padbangers/glass-spheres/blob/1fe3b0a17b14dc07d327e5408d7aaf438715ce44/cv/README.md), [cv/cv-capture-1.log](https://github.com/padbangers/glass-spheres/blob/1fe3b0a17b14dc07d327e5408d7aaf438715ce44/cv/cv-capture-1.log),
[src/cv_out.cc](https://github.com/padbangers/glass-spheres/blob/1fe3b0a17b14dc07d327e5408d7aaf438715ce44/src/cv_out.cc), [src/cv_out.h](https://github.com/padbangers/glass-spheres/blob/1fe3b0a17b14dc07d327e5408d7aaf438715ce44/src/cv_out.h),
[src/engine.cc](https://github.com/padbangers/glass-spheres/blob/1fe3b0a17b14dc07d327e5408d7aaf438715ce44/src/engine.cc) and [tested.json](https://github.com/padbangers/glass-spheres/blob/1fe3b0a17b14dc07d327e5408d7aaf438715ce44/tested.json).
The test record names **MPC X, OS 3.9.1, 2026-10-04**. Its README's statement that only
MPC X has CV jacks is contradicted by Akai's One documentation. Correct that inference
before porting; do not discard the actual X protocol capture.

The captured X protocol uses channel-16 CC pairs: logical output n, zero-based,
uses CC 2n as MSB and CC 2n+1 as LSB, each with full status 0xBF. The combined command
is 14-bit, 0–16383, nominally scaled by 3276.8 codes/volt and clamped to roughly 0–5V.
This describes **command resolution**, not independently measured physical DAC resolution
or calibrated One accuracy. The upstream octave/LSB evidence is auditory, not a precision
voltage/linearity sweep. A nominal 5V/16384 command step is about 0.305mV, or 0.366 cents
at 1V/oct; these are calculations, not measured effective accuracy.

The plugin locates MPC's already-open raw-MIDI output fd using `/proc/self/fd` and an
ALSA raw-MIDI info ioctl. It does not open a documented CV driver or claim a VST opcode.
Captured device numbers such as card2/subdevice1 belong to that X session, not the One.
Port-name heuristics alone are insufficient hardware identification for Universal.

Source-review concerns to resolve before reuse:

- A sender thread keeps device writes off the audio callback, but coalesces a latest value
  per output. `WriteOutputs` stretches an intra-block gate to one block; the worker can
  still miss the entire high/low transition if scheduling/coalescing overwrites it.
- The sender sleeps at least 2ms between batches and can wait up to 10ms after a missed
  wakeup. These are code constants, not output jitter measurements or guarantees.
- `write` handling only specially retries discovery for EBADF. Partial writes, EAGAIN,
  other failures, stale fd reuse and reliable shutdown flush need stronger handling.
- Latest-claim-wins ownership is static within this loaded module. It does not arbitrate
  all other plugin DSOs, Akai CV tracks or other controller writers.
- The X capture shows host stop zeroing ports used by CV tracks. Separate ownership is
  necessary; plugin and host cannot safely compete for the same output.
- The combined MSB/LSB sequence is not a timestamped voltage transaction. Ordering,
  controller latch behavior and concurrent writes need measurement.
- Existing capture tools use preload/bind-mounted launchers. **Do not run them here or
  make launcher modification part of the default architecture.** No injection commands
  or CV output code were executed in this research.

**Reuse classification:** ADAPT EXISTING CODE + REQUIRES HARDWARE VALIDATION.
Use the protocol and tests as evidence; do not transplant the sender unchanged as a
lossless trigger sequencer or claim it is supported on all MPC hardware.

## Feasible routing architectures

| Route | Concrete data flow | Assessment |
|---|---|---|
| Native sequencer | MPC CV track → built-in CV/Gate → modular | Best first baseline; manufacturer-supported workflow |
| Pd via host CV track | Pd notes/CC → worker/virtual ALSA port → monitored MPC CV track → built-in outputs | Strong near-term candidate; adapt proven virtual port; test internal input routing/loops |
| Pd via external MIDI-to-CV | Pd → virtual port/host external MIDI track → DIN/USB MIDI interface → modular | Reuses established converters; additional latency/bandwidth, channel and USB role checks |
| Pd direct built-in CV | Pd control/event lanes → owned output service → private controller fd → outputs | X source precedent; One protocol, calibration, arbitration and event timing are blockers |
| Pd audio/CV through DC interface | Pd float audio lanes → VST/audio track → selected USB output → verified DC-coupled interface | Candidate for audio-rate modulation; host channel routing, DC preservation and calibration untested |
| Modular to Pd | Modular → external CV-input converter or DC-coupled ADC → MIDI/audio → Pd | Requires external hardware; no built-in One CV input assumed |

The current int16 stereo engine interface is not a general multichannel voltage transport.
Audio-rate/DC output would require audited float processing, dedicated channel routing and
proof that mixers, effects, mute/solo, master level and conversion preserve DC. Do not route
control signals to speakers. ES-9 has documented DC-coupled outputs; this does not prove
MPC USB enumeration or VST multichannel routing. ES-3 requires an appropriate ADAT source;
the One has no built-in ADAT output. Conventional AC-coupled line/headphone outputs cannot
hold pitch CV or static offsets reliably. [Expert Sleepers ES-9](https://www.expert-sleepers.co.uk/es9.html).

## What Pd can generate versus what a backend can deliver

| Function | Preferred existing mechanism | Fidelity / restriction |
|---|---|---|
| Pitch V/oct | MIDI notes/pitch bend → native CV track or calibrated converter | Establish note-to-voltage reference and bend range; MIDI note labels vary by octave naming |
| Gates/triggers | Note on/off or queued edge events | Ordered events and minimum physical pulse width; never last-value coalescing |
| Clock pulses | MPC sequence/native clock or converter's hardware clock engine | PPQN, pulse width, run and reset semantics must match |
| Reset | Dedicated pulse/run lane, converter start/reset mapping | Reset is not universally identical to run; test edge ordering |
| LFO | Native CV playground or external converter's local synced LFO | Best reuse; CC/direct sampled output is lower-rate control, not sample accurate |
| Random/stepped CV | Pd events → quantized/rate-limited controls; Phazerville/Marbles as external engines | Seed/restart behavior and intended smoothing must be specified |
| Envelopes | Host envelopes or local hardware envelopes triggered by Pd notes | Avoid streaming every envelope sample through MIDI |
| Sequencers | Pd ordered note/CC events → host/converter | Reuse transport-grid logic for loops, locates and tempo changes |
| Probability triggers | Pd decides discrete events; event backend guarantees delivery or explicit overflow | Decide ahead of deadlines; retain deterministic seed optionally |
| Generative modulation | Pd control/event outputs + native/external modulation engines | Backend determines achievable rate/range, not Pd signal rate alone |

A future backend-neutral patch contract should distinguish normalized control values,
explicit volts, pitch units and timestamped gate edges. Ports, allowed ranges, polarity,
calibration, slew/rate limits, minimum pulse width, ownership and stop/panic policy belong
in the manifest. This is a design proposal, not newly implemented software.

## Timing and synchronization

At the current wrapper's 44.1kHz/128-frame contract, one block is about 2.902ms; a Pd tick
is 64 samples, about 1.451ms. Neither is a measured physical-CV latency. X's worker layer,
USB/controller processing, host routing and external converters add their own delay/jitter.
A high-resolution command code says nothing about time resolution.

Reuse absolute host PPQ/transport scheduling from [docs/MIDI_TIMING.md](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/docs/MIDI_TIMING.md)
and [vst/acid_vst.cpp](https://github.com/sd88me/mpc-vst-acid/blob/d4bb7c23c3fb6b8203350c9f34b06d6c8ea7f50b/vst/acid_vst.cpp). Handle play/stop, loop wrap, locate, tempo change and
lost/duplicate blocks; do not accumulate synthetic MIDI clock ticks as absolute song phase.
The wrapper's optional song-position callback exists, but our current Organelle engine
leaves it null. Pd transport receivers therefore need explicit future adaptation.

MIDI clock is 24 pulses/quarter, with distinct start/continue/stop and song-position rules.
DIN MIDI serial bandwidth is 31.25kbit/s: a three-byte message with framing takes about
0.96ms on a physical DIN link. Eight CC lanes at 100Hz would require 2400 payload bytes/s,
roughly 77% of its 3125-byte/s framed capacity before notes/clock; USB and the internal
controller link must not be assumed to have that same limit or zero scheduling jitter.
A 7-bit 0–5V CC mapping has about 39.4mV between adjacent values (about 47 cents V/oct).
Use notes/bend for pitch and local interpolation/generation for smooth modulation.

Prefer MPC as the single tempo/transport authority for a project. Let its Link integration
follow peers, and read the resulting host transport in the plugin; avoid adding a second
competing Link participant. Link shares beat/tempo/phase; optional Start Stop Sync is not
universal song-position/reset synchronization. Embedding Link directly introduces GPL or
alternative-license questions: [LICENSE.md](https://github.com/Ableton/link/blob/9c9091275e707ab09d09a5a608fcdb84bf0dec85/LICENSE.md). External Eurorack clock pulses
still need a hardware bridge; Link is not a voltage output API.

## Integration with specific systems

**Pamela's NEW Workout (ALM-017, not PRO):** manufacturer manual supports external clock,
configurable 1–48 PPQN and optional run control; default 24 PPQN plus run. CV1 uses 0–5V,
CV2 −5 to +5V. Use a verified native/converter clock plus a distinct run signal, configure
both ends identically, and let Pam generate local divisions/modulation. Its manual warns
that poor computer MIDI clocks propagate jitter; it does not promise to repair a sloppy
source. The original One's unipolar output can address the positive part of bipolar CV2,
not its whole range without conditioning. Reset/run assignments depend on its menu mode.
[ALM manual pp15–17](https://assets.busycircuits.com/docs/alm017-manual.pdf#page=15).

**Ornament & Crime / Phazerville:** [software/src/apps/CaptainMIDI.h](https://github.com/djphazer/O_C-Phazerville/blob/195932ab6f1d24813b317f9e5f76dc4957ee6ef0/software/src/apps/CaptainMIDI.h),
[software/src/applets/hMIDIIn.h](https://github.com/djphazer/O_C-Phazerville/blob/195932ab6f1d24813b317f9e5f76dc4957ee6ef0/software/src/applets/hMIDIIn.h), [software/src/HSMIDI.h](https://github.com/djphazer/O_C-Phazerville/blob/195932ab6f1d24813b317f9e5f76dc4957ee6ef0/software/src/HSMIDI.h)
and [software/src/OC_DAC.cpp](https://github.com/djphazer/O_C-Phazerville/blob/195932ab6f1d24813b317f9e5f76dc4957ee6ef0/software/src/OC_DAC.cpp) are concrete MIDI conversion, clock/run,
voice-allocation and DAC/calibration references. Use existing applets as external hardware
rather than porting MCU DAC code into the MPC. Output voltage range, calibration, USB port,
firmware build and supported applets depend on the physical O_C/ORN8/uO_C revision.
Do not equate all Phazerville builds. CV-to-MIDI on that external module can supply a
return control path; it does not add an internal One CV ADC.

**Intellijel systems:** use native pitch/gate into compatible modules, or MIDI 1U's
manufacturer-defined pitch/gate/velocity/modulation plus clock and configurable reset/run.
Its specified pitch range differs from the One; configure reference note/bend deliberately.
Intellijel cases' USB/MIDI connections depend on the installed MIDI module. A case socket
alone is not a converter. [MIDI 1U manual](https://intellijel.com/downloads/manuals/midi-1u_manual_v1.2_2026.01.06.pdf).

**Norns:** use its existing MIDI/Link clock framework and crow bridge when available.
Crow provides external CV input/output and local shapes; it is USB serial, not a generic
USB-MIDI endpoint. A direct MPC–crow connection would need a new, audited serial/backend
integration; MIDI→norns script→crow composes existing components. Two USB hosts cannot be
connected as if one were a USB-MIDI device. [Norns control/clock](https://monome.org/docs/norns/control-clock/),
[crow FAQ](https://monome.org/docs/crow/faq/), [lib/clock.c](https://github.com/monome/crow/blob/b340579d94e57e2b43336611b2c4037cff74bb80/lib/clock.c), [lib/slopes.c](https://github.com/monome/crow/blob/b340579d94e57e2b43336611b2c4037cff74bb80/lib/slopes.c).

**External MIDI-to-CV:** FH-2 documents MIDI conversion, synced LFOs, clock and VCO
calibration; use its device port when MPC is USB host, or supported DIN breakout. MIDI 1U
and Phazerville offer other established paths. Product functionality is not evidence of
open firmware: no reusable FH-2/Intellijel firmware source was established here.
[Expert Sleepers FH-2](https://www.expert-sleepers.co.uk/fh2.html).

## Electrical and operational safety

- Check each destination's actual voltage/polarity/trigger thresholds. Eurorack has no
  single all-input voltage tolerance; pitch, logic, audio and power connectors differ.
- Use proper TRS-dual-TS CV breakouts and common ground; identify tip/ring separately.
  A stereo Y splitter is not necessarily a dual-mono breakout. Do not join outputs together.
- Never feed Eurorack CV back into One CV **outputs**. Do not feed rack-level signals into
  ordinary audio inputs without appropriate attenuation/protection and documented range.
- Do not assume 5V reliably triggers every device, or that a module expects positive V-trigger
  rather than inverted/S-trigger. Use a suitable proven converter/conditioner where required.
- Validate unloaded voltages with a high-impedance meter and timing with a suitably rated,
  high-impedance scope; assess grounding before connecting grounded instruments. Avoid
  shorts between the TRS ring/tip and sleeve while probing live.
- Pitch endpoints, gain/offset error, monotonicity, loaded voltage, settling and temperature
  drift must be measured. Clamp/range checks do not replace oscillator/DAC calibration.
- Reserve exclusive ports; prepare stop/panic behavior that clears gates. A plugin crash,
  app restart or unplug can leave a held voltage. Do not guarantee a software cleanup handler
  can make hardware safe after a crash; keep a physical disconnect/receiver mute plan.
- Keep experimental CV away from audio monitoring, speakers and uncontrolled high-gain
  feedback paths. No power headers or system wiring are part of this project.

## Recommended staged validation — separate future approval

1. **Offline:** event conversion/range tests, NaN/overflow rejection, command encoding,
   edge ordering, note-off/panic, port arbitration, stale-fd/error simulation, bounded queues,
   no device I/O from callback. Research performs none of these output implementations now.
2. **Native baseline:** user explicitly approves hardware work; save/back up projects/settings,
   record One/Hakai version and existing routes, disconnect modular loads. Use Akai CV tracks
   and meter each physical leg at selected low/medium/high values. Record channel map and
   pitch/gate curves; compare against receiver specifications before connecting.
3. **Host MIDI bridge:** demonstrate virtual port visibility, monitoring, notes/mod-wheel,
   recording/playback, stop/continue, duplicate/feedback prevention and coexistence with existing
   tracks. Capture input-event versus physical-output timing at several tempos/project loads.
4. **Private backend investigation:** only after explicit approval and identified safe method,
   establish the One's controller identity/protocol from passive evidence before any write.
   Do not reuse X's fd/channel mapping blindly or modify launchers as a default procedure.
5. **Timing characterization:** scope clock/pitch/gate with 1/2/5/10ms pulse cases, 24 PPQN,
   fast triggers, swing, loops, locates, tempo ramps, USB stress and simultaneous output changes.
   Report min/median/p95/p99/max delay, jitter, missing/extra edges, pitch/gate skew, settling,
   load and test duration. Compare native, virtual-MIDI and direct routes separately.
6. **Specific modules:** configure Pam PPQN/run; test Phazerville applet/build/calibration;
   verify MIDI 1U mappings and norns/crow host roles. Start with noncritical ports and low levels.
7. **Faults:** bypass, close/reopen, track disable/mute, stop, unload, app crash and power cycle;
   document persistent voltages and physical recovery. Never test crashes with valuable projects.
8. **DC/audio route:** separately validate USB interface enumeration, channel map, DC unity,
   meters/limiters/faders/bypass, latency and volts/full-scale. Do not label it sample-accurate CV
   until end-to-end measurements establish that claim.

## Licensing and go/no-go

Glass Spheres bridge/Marbles references are MIT-scoped; retain actual file notices and inspect
vendored licenses (its top-level third-party paragraph has stale Clouds/Parasites wording).
Phazerville conversion files have MIT headers; crow is GPL-3.0; Link GPL-2.0-or-later or
alternative license. Native Akai CV features are proprietary functionality, not source to copy.
The overall project license and VST2 redistribution review remain unresolved; see
[UNIVERSAL_ARCHITECTURE.md](UNIVERSAL_ARCHITECTURE.md).

**Go now:** research, native-host workflow documentation, reusable event architecture,
existing converter comparison. **Requires approval and evidence:** any One CV electrical test
or private-controller access. **Not justified:** audio-rate built-in CV, an official CV VST API,
universal module electrical compatibility, or CV input through the One's output jacks.

## Follow-up: monome portal and existing Pd control generators

[src/ansible_midi.c](https://github.com/monome/ansible/blob/9796656cbbe9eb0227d380d555e31192f0cf32bf/src/ansible_midi.c) is an additional concrete open-source MIDI-to-CV
implementation: allocation, bend, CC/velocity conversion, clock/arp and panic. Its
[src/config.mk](https://github.com/monome/ansible/blob/9796656cbbe9eb0227d380d555e31192f0cf32bf/src/config.mk) targets AVR32 UC3B0512 and board DAC/GPIO/USB-host
support; it is not an ARMv7 Linux or MPC CV driver. Use the external hardware first,
with USB host/device roles and routing verified. GPL-2.0 root and vendor notices apply.

[src/ops/hardware.c](https://github.com/monome/teletype/blob/e390bacdf024f2234e2ade6a4821773b9a80a322/src/ops/hardware.c) provides external CV/slew/calibration and gate/pulse
operators. Its firmware/simulator does not establish an MPC ii/I2C API. Existing MPC
gate/clock can trigger module-local scripts after voltage/threshold tests; no direct
connection to an internal bus is proposed.

Norns clocks/crow serial source, serialosc/grid abstractions and downloaded CCquencer2
and MIDI Maker source were inspected in [ADDITIONAL_SOURCE_AUDIT.md](ADDITIONAL_SOURCE_AUDIT.md).
They provide existing sequencing/OSC/MIDI/control patterns, not native MPC CV access.
Norns' 1 ms scheduler polling and module DAC comments must not be treated as One timing
measurements. PLAY's control graph is in its custom mother and requires Faust/Jam/Link;
its licensing and embedded external ABI remain unresolved.
