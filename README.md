# ESP32 Relay + Weather Station

โปรเจกต์เรียนรู้ ESP32 (PlatformIO + Arduino framework) ที่ควบคุมรีเลย์ 3 ช่องด้วยสวิตช์ 3 ตัว เชื่อมต่อ WiFi ผ่านหน้าเว็บตั้งค่า (WiFiManager) ดึงสภาพอากาศและคุณภาพอากาศ (AQI/PM2.5) ของจังหวัดนนทบุรีจาก OpenWeather แล้วแสดงบนจอ OLED

> รายละเอียดฮาร์ดแวร์ของบอร์ด (pinout, ข้อควรระวังของขา) อยู่ที่ [ESP32DevkitBoard.md](ESP32DevkitBoard.md)

## 1. ความสามารถของโปรเจกต์

| ฟีเจอร์ | รายละเอียด |
|---|---|
| ควบคุมรีเลย์ | สวิตช์ SW1–SW3 กดครั้งหนึ่ง = สลับสถานะรีเลย์ 1–3 (toggle ตอนกดลง) พร้อม debounce 30 ms ทำงานแบบ non-blocking ด้วย `millis()` |
| WiFi | ใช้ WiFiManager ถ้ายังไม่เคยตั้งค่าจะเปิด AP ชื่อ `ESP32-Relay` ให้เชื่อมต่อแล้วตั้งค่า WiFi ผ่านเว็บที่ `192.168.4.1` (โหมด non-blocking สวิตช์/รีเลย์ใช้งานได้ระหว่างรอ WiFi) |
| รีเซ็ต WiFi | กด **SW1 ค้าง 5 วินาที** จะล้างค่า WiFi ที่บันทึกไว้แล้วรีสตาร์ทเพื่อเปิด config portal ใหม่ |
| สภาพอากาศ | ดึงจาก OpenWeather (`/data/2.5/weather` และ `/data/2.5/air_pollution`) ทุก 2 นาที พิกัดนนทบุรี (13.8621, 100.5144) หน่วย metric ภาษาอังกฤษ |
| จอ OLED | แสดงอุณหภูมิ, ความชื้น, feels like, คำอธิบายสภาพอากาศ, AQI และ PM2.5/PM10 วาดจอใหม่เฉพาะเมื่อข้อมูลหรือสถานะ WiFi เปลี่ยน |

## 2. ฮาร์ดแวร์และการต่อสาย

บอร์ด: ESP32 DevKit (DOIT ESP32 DevKit V1 / ESP-WROOM-32)

| อุปกรณ์ | ขา GPIO | หมายเหตุ |
|---|---|---|
| Relay 1 / 2 / 3 | 17 / 18 / 4 | Active LOW (LOW = ON) |
| Switch SW1 / SW2 / SW3 | 34 / 35 / 32 | Active LOW + external pull-up (GPIO34/35 ไม่มี pull-up ภายใน) |
| OLED SSD1306 128x64 (I2C) | SDA = 21, SCL = 22 | address `0x3C` (บางรุ่น `0x3D`), ใช้ไฟ 3V3 |

## 3. โครงสร้างไฟล์

```
.
├── platformio.ini          # ตั้งค่าบอร์ด, พอร์ต, ไลบรารี
├── ESP32DevkitBoard.md     # ข้อมูลบอร์ดและอุปกรณ์ต่อพ่วง
└── src/
    ├── main.cpp            # setup/loop: WiFi, สวิตช์, รีเลย์, รีเซ็ต WiFi
    ├── weather.h/.cpp      # ดึงและ parse ข้อมูล OpenWeather
    ├── display.h/.cpp      # วาดข้อมูลบนจอ OLED
    ├── secrets.example.h   # ตัวอย่างไฟล์เก็บ API key
    └── secrets.h           # API key จริง (ถูก ignore ไม่ขึ้น git)
```

## 4. การติดตั้งและเปิดโปรเจกต์ด้วย VS Code

