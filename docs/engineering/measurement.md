# Measurement behavior

| Quantity | Source | Handling |
| --- | --- | --- |
| Temperature / relative humidity | BME280 | Update every ten seconds; used to compensate CCS811 |
| VOC estimate | CCS811 at 0x5A | Sample every second; raw API publishing is throttled and can continue during warm-up; evaluation remains unavailable until ready |
| Pressure | BME280 | Available as a diagnostic measurement |
| Warning state | Local monitor | Sustained thresholds, hysteresis, sensor freshness and compensation checks |

Required BME280 chip ID is 0x60. A BMP280 is rejected for humidity compensation. Display addresses 0x3C/0x3D and BME addresses 0x76/0x77 are detected at startup. See the [hardware interface](../hardware.md) and [purchasing pin checks](../sourcing.md).

A factory-new sensor defaults to 48 hours of uninterrupted initial conditioning; subsequent starts use 20 minutes once the saved conditioned state is set. This conservative release policy is distinct from calibration. A baseline restored after warm-up adds one minute of settling. Baseline storage is explicitly user initiated; it does not prove the sampled air was clean.

Partial initial conditioning is not accumulated across power cycles. The OLED withholds its VOC number during warm-up; API consumers must check **VOC evaluation ready**, since a fresh raw estimate alone does not establish readiness. Missing/stale gas data can restart conditioning. The [build guide](../assembly.md#7-let-the-sensors-settle) explains the first-start and restart sequence for assembly.

| Rule | Default |
| --- | --- |
| Preliminary API indication | ≥500 ppb for 120 seconds |
| Home Assistant high indicator | ≥2,000 ppb for 30 seconds |
| OLED high banner | Immediately at a valid, ready estimate ≥2,000 ppb; also while the high indicator remains latched |
| High clears | <1,600 ppb for five minutes |
| Preliminary indication clears | <400 ppb for five minutes after high clears |
| Gas sample freshness | Maximum 15 seconds |

Invalid, missing or stale data invalidates assessment rather than publishing a healthy zero. Timer arithmetic handles uptime wrap. Reference comparisons and actual sensor response in the closed enclosure remain hardware acceptance work.

The CCS811 is a broad VOC estimator. Resin and printing emissions can include compounds and particles this sensor does not adequately characterize. These thresholds are configurable engineering indications, not occupational or medical limits. Do not derive exposure safety or a ventilation design from a low reading.

Use the [physical validation plan ](validation-plan.md) for reference comparisons, enclosure thermal checks, VOC limitations and mechanical acceptance. No completed reference-measurement record is currently published.
