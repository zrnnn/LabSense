# Hardware and measurements

![Component positions on the actual carrier](images/assembly/pcb-assembly-map.png)

For a build sequence, use the [illustrated assembly guide](assembly.md). For individual board images, see [front](images/assembly/pcb-front.png) and [physically mirrored rear](images/assembly/pcb-rear.png).

## Assembly

| Part | Current design |
| --- | --- |
| Controller | Seeed Studio XIAO ESP32C3, 4 MB flash |
| VOC | CCS811 breakout, I²C address 0x5A |
| Temperature / humidity / pressure | Intended BME280 breakout, automatically checks 0x76 / 0x77 and chip ID 0x60 |
| Display | SSD1306-compatible 128 × 64 OLED, auto 0x3C / 0x3D |
| Power | USB for this iteration |
| Antenna | External 40 × 20 mm adhesive foil, approximately 60 mm cable |

The BME/BMP280 printing on a breakout does not establish the actual sensor identity. A BMP280 cannot supply humidity; the firmware checks chip identity and does not treat missing compensation as valid VOC assessment.

## Bus and carrier connections

| Signal | XIAO pin | ESP32-C3 pin | Connections |
| --- | --- | --- | --- |
| SDA | D4 | GPIO6 | CCS811, BME280 and OLED |
| SCL | D5 | GPIO7 | CCS811, BME280 and OLED |
| Supply | 3V3 | — | Breakout supply rail |
| Ground | GND | — | Breakouts, CCS811 nWAKE and ADDR |

Bus speed is 100 kHz. CCS811 nRESET / nINT are broken out on the carrier but are not controlled by the firmware. The design has battery pads, but battery operation and charging are outside this iteration.

## Controller pin reference

These mappings are for **XIAO ESP32C3**, not every XIAO-family module. Carrier legends use peripheral names on several positions. The [Seeed pin map](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/#pin-map) supplies the controller GPIO mapping; carrier usage is reconstructed from this Gerber export.

| Carrier legend | XIAO pin | GPIO / rail | Use in LabSense |
| --- | --- | --- | --- |
| D0 | D0 | GPIO2 | Unused |
| D1 | D1 | GPIO3 | Unused |
| D2 | D2 | GPIO4 | Auxiliary H2, unused |
| D3 | D3 | GPIO5 | Auxiliary H3, unused |
| SDA | D4 | GPIO6 | Shared I²C data |
| SCL | D5 | GPIO7 | Shared I²C clock |
| TX | D6 | GPIO21 | Unused |
| RX | D7 | GPIO20 | Unused |
| SCK | D8 | GPIO8 | Unused |
| MISO | D9 | GPIO9 | No external SPI use; also the on-module BOOT/recovery button |
| MOSI | D10 | GPIO10 | Unused |
| 3V3 | 3V3 | Regulated 3.3 V | All three breakouts |
| GND | GND | Ground | Common return |
| 5V | 5V | USB VBUS | Supplied through the XIAO USB connector; not the breakout supply |

With USB pointing right in the component-side images, the XIAO upper row reads **TX, SCL, SDA, D3, D2, D1, D0**, and its lower row reads **RX, SCK, MISO, MOSI, 3V3, GND, 5V**, left to right. The PCB footprint includes underside BAT/THERM landing pads for other mounting methods; the socketed USB build does not require wiring those pads.

| Auxiliary carrier connection | Reconstructed wiring | Current assembly |
| --- | --- | --- |
| Three-pin header, top→bottom D3 / RST / VIN | GPIO5 / CCS811 nRESET / 3V3 | Leave unpopulated; no jumper |
| Two-pin header, top→bottom INT / D2 | CCS811 nINT / GPIO4 | Leave unpopulated; no jumper |
| Battery connector BAT+ / BAT− | XIAO battery landing pads | Leave unpopulated for USB-only use |

The auxiliary labels do not mean INT is already wired to D2, or RST to D3: each is a separate pin on its header. CCS811 nWAKE and ADDR are already grounded by carrier copper. No extra wire is needed for the documented I²C build.

Connectivity was reconstructed from the Gerbers and checked by a raster connectivity analysis. This is not an authoritative schematic, netlist or fabrication DRC. The editable PCB design, schematic, original module supplier part numbers and BOM were not supplied. The [purchasing guide](sourcing.md) now identifies researched candidates by pin layout, with a [purchasing BOM](../hardware/bom/parts.csv); replacement modules remain subject to mechanical fit checks. See [PCBWay ordering](pcb-ordering.md) for the confirmed 1.5 mm board requirement.

## Mechanical datums

All component Z measurements below start at the **underside of the carrier PCB**, positive toward the components. PCB thickness is already included; do not add it again.

| Measurement | Value | Basis |
| --- | --- | --- |
| Carrier outline | nominal 55 × 50 mm, 3 mm corners | Gerber outline centerlines |
| PCB thickness | 1.5 mm | User measured |
| Interior floor to PCB underside | 4 mm | User requested pin clearance |
| Display face / maximum assembly height | 9.5 mm | User measured from PCB underside |
| USB-C socket top | 8.9 mm | User measured from PCB underside |
| USB-C socket bottom | 5.2 mm | 3.7 mm above PCB top + 1.5 mm board |
| USB protrusion past carrier edge | 0.3 mm | User measured |
| Display glass | 26.7 × 19.6 mm | User measured |
| Antenna foil | 40 × 20 mm | User supplied; thickness envelope remains provisional |

Two carrier fixing holes are nominal **Ø3.05 mm**. In the analysis coordinate system (board lower-left), their centers are **(51.816, 47.196)** and **(50.673, 13.414) mm**. These coordinates use the nominal lower-left board datum. Do not mix Gerber raw axes with the assembly view.

The user confirmed the USB opening and display opening location fit the printed enclosure. The deeper display collar and the three-screw v3 enclosure still need a printed fit check.

## Fabrication and drawings

- [Original Gerber fabrication ZIP](../hardware/pcb/source/LabSense-Original-Gerbers.zip)
- [Board mechanical drawing](images/assembly/mechanical-drawing.png)
- [Module pin map](images/sourcing/module-pinout.png)
- [Parts list](../hardware/bom/parts.csv)

The files and measurements on this page apply to **LabSense Original**, the XIAO ESP32C3/CCS811/BME280 carrier. Enclosure revision v3 is a revision of the original case, not LabSense v2. Other boards and sensor combinations require their own firmware and mechanical design.
