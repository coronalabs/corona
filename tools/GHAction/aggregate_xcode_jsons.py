#!/usr/bin/env python
import json
import sys
import os
import subprocess

output_dir = sys.argv[1]
files = sys.argv[2:]
files = [f for f in files if f.endswith('.json') and os.path.basename(f).count('_') == 1]
files.sort(key=os.path.basename)

out = {}
def processFile(f):
    with open(f) as fp:
        obj = json.load(fp)
    out_file = os.path.basename(f).split('_')[1] # type: str
    out[out_file] = out.get(out_file, {})
    for platform, entries in obj.items():
        out[out_file][platform] = out[out_file].get(platform, [])
        out[out_file][platform].extend(entries)
    
map(processFile, files)

subprocess.check_call(['/bin/mkdir', '-p', output_dir])
for out_file, content in out.items():
    with open(os.path.join(output_dir, out_file), 'w') as fp:
        json.dump(content, fp, indent=4, sort_keys=True)
