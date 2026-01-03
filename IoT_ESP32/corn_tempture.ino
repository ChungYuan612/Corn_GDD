#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> // 這是連線 Google HTTPS 必須的
#include "DHT.h"

// ================== 使用者設定區 (請修改這裡) ==================

// 1. 設定 WiFi ★
const char* ssid = "Cyperion2048";
const char* password = "26bc180125f2";

// 2. 設定 Google Apps Script ID ★
// 部署網址：https://script.google.com/macros/s/AKfycbwgYl8lRJ3VQgvTXIcUmbkBuqAk5qAkH6k-mJqFGPfwEOG31foeuSb2JFM2HYxxvy3aEg/exec
// 只要 /s/ 和 /exec 中間那串很長的亂碼 ID
const String GAS_ID = "AKfycbwgYl8lRJ3VQgvTXIcUmbkBuqAk5qAkH6k-mJqFGPfwEOG31foeuSb2JFM2HYxxvy3aEg"; 

// 3. 設定感測器 ★
#define DHTPIN 2      // 對應 NodeMCU 的 D4 腳位 (GPIO 2)
#define LED_PIN 0  // 提示LED
#define DHTTYPE DHT11 // 藍色尼哥

// =============================================================

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  // 連接 WiFi
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi");
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    timeout++;
    if(timeout > 20) { // 如果 10 秒連不上就重啟
      Serial.println("\nWiFi connect failed, restarting...");
      ESP.restart();
    }
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // 1. 讀取數據
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // 攝氏溫度

  // 檢查讀取是否失敗
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  Serial.print("Temp: "); Serial.print(t);
  Serial.print(" Hum: "); Serial.println(h);

  // 2. 上傳數據到 Google Sheet
  sendDataToGoogle(t, h);
  digitalWrite(LED_PIN, HIGH);
  delay(400); 
  digitalWrite(LED_PIN, LOW);

  // 3. 等待時間
  // 這裡設定為 6 H (6x60x60x1000)
  Serial.println("Waiting 6 hours");
  delay(21600000-400); //扣掉upload的time
}

void sendDataToGoogle(float temp, float hum) {
  WiFiClientSecure client;
  
  // 【關鍵】忽略 SSL 憑證檢查
  // Google 的憑證會定期更換，設定 setInsecure 雖然安全性較低，
  // 但能確保你的 ESP8266 不會因為憑證過期而無法上傳，這是最穩定的做法。
  client.setInsecure(); 
  
  HTTPClient http;
  
  // 組合 URL
  // 最終網址長相: https://script.google.com/macros/s/ID/exec?temp=25.5&hum=60.2
  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?temp=" + String(temp) + "&hum=" + String(hum);
  
  Serial.print("Uploading data... ");
  
  // 開始連線
  if (http.begin(client, url)) {
    
    // 這一行會送出資料並跟隨 Google 的重新導向 (Redirect)
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    int httpCode = http.GET(); // 發送 GET 請求
    
    if (httpCode > 0) {
      Serial.printf("[HTTP] Code: %d\n", httpCode);
      // HTTP 200 代表成功
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response: " + payload);
      }
    } else {
      Serial.printf("[HTTP] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("[HTTP] Unable to connect");
  }
}