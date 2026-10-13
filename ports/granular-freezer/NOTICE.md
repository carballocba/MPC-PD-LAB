# Granular Freezer notices

Original patch: Critter & Guitari, copyright 2016, BSD-3-Clause except attributed
imports, from `critterandguitari/Organelle_Patches` revision
`72d19ed847ee40a89e6f40bcebce56229be1ef0b`, `Effects/Granular Freezer`.
All four upstream Pd files and LICENSE.txt are included unchanged in data/upstream.

The two crossfade subpatches in main.pd credit Hans-Christoph Steiner (2004),
adapted from equal_power_pan~ and pan_core~. Their reference library is pan 0.1.2,
GNU GPL v2 or later (`pan-meta.pd`). We preserve the attribution and include the
full GPL v2 text in pan-GPL-2.0.txt in addition to C&G's BSD notice. The complete
editable Pd source of these adaptations is shipped, not just a binary.
Reference source distribution (no reference-library external is required at runtime):
https://deb.debian.org/debian/pool/main/p/pd-pan/pd-pan_0.1.2.orig.tar.gz

libpd and Pure Data notices and sd88me wrapper/installer MIT notice are included
alongside these files. No Zynthian code is copied into this runtime; its GPL-3.0
port was studied as a protocol reference only. New MPC bridge code remains in
MPC-PD-LAB, whose overall project license has not yet been declared. This package
is not submitted to the sd88me catalog and does not claim its verification badge.
