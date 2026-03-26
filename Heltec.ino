#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <RH_RF95.h>

// ── HELTEC V3 RFM95W PINS ─────────────────────────────────────────
// Heltec V3 has SX1262 built in — we use its SPI bus for the external
// RFM95W if wired in, otherwise use the built-in LoRa via LoRaWan_APP
// For simplicity use the Heltec built-in LoRa with matching settings

#include <LoRaWan_APP.h>

// ── WIFI CREDENTIALS ─────────────────────────────────────────────
const char* ssid     = "Michelle iPhone";
const char* password = "Mitchkay123";

// ── FLASK SERVER ──────────────────────────────────────────────────
const char* serverURL = "http://172.20.10.4:5000/data";

// ── LORA SETTINGS (must match Arduino sender exactly) ─────────────
#define RF_FREQUENCY          915000000
#define LORA_BANDWIDTH        0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE       1
#define LORA_PREAMBLE_LENGTH  8
#define LORA_SYMBOL_TIMEOUT   0
#define BUFFER_SIZE           64

char rxpacket[BUFFER_SIZE];
static RadioEvents_t RadioEvents;
bool received = false;

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0';
    received = true;
    Serial.print("[LoRa RECEIVED] ");
    Serial.print(rxpacket);
    Serial.print("  RSSI: ");
    Serial.println(rssi);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Connect WiFi
    Serial.print("[WiFi] Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());

    // Initialize LoRa
    RadioEvents.RxDone = OnRxDone;
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetRxConfig(
        MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
        LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
        LORA_SYMBOL_TIMEOUT, false, 0, true,
        0, 0, false, true
    );
    Radio.Rx(0);
    Serial.println("[LoRa] Listening...");
}

void loop() {
    Radio.IrqProcess();

    if (received) {
        received = false;

        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            http.begin(serverURL);
            http.addHeader("Content-Type", "text/plain");
            int code = http.POST(rxpacket);
            Serial.print("[HTTP] Response: ");
            Serial.println(code == 200 ? "200 OK" : String(code));
            http.end();
        } else {
            Serial.println("[WiFi] Disconnected — skipping POST");
        }

        Radio.Rx(0);
    }
}