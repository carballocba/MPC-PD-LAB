# v0.2 GUI implementation

The production engine, manifest validator, Pure Data patches and private Pd
build fixes are byte-for-byte v0.1. `packaging/universal/gui/preserved-files.json`
also pins all 19 pre-existing main source/patch/build/package/test files.
Delay/Freezer, IPTV Digger and their artifacts are not edited or installed.

Changes are confined to the Loader wrapper, its UI metadata/assets generator,
package pipeline, tests and GUI documentation:

- src/universal/vst2_wrap.c adds five readonly getters/displays. The first four
  setters/getters and render function are retained. No GUI thread, timer, mutex,
  allocation, filesystem or host refresh operation is added to rendering.
- packaging/universal/gui/layout.conf defines the actual dark native page. Native
  text is Titillium Web, baked titles use the pinned upstream OFL font. Minimum
  touch controls are 44 px; knob filmstrips stay below the upstream 12288 px
  image-height limit. There are no decorative meters.
- scripts/build_universal_gui.py composes pinned shadow_skin/force-shadow tools.
  A one-call adaptation directs free-standing text through force-shadow's
  existing TrueType label renderer. No new renderer/framework is created.
  Previous/Next use the generator's native Button/radio helpers with distinct
  asset names: two enum groups with the same key otherwise overwrite images in
  the upstream generator. Touch mappings remain Parameter 0 in both groups.
- packaging/universal/params.json appends readonly display slots; vst.json and
  the binary version become 200. MPUL and UL1 chunks are preserved.
- scripts/package_universal.py includes the generated skin in a portable 0.2.0
  ZIP, font/renderer notices and exact provenance. It still refuses replacing an
  existing Loader and never executes install.sh/uninstall.sh.
- tests/universal_gui_test.c exercises the native parameter contract through the
  VST ABI; tests/test_universal_gui.py validates actual native TUI/PNG/mappings,
  touch sizes, overlap/edges and preserved files. Existing engine/stress,
  sanitizer, ARM/QEMU and coexistence gates remain enabled.

Build-only Pillow 11.3.0 is the declared upstream skin/preview dependency and is
pinned in scripts/requirements-gui.txt. It is not shipped to ARM. sd88me tools
are MIT; vendored force-shadow renderer commit fa456fc90d07951723ab5f4e643d906cd2ac1bca
is MIT; Titillium Web is OFL-1.1. No new SDK or GPL runtime dependency is added.
The font and renderer notices are included in the ZIP. Hakai's historical
launcher is inspected only, not copied or executed.

Reproduce with the same pinned tools checkout used for v0.1:

```sh
python3 -m pip install -r scripts/requirements-gui.txt
python3 scripts/build_universal.py native
python3 scripts/build_universal_gui.py --upstream work/packaging-upstream
python3 tests/test_universal_gui.py --upstream work/packaging-upstream --skin 'work/universal-gui-skin/CARBALLO - VST - MPC PD Universal Loader/Plugin Skins'
```

Build directories are new `work/universal-gui-*`; validated v0.1 outputs and ZIP
names are not overwritten. After the pinned ARM baseline, run the same script
with armv7 inside its isolated toolchain container; package with
`python3 scripts/package_universal.py --upstream work/packaging-upstream` on the
build host with Pillow and ARM readelf. Outputs are work/universal-gui-dist.

The generated native-assets-0.png composites the actual native skin assets with
upstream studio.preview. Live Label Value fields are placeholders because the
MPC draws those values at runtime; it is an offline asset preview, not a hardware
screenshot or proof of host refresh. The selected radio state and knob positions
in the preview likewise do not represent a running MPC session.
