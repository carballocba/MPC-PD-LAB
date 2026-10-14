# v0.2 GUI — installation and hardware acceptance (NOT YET RUN)

**Do not install or contact hardware until explicitly approved.** Target original
MPC One / Hakai 3.9 or 3.91; record the exact firmware rather than assuming these
versions are interchangeable. No hardware compatibility result is claimed.

## Preparation and rollback

Follow docs/UNIVERSAL_LOADER_INSTALL.md for complete off-device project, plugin
folder and dated full MPC.settings backups. Never replace settings with an XML
fragment. Keep the known-good v0.1 ZIP and verify its hash before the session.

The 0.2.0 ZIP uses the supported pinned sd88me portable installer. Its guard
refuses an existing Loader folder/MPUL registration. It is **fresh-only**, not
an automatic update. Do not bypass that guard. In an approved session, first
back up Loader user data and remove only Loader with its supported uninstaller,
then install the reviewed GUI package. Delay and Freezer must remain untouched.

On macOS, verify the published SHA256, unzip
MPC-PD-Universal-Loader-0.2.0-mpc-armv7.zip and copy the whole extracted folder to
/tmp on the MPC using the existing SSH procedure. Run the package's install.sh
**on the device**, using its supported `-t /storage/Synths` option for the verified
mount. Do not run it locally on macOS, hand-edit settings or install another
firmware patch. Follow [upstream instructions](https://sd88me.github.io/mpc-vst-plugins/install.html).
If storage differs, select the correct supported Synths mount; no sdcard-only
assumption is made. Do not apply noexec/firmware workarounds without separate
review. Keep the installer's dated complete settings backup.

If anything fails, stop transport, lower monitoring volume, capture loader.log,
remove only Loader via the supported uninstaller with the same destination, and
reinstall the known-good v0.1 package in the approved session. Restore verified
complete settings/project backups only if needed. Confirm unrelated plugin
entries remain and Delay/Freezer still function. Neither uninstaller is executed
by development/CI; even fixture installer tests are excluded.

## Acceptance in an empty, backed-up project

- Discover as an audio effect; one instance, stereo, 44.1 kHz and 128-frame host
  blocks. Confirm a native LOADER tab appears with all assets and no blank page.
- Confirm 1280×628 content scaling, no clipped labels, readable text, finger-sized
  hit targets and accurate knob drag/touch regions on the One touchscreen.
- Tap Trim/Lowpass; only selection highlight changes. Active name/ID stay on the
  committed patch until a valid stopped-transport Apply completes.
- Previous selects Trim, Next selects Lowpass. Confirm the browser, both radio
  groups and physical Patch Q-Link stay synchronized.
- Apply shows REARM after requesting a switch. Tap REARM to set low before the
  next Apply. A held Apply must not retrigger. Confirm Q9 also rearms/requests
  correctly and both interaction methods share state.
- Q13 Patch, Q9 Apply, Q5 Gain, Q1 Mix; verify both screen/program modes and the
  8by1 mapping. First four parameter indices are unchanged.
- Validate active Trim/Lowpass text, status, full curated ID across its two fields,
  revision, and Gain/Mix values at 0/0.5/1. No meter or waveform is expected.
- While playing, request another patch: WAIT and old active name. Stop: LOAD,
  intentional fade/silence, then READY and new active name. Check asynchronous
  display refresh without touching a control. Unknown transport must not switch.
- Resume during fade/load, cancel by selecting/applying the active patch, and
  verify engine/GUI agreement and rollback status. No filesystem action should
  be tied to a touchscreen gesture callback.
- With plugin unloaded, temporarily remove a candidate in a disposable test
  folder. Reload and request it: MISSING, retained active patch, clear recovery
  after restoring the file/rearming. Test other engine errors/rollback using the
  existing v0.1 checklist; do not modify stable patches or use untrusted code.
- Save/reopen a v0.2 project on both patches with non-default controls. Open a
  copy of a backed-up v0.1 project and validate the 4→9 parameter extension and
  unchanged UL1 chunk recall. No readonly field write may affect sound or load.
- Test stereo isolation/filter behavior, fades, long-duration memory/CPU/xruns,
  deadline headroom, module residency and coexistence with installed stable
  plugins. QEMU results do not establish real-time performance.

The browser remains exactly two trusted flat Vanilla patches. No arbitrary
externals, MIDI, CV/Gate, dynamic filesystem browser or additional DSP. Native
crashes can affect MPC. Text is limited to seven characters per VST field; IDs
longer than 14 and revisions longer than seven visibly report LONG ID/LONGREV.
Detailed errors remain in loader.log. If One does not poll the readonly fields
as upstream Force does, record the host behavior and stop; do not replace the
native UI with a browser or add audio-thread host callbacks.
