"""Compile the generic release image and bind its artifacts to exact source inputs."""
from pathlib import Path
import json,os,subprocess,sys
from release_inputs import digest,expected_esphome,fingerprint,git,version
ROOT=Path(__file__).resolve().parents[1]
BUILD=ROOT/'firmware/release/.esphome/build/labsense/build'
RECORD=ROOT/'.local/release-build/provenance.json'
def source_revision(root):
    if not (root/'.git').exists():return 'source-archive'
    try:return git(root,'rev-parse','HEAD').decode().strip()
    except subprocess.CalledProcessError:return 'source-archive'
def main():
    before,stamp=fingerprint(ROOT);release_version=version(ROOT)
    RECORD.parent.mkdir(parents=True,exist_ok=True)
    RECORD.unlink(missing_ok=True)
    subprocess.run([sys.executable,'-m','esphome','compile',str(ROOT/'firmware/release/labsense.yaml')],cwd=ROOT,check=True)
    after,after_stamp=fingerprint(ROOT)
    if before!=after or stamp!=after_stamp:raise RuntimeError('Build inputs changed during compilation; discard this build')
    info=json.loads((BUILD.parent/'build_info.json').read_text())
    if info['esphome_version']!=expected_esphome(ROOT):raise RuntimeError('The build environment differs from the pinned ESPHome version')
    record={'schema':1,'version':release_version,'source_commit_at_build':source_revision(ROOT),
      'input_sha256':stamp,'inputs':after,'esphome_version':info['esphome_version'],'config_hash':info['config_hash'],
      'build_time':info['build_time'],'binaries':{kind:digest((BUILD/('firmware.'+kind+'.bin')).read_bytes()) for kind in ('factory','ota')}}
    temporary=RECORD.with_suffix('.tmp');temporary.write_text(json.dumps(record,indent=2)+'\n');os.replace(temporary,RECORD)
    print('Generic build provenance recorded: '+release_version+' / '+stamp[:12])
if __name__=='__main__':main()
