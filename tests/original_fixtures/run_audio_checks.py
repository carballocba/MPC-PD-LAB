"""Compile a standalone harness against an existing libpd archive; do not rebuild runtime."""
from pathlib import Path
import argparse,subprocess,sys
p=argparse.ArgumentParser();p.add_argument('--libpd',type=Path,required=True);p.add_argument('--out',type=Path,required=True);a=p.parse_args()
repo=Path(__file__).resolve().parents[2];pd=a.libpd.resolve();out=a.out.resolve();assert not out.exists();out.mkdir(parents=True)
# This small host is exclusively test glue outside the admitted patch tree.
(out/'fixture-host.pd').write_text('#N canvas 0 0 500 400 10;\n#X obj 20 20 adc~ 1 2;\n#X obj 20 100 main;\n#X obj 20 180 dac~ 1 2;\n#X connect 0 0 1 0;\n#X connect 0 1 1 1;\n#X connect 1 0 2 0;\n#X connect 1 1 2 1;\n')
exe=out/'audio_check'
subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-I'+str(pd/'libpd_wrapper'),'-I'+str(pd/'pure-data/src'),str(repo/'tests/original_fixtures/audio_check.c'),str(pd/'libs/libpd.a'),'-lm','-lpthread',*([] if sys.platform=='darwin' else ['-ldl']),'-o',str(exe)],check=True)
with (out/'RESULTS.txt').open('w') as log:
 for suffix,flag in [('gain-mix','0'),('lowpass','1')]:
  r=subprocess.run([str(exe),str(repo/f'library-v03/patches/org.pdlab.original-{suffix}/1'),str(out),flag],check=True,capture_output=True,text=True);log.write(r.stdout+r.stderr);print(r.stdout,end='')
