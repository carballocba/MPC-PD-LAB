#!/usr/bin/env python3
"""Validate the ZIP offline on macOS or Linux. Requires pyelftools==0.33."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
import zipfile
from elftools.elf.elffile import ELFFile

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('zip', type=Path)
p.add_argument('--upstream', required=True, type=Path)
a = p.parse_args()
sys.path.insert(0, str(a.upstream.resolve() / 'tools'))
import catalog_check
errors, warnings, record = catalog_check.check(str(a.zip), expect_id='mpc-pd-delay', expect_repo='carballocba/MPC-PD-LAB')
assert not errors, errors
for w in warnings:
    print('Upstream review note:', w)
with zipfile.ZipFile(a.zip) as z:
    names = z.namelist()
    assert len(names) == len(set(names))
    top = 'MPC-PD-Delay-1.0.0/'
    assert all(n.startswith(top) and '..' not in n.split('/') for n in names)
    m = json.loads(z.read(top + 'mpc-plugin.json'))
    assert {k: m[k] for k in ('schema', 'id', 'uid', 'kind', 'arch', 'layout', 'skin', 'so')} == {
        'schema': 1, 'id': 'mpc-pd-delay', 'uid': '4d50444c', 'kind': 'effect', 'arch': 'armv7',
        'layout': 'portable', 'skin': 'CARBALLO - VST - MPC PD Delay', 'so': 'mpc_pd_delay.so'}
    base = top + m['folder'] + '/'
    assert json.loads(z.read(base + 'mpc-plugin.json')) == m
    meta = ET.fromstring(z.read(base + 'plugin-meta.xml'))
    for k, v in {'uid': '4d50444c', 'format': 'VST', 'category': 'Effect', 'isInstrument': '0',
                 'numInputs': '2', 'numOutputs': '2', 'manufacturer': 'CARBALLO',
                 'file': '%payload-path%/' + m['skin'] + '/' + m['so']}.items():
        assert meta.get(k) == v, (k, meta.get(k))
    ET.fromstring(z.read(base + 'version.xml'))
    assert json.loads(z.read(base + 'Plugin Skins/TUI.json'))['pageData']['tabs'] == []
    root = Path(__file__).resolve().parents[1]
    for patch in ('libpd_host.pd', 'stereo_delay_smoke.pd'):
        assert z.read(base + 'data/' + patch) == (root / 'patches' / patch).read_bytes()
    for name in ('libpd.txt', 'pure-data.txt', 'wrapper.txt'):
        assert len(z.read(base + 'licenses/' + name)) > 100
    data = z.read(base + m['so'])
    assert hashlib.sha256(data).hexdigest() == '76a7340039def8494f9d78acb4ad23ac75f8b02b889557f47eb84dc26930e0df'
    elf = ELFFile(io.BytesIO(data))
    assert elf.elfclass == 32 and elf.little_endian
    assert elf['e_machine'] == 'EM_ARM' and elf['e_type'] == 'ET_DYN'
    assert elf['e_flags'] & 0xff000000 == 0x05000000
    assert elf['e_flags'] & 0x600 == 0x400  # hard-float, no soft-float flag
    attrs = {attr.tag: attr.value for sub in elf.get_section_by_name('.ARM.attributes').iter_subsections()
             for ss in sub.iter_subsubsections() for attr in ss.iter_attributes()}
    assert attrs['TAG_CPU_ARCH'] == 10 and attrs['TAG_ABI_VFP_ARGS'] == 1
    dynamic = list(elf.get_section_by_name('.dynamic').iter_tags())
    needed = {t.needed for t in dynamic if t.entry.d_tag == 'DT_NEEDED'}
    assert needed == {'libm.so.6', 'libdl.so.2', 'libpthread.so.0', 'libc.so.6'}, needed
    assert not any(t.entry.d_tag in ('DT_RPATH', 'DT_RUNPATH', 'DT_TEXTREL') for t in dynamic)
    exports = [s.name for s in elf.get_section_by_name('.dynsym').iter_symbols()
               if s['st_info']['bind'] == 'STB_GLOBAL' and s['st_shndx'] != 'SHN_UNDEF']
    assert exports == ['VSTPluginMain'], exports
    with tempfile.TemporaryDirectory() as td:
        so = Path(td) / m['so']
        so.write_bytes(data)
        assert catalog_check.max_glibc(str(so)) == m['max_glibc'] == '2.29'
        # Reuse the Phase 2 readelf validator wherever the tool is available.
        if shutil.which('readelf') or shutil.which('arm-linux-gnueabihf-readelf'):
            subprocess.run([sys.executable, str(root / 'scripts/verify_elf.py'), str(so)], check=True, stdout=subprocess.DEVNULL)
    assert b'MPC-PD-LAB\0' in data  # embedded vendor intentionally preserved
print('PASS: ZIP, upstream catalog rules, checksums, metadata, MPDL identity, runtime assets and licenses')
print('PASS: ARMv7 ELF32 EABI5 hard-float, VSTPluginMain, exact dependencies, glibc 2.29, no runtime paths/text relocations')
print('Binary SHA256:', hashlib.sha256(data).hexdigest())
print('ZIP SHA256:', hashlib.sha256(a.zip.read_bytes()).hexdigest())
