import copy
import importlib.util
import json
from pathlib import Path
import shutil
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('library', ROOT/'tools/pdlab_offline/library.py')
lib = importlib.util.module_from_spec(spec); spec.loader.exec_module(lib)

class LibraryTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.base = Path(self.tmp.name).resolve()
        self.root = self.base/'library'
        shutil.copytree(ROOT/'library-v03', self.root)
        self.patch = self.root/'patches/org.pdlab.original-gain-mix/1'
        self.approvals = self.base/'approvals.json'
        shutil.copyfile(ROOT/'docs/library-v03/fixture-approvals.json',self.approvals)
    def tearDown(self): self.tmp.cleanup()
    def manifest(self): return lib.loads((self.patch/'manifest.json').read_bytes())
    def save(self,m): (self.patch/'manifest.json').write_bytes(lib.canonical(m))
    def reject(self):
        with self.assertRaises(lib.Invalid): lib.validate_patch(self.patch)
    def rehash(self):
        m=self.manifest()
        prov=lib.loads((self.patch/'PROVENANCE.json').read_bytes())
        prov['sourceSha256']=lib.sha((self.patch/'main.pd').read_bytes())
        (self.patch/'PROVENANCE.json').write_bytes(lib.canonical(prov))
        for r in m['files']:
            data=(self.patch/r['path']).read_bytes();r.update(sha256=lib.sha(data),bytes=len(data))
        m['source']['sha256']=lib.sha((self.patch/'main.pd').read_bytes());self.save(m)
    def catalog(self, previous=None):return lib.build_catalog(self.root,self.approvals,previous,ROOT/'docs/library-v03/revocations.json' if previous is None else None)
    def test_examples(self):
        self.assertEqual(self.catalog(),lib.loads((ROOT/'docs/library-v03/example-catalog.json').read_bytes()))
    def test_new_dsp_independent(self):
        import importlib.util
        spec=importlib.util.spec_from_file_location('creator',ROOT/'tools/pdlab_offline/create_original_fixtures.py')
        import sys
        sys.path.insert(0,str(ROOT/'tools/pdlab_offline'))
        creator=importlib.util.module_from_spec(spec);spec.loader.exec_module(creator)
        for suffix,old in [('gain-mix','trim'),('lowpass','lowpass')]:
            data=(self.root/f'patches/org.pdlab.original-{suffix}/1/main.pd').read_bytes()
            self.assertNotEqual((ROOT/f'patches/universal/{old}/main.pd').read_bytes(),data)
            self.assertEqual(data,creator.graph(suffix=='lowpass'))

    def test_deterministic(self): self.assertEqual(lib.canonical(self.catalog()),lib.canonical(self.catalog()))
    def test_catalog_tampering(self):
        c=self.catalog();c['patches'][0]['treeSha256']='0'*64
        self.assertNotEqual(c,self.catalog())
    def test_duplicate_json(self):
        with self.assertRaises(lib.Invalid):lib.loads(b'{"id":1,"id":2}')
    def test_nonfinite(self):
        for v in [b'NaN',b'Infinity',b'1e999']:
            with self.assertRaises(lib.Invalid):
                x=lib.loads(v);lib.schema_check(x,{'type':'number'})
    def test_unknown_field(self): m=self.manifest();m['trusted']=True;self.save(m);self.reject()
    def test_boolean_integer(self): m=self.manifest();m['mappingVersion']=True;self.save(m);self.reject()
    def test_bad_ids(self):
        for id in ['../a','A','a/b','a\x00','', 'a'*65]:
            m=self.manifest();m['id']=id;self.save(m);self.reject()
    def test_bad_revision(self):m=self.manifest();m['revision']='../1';self.save(m);self.reject()
    def test_wrong_hash(self):m=self.manifest();m['files'][0]['sha256']='0'*64;self.save(m);self.reject()
    def test_missing_file(self):(self.patch/'main.pd').unlink();self.reject()
    def test_extra_file(self):(self.patch/'evil.py').write_text('pass');self.reject()
    def test_corrupt_manifest(self):(self.patch/'manifest.json').write_bytes(b'{');self.reject()
    def test_corrupt_pd(self):(self.patch/'main.pd').write_bytes(b'garbage;');self.rehash();self.reject()
    def test_unknown_object(self):
        p=self.patch/'main.pd';p.write_text(p.read_text().replace('inlet~','shell',1));self.rehash();self.reject()
    def test_receiver(self):
        p=self.patch/'main.pd';p.write_text(p.read_text().replace('r ul-gain','r pd'));self.rehash();self.reject()
    def test_missing_dependency(self):m=self.manifest();m['dependencies']['vanillaObjects'].remove('r');self.save(m);self.reject()
    def test_externals(self):m=self.manifest();m['dependencies']['externals']=['foo'];self.save(m);self.reject()
    def test_assets(self):m=self.manifest();m['assets']=['sample.wav'];self.save(m);self.reject()
    def test_paths(self):
        for value in ['../x','/tmp/x','a//b','a/./b','a/../b','a\\b','a/']:
            with self.assertRaises(lib.Invalid):lib.relative(value)
    def test_symlink(self):(self.patch/'link').symlink_to('/etc/passwd');self.reject()
    def test_symlink_root(self):
        p=self.base/'link';p.symlink_to(self.patch,target_is_directory=True)
        with self.assertRaises(lib.Invalid):lib.validate_patch(p)
    def test_hardlink(self):
        import os
        os.link(self.patch/'main.pd',self.patch/'copy.pd');self.reject()
    def test_executable(self):(self.patch/'main.pd').chmod(0o755);self.reject()
    def test_quota(self):(self.patch/'main.pd').write_bytes(b'x'*131073);self.reject()
    def test_license_empty(self):(self.patch/'LICENSE.txt').write_bytes(b'');self.rehash();self.reject()
    def test_license_reference(self):m=self.manifest();m['files'][0]['licenseRef']='unknown';self.save(m);self.reject()
    def test_control_contract(self):m=self.manifest();m['parameters'][0]['slot']=1;self.save(m);self.reject()
    def test_page_contract(self):m=self.manifest();m['pages'][0]['controlIDs']=['mix','gain'];self.save(m);self.reject()
    def test_unapproved(self):
        self.approvals.write_bytes(lib.canonical({'schemaVersion':1,'approvals':[]}))
        with self.assertRaises(lib.Invalid):self.catalog()
    def test_mutated_revision(self):
        previous=self.base/'previous.json';previous.write_bytes(lib.canonical(self.catalog()))
        p=self.patch/'main.pd';p.write_text(p.read_text().replace('pack f 20','pack f 21'));self.rehash()
        r,_=lib.validate_patch(self.patch)
        a=lib.loads(self.approvals.read_bytes())
        for x in a['approvals']:
            if x['id']==r['id']:x['treeSha256']=r['treeSha256']
        self.approvals.write_bytes(lib.canonical(a))
        with self.assertRaisesRegex(lib.Invalid,'immutable'):self.catalog(previous)
    def test_new_revision_preserves_old(self):
        previous=self.base/'previous.json';previous.write_bytes(lib.canonical(self.catalog()))
        dest=self.patch.parent/'2';shutil.copytree(self.patch,dest)
        m=lib.loads((dest/'manifest.json').read_bytes());m['revision']='2';(dest/'manifest.json').write_bytes(lib.canonical(m))
        r,_=lib.validate_patch(dest);a=lib.loads(self.approvals.read_bytes());a['approvals'].append(dict(a['approvals'][0],id=r['id'],revision='2',treeSha256=r['treeSha256']));self.approvals.write_bytes(lib.canonical(a))
        self.assertEqual(len(self.catalog(previous)['patches']),3)
    def test_duplicate_identity(self):
        shutil.copytree(self.patch,self.root/'patches/duplicate/1')
        with self.assertRaises(lib.Invalid):self.catalog()
    def test_catalog_order_independent(self):
        a=lib.loads(self.approvals.read_bytes());a['approvals'].reverse();self.approvals.write_bytes(lib.canonical(a));self.test_examples()
    def test_import_no_overwrite(self):
        root=self.base/'imported';root.mkdir()
        lib.import_patch(self.patch,root,self.approvals)
        with self.assertRaises(lib.Invalid):lib.import_patch(self.patch,root,self.approvals)
        self.assertEqual(lib.validate_patch(self.patch)[0],lib.validate_patch(root/'patches/org.pdlab.original-gain-mix/1')[0])
    def test_import_unapproved_no_files(self):
        root=self.base/'imported';root.mkdir();self.approvals.write_bytes(lib.canonical({'schemaVersion':1,'approvals':[]}))
        with self.assertRaises(lib.Invalid):lib.import_patch(self.patch,root,self.approvals)
        self.assertEqual(list(root.iterdir()),[])
    def test_schema_keywords(self):
        allowed={'$schema','$id','title','type','properties','required','additionalProperties','const','enum','items','minItems','maxItems','uniqueItems','minLength','maxLength','pattern','minimum','maximum'}
        def walk(s):
            self.assertFalse(set(s)-allowed)
            for v in s.get('properties',{}).values():walk(v)
            if 'items' in s:walk(s['items'])
        walk(lib.loads((lib.BASE/'manifest.schema.json').read_bytes()))
    def test_cycles(self):
        (self.patch/'main.pd').write_bytes(b'#N canvas 0 0 1 1 10; #X obj 0 0 loop;')
        (self.patch/'loop.pd').write_bytes(b'#N canvas 0 0 1 1 10; #X obj 0 0 loop;')
        m=self.manifest();m['dependencies']['abstractions']=['loop.pd'];m['dependencies']['vanillaObjects']=[]
        m['files'].append({'path':'loop.pd','bytes':1,'sha256':'0'*64,'licenseRef':'inherited-mit'})
        self.save(m);self.rehash();self.reject()
    def test_provenance(self):
        p=self.patch/'PROVENANCE.json';x=lib.loads(p.read_bytes());x['sourceCommit']='wrong';p.write_bytes(lib.canonical(x));self.rehash();self.reject()
    def test_profile_not_expanded(self):
        c=(ROOT/'src/universal/manifest.c').read_text()
        import re
        allowed=re.search(r'const char \*allowed\[\]=\{([^}]+)',c).group(1)
        self.assertEqual(set(re.findall(r'"([^" ]+)"',allowed)),lib.ALLOWED)

    def test_cli_catalog_and_check(self):
        import subprocess,sys
        dest=self.base/'catalog.json'
        args=[sys.executable,str(lib.BASE/'library.py')]
        tail=[str(self.root),'--approvals',str(self.approvals),'--catalog',str(dest)]
        self.assertEqual(subprocess.run(args+['catalog']+tail,capture_output=True).returncode,0)
        self.assertEqual(subprocess.run(args+['check']+tail,capture_output=True).returncode,0)
        before=dest.read_bytes()
        self.assertNotEqual(subprocess.run(args+['catalog']+tail,capture_output=True).returncode,0)
        self.assertEqual(before,dest.read_bytes())
        doc=lib.loads(before);doc['patches'][0]['revision']='9';dest.write_bytes(lib.canonical(doc))
        self.assertNotEqual(subprocess.run(args+['check']+tail,capture_output=True).returncode,0)
    def test_no_license_grant(self):
        m=self.manifest();self.assertEqual(m['licenses'][0]['spdx'],'NOASSERTION')
        c=self.catalog();self.assertFalse(c['runtimeApproved']);self.assertFalse(c['redistributionApproved'])
    def test_mit_claim_rejected(self):
        m=self.manifest();m['licenses'][0]['spdx']='MIT';self.save(m);self.reject()
    def test_quarantine_rejected(self):
        old=ROOT/'quarantine/stage1-fixtures/library-v03/patches/org.pdlab.stereo-trim/1'
        with self.assertRaises(lib.Invalid):lib.validate_patch(old)
    def test_old_approvals_rejected(self):
        with self.assertRaises(lib.Invalid):lib.approval_records(ROOT/'quarantine/stage1-fixtures/historical-approvals-NOT-VALID.json')
    def test_explicit_revocation(self):
        prev=ROOT/'quarantine/stage1-fixtures/historical-catalog-NOT-VALID.json'
        with self.assertRaises(lib.Invalid):lib.build_catalog(self.root,self.approvals,prev)
        cat=lib.build_catalog(self.root,self.approvals,prev,ROOT/'docs/library-v03/revocations.json')
        self.assertEqual(len(cat['revoked']),2)
    def test_wrong_revocation_hash(self):
        rev=lib.loads((ROOT/'docs/library-v03/revocations.json').read_bytes());rev['revoked'][0]['treeSha256']='0'*64
        path=self.base/'revoked.json';path.write_bytes(lib.canonical(rev))
        with self.assertRaises(lib.Invalid):lib.build_catalog(self.root,self.approvals,ROOT/'quarantine/stage1-fixtures/historical-catalog-NOT-VALID.json',path)
    def test_quarantine_bytes_preserved(self):
        import subprocess
        old=ROOT/'quarantine/stage1-fixtures/library-v03'
        for p in old.rglob('*'):
            if p.is_file():
                original=subprocess.check_output(['git','-C',str(ROOT),'show','c565aaf:library-v03/'+p.relative_to(old).as_posix()])
                self.assertEqual(p.read_bytes(),original)
    def test_provenance_generator(self):
        p=lib.loads((self.patch/'PROVENANCE.json').read_bytes())
        self.assertEqual(p['creation']['generatorSha256'],lib.sha((ROOT/'tools/pdlab_offline/create_original_fixtures.py').read_bytes()))
        self.assertFalse(p['priorPatchCodeCopied'])
        self.assertEqual(p['creation']['rightsHolder'],'not established')
    def test_connection_range(self):
        with self.assertRaises(lib.Invalid):lib.audit_pd(b'#N canvas 0 0 1 1 10; #X obj 0 0 inlet~; #X connect 99 0 0 0;',set())

if __name__=='__main__':unittest.main()
