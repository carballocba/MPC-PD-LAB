# Organelle Granular Freezer: separate experimental Hakai package

**This plugin has not been tested on MPC hardware.** The existing MPC PD Delay
is a separate regression baseline. The user reports that Delay loads, appears
under effects, processes audio with approximately 350 ms delay, restores immediate
playback when disabled and displays a black page on an original MPC One/Hakai
3.9.1. That report does not verify Granular Freezer or real-time stability.

No development script connects to the MPC, installs files or restarts services.
Installation below is a separate, explicit user-controlled hardware-test step.
Do not execute it until you choose to perform that test.

## Identity and destination

- Name: **Organelle Granular Freezer**; VST2 ID `CGFZ` / `4347465a`.
- Library: `organelle_granular_freezer.so`.
- Folder: `CARBALLO - VST - Organelle Granular Freezer`.
- Delay's MPDL / `4d50444c`, library and folder stay unchanged.
- ZIP: `Organelle-Granular-Freezer-1.0.0-mpc-armv7.zip`.

The package uses the same pinned sd88me release installer/registration route as
Phase 3, with a fresh-install guard. It refuses existing target/staging/backup
folders, legacy copies and colliding UID/binary registrations before changing
settings or storage. It is not an in-place upgrade installer. Do not run other
installers concurrently. The upstream uninstaller is included.

Use the unit's verified `/storage/Synths` destination deliberately. The release
shell installer supports an explicit absolute `-t` root; the desktop installer's
default scan only includes `/sdcard/Synths` and `/media/*/Synths`. Do not assume
these are aliases for `/storage/Synths`, or choose the default blindly. An
executable, writable mount, enough space, and a supported SynthContentLocation
are prerequisites. Do not change mount/system files to bypass a failure.

## Back up first on macOS

1. Save and independently back up all MPC projects/samples you care about. Test
   the backup. Use an empty disposable project for all first-load tests.
2. Verify the ZIP with `shasum -a 256 Organelle-Granular-Freezer-1.0.0-mpc-armv7.zip`
   against the supplied validation report. Extract into a new local directory.
   Keep the original ZIP and its notices/editable Pd sources.
3. Before installing, locate the actual persistent `MPC.settings` and identify
   the existing MPC service (`acvs` or `inmusic-mpc`). The upstream installer
   searches `/media/az01-internal/Settings/*/MPC.settings` and
   `/data/Settings/*/MPC.settings`; otherwise use its `MPC_SETTINGS` override.
   With MPC stopped, retain a complete dated settings backup both on the device
   and on the Mac via scp. Back up the target `.mpc-store` if present, and retain
   a copy of the current delay package/settings. Verify the saved files.
4. Confirm `/storage/Synths` is the intended executable storage root and already
   a content location. Inspect settings and installed metadata for UID
   `4347465a`, filename and target folder collisions. Stop on a collision; do not
   delete another plugin or change this plugin's established ID to work around it.
5. If settings are bind-mounted from a temporary copy or overwritten by a
   launcher, the upstream atomic edit/persistence may not work. Identify the
   persistent settings through firmware-specific documentation before proceeding;
   this package does not alter the launcher or supply a persistence workaround.

