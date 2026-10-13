# v0.1 validation and hardware acceptance checklist

## Automated, no hardware

- Build pinned sources: `scripts/dependencies.sh`, then
  `python3 scripts/build_universal.py native` on Linux.
- Repeat with `--sanitizer address` (ASan + UBSan) and `--sanitizer thread`.
  Dedicated CI uses a 28-bit mmap randomization setting on its ephemeral runner
  for GCC TSan address-space compatibility; this never touches an MPC.
- Engine tests: stereo isolation, smoothed gain, nonfinite input, transport known/
  unknown/playing, cancellation, resume during fades, stalled worker silence,
  malformed candidate rollback, missing-file rejection, deactivated handoff,
  1000 alternating switches, concurrent controls/state publication, and shutdown.
- Manifest tests reject unsupported objects and invalid metadata. VST ABI tests
  exercise effect identity, four controls, chunks, reopen/recall and bad configs.
- `python3 scripts/audit_universal.py 'work/universal-native/package/CARBALLO - VST - MPC PD Universal Loader/data/user'` audits the flat closure.
- `scripts/armv7.sh`, then run `scripts/build_universal.py armv7` in that same
  pinned toolchain container. ELF checks require ARM EABI hard-float, VSTPluginMain,
  private Pd symbols and the established dynamic-library/glibc policy. QEMU tests
  are functional tests; their timing does not establish hardware performance.
- `scripts/package_universal.py --upstream <pinned tools checkout>` verifies ZIP
  topology, exact identity, checksums, assets/licenses, native Q-Link mappings and
  fresh-install guard. Repeat packaging and compare ZIP bytes. Installer execution
  is deliberately excluded by the user's instruction.
- Upstream Linux non-installer suites provide independent metadata/tooling checks.
  Earlier 24 macOS failures and GNU/BSD classification remain in
  `docs/design/UPSTREAM_TEST_FAILURE_CLASSIFICATION.md`; no failing test is
  silently reclassified as a pass.

## Hardware — pending explicit approval, never automated deployment

Record artifact SHA256, firmware (target Hakai 3.9.1), original One model,
44.1 kHz, host block size and storage destination. Complete backup and recovery
rehearsal first. Require all of these before declaring hardware acceptance:

- Effect discovery, four Q-Link labels/ranges, one instance and READY indication.
- Left-only, right-only and stereo signal checks on both patches; low volume.
- Gain 0/0.5/1, wet/dry lowpass behavior, no NaN/clipping from bounded test inputs.
- Switch A/B while stopped at least 100 times. Confirm fades and intentional
  silence, no application hang, no clicks beyond measured acceptance threshold.
- Request during playback/unknown transport; confirm no graph replacement until
  positively stopped. Resume at each fade/load stage; confirm old graph recovery
  or silence until stopped, never simultaneous Pd ownership.
- Cancel, missing candidate, malformed connection and revision mismatch: retained
  graph or safe silence, readable error, recovery with a valid request.
- Save/reopen a project on each patch with non-default Gain/Mix; match stable ID,
  revision and values. Test reordered catalog and unavailable ID explicitly.
- Restart after a simulated unsuccessful load; verify Delay/Freezer unchanged.
- Measure xruns, CPU/memory, worst-case callback time and switch duration over a
  prolonged session. Require callback completion within 128/44100 seconds with
  agreed safety headroom and no persistent memory growth. QEMU is insufficient.
- Uninstall/rollback only Loader, verify settings unrelated entries and original
  plugins, then restore the known-good project.

Do not feed CV hardware, modify calibration, or test arbitrary externals here.
A native in-process libpd crash cannot be contained by this prototype.
