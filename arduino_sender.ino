#include <SPI.h>
#include <LoRa.h>
#include <Arduino_LSM9DS1.h>

// ── RFM95W PIN DEFINITIONS ────────────────────────────────────────
#define RFM95_CS   10   // NSS → D10
#define RFM95_RST   9   // RST → D9
#define RFM95_INT   2   // DIO0 → D2
#define RF95_FREQ  915E6

// ── MOISTURE PINS ─────────────────────────────────────────────────
#define MOISTURE_POWER  7
#define MOISTURE_SIG   A0

// ── THRESHOLDS ────────────────────────────────────────────────────
#define GYRO_DANGER     15.0
#define MOISTURE_DANGER  800

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize LoRa
    LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);
    if (!LoRa.begin(RF95_FREQ)) {
        Serial.println("[LoRa] RFM95W init FAILED — check wiring");
        while (1);
    }
    LoRa.setTxPower(23);
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    Serial.println("[LoRa] RFM95W ready at 915MHz");

    // Initialize IMU
    if (!IMU.begin()) {
        Serial.println("[IMU] LSM9DS1 init FAILED");
        while (1);
    }
    Serial.println("[IMU] LSM9DS1 ready");

    // Moisture power pin
    pinMode(MOISTURE_POWER, OUTPUT);
    digitalWrite(MOISTURE_POWER, LOW);
}

void loop() {
    // ── READ GYROSCOPE ────────────────────────────────────────────
    float gx, gy, gz;
    float gyroMax = 0.0;

    if (IMU.gyroscopeAvailable()) {
        IMU.readGyroscope(gx, gy, gz);
        gyroMax = max(abs(gx), max(abs(gy), abs(gz)));
    }

    // ── READ MOISTURE ─────────────────────────────────────────────
    int moisture = 0;
    digitalWrite(MOISTURE_POWER, HIGH);
    delay(100);
    moisture = analogRead(MOISTURE_SIG);
    digitalWrite(MOISTURE_POWER, LOW);

    // ── DETERMINE RISK ────────────────────────────────────────────
    String risk = "SAFE";
    if (gyroMax > GYRO_DANGER || moisture > MOISTURE_DANGER) {
        risk = "DANGER";
    }

    // ── BUILD PACKET ──────────────────────────────────────────────
    String packet = "GYRO:" + String(gyroMax, 1) +
                    ",MOISTURE:" + String(moisture) +
                    ",RISK:" + risk;

    Serial.print("[SENDING] ");
    Serial.println(packet);

    // ── TRANSMIT VIA LORA ─────────────────────────────────────────
    LoRa.beginPacket();
    LoRa.print(packet);
    LoRa.endPacket();
    Serial.println("[LoRa] Packet sent");

    delay(2000);
}
