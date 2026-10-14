#!/usr/bin/env python3
"""Generate a portable installer ZIP using pinned community tools. NEVER install it."""
from pathlib import Path
import argparse,json,subprocess,sys,shutil,tempfile,zipfile,hashlib
ROOT=Path(__file__).resolve().parents[1];PIN='5e3a96885b6de76a48b498821c379568a05cc006'
p=argparse.ArgumentParser();p.add_argument('--upstream',required=True,type=Path);p.add_argument('--build',type=Path,default=ROOT/'work/universal-gui-armv7');p.add_argument('--out',type=Path,default=ROOT/'work/universal-gui-dist');a=p.parse_args()
up=a.upstream.resolve();assert subprocess.check_output(['git','-C',str(up),'rev-parse','HEAD'],text=True).strip()==PIN
assert not subprocess.check_output(['git','-C',str(up),'status','--porcelain','--untracked-files=no'],text=True).strip()
folder='CARBALLO - VST - MPC PD Universal Loader';pkg=a.build/'package'/folder;so=pkg/'mpc_pd_universal_loader.so'
subprocess.run([sys.executable,str(ROOT/'scripts/verify_elf.py'),str(so)],check=True,stdout=subprocess.DEVNULL)
sys.path.insert(0,str(up/'tools'));import gen_vst,shadow_skin
cfg=json.loads((ROOT/'packaging/universal/vst.json').read_text())
with tempfile.TemporaryDirectory() as temp:
 t=Path(temp)
 subprocess.run([sys.executable,str(ROOT/'scripts/build_universal_gui.py'),'--upstream',str(up),'--out',str(t)],check=True)
 skin=t/folder
 shutil.copy2(up/'tools/html_art/fonts/OFL.txt',pkg/'licenses/gui-font-OFL.txt')
 shutil.copy2(up/'tools/vendor/force-shadow/LICENSE',pkg/'licenses/gui-renderer.txt')
 entry=t/'entry.xml';entry.write_text(gen_vst.entry(cfg)+'\n')
 data=t/'data';shutil.copytree(pkg/'data',data,ignore=shutil.ignore_patterns('.loader-cache','loader.log','loader.log.previous'))
 subprocess.run([sys.executable,str(up/'tools/release.py'),'--so',str(so.resolve()),'--skin',str(skin),'--entry',str(entry),'--version','0.2.0','--id','mpc-pd-universal-loader','--repo','carballocba/MPC-PD-LAB','--license','MIT','--about','Experimental two-folder Vanilla Pd loader; stopped switching and four controls','--requires','Original MPC One/Hakai 3.9.1 hardware UNTESTED; 44.1 kHz, 128 frames, one instance; no arbitrary externals','--user-data','data/user','--extra',str(data)+':data','--extra',str((pkg/'licenses').resolve())+':licenses','-o',str(t/'dist')],check=True)
 source=next((t/'dist').glob('*.zip'))
 with zipfile.ZipFile(source) as z:files={i.filename:(z.read(i),i.external_attr) for i in z.infolist()}
 top=source.stem.removesuffix('-mpc-armv7')+'/'
 # release.py uses the human-readable name/version, not its catalog id, for top.
 top=next(name.split('/')[0]+'/' for name in files)
 script,mode=files[top+'install.sh'];anchor=b'[ -f "portable/$SKIN/plugin-meta.xml" ]'
 assert script.count(anchor)==1;guard=(ROOT/'packaging/universal/fresh_install_guard.sh').read_bytes()
 files[top+'install.sh']=(script.replace(anchor,guard+b'\n'+anchor),mode)
 files[top+'INSTALL.md']=((ROOT/'docs/UNIVERSAL_LOADER_GUI_HARDWARE_CHECKLIST.md').read_bytes(),0o100644<<16)
 provenance={'gui_version':'0.2.0','plugin_uid':'MPUL','hardware_tested':False,'binary_sha256':hashlib.sha256(so.read_bytes()).hexdigest(),'source_commit':subprocess.check_output(['git','-C',str(ROOT),'rev-parse','HEAD'],text=True).strip(),'libpd':'ba0dc63262901d658af8bbda5e619a60fa975e78','pd':'f009fd8d7b537e209e09898d487fdf1bf547da2b','runtime_wrapper_base':'208745830fc2ed90b2fd0dc480ef5c554284292f','packaging_tools':PIN,'jsmn':'25647e692c7906b96ffd2b05ca54c097948e879c','pd_threads':0,'elf_nodelete':True,'pd_local_patch_sha256':hashlib.sha256((ROOT/'vendor/universal/pd-no-threads.patch').read_bytes()).hexdigest()}
 files[top+'PROVENANCE.json']=((json.dumps(provenance,indent=2)+'\n').encode(),0o100644<<16)
 for rel in ['mpc-plugin.json','portable/'+folder+'/mpc-plugin.json']:
  data,mode=files[top+rel];m=json.loads(data);m['os_compat']=['3.x'];files[top+rel]=((json.dumps(m,indent=2)+'\n').encode(),mode)
 files[top+'SHA256SUMS']=(''.join(hashlib.sha256(data).hexdigest()+'  '+name[len(top):]+'\n' for name,(data,_) in sorted(files.items()) if name!=top+'SHA256SUMS').encode(),0o100644<<16)
 a.out.mkdir(parents=True,exist_ok=True);shutil.copy2(t/'native-assets-0.png',a.out/'GUI-native-assets.png');dest=a.out/source.name
 with zipfile.ZipFile(dest,'w',zipfile.ZIP_DEFLATED) as z:
  for name,(data,mode) in sorted(files.items()):
   info=zipfile.ZipInfo(name,(2026,1,1,0,0,0));info.create_system=3;info.external_attr=mode;info.compress_type=zipfile.ZIP_DEFLATED;z.writestr(info,data)
 subprocess.run([sys.executable,str(ROOT/'tests/test_universal_package.py'),str(dest),'--upstream',str(up)],check=True)
 print('ZIP SHA256',hashlib.sha256(dest.read_bytes()).hexdigest());print(dest)
