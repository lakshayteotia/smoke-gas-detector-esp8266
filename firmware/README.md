# Firmware

`smoke_gas_detector/smoke_gas_detector.ino` is the sketch as demonstrated in the demo video.

- Board: NodeMCU 1.0 (ESP-12E Module), ESP8266 Arduino core
- Libraries: `ESP8266WiFi`, `ESP8266WebServer`, `Wire`, `LiquidCrystal_I2C`, `Servo`
- LCD I2C address is hardcoded to `0x27` (change it if your backpack uses a different address)
- Pin definitions are at the top of the sketch and listed in [`../docs/hardware.md`](../docs/hardware.md)
- The sketch creates a WPA2 access point named `ESP_FireAlarm` and serves the dashboard at `http://192.168.4.1`. The password comes from `config.h`: copy `smoke_gas_detector/config.example.h` to `smoke_gas_detector/config.h` and set your own (8+ characters). `config.h` is gitignored, so the sketch will not compile until you create it. See [`../docs/known-limitations.md`](../docs/known-limitations.md).
- Sensor warm-up is a blocking 15 s `delay()` in `setup()`, so the dashboard is unavailable during that time.
