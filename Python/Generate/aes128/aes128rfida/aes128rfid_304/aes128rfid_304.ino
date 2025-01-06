
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Crypto.h>
#include <AES.h>
#include <MemoryFree.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
Adafruit_INA219 ina219;
LiquidCrystal_I2C lcd(0x27, 16, 4);

AES128 aes128;
byte buffer[16];

byte ciphertext[304];
byte decryptedtext[304];
unsigned long encryptionTime;
unsigned long decryptionTime;

float encryptionVoltage;
float encryptionCurrent;
float decryptionVoltage;
float decryptionCurrent;

const byte keyAES128[16] = { 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 
                            0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

void encryptAndPrint(BlockCipher *cipher, const byte *key, size_t keySize, byte *plaintext, size_t plaintextSize) {
  unsigned long startTime = micros();
  cipher->setKey(key, keySize);

  for (size_t i = 0; i < plaintextSize; i += 16) {
    cipher->encryptBlock(buffer, &plaintext[i]);
    memcpy(&ciphertext[i], buffer, 16);
  }

  unsigned long endTime = micros();
  encryptionVoltage = ina219.getBusVoltage_V();
  encryptionCurrent = ina219.getCurrent_mA();
  encryptionTime = endTime - startTime;

  // Output encrypted data (ciphertext)
  Serial.print(F("Ciphertext: "));
  for (size_t i = 0; i < plaintextSize; i++) {
    if (ciphertext[i] < 0x10) Serial.print("0");
    Serial.print(ciphertext[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print(F("Encryption Completed in "));
  Serial.print(encryptionTime);
  Serial.println(F(" us"));
}

void decryptAndPrint(BlockCipher *cipher, const byte *key, size_t keySize, byte *ciphertext, size_t ciphertextSize) {
    unsigned long startdecTime = micros();
    cipher->setKey(key, keySize);

    for (size_t i = 0; i < ciphertextSize; i += 16) {
        cipher->decryptBlock(buffer, &ciphertext[i]);
        memcpy(&decryptedtext[i], buffer, 16);
    }

    unsigned long enddecTime = micros();
    decryptionVoltage = ina219.getBusVoltage_V();
    decryptionCurrent = ina219.getCurrent_mA();
    decryptionTime = enddecTime - startdecTime;

    // Output decrypted data
    Serial.print(F("Decrypted Text: "));
    for (size_t i = 0; i < ciphertextSize; i++) {
        if (decryptedtext[i] < 0x10) Serial.print("0");
        Serial.print(decryptedtext[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    Serial.print(F("Decryption Completed in "));
    Serial.print(decryptionTime);
    Serial.println(F(" us"));
}

void displayData() {
    float encryptionPower = encryptionVoltage * (encryptionCurrent / 1000);
    float decryptionPower = decryptionVoltage * (decryptionCurrent / 1000);
    int memoryUsage = 2048 - freeMemory();

    Serial.print(F("Memory Usage = "));
    Serial.print(memoryUsage);
    Serial.println(F(" B"));
    Serial.println();

    Serial.print(F("Encryption Voltage: "));
    Serial.print(encryptionVoltage);
    Serial.println(F(" V"));
    Serial.print(F("Encryption Current: "));
    Serial.print(encryptionCurrent);
    Serial.println(F(" mA"));
    Serial.print(F("Encryption Power: "));
    Serial.print(encryptionPower);
    Serial.println(F(" W"));
    Serial.println();
    
    Serial.print(F("Decryption Voltage: "));
    Serial.print(decryptionVoltage);
    Serial.println(F(" V"));
    Serial.print(F("Decryption Current: "));
    Serial.print(decryptionCurrent);
    Serial.println(F(" mA"));
    Serial.print(F("Decryption Power: "));
    Serial.print(decryptionPower);
    Serial.println(F(" W"));

    Serial.println(F("selesai"));
}

void setup() {
    Serial.begin(9600);
    SPI.begin();
    rfid.PCD_Init();
    ina219.begin();
    
    lcd.begin(16, 4);
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("RFID System Ready");
    delay(2000);
    lcd.clear();
}

void loop() {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        byte cardID[304] = {0};
        size_t idLength = rfid.uid.size;

        for (byte i = 0; i < idLength && i < 304; i++) {
            cardID[i] = rfid.uid.uidByte[i];
        }

        if (idLength < 304) {
            memset(&cardID[idLength], 0, 304 - idLength);
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RFID Detected!");
        lcd.setCursor(0, 1);
        lcd.print("Encrypting...");

        encryptAndPrint(&aes128, keyAES128, sizeof(keyAES128), cardID, 304);
        decryptAndPrint(&aes128, keyAES128, sizeof(keyAES128), ciphertext, 304);
        displayData();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Encryption Done!");
        delay(2000);
        lcd.clear();
    }
}
