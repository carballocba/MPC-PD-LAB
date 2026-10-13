# Upstream test failures: complete classification for Loader prototype

Date: 2026-10-13. Upstream revision `5e3a96885b6de76a48b498821c379568a05cc006`.
Research diagnostics only. All installer/store tests operate on generated fake packages,
synthetic settings, temporary systemd roots and mocked service calls. No physical MPC,
installed plugin, real MPC.settings or real service configuration was accessed.

## Result and root causes

The original 125-test catalog suite reproduces **101 passes, 24 failures, zero errors**.
A controlled host-compatibility experiment isolates the failures without changing upstream
code or adding dependencies to the plugin:

| Stage | Change confined to temporary diagnostic PATH | Result |
|---|---|---|
| Baseline | Native macOS tools | 125 run; 101 pass; 24 fail |
| Checksum | `sha256sum` delegates to `/usr/bin/shasum -a 256` | 125 run; 123 pass; 2 fail |
| Add-in follow-up | Also translate GNU `sed -i` to BSD `sed -i ''` | Remaining 2 tests rerun; mixed store/add-in passes, add-in shell aggregate fails |
| Assertion follow-up | Also normalize padded stdin `wc -l` output | Remaining add-in aggregate rerun; passes |

The checksum substitute computes/checks actual SHA-256; it does not bypass verification.
The sed substitute only translates the `-i` spelling. The wc substitute only strips the
padding of a stdin line count, exposing two string-equality assertions at test_addin.sh
lines 66 and 185. These diagnostic adapters were not committed as production tools,
installed globally or proposed as an MPC workaround. No claim of one full native Linux
125/125 run is made: the final two/one runs are targeted follow-ups on macOS.

**Interpretation:** 22 original failing cases are cleared by checksum-tool compatibility
alone. One mixed store/add-in case additionally needs GNU sed semantics. One add-in
shell aggregate additionally exposes BSD wc assertion formatting. The early checksum
failure hid deeper add-in portability issues; the 24 failures were never evidence of
24 libpd/DSP bugs. No loader or Pd audio test belongs to this suite.

## Every failing case

| Exact unittest case | Root cause | Relevance to proposed prototype | Controlled result |
|---|---|---|---|
| AddinTest.test_installer_shell_tests | GNU sed -i spelling; after translation, two BSD wc padding assertions | Out of scope: LD_PRELOAD/systemd add-in installer, not VST payload | Fails with checksum only; passes after sed and wc diagnostic translations |
| InstallerTest.test_a_failed_settings_edit_still_restarts_mpc_unless_deferred | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_defer_flag_leaves_mpc_alone_for_a_batch | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_files_next_to_no_legacy_install_are_not_touched | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_fresh_and_legacy_upgrade | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_install_and_uninstall_stop_and_start_mpc_once | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_legacy_user_files_are_moved_and_packaged_data_removed | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_lost_exec_bits_and_symlinks_are_restored | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_reinstall_is_idempotent_and_keeps_user_files | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_second_run_keeps_what_the_first_moved | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_uninstall_leaves_only_the_user_data_folders | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| InstallerTest.test_uninstall_removes_entry_and_folder_but_keeps_user_files | Missing sha256sum; integrity preflight fails before expected downstream behavior | Direct: package safety, install/uninstall/backup/preservation | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_a_3x_only_plugin_on_a_device_that_looks_like_2x_gets_a_note_and_still_installs | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_a_gen1_device_still_installs_the_armv7_zip | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_a_gen2_device_installs_the_aarch64_zip | Missing sha256sum; integrity preflight fails before expected downstream behavior | Excluded target: AArch64/Gen2; no bearing on ARMv7 loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_a_plugin_that_needs_a_newer_glibc_than_the_device_has_gets_a_warning | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_addins_install_with_plugins_in_one_restart_update_and_remove | Missing sha256sum masks add-in GNU sed -i incompatibility | Mixed: VST package half relevant; add-in half excluded | Fails with checksum only; passes with checksum and sed translations |
| StoreTest.test_an_old_installer_is_not_given_n_and_restarts_mpc_by_itself | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_install_verifies_installs_and_restarts_mpc_once | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_no_note_on_a_3x_device_or_when_the_libc_cannot_be_found | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_older_glibc_is_read_from_the_library_file_name | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_remove_deletes_the_folder_and_entry_but_keeps_user_files | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_sync_command_uses_hash_checked_helpers | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |
| StoreTest.test_update_takes_a_newer_version_and_holds_back_a_major_change | Missing sha256sum; integrity preflight fails before expected downstream behavior | Indirect: optional catalog/store path and package checks; not loader DSP | Passes with checksum diagnostic translation; no upstream source changes |

## Source evidence and release disposition

- [tools/test_catalog.py](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/test_catalog.py): InstallerTest, StoreTest and AddinTest fixture/environment setup and exact cases.
- [tools/release/install.sh](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release/install.sh): `sha256sum -c SHA256SUMS` runs before copying/settings mutation. A checksum preflight failure also explains the missing expected stop/start calls in the failed-settings-edit test.
- [tools/mpc-store.sh](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/mpc-store.sh): `sha_of` invokes sha256sum before installing. Warnings, updates, removal and sync tests can fail at that prerequisite instead of their named behavior.
- [tools/release/addin/addin-lib.sh](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/release/addin/addin-lib.sh): edit_preload uses GNU-style `sed -i`.
- [tools/test_addin.sh](https://github.com/sd88me/mpc-vst-plugins/blob/5e3a96885b6de76a48b498821c379568a05cc006/tools/test_addin.sh): the two padded `wc -l` string comparisons. This LD_PRELOAD path is excluded entirely from Loader v0.1.

All 11 InstallerTest failures are relevant to the package safety story and must remain
release gates. Store tests inform checksum/device/version handling but the prototype
need not ship a catalog store. Gen2 installation and add-in preload behavior are excluded.
Run the full untouched suite in a recorded Linux GNU/BusyBox-compatible CI environment
before releasing the future loader package, then run our package-specific preservation
fixtures. Test fixtures must never point at real settings or hardware. Do not alter the
standard device installer just to make macOS fixture syntax pass.

The initial 75 offline catalog/build/site tests and 10 skin tests remain useful, but are
not substituted for the new concurrency, patch switching and hardware tests in the
[prototype design](UNIVERSAL_LOADER_PROTOTYPE.md). Machine-readable evidence is in
[evidence/UPSTREAM_FAILURE_RESULTS.json](evidence/UPSTREAM_FAILURE_RESULTS.json).
