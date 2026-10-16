# Firmware: LabSense Original

## Software status

The published source is **1.3.0-rc6**, an ESPHome release candidate for **XIAO ESP32C3, CCS811, BME280 and SSD1306 128 × 64**. Firmware builds and interface checks do not establish calibrated measurement accuracy or fit of alternative modules. No stable binary download is provided yet. Enclosure revision v3 is part of the original hardware, not a second-generation device.

## Build the source

Download this repository using **Code → Download ZIP**, extract it, and open a terminal in the extracted project folder. A Git clone also works. Keep the complete project: the YAML depends on the included C++ headers, local ESPHome components and font.

Use Python 3.12 or a version supported by the pinned ESPHome release, a USB data cable and internet for the initial toolchain/dependency download:

```sh
python3 -m venv .local/venv
.local/venv/bin/python -m pip install -r requirements.txt
.local/venv/bin/python tools/build_release.py
.local/venv/bin/esphome upload firmware/release/labsense.yaml --device YOUR_SERIAL_PORT
```

Replace `YOUR_SERIAL_PORT` with the connected controller's port. On Windows, use `.local\venv\Scripts\python.exe` and `.local\venv\Scripts\esphome.exe` instead. Follow [installation and first setup](installation.md) after flashing.

The configuration is generic. Do not add your home Wi-Fi password or a shared API key to the source. The sensor generates its individual setup password and pairing key and saves Wi-Fi credentials locally during setup. The browser page is embedded; runtime setup needs no internet.

The build records the exact input hashes, pinned ESPHome version and factory/OTA binary hashes under ignored `.local/release-build/`. Downloaded source works without Git metadata; a clone records its commit as well. Compilation output stays under ignored firmware caches.

## Updates and existing devices

Preserve a working sensor's pairing card and settings. Do not factory-erase as a routine update. Older, customized or compiled-in-key firmware requires a separately reviewed migration; the generic configuration is intended for a fresh device or the same supported runtime-key release family.

Initial USB installation uses the factory image. An OTA application image must not be flashed at address zero. For an ordinary network update, use the OTA image and your saved pairing key:

```sh
.local/venv/bin/python tools/update_device.py --host YOUR_SENSOR.local --image PATH_TO_FIRMWARE.ota.bin
```

The tool prompts for the key without echoing it and refuses plaintext fallback. Never put real credentials in issues or command examples. [Security and recovery](../SECURITY.md) · [operation](operation.md).

## Source layout and maintenance

`firmware/release/labsense.yaml` defines the hardware and entities. `voc_monitor.h` handles readiness and warnings; `runtime_pairing.h` handles device credentials/recovery; `oled_ui.h` and `oled_canvas.h` draw the display. Local components retain upstream licenses and modification notes.

The editable setup page is `web/setup-release/index.html`; regenerate its embedded counterpart with `python tools/build_portal_assets.py`. Preserve pinned versions and review security integrations before upgrading ESPHome. The build helper uses the same unchanged source inputs as the documented candidate.
