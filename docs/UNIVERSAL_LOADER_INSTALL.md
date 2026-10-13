# Universal Loader v0.1 — installation and rollback

**Experimental; real MPC hardware has not been tested. Do not install until the
reviewed artifact and hardware-testing approval are available.** This document
is a procedure for the operator; development and CI do not connect to an MPC or
execute installers.

## Before the first hardware session

1. Save and back up projects, user samples, all plugin folders, and the existing
   complete MPC.settings using the established sd88me backup instructions. Keep
   a dated copy off the MPC. Record firmware, storage mount and existing plugins.
   Never substitute an XML fragment for the full settings file.
2. Read the package SHA256SUMS and PROVENANCE.json; on macOS use `shasum -a 256`
   to compare the downloaded ZIP against the review's published hash. Inspect
   `INSTALL.md` and `install.sh` before execution. Use an empty project, low
   monitoring volume, and no important unsaved work.
3. This is a **fresh installation only**. The package refuses an existing MPUL
   unique ID, Loader filename/folder, or staging/rollback collision. Do not remove
   a collision to bypass the guard; investigate and back it up first. Delay and
   Granular Freezer have separate identities and must remain installed unchanged.

## Supported installer and destination

Use the pinned sd88me portable installer included in the ZIP, following
[upstream installation documentation](https://sd88me.github.io/mpc-vst-plugins/install.html).
On macOS, extract the entire ZIP, open Terminal in the extracted release folder,
and consult `bash install.sh --help` only during the approved installation session.
Provide the existing SSH connection details required by that installer. Select
`/storage/Synths` explicitly with its supported `-t /storage/Synths` target option
for the verified original MPC One. The installer also supports other mounted
Synths destinations; do not assume `/sdcard/Synths` is the correct storage.
Do not use any separate hand-written registration command or edit MPC.settings.
The community installer performs its supported entry-level registration and
backup. Confirm its backup succeeded before allowing replacement or restarting.

The new folder is `CARBALLO - VST - MPC PD Universal Loader`; its binary is
`mpc_pd_universal_loader.so`, VST2 ID `MPUL`. The installer carries the two trusted
patch folders and licenses. No existing Delay or Freezer folder is included.

## First-use validation

Use the checklist in UNIVERSAL_LOADER_VALIDATION.md. Find Loader under **audio
effects**, create one instance at **44.1 kHz / 128 frames**, then wait for READY.
Four Program-mode Q-Links are Patch, Apply, Gain and Mix. Patch selects A or B;
Apply must return low before its next rising edge. Apply requests while playing
wait for a positively stopped transport. Unknown transport also waits. Changes
fade out, load on the worker, then fade in; a short intentional silence occurs.
Gain/Mix are normalized 0–1. Trim's Mix is audibly neutral; Lowpass Mix blends
between dry and fixed 1200 Hz filtering. There is no custom touchscreen GUI.

Patch folders live in `data/user/patches`; the catalog declares exactly two IDs.
Only the audited flat Vanilla subset is supported. Close/unload the plugin before
editing folders; validate them with `scripts/audit_universal.py`. Trusted means
reviewed by the operator: this subset is not a security sandbox. Never load
untrusted patches or externals. Keep IDs/revisions stable for project recall;
changed content requires a new revision. Unknown IDs/revisions fail safely rather
than silently substituting another patch. No automatic migration or saved DSP
buffer restoration is provided. A singleton permits one Loader instance per
loaded module; multi-instance project usage is unsupported in v0.1.

## Failure and rollback

Stop transport and lower monitoring volume. A rejected preflight retains the
current graph; a failed activation attempts the retained immutable snapshot.
If rollback itself fails, output is initialized silence and an error is shown.
Inspect `data/user/loader.log` (bounded rotation) after stopping. Do not repeatedly
retry a failing patch in a live performance. Plugin crashes can terminate the MPC
application: restart it and reopen a known-good project without Loader if needed.

Use the supported sd88me uninstall operation documented by the bundled installer
and upstream instructions during the approved session. Verify its target is only
Loader/MPUL. Back up Loader user data first. Do not recursively delete Synths or
replace settings wholesale. If registration restoration is required, use the
installer's own dated complete backup and documented recovery procedure after
verifying its date and unrelated entries. Restore projects from the off-device
backup only if necessary. Confirm Delay and Freezer still appear and process
audio, then archive the failed Loader artifact, log, and firmware version.

MPC system files, firmware and existing plugin binaries are outside this package's
scope. Hardware validation needs explicit approval; no CV/Gate, MIDI, externals,
patch browser, arbitrary Pd compatibility or real-time performance is claimed.