1. ติดตั้ง [Visual Studio Code](https://code.visualstudio.com/)
2. ติดตั้งส่วนขยาย **PlatformIO IDE** (`platformio.platformio-ide`) โปรเจกต์แนะนำตัวนี้ไว้ใน `.vscode/extensions.json` และไม่แนะนำ `ms-vscode.cpptools-extension-pack` กับ `pioarduino.pioarduino-ide`
3. เปิดโฟลเดอร์โปรเจกต์: เมนู **File > Open Folder...** เลือกโฟลเดอร์ `ESP32` (หรือใช้คำสั่ง `code /Users/ggt/Documents/Coding/Learning/ESP32` ในเทอร์มินัล) PlatformIO จะตรวจพบ `platformio.ini` เอง
4. รอ PlatformIO ดาวน์โหลด platform `espressif32` และไลบรารีครั้งแรก (ใช้เวลาสักครู่ ต้องต่ออินเทอร์เน็ต)
5. ปุ่มที่แถบสถานะด้านล่างของ VS Code: **Build** (เครื่องหมายถูก), **Upload** (ลูกศรขวา), **Serial Monitor** (ปลั๊ก)

### ตั้งค่า API key

1. สมัครรับ API key ที่ https://openweathermap.org/api (ต้องมี key ที่เรียก Current Weather และ Air Pollution ได้)
2. คัดลอก `src/secrets.example.h` เป็น `src/secrets.h`
3. แก้ค่า `OPENWEATHER_API_KEY` เป็น key จริง ไฟล์นี้ถูกใส่ใน `.gitignore` จึงไม่ถูก commit

### Build / Upload / Monitor

ใช้ปุ่มบนแถบสถานะ หรือสั่งจากเทอร์มินัล (ต้องมี PlatformIO CLI `pio`; ถ้าติดตั้งผ่านส่วนขยายอยู่ที่ `~/.platformio/penv/bin/pio`):

```bash
pio run                 # build
pio run -t upload       # อัปโหลดลงบอร์ด
pio device monitor      # ดู Serial ที่ 115200 baud
```

พอร์ตที่ตั้งไว้ใน `platformio.ini` คือ `/dev/cu.usbserial-210` ถ้าเครื่องคุณได้พอร์ตอื่นให้ตรวจด้วย `ls /dev/cu.*` แล้วแก้ `upload_port` และ `monitor_port`

ถ้าอัปโหลดขึ้น "Connecting......" ค้าง ให้กดปุ่ม **BOOT** บนบอร์ดค้างไว้จนเริ่มอัปโหลด ถ้าไม่เห็นพอร์ตเลยให้ติดตั้งไดรเวอร์ CH340 / CP210x และตรวจว่าใช้สาย USB ที่รองรับข้อมูล

## 5. วิธีใช้งานอุปกรณ์

1. จ่ายไฟและอัปโหลดโปรแกรม จอ OLED จะขึ้น "Starting..."
2. **ตั้งค่า WiFi ครั้งแรก:** ใช้มือถือหรือคอมพิวเตอร์เชื่อมต่อ WiFi ชื่อ `ESP32-Relay` เปิดเบราว์เซอร์ไปที่ `192.168.4.1` เลือกเครือข่ายและใส่รหัสผ่าน แล้วบันทึก บอร์ดจะจำค่าไว้ในหน่วยความจำ
3. เมื่อเชื่อมต่อสำเร็จ จอจะขึ้น "Loading weather..." แล้วแสดงข้อมูลสภาพอากาศ อัปเดตทุก 2 นาที
4. กด SW1/SW2/SW3 เพื่อเปิด/ปิดรีเลย์ 1/2/3 (รีเลย์เริ่มต้นเป็น OFF ทุกครั้งที่บูต)
5. **เปลี่ยน WiFi:** กด SW1 ค้าง 5 วินาที จอขึ้น "Reset WiFi..." บอร์ดรีสตาร์ทและเปิด AP `ESP32-Relay` ให้ตั้งค่าใหม่ (ตอนเริ่มกด รีเลย์ 1 จะสลับสถานะหนึ่งครั้งตามปกติ แล้วกลับเป็น OFF หลังรีสตาร์ท)

### ตัวอย่างหน้าจอ

```
Nonthaburi        WiFi
<description>
28.5C          RH 70%
               FL 32C
AQI 2 Fair
PM2.5 15 PM10 22
```

AQI ตามนิยามของ OpenWeather: 1 Good, 2 Fair, 3 Moderate, 4 Poor, 5 Very Poor

## 6. ไลบรารีที่ใช้ (`lib_deps`)

| ไลบรารี | เวอร์ชัน | ใช้ทำอะไรในโปรเจกต์ | ตัวอย่างการใช้งานย่อ |
|---|---|---|---|
| ArduinoJson (bblanchon) | ^7.0.0 | parse JSON จาก OpenWeather | `JsonDocument doc; deserializeJson(doc, stream); float t = doc["main"]["temp"] \| 0.0f;` |
| Adafruit SSD1306 | ^2.5.7 | ขับจอ OLED | `Adafruit_SSD1306 oled(128, 64, &Wire, -1); oled.begin(SSD1306_SWITCHCAPVCC, 0x3C); ... oled.display();` |
| Adafruit GFX Library | ^1.11.9 | วาดข้อความ/กราฟิกบนจอ (ใช้ร่วมกับ SSD1306) | `oled.setTextSize(2); oled.setCursor(0, 24); oled.printf("%.1fC", temp);` |
| WiFiManager (tzapu) | ^2.0.17 | หน้าเว็บตั้งค่า WiFi | `wm.setConfigPortalBlocking(false); wm.autoConnect("ESP32-Relay");` แล้วเรียก `wm.process()` ใน `loop()`; ล้างค่าด้วย `wm.resetSettings()` |
| PubSubClient (knolleary) | ^2.8 | MQTT | ประกาศไว้ใน `platformio.ini` แต่**ยังไม่ได้ใช้ในโค้ดปัจจุบัน** |

ไลบรารีที่มากับ Arduino-ESP32 (ไม่ต้องติดตั้งเพิ่ม): `WiFi`, `WiFiClientSecure`, `HTTPClient`, `Wire`

หมายเหตุการใช้งาน:
- ใช้ `client.setInsecure()` กับ HTTPS (ไม่ตรวจ certificate) เพราะข้อมูลสภาพอากาศไม่ sensitive
- `oled.display()` ส่งทั้งเฟรมผ่าน I2C ใช้ ~20–30 ms จึงเรียกเมื่อข้อมูลเปลี่ยนเท่านั้น
- ฟอนต์มาตรฐานของ Adafruit GFX แสดงภาษาไทยไม่ได้ จึงตั้ง `lang=en` ในการเรียก OpenWeather

## 7. ค่าที่ปรับแต่งได้

| ค่า | ไฟล์ | ความหมาย |
|---|---|---|
| `WEATHER_LAT`, `WEATHER_LON` | `src/weather.cpp` | พิกัดที่ต้องการดูสภาพอากาศ |
| `WEATHER_INTERVAL_MS` | `src/weather.cpp` | ช่วงเวลาดึงข้อมูล (ค่าเริ่มต้น 2 นาที) |
| `HTTP_TIMEOUT_MS` | `src/weather.cpp` | timeout ของ HTTP (5 วินาที) |
| `DEBOUNCE_MS` | `src/main.cpp` | เวลา debounce สวิตช์ (30 ms) |
| `WIFI_RESET_HOLD_MS` | `src/main.cpp` | เวลากด SW1 ค้างเพื่อรีเซ็ต WiFi (5000 ms) |
| `OLED_ADDR` | `src/display.cpp` | I2C address ของจอ (`0x3C` หรือ `0x3D`) |
| `RELAY_PINS`, `SW_PINS` | `src/main.cpp` | ขาของรีเลย์และสวิตช์ |

## 8. แก้ปัญหาเบื้องต้น

| อาการ | วิธีแก้ |
|---|---|
| จอ OLED ไม่ติด | ตรวจสาย SDA/SCL, ลองเปลี่ยน `OLED_ADDR` เป็น `0x3D`, ดู Serial ว่าขึ้น "OLED init failed" หรือไม่ |
| ไม่เห็นพอร์ต / อัปโหลดไม่ได้ | เปลี่ยนสาย USB เป็นแบบมีสายข้อมูล, ติดตั้งไดรเวอร์ CH340/CP210x, กด BOOT ค้างตอนอัปโหลด, แก้ `upload_port` |
| ดึงสภาพอากาศไม่ได้ ("OpenWeather HTTP error") | ตรวจ API key ใน `src/secrets.h` (key ใหม่อาจใช้เวลาสักพักก่อนทำงาน) และการเชื่อมต่ออินเทอร์เน็ต |
| บอร์ดรีเซ็ตเองบ่อย (brownout) | ใช้แหล่งจ่ายไฟ/สาย USB ที่จ่ายกระแสได้พอ หรือเพิ่มตัวเก็บประจุ 10–100 µF ที่ขา 3V3 |
| build ไม่ผ่านเพราะไม่มี `secrets.h` | คัดลอกจาก `src/secrets.example.h` ตามหัวข้อ "ตั้งค่า API key" |
| Flash ใกล้เต็ม | ตอนนี้ใช้ประมาณ 81% (WiFi + HTTPS ใช้พื้นที่มาก) ควรระวังเมื่อเพิ่มฟีเจอร์ |

## 9. ข้อควรระวังด้านฮาร์ดแวร์

- GPIO ทน 3.3 V เท่านั้น ห้ามต่อสัญญาณ 5 V ตรง
- ตั้ง `digitalWrite(pin, HIGH)` ก่อน `pinMode(OUTPUT)` กับรีเลย์ Active LOW เพื่อไม่ให้รีเลย์ดีดตอนบูต (โค้ดทำไว้แล้ว)
- ห้ามใช้ GPIO6–11 (ต่อกับ Flash) และระวังขา strapping (0, 2, 5, 12, 15)
- ดูรายละเอียดเพิ่มที่ [ESP32DevkitBoard.md](ESP32DevkitBoard.md)
