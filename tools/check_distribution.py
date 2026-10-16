"""Validate the public source distribution, local documentation links and fabrication input."""
from pathlib import Path
import argparse, hashlib, io, re, subprocess, zipfile
ROOT=Path(__file__).resolve().parents[1]
FORBIDDEN={'.fcstd','.stl','.step','.stp','.3mf','.obj','.ply','.brep','.kicad_pcb','.kicad_sch','.kicad_pro','.bin','.elf','.log'}
def inspect(name,data):
    assert Path(name).suffix.lower() not in FORBIDDEN, 'Non-distribution payload: '+name
    assert not any(x in Path(name).parts for x in ('rev4','experimental','.local','.esphome','marketing')), 'Internal workspace content: '+name
    assert Path(name).name not in ('AGENTS.md','secrets.yaml','.env'), 'Private/internal file: '+name
    if name.lower().endswith('.zip'):
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            assert len([m for m in archive.infolist() if not m.is_dir()])==13,'Unexpected fabrication archive content'
            for member in archive.infolist():
                if member.is_dir():continue
                p=Path(member.filename)
                assert not p.is_absolute() and '..' not in p.parts,'Unsafe archive member'
                assert p.suffix.lower() in ('.gtl','.gbl','.gts','.gbs','.gto','.gbo','.gko','.gdl','.drl','.gtp','.txt'),'Non-Gerber archive member'
    if name.endswith('.md'):
        text=data.decode()
        for target in re.findall(r'\]\(([^)]+)\)',text):
            target=target.split('#')[0].strip('<>')
            if not target or re.match(r'^[a-z]+:',target):continue
            assert (ROOT/name).parent.joinpath(target).exists(),'Broken documentation link: '+name+' -> '+target
    for rule in [rb'/(?:ho'+rb'me|m'+rb'nt|Us'+rb'ers|t'+rb'mp)/[^\s"<>\x00]+',rb'\b(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,})\b',rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----']:
        assert not re.search(rule,data),'Private content pattern in '+name
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--build',action='store_true',help='Check compiled firmware security flags')
args=parser.parse_args()
names=[n for n in subprocess.check_output(['git','ls-files','-z'],cwd=ROOT).decode().split('\0') if n]
assert names,'No tracked source distribution'
for name in names:inspect(name,(ROOT/name).read_bytes())
a=ROOT/'hardware/pcb/source/LabSense-Original-Gerbers.zip'
assert hashlib.sha256(a.read_bytes()).hexdigest()==(a.parent/'SHA256SUMS').read_text().split()[0],'Fabrication checksum mismatch'
if args.build:
    build=ROOT/'firmware/release/.esphome/build/labsense/src/esphome'
    defines=(build/'core/defines.h').read_text()
    for flag in ('USE_OTA_ENCRYPTION','USE_OTA_ENCRYPTION_PROVISIONED','USE_OTA_ENCRYPTION_REQUIRED','USE_WIFI_SCAN_RESULTS_LISTENERS'):
        assert '#define '+flag+'\n' in defines,'Missing build security flag: '+flag
    for flag in ('USE_OTA_WEB_SERVER','USE_WEBSERVER_OTA','USE_WEB_SERVER_OTA','USE_OTA_PASSWORD','USE_API_NOISE_PSK_FROM_YAML','USE_IMPROV_SERIAL'):
        assert '#define '+flag not in defines,'Unexpected build flag: '+flag
    assert not (build/'components/web_server/ota').exists(),'Browser updater must not be built'
    print('Compiled encryption and setup guards verified.')
print('Distribution checks PASS:',len(names),'files; links, privacy patterns and original Gerbers verified.')
