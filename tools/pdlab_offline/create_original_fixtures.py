"""New PD LAB fixture graphs, composed in this task from equations, not old patches.

No old Pd files are read. Uses only the already supported Vanilla objects.
Copyright holder/license grant not asserted. See docs/original-fixtures.
"""
from pathlib import Path
import argparse
import library as lib

DESIGN = 'original-design-2026-10-16-v1'

def graph(lowpass=False):
    nodes, edges = [], []
    def obj(x,y,text):
        nodes.append(f'#X obj {x} {y} {text};');return len(nodes)-1
    def link(a,out,b,port):edges.append(f'#X connect {a} {out} {b} {port};')
    # Desired contract: y = dry + mix * (gain * processed - dry).
    left=obj(40,35,'inlet~');right=obj(440,35,'inlet~')
    gain=obj(800,35,'r ul-gain');gpack=obj(800,80,'pack f 20');g=obj(800,125,'line~')
    mix=obj(1010,35,'r ul-mix');mpack=obj(1010,80,'pack f 20');m=obj(1010,125,'line~')
    link(gain,0,gpack,0);link(gpack,0,g,0);link(mix,0,mpack,0);link(mpack,0,m,0)
    for x,dry in [(40,left),(440,right)]:
        processed=dry
        if lowpass:
            processed=obj(x,100,'lop~ 900');link(dry,0,processed,0)
        wet=obj(x,175,'*~');difference=obj(x,250,'-~');blend=obj(x,325,'*~');summed=obj(x,400,'+~');output=obj(x,475,'outlet~')
        link(processed,0,wet,0);link(g,0,wet,1)
        link(wet,0,difference,0);link(dry,0,difference,1)
        link(difference,0,blend,0);link(m,0,blend,1)
        link(dry,0,summed,0);link(blend,0,summed,1);link(summed,0,output,0)
    return ('#N canvas 10 10 1200 560 10;\n'+'\n'.join(nodes+edges)+'\n').encode('ascii')

STATUS = b'''LICENSE STATUS: NO GRANT ESTABLISHED
These new Pd fixtures were generated specifically for PD LAB in the task recorded
in PROVENANCE.json. No copyright holder is asserted by this file.
The proposed MIT template is in docs/original-fixtures/MIT-DRAFT.txt.
That template is NOT an effective grant for these files. Publication under MIT
requires documented authorization from the appropriate rights holder(s).
Offline development/test admission is not runtime or redistribution approval.
'''

def create(root):
    root=Path(root)
    for lowpass,suffix,name,category in [(False,'gain-mix','Stereo Gain/Mix','Utility'),(True,'lowpass','Stereo Lowpass','Filter')]:
        id='org.pdlab.original-'+suffix
        dest=root/'patches'/id/'1';dest.mkdir(parents=True,exist_ok=False)
        data=graph(lowpass);(dest/'main.pd').write_bytes(data);(dest/'LICENSE.txt').write_bytes(STATUS)
        sourcepath='tools/pdlab_offline/create_original_fixtures.py'
        provenance={'sourceCommit':DESIGN,'sourcePath':sourcepath,'sourceSha256':lib.sha(data),
          'creation':{'date':'2026-10-16','agent':'OpenAI Codex AI assistant, at user direction in this task','method':'new graph composed from stated equations; no previous patch file read by generator','generatorSha256':lib.sha(Path(__file__).read_bytes()),'humanAuthorship':'not established','rightsHolder':'not established'},
          'design':{'equation':'dry + mix * (gain * processed - dry)','processed':'lop~ 900 per channel' if lowpass else 'dry','controls':'ul-gain and ul-mix, 20 ms internal ramps'},
          'externalComponents':[{'name':'Pure Data Vanilla built-in objects','use':'runtime object references only; no Pd C source copied into graph','license':'Pd runtime BSD notices remain with runtime'}, {'name':'libpd','use':'existing test/runtime host, not embedded into this fixture','license':'existing BSD notices retained separately'}],
          'licenseStatus':'pending-rightsholder-authorization','priorPatchCodeCopied':False}
        (dest/'PROVENANCE.json').write_bytes(lib.canonical(provenance))
        m={'schemaVersion':2,'id':id,'revision':'1','name':name,'author':'AI-generated for PD LAB; human authorship and rights holder unconfirmed','category':category,'classification':'Native',
          'source':{'url':'https://github.com/carballocba/MPC-PD-LAB','revision':DESIGN,'path':sourcepath,'sha256':lib.sha(data)},
          'runtime':{'profile':lib.PROFILE,'entry':'main.pd','audio':{'kind':'effect','inputs':2,'outputs':2,'sampleRate':44100},'midi':{'input':False,'output':False}},'mappingVersion':1,
          'parameters':[{'id':key,'slot':i,'receiver':'ul-'+key,'type':'continuous','min':0,'max':1,'default':1,'curve':'linear','unit':'ratio','smoothingMs':0,'automation':True} for i,key in enumerate(['gain','mix'])],
          'pages':[{'id':'main','title':'Main','controlIDs':['gain','mix']}],
          'dependencies':{'vanillaObjects':sorted(lib.audit_pd(data,{'main'})[0]),'abstractions':[],'externals':[]},'assets':[],
          'licenses':[{'id':'pending-rights','spdx':'NOASSERTION','file':'LICENSE.txt','provenanceStatus':'pending-rightsholder-authorization'}],
          'files':[{'path':p.name,'sha256':lib.sha(p.read_bytes()),'bytes':len(p.read_bytes()),'licenseRef':'pending-rights'} for p in sorted(dest.iterdir())],
          'state':{'policy':'controls-only'}}
        (dest/'manifest.json').write_bytes(lib.canonical(m))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('root');create(p.parse_args().root)
