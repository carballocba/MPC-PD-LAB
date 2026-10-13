# Phase 3: MPC PD Delay for Hakai — experimental

Prepared for the original MPC One, ARMv7 hard-float, Hakai 3.9.1.
**Real hardware compatibility remains unverified. Do not use a valuable project
for the first test.** No hardware was connected to or modified in Phase 3.

## Verified upstream behavior and evidence

Packaging/installer reference is sd88me/mpc-vst-plugins at
[`5e3a96885b6de76a48b498821c379568a05cc006`](https://github.com/sd88me/mpc-vst-plugins/tree/5e3a96885b6de76a48b498821c379568a05cc006).
Build/wrapper remains at Phase 2's separate pin `208745830fc2ed90b2fd0dc480ef5c554284292f`.
The [catalog](https://sd88me.github.io/mpc-vst-plugins/) and
[upstream README](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/README.md)
identify the supported desktop installer and release scripts.

| Finding | Source at the pinned packaging revision |
| --- | --- |
| MPC's JUCE VST2 host reads `pluginList-arm` / `KNOWNPLUGINS` from `MPC.settings` at startup. Merely copying a library is insufficient for this installation route. | [README.md, How it works](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/README.md); [tools/release/plugin_list.awk](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release/plugin_list.awk) |
| Supported release installer backs up and edits the existing settings, validates the result, and restarts MPC; desktop app invokes this same script. It selects `acvs`, falling back to `inmusic-mpc` for Hakai. | [tools/release/install.sh](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release/install.sh); [tools/desktop/job.go](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/desktop/job.go) |
| One ZIP top folder with `mpc-plugin.json`, `install.sh`, `uninstall.sh`, `plugin_list.awk`, `SHA256SUMS`, `INSTALL.md`, and `portable/<skin>/`. | [tools/release.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release.py); [tools/desktop/pkg.go, OpenPackage](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/desktop/pkg.go) |
| Manifest is required by the desktop app; release.py generates it at top level and copies it into the installed folder. `plugin-meta.xml` is required for portable registration and generated from the plugin entry. `version.xml` and `Plugin Skins/TUI.json` must exist before release.py; the catalog checker also requires them. They are not all required merely to dlopen a VST, but are part of the supported release contract. Q-Link files are optional for this zero-parameter effect. | [tools/release.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release.py); [tools/catalog_check.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/catalog_check.py); [tools/shadow_skin.py, write_skin](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/shadow_skin.py) |
| Effects use manifest `kind: effect`, XML `category="Effect"`, `isInstrument="0"`, stereo inputs/outputs, plus matching compiled VST flags/category. | [tools/gen_vst.py, entry and gen_params](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/gen_vst.py); Phase 2 `tests/vst_host.c` |
| Four-character ID is encoded big-endian in VST and hex in XML. Keep it stable. Upstream registration replaces matching UID or file entries; it is not a global uniqueness registry. | [tools/gen_vst.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/gen_vst.py); [tools/release/plugin_list.awk](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release/plugin_list.awk) |
| Shell installer accepts any absolute `-t` destination, including `/storage/Synths`; portable metadata uses `%payload-path%`. Desktop defaults scan `/sdcard/Synths /media/*/Synths`, not `/storage/Synths`. Do not assume an alias or silently choose another drive. | [tools/release/install.sh](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release/install.sh); [tools/desktop/device.go, DefaultConfig](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/desktop/device.go) |
| `max_glibc` is derived from ELF version-needs. Catalog enforces a maximum of 2.36 for ARMv7; desktop UI warnings can be overridden, and shell install does not enforce glibc metadata. ELF loading still requires actual compatible libraries. | [tools/catalog_check.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/catalog_check.py); [tools/desktop/README.md, MPC OS warnings](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/desktop/README.md) |

Hakai-source limitation: the historically referenced firmware repository
[RedHate/Hakai-MPC](https://github.com/RedHate/Hakai-MPC) returned HTTP 404 on
2026-10-13 and could not be cloned. No accessible 3.9.1 firmware/plugin-manager
source has been established here. Consequently, the table verifies the sd88me
installation route, not a claimed independent audit of Hakai's discovery code.
If a different Hakai manager is intended, obtain its exact source/version before
using that route. Do not invent a native manager integration.

User-supplied hardware observations: `armv7l`, `/lib/ld-linux-armhf.so.3`, glibc
2.39, available libm/libdl/libpthread, existing sd88me plugins in `/storage/Synths`.
These were not independently rechecked in Phase 3. Binary requires glibc at most
2.29, with libc/libm/libdl/libpthread and embedded libpd; no external libpd.so.

## Package and identity

```
MPC-PD-Delay-1.0.0/
  install.sh, uninstall.sh, plugin_list.awk, SHA256SUMS
  mpc-plugin.json, INSTALL.md, PROVENANCE.json
  portable/CARBALLO - VST - MPC PD Delay/
    mpc_pd_delay.so
    mpc-plugin.json, plugin-meta.xml, version.xml
    Plugin Skins/TUI.json
    data/libpd_host.pd
    data/stereo_delay_smoke.pd
    licenses/libpd.txt, pure-data.txt, wrapper.txt
```

UID stays `MPDL` / `0x4d50444c`; filename, name, version 1000, parameters and
binary remain unchanged. Registry/folder manufacturer is `CARBALLO`; compiled
vendor still reports `MPC-PD-LAB`. This branding difference is deliberate to
reuse the validated binary; its effect on real host presentation is unverified.
The minimal empty TUI container satisfies the release format; it offers no
custom GUI or Q-Link mapping. Its on-device behavior is untested.

The installer is generated upstream with one reviewed addition:
`packaging/fresh_install_guard.sh` refuses existing target/staging/backup folders,
legacy binary, or MPDL UID/binary registrations before any package mutation.
This is a fresh-install-only release; it cannot upgrade an existing installation.
The upstream checker reports the modified install script for review.
UID collision checks cannot prove global uniqueness or protect against concurrent
external changes. Do not run another installer at the same time.

The upstream static capability scan flags network/process/dlopen/setuid symbols
compiled into libpd/Pure Data, including unused Pd objects. This is a static
review flag, not evidence that the fixed delay patch invokes them. The original
Phase 2 binary is unchanged; no capability removal or DSP changes were made.

Third-party licenses travel with the binary: libpd, Pure Data and wrapper.
The repository has no declared project license; none is fabricated by packaging.
This is a private/test installation deliverable, not a catalog submission or
assertion of redistribution permission. Catalog `--catalog` requires a declared
project license before submission. Hardware-tested badges are not claimed.

## Prepare on macOS — before connecting for a later hardware test

1. Save projects, then make independent backups of your MPC projects and samples
   to your Mac or another drive. Verify that representative backups open.
2. Retain the original ZIP, its SHA-256, and the Phase 2 validation record. Verify
   the ZIP on macOS with `shasum -a 256 MPC-PD-Delay-1.0.0-mpc-armv7.zip` against
   the supplied validation report. Unzip to a new local directory; do not merge
   with an older extracted package.
3. Obtain sd88me's [supported installer](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/desktop/README.md)
   from its official release if using the desktop route. Right-click/Open is the
   upstream macOS launch procedure for its unsigned app. Trust only reviewed
   packages: install scripts execute as root.

Everything below is a procedure for a future user-controlled test, not something
executed during preparation. Use your own device IP; verify its SSH fingerprint.

## Back up settings and select the destination

Before installing, locate the actual `MPC.settings` on your unit. Upstream checks
`/media/az01-internal/Settings/*/MPC.settings` and
`/data/Settings/*/MPC.settings`. If neither matches, stop and identify the actual
file; the shell installer accepts `MPC_SETTINGS` explicitly. With MPC stopped,
copy the **complete** settings file to your Mac with `scp`, retain a dated copy
on the device, and back up the destination's `.mpc-store` if present. Record
`SynthContentLocations`, the existing plugin-list entries, and service name.
Use `systemctl cat acvs` / `systemctl cat inmusic-mpc` to identify the service;
stop/start that existing service only. Do not change system configuration.
Restart MPC after taking the backup if postponing installation.

Use `/storage/Synths` for the verified unit unless you deliberately select
another writable, executable storage location. Check its mount and free space;
a `noexec` mount cannot load the plugin. Confirm the selected root is in
`SynthContentLocations` using the actual settings. If absent, stop and resolve
through the unit's supported content-location UI or documented manager; this
package does not modify content locations or system mount files.

Check there is no `CARBALLO - VST - MPC PD Delay` folder and no XML plugin with
UID `4d50444c` (case-insensitive) or `mpc_pd_delay.so` in settings. Inspect other
installed manifests/meta files for the same UID. A collision is a blocker;
do not delete or rename someone else's plugin to bypass it. Do not change MPDL
on an established plugin: saved projects depend on it.

## Install with the supported sd88me installer

For the verified `/storage/Synths` location, use the release's sd88me **shell
installer**, driven from macOS Terminal. The desktop app's default scan cannot
select this path unless it is also exposed through one of its supported roots.
No alias is assumed. Save the project and accept the installer restart prompt:

```sh
scp -r MPC-PD-Delay-1.0.0 root@MPC_IP:/tmp/
ssh -t root@MPC_IP 'cd /tmp/MPC-PD-Delay-1.0.0 && sh install.sh -t /storage/Synths'
```

Replace `MPC_IP` with the device IP. If settings require an explicit path, prefix
`sh install.sh` with `MPC_SETTINGS='/actual/path/MPC.settings'`. Do not guess it.
The installer verifies `SHA256SUMS`, refuses collisions, stops MPC, copies only
this plugin, backs up the complete settings, edits only matching plugin-list
records and restarts MPC. Record its printed backup path and log. Never replace
`MPC.settings` with `plugin-meta.xml` or any XML fragment.

Alternatively, if your deliberately selected root is actually shown by the
sd88me desktop app: connect, drop the ZIP into Choose, select the correct
storage location, select only MPC PD Delay, and Install. Verify the destination
and restart count in its confirmation. Do not choose `/sdcard/Synths` merely
because it is the default. If the correct root is missing, use the supported
`-t` shell route above; do not patch the desktop app or create storage symlinks.

## Controlled first test

1. Start an empty project at **44.1 kHz**, with monitoring at a low level and no
   valuable unsaved work. Look for **MPC PD Delay under audio insert effects**,
   not instruments. If missing or shown as Load Plugin, stop and review logs,
   full settings backup/current entry, mount flags, and runtime files. Do not
   repeatedly load it or edit unrelated plugins.
2. Insert one instance on a stereo audio track. Send a quiet left-only impulse
   or short click, then a right-only one. Confirm each delayed output appears
   on its own channel with no unintended crossfeed; compare bypass. Phase 2's
   offline impulse peaked at frame 15435 (350 ms), with additional wrapper FIFO
   latency possible for non-128-frame host buffers. Observe the actual host.
3. Try quiet stereo material. Watch for clipping, silence, channel reversal,
   instability or dropouts. Stop if any appear. No delay/time/feedback controls
   or Q-Link mappings are expected. Do not try multiple instances: the current
   module accepts only one active instance.
4. Save only this disposable test project; close/reopen it and verify the effect
   still loads. Record firmware, buffer/rate if exposed, destination, ZIP hash,
   levels, CPU/dropouts and pass/fail. Only real hardware results can establish
   host compatibility or performance.

## Uninstall and recover

Remove the effect from the disposable project and save/close it first. Retain
the package and original settings backups. Use the same destination as install:

```sh
ssh -t root@MPC_IP 'cd /tmp/MPC-PD-Delay-1.0.0 && sh uninstall.sh -t /storage/Synths'
```

Use the same explicit `MPC_SETTINGS` override if needed. The upstream uninstaller
backs up settings, removes this UID/file registration and this plugin folder,
then restarts MPC. This package declares no user-data paths; do not store your
own files inside its folder. For the desktop route, drop this ZIP again if
necessary to identify it, select only this plugin under Remove on the same root,
and remove it. Shell-installed plugins do not get a desktop `.mpc-store` record.

If MPC cannot start or crashes on reload, use SSH from the Mac, stop the previously
identified MPC service, and run the uninstaller with MPC stopped (its `-n` flag
leaves restart to you), then start that service. If a settings edit itself is
broken, keep the failed file for diagnosis and restore the **complete original
backup** with MPC stopped, after confirming there have been no intervening
settings changes you need to preserve. Never restore an XML fragment. A full
backup restore reverts subsequent settings too. If rollback also fails, stop
and seek firmware-specific support; do not remount or modify system files.

## Reproduce and validate offline

Install `scripts/requirements-packaging.txt` in a Python virtual environment.
Clone sd88me and detach at the pin above. Extract the Phase 2 ARM tar into a
scratch folder (or run the existing `scripts/armv7.sh` on a Linux Docker host).
Then run:

```sh
python3 scripts/package_hakai.py --phase2-folder '/path/MPC-PD-LAB - VST - MPC PD Delay' --upstream /path/mpc-vst-plugins --out work/dist
python3 tests/test_hakai_package.py work/dist/MPC-PD-Delay-1.0.0-mpc-armv7.zip --upstream /path/mpc-vst-plugins
```

The build is intentionally pinned to the validated Phase 2 binary SHA-256
`76a7340039def8494f9d78acb4ad23ac75f8b02b889557f47eb84dc26930e0df`.
It copies existing licenses and runtime patches; it does not rebuild DSP.
ZIP timestamps/order/modes are normalized. The validator checks real ELF
attributes/dependencies/exports, glibc needs, metadata, binary hash, runtime
assets, checksums and upstream catalog rules. The installer tests use only local
temporary settings/storage, never SSH. Existing readelf validation is reused
when available. CI also runs the existing desktop and ARM/QEMU audio tests.

## Remaining limits

- Real hardware compatibility, discovery, UI and stability remain unverified.
- Delay is fixed at **350 ms**; there is no custom GUI or Q-Link mapping yet.
- Only 44.1 kHz and one active instance are supported; int16 conversion can clip
  or quantize, and wrapper buffering can add latency.
- QEMU timing does not guarantee real-time performance on the MPC.
- A plugin runs inside MPC; a crash may affect the entire MPC application.
- The unavailable Hakai source, empty TUI, branding difference, and untested
  on-device resource lookup are explicit evidence gaps, not compatibility claims.
