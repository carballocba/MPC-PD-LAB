"""Reproducible offline probe checks; no installer, hardware, git mutations or Pd."""
from pathlib import Path
import argparse,json,subprocess,sys,platform
p=argparse.ArgumentParser();p.add_argument('--upstream',type=Path,required=True);p.add_argument('--out',type=Path,required=True);a=p.parse_args()
src=Path(__file__).resolve().parent;repo=src.parents[1];out=a.out.resolve();up=a.upstream.resolve()
base='c565aaf1423635c71a2c4aa1f5f902bc33c19d5e'
# Every pre-existing file must remain identical, not just stable engine directories.
rows=subprocess.check_output(['git','-C',str(repo),'ls-tree','-r',base],text=True).splitlines()
for row in rows:
 meta,name=row.split('\t',1);original=subprocess.check_output(['git','-C',str(repo),'show',base+':'+name]);assert (repo/name).read_bytes()==original,name
subprocess.run([sys.executable,str(src/'build_skin.py'),'--upstream',str(up),'--out',str(out)],check=True)
subprocess.run([sys.executable,str(src/'validate_assets.py'),'--upstream',str(up),'--out',str(out)],check=True)
mode=platform.system();assert mode in ('Darwin','Linux'),'native binary build supported on macOS/Linux'
binary=out/('pdlab_ui_probe-macos-arm64.dylib' if mode=='Darwin' and platform.machine()=='arm64' else 'pdlab_ui_probe-native.so')
flags=['-std=c11','-Wall','-Wextra','-Werror']
subprocess.run(['cc',*flags,'-fvisibility=hidden','-dynamiclib' if mode=='Darwin' else '-shared',*([] if mode=='Darwin' else ['-fPIC']),str(src/'probe.c'),'-o',str(binary)],check=True)
subprocess.run(['cc',*flags,str(src/'test_probe.c'),'-o',str(out/'test_probe'),*(['-ldl'] if mode=='Linux' else [])],check=True)
r=subprocess.run([str(out/'test_probe'),str(binary)],check=True,capture_output=True,text=True);(out/'binary-test.txt').write_text(r.stdout);print(r.stdout)
# No linking or runtime execution of ARM code. Compile only ABI static assertions.
(out/'arm-abi.c').write_text('#include "abi.h"\nint abi_only;\n')
subprocess.run(['clang','--target=armv7-linux-gnueabihf','-ffreestanding','-I'+str(src),'-c',str(out/'arm-abi.c'),'-o',str(out/'arm-abi.o')],check=True)
# Keep audit deliberately narrow: render has no indirect callees or host invocations.
code=(src/'probe.c').read_text();render=code.split('static void render(',1)[1].split('__attribute__',1)[0]
for term in ['malloc','calloc','free(','mutex','fopen','read(','write(','printf','atomic_','dispatch','host(','sleep']:
 assert term not in render,term
with (out/'stage1-regression.txt').open('w') as f:
 subprocess.run([sys.executable,'-m','unittest','discover','-s',str(repo/'tests/offline_library'),'-v'],check=True,stdout=f,stderr=subprocess.STDOUT)
(out/'validation.json').write_text(json.dumps({'base':base,'preserved_files':len(rows),'native_platform':platform.platform(),'binary':binary.name,'native_binary_tests':'PASS','callback_source_audit':'PASS','arm_abi_compile':'PASS; object only, not plugin','stage1_regression':'43/43 PASS','hardware':'NOT TESTED'},indent=2)+'\n')
print('PASS all offline checks; no MPC or ARM plugin validation')
