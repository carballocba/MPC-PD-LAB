# Phase 3 offline validation — 2026-10-13

No MPC connection, installation, system-file modification or hardware test occurred.

## Binary provenance

Reused the main-branch Phase 2 GitHub Actions artifact `11793838069`, run
[38359378179](https://github.com/carballocba/MPC-PD-LAB/actions/runs/38359378179),
commit `aa96274280c260b92dbee3aa5636d6e0ed8abf7a`.
Downloaded artifact ZIP SHA-256 matched GitHub's published artifact digest:
`e866d3886930eb3f350b1693ad26a53cf0688de9dd7420db7837b55516aab982`.
Extracted `.so` matches Phase 2's recorded binary SHA-256 exactly:
`76a7340039def8494f9d78acb4ad23ac75f8b02b889557f47eb84dc26930e0df`.
No DSP, wrapper, parameter or binary identity changes.

## Results

| Check | Result |
| --- | --- |
| ARMv7 ELF32 little-endian DYN, EABI5 hard-float, v7 attributes, VFP argument ABI | PASS |
| Defined global dynamic export | PASS: only VSTPluginMain |
| Exact dependencies | PASS: libc.so.6, libm.so.6, libdl.so.2, libpthread.so.0; no libpd.so |
| Required maximum glibc | PASS: 2.29; equals manifest; below verified unit's user-reported 2.39 |
| Runtime search paths and text relocations | PASS: absent |
| MPDL / 4d50444c, name, effect classification, stereo I/O, portable metadata | PASS |
| Both Pd patches byte-for-byte against repository; bundled license notices | PASS |
| Single ZIP top folder, paths, checksums, upstream catalog_check | PASS (review notes below) |
| Actual upstream desktop OpenPackage against ZIP, using pinned pkg.go | PASS, Go 1.27.2 on macOS ARM64 |
| Generated upstream install at explicit temporary storage path | PASS; complete settings backup and unrelated plugin/settings preserved |
| Case-insensitive UID collision, existing folder, repeat install | PASS: refused without settings/storage changes |
| Corrupted runtime payload | PASS: refused without settings/storage changes |
| Uninstall | PASS: removes own registration/folder, preserves unrelated plugin |
| Repeat packaging, identical ZIP bytes | PASS |
| Python compile, shell syntax, git whitespace | PASS |
| Fresh complete desktop and ARM/QEMU audio tests plus package tests | PASS in [Phase 3 branch run 38366403854](https://github.com/carballocba/MPC-PD-LAB/actions/runs/38366403854), before PR opening |
| MPC hardware loading, resource lookup, empty skin, effect display, real-time performance | UNTESTED |

The first Phase 3 CI run passed all steps on implementation commit `4b624d7`.
Subsequent changes add historical Hakai source findings and this report, and
remove unused imports. Final-branch CI is linked from the PR checks; the workflow
uploads the ZIP and build reports and compares two packaging runs.
The local final package was revalidated after those edits.

Final local release ZIP SHA-256:
`a802278ab1ded9ff12cec54aa5543b1095b6907aca7d669d398168a03ef7e0e5`.
The validation report is outside the package to avoid circular checksums.

## Upstream review notes

- The project has no declared license. Third-party libpd/Pure Data/wrapper
  licenses are included. `catalog_check` accepts a local release with a warning;
  `--catalog` submission is blocked until project licensing is declared.
- `install.sh` deliberately differs from the upstream generated template by
  the fresh-install-only preflight. It refuses overwrites/collisions rather than
  allowing the upstream update/migration path. `uninstall.sh` and registry AWK
  remain upstream-generated. Review `packaging/fresh_install_guard.sh`.
- Static ELF scanning flags libpd/Pure Data network/process/dlopen/setuid symbols.
  That is a capability review flag; the fixed delay patch does not use those
  objects. No claim of sandboxing or removal of those compiled capabilities.
- Metadata/folder manufacturer is CARBALLO, while unchanged binary reports
  MPC-PD-LAB. MPDL, filename, name and VST version remain stable.
- Historical Hakai fork was inspected, but its old launcher has no VST manager.
  Current 3.9.1 discovery source remains unavailable; only sd88me's route is
  verified from installer source.

See PHASE3_HAKAI_INSTALL.md for installation/rollback steps and all limitations.
QEMU timings do not establish MPC real-time performance. The fixed 350 ms delay
has no custom GUI/Q-Link mapping. Plugin crashes may affect the MPC application.
