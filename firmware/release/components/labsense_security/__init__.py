"""Require upstream ESPHome encrypted OTA with the runtime API key.

Pinned to ESPHome 2026.9.1: the runtime already implements this combination,
but its YAML encryption validator only accepts build-time keys.
"""
import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome.core import CORE
CONFIG_SCHEMA = cv.Schema({})
DEPENDENCIES = ['api', 'wifi', 'ota']
def validate(config):
    full = fv.full_config.get()
    api = full.get('api', {})
    ota = full.get('ota', [])
    if 'encryption' not in api or api['encryption'].get('key'):
        raise cv.Invalid('LabSense requires a runtime API key; never bake a device key into a release image.')
    if len(ota) != 1 or ota[0].get('platform') != 'esphome' or 'password' in ota[0]:
        raise cv.Invalid('LabSense permits only the encrypted ESPHome OTA transport, without a password fallback.')
    if 'web_server' in full or 'improv_serial' in full:
        raise cv.Invalid('Use the LabSense setup portal; additional commissioning/upload surfaces are not permitted.')
FINAL_VALIDATE_SCHEMA = validate
async def to_code(config):
    cg.add_define('USE_OTA_ENCRYPTION_REQUIRED')
