#pragma once

// ค่าตั้งค่าการแจ้งเตือน Telegram (แก้ที่นี่ ไม่ต้องแก้ logic ใน main.cpp)

// แจ้งเตือนเมื่อ AQI >= ค่านี้ (1 ดีมาก, 2 พอใช้, 3 ปานกลาง, 4 แย่, 5 แย่มาก) ใส่ 0 = ปิด
constexpr int ALERT_AQI_LEVEL = 4;

// แจ้งเตือนเมื่อ PM2.5 >= ค่านี้ (µg/m3) ใส่ 0 = ปิด
constexpr float ALERT_PM25_UGM3 = 37.5f;

// ต้องลดต่ำกว่าเกณฑ์ PM2.5 เท่านี้ (µg/m3) ถึงจะถือว่ากลับเป็นปกติ กันแจ้งซ้ำรัวๆ เมื่อค่าแกว่งรอบเกณฑ์
constexpr float ALERT_PM25_HYSTERESIS = 2.0f;

// ส่งสรุปสภาพอากาศเป็นรอบ ทุกกี่มิลลิวินาที ใส่ 0 = ปิด
constexpr unsigned long REPORT_INTERVAL_MS = 60UL * 60UL * 1000UL;

// NTP: เวลาจริงตามเขตเวลา Asia/Bangkok (POSIX TZ: ICT = UTC+7 ไม่มี DST)
constexpr const char *NTP_TZ = "ICT-7";
constexpr const char *NTP_SERVER1 = "th.pool.ntp.org";
constexpr const char *NTP_SERVER2 = "pool.ntp.org";