For the full upstream evidence and backup details, also see
[Phase 3 install guide](https://github.com/carballocba/MPC-PD-LAB/blob/main/docs/PHASE3_HAKAI_INSTALL.md).
That guide's old hardware-unverified statement is a historical record; the user
report above is the current Delay baseline evidence.

## Supported installation route, when explicitly approved for hardware testing

From macOS Terminal, after saving/closing the current project, use the package's
sd88me installer. Replace `MPC_IP` with your device's IP and verify its SSH host
fingerprint:

```sh
scp -r Organelle-Granular-Freezer-1.0.0 root@MPC_IP:/tmp/
ssh -t root@MPC_IP 'cd /tmp/Organelle-Granular-Freezer-1.0.0 && sh install.sh -t /storage/Synths'
```

If needed, prefix `sh install.sh` with `MPC_SETTINGS='/actual/persistent/path/MPC.settings'`;
never guess that path. The installer checks SHA256SUMS, prompts, stops MPC, installs
only this folder, backs up the complete settings, registers the effect and
restarts the existing service. Save its log and printed backup path. Never
replace the complete `MPC.settings` with a plugin XML fragment.

The desktop sd88me app can also accept this ZIP. Use it only if the deliberately
selected correct root is actually listed, then select only this plugin and
review destination/restart count. If `/storage/Synths` is absent, use the supported
shell `-t` route above. Do not create aliases or modify the desktop app to hide
the mismatch.

## First hardware tests to record

1. Empty project, **44.1 kHz**, low monitoring levels, no valuable unsaved work.
   Confirm the plugin appears as an **audio insert effect**, loads and processes
   without freezing. It has an empty skin scaffold, so a black page remains
   expected until a native page is implemented/tested.
2. Determine whether the MPC exposes its six VST parameters anywhere in the
   host UI. Do not assume Q-Link support. If parameter access is unavailable,
   test MIDI only if the MPC can route MIDI to the insert; that routing also
   needs verification. Do not claim a usable hardware-control path from offline
   parameter tests alone.
3. Dry mix: test left-only then right-only quiet clicks and confirm separation.
   Wet mix: test each input; original DSP sums inputs to the freezer and emits
   stereo randomized grains. Wet cross-channel energy is intentional. Check
   no unexpected silence, clipping or channel reversal. Allow up to five seconds
   to fill the buffer.
4. Test Grain Size, Spread, Density and Wet/Dry. Freeze Off/On maps to original
   aux-toggle edges. At a useful spread and full wet, capture input, freeze and
   stop input; the retained grains should continue. Verify release resumes live
   input. Observe actual automation/feedback rather than assuming support.
5. Save/reopen this disposable project to check scalar parameter recall. Frozen
   audio, tails, latched notes and RNG state are not saved. Test close/reload,
   then separately test coexistence with Delay without loading a second Freezer
   instance. A second Freezer instance is intentionally rejected.
6. Record hardware/firmware, ZIP hash, settings root, buffer size if exposed,
   audio/MIDI/parameter paths, CPU/dropouts, stability and pass/fail. Stop at
   crashes or instability. QEMU results do not establish real-time performance.

## Remove or roll back

Remove the insert from the disposable project, save/close it and retain backups.
Use the same explicit root and settings override as installation:

```sh
ssh -t root@MPC_IP 'cd /tmp/Organelle-Granular-Freezer-1.0.0 && sh uninstall.sh -t /storage/Synths'
```

The uninstaller backs up settings, removes only this ID/file entry and folder,
and restarts MPC. No user-data paths are declared: do not put personal files
inside the plugin folder. Delay and unrelated plugins must remain present.
If using the desktop route, drop this same ZIP again to identify the plugin and
remove only it on the same root. Shell installs do not create `.mpc-store` records.

If MPC cannot load normally, stop its identified service through SSH and run the
uninstaller with `-n` so you control the restart. Preserve failed settings for
diagnosis. Restore a **complete known-good settings backup** with MPC stopped
only if an edit is broken and you understand which subsequent changes it will
revert. Never use an XML fragment. If safe rollback fails, stop and seek
firmware-specific support rather than modifying system files.

## Limits and licenses

The original C&G patch files are unchanged. BSD and GPL imported-code notices,
complete Pd sources, libpd/Pure Data notices and sd88me MIT notice are included.
Project-wide licensing is still undeclared; this is not a catalog submission or
hardware verification badge.

This first port supports 44.1 kHz, one active instance per module and six scalar
parameters. It retains int16 conversion/clipping and the wrapper's mutex/FIFO.
The tested named-bus graph adds 64 dry-path samples; latency compensation is not
reported to the host. No custom GUI, verified Q-Link mapping, OLED, SysEx/MIDI
output, full preset system or frozen-audio persistence is supplied. A crash in
this in-process plugin may affect the MPC application. No compatibility or
real-time-performance claim is made without a controlled hardware test.
