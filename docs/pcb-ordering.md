# Order the LabSense carrier PCB

**PCBWay** and **JLCPCB** accept the original Gerber archive for a straightforward upload-and-order workflow. The pictured bare prototype board was supplied by JLCPCB. That was an order for bare boards: solder the modules onto the delivered carrier yourself.

![Carrier PCB front and physically mirrored rear](images/assembly/pcb-board-views.png)

The front view shows the module labels and USB position. These Gerber-derived illustrations explain the board; the original ZIP remains the manufacturing input. Continue with the [illustrated soldering and assembly guide](assembly.md) after the boards arrive.

## Order in five steps

1. Download the original ZIP linked below; do not unpack and recompress it.
2. Open PCBWay Quick Order and upload the ZIP for **bare PCB fabrication**.
3. Check the detected 55 × 50 mm, two-layer board; request **1.5 mm** thickness and select the mask color you want.
4. Review their Gerber preview, drill/outline interpretation, engineering questions and the full production/shipping/tax quote.
5. Confirm the accepted quote and order. On arrival, inspect the boards before soldering the separately sourced modules.

## File to upload

Download the original [LabSense Gerber ZIP](../hardware/pcb/source/LabSense-Original-Gerbers.zip) and upload it unchanged to [PCBWay Quick Order](https://www.pcbway.com/QuickOrderOnline.aspx). Use this fabrication ZIP, not the enclosure print profile.

Choose PCB fabrication. PCB assembly is a separate service that would need a BOM, placement data and an assembly agreement; those are not included in these Gerbers. No stencil is needed for the intended hand-soldered breakout build.

For one prototype order, five boards cost US$2.23 plus shipping. This historical example is not a current quotation.

## Quote settings

| Setting | LabSense selection |
| --- | --- |
| Board type | Single design, bare rigid PCB, FR-4 |
| Layers | **2 copper layers** |
| Dimensions | **55 × 50 mm**, nominal outline with R3 corners |
| Quantity | Compare 5 and 10 boards; order the quantity you need |
| Thickness | **1.5 mm**, measured on the original assembly and used in enclosure dimensions |
| Copper | Standard **1 oz** recommended for a reorder; original order's copper weight is not recorded |
| Solder mask | Your choice of color; black matches the prototype |
| Silkscreen | White on black; choose readable contrast on another mask color |
| Surface finish | Standard lead-free HASL is a budget option; confirm the quoted selection |
| Other options | Standard service, no assembly or stencil; confirm drill/outline interpretation in review |

**Thickness check:** PCBWay's [standard quote form](https://www.pcbway.com/orderonline.aspx) currently lists **1.6 mm**, rather than 1.5 mm. Reorder the original saved job if available, or ask PCBWay to confirm the 1.5 mm requirement and its price. Do not silently substitute 1.6 mm in a tight enclosure: it changes component positions measured from the underside by 0.1 mm when the mounting stack is otherwise unchanged. Mask color can be chosen freely for fit; check its effect on the quote and production time.

The finish and copper choices above are purchasing recommendations, not recovered specifications from the original invoice. A Gerber file does not encode board thickness, copper weight or surface finish.

## Check the upload preview

1. Confirm a single **55 × 50 mm** board, rounded corners, two copper sides, and the expected module footprints.
2. Check drill preview, plated header holes and both approximately **Ø3.05 mm unplated mounting holes**. Smallest nominal plated drill is approximately **0.306 mm**.
3. Check top/bottom masks and silkscreen orientation. Use the [mechanical drawing](images/assembly/mechanical-drawing.png) as a reference, not as a replacement fabrication layer.
4. Review engineering questions before paying. The supplied drill export includes a few near-zero-length G85 slots (about 0.005–0.006 mm). If flagged as slots/routing, ask the fabricator to review the supplied files; do not delete drill data to bypass a warning.
5. Review production price, destination shipping, applicable VAT/import handling and the quoted dispatch date. Pay only after the fabrication review is acceptable.

### Layer map in the supplied archive

| File suffix/name | Purpose |
| --- | --- |
| GTL / GBL | Top / bottom copper |
| GTS / GBS | Top / bottom solder mask |
| GTO / GBO | Top / bottom silkscreen |
| GKO | Board outline |
| GDL | Documentation/drawing layer; **not extra copper** and not a second outline |
| GTP | Top paste-mask layer; a stencil is not needed for this hand-soldered build |
| Drill_PTH_Through.DRL | Plated through holes |
| Drill_PTH_Through_Via.DRL | Plated vias |
| Drill_NPTH_Through.DRL | Unplated holes |

The unchanged archive contains 13 files: 12 fabrication/drawing files and one exporter ordering note. The 11 extracted Gerber/drill files used for the earlier board analysis omit the optional top-paste layer and that note. Retain it as supplied; ensure GKO is the selected outline and GDL is treated as documentation. Verify the download against the [SHA-256 checksum](../hardware/pcb/source/SHA256SUMS).
## Budget and delivery

Obtain a live quote rather than relying on an old “$5 PCB” promotion. A **€5–15 production allowance for a small batch** is used in the [parts budget](sourcing.md), purely as a planning assumption; thickness, options and shipping can change it. A batch allocation is not the cash cost of a first order. Delivery times depend on the accepted order.

PCBWay references: [ordering steps](https://www.pcbway.com/helpcenter/Findproducts/How_do_I_place_an_order_.html), [online Gerber viewer](https://www.pcbway.com/project/OnlineGerberViewer-html). This is a fabrication workflow, not an endorsement that a new upload has already passed production review.
