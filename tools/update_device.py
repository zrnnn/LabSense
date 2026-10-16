"""Upload a LabSense OTA application with encrypted transport and no downgrade."""
from pathlib import Path
import argparse, base64, getpass
from esphome.espota2 import run_ota

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host',required=True)
    parser.add_argument('--image',type=Path,required=True)
    args=parser.parse_args()
    if not args.image.is_file() or not args.image.name.endswith('.ota.bin'):
        parser.error('Choose the release .ota.bin application image.')
    key=getpass.getpass('Device pairing key (hidden): ').strip()
    try: raw=base64.b64decode(key,validate=True)
    except ValueError: parser.error('The pairing key is not valid base64.')
    if len(raw)!=32 or not any(raw): parser.error('Expected a nonzero 32-byte pairing key.')
    result,_=run_ota(args.host,3232,None,args.image,noise_psk=key,plaintext_fallback=False)
    return result
if __name__=='__main__':raise SystemExit(main())
