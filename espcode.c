#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <MFRC522.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// FastAPI endpoint
// server_ip would be the network ip of the machine and it would run
const char* apiEndpoint = "http://YOUR_SERVER_IP:YOUR_PORT/card_data";

// RFID reader pins
#define SS_PIN D2
#define RST_PIN D1

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  
  // Initialize RFID reader
  SPI.begin();
  rfid.PCD_Init();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Check if a new card is present
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String cardId = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      cardId += String(rfid.uid.uidByte[i], HEX);
    }
    
    Serial.print("Card detected: ");
    Serial.println(cardId);
    
    // Send card ID to FastAPI endpoint
    sendToFastAPI(cardId);
    
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
  
  delay(1000);
}

void sendToFastAPI(String cardId) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    
    // Construct the URL with the card ID as a query parameter
    String url = String(apiEndpoint) + "?card_id=" + cardId;
    
    Serial.print("Connecting to: ");
    Serial.println(url);
    
    http.begin(client, url);
    
    int httpResponseCode = http.POST("");  // Send an empty POST request
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
