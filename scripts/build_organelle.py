#!/usr/bin/env python3
"""Build/test the audited Organelle port; baseline delay build is independent."""
from pathlib import Path
import argparse
import os
import shutil
import subprocess
import sys
import json

ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('mode',choices=['native','desktop','armv7'])
a=p.parse_args()
mode=a.mode
out=ROOT/'work'/('organelle-'+mode)
out.mkdir(parents=True,exist_ok=True)
deps=ROOT/'work/deps'
subprocess.run([sys.executable,str(ROOT/'scripts/check_pins.py'),str(deps)],check=True)
subprocess.run([sys.executable,str(ROOT/'scripts/prepare_organelle.py'),str(out)],check=True)
cc='arm-linux-gnueabihf-gcc' if mode=='armv7' else os.environ.get('CC','cc')
flags=['-march=armv7-a','-mfpu=vfpv3-d16','-mfloat-abi=hard'] if mode=='armv7' else []
flags+=['-D_FILE_OFFSET_BITS=64']
base=ROOT/'work'/mode
if mode=='native':
    base=out
    libpd=base/'libpd'
    if not libpd.exists(): shutil.copytree(deps/'libpd',libpd,ignore=shutil.ignore_patterns('.git','*.o'))
    subprocess.run(['make','-C',str(libpd),'-j2','CC='+cc,'UTIL=false','EXTRA=false','MULTI=false','SETLOCALE=false','STATIC=true','ADDITIONAL_CFLAGS=-fvisibility=hidden -D_FILE_OFFSET_BITS=64','libpd'],check=True)
    shutil.copy2(libpd/'libs/libpd.a',base/'libpd.a')
libpd=base/'libpd'
assert (base/'libpd.a').is_file(), 'Run baseline scripts/build.sh '+mode+' first'
up=deps/'mpc-vst-plugins'
subprocess.run([sys.executable,str(up/'tools/gen_vst.py'),str(out/'vst.json')],check=True)
incs=['-I'+str(up/'wrapper'),'-I'+str(libpd/'libpd_wrapper'),'-I'+str(libpd/'pure-data/src'),'-I'+str(out),'-I'+str(out/'build')]
common=[cc,'-O2','-Wall','-Wextra','-Wno-unused-parameter','-std=gnu11',*flags,*incs]
engine=str(ROOT/'src/organelle_engine.c')
libs=[str(base/'libpd.a'),'-lm','-ldl','-lpthread']
if sys.platform=='darwin': shared=['-dynamiclib']; run=[]
else:
    shared=['-shared','-Wl,--no-undefined,-Bsymbolic,--exclude-libs,ALL']
    run=['qemu-arm','-L','/usr/arm-linux-gnueabihf'] if mode=='armv7' else []
so=out/'organelle_granular_freezer.so'
compile_plugin=[*common,'-fPIC','-fvisibility=hidden',engine,str(up/'wrapper/vst2_wrap.c'),*libs,*shared,'-o',str(so)]
subprocess.run(compile_plugin,check=True)
if mode=='armv7':
    first=so.read_bytes()
    subprocess.run(compile_plugin,check=True)
    assert so.read_bytes()==first, 'Repeated ARM plugin link differs'
    print('PASS repeated ARM plugin compile/link byte-identical (same pinned archive/toolchain)')
for name in ('organelle_engine_test','organelle_vst_test'):
    sources=[str(ROOT/'tests'/ (name+'.c'))]
    if name=='organelle_engine_test': sources.insert(0,engine)
    subprocess.run([*common,'-Werror',*sources,*libs,'-o',str(out/name)],check=True)
    arg=str(out/'data') if name=='organelle_engine_test' else str(so)
    subprocess.run([*run,str(out/name),arg],check=True)
# Each required runtime file missing must fail load. Unknown object creation
# must fail even though libpd_openfile itself normally returns a canvas.
required=json.loads((out/'data/manifest.json').read_text())['required_files']
for name in required:
    path=out/'data'/name
    saved=path.with_name(path.name+'.saved')
    path.rename(saved)
    try: subprocess.run([*run,str(out/'organelle_vst_test'),str(so),'--expect-failure'],check=True)
    finally: saved.rename(path)
host=out/'data/host.pd'
old=host.read_bytes()
try:
    host.write_bytes(old+b'#X obj 20 120 phase4_missing_external;\n')
    subprocess.run([*run,str(out/'organelle_vst_test'),str(so),'--expect-failure'],check=True)
finally: host.write_bytes(old)
if mode=='armv7': subprocess.run([sys.executable,str(ROOT/'scripts/verify_elf.py'),str(so)],check=True)
print('PASS '+mode+' Organelle engine/VST tests, missing-file/external failures')
