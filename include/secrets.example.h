#pragma once

// คัดลอกไฟล์นี้เป็น src/secrets.h แล้วใส่ค่าจริง (secrets.h ถูก ignore ไม่ขึ้น git)
#define OPENWEATHER_API_KEY "your_api_key_here"

// Telegram: สร้างบอตที่ @BotFather เพื่อรับ token, ส่งข้อความหาบอตแล้วดู chat id ที่
// https://api.telegram.org/bot<TOKEN>/getUpdates
#define TELEGRAM_BOT_TOKEN "your_bot_token_here"
#define TELEGRAM_CHAT_ID "your_chat_id_here"

// MQTT (HiveMQ public broker: broker.hivemq.com ไม่มีรหัสผ่าน ใครก็ publish/subscribe ได้)
// BOARD_ID ต้องไม่ซ้ำกันในแต่ละบอร์ด (และไม่ซ้ำคนอื่นบน public broker) — topic คือ esp32relay/<BOARD_ID>/telemetry และ .../control
// ปล่อยเป็น "" เพื่อใช้ค่าจาก MAC address ของบอร์ด (เช่น esp32-a1b2c3d4)
#define BOARD_ID "board-xxxxxxxx"
