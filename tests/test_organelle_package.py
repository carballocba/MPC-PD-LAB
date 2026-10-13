#!/usr/bin/env python3
"""Validate portable release and run its installers on local fixtures only."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
import zipfile
from elftools.elf.elffile import ELFFile
import io

ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('zip',type=Path)
p.add_argument('--upstream',type=Path,required=True)
a=p.parse_args()
sys.path.insert(0,str(a.upstream.resolve()/'tools'))
import catalog_check
errors,warnings,_=catalog_check.check(str(a.zip),expect_id='organelle-granular-freezer',expect_repo='carballocba/MPC-PD-LAB')
assert not errors,errors
for warning in warnings: print('Upstream review note:',warning)
with zipfile.ZipFile(a.zip) as z:
    names=z.namelist();assert len(names)==len(set(names))
    top='Organelle-Granular-Freezer-1.0.0/'
    assert all(n.startswith(top) for n in names)
    m=json.loads(z.read(top+'mpc-plugin.json'))
    assert m['uid']=='4347465a' and m['uid']!='4d50444c' and m['kind']=='effect' and m['arch']=='armv7'
    assert m['skin']=='CARBALLO - VST - Organelle Granular Freezer' and m['layout']=='portable'
    base=top+m['folder']+'/'
    meta=ET.fromstring(z.read(base+'plugin-meta.xml'))
    assert meta.get('uid')==m['uid'] and meta.get('isInstrument')=='0' and meta.get('numInputs')=='2'
    assert meta.get('file')=='%payload-path%/'+m['skin']+'/'+m['so']
    assert json.loads(z.read(base+'mpc-plugin.json'))==m
    data=z.read(base+m['so']);e=ELFFile(io.BytesIO(data))
    assert e.elfclass==32 and e.little_endian and e['e_machine']=='EM_ARM' and e['e_type']=='ET_DYN'
    assert e['e_flags']&0xff000600==0x05000400
    attrs={v.tag:v.value for sub in e.get_section_by_name('.ARM.attributes').iter_subsections() for ss in sub.iter_subsubsections() for v in ss.iter_attributes()}
    assert attrs['TAG_CPU_ARCH']==10 and attrs['TAG_ABI_VFP_ARGS']==1
    exports=[s.name for s in e.get_section_by_name('.dynsym').iter_symbols() if s['st_info']['bind']=='STB_GLOBAL' and s['st_shndx']!='SHN_UNDEF']
    assert exports==['VSTPluginMain'],exports
    needed={t.needed for t in e.get_section_by_name('.dynamic').iter_tags() if t.entry.d_tag=='DT_NEEDED'}
    assert needed=={'libc.so.6','libm.so.6','libdl.so.2','libpthread.so.0'},needed
    port=ROOT/'ports/granular-freezer'
    hashes=json.loads((port/'upstream.sha256.json').read_text())
    for name,digest in hashes.items():assert hashlib.sha256(z.read(base+'data/upstream/'+name)).hexdigest()==digest
    assert z.read(base+'data/mpc_organelle_mother.pd')==(ROOT/'compat/organelle/mpc_organelle_mother.pd').read_bytes()
    for name in ('libpd.txt','pure-data.txt','sd88me.txt','pan-GPL-2.0.txt','Granular-Freezer-BSD.txt','NOTICE.md'):assert len(z.read(base+'licenses/'+name))>100
    with tempfile.TemporaryDirectory() as tmp:
        t=Path(tmp);z.extractall(t/'release');pkg=t/'release'/top
        so=t/'plugin.so';so.write_bytes(data)
        assert m['max_glibc']==catalog_check.max_glibc(str(so))
        if shutil.which('arm-linux-gnueabihf-readelf') or shutil.which('readelf'):
            subprocess.run([sys.executable,str(ROOT/'scripts/verify_elf.py'),str(so)],check=True,stdout=subprocess.DEVNULL)
        storage=t/'storage/Synths';storage.mkdir(parents=True)
        other=storage/'CARBALLO - VST - MPC PD Delay';other.mkdir();(other/'mpc_pd_delay.so').write_bytes(b'delay sentinel')
        settings=t/'MPC.settings';log=t/'services.log'
        original='''<PROPERTIES>
<VALUE name="SynthContentLocations"><SynthContentLocations><Location>ROOT</Location></SynthContentLocations></VALUE>
<VALUE name="other" val="keep"/>
<VALUE name="pluginList-arm">
<KNOWNPLUGINS>
<PLUGIN name="MPC PD Delay" file="/unrelated/mpc_pd_delay.so" uid="4d50444c"/>
</KNOWNPLUGINS>
</VALUE>
</PROPERTIES>
'''.replace('ROOT',str(storage))
        settings.write_text(original)
        env=dict(os.environ,MPC_INSTALL_TEST='1',MPC_SETTINGS=str(settings),MPC_TEST_LOG=str(log),MPC_LEGACY_ROOT=str(t/'legacy'))
        bindir=t/'bin';bindir.mkdir();sha=bindir/'sha256sum'
        sha.write_text('#!'+sys.executable+'\n'+'''import sys,hashlib
from pathlib import Path
assert sys.argv[1:]==['-c','SHA256SUMS']
for line in Path('SHA256SUMS').read_text().splitlines():
 d,n=line.split('  ',1)
 if hashlib.sha256(Path(n).read_bytes()).hexdigest()!=d: sys.exit(1)
''');sha.chmod(0o755);env['PATH']=str(bindir)+os.pathsep+env['PATH']
        def snap():return {str(f.relative_to(t)):f.read_bytes() for f in t.rglob('*') if f.is_file()}
        def run(name,ok=True):
            result=subprocess.run(['sh',str(pkg/name),'-y','-t',str(storage)],env=env,text=True,capture_output=True)
            assert (result.returncode==0)==ok,result.stdout+result.stderr
            assert (other/'mpc_pd_delay.so').read_bytes()==b'delay sentinel'
        # Different plugin using this UID must not be replaced.
        settings.write_text(original.replace('4d50444c','4347465A'));before=snap();run('install.sh',False);assert snap()==before
        settings.write_text(original)
        target=storage/m['skin'];target.mkdir();(target/'user').write_text('preserve');before=snap();run('install.sh',False);assert snap()==before;shutil.rmtree(target)
        patch=pkg/'portable'/m['skin']/'data/upstream/main.pd';saved=patch.read_bytes();patch.write_bytes(saved+b'broken');before=snap();run('install.sh',False);assert snap()==before;patch.write_bytes(saved)
        run('install.sh');plugins=ET.parse(settings).findall('.//PLUGIN');assert len(plugins)==2
        assert {x.get('uid') for x in plugins}=={'4d50444c','4347465a'}
        assert any(f.read_text()==original for f in t.glob('MPC.settings.bak-*'))
        assert ET.parse(settings).find('.//VALUE[@name="other"]').get('val')=='keep'
        before=snap();run('install.sh',False);assert snap()==before
        run('uninstall.sh');assert not target.exists()
        assert [x.get('uid') for x in ET.parse(settings).findall('.//PLUGIN')]==['4d50444c']
        assert log.read_text().splitlines()==['stop','start','stop','start']
        if shutil.which('go'):
            shutil.copy2(a.upstream/'tools/desktop/pkg.go',t/'pkg.go')
            (t/'pkg_test.go').write_text('''package main
import("os";"testing")
func TestPortZIP(t *testing.T){p,e:=OpenPackage(os.Getenv("PORT_ZIP"),"upload");if e!=nil{t.Fatal(e)};if !p.Defer||len(p.Plugins)!=1||p.Plugins[0].UID!="4347465a"||p.Plugins[0].Kind!="effect"{t.Fatal(p)}}
''')
            subprocess.run(['go','test',str(t/'pkg.go'),str(t/'pkg_test.go')],env=dict(os.environ,PORT_ZIP=str(a.zip.resolve())),check=True)
        else:print('SKIP actual desktop OpenPackage: Go unavailable')
print('PASS ARM ELF, source identity, separate UID, runtime/license assets, ZIP integrity, upstream OpenPackage and installer isolation')
print('PASS install/uninstall preserves MPC PD Delay, settings backup, UID/folder/repeat/corrupt-payload refusal')
