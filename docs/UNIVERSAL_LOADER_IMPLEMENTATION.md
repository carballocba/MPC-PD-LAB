# Universal Loader v0.1 implementation notes

The validated bridge is extended as a separate effect, MPUL, without modifying
Delay/Freezer source, identities or assets. Community pins and licensing are
recorded in the release PROVENANCE.json and vendor/universal/PROVENANCE.md.
`vst2_wrap.c` is a Loader-only derivative of the pinned sd88me C ABI/resource
lookup/conversion patterns. jsmn is vendored under MIT. libpd and Pd retain their
upstream licenses; no external SDK or arbitrary Pd external is redistributed.

## Ownership and bounded callback

The worker owns discovery, file I/O, manifest parsing, snapshot creation, Pd graph
open/close, warmup and error logging. Audio owns normal Pd processing and ramped
output. A lock-free atomic state machine and nonblocking CAS ownership guard
transfer the single private Pd instance. Audio never waits for the worker; when
ownership is unavailable it writes silence. Setters publish normalized values;
Gain/Mix messages are sent only on change, avoiding retriggered Pd ramps.
Transport must be positively stopped to replace an audible graph. Deactivation
permits worker takeover under the same guard. Apply is rising-edge, rearmed low.
Selecting/applying the current patch cancels an in-flight different selection.
Rollback reopens the retained snapshot; a rollback failure becomes safe silence.

Fade ramps are 512 samples out/in at 44.1 kHz. Audio processing is strictly 128
frames/stereo and rejects unsupported configuration. libpd is private and built
with `MULTI=false` **and `PDTHREADS=0`**; one instance per loaded module is supported.
This restriction avoids pretending independent simultaneous Pd contexts exist.
Project chunks store stable ID/revision and normalized Gain/Mix, never catalog
index or machine-specific paths. A bounded mailbox transfers restores to the
worker. In-progress state saves expose the last committed identity.

## Deliberate v0.1 scope refinements

Both supplied patches are Vanilla and use the same Gain/Mix contract. Flat
folders contain declared .pd files only, with an audited small object subset,
acyclic abstractions, at most 64 files/128 KiB each/1 MiB total. This is narrower
than arbitrary Vanilla compatibility and is intentional. No scripts, dynamic
objects, message boxes, loops, nested directories, DSOs or system services.
Snapshot directories alternate between two fixed names inside one owned session,
limiting Pd's interned path growth across repeated switching. Source folders are
not modified; snapshots/logs use writable data/user. Crash residue is not deleted
automatically: inspect it with the plugin unloaded. There is no watchdog that can
recover from arbitrary native crashes or an unbounded Pd computation.

A minimal fixed VST2 wrapper omits unrelated upstream MIDI/UI polling machinery.
Native Program-mode Q-Link maps are generated using upstream qlink_for_slot;
actual MPC mapping/transport/block-size behavior remains a hardware acceptance
gate. Generic host displays provide status; no custom GUI is claimed. CV/Gate,
MIDI, externals, dynamic browser and multiple instances remain later work.

## Symbol boundary

The ARM effect exports only VSTPluginMain; Pd symbols are hidden/local and linked
symbolically. A separately dlopened ARM .pd_linux external cannot resolve
class_new/gensym/pd_new against this private namespace. Correct CPU/float ABI alone
does not solve that boundary. v0.1 rejects externals rather than weakening symbol
isolation or relying on host-global Pd. See docs/design/PD_SYMBOL_BOUNDARY.md.

## Reproduction and review

Dedicated CI runs Linux native, sanitizer, pinned ARM/QEMU and deterministic ZIP
validation without installers. The legacy installer-fixture CI is skipped only
for this development branch to honor the explicit no-installer instruction.
Code and package are reviewable independently of hardware approval. Native
sanitizers and stress tests provide evidence, not proof of deadline performance.

## Correction discovered by the callback audit

The pinned Pd header defaults PDTHREADS to 1 even with libpd MULTI=false. The
initial Linux/ARM callback audit demonstrated a pthread_mutex_lock call in
libpd_process_short. Loader therefore builds its **own** isolated archive with
PDTHREADS=0, leaving the existing Delay/Freezer archives and code unchanged.
Exclusive host ownership replaces Pd's internal mutex. This corrects the earlier
design assumption that MULTI=false by itself makes sys_lock a no-op. The native
and ARM callback audits must pass with this configuration.

The pinned no-thread Pd branch also fails compilation: pd_queue_mess has a
by-value instance where its public declaration takes a pointer; macOS scheduling
code lacks pthread declarations when its include is conditional. Two minimal
repairs are applied only to the copied Loader build source, recorded verbatim in
vendor/universal/pd-no-threads.patch and redistributed in licenses/. DSP sources
and the pinned dependency checkout are unchanged.

## LeakSanitizer cleanup correction

Linux LSan identified 47,808 leaked bytes in 664 allocations during the stress
suite. Every reported allocation originated in the pinned g_io.c signal
inlet/outlet reblocker constructors. Their destructors release the nested buffers
but omit freeing x_rb itself. Loader's copied Pd source now frees that array
after its members. This worker-side lifetime repair does not change DSP and is
recorded in the redistributed patch; legacy dependencies remain untouched.
Leak detection stays enabled. The callback audit also interposes read/write/close
and nanosleep, alongside allocation, open/fopen, locks and join.

## Module lifetime

After the per-switch leak was fixed, engine LSan passed. VST DSO dlclose exposed
303,483 bytes of Pd class/symbol/process globals: libpd has no supported full
teardown of its default single instance. Linux/ARM Loader therefore links with
ELF NODELETE, retaining one initialized private module until host-process exit.
Close still joins the worker, closes graphs and frees Loader resources. A host
dlclose/reopen test verifies reuse. This fixes repeated unload/reload abandoning
Pd globals without suppressing LSan or trying unsupported global teardown.
The tradeoff is resident module/class memory for the MPC application lifetime;
restarting the application reclaims it. macOS is a development-only host.
