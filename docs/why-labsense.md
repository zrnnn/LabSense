# Why LabSense belongs in a little lab

A printer, resin wash station and workbench can turn a spare room into a useful workshop. We choose machines and materials carefully; the room we spend hours in deserves attention too. LabSense Original is a modest DIY project that makes a VOC estimate, temperature and humidity visible while we work, with local Home Assistant history if wanted.

Its value is awareness: noticing a change, reviewing a pattern and investigating the context. It does not make a printing process safe or replace emission controls.

## VOC, in everyday language

**VOC** stands for **volatile organic compound**. These are carbon-based chemicals that can enter room air as gases. VOC describes a broad group, not one substance with one level of risk. Some VOCs can irritate the eyes or airways; other health effects depend on the substance and exposure. The mix of chemicals, their concentrations and exposure duration matter; a generic VOC number cannot identify those chemicals. [EPA: technical overview of VOCs](https://www.epa.gov/indoor-air-quality-iaq/technical-overview-volatile-organic-compounds), [EPA: indoor sources and health context](https://www.epa.gov/indoor-air-quality-iaq/volatile-organic-compounds-impact-indoor-air-quality).

In printing spaces, emissions can arise during preparation, printing, post-processing and cleaning. Heated filament and resin processes can release gases and ultrafine particles; resin washing adds solvent use such as isopropyl alcohol (IPA). The printer, material, operating conditions and ventilation affect what reaches the room. A printer cover alone is not evidence that emissions are contained. [NIOSH: safe 3D printing](https://www.cdc.gov/niosh/bulletin/2024/safe-3d-printing.html), [NIOSH: resin printing and post-processing](https://www.cdc.gov/niosh/media/pdfs/2025/01/Safe-3D-Printing.pdf).

## What the original sensor measures

| Reading | What it means | What it does not establish |
| --- | --- | --- |
| **VOC estimate, ppb** | CCS811 metal-oxide gas response processed into an **equivalent total VOC (eTVOC)** estimate | Identity or accurate concentration of a particular solvent, resin ingredient or every VOC combined |
| **Temperature, °C** | BME280 temperature at the sensor's location | Whole-room uniformity or assembled-product calibrated accuracy |
| **Relative humidity, %** | BME280 relative humidity at the sensor's location | Moisture inside a filament spool or whether a material is dry enough to print |
| **eCO2 VOC-based, ppm**, optional API entity | An equivalent CO2 value derived from gas response | A direct CO2 measurement; use an appropriate CO2 instrument for that task |

The CCS811 is sensitive to a range of gases, including alcohols and other VOC classes. It converts a combined response into an estimate; it does not separate a mixture into individual chemicals. Sensitivity varies between compounds, and temperature, humidity, baseline history and conditioning influence interpretation. This hardware has not been validated as a detector for a named resin or filament emission. [ams CCS811 datasheet, v1-06](https://cdn.sparkfun.com/assets/2/4/2/9/6/CCS811_Datasheet.pdf), [ams sensing-properties factsheet](https://cdn.sparkfun.com/assets/d/d/3/8/b/CCS811_Factsheet.pdf).

**ppb** means parts per billion; **1 ppm = 1,000 ppb**. Here it is the sensor algorithm's gas-equivalent scale, not a verified concentration of a named chemical. Do not compare it directly with a chemical-specific occupational limit or convert it into a mixed-gas mass concentration without a valid measurement model.

## How it can be useful

- Keep changing room conditions visible without opening an app.
- Review time-aligned VOC estimates, temperature and humidity in Home Assistant alongside notes about ordinary printing, cleaning and ventilation.
- Notice a sustained rise and investigate likely sources or changes in the workspace. A rise during cleaning may come from the cleaner; it does not prove that the printer caused it.
- Create your own reminders in Home Assistant. LabSense exposes readings and warning entities; it does not install automations for you.

A falling estimate after ventilation is an observation about this sensor's response, not proof that all pollutants were removed. Avoid turning ventilation off, releasing solvents or deliberately exposing yourself to emissions to test the sensor.

## Readings have a ready state

A new installation uses **48 continuous hours** of valid gas sampling under LabSense's conservative conditioning policy. Later starts use **20 minutes**, with another **60 seconds** after a successful saved-baseline restore. This is stabilization, not calibration. Check **VOC evaluation ready**, compensation and fault status before interpreting the estimate. [Exact readiness and warning behavior](engineering/measurement.md).

The default **2,000 ppb high indication** is a configurable technical prompt, not a health, toxicological or legal exposure limit. A low value, a quiet notification or completed warm-up cannot certify safe air.

## What remains essential

LabSense Original has **no particle sensor** and does not directly measure CO2 or CO. It is not a smoke detector, combustible-gas alarm or compound-specific exposure instrument. Do not rely on it to control access to a room or decide that protective measures are no longer needed.

Follow the material's safety data and appropriate source extraction, ventilation and handling requirements from the start. The monitor complements those practices by making some changes easier to notice; it cannot establish their adequacy. NIOSH's [makerspace guide](https://www.cdc.gov/niosh/bulletin/2024/safe-3d-printing.html) is a useful starting point.

This guide applies to **LabSense Original: XIAO ESP32C3, CCS811, BME280 and SSD1306**. It does not describe second-generation hardware. [Build the original device](assembly.md) · [Set it up](installation.md).
