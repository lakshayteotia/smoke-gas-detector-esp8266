# Hardware

## Parts

| Part | Role | Notes |
|---|---|---|
| NodeMCU ESP8266 | Main controller, WiFi AP + web server | |
| MQ-series gas sensor | Smoke/gas detection | TODO: exact model. Digital output (DO) used; threshold set by onboard potentiometer |
| Relay module | Switches the exhaust fan | The sketch drives the pin HIGH for fan ON and LOW for fan OFF |
| DC exhaust fan | Removes smoke/gas | |
| Servo motor | Vent: 0 deg closed, 90 deg open | |
| Passive buzzer | Audio alarm | Driven with `tone()` at 1 kHz |
| 16x2 I2C LCD | Local status display | I2C address 0x27 |
| DC-DC buck converter | Battery voltage down to regulated 5 V | |
| 3x 18650 Li-ion pack | Portable power | |

## Pin mapping (from the sketch)

| Function | NodeMCU pin | GPIO |
|---|---|---|
| MQ sensor DO | D5 | GPIO14 |
| Relay IN | D6 | GPIO12 |
| Buzzer | D7 | GPIO13 |
| Servo signal | D4 | GPIO2 |
| LCD SDA | D2 | GPIO4 |
| LCD SCL | D1 | GPIO5 |

GPIO2 is a boot-strap pin on the ESP8266 (it must be HIGH at boot), so a servo signal line on it can interfere with startup.

## Power

Battery pack -> buck converter -> 5 V rail feeding the ESP8266, relay, LCD, MQ sensor, servo, and buzzer.

Early builds used a perfboard with a broken solder joint, which gave an unstable rail and relay malfunctions. The rail was rebuilt with breadboard power rails and direct buck-converter distribution. A single shared rail for a servo, relay, and microcontroller is a known weak point (see known limitations).

## Photos and wiring diagram

TODO: add a labeled photo of the build and, ideally, a wiring diagram (Fritzing or hand-drawn) to `media/` and link them here.
