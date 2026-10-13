#!/usr/bin/env python3
"""Verify dependency revisions; host dependencies.sh also checks cleanliness."""
from pathlib import Path
import sys, shutil, subprocess
root = Path(sys.argv[1])
for name, pin in {
    'libpd': 'ba0dc63262901d658af8bbda5e619a60fa975e78',
    'libpd/pure-data': 'f009fd8d7b537e209e09898d487fdf1bf547da2b',
    'mpc-vst-plugins': '208745830fc2ed90b2fd0dc480ef5c554284292f',
}.items():
    dotgit = root / name / '.git'
    if dotgit.is_file():
        text = dotgit.read_text().strip()
        assert text.startswith('gitdir: '), text
        dotgit = dotgit.parent / text[8:]
    if shutil.which('git'):
        actual = subprocess.check_output(['git', '-C', str(root / name), 'rev-parse', 'HEAD'], text=True).strip()
        dirty = subprocess.check_output(['git', '-C', str(root / name), 'status', '--porcelain', '--untracked-files=no'], text=True)
        if dirty:
            sys.exit(f'{name}: tracked source changes are unsupported')
    else:
        actual = (dotgit / 'HEAD').read_text().strip()
    if actual != pin:
        sys.exit(f'{name}: expected pinned {pin}, got {actual}')
print('PASS dependency revision pins')
