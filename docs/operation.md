# Operation and troubleshooting

After a short startup animation, the OLED reports sensor recognition and Wi-Fi/API connection state. During normal operation a fixed borderless view shows a **large VOC estimate**, with **temperature and relative humidity below**. No pages rotate. A high VOC indication adds an inverted ventilation banner; faults and warm-up use explicit English text and dashes. See the [display specification and illustrations](engineering/display.md).

The current view uses English status text rather than a smiley. A low VOC estimate does not establish that room air is safe.

A new CCS811 has a conservative initial 48-hour conditioning period in the release configuration. The conditioned state is saved after this period completes. Later boots use a 20-minute warm-up. Interrupted initial conditioning starts again; `---` on the VOC display means assessment is not ready or is unavailable. Check the readiness and fault entities in Home Assistant to distinguish these states.

| Symptom | Check |
| --- | --- |
| No setup hotspot | Wait for startup; inspect the OLED, USB supply and antenna. The hotspot name includes a device suffix. For a configured sensor, use physical recovery if you need the key again. |
| Setup page will not open | Remain on the sensor network despite “no internet”; try 192.168.4.1. Temporarily disable phone features that automatically switch away from it. |
| No home network in the chooser | Refresh, verify 2.4 GHz coverage, or use Other network. Enterprise Wi-Fi/captive hotel networks are outside this build's setup flow. |
| Wi-Fi does not connect | Rejoin the setup network and re-enter credentials. If the page was reloaded after saving credentials, use BOOT recovery to retrieve/save the key and start a new setup session. |
| Home Assistant does not discover it | Check the router's device list, subnet/multicast policy, then add ESPHome manually on port 6053. |
| Home Assistant rejects the key | Use that sensor's saved card. Physical recovery reveals its current key; a factory erase changes it. |
| VOC is unavailable | Check conditioning, BME280 identity, both sensor connections and the fault/readiness entities. Missing compensation is not treated as clean air. |
| Temperature reads warm | Compare after thermal settling in the closed case; check vents and heat sources before applying an offset. |
| Update fails | Check the image version, correct sensor key and network address. The release intentionally rejects unencrypted uploads. Use USB if firmware cannot start. |

The default Home Assistant high indicator is 2,000 ppb sustained for 30 seconds, clearing after five minutes below 1,600 ppb. The OLED reacts immediately to a valid, ready estimate at or above 2,000 ppb and keeps the warning while that indicator is latched. These are engineering indications, not toxicological exposure limits. The CCS811 does not identify specific resin chemicals, measure particles or prove that air is safe. See [measurement behavior](engineering/measurement.md).

## Display orientation

In the setup page, open **Display settings → USB port position**. Changes apply immediately and are saved independently of Wi-Fi and the pairing key. After commissioning, the **Display orientation** setting is also available through the device's Home Assistant entities; no Home Assistant configuration is installed by this project.

| USB position | Clockwise screen rotation |
| --- | --- |
| Left | 0°, default and original view |
| Bottom | 90° |
| Right | 180° |
| Top | 270° |

Portrait positions use a 64 × 128 layout with the same readings and status information. Temperature and RH are rounded to whole units on the OLED for readability; Home Assistant retains the configured decimal precision. Startup, setup and recovery screens follow the saved orientation too. All current release messages and settings are English.
