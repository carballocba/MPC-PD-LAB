#!/usr/bin/env python3
"""Build only Universal in isolated output trees; never install or touch other plugins."""
from pathlib import Path
import argparse,os,shutil,subprocess,sys,json
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('mode',choices=['native','armv7']);p.add_argument('--sanitizer',choices=['address','thread']);a=p.parse_args()
if a.mode=='armv7' and a.sanitizer:sys.exit('Native sanitizers only; ARM/QEMU is a separate correctness run')
deps=ROOT/'work/deps';subprocess.run([sys.executable,str(ROOT/'scripts/check_pins.py'),str(deps)],check=True)
suffix='-'+a.sanitizer if a.sanitizer else '';out=ROOT/'work'/('universal-'+a.mode+suffix);out.mkdir(parents=True,exist_ok=True)
cc='arm-linux-gnueabihf-gcc' if a.mode=='armv7' else os.environ.get('CC','cc')
flags=['-DPDTHREADS=0','-D_FILE_OFFSET_BITS=64','-fPIC','-fvisibility=hidden']
if a.mode=='armv7':flags+=['-march=armv7-a','-mfpu=vfpv3-d16','-mfloat-abi=hard']
san=['-fsanitize='+('address,undefined' if a.sanitizer=='address' else 'thread'),'-fno-omit-frame-pointer','-g'] if a.sanitizer else []
if sys.platform=='darwin':san=[flag for flag in san if flag!='-g']
# Loader-only libpd archive: MULTI=false alone still leaves PDTHREADS=1 in Pd.
# Explicitly disable internal mutexes; the engine enforces exclusive ownership.
libpd=out/'libpd'
key=json.dumps({'flags':flags+san,'cc':cc,'pd_no_threads_fix':2},sort_keys=True)
stamp=out/'libpd-build-config.json'
if libpd.exists() and (not stamp.exists() or stamp.read_text()!=key):shutil.rmtree(libpd)
if not libpd.exists():
 shutil.copytree(deps/'libpd',libpd,ignore=shutil.ignore_patterns('.git','*.o','libs'))
 # Minimal repairs in Pd's otherwise unused no-thread branch; never patch deps.
 source=libpd/'pure-data/src/s_inter.c';text=source.read_text()
 for old,new in [('#if PDTHREADS\n#include "pthread.h"\n#endif','#include "pthread.h"'),('void pd_queue_mess(struct _pdinstance instance,','void pd_queue_mess(struct _pdinstance *instance,')]:
  assert text.count(old)==1,'Pinned no-thread repair anchor changed';text=text.replace(old,new)
 source.write_text(text)
 source=libpd/'pure-data/src/g_io.c';text=source.read_text()
 for end in ['t_inlet *vinlet_getit','t_outlet *voutlet_getit']:
  old='    }\n}\n\n'+end
  new='    }\n    if (x->x_rb) freebytes(x->x_rb, x->x_nchans * sizeof(*x->x_rb));\n}\n\n'+end
  assert text.count(old)==1,'Pinned reblocker cleanup anchor changed';text=text.replace(old,new)
 source.write_text(text)
(libpd/'libs').mkdir(exist_ok=True)
if not (libpd/'libs/libpd.a').exists():
 subprocess.run(['make','-C',str(libpd),'-j2','CC='+cc,'AR='+('arm-linux-gnueabihf-ar' if a.mode=='armv7' else 'ar'),'STATIC=false','UTIL=false','EXTRA=false','MULTI=false','SETLOCALE=false','ADDITIONAL_CFLAGS='+' '.join(flags+san),'ADDITIONAL_LDFLAGS='+' '.join(san),'libpd'],check=True)
 subprocess.run([('arm-linux-gnueabihf-ar' if a.mode=='armv7' else 'ar'),'rcs',str(libpd/'libs/libpd.a'),*map(str,sorted(libpd.rglob('*.o')))],check=True)
 stamp.write_text(key)
archive=libpd/'libs/libpd.a'
incs=['-I'+str(ROOT/'src/universal'),'-I'+str(libpd/'libpd_wrapper'),'-I'+str(libpd/'pure-data/src'),'-I'+str(deps/'mpc-vst-plugins/wrapper')]
common=[cc,'-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',*flags,*san,*incs]
sources=[ROOT/'src/universal/engine.c',ROOT/'src/universal/manifest.c']
libs=[str(archive),'-lm','-lpthread']+([] if sys.platform=='darwin' else ['-ldl'])
pkg=out/'package'/'CARBALLO - VST - MPC PD Universal Loader'
if pkg.exists():shutil.rmtree(pkg)
(pkg/'data/user/patches').mkdir(parents=True);(pkg/'data/defaults').mkdir(parents=True);(pkg/'licenses').mkdir()
for folder in ['trim','lowpass']:shutil.copytree(ROOT/'patches/universal'/folder,pkg/'data/user/patches'/folder)
for target in ['user','defaults']:shutil.copy2(ROOT/'patches/universal/catalog.json',pkg/'data'/target/'catalog.json')
for source,name in [(deps/'libpd/LICENSE.txt','libpd.txt'),(deps/'libpd/pure-data/LICENSE.txt','pure-data.txt'),(deps/'mpc-vst-plugins/LICENSE','wrapper.txt'),(ROOT/'vendor/universal/LICENSE','jsmn.txt'),(ROOT/'src/universal/LICENSE','universal.txt'),(ROOT/'vendor/universal/pd-no-threads.patch','pd-no-threads.patch')]:shutil.copy2(source,pkg/'licenses'/name)
shared=['-dynamiclib'] if sys.platform=='darwin' else ['-shared','-Wl,--no-undefined,-Bsymbolic,--exclude-libs,ALL,-z,nodelete']
so=pkg/'mpc_pd_universal_loader.so'
link=[*common,*map(str,sources),str(ROOT/'src/universal/vst2_wrap.c'),*libs,*shared,'-o',str(so)]
subprocess.run(link,check=True)
if a.mode=='armv7':
 first=so.read_bytes();subprocess.run(link,check=True);assert so.read_bytes()==first,'Repeated ARM link differs'
 print('PASS repeated Universal ARM link byte-identical')
run=['qemu-arm','-L','/usr/arm-linux-gnueabihf'] if a.mode=='armv7' else []
for name in ['universal_engine_test','universal_manifest_test','universal_vst_test']:
 test=ROOT/'tests'/(name+'.c')
 if not test.exists():continue
 extra=[*map(str,sources),*libs] if name=='universal_engine_test' else [str(ROOT/'src/universal/manifest.c'),'-lm'] if name=='universal_manifest_test' else ([] if sys.platform=='darwin' else ['-ldl'])+['-lm','-lpthread']
 binary=out/name
 if name=='universal_engine_test' and sys.platform!='darwin' and not a.sanitizer:
  extra += [str(ROOT/'tests/universal_rt_audit.c'),'-DUL_RT_AUDIT','-Wl,'+','.join('--wrap='+symbol for symbol in ['malloc','calloc','realloc','free','open','fopen','open64','fopen64','pthread_mutex_lock','pthread_join','read','write','close','nanosleep'])]
 subprocess.run([*common,'-DUL_TESTING',str(test),*extra,'-o',str(binary)],check=True)
 arg=str(so) if name=='universal_vst_test' else str(pkg/'data')
 subprocess.run([*run,str(binary),arg],check=True)
if a.mode=='armv7':subprocess.run([sys.executable,str(ROOT/'scripts/verify_elf.py'),str(so)],check=True)
print('PASS Universal '+a.mode+suffix+' build; '+str(so))
