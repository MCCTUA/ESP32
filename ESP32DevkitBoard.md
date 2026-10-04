# ESP32 DevKit Board (DOIT ESP32 DevKit V1 / V2)

> หมายเหตุ: `platformio.ini` ในโปรเจกต์นี้ใช้ `board = esp32doit-devkit-v1` ซึ่งเป็นโปรไฟล์ที่ใช้กับบอร์ด ESP32 DevKit ทั่วไป (โมดูล ESP-WROOM-32) รวมถึงรุ่นที่ขายในชื่อ "V2" ด้วย
> ความต่างระหว่าง V1 / V2 ส่วนใหญ่อยู่ที่จำนวนขา (30 / 38 ขา), ชิป USB-UART และตำแหน่งขา ควรเทียบกับตัวบอร์ดจริงอีกครั้ง

## 1. ภาพรวม

| หัวข้อ | รายละเอียด |
|---|---|
| โมดูล | ESP-WROOM-32 (ESP32-D0WDQ6, Espressif) |
| CPU | Xtensa LX6 Dual-core 32-bit, สูงสุด 240 MHz |
| SRAM | 520 KB |
| Flash | 4 MB (SPI Flash, ในโมดูล) |
| Wi-Fi | 802.11 b/g/n (2.4 GHz) |
| Bluetooth | Bluetooth v4.2 BR/EDR + BLE |
| แรงดันทำงาน (logic) | 3.3 V (**ขา GPIO ไม่ทน 5 V**) |
| ไฟเลี้ยงบอร์ด | 5 V ผ่าน USB หรือขา VIN (5 V) |
| Regulator | AMS1117-3.3 (จ่ายได้ราว 800 mA) |
| USB-UART | CP2102 หรือ CH340 (ขึ้นกับล็อตที่ผลิต) |
| ปุ่ม | EN (Reset), BOOT (GPIO0) |
| LED บนบอร์ด | LED ไฟเลี้ยง และ LED ผู้ใช้ที่ GPIO2 |
| ขั้วต่อ USB | Micro-USB (บางรุ่นเป็น USB-C) |
| ขนาด | ประมาณ 55 x 28 mm |

## 2. Peripherals ในตัวชิป

- GPIO สูงสุด 34 ขา (ที่ใช้ได้จริงบนบอร์ดประมาณ 25 ขา)
- ADC 12-bit: ADC1 (8 ช่อง), ADC2 (10 ช่อง)
- DAC 8-bit 2 ช่อง: GPIO25, GPIO26
- Touch sensor 10 ช่อง
- PWM (LEDC) 16 ช่อง
- UART 3 ชุด, SPI 3 ชุด (ใช้งานได้ 2: HSPI, VSPI), I2C 2 ชุด, I2S 2 ชุด
- CAN (TWAI) 1 ชุด
- RTC, Hall sensor, temperature sensor ในตัว
- Deep sleep ใช้กระแสต่ำประมาณ 10 µA

## 3. ขา (Pinout) ที่ใช้บ่อย

| ฟังก์ชัน | ขาเริ่มต้น |
|---|---|
| I2C SDA / SCL | GPIO21 / GPIO22 |
| SPI (VSPI) MOSI / MISO / SCK / SS | GPIO23 / GPIO19 / GPIO18 / GPIO5 |
| SPI (HSPI) MOSI / MISO / SCK / SS | GPIO13 / GPIO12 / GPIO14 / GPIO15 |
| UART0 TX / RX (ต่อ USB) | GPIO1 / GPIO3 |
| UART2 TX / RX | GPIO17 / GPIO16 |
| DAC | GPIO25, GPIO26 |
| Touch | GPIO4, 0, 2, 15, 13, 12, 14, 27, 33, 32 |
| LED บนบอร์ด | GPIO2 |

### ข้อควรระวังของขา

| กลุ่มขา | ข้อจำกัด |
|---|---|
| GPIO34, 35, 36 (VP), 39 (VN) | เป็น **Input only**, ไม่มี pull-up/pull-down ภายใน |
| GPIO6 – GPIO11 | ต่อกับ Flash ภายใน **ห้ามใช้** |
| GPIO0, 2, 5, 12, 15 | เป็น Strapping pins มีผลตอนบูต ถ้าต่อวงจรภายนอกอาจทำให้บูต/อัปโหลดไม่ได้ |
| GPIO12 | ถ้าถูกดึง HIGH ตอนบูต จะตั้งแรงดัน flash เป็น 1.8 V และบูตไม่ขึ้น |
| GPIO0 | ต่ำ (LOW) ตอนบูต = เข้าโหมดดาวน์โหลดโปรแกรม |
| ADC2 (GPIO0, 2, 4, 12–15, 25–27) | ใช้ร่วมกับ Wi-Fi ไม่ได้ ถ้าเปิด Wi-Fi ให้ใช้ ADC1 (GPIO32–39) แทน |

