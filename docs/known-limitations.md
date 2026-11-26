# Known limitations

This is a learning project. These are the real gaps, including bugs found in code review.

| Limitation | Why it matters | Planned fix |
|---|---|---|
| Reads the sensor's **digital output only** | A potentiometer-set threshold, not a measurement. No concentration, no calibration curve. The dashboard "Flame Meter" is a two-state bar (100% or 4%), not a measurement | Read the analog output and calibrate against a reference |
| **MQ sensor cross-sensitivity** | The MQ-2 responds to smoke, LPG, methane, hydrogen, alcohol and other combustibles; the system cannot tell fire from a gas leak | Consider a dedicated sensor per hazard |
| **Short warm-up (~15 s)** | MQ datasheets call for a much longer preheat for stable readings; the delay mainly hides startup false triggers | Longer preheat and baseline logic |
| **Weak AP password, unauthenticated controls** | The AP uses a short shared WPA2 password, and the control endpoints are plain unauthenticated GET requests. Anyone who joins can turn the fan or vent off or disable the alarm | Strong per-device password; authenticated, non-GET control endpoints |
| **Dashboard is local only** | It only works within WiFi range of the device. Not remote monitoring | Join an existing network and publish to a broker |
| **No latching, and "Acknowledge" has no lasting effect** | `loop()` re-asserts the buzzer, fan, and vent every 200 ms while smoke is present, so Acknowledge and manual Fan OFF are overridden almost immediately. The alarm also clears the moment the sensor clears | Latched alarm state that holds until acknowledged |
| **Dashboard computes its own alarm state** | `/status` uses a separate timer and ignores the Alarm Enabled setting, so the dashboard can show FIRE while the device and LCD show SAFE | One shared alarm state used by the loop, LCD, and dashboard |
| **No release hysteresis** | One HIGH reading clears the alarm, so a sensor hovering at its threshold can chatter the relay, fan, and vent | Off-delay or hysteresis on release |
| **Detection timer starts at zero** | The 2 s confirmation timer is not reset after warm-up, so if the sensor is already LOW on the first loop pass the alarm can trigger immediately | Initialise the timer after warm-up |
| **Servo on GPIO2** | GPIO2 is a boot-strap pin and also drives the onboard LED | Move the servo to another pin |
| **Fan switched by a relay near a possible hazard** | If the target gas is flammable, switching a motor nearby is an ignition-risk question | Sealed or remote switching; review for the intended gas |
| **Single shared 5 V rail** | Servo and relay load can destabilize the microcontroller supply | Separate or better-regulated rails |
| **Prototype build** | Breadboard/perfboard wiring is fragile | Custom PCB or properly soldered board |

Ideas on the original wishlist: mobile alerts, cloud integration, battery monitoring, multi-sensor support, data logging.
