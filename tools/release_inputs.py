"""Shared build provenance: only tracked, generic firmware inputs are eligible."""
from pathlib import Path
import hashlib,json,re,subprocess
INPUT_ROOTS=('firmware/release/','web/setup-release/')
INPUT_FILES={'requirements.txt','tools/build_portal_assets.py'}
EXCLUDED={'.local','.esphome','__pycache__'}
def digest(data):return hashlib.sha256(data).hexdigest()
def git(root,*args):return subprocess.check_output(['git',*args],cwd=root)
def input_files(root):
    if not (root/'.git').exists():
        names=[n for n in INPUT_FILES if (root/n).is_file()]
        for folder in INPUT_ROOTS:
            names.extend(str(p.relative_to(root)) for p in (root/folder).rglob('*') if p.is_file() and not EXCLUDED.intersection(p.relative_to(root).parts) and p.name!='secrets.yaml')
        return sorted(names)
    names=[]
    for name in git(root,'ls-files','-z').decode().split('\0'):
        if not name:continue
        if name not in INPUT_FILES and not name.startswith(INPUT_ROOTS):continue
        path=Path(name)
        if EXCLUDED.intersection(path.parts) or path.name=='secrets.yaml':raise ValueError('Private build input is tracked')
        names.append(name)
    # A new, untracked include/config must never be silently omitted from provenance.
    for name in git(root,'ls-files','--others','--exclude-standard','-z').decode().split('\0'):
        if name and (name in INPUT_FILES or name.startswith(INPUT_ROOTS)):raise ValueError('Untracked firmware input: '+name)
    return sorted(names)
def fingerprint(root):
    files={name:digest((root/name).read_bytes()) for name in input_files(root)}
    return files,digest(json.dumps(files,sort_keys=True,separators=(',',':')).encode())
def version(root):
    text=(root/'firmware/release/labsense.yaml').read_text()
    match=re.search(r'^    version: "([^"\n]+)"$',text,re.M)
    if not match:raise ValueError('Missing explicit project version')
    return match[1]
def expected_esphome(root):
    match=re.search(r'^esphome==([^\s]+)$',(root/'requirements.txt').read_text(),re.M)
    if not match:raise ValueError('ESPHome must be pinned to an exact version')
    return match[1]
def verify(root,record,build):
    files,stamp=fingerprint(root)
    if record.get('input_sha256')!=stamp or record.get('inputs')!=files:raise ValueError('Firmware inputs changed since the recorded build')
    if record.get('esphome_version')!=expected_esphome(root):raise ValueError('Build used a different ESPHome version from the pinned dependency')
    if record.get('version')!=version(root):raise ValueError('Build version differs from the source')
    for kind in ('factory','ota'):
        if digest((build/('firmware.'+kind+'.bin')).read_bytes())!=record.get('binaries',{}).get(kind):raise ValueError('Firmware binary changed since the recorded build: '+kind)
    return files,stamp
