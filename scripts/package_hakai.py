#!/usr/bin/env python3
"""Repackage an existing Phase 2 ARM folder; never rebuild or change its .so.
Requires the pinned sd88me checkout, Python 3 and pyelftools (validation).
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import zipfile

PIN = '5e3a96885b6de76a48b498821c379568a05cc006'
FOLDER = 'CARBALLO - VST - MPC PD Delay'
ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--phase2-folder', required=True, type=Path)
p.add_argument('--upstream', required=True, type=Path)
p.add_argument('--out', type=Path, default=ROOT / 'work/dist')
a = p.parse_args()
a.upstream = a.upstream.resolve()
a.out = a.out.resolve()
assert subprocess.check_output(['git', '-C', str(a.upstream), 'rev-parse', 'HEAD'], text=True).strip() == PIN
assert not subprocess.check_output(['git', '-C', str(a.upstream), 'status', '--porcelain', '--untracked-files=no'], text=True).strip()
so = a.phase2_folder / 'mpc_pd_delay.so'
# This package intentionally supports only the validated Phase 2 binary.
assert hashlib.sha256(so.read_bytes()).hexdigest() == '76a7340039def8494f9d78acb4ad23ac75f8b02b889557f47eb84dc26930e0df'
for patch in ('libpd_host.pd', 'stereo_delay_smoke.pd'):
    assert (a.phase2_folder / 'data' / patch).read_bytes() == (ROOT / 'patches' / patch).read_bytes()
for license_name in ('libpd.txt', 'pure-data.txt', 'wrapper.txt'):
    assert (a.phase2_folder / 'licenses' / license_name).stat().st_size > 100
sys.path.insert(0, str(a.upstream / 'tools'))
import gen_vst
with tempfile.TemporaryDirectory() as td:
    t = Path(td)
    skin = t / FOLDER
    (skin / 'Plugin Skins').mkdir(parents=True)
    # Minimal empty page follows shadow_skin.write_skin's containers. No controls,
    # artwork or Q-Link bindings; actual UI behavior remains hardware-unverified.
    tui = {'pageData': {'version': 1, 'componentDefinitions': {'version': 2,
           'importFiles': [], 'localComponentDefinitions': []},
           'info': {'version': 1, 'type': 'CompleteDescription'}, 'tabs': []}}
    (skin / 'Plugin Skins/TUI.json').write_text(json.dumps(tui, indent=2) + '\n')
    (skin / 'version.xml').write_text('<?xml version="1.0" encoding="utf-8"?>\n<plugincontent version="1.0"><identifier>CARBALLO.vst.mpcpddelay</identifier><version>1.0.0.0</version></plugincontent>\n')
    cfg = json.loads((ROOT / 'packaging/vst.json').read_text())
    # Folder/registry branding only. Embedded vendor remains MPC-PD-LAB; UID,
    # name, filename, version, parameters, wrapper and DSP remain byte-identical.
    cfg['vendor'] = 'CARBALLO'
    entry = t / 'entry.xml'
    entry.write_text(gen_vst.entry(cfg) + '\n')
    subprocess.run([sys.executable, str(a.upstream / 'tools/release.py'),
        '--so', str(so.resolve()), '--skin', str(skin), '--entry', str(entry),
        '--version', '1.0.0', '--id', 'mpc-pd-delay', '--repo', 'carballocba/MPC-PD-LAB',
        '--requires', 'Experimental; original MPC One Hakai 3.9.1 hardware UNTESTED; 44.1 kHz; one instance',
        '--about', 'Fixed 350 ms stereo libpd delay; no controls; hardware unverified',
        '--extra', str((a.phase2_folder / 'data').resolve()) + ':data',
        '--extra', str((a.phase2_folder / 'licenses').resolve()) + ':licenses',
        '-o', str(t / 'dist')], check=True)
    source = next((t / 'dist').glob('*.zip'))
    with zipfile.ZipFile(source) as z:
        files = {i.filename: (z.read(i), i.external_attr) for i in z.infolist()}
    top = 'MPC-PD-Delay-1.0.0/'
    script, mode = files[top + 'install.sh']
    # Upstream otherwise replaces same-UID entries and folders and can migrate
    # legacy assets. Refuse all those cases before any target/settings mutation.
    anchor = b'[ -f "portable/$SKIN/plugin-meta.xml" ]'
    guard = (ROOT / 'packaging/fresh_install_guard.sh').read_bytes()
    assert script.count(anchor) == 1
    script = script.replace(anchor, guard + b'\n' + anchor)
    files[top + 'install.sh'] = script, mode
    guide = (ROOT / 'docs/PHASE3_HAKAI_INSTALL.md').read_bytes()
    files[top + 'INSTALL.md'] = guide, 0o100644 << 16
    for rel in ('mpc-plugin.json', 'portable/' + FOLDER + '/mpc-plugin.json'):
        old, mode = files[top + rel]
        m = json.loads(old)
        m['os_compat'] = ['3.x']  # Scope, not a hardware certification.
        files[top + rel] = (json.dumps(m, indent=2) + '\n').encode(), mode
    provenance = {'upstream_packager': PIN, 'binary_sha256': hashlib.sha256(so.read_bytes()).hexdigest(),
                  'phase2_commit': 'aa96274280c260b92dbee3aa5636d6e0ed8abf7a',
                  'hardware_tested': False, 'license': 'See bundled third-party notices; project has no declared license'}
    files[top + 'PROVENANCE.json'] = (json.dumps(provenance, indent=2) + '\n').encode(), 0o100644 << 16
    sums = ''.join(hashlib.sha256(data).hexdigest() + '  ' + name[len(top):] + '\n'
                   for name, (data, _) in sorted(files.items()) if name != top + 'SHA256SUMS')
    files[top + 'SHA256SUMS'] = sums.encode(), 0o100644 << 16
    a.out.mkdir(parents=True, exist_ok=True)
    dest = a.out / source.name
    with zipfile.ZipFile(dest, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, (data, mode) in sorted(files.items()):
            i = zipfile.ZipInfo(name, (2026, 1, 1, 0, 0, 0))
            i.create_system = 3
            i.external_attr = mode
            i.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(i, data)
    subprocess.run([sys.executable, str(ROOT / 'scripts/validate_hakai.py'), str(dest), '--upstream', str(a.upstream)], check=True)
    print(dest)
