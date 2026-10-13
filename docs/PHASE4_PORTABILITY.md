# Reusable Organelle bridge and Phase 5

## Architecture

The first proof uses libpd, the existing VST wrapper and original C&G Granular
Freezer. It is not a new granular effect or a patch-to-C conversion.

`ports/granular-freezer/manifest.json` is a proposed version-1 port manifest.
`prepare_organelle.py` validates this audited port and generates the wrapper's
parameter list, VST definition and the C control-map header. The host loads
`host.pd`, which instantiates the compatibility mother and unchanged upstream
patch independently. No original Pd file is rewritten by the build.

```
VST parameters / MIDI -> manifest-generated C mapping
                      -> mpc-* message receivers in mother
                      -> Organelle knob/aux/fs/exp/notes symbols
adc~ -> inL/inR        -> authentic patch DSP -> outL/outR -> volume/clip -> dac~
```

The manifest records original name/author, source repository and exact revision,
license, entry point, audio/MIDI requirements, external/abstraction inventory,
assets, control receivers/defaults/modes, distinct VST identity, compatibility
tier and limitations. Source hashes are recorded separately. The generator is
intentionally limited to one audited port, fixed 44.1 kHz stereo and six controls;
it is not yet an arbitrary manifest runtime or automatic converter.

`compat/organelle/mpc_organelle_mother.pd` is the reusable Pd boundary. Per-port
JSON controls can target its receivers. `src/organelle_engine.c` implements the
host lifecycle and normalized value/edge translation using generated mappings.
No hardware controller daemon, OSC port, filesystem remount or system service is
required. Future patches must be inspected; global symbols and control semantics
are not guaranteed to match.

## Controls and UI evidence

| VST index | Key | Mapping |
| --- | --- | --- |
| 0 | knob1 | Grain Size, 0–1 |
| 1 | knob2 | Spread, 0–1 |
| 2 | knob3 | Density, 0–1 |
| 3 | knob4 | Wet/Dry, 0–1 |
| 4 | freeze | Absolute Off/On translated to original aux toggle pulses |
| 5 | vol | Mother output volume, squared then smoothed |

Indices/UID are stable for saved projects. Do not reorder them in a later
release. VST get/setParameter, parameter names/readback and chunk save/restore
are tested with the real wrapper. The pinned wrapper supports enumeration and
normalization, flags effect/audio input correctly, and serializes engine access
with its existing per-instance mutex. Its parameter-0 deferral is retained.

