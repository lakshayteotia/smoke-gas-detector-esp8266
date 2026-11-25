#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "config.h"   // copy config.example.h to config.h and set AP_PASSWORD

// ===== Pins =====
#define MQ_PIN     D5
#define RELAY_PIN  D6
#define BUZZER_PIN D7
#define SERVO_PIN  D4

// ===== WiFi Settings =====
const char* ssid     = "ESP_FireAlarm";
const char* password = AP_PASSWORD;

ESP8266WebServer server(80);

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo ventServo;

// ===== System States =====
bool fanState      = false;
bool ventState     = false;
bool buzzerState   = false;
bool alarmEnabled  = true;
bool prevFireState = false;  // tracks previous fire state to detect transitions

// ===== LCD change-detection (prevents flicker) =====
String lastLine0 = "";
String lastLine1 = "";

void lcdWrite(const char* line0, const char* line1) {
  if (String(line0) != lastLine0) {
    lcd.setCursor(0, 0);
    lcd.print(line0);
    lastLine0 = String(line0);
  }
  if (String(line1) != lastLine1) {
    lcd.setCursor(0, 1);
    lcd.print(line1);
    lastLine1 = String(line1);
  }
}

// ===== HTML UI (unchanged) =====
const char webpage[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8" /><meta name="viewport" content="width=device-width,initial-scale=1" />
<title>ESP Fire — Interactive</title>
<style>
:root{--bg:#071024;--card:rgba(255,255,255,0.03);--neon-fire:#ff2d55;--accent:#00d4ff;--muted:#9aa6b2;}
*{box-sizing:border-box}
body{margin:0;font-family:Inter,Arial,sans-serif;background:linear-gradient(180deg,#041026 0%,#071024 100%);color:#ecf6ff;padding:18px}
.header{display:flex;gap:12px;align-items:center;margin-bottom:18px}
.logo{width:56px;height:56px;border-radius:12px;background:linear-gradient(135deg,#ff6b6b,#ff2d55);display:flex;align-items:center;justify-content:center;font-weight:700}
.title{font-size:20px;font-weight:700}.subtitle{color:var(--muted);font-size:13px}
.grid{display:grid;grid-template-columns:1fr 360px;gap:16px;align-items:start}
@media(max-width:900px){.grid{grid-template-columns:1fr}.rightCol{order:2}.leftCol{order:1}}
.card{background:linear-gradient(180deg,rgba(255,255,255,0.02),rgba(255,255,255,0.01));padding:14px;border-radius:12px;border:1px solid rgba(255,255,255,0.03);backdrop-filter:blur(6px)}
.controls{display:flex;flex-wrap:wrap;gap:10px;margin-top:8px}
.btn{padding:10px 14px;border-radius:10px;background:linear-gradient(180deg,#071225,#0b1730);border:1px solid rgba(255,255,255,0.04);color:var(--accent);cursor:pointer;min-width:120px;transition:transform .12s}
.btn:hover{transform:translateY(-3px)}.btn.prim{background:linear-gradient(90deg,#ff6b6b,#ff2d55);color:#fff;border:none}
.btn.warn{background:linear-gradient(90deg,#ffb86b,#ff6b2d);color:#111;border:none}
.statusWrap{display:flex;flex-direction:column;align-items:center;gap:14px;padding:18px}
.ring{width:220px;height:220px;border-radius:50%;display:flex;align-items:center;justify-content:center;background:conic-gradient(#0b3450 0deg,#082536 360deg);position:relative;box-shadow:0 20px 60px rgba(2,6,23,0.7);}
.ringInner{width:170px;height:170px;border-radius:50%;display:flex;flex-direction:column;align-items:center;justify-content:center;background:linear-gradient(180deg,rgba(255,255,255,0.02),rgba(255,255,255,0.01));border:1px solid rgba(255,255,255,0.03)}
.statusText{font-weight:800;font-size:18px}.small{color:var(--muted);font-size:12px;margin-top:6px}
.ring.safe{box-shadow:0 30px 80px rgba(0,212,255,0.03),inset 0 0 30px rgba(0,212,255,0.02)}
.ring.alarm{box-shadow:0 30px 120px rgba(255,45,85,0.12),inset 0 0 50px rgba(255,45,85,0.06);animation:ringPulse 1.8s infinite;}
@keyframes ringPulse{0%{transform:scale(1);}50%{transform:scale(1.02);}100%{transform:scale(1);}}
.flameMeter{width:60px;height:180px;border-radius:12px;background:linear-gradient(180deg,#071f2e,#02101a);padding:6px;display:flex;align-items:flex-end}
.flameFill{width:100%;border-radius:8px;background:linear-gradient(180deg,#ff9a6b,#ff2d55);height:0%;transition:height .3s ease}
.indRow{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:12px}
.indBox{padding:12px;border-radius:10px;background:linear-gradient(180deg,rgba(255,255,255,0.01),transparent);text-align:center}
.indVal{font-weight:800}
#popup{position:fixed;left:50%;top:10%;transform:translateX(-50%);z-index:9999;padding:18px 22px;border-radius:14px;background:rgba(255,45,85,0.9);display:none;max-width:720px;text-align:center}
.footer{text-align:center;color:var(--muted);margin-top:18px;font-size:13px}
</style>
</head>
<body>
    <div class="header">
        <div class="logo">FP</div>
        <div>
            <div class="title">Fire Control Panel — Interactive</div>
            <div class="subtitle">Premium — Ring + Flame Meter • SoftAP UI</div>
        </div>
    </div>
    <div class="grid">
        <div class="leftCol">
            <div id="popup">
                <div style="font-size:20px;font-weight:800;color:#fff">🔥 FIRE DETECTED 🔥</div>
                <div style="color:#ffe8e8;margin-bottom:10px">Evacuate & call emergency services.</div>
                <button class="btn prim" onclick="cmd('/ack')">Acknowledge</button>
            </div>
            <div class="card statusWrap">
                <div id="ring" class="ring safe">
                    <div class="ringInner">
                        <div id="statusIcon" style="font-size:34px">🔵</div>
                        <div id="statusText" class="statusText">SAFE</div>
                        <div class="small" id="smallTxt">System normal</div>
                    </div>
                </div>
                <div style="display:flex;gap:18px;margin-top:12px;align-items:flex-start">
                    <div style="display:flex;flex-direction:column;align-items:center;gap:10px">
                        <div class="small">Flame Meter</div>
                        <div class="flameMeter"><div id="flameFill" class="flameFill"></div></div>
                        <div class="small" id="mqLabel">MQ: --</div>
                    </div>
                    <div style="flex:1">
                        <div style="font-weight:700">Controls</div>
                        <div class="controls">
                            <button class="btn" onclick="cmd('/fanOn')">Fan ON</button>
                            <button class="btn" onclick="cmd('/fanOff')">Fan OFF</button>
                            <button class="btn" onclick="cmd('/ventOpen')">Vent OPEN</button>
                            <button class="btn" onclick="cmd('/ventClose')">Vent CLOSE</button>
                            <button class="btn warn" onclick="cmd('/buzzerTest')">Buzzer Test</button>
                            <button class="btn prim" onclick="cmd('/alarmToggle')">Toggle Alarm</button>
                        </div>
                        <div class="indRow">
                            <div class="indBox"><div class="small">Fan</div><div id="fanStatus" class="indVal">OFF</div></div>
                            <div class="indBox"><div class="small">Vent</div><div id="ventStatus" class="indVal">CLOSED</div></div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        <div class="rightCol">
            <div class="card">
                <div style="font-weight:700;margin-bottom:8px">Quick Status</div>
                <div class="indRow">
                    <div class="indBox"><div class="small">Buzzer</div><div id="buzStatus" class="indVal">IDLE</div></div>
                    <div class="indBox"><div class="small">Alarm Enabled</div><div id="almEn" class="indVal">YES</div></div>
                </div>
            </div>
            <div class="card" style="margin-top:16px">
                <div style="font-weight:700;margin-bottom:8px">About</div>
                <div style="color:var(--muted);font-size:13px">Designed by Lakshay Teotia</div>
            </div>
        </div>
    </div>
    <div class="footer">© <span id="year"></span> Lakshay Teotia</div>
    <script>
        document.getElementById('year').innerText = new Date().getFullYear();
        function update() {
            fetch('/status').then(r => r.json()).then(j => {
                const isAlarm = j.state === "ALARM";
                document.getElementById('ring').className = isAlarm ? 'ring alarm' : 'ring safe';
                document.getElementById('statusIcon').innerText = isAlarm ? '🔥' : '🔵';
                document.getElementById('statusText').innerText = isAlarm ? 'FIRE' : 'SAFE';
                document.getElementById('smallTxt').innerText = isAlarm ? 'Evacuate immediately' : 'System normal';
                document.getElementById('popup').style.display = isAlarm ? 'block' : 'none';
                document.getElementById('fanStatus').innerText = j.fan ? 'ON' : 'OFF';
                document.getElementById('ventStatus').innerText = j.vent ? 'OPEN' : 'CLOSED';
                document.getElementById('buzStatus').innerText = j.buzzer ? 'ON' : 'IDLE';
                document.getElementById('almEn').innerText = j.alarmEnabled;
                document.getElementById('flameFill').style.height = (j.mq == 0 ? 100 : 4) + '%';
                document.getElementById('mqLabel').innerText = 'MQ: ' + (j.mq == 0 ? 'FIRE' : 'SAFE');
            });
        }
        function cmd(path) { fetch(path); setTimeout(update, 200); }
        setInterval(update, 1000);
    </script>
</body>
</html>
)rawliteral";

// ===== Web Status Handler =====
void handleStatus() {
  int mqVal = digitalRead(MQ_PIN);
  static unsigned long fireTimerWeb = 0;
  bool fireDetected = false;
  if (mqVal == LOW) {
    if (millis() - fireTimerWeb > 2000) fireDetected = true;
  } else {
    fireTimerWeb = millis();
  }

  String json = "{";
  json += "\"state\":\"" + String(fireDetected ? "ALARM" : "SAFE") + "\",";
  json += "\"fan\":"    + String(fanState    ? "true" : "false") + ",";
  json += "\"vent\":"   + String(ventState   ? "true" : "false") + ",";
  json += "\"buzzer\":" + String(buzzerState ? "true" : "false") + ",";
  json += "\"alarmEnabled\":\"" + String(alarmEnabled ? "YES" : "NO") + "\",";
  json += "\"mq\":"     + String(mqVal);
  json += "}";
  server.send(200, "application/json", json);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  pinMode(MQ_PIN,     INPUT);
  pinMode(RELAY_PIN,  OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  fanState      = false;
  ventState     = false;
  buzzerState   = false;
  prevFireState = false;

  digitalWrite(RELAY_PIN, LOW);
  ventServo.attach(SERVO_PIN);
  ventServo.write(0);
  noTone(BUZZER_PIN);

  delay(200);
  Wire.begin(D2, D1);
  delay(500);
  lcd.init();
  lcd.backlight();

  lcdWrite("Warming Sensor  ", "Please wait...  ");
  delay(15000);
  lcd.clear();
  lastLine0 = "";
  lastLine1 = "";

  WiFi.softAP(ssid, password);
  server.on("/", [](){ server.send_P(200, "text/html", webpage); });
  server.on("/status", handleStatus);

  server.on("/fanOn",      [](){ fanState    = true;  server.send(200, "text/plain", "OK"); });
  server.on("/fanOff",     [](){ fanState    = false; server.send(200, "text/plain", "OK"); });
  server.on("/ventOpen",   [](){ ventState   = true;  server.send(200, "text/plain", "OK"); });
  server.on("/ventClose",  [](){ ventState   = false; server.send(200, "text/plain", "OK"); });
  server.on("/ack",        [](){ buzzerState = false; server.send(200, "text/plain", "OK"); });
  server.on("/buzzerTest", [](){ buzzerState = true;  server.send(200, "text/plain", "OK"); });

  server.on("/alarmToggle", [](){
    alarmEnabled = !alarmEnabled;
    buzzerState  = false;
    noTone(BUZZER_PIN);
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

// ===== LOOP =====
void loop() {
  server.handleClient();

  int mqVal = digitalRead(MQ_PIN);
  static unsigned long fireTimer = 0;
  bool fireDetected = false;

  if (mqVal == LOW) {
    if (millis() - fireTimer > 2000) fireDetected = true;
  } else {
    fireTimer = millis();
  }

  // --- AUTOMATIC OVERRIDE ---
  if (alarmEnabled && fireDetected) {
    fanState    = true;
    ventState   = true;
    buzzerState = true;
    lcdWrite("FIRE DETECTED!! ", "SIREN ACTIVE    ");
  } else {
    // FIX: only reset flags at the exact moment fire clears (transition: true → false)
    // On all other iterations with no fire, leave flags untouched so manual commands persist
    if (prevFireState) {
      fanState    = false;
      ventState   = false;
      buzzerState = false;
    }
    lcdWrite("STATUS: SAFE    ", "SYSTEM NORMAL   ");
  }

  prevFireState = fireDetected;  // remember this iteration's state for next iteration

  // --- APPLY PHYSICAL STATES ---
  digitalWrite(RELAY_PIN, fanState  ? HIGH : LOW);
  ventServo.write(ventState         ? 90   : 0);
  if (buzzerState) tone(BUZZER_PIN, 1000);
  else             noTone(BUZZER_PIN);

  delay(200);
}
