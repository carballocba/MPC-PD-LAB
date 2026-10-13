# MPC-PD-LAB

Experimental Pure Data audio effects for the original ARMv7 Akai MPC One.
The first prototype is a 350 ms wet-only stereo delay using libpd and the
sd88me/mpc-vst-plugins VST2 wrapper. Hardware compatibility is unverified.

See [ARMv7 build and testing](docs/ARMV7_BUILD.md),
[validation results](docs/PHASE2_VALIDATION.md), and
[the corrected libpd adapter](docs/LIBPD_ADAPTER.md).

```sh
scripts/armv7.sh
```

This builds and tests locally; it never connects to or modifies MPC hardware.

Phase 3: see [Hakai packaging and installation](docs/PHASE3_HAKAI_INSTALL.md).
Hardware compatibility remains unverified; no hardware deployment is part of this work.
