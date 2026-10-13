# Hidden Pd symbols and ARMv7 externals: prototype decision

Date: 2026-10-13. Static binary/source investigation, with no ARM binaries loaded and
no changes to working plugins. New runtime external support is excluded from v0.1.

## What the boundary is

An external such as `jam.pd_linux` is a shared ELF module. Besides exporting its setup
function, it imports Pd functions/data (`class_new`, `pd_new`, `gensym`, `s_list`, etc.).
The ELF loader must find a compatible Pd provider in its symbol resolution scope. A
`.pd_linux` filename and matching ARM hard-float format do not establish that provider.

Our build compiles private Pd into the VST shared object. [scripts/build.sh](https://github.com/carballocba/MPC-PD-LAB/blob/879ab14336fd5c4576ab9832127b4047aaa2a2b6/scripts/build.sh)
uses `-fvisibility=hidden`, a static archive, `-Wl,--exclude-libs,ALL` and `-Bsymbolic`.
[scripts/build_organelle.py](https://github.com/carballocba/MPC-PD-LAB/blob/879ab14336fd5c4576ab9832127b4047aaa2a2b6/scripts/build_organelle.py) uses the same ARM policy.
[scripts/verify_elf.py](https://github.com/carballocba/MPC-PD-LAB/blob/879ab14336fd5c4576ab9832127b4047aaa2a2b6/scripts/verify_elf.py) explicitly requires only `VSTPluginMain`
to be exported. This deliberate isolation reduces cross-plugin symbol interposition.

The existing libpd headers/APIs used internally still work because they are linked
within that module. This does not export the Pd object API to independently loaded
external DSOs. `-Bsymbolic` binds internal references; removing it alone does not expose
hidden/local symbols. Loading an external with `RTLD_GLOBAL` likewise cannot turn the
VST's local symbols into exported definitions. Exported APIs also require compatible
loader scope; blanket visibility changes are insufficient and can bind the wrong Pd.

## Static binary evidence

The locally cached Phase 4 ARM artifact `organelle_granular_freezer.so` has SHA-256
`9e736bbb51464e5af7ed15084772839df285248066281b7e00351a573fbff24d`.
It exports exactly **VSTPluginMain**. Its complete symbol table contains definitions
for `class_new`, `gensym`, `pd_new`, `sys_lock` and `s_list` as **STB_LOCAL** symbols.
Presence in `.symtab` is not availability in `.dynsym` to the external loader.
This fingerprint is of the local CI artifact; the installed MPC binary was not read.

The inspected PLAY/CZ binaries are ARM ELF32 hard-float, but:

| External | SHA-256 | Pd unresolved imports observed |
|---|---|---|
| jam.pd_linux | `f1ebc93e98a3cfd02b328c3b1d8b3da050bfa3ca8a83f412d5ecc51e0fbd2c20` | class_new, pd_new, gensym, s_list |
| fsynth~.pd_linux | `3a04ded915f61999011b36cee2e1faeb8fcbcbcbd0dac01c9c7f65b72de2d359` | class_new, pd_new, gensym |

These imports cannot be satisfied by the observed plugin's local Pd definitions.
Another global Pd provider in the host could change resolution, but its existence,
ABI and ownership are not established and must never be relied on implicitly. This
is static evidence of a binding boundary, not a claimed measured MPC dlopen failure.
Raw observations are in [evidence/PD_SYMBOL_OBSERVATIONS.json](evidence/PD_SYMBOL_OBSERVATIONS.json).

## Loader and ABI details

[src/s_loader.c](https://github.com/pure-data/pure-data/blob/f009fd8d7b537e209e09898d487fdf1bf547da2b/src/s_loader.c) uses `dlopen(...RTLD_NOW | RTLD_GLOBAL)` and setup
functions, and keeps loaded-library records. Closing a Pd canvas is not a promise of
unloading external DSOs/resetting globals. [src/m_pd.h](https://github.com/pure-data/pure-data/blob/f009fd8d7b537e209e09898d487fdf1bf547da2b/src/m_pd.h) distinguishes
float size and instance/thread configuration. ARMv7/VFP arguments, instruction set,
glibc/GLIBCXX, library dependencies, setup symbol spelling, Pd API/float ABI and global
state all need verification. Missing objects can still leave a non-null canvas;
[src/g_text.c](https://github.com/pure-data/pure-data/blob/f009fd8d7b537e209e09898d487fdf1bf547da2b/src/g_text.c) reports creation errors that a host must capture.

[libpd_wrapper/z_libpd.c](https://github.com/libpd/libpd/blob/ba0dc63262901d658af8bbda5e619a60fa975e78/libpd_wrapper/z_libpd.c) wraps processing and messages in sys_lock;
[src/s_inter.c](https://github.com/pure-data/pure-data/blob/f009fd8d7b537e209e09898d487fdf1bf547da2b/src/s_inter.c) defines it as a no-op without PDTHREADS. The current
[Makefile](https://github.com/libpd/libpd/blob/ba0dc63262901d658af8bbda5e619a60fa975e78/Makefile) only adds PDINSTANCE/PDTHREADS for `MULTI=true`.
Keep `MULTI=false` and one active Universal instance for this prototype; exclusive
thread ownership is essential because there is no mutex protecting concurrent calls.
Do not enable MULTI as an incidental fix: it changes locking and does not automatically
isolate every external's globals.

## Options and decision

| Approach | Change / consequence | v0.1 decision |
|---|---|---|
| Vanilla built-in objects + text abstractions | Existing private Pd handles graph loading with no independent external DSO | **Use now**; runtime .pd files can change without rebuilding VST |
| Statically compile an audited external and call its setup once | Links against the same private Pd; rebuild VST when adding a new object implementation | Later option; patches can subsequently use that registered object without recompilation |
| Export Pd API from plugin | Changes symbol isolation and must handle host loader scope/cross-plugin conflicts | Defer |
| Shared external libpd provider | New runtime dependency, ABI/version/ownership and deployment design | Defer |
| Isolated helper process | IPC/audio scheduling/latency and process management cost | Long-term research, not first prototype |

No LD_PRELOAD, system service edits, arbitrary dynamic external loading or new global
Pd symbols are needed to demonstrate runtime patch selection. Reject manifests declaring
externals and binary/script assets; offline object audit plus curated trusted patches is
a compatibility gate, not a sandbox against hostile Pd code. Unknown-object/load errors
abort activation and trigger rollback. The working Delay and Freezer keep their current
symbol policies and identities. Jam/PLAY licensing remains unresolved independently.

For a future external-support milestone, require an ARM host fixture that compares
import/export sets, verifies the chosen API provider under actual loader flags, loads
one explicitly licensed external, processes it, checks multiple-plugin isolation and
repeated graph unload/reload, and measures original-One timing. No such runtime claim
is made here.

## Implementation correction (v0.1 callback audit)

The conclusion above about MULTI=false disabling mutexes was incomplete:
[src/m_pd.h](https://github.com/pure-data/pure-data/blob/f009fd8d7b537e209e09898d487fdf1bf547da2b/src/m_pd.h#L1013) defaults PDTHREADS to 1.
Linux and ARM callback audits demonstrated a mutex call. Loader now explicitly
builds its isolated archive with PDTHREADS=0 and MULTI=false; only that combined
configuration supplies the no-op sys_lock. No legacy plugin is rebuilt with
changed flags. tests/universal_rt_audit.c guards this requirement.
