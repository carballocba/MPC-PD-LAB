#!/usr/bin/env python3
"""Run packaged upstream scripts against disposable local storage only."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
import zipfile

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('zip', type=Path)
p.add_argument('--upstream', type=Path, required=True)
a = p.parse_args()
ROOT = Path(__file__).resolve().parents[1]
subprocess.run([sys.executable, str(ROOT / 'scripts/validate_hakai.py'), str(a.zip), '--upstream', str(a.upstream)], check=True)
with tempfile.TemporaryDirectory() as tmp:
    t = Path(tmp)
    with zipfile.ZipFile(a.zip) as z:
        z.extractall(t / 'release')
    pkg = t / 'release/MPC-PD-Delay-1.0.0'
    storage = t / 'storage/Synths'
    storage.mkdir(parents=True)
    other = storage / 'sd88me - VST - Other'
    other.mkdir()
    (other / 'keep.so').write_bytes(b'other plugin sentinel')
    settings = t / 'MPC.settings'
    original = '''<?xml version="1.0"?>
<PROPERTIES>
  <VALUE name="unrelated" val="preserve me"/>
  <VALUE name="SynthContentLocations">
    <SynthContentLocations><Location>DEST</Location></SynthContentLocations>
  </VALUE>
  <VALUE name="pluginList-arm">
    <KNOWNPLUGINS>
      <PLUGIN name="Other" uid="12345678" file="/elsewhere/keep.so" format="VST"/>
    </KNOWNPLUGINS>
  </VALUE>
</PROPERTIES>
'''.replace('DEST', str(storage))
    settings.write_text(original)
    log = t / 'services.log'
    env = dict(os.environ, MPC_INSTALL_TEST='1', MPC_SETTINGS=str(settings),
               MPC_TEST_LOG=str(log), MPC_LEGACY_ROOT=str(t / 'legacy'))
    # macOS lacks GNU sha256sum; this test-only adapter implements its -c mode.
    # The package itself still requires the actual device sha256sum command.
    bindir = t / 'bin'
    bindir.mkdir()
    adapter = bindir / 'sha256sum'
    adapter.write_text('#!' + sys.executable + '\n' + '''import hashlib, sys
from pathlib import Path
assert sys.argv[1:] == ['-c', 'SHA256SUMS']
for line in Path('SHA256SUMS').read_text().splitlines():
    digest, name = line.split('  ', 1)
    if hashlib.sha256(Path(name).read_bytes()).hexdigest() != digest:
        sys.exit(1)
''')
    adapter.chmod(0o755)
    env['PATH'] = str(bindir) + os.pathsep + env['PATH']
    def run(script, success=True):
        result = subprocess.run(['sh', str(pkg / script), '-y', '-t', str(storage)],
                                env=env, text=True, capture_output=True)
        assert (result.returncode == 0) == success, result.stdout + result.stderr
        assert (other / 'keep.so').read_bytes() == b'other plugin sentinel'
        return result
    def snapshot():
        return {str(f.relative_to(t)): f.read_bytes() for f in t.rglob('*') if f.is_file()}
    # A colliding UID belongs to someone else. Refuse, with no storage/settings edits.
    settings.write_text(original.replace('12345678', '4D50444C'))
    before = snapshot()
    run('install.sh', False)
    assert snapshot() == before
    settings.write_text(original)
    target = storage / 'CARBALLO - VST - MPC PD Delay'
    target.mkdir()
    (target / 'mine').write_text('do not overwrite')
    before = snapshot()
    run('install.sh', False)
    assert snapshot() == before
    shutil.rmtree(target)
    # Integrity failure must precede copying or registration.
    patch = pkg / 'portable/CARBALLO - VST - MPC PD Delay/data/libpd_host.pd'
    good = patch.read_bytes()
    patch.write_bytes(good + b'corrupt')
    before = snapshot()
    run('install.sh', False)
    assert snapshot() == before
    patch.write_bytes(good)
    run('install.sh')
    tree = ET.parse(settings)
    plugins = tree.findall('.//PLUGIN')
    assert len(plugins) == 2
    own = next(x for x in plugins if x.get('uid') == '4d50444c')
    assert own.get('file') == str(target / 'mpc_pd_delay.so')
    assert own.get('isInstrument') == '0'
    assert tree.find('.//VALUE[@name="unrelated"]').get('val') == 'preserve me'
    assert any(x.read_text() == original for x in t.glob('MPC.settings.bak-*'))
    assert (target / 'data/libpd_host.pd').read_bytes() == good
    assert log.read_text().splitlines() == ['stop', 'start']
    before = snapshot()
    run('install.sh', False)  # rerun may not overwrite an existing installation
    assert snapshot() == before
    run('uninstall.sh')
    assert not target.exists()
    assert len(ET.parse(settings).findall('.//PLUGIN')) == 1
    assert ET.parse(settings).find('.//PLUGIN').get('uid') == '12345678'
    assert log.read_text().splitlines() == ['stop', 'start', 'stop', 'start']
    # Test actual upstream desktop OpenPackage without a network or device.
    go = shutil.which('go')
    if go:
        shutil.copy2(a.upstream / 'tools/desktop/pkg.go', t / 'pkg.go')
        (t / 'pkg_test.go').write_text('''package main
import ("os"; "testing")
func TestPhase3ZIP(t *testing.T) {
 p, err := OpenPackage(os.Getenv("PHASE3_ZIP"), "upload")
 if err != nil { t.Fatal(err) }
 if !p.Defer || len(p.Plugins)!=1 || p.Plugins[0].UID!="4d50444c" || p.Plugins[0].Kind!="effect" { t.Fatalf("bad package: %+v", p) }
}
''')
        subprocess.run([go, 'test', str(t / 'pkg.go'), str(t / 'pkg_test.go')],
                       env=dict(os.environ, PHASE3_ZIP=str(a.zip.resolve())), check=True)
    else:
        print('SKIP actual desktop OpenPackage: Go not installed')
print('PASS: local install/uninstall at explicit destination, settings backup and unrelated entry preservation')
print('PASS: UID/folder collision refusal, repeat refusal, corrupt payload refusal; no hardware access')
