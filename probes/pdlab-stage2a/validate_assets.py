"""Validate generated native structures and render explicitly illustrative mock text."""
from pathlib import Path
import argparse,json,re,sys
from PIL import Image,ImageDraw,ImageFont
p=argparse.ArgumentParser();p.add_argument('--upstream',type=Path,required=True);p.add_argument('--out',type=Path,required=True);a=p.parse_args()
sys.path.insert(0,str(a.upstream.resolve()/'tools'));import skin_check,studio,shadow_skin
src=Path(__file__).resolve().parent;out=a.out.resolve()
params=json.loads((src/'params.json').read_text());assert len(params)==27 and len({p['key'] for p in params})==27
skin=out/'CARBALLO - VST - PD LAB UI Probe'/'Plugin Skins'
assert not skin_check.check(str(skin))
tui=json.loads((skin/'TUI.json').read_text())['pageData'];defs={d['key']:d['value'] for d in tui['componentDefinitions']['localComponentDefinitions']}
q=json.loads((skin/'Q-Links.json').read_text());q8=json.loads((skin/'Q-Links - 8by1.json').read_text());assert q==q8
maps=q['Screen Mode Q-Links']['map'];assert len(maps)==2
assert {v for v in maps[0]['Q-Links'].values() if v>=0}=={0,1,2,3}
assert set(maps[1]['Q-Links'].values())==set(range(10,26))
assert maps[1]['Q-Links']==q['Program Mode Q-Links']
assert maps[1]['Q-Links']=={'Q-Link '+str(shadow_skin.qlink_for_slot(i)):10+i for i in range(16)}
assert any('IndexedEnabling/1/2/Parameter 0' in str(c) for t in tui['tabs'] for c in studio.tab_components(t,defs))
results={'layout':'PASS','qlink_maps':'PASS','parameter_count':27,'image_checks':0,'interactive_regions':0,'readonly_regions':0,'mock_previews':'illustrative only; not a native host renderer'}
for path in skin.glob('*.png'):
 with Image.open(path) as im:
  im.verify()
 with Image.open(path) as im:assert im.width<=16384 and im.height<=16384
 results['image_checks']+=1
font=ImageFont.truetype(str(a.upstream.resolve()/'tools/html_art/fonts/TitilliumWeb-SemiBold.ttf'),22)
for mode,title,asset in [(0,'Library','native-assets-0.png'),(1,'Performance','native-assets-0_mode0-1.png')]:
 state={0:mode};components=studio.tab_components(tui['tabs'][0],defs)
 visible=[c for c in components if studio.shown(c,state)]
 for c in visible:
  cd=c['componentData'];bounds=list(map(int,c['bounds']['bounds'].split()))
  if cd['type']=='Image':continue
  definition=defs[cd['type']];interactive=bool(definition.get('actions')) or any(s['componentData']['type'] in ('Knob','Button') for s in definition['componentsData'])
  if interactive:
   assert bounds[2]>=64 and bounds[3]>=64,(cd,bounds)
   results['interactive_regions']+=1
  else:results['readonly_regions']+=1
  for m in c.get('handle remapping',{}).get('map',[]):
   match=re.fullmatch(r'Parameter (\d+)',m.get('value',''))
   if match:assert int(match[1])<27
 row_components=[c for c in visible if c.get('handle remapping',{}).get('map') and any(x['value'] in ['Parameter '+str(i) for i in range(6,10)] for x in c['handle remapping']['map'])]
 assert len(row_components)==(4 if mode==0 else 0)
 # Values are explicit mock illustration, not proof of live redraw.
 im=Image.open(out/asset).convert('RGB');canvas=Image.new('RGB',(1280,672),'#101217');canvas.paste(im,(0,0));draw=ImageDraw.Draw(canvas)
 values={0:'Library' if mode==0 else 'Perform',1:'0.000',2:'0.000',3:'Ready',4:'Ready',5:'M000R0',6:'Trim',7:'Lowpass',8:'Mock 3',9:'Mock 4',26:'Trim',**{i:'0.000' for i in range(10,26)}}
 for c in visible:
  cd=c['componentData'];x,y,_,_=map(int,c['bounds']['bounds'].split())
  if cd['type']=='Image':continue
  mapping={m['key']:m['value'] for m in c.get('handle remapping',{}).get('map',[])}
  for sub in defs[cd['type']]['componentsData']:
   data=sub['componentData']
   if data['type']!='Label' or data['data'].get('type')!='Value':continue
   target=mapping.get(data['data'].get('handleName','Data'),'')
   m=re.fullmatch(r'Parameter (\d+)',target)
   if not m:continue
   sx,sy,w,h=map(int,sub['bounds']['bounds'].split());draw.text((x+sx+w/2,y+sy+h/2),values[int(m[1])],anchor='mm',font=font,fill='white')
 draw.text((640,650),'RENDERED MOCK DATA / NOT MPC HARDWARE',anchor='mm',font=font,fill='#ff8890')
 canvas.save(out/('review-'+title.lower()+'.png'))
(out/'asset-validation.json').write_text(json.dumps(results,indent=2)+'\n');print(json.dumps(results))
