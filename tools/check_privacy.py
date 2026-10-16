"""Audit tracked files, archive members and optionally history for private content.

This heuristic complements review; it cannot prove that all sensitive data is absent.
Never prints a matching secret, only the file or history object location.
"""
from pathlib import Path
import argparse, io, re, subprocess, zipfile
ROOT = Path(__file__).resolve().parents[1]
RULES = {
    'personal filesystem path': re.compile(rb'/(?:ho' + rb'me|m' + rb'nt|Us' + rb'ers|t' + rb'mp)/[^\s"<>\x00]+'),
    'literal MAC address': re.compile(rb'\b(?:[0-9a-f]{2}:){5}[0-9a-f]{2}\b', re.I),
    'GitHub credential': re.compile(rb'\b(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,})\b'),
    'private key': re.compile(rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----'),
}

def inspect(data, location, failures):
    for label, pattern in RULES.items():
        if pattern.search(data): failures.add((location, label))
    stream = io.BytesIO(data)
    if zipfile.is_zipfile(stream):
        with zipfile.ZipFile(stream) as archive:
            for member in archive.infolist():
                if member.file_size > 100_000_000:
                    failures.add((location, 'archive member too large to audit'))
                elif not member.is_dir():
                    inspect(archive.read(member), location + ' :: ' + member.filename, failures)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--history', action='store_true', help='Also check all locally reachable Git blobs')
    parser.add_argument('--revision', default='--all', help='History revision to inspect; default is all refs')
    args = parser.parse_args()
    failures = set()
    tracked = subprocess.check_output(['git', 'ls-files', '-z'], cwd=ROOT).decode().split('\0')
    for name in filter(None, tracked):
        if any(part in {'.local', '.esphome'} for part in Path(name).parts) or Path(name).name == 'secrets.yaml':
            failures.add((name, 'private file is tracked'))
        else:
            inspect((ROOT / name).read_bytes(), name, failures)
    if args.history:
        for line in subprocess.check_output(['git', 'rev-list', '--objects', args.revision], cwd=ROOT).decode().splitlines():
            oid, _, name = line.partition(' ')
            kind = subprocess.check_output(['git', 'cat-file', '-t', oid], cwd=ROOT).strip()
            if kind == b'blob':
                inspect(subprocess.check_output(['git', 'cat-file', 'blob', oid], cwd=ROOT), 'history:' + oid[:10] + ':' + name, failures)
    for location, reason in sorted(failures): print(f'FAIL: {location}: {reason}')
    if failures: return 1
    print('Privacy checks PASS: tracked content' + (' and reachable history' if args.history else '') + ', including archive members.')
    return 0

if __name__ == '__main__': raise SystemExit(main())