## 4. ขาไฟและควบคุม

| ขา | หน้าที่ |
|---|---|
| 3V3 | ไฟ 3.3 V ออก |
| VIN / 5V | ไฟ 5 V เข้า/ออก (ตามที่จ่ายผ่าน USB) |
| GND | กราวด์ |
| EN | Reset (active LOW) |

## 5. การตั้งค่าใน PlatformIO (โปรเจกต์นี้)

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
upload_port = /dev/cu.usbserial-210
monitor_port = /dev/cu.usbserial-210
monitor_speed = 115200
```

ไลบรารีที่ใช้: ArduinoJson, Adafruit SSD1306, Adafruit GFX, WiFiManager, PubSubClient

## 6. การอัปโหลดโปรแกรม

1. ต่อสาย USB (ใช้สายที่รองรับข้อมูล ไม่ใช่สายชาร์จอย่างเดียว)
2. macOS: ตรวจพอร์ตด้วย `ls /dev/cu.*` (เช่น `/dev/cu.usbserial-210`)
3. ถ้าเป็นชิป CH340 และเครื่องไม่เห็นพอร์ต ให้ติดตั้งไดรเวอร์ CH340 / CP210x
4. อัปโหลด: `pio run -t upload`
5. ถ้าขึ้น "Connecting......" ค้าง ให้กดปุ่ม **BOOT** ค้างไว้จนเริ่มอัปโหลด แล้วปล่อย
6. ดู Serial: `pio device monitor` (115200 baud)

## 7. ตัวอย่างอุปกรณ์ที่ต่อกับโปรเจกต์นี้

| อุปกรณ์ | การเชื่อมต่อ |
|---|---|
| OLED SSD1306 (I2C) | VCC → 3V3, GND → GND, SDA → GPIO21, SCL → GPIO22 (address มักเป็น 0x3C) |

## 8. Relay

**ชนิด: Active LOW** (สั่ง `LOW` = รีเลย์ ON, สั่ง `HIGH` = รีเลย์ OFF)

| Relay | GPIO |
|---|---|
| relay1 | GPIO17 |
| relay2 | GPIO18 |
| relay3 | GPIO4 |

ตัวอย่างโค้ด:

```cpp
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

const uint8_t RELAY_PINS[3] = {17, 18, 4};

void setup() {
  for (uint8_t pin : RELAY_PINS) {
    digitalWrite(pin, RELAY_OFF);  // ตั้งค่าก่อน pinMode เพื่อไม่ให้รีเลย์ดีดตอนเริ่ม
    pinMode(pin, OUTPUT);
  }
}
```

ข้อสังเกต:
- ตั้ง `digitalWrite(pin, HIGH)` ก่อน `pinMode(OUTPUT)` เสมอ ไม่เช่นนั้นรีเลย์อาจทำงานชั่วขณะตอนบูต
- GPIO17 ปกติเป็น UART2 TX และ GPIO18 เป็น SCK ของ VSPI ถ้าใช้รีเลย์แล้วจะใช้หน้าที่เหล่านั้นไม่ได้
- GPIO4 อยู่บน ADC2 จึงใช้อ่านค่า analog ร่วมกับ Wi-Fi ไม่ได้ (ใช้เป็น output ปกติ)
- ขา OLED (GPIO21/22) ไม่ชนกับขารีเลย์

## 9. ข้อควรระวังทั่วไป

- GPIO ทน 3.3 V เท่านั้น ถ้าต่ออุปกรณ์ 5 V ต้องใช้ level shifter หรือวงจรแบ่งแรงดัน
- GPIO แต่ละขาจ่ายกระแสได้ประมาณ 12 mA (สูงสุดไม่ควรเกิน 20 mA)
- ขณะ Wi-Fi ส่งสัญญาณ ESP32 กินกระแสสูงถึง 240 mA ขึ้นไป ควรใช้แหล่งจ่ายที่เพียงพอ (USB พอร์ตคอมพิวเตอร์บางพอร์ตไม่พอ)
- ถ้าบอร์ดรีเซ็ตเองบ่อย (brownout) ให้ลองเปลี่ยนสาย USB หรือเพิ่มตัวเก็บประจุ 10–100 µF ที่ขา 3V3 / EN
- ห้ามจ่าย 5 V เข้าขา 3V3 โดยตรง

## 10. แหล่งข้อมูลอ้างอิง

- ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
- ESP32 Technical Reference Manual: https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
- PlatformIO board: https://docs.platformio.org/en/latest/boards/espressif32/esp32doit-devkit-v1.html
