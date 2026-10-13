#!/usr/bin/env python3
"""Offline authoring checks for the curated v0.1 Vanilla subset; never starts Pd."""
from pathlib import Path
import json,re,argparse,hashlib
ALLOWED=set('inlet~ outlet~ r pack line~ *~ +~ -~ sig~ lop~'.split())
def unique(pairs):
 d={}
 for k,v in pairs:
  if k in d:raise ValueError('duplicate key: '+k)
  d[k]=v
 return d

def audit(root):
 catalog=json.loads((root/'catalog.json').read_text(),object_pairs_hook=unique)
 assert set(catalog)=={'schema','patches'} and catalog['schema']==1 and len(catalog['patches'])==2
 ids=set();result={}
 for item in catalog['patches']:
  assert set(item)=={'id','folder'} and re.fullmatch('[a-zA-Z0-9_-]{1,63}',item['id']) and item['id'] not in ids;ids.add(item['id'])
  parts=Path(item['folder']).parts;assert parts and not Path(item['folder']).is_absolute() and all(p not in ('.','..') for p in parts)
  directory=root/item['folder'];assert directory.is_dir() and not directory.is_symlink()
  m=json.loads((directory/'manifest.json').read_text(),object_pairs_hook=unique)
  assert m['id']==item['id'] and m['schema']==1 and m['revision'] and m['entry']=='main.pd' and m['sample_rate']==44100 and m['inputs']==m['outputs']==2 and m['externals']==[]
  files=m['required_files'];assert 1<=len(files)<=64 and len(set(files))==len(files) and 'main.pd' in files
  assert sorted(m['controls'],key=lambda c:c['slot'])==[{'slot':i,'key':key,'receiver':'ul-'+key,'min':0,'max':1,'default':1} for i,key in enumerate(['gain','mix'])]
  abstracts={f[:-3] for f in files};edges={f[:-3]:set() for f in files};total=0;hashes={}
  for filename in files:
   assert re.fullmatch('[a-zA-Z0-9_.-]+\.pd',filename) and filename!='host.pd'
   path=directory/filename;assert path.is_file() and not path.is_symlink();b=path.read_bytes();total+=len(b)
   assert 0<len(b)<=131072 and b'\0' not in b
   hashes[filename]=hashlib.sha256(b).hexdigest()
   for record in re.split(r'(?<!\\);',b.decode('ascii')):
    record=record.strip()
    if not record:continue
    if record.startswith('#X obj '):
     words=record.split();obj=words[4].rstrip(',');assert obj in ALLOWED|abstracts,(filename,obj)
     if obj in abstracts:edges[filename[:-3]].add(obj)
     if obj=='r':assert words[5] in ('ul-gain','ul-mix')
    else:assert record.startswith(('#N canvas ','#X text ','#X connect ')),record
  assert total<=1048576
  done=set()
  def visit(name,stack):
   assert name not in stack,'recursive abstraction'
   if name in done:return
   for child in edges[name]:visit(child,stack|{name})
   done.add(name)
  for name in edges:visit(name,set())
  result[item['id']]={'revision':m['revision'],'source_sha256':hashes}
 return result
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('user_root',type=Path);a=p.parse_args();print(json.dumps(audit(a.user_root),indent=2));print('PASS curated Vanilla folder audit (not an untrusted-code sandbox)')
