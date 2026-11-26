# Smoke & Gas Detection and Automated Response System (ESP8266)

> **Educational prototype. Not a certified safety device.** Do not rely on it to protect people or property.

An ESP8266 (NodeMCU) system that watches an MQ smoke/gas sensor and, when it trips for a sustained period, switches an exhaust fan through a relay, opens a servo-driven vent, sounds a buzzer, updates an LCD, and shows live status on a web dashboard with manual controls.

**Demo:** video link to be added after the LinkedIn post.

> The demo triggers the system with smoke. Gas-leak response was not demonstrated.

## What it does

- Detects smoke/gas using an MQ-2 sensor (digital output)
- Waits out a sensor warm-up before enabling automation
- Requires detection to persist (~2 s) before acting, to cut false alarms
- On detection: fan ON (relay), vent OPEN (servo), buzzer ON, LCD alert, dashboard alert
- Returns to SAFE state automatically when the sensor clears
- Local web dashboard with status display and manual fan / vent / buzzer / alarm controls
- Battery powered (3x 18650 through a 5 V buck converter)

## Hardware

NodeMCU ESP8266, MQ-2 gas/smoke sensor module, relay module, DC exhaust fan, servo motor, passive buzzer, 16x2 I2C LCD, DC-DC buck converter, 3x 18650 pack.

Full parts list and pin mapping: [`docs/hardware.md`](docs/hardware.md)

## How it works

Startup -> warm-up -> safe monitoring -> detection -> automatic response -> reset.
State diagram and details: [`docs/how-it-works.md`](docs/how-it-works.md)

## Dashboard

The ESP8266 runs its own WiFi access point (SSID `ESP_FireAlarm`) and serves the dashboard at `http://192.168.4.1`.

- **Local only.** You must be within WiFi range of the device. It is not remote monitoring.
- The access point is WPA2-protected, but the password is short and shared, and once connected the controls are unauthenticated: anyone who joins can change the fan or vent state or disable the alarm. Set your own password in `config.h`.
- The "Flame Meter" is a two-state indicator driven by the sensor's digital output, not a measurement.

## Build and flash

1. Arduino IDE with the ESP8266 board package installed (Boards Manager)
2. Board: NodeMCU 1.0 (ESP-12E Module)
3. Libraries: `ESP8266WiFi`, `ESP8266WebServer` (bundled with the ESP8266 core), `Wire`, `LiquidCrystal_I2C`, `Servo`
4. In `firmware/smoke_gas_detector/`, copy `config.example.h` to `config.h` and set your own AP password (8+ characters)
5. Open `smoke_gas_detector.ino` and upload

## Known limitations

Short version: digital threshold only (no ppm), a weak shared AP password with unauthenticated controls, no latching alarm (the Acknowledge button does not hold), and a short warm-up that does not fully stabilize an MQ sensor.
Details and planned fixes: [`docs/known-limitations.md`](docs/known-limitations.md)

## What I learned

- **Sensor warm-up and debouncing.** The MQ sensor false-triggered at startup. A 15 s warm-up and a 2 s confirmation timer reduced false alarms but do not truly stabilise the sensor (see known limitations).
- **Check the power rail early.** A broken perfboard joint gave an unstable rail and a misbehaving relay. The fix was rebuilding the power distribution, not changing code.
- **A dashboard control has to change real state.** My Acknowledge button does nothing lasting because the main loop overrides it. Finding that taught me to design state first and the UI second.

## Next project

Next project: VehicleSentinel, a dual-node ESP32 telemetry system (in progress).

## License

MIT. See [`LICENSE`](LICENSE).
