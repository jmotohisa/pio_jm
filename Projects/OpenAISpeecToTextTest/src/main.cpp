#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);


#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "certificates.h"

const char* WIFI_SSID = "0024A5B58F84";
const char* WIFI_PASSWORD = "fncu4tredt6rf";

// 接続先の証明書チェーンを検証できる
// 信頼できるルートCA証明書を設定してください。
extern const char* ROOT_CA_CERT;

void setup() {
  auto cfg = M5.config();

//  cfg.external_spk = true;    /// use external speaker (SPK HAT / ATOMIC SPK)
//cfg.external_spk_detail.omit_atomic_spk = true; // exclude ATOMIC SPK
//cfg.external_spk_detail.omit_spk_hat    = true; // exclude SPK HAT

  M5.begin(cfg);
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Serial.print("Connecting Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected");
  Serial.println(WiFi.localIP());

  // 1. 時刻同期
  configTime(0, 0, "pool.ntp.org", "time.google.com");

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 20000)) {
    Serial.println("ERROR: NTP time sync failed");
    return;
  }

  Serial.print("Current time: ");
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");

  // 2. TLS証明書検証
  WiFiClientSecure client;
  client.setCACert(ROOT_CA_CERT);
  client.setTimeout(15000);

  Serial.println("Connecting to api.openai.com:443");

  if (!client.connect("api.openai.com", 443)) {
    char errorBuffer[256] = {};
    client.lastError(errorBuffer, sizeof(errorBuffer));
    Serial.print("TLS ERROR: ");
    Serial.println(errorBuffer);
    client.stop();
    return;
  }

  Serial.println("TLS connection successful");

  // 3. APIキーなしでHTTP応答を確認
  client.println("GET /v1/models HTTP/1.1");
  client.println("Host: api.openai.com");
  client.println("Connection: close");
  client.println();

  unsigned long start = millis();
  while (!client.available() && client.connected() &&
         millis() - start < 15000) {
    delay(10);
  }

  if (client.available()) {
    Serial.print("HTTP response: ");
    Serial.println(client.readStringUntil('\n'));
  } else {
    Serial.println("No HTTP response received");
  }

  client.stop();
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}