# GUI v0.2 test record

Base: PR #5 head 2b7d25b4738145c279dd4ab511d73464976b3ebf. Hardware NOT tested.
No installer, MPC.settings operation, hardware access or merge is part of this
validation. Exact final CI run/commit and artifact hashes are recorded in the
review PR and ZIP PROVENANCE.json.

Local development checks passed: existing engine/manifest/VST tests with 20
switches, plus the native GUI VST contract (selection versus active, readonly
writes ignored, held Apply/rearm, playing WAIT, stopped load, Gain/Mix readback,
UL1 project recall, missing candidate and recovery). Native GUI generation and
skin_check passed. Actual generated TUI/PNG tests cover 44px touch boxes, overlap,
image validity, enum button IDs, Apply Toggle Switch actions, all nine parameter
bindings, readonly fields and unchanged four Q-Link bindings. Preserved file
hashes include the v0.1 engine and all 19 original main files.

Required CI gates remain: Linux 1,000-switch/concurrency suite; ASan/UBSan/LSan,
TSan, callback interposition including large-file opens; native GUI contract;
75 non-installer upstream catalog tests and 10 skin tests; unchanged Delay and
Freezer individual/coexistence tests; ARM hard-float/QEMU GUI/engine tests and
repeat link/ZIP byte comparisons. The draft PR must not be opened until these
pass on its implementation snapshot. Fifty installer-related upstream tests
remain intentionally excluded by the no-installer instruction.

The native asset preview uses actual generated files. Host-driven text values
and selected states are placeholders, not a hardware capture. Physical touch,
readout refresh, host parameter-count upgrade compatibility, Q-Link page behavior,
audio deadlines and installation/rollback remain explicit hardware gates in
UNIVERSAL_LOADER_GUI_HARDWARE_CHECKLIST.md.

The implementation does not add fake meters/progress, unsafe refresh callbacks,
a desktop editor or an alternate runtime display. Appended readonly slots and
two-stage Apply/rearm are intentional documented boundaries. Additional patches
still require an audited catalog/engine extension; no unreviewed file browser is
exposed. The validated v0.1 package is preserved separately.
