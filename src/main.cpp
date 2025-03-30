#include <SPI.h>
#include <MFRC522.h>

#define BUZZER_PIN 26
#define RST_PIN 22
#define SS_PIN 20

MFRC522 mfrc522(SS_PIN, RST_PIN);

void parseNDEF(byte *data, int length);
String getURIFromIdentifier(byte identifier);

void setup()
{
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  while (!Serial)
    ;
  SPI.begin();
  mfrc522.PCD_Init();
}

void loop()
{
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return;

  byte ndefData[144]; // Capacity for 144 bytes
  int index = 0;

  for (byte i = 4; i < 39; i++) // Read from block 4 to block 39
  {
    byte buffer[18];
    byte size = sizeof(buffer);

    if (mfrc522.MIFARE_Read(i, buffer, &size) != MFRC522::STATUS_OK)
    {
      Serial.println("Read failed, Invalid UID or no NDEF data");
      return;
    }

    for (byte j = 0; j < 4; j++) // Read only the first 4 bytes of each block
    {
      ndefData[index++] = buffer[j];
    }
  }

  parseNDEF(ndefData, index);
  mfrc522.PICC_HaltA();

  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void parseNDEF(byte *data, int length)
{
  for (int i = 0; i < length; i++)
  {
    if (data[i] != 0x03)
    {
      continue;
    }

    byte recordHeader = data[i + 2];
    if ((recordHeader & 0xD0) != 0xD0)
    {
      continue;
    }

    byte payloadLength = data[i + 4];
    byte uriIdentifier = data[i + 6];

    String url = getURIFromIdentifier(uriIdentifier);

    for (int j = i + 7; j < i + 7 + payloadLength - 1; j++)
    {
      if (data[j] > 31 && data[j] < 127)
      {
        url += (char)data[j];
      }
    }

    Serial.print(url);
    break;
  }
}

String getURIFromIdentifier(byte identifier)
{
  switch (identifier)
  {
  case 0x00:
    return "";
  case 0x01:
    return "http://www.";
  case 0x02:
    return "https://www.";
  case 0x03:
    return "http://";
  case 0x04:
    return "https://";
  default:
    return "UnknownPrefix://";
  }
}