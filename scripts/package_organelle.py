#!/usr/bin/env python3
"""Package the separately identified, validated Granular Freezer ARM build."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import zipfile
import shutil

ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--build',type=Path,default=ROOT/'work/organelle-armv7')
p.add_argument('--upstream',type=Path,required=True)
p.add_argument('--out',type=Path,default=ROOT/'work/organelle-dist')
a=p.parse_args()
assert subprocess.check_output(['git','-C',str(a.upstream),'rev-parse','HEAD'],text=True).strip()=='5e3a96885b6de76a48b498821c379568a05cc006'
assert not subprocess.check_output(['git','-C',str(a.upstream),'status','--porcelain','--untracked-files=no'],text=True).strip()
from prepare_organelle import audit
m=audit()
so=a.build/m['vst']['so']
subprocess.run([sys.executable,str(ROOT/'scripts/verify_elf.py'),str(so)],check=True,stdout=subprocess.DEVNULL)
folder=m['vst']['vendor']+' - VST - '+m['vst']['name']
with tempfile.TemporaryDirectory() as tmp:
    t=Path(tmp);skin=t/folder;(skin/'Plugin Skins').mkdir(parents=True)
    (skin/'version.xml').write_text('<plugincontent version="1.0"><identifier>CARBALLO.vst.organellegranularfreezer</identifier><version>1.0.0.0</version></plugincontent>\n')
    tui={'pageData':{'version':1,'componentDefinitions':{'version':2,'importFiles':[],'localComponentDefinitions':[]},'info':{'version':1,'type':'CompleteDescription'},'tabs':[]}}
    (skin/'Plugin Skins/TUI.json').write_text(json.dumps(tui,indent=2)+'\n')
    licenses=t/'licenses';licenses.mkdir()
    for src,name in [(ROOT/'work/deps/libpd/LICENSE.txt','libpd.txt'),(ROOT/'work/deps/libpd/pure-data/LICENSE.txt','pure-data.txt'),(a.upstream/'LICENSE','sd88me.txt'),(ROOT/'ports/granular-freezer/PAN-GPL-2.0.txt','pan-GPL-2.0.txt'),(ROOT/'ports/granular-freezer/upstream/LICENSE.txt','Granular-Freezer-BSD.txt')]: shutil.copy2(src,licenses/name)
    shutil.copy2(ROOT/'ports/granular-freezer/NOTICE.md',licenses)
    subprocess.run([sys.executable,str(a.upstream.resolve()/'tools/release.py'),'--so',str(so.resolve()),'--skin',str(skin),'--entry',str((a.build/'build/pluginlist-entry.xml').resolve()),'--version','1.0.0','--id','organelle-granular-freezer','--repo','carballocba/MPC-PD-LAB','--about','Authentic C&G Granular Freezer; MPC hardware untested','--requires','44.1 kHz, one active instance; real MPC validation pending','--extra',str((a.build/'data').resolve())+':data','--extra',str(licenses)+':licenses','-o',str(t/'dist')],check=True)
    source=next((t/'dist').glob('*.zip'))
    with zipfile.ZipFile(source) as z: files={i.filename:(z.read(i),i.external_attr) for i in z.infolist()}
    top='Organelle-Granular-Freezer-1.0.0/'
    text,mode=files[top+'install.sh']
    anchor=b'[ -f "portable/$SKIN/plugin-meta.xml" ]'
    assert text.count(anchor)==1
    guard=(ROOT/'compat/organelle/fresh_install_guard.sh').read_bytes()
    files[top+'install.sh']=(text.replace(anchor,guard+b'\n'+anchor),mode)
    files[top+'INSTALL.md']=((ROOT/'docs/PHASE4_INSTALL.md').read_bytes(),0o100644<<16)
    for rel in ('mpc-plugin.json','portable/'+folder+'/mpc-plugin.json'):
        content,mode=files[top+rel];manifest=json.loads(content);manifest['os_compat']=['3.x']
        files[top+rel]=((json.dumps(manifest,indent=2)+'\n').encode(),mode)
    provenance={'upstream_patch':m['upstream'],'binary_sha256':hashlib.sha256(so.read_bytes()).hexdigest(),'hardware_tested':False,'source_hashes':json.loads((ROOT/'ports/granular-freezer/upstream.sha256.json').read_text())}
    files[top+'PROVENANCE.json']=((json.dumps(provenance,indent=2)+'\n').encode(),0o100644<<16)
    files[top+'SHA256SUMS']=(''.join(hashlib.sha256(data).hexdigest()+'  '+name[len(top):]+'\n' for name,(data,_) in sorted(files.items()) if name!=top+'SHA256SUMS').encode(),0o100644<<16)
    a.out.mkdir(parents=True,exist_ok=True);dest=a.out/source.name
    with zipfile.ZipFile(dest,'w',zipfile.ZIP_DEFLATED) as z:
        for name,(data,mode) in sorted(files.items()):
            info=zipfile.ZipInfo(name,(2026,1,1,0,0,0));info.create_system=3;info.external_attr=mode;info.compress_type=zipfile.ZIP_DEFLATED;z.writestr(info,data)
    subprocess.run([sys.executable,str(ROOT/'tests/test_organelle_package.py'),str(dest),'--upstream',str(a.upstream)],check=True)
    print('ZIP SHA256:',hashlib.sha256(dest.read_bytes()).hexdigest());print(dest)
