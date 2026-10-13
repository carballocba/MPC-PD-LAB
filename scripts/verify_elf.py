#!/usr/bin/env python3
import re, subprocess, sys, shutil
path = sys.argv[1]
def read(*args):
    return subprocess.check_output([shutil.which('arm-linux-gnueabihf-readelf') or 'readelf', *args, path], text=True)
h, a, d, s, v = (read('-h'), read('-A'), read('-d'), read('--dyn-syms', '-W'), read('-V'))
assert re.search(r'Class:\s+ELF32', h), h
assert re.search(r'Data:.*little endian', h), h
assert re.search(r'Machine:\s+ARM', h), h
assert re.search(r'Type:\s+DYN', h), h
assert 'Version5 EABI' in h and 'hard-float ABI' in h, h
assert re.search(r'Tag_CPU_arch:\s+v7\b', a), a
assert 'Tag_ABI_VFP_args: VFP registers' in a, a
assert re.search(r'GLOBAL\s+DEFAULT\s+\d+\s+VSTPluginMain\b', s), s
needed = re.findall(r'NEEDED.*\[(.*?)\]', d)
assert set(needed) <= {'libm.so.6','libc.so.6','libdl.so.2','libpthread.so.0','libgcc_s.so.1'}, needed
assert not any('libpd' in n for n in needed), needed
exports = re.findall(r'GLOBAL\s+DEFAULT\s+\d+\s+(\S+)', s)
assert exports == ['VSTPluginMain'], exports
versions = sorted(set(re.findall(r'GLIBC_(\d+\.\d+)', v)), key=lambda x: tuple(map(int,x.split('.'))))
assert versions and tuple(map(int, versions[-1].split('.'))) <= (2,31), versions
assert not re.search(r'RPATH|RUNPATH|TEXTREL', d), d
print(h + a + d)
print('PASS: ARMv7 ELF32 EABI5 hard-float; VSTPluginMain; dependencies:', needed)
print('Required glibc versions:', versions)