Relevant wrapper source:
[gen_vst.py](https://github.com/sd88me/mpc-vst-plugins/blob/208745830fc2ed90b2fd0dc480ef5c554284292f/tools/gen_vst.py),
[vst2_wrap.c](https://github.com/sd88me/mpc-vst-plugins/blob/208745830fc2ed90b2fd0dc480ef5c554284292f/wrapper/vst2_wrap.c)
and [shadow_skin.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/shadow_skin.py).
A native MPC page and Q-Link bindings are generated separately from the VST
parameter table. Phase 3's TUI has `tabs: []` and no Q-Link maps: the reported
black page is consistent with that empty content. It is not evidence that libpd
or Hakai cannot display controls. Hardware access was not used to determine
which parameters Hakai offers elsewhere in its UI.

This Phase 4 package retains the empty scaffold. It has **no claimed working
custom GUI or Q-Link mapping**. The six parameters are useful through a generic
VST host and MIDI CCs; MPC parameter access/automation still needs explicit
hardware verification. Phase 5 should generate a minimal native page from the
same manifest rather than hardcode controls in the DSP.

MIDI input supports note on/off (all channels collapsed into the Organelle
pitch/velocity convention), CC21–24 knobs, CC25 absolute freeze, CC7 volume,
CC11 expression and CC64 footswitch edges. libpd also receives native note/CC
messages for patches using notein/ctlin. Granular Freezer ignores expression.
Other message types, MIDI output and host automation notifications for incoming
MIDI are not implemented. Raw/direct parameter readback reflects mapped CCs.

## Lifecycle, paths and state

A second instance of this module is rejected before touching libpd. The separate
delay module remains unchanged; simultaneous coexistence of the two different
plugins still requires a hardware test. Original patch globals are not renamed.

Create checks every required runtime file, initializes Pd/audio, binds status
receivers, clears/adds the private search path and loads the host. Native
loadbangs finish before host defaults are applied and `mpc-ready` is sent.
Object-creation/errors are captured into a bounded diagnostic buffer. A partial
canvas with missing externals is closed and rejected. Runtime Pd errors silence
processing and can be read through the engine diagnostic key. Hooks perform no
file I/O or logging from audio callbacks. Missing runtime files are reported at
initialization, outside processing.

On Linux, the inherited wrapper finds the directory containing its own mapped
.so and uses its adjacent `data/`; its fallback is deliberately nonexistent.
The test-only native macOS build uses a generated absolute fallback because the
Linux /proc/self/maps resolver is not available there. No absolute build path is
compiled into the ARM module. Original abstraction-local paths resolve relative
to `data/upstream`. No samples are required by this patch.

State chunks contain a version and six scalar values. Restore validates the
whole state before applying it, and synchronizes freeze through the same edges.
Delay buffers, frozen audio, grain RNG state, latched notes and tails are **not**
persisted. Reopen starts from upstream loadbangs and host defaults. There are no
factory preset files or disk state writes. This is a parameter-state proof, not
full Organelle session emulation.

The adapter does no file loading, explicit heap allocation or lifecycle locking
in its process callback. Pd's original message/list/voice operations can still
allocate internally, and the inherited wrapper takes a mutex; this work does not
prove hard real-time safety or remove those original behaviors. QEMU timing is
not a hardware performance guarantee.

The mother/patch named-bus graph adds one 64-sample tick (about 1.45 ms at
44.1 kHz) in the tested dry path. Non-128 host buffers can add inherited wrapper
FIFO latency. The wrapper does not report this new bus latency for automatic
host compensation. Wet grain timing remains the original stochastic algorithm.

## Compatibility tiers

| Tier | Definition and review needed |
| --- | --- |
| A | Vanilla Pd, minimal Organelle dependencies; plain entry/audio adaptation |
| B | Needs reusable Organelle controls/audio/lifecycle adapter; no externals. Granular Freezer is the first audited example. |
| C | Needs supported externals, samples/assets, or additional runtime adaptation; inspect source licenses, rebuild externals for the pinned ABI and test failure paths |
| D | Significant platform dependencies or unsupported features, e.g. shell/system access, hardware services, unsupported multi-page UI or unavailable external source |

Tier is an engineering classification, not a compatibility certification.
Tier B on desktop/QEMU does not imply that Hakai accepts the skin or that the
MPC meets the audio deadline. Every patch needs its own source/license, namespace,
resource, CPU/memory and behavior audit.

## Reproduce offline

Run existing `scripts/armv7.sh` and `scripts/build.sh desktop` first on the existing
Linux/Docker toolchain. Then run:

```sh
python3 tests/test_organelle_audit.py
python3 scripts/build_organelle.py desktop
docker run --rm --network=none -u "$(id -u):$(id -g)" -v "$PWD:/project" mpc-pd-armv7:phase2 python3 scripts/build_organelle.py armv7
python3 -m pip install -r scripts/requirements-packaging.txt
python3 scripts/package_organelle.py --upstream work/packaging-upstream
```

The packaging checkout must be detached at
`5e3a96885b6de76a48b498821c379568a05cc006`, as in Phase 3. Build dependencies keep
the existing pins. ARM tests use the unchanged hard-float sysroot and QEMU;
readelf validation is reused. `build_organelle.py native` additionally supports
local macOS engine/VST tests after `scripts/dependencies.sh`, with a separate
native libpd object/archive directory. That native module is not an MPC package.

CI retains all Phase 1–3 tests, builds both new targets, exercises routing and
original DSP, rejects missing files/classes, validates the release and actual
upstream ZIP reader, tests install/uninstall against temporary settings containing
the delay, and compares two ZIPs byte for byte. No test connects to hardware.

## Phase 5 roadmap

1. User-controlled Granular Freezer hardware installation and evidence log:
   effects registration, stereo input, wet/freeze, state recall, CPU/dropouts,
   coexistence with the delay and clean removal. Do not use valuable projects.
2. Generate one minimal native MPC parameter page and Q-Link maps from the
   manifest. Test real enumeration, automation and control/state feedback before
   claiming support. Preserve existing parameter indices and UID.
3. Audit a second authentic Tier A/B effect to discover which parts of the
   manifest/generator are actually reusable. Add per-port build selection only
   when needed by that concrete patch.
4. Add vetted Tier C external source builds and sample-path policies, including
   licenses, ABI checks, missing resources, memory limits and off-audio-thread
   asset loading. Do not ship foreign .pd_linux binaries blindly.
5. Design state/namespace/multiple-instance isolation and long-lived stability
   tests based on observed limitations. Keep unsupported Tier D patches explicit.

Do not start a bulk converter or invent replacement instruments as a shortcut.
