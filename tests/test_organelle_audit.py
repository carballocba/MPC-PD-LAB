#!/usr/bin/env python3
import hashlib
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from prepare_organelle import audit,PORT
class Audit(unittest.TestCase):
    def test_original(self):self.assertEqual(audit()['tier'],'B')
    def test_missing_asset(self):
        with tempfile.TemporaryDirectory() as t:
            d=Path(t)/'port';shutil.copytree(PORT,d);(d/'upstream/grain.pd').unlink()
            with self.assertRaises(AssertionError):audit(d)
    def test_changed_source(self):
        with tempfile.TemporaryDirectory() as t:
            d=Path(t)/'port';shutil.copytree(PORT,d);(d/'upstream/grain.pd').write_text('changed')
            with self.assertRaises(AssertionError):audit(d)
    def test_unknown_external_even_if_rehashed(self):
        with tempfile.TemporaryDirectory() as t:
            d=Path(t)/'port';shutil.copytree(PORT,d);p=d/'upstream/grain.pd';p.write_text(p.read_text()+'#X obj 0 0 missing_external;\n')
            f=d/'upstream.sha256.json';h=json.loads(f.read_text());h['grain.pd']=hashlib.sha256(p.read_bytes()).hexdigest();f.write_text(json.dumps(h))
            with self.assertRaisesRegex(AssertionError,'missing_external'):audit(d)
if __name__=='__main__':unittest.main()
