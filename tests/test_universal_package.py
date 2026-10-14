#!/usr/bin/env python3
"""Offline ZIP/source preservation checks. Does not execute an installer or access settings."""
from pathlib import Path
import argparse,json,hashlib,zipfile,sys,subprocess,xml.etree.ElementTree as ET
p=argparse.ArgumentParser();p.add_argument('zip',type=Path);p.add_argument('--upstream',required=True,type=Path);a=p.parse_args()
sys.path.insert(0,str(a.upstream.resolve()/'tools'));import catalog_check,shadow_skin
errors,warnings,record=catalog_check.check(str(a.zip),catalog=True,expect_id='mpc-pd-universal-loader',expect_repo='carballocba/MPC-PD-LAB')
assert not errors,errors
for warning in warnings:print('Upstream review note:',warning)
with zipfile.ZipFile(a.zip) as z:
 names=z.namelist();assert len(names)==len(set(names));assert not z.testzip()
 top=names[0].split('/')[0]+'/'
 assert all(n.startswith(top) and '..' not in n.split('/') and not n.startswith('/') for n in names)
 m=json.loads(z.read(top+'mpc-plugin.json'));assert m['id']=='mpc-pd-universal-loader' and m['uid']=='4d50554c' and m['arch']=='armv7' and m['kind']=='effect' and m['layout']=='portable'
 assert m['user_data']==['data/user'];base=top+m['folder']+'/'
 assert m==json.loads(z.read(base+'mpc-plugin.json'))
 plugin=ET.fromstring(z.read(base+'plugin-meta.xml'))
 for k,v in {'uid':'4d50554c','isInstrument':'0','category':'Effect','numInputs':'2','numOutputs':'2','manufacturer':'CARBALLO'}.items():assert plugin.get(k)==v,(k,plugin.get(k))
 assert plugin.get('file')=='%payload-path%/CARBALLO - VST - MPC PD Universal Loader/mpc_pd_universal_loader.so'
 assert json.loads(z.read(base+'Plugin Skins/TUI.json'))['pageData']['tabs'], 'GUI must have real native tabs'
 q=json.loads(z.read(base+'Plugin Skins/Q-Links.json'))['Program Mode Q-Links']
 assert sorted(v for v in q.values() if v>=0)==[0,1,2,3]
 for i in range(4):assert q['Q-Link '+str(shadow_skin.qlink_for_slot(i))]==i
 for patch in ['trim','lowpass']:
  for filename in ['main.pd','manifest.json','LICENSE.txt']:assert base+'data/user/patches/'+patch+'/'+filename in names
 for name in ['libpd.txt','pure-data.txt','wrapper.txt','jsmn.txt','universal.txt']:assert len(z.read(base+'licenses/'+name))>100
 assert not any('.loader-cache/' in name or name.endswith('/loader.log') for name in names)
 sums=z.read(top+'SHA256SUMS').decode().splitlines()
 assert len(sums)==len(names)-1
 for line in sums:
  digest,rel=line.split('  ',1);assert hashlib.sha256(z.read(top+rel)).hexdigest()==digest,rel
 binary=z.read(base+'mpc_pd_universal_loader.so');assert catalog_check.arm32(binary)
 assert hashlib.sha256(binary).hexdigest()==json.loads(z.read(top+'PROVENANCE.json'))['binary_sha256']
 assert not any('mpc_pd_delay.so' in n or 'organelle_granular_freezer.so' in n for n in names)
 script=z.read(top+'install.sh').decode();root=Path(__file__).resolve().parents[1];guard=(root/'packaging/universal/fresh_install_guard.sh').read_text()
 assert script.count(guard)==1 and script.index(guard)<script.index('mpc_ctl stop') and script.index(guard)<script.index('rm -rf "$STAGE"')
 assert '4d50554c' in guard and 'mpc_pd_universal_loader' in guard and 'MPDL' not in guard and 'CGFZ' not in guard
 # Syntax only; explicitly never run the install/uninstall scripts.
 for name in ['install.sh','uninstall.sh']:
  result=subprocess.run(['sh','-n'],input=z.read(top+name),capture_output=True);assert result.returncode==0,result.stderr
 print('PASS portable ARM effect ZIP, four Q-Links, IDs/assets/notices/checksums, fresh-only guard before mutation; installers NOT executed')
