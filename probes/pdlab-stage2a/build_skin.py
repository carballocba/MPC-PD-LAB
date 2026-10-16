"""Isolated Stage 2A native asset builder; never installs or modifies stable assets."""
from pathlib import Path
import argparse,os,json,subprocess,sys
p=argparse.ArgumentParser();p.add_argument('--upstream',type=Path,required=True);p.add_argument('--out',type=Path,required=True);a=p.parse_args()
up=a.upstream.resolve();out=a.out.resolve();src=Path(__file__).resolve().parent
assert subprocess.check_output(['git','-C',str(up),'rev-parse','HEAD'],text=True).strip()=='5e3a96885b6de76a48b498821c379568a05cc006'
assert not out.exists(),'fresh output required'
out.mkdir(parents=True)
sys.path.insert(0,str(up/'tools'));import shadow_skin,skin_check,studio
code=(up/'tools/shadow_art.c').read_text();old='draw_text_c(atoi(a[1]), atoi(a[2]), a[5]';assert code.count(old)==1
code=code.replace(old,'label_text_c(atoi(a[1]), atoi(a[2]), a[5]')
(out/'renderer.c').write_text(code)
subprocess.run(['cc','-O2','-I'+str(up/'tools/vendor/force-shadow/tools'),str(out/'renderer.c'),'-o',str(out/'renderer'),'-lm'],check=True)
font=up/'tools/html_art/fonts/TitilliumWeb-SemiBold.ttf';os.environ['SHADOW_TITLE_FONT']=str(font)
layout=out/'layout.conf';layout.write_text('font_label='+str(font)+'\n'+(src/'layout.conf').read_text())
params=json.loads((src/'params.json').read_text())
skin=Path(shadow_skin.write_skin(str(out),'CARBALLO','PD LAB UI Probe',str(layout),params,str(out/'renderer'),mpc_os=3))
findings=skin_check.check(str(skin/'Plugin Skins'));assert not findings,findings
studio.preview(str(skin/'Plugin Skins'),str(out/'native-assets-%d.png'),frame=0)
(out/'build.json').write_text(json.dumps({'skin':str(skin.relative_to(out)),'upstream':'5e3a96885b6de76a48b498821c379568a05cc006','findings':findings},indent=2))
print('PASS: native generator, touch/edge/Q-Link checks',skin)
