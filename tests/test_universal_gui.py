#!/usr/bin/env python3
"""Validate the actual native TUI/assets, not a web UI or conceptual mockup."""
from pathlib import Path
import argparse,json,sys,hashlib
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('--upstream',type=Path,required=True);p.add_argument('--skin',type=Path,required=True);a=p.parse_args()
sys.path.insert(0,str(a.upstream.resolve()/'tools'));import skin_check
assert not skin_check.check(str(a.skin))
tui=json.loads((a.skin/'TUI.json').read_text());assert tui['pageData']['tabs']
params=set();buttons={};readouts=set()
for tab,kids,defs in skin_check.pages(tui):
 for c in kids:
  index=skin_check._param(c);cd=c['componentData'];definition=defs.get(cd['type'],{})
  if index is not None:
   assert 0<=index<9;params.add(index)
   actions=definition.get('actions',[])
   if actions:
    x,y,w,h=map(float,c['bounds']['bounds'].split());assert w>=44 and h>=44,(cd['name'],w,h)
   elif index>=4:readouts.add(index)
   for sub in definition.get('componentsData',[]):
    data=sub['componentData']
    if data['type']=='Button':buttons.setdefault(index,[]).append((cd['name'],data['data'],actions))
  for sub in [c,*definition.get('componentsData',[])]:
   data=sub['componentData']['data']
   for key in ('image','filmStrip','onImage','offImage'):
    name=data.get(key)
    if name:
     assert not Path(name).is_absolute() and '..' not in Path(name).parts
     with Image.open(a.skin/name) as im:assert 0<im.width<=12288 and 0<im.height<=12288;im.verify()
assert params==set(range(9));assert readouts==set(range(4,9))
assert sorted(b['buttonId'] for _,b,_ in buttons[0])==[0,0,1,1]
assert any('TRIM' in n for n,_,_ in buttons[0]) and any('LOWPASS' in n for n,_,_ in buttons[0])
assert any(n=='PREVIOUS' for n,_,_ in buttons[0]) and any(n=='NEXT' for n,_,_ in buttons[0])
assert any(act['onAction']=='Mouse Down' and act['handler']=='Toggle Switch' for _,_,actions in buttons[1] for act in actions)
for filename in ('Q-Links.json','Q-Links - 8by1.json'):
 q=json.loads((a.skin/filename).read_text());assert {k:v for k,v in q['Program Mode Q-Links'].items() if v>=0}=={'Q-Link 13':0,'Q-Link 9':1,'Q-Link 5':2,'Q-Link 1':3}
 assert q['Screen Mode Q-Links']['map']
root=Path(__file__).resolve().parents[1]
for path,digest in json.loads((root/'packaging/universal/gui/preserved-files.json').read_text()).items():assert hashlib.sha256((root/path).read_bytes()).hexdigest()==digest,path
print('PASS native TUI resources, 44px touch targets, patch/radio/Apply mappings, readonly telemetry, four Q-Links and preserved engine/legacy files')
