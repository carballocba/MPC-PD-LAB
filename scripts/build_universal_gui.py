#!/usr/bin/env python3
"""Build real native MPC skin resources using pinned community tools. No installer."""
from pathlib import Path
import argparse,subprocess,sys,json,shutil,os
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[1];PIN='5e3a96885b6de76a48b498821c379568a05cc006'
p=argparse.ArgumentParser();p.add_argument('--upstream',type=Path,required=True);p.add_argument('--out',type=Path,default=ROOT/'work/universal-gui-skin');a=p.parse_args();up=a.upstream.resolve()
assert subprocess.check_output(['git','-C',str(up),'rev-parse','HEAD'],text=True).strip()==PIN
sys.path.insert(0,str(up/'tools'));import shadow_skin,skin_check,studio
out=a.out.resolve();out.mkdir(parents=True,exist_ok=True);art=out/'shadow_art'
# Reuse force-shadow's real-font label renderer for free-standing static text.
source=(up/'tools/shadow_art.c').read_text();old='draw_text_c(atoi(a[1]), atoi(a[2]), a[5]';assert source.count(old)==1
source=source.replace(old,'label_text_c(atoi(a[1]), atoi(a[2]), a[5]');adapted=out/'shadow_art_ttf.c';adapted.write_text(source)
subprocess.run([os.environ.get('CC','cc'),'-O2','-I'+str(up/'tools/vendor/force-shadow/tools'),'-o',str(art),str(adapted),'-lm'],check=True)
params=json.loads((ROOT/'packaging/universal/params.json').read_text());layout=ROOT/'packaging/universal/gui/layout.conf'
font=up/'tools/html_art/fonts/TitilliumWeb-SemiBold.ttf'
os.environ['SHADOW_TITLE_FONT']=str(font)
configured=out/'layout.conf';configured.write_text('font_label='+str(font)+'\n'+layout.read_text())
layout=configured
skin=Path(shadow_skin.write_skin(str(out),'CARBALLO','MPC PD Universal Loader',str(layout),params,str(art),mpc_os=3))
# Two direct radio-group targets share Patch index 0. Separate asset names avoid
# the upstream generator's duplicate-key enum image collision.
skin_dir=skin/'Plugin Skins';tui=json.loads((skin_dir/'TUI.json').read_text())
page=tui['pageData']['tabs'][0].get('componentDefinition')
if page is None:
 key=tui['pageData']['tabs'][0]['componentName'];page=next(d['value'] for d in tui['pageData']['componentDefinitions']['localComponentDefinitions'] if d['key']==key)
for i,label in enumerate(['PREVIOUS','NEXT']):
 key='ulNavigation'+str(i);w,h=212,56
 for state,fill in [('off','#242b36'),('on','#e34d59')]:
  image=Image.new('RGB',(w,h),fill);draw=ImageDraw.Draw(image);draw.text((w/2,h/2),label,anchor='mm',font=ImageFont.truetype(str(font),24),fill='white');image.save(skin_dir/(key+'_'+state+'.png'))
 definition=shadow_skin._local(key,[shadow_skin._action('Mouse Down','Q-Link')],[shadow_skin._button(key+'_on.png',key+'_off.png',i,2,w,h)])
 tui['pageData']['componentDefinitions']['localComponentDefinitions'].append(definition)
 page['componentsData'].append(shadow_skin._placed(key,label,0,97+i*214,416-28,w,h))
(skin_dir/'TUI.json').write_text(json.dumps(tui,indent=2)+'\n')
findings=skin_check.check(str(skin/'Plugin Skins'));assert not findings,findings
# TUI native live values are intentionally blank in this upstream asset preview.
studio.preview(str(skin/'Plugin Skins'),str(out/'native-assets-%d.png'))
shutil.rmtree(skin/'Plugin Skins/.art',ignore_errors=True)
(skin/'version.xml').write_text('<plugincontent version="1.0"><identifier>CARBALLO.vst.mpcpduniversalloader</identifier><version>0.2.0.0</version></plugincontent>\n')
print('PASS native skin generator and touch/edge/Q-Link checks:',skin)
