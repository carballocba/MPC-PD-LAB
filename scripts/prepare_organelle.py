#!/usr/bin/env python3
"""Validate one audited port manifest and generate wrapper/engine mapping inputs."""
import hashlib
import json
from pathlib import Path
import re
import shutil
import sys

ROOT = Path(__file__).resolve().parents[1]
PORT = ROOT / 'ports/granular-freezer'
# Audited for this port and pinned libpd. This is deliberately not an automatic
# converter or a claim that an arbitrary Pd object is supported/safe.
VANILLA = set('adc~ dac~ s~ r~ s r catch~ throw~ * *~ + +~ - / % i f t bang loadbang inlet inlet~ outlet outlet~ list pack unpack route sel swap change clip clip~ expr expr~ delwrite~ vd~ noise~ phasor~ samphold~ sig~ tabread4~ random mtof vline~ line~ stripnote bag bng tgl until moses lop~'.split())

def audit(directory=PORT):
    m = json.loads((directory / 'manifest.json').read_text())
    hashes = json.loads((directory / 'upstream.sha256.json').read_text())
    assert set(hashes) == {'main.pd', 'grain.pd', 'grainvoice.pd', 'latch-notes.pd', 'LICENSE.txt'}
    assert {p.name for p in (directory / 'upstream').iterdir()} == set(hashes), 'Unexpected upstream files'
    for name, digest in hashes.items():
        assert hashlib.sha256((directory / 'upstream' / name).read_bytes()).hexdigest() == digest, name
    assert not m['externals'] and not m['assets']
    assert m['vst']['uid'] == 'CGFZ' and m['vst']['uid'] != 'MPDL'
    unknown = set()
    for f in (directory / 'upstream').glob('*.pd'):
        # Pd records may span lines; escaped semicolons belong to message atoms.
        for record in re.split(r'(?<!\\);', f.read_text()):
            match = re.search(r'(?:^|\n)#X obj \S+ \S+ (\S+)', record)
            if match:
                name = match[1].rstrip(',')
                if name not in VANILLA | set(m['abstractions']): unknown.add(name)
    assert not unknown, 'Unaudited/missing externals: ' + ', '.join(sorted(unknown))
    assert len(m['controls']) == 6 and len({c['key'] for c in m['controls']}) == 6
    for c in m['controls']:
        assert re.fullmatch(r'[a-z0-9-]+', c['key']) and re.fullmatch(r'mpc-[a-z0-9-]+', c['receiver'])
        assert c['mode'] in ('continuous', 'toggle-edge') and 0 <= c['default'] <= 1
    return m

def prepare(out):
    m = audit()
    out.mkdir(parents=True, exist_ok=True)
    data = out / 'data'
    shutil.copytree(PORT / 'upstream', data / 'upstream', dirs_exist_ok=True)
    shutil.copy2(PORT / 'host.pd', data / 'host.pd')
    shutil.copy2(ROOT / 'compat/organelle/mpc_organelle_mother.pd', data)
    shutil.copy2(PORT / 'manifest.json', data)
    rows = ['{"%s", "%s", %sf, %d}' % (c['key'], c['receiver'], float(c['default']), c['mode']=='toggle-edge') for c in m['controls']]
    header = 'static const control_map MAP[] = {\n' + ',\n'.join(rows) + '\n};\n#define NCONTROLS 6\n'
    header += 'static const char *REQUIRED[] = {\n' + ',\n'.join(json.dumps(x) for x in m['required_files']) + '\n};\n'
    header += 'static const int MIDI_CC[] = {' + ','.join(str(next((k for k,v in m['midi']['cc'].items() if v==c['key']),-1)) for c in m['controls']) + '};\n'
    (out / 'organelle_map.h').write_text(header)
    params = [{'key': c['key'], 'name': c['name'], 'min': 0, 'max': 1, 'default': c['default'], **({'options':['Off','On']} if c['mode']=='toggle-edge' else {})} for c in m['controls']]
    (out / 'params.json').write_text(json.dumps(params, indent=2)+'\n')
    v=m['vst']
    cfg={'name':v['name'],'vendor':v['vendor'],'uid':v['uid'],'version':v['version'],'so':v['so'],
         'params':'params.json','effect':True,'custom_skin':True,'cc':False,'nrpn':False,
         'defines':{'MODULE_SUBDIR':'"data"','MODULE_DIR':json.dumps(str(data.resolve()))}}
    # ARM never gets a build-host absolute fallback; resource discovery must be relative.
    if 'armv7' in str(out): cfg['defines']['MODULE_DIR']='"/nonexistent-mpc-organelle-data"'
    (out / 'vst.json').write_text(json.dumps(cfg,indent=2)+'\n')
    print('PASS original source hashes, vanilla object audit, manifest and generated control map')

if __name__=='__main__':
    prepare(Path(sys.argv[1]))
