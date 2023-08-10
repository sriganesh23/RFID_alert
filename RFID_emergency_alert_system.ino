//This is the code to run the Node MCU for emergency alert system with RFID
//This code belong to GUBBALA SRI GANESH
// 21BEC7184 - Vellore Institute of Technology, Andhra Pradesh

#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

//-----------------------------------------
#define RST_PIN  D3       // RFID RST Pin
#define SS_PIN   D4       // RFID SS Pin
#define BUZZER   D1       // BUZZER Pin
#define LED_PIN  D0       // LED Pin
//-----------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
//-----------------------------------------
int blockNum = 2;
byte bufferLen = 18;
byte readBlockData[18];
//-----------------------------------------
String card_holder_name;
const String sheet_url = "https://script.google.com/macros/s/AKfycbxDP1Jy4EXOAJ4fXyFk4l5OWHZrzII6Hzm5cX0WtjDJ2wpkdIc3BaacowYVvahUqu5B/exec?name=";
//-----------------------------------------
// 22 D6 3F 7A CA 1E 3B 04 40 02 A1 AF 49 B4 02 8E 8D 0E F9 43
const uint8_t fingerprint[20] = {0x22, 0xD6, 0x3F, 0x7A, 0xCA, 0x1E, 0x3B, 0x04, 0x40, 0x02, 0xA1, 0xAF, 0x49, 0xB4, 0x02, 0x8E, 0x8D, 0x0E, 0xF9, 0x43};
//-----------------------------------------
#define WIFI_SSID "Ecs2"
#define WIFI_PASSWORD "vitapu21"
//-----------------------------------------

bool ledState = false;

/****************************************************************************************************
 * setup() function
 ****************************************************************************************************/
void setup()
{
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
  }

  pinMode(BUZZER, OUTPUT);

  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(LED_PIN, OUTPUT);
}

/****************************************************************************************************
 * loop() function
 ****************************************************************************************************/
void loop()
{
  mfrc522.PCD_Init();

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock(blockNum, readBlockData);
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  Serial.println();
  Serial.print(F("Last data in RFID:"));
  Serial.print(blockNum);
  Serial.print(F(" --> "));
  for (int j = 0; j < 16; j++) {
    Serial.write(readBlockData[j]);
  }
  Serial.println();

  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  //delay(200);
  ledState = !ledState; // Toggle the LED state
  digitalWrite(LED_PIN, ledState); // Set the LED state
  
  delay(1000);
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    mfrc522.PICC_HaltA();
  }

  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(fingerprint);

    card_holder_name = sheet_url + String((char*)readBlockData);
    card_holder_name.trim();

    HTTPClient https;
    if (https.begin(*client, (String)card_holder_name)) {
      int httpCode = https.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      }
      else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
      delay(1000);
    }
    else {
      Serial.printf("[HTTP] Unable to connect\n");
    }
  }
}

/****************************************************************************************************
 * ReadDataFromBlock() function
 ****************************************************************************************************/
void ReadDataFromBlock(int blockNum, byte readBlockData[])
{
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println("Authentication success");
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    Serial.println("Block was read successfully");
  }
}
