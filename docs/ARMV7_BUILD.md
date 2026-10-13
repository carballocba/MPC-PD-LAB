# ARMv7 VST2 build and controlled testing handoff

This prototype targets the original MPC One running Hakai 3.9.1: ARMv7 Linux,
EABI5 hard-float, stereo 44,100 Hz, 128 frames per DSP call. It preserves the
350 ms wet-only delay. Hardware compatibility remains unverified. Nothing in
these build scripts connects to an MPC, changes its settings, or restarts it.

## Dependencies and build

Use an x86_64 Linux machine with Git, Bash, Docker with BuildKit, tar, gzip,
coreutils, and a working CA bundle. Source acquisition requires GitHub;
toolchain construction requires Docker Hub and the official Debian main mirror.
The compile/test container runs with networking disabled. No ARM binfmt setup
or privileged container is required; `qemu-arm` runs the offline executables.

Pinned inputs:

- libpd: `ba0dc63262901d658af8bbda5e619a60fa975e78`.
- Its Pure Data submodule (0.56.5): `f009fd8d7b537e209e09898d487fdf1bf547da2b`.
- sd88me/mpc-vst-plugins wrapper/generator: `208745830fc2ed90b2fd0dc480ef5c554284292f`.
- Debian Bullseye base and Dockerfile frontend: immutable digests in
  `scripts/Dockerfile.armv7`.
- GCC ARM cross compiler 10.2.1, binutils 2.35.2, ARM sysroot glibc 2.31,
  make 4.3, Python 3.9, QEMU 5.2; all additional Debian packages and versions
  are fixed in `scripts/toolchain-packages.lock`. The base image supplies the
  remaining packages; `toolchain-packages.txt` records the full inventory.

From the repository root:

```sh
scripts/armv7.sh
```

For an intercepting HTTPS proxy, supply its trusted PEM certificate bundle
through `CODEX_PROXY_CERT`. Proxy variables and TLS verification stay enabled;
CA trust is mounted as a BuildKit secret and is never copied into the image.
Normal machines use `/etc/ssl/certs/ca-certificates.crt`. The script can propagate
an existing `proxy` hostname entry into the build container.

Output: `work/armv7/mpc-pd-delay-armv7.tar.gz`. It contains the plugin folder,
its two Pd files under `data/`, licenses, manifest, plugin-list XML fragment,
and SHA-256 checksums. Build/test logs and ELF/dependency reports remain under
`work/armv7/`. Binaries and dependency caches are ignored by Git.

The builder checks pinned dependency revisions (detached in newly acquired caches). The acquisition script refuses
wrong revisions or tracked source edits instead of resetting dependency caches.
For repeat builds, existing pinned checkouts require no source fetch. Debian
packages must remain available at their pinned versions; an unavailable package
is a build failure, never an automatic upgrade. Keep the verified toolchain
image and recorded package inventory if long-term mirror availability matters.

To reuse the published desktop dependency cache for a full desktop build:

```sh
DEPS_DIR=/workspace/.mpc-pd-setup scripts/build.sh desktop
```

Otherwise run `scripts/dependencies.sh` then `scripts/build.sh desktop` with a
native Linux C compiler, ar, and Python 3 installed. This also runs both audio
smoke modes and the VST host test. Objects are built under `work/desktop`; the
original dependency cache, its `audio_smoke` executable, and host patch remain
untouched. The historical smoke commands in `LIBPD_ADAPTER.md` still work (add
`-lpthread` when compiling the adapter on older desktop glibc).

## Implementation and packaging

The pinned wrapper is compiled unchanged with generated `params.h` from
`packaging/vst.json`: effect category, two inputs and outputs, UID `MPDL`
(`0x4d50444c`), vendor `MPC-PD-LAB`, version 1000, no controls or MIDI mapping.
`VSTPluginMain` is the only public defined dynamic symbol. Linux JUCE loads that
entry point; there is no legacy `main` alias or VST3 entry point.

libpd is built in a separate source/object tree with PIC, single-instance mode,
no extra externals or utility queues, and numeric locale changes disabled.
Its archive is linked into the plugin with hidden symbols, `--exclude-libs,ALL`,
`-Bsymbolic`, and `--no-undefined`; the plugin needs no separately installed
libpd library and avoids interposition with other loaded Pd runtimes.

ARM flags are `-march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard`; NEON is not
required. All sources use `-D_FILE_OFFSET_BITS=64` so Pd can load abstractions
from filesystems with large inode numbers. The ARM sysroot is glibc 2.31;
the verifier enforces a maximum symbol version of 2.31 and checks every
`DT_NEEDED` library against an explicit allowlist.

`MODULE_SUBDIR="data"` lets the wrapper find both patches relative to the
loaded `.so`, using `/proc/self/maps`, without an MPC path baked into the DSP.
Keep `data/` adjacent to the plugin. `libpd_host.pd` connects `adc~` and `dac~`
through the unchanged `stereo_delay_smoke` abstraction.

Only one active instance per loaded plugin library is supported. A second load
returns NULL. Lifecycle initialization/destruction is serialized with a mutex;
the adapter's audio callback does not take that mutex, allocate, copy buffers,
or log. The wrapper retains its existing per-instance engine-call mutex and
float/int16 conversion. Hosts must stop audio before destruction. Renamed copies
of the library and concurrent host destruction/audio calls are unsupported.

44.1 kHz is a fixed contract: the reference wrapper acknowledges sample-rate
messages without reconfiguring the engine. Other sample rates are unsupported.
128-frame host blocks have no added wrapper buffering delay. Other host block
sizes use the wrapper's FIFO and can add 128 frames. Audio is quantized/clipped
to int16 by the wrapper. Missing inputs produce silence. Delay tails are not
saved in projects, and no configurable parameters, custom touchscreen skin,
or state chunk implementation is included in this phase.

## Installation prerequisites and rollback (future hardware phase only)

Before separate hardware testing, verify on the intended firmware that MPC is
32-bit armhf, its glibc meets the reported symbol requirements, all listed
system libraries exist, and `/proc/self/maps` is available. Confirm how Hakai
3.9.1 exposes third-party VST2 effects, its supported plugin paths, permissions,
and settings location. The generated XML uses the reference wrapper's
`/sdcard/Synths/MPC-PD-LAB - VST - MPC PD Delay/mpc_pd_delay.so`; change the
settings entry to the actual chosen path if necessary. This is an XML fragment,
not a replacement settings file and not an installer.

Back up the complete original settings file and any existing plugin folder,
record ownership/permissions/checksums, and keep a known-good recovery method.
Only in an authorized hardware session, with MPC safely stopped, copy the entire
plugin folder and add its single effect entry to the existing plugin list using
the device's established workflow. This package has no skin: native touchscreen
presentation and effect-browser discovery need separate confirmation.

Rollback in that later session: stop MPC safely, restore the original settings
and prior plugin folder with their original permissions, or remove only this
new plugin folder if none existed. Restore any test-project backup and restart
using the device's confirmed service workflow. Do not delete unrelated plugins
or replace an entire settings file with the generated fragment.

Hardware acceptance must independently check discovery, patch resource loading,
stereo routing/timing, insertion/removal/reload, project reload, single-instance
failure behavior, tails, xruns, CPU/RAM and long-running stability. A passing
QEMU test is not evidence of real MPC performance or host compatibility.
