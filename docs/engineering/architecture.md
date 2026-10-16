# System architecture

## Product boundary

LabSense performs local sensing, compensation, display and warning-state evaluation. Wi-Fi carries measurements to Home Assistant through ESPHome's encrypted native API. Home Assistant automations and notifications are outside the firmware's scope.

| Layer | Responsibility | Source |
| --- | --- | --- |
| Sensors | CCS811 VOC; BME280 temperature/humidity/pressure | Release YAML, I²C on GPIO6/7 |
| Measurement policy | Warm-up, stale data, compensation, threshold timers and hysteresis | `voc_monitor.h` |
| Local interface | TEMP / RH / VOC, high indication, initial setup details | OLED lambda |
| Commissioning | Improv BLE Wi-Fi credentials; portal scan, POST, copy/save key | `web/setup-release` and pinned captive portal |
| Device identity | Random API key and separate random eight-digit hotspot password, persistent storage | `runtime_pairing.h` |
| Connectivity | Native API, mDNS, encrypted ESPHome OTA | ESPHome 2026.9.1 |
| Source build | Pinned ESPHome version and input/binary hashes | `tools/build_release.py`, `tools/release_inputs.py` |

## Trust and lifecycle

Setup-password generation runs before BLE and Wi-Fi start, using the ESP-IDF entropy source. API-key generation runs after API initialization and after Wi-Fi enables hardware entropy. Failed credential persistence disables networking. Existing saved API keys are reused.

The first-setup pairing endpoint is only available on an active, disconnected setup portal. Saving network credentials closes key disclosure. A later router outage may open Wi-Fi recovery but cannot reveal the key. An eight-second physical BOOT hold authorizes a new recovery session; it retains the key while clearing network credentials.

The release's small `labsense_security` integration requires ESPHome's existing encrypted OTA transport with its runtime API key. It sets the upstream `USE_OTA_ENCRYPTION_REQUIRED` guard because the pinned YAML validator cannot express runtime-key inheritance. Build validation checks this guard and rejects the browser OTA component, plaintext password fallback and static API keys. This integration must be re-audited on every ESPHome upgrade.

The captive portal is derived from upstream and retains its license. Release changes remove implicit browser firmware upload, use POST for credentials, bound accepted input and avoid logging Wi-Fi details. There are no external page assets or analytics requests.

Recovery deletes the pinned ESPHome runtime Wi-Fi preference keys before reboot. An empty SSID must never be stored as a reset value: upstream treats it as a wildcard. This key-specific integration must be re-audited with any ESPHome update; it leaves API credentials and sensor state untouched.

## BLE commissioning

The locally pinned `esp32_improv` component gates both advertising startup and credential writes on the LabSense setup window. The window opens on first commissioning or deliberate BOOT recovery and expires after ten minutes of uptime; connection closes it. Ordinary router outages do not open it. Credential length checks are shared with the portal, payload buffering is bounded, and credential logging is removed. Final provisioning status is retained briefly so clients can read it.

BLE and portal credential submission both require a matching-key acknowledgement from the active local portal in the current setup session. Improv advertises awaiting authorization until this acknowledgement arrives. No Home Assistant changes are required.

The API key is never included in Improv redirects. The protected portal remains the key copy/save interface; standard Improv clients only provision Wi-Fi. See [installation](../installation.md) for iOS fallback and Home Assistant Bluetooth prerequisites.

## Recovery constraints

Automatic ESPHome safe mode is disabled in this candidate: its reduced startup bypasses the hooks that initialize private setup credentials. A boot-looping unit uses physical USB recovery. Secure boot and flash encryption are not enabled. The device is a room indicator, not a safety controller.

## Hardware compatibility

This implementation targets LabSense Original with XIAO ESP32C3, CCS811, BME280 and SSD1306. It is not firmware for a second-generation board. The original case revision v3 and software version 1.3.x do not change that hardware identity.

The scan POST uses a nonempty form body because ESP-IDF rejects POST requests without Content-Length before routing them. The portal reads available results even if an explicit refresh fails and polls queued scans.
