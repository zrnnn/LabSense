# Check measurements and enclosure effects

Complete-product accuracy has not been established against calibrated reference equipment. Use this procedure to assess your assembly; chip datasheet tolerances are not an assembled-device accuracy claim.

## Reference equipment and run record

Before testing, record the firmware/configuration revision, carrier/enclosure revision, orientation, room conditions, power source, display setting, temperature offset and whether the enclosure is open or closed. Identify the reference thermometer/hygrometer and its stated uncertainty or accuracy, calibration status and response time. If uncertainty is unknown, label the exercise a comparison, not a calibrated accuracy validation.

Record paired readings, times, conditions and reference uncertainty in a spreadsheet. Keep identifying information out of public reports. Do not insert invented samples to complete a report.

## Temperature, humidity and thermal bias

1. Place the powered sensor and reference in the same room-air region and at the same height, with no physical contact or direct heat/light source. Record their positions. Do not place the reference in the enclosure's warm-air plume.
2. Start logging during warm-up. Keep the raw BME temperature, displayed temperature, raw humidity, reference readings and time alignment. Record changes in window/door position or ventilation rather than silently discarding transients.
3. Wait for a stable comparison window supported by both instruments' trends. Store at least 30 paired samples over ten minutes after settling; these are selected procedural minima, not a claim that thermal stabilization always takes ten minutes.
4. Compare open-board and closed-enclosure operation in desk and wall orientation. Keep firmware, ambient conditions and reference placement consistent. Repeat with the ordinary display enabled and disabled to quantify any effect of electronics/display heat.
5. Repeat under several naturally available indoor conditions. A single room point cannot establish an accuracy specification over a broad range.
6. Report per-run mean error, error range, sample spread, drift and observed enclosure bias. Preserve outliers and explain exclusions. Reference uncertainty and correlated samples limit what can be concluded; do not divide uncertainty by the square root of sample count without an appropriate model.
7. Agree the intended product accuracy range and acceptance tolerance before assigning pass/fail. Do not copy an IC datasheet tolerance into the complete-product specification without enclosure/reference evidence.

Bosch states that BME280 temperature is affected by board temperature and sensor heating, so it may differ from ambient room air. This makes the enclosure comparison a release requirement. The manufacturer's humidity accuracy is also specified under particular conditions; it is not a blanket assembled-product guarantee. [BME280 datasheet, sections 1.2 and 1.4](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf)

The current temperature-offset entity changes the reported/displayed temperature only. CCS811 compensation still uses raw BME readings. An offset does not remove a humidity bias caused by a warm board and does not validate VOC compensation. Resolve thermal/layout issues and characterize them before treating an offset as a product calibration.

## VOC behavior and limits

- Record sensor ID, reported application firmware, lifetime-use history when known, warm-up/conditioning state, baseline actions, raw estimated VOC/eCO2, compensation inputs and data age.
- Observe ordinary occupied-room and normal ventilation behavior without deliberately generating printer/resin/solvent emissions for a test. A ventilated room is not a certified zero-VOC reference.
- Validate threshold timing, hysteresis and display symbol transitions using host simulations. These verify software behavior, not chemical detection performance.
- Validate stale/missing-data handling and restart recovery with controlled disconnection/power tests that do not compromise personal safety or other equipment. Record the observed sequence and restoration time.
- Do not claim compound-specific detection, absolute VOC accuracy or exposure-safety performance without suitable reference instruments and a controlled qualified measurement method. Particle and direct CO2 measurements are outside this hardware.

The 48-hour initial-conditioning rule in the generic firmware is a conservative LabSense policy, not a calibration certificate. Manufacturer documentation evolved across CCS811 revisions; the 2018 change notice describes earlier availability of estimated outputs. Record the installed sensor/application version rather than assuming all online descriptions apply. [ams CCS811 datasheet change notification CN18-2018](https://ams.com/documents/20143/307151/CN18-2018_Change%20notification%20for%20CCS811%20datasheet%20updates.pdf)

## Mechanical and assembly review

Check the actual printed v3 parts against the CAD/export revision: no broken clips/tabs, correct M3 cylinder/button-head seating, board support and underside pin clearance, no glass pressure at the display frame, a usable USB cable opening, concealed wall plate, secure sliding engagement and accessible ventilation. Record measured deviations, print settings and photographs before marking fit accepted. Adhesive wall attachment and screw attachment need separate checks; do not assume an untested adhesive/load rating.

Inspect foil-antenna placement and cable routing in the assembled product. Compare Wi-Fi/BLE setup and reconnection in intended mounting positions with the case open and closed. A single successful nearby connection does not establish a range specification.

## Evidence and sign-off

Store a sanitized run summary, reference-equipment limitations, actual enclosure observations and relevant revision hashes. Keep pending or failed checks visible; a documented procedure alone does not pass a hardware or measurement gate.
