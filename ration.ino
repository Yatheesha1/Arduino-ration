#include <SPI.h>
#include <Ethernet.h>


byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = { 192, 168, 1, 6 };// ip ethernet
byte serverName[] = { 192, 168, 1, 4 };//ip system
EthernetClient client;


#include <MFRC522.h>
#define RST_PIN   8
#define SS_PIN    7
MFRC522 mfrc522(SS_PIN, RST_PIN);
#define NEW_UID {0xDE, 0xAD, 0xBE, 0xEF}
MFRC522::MIFARE_Key key;


float value = 0.0;
float temp;



byte sensorInterrupt = 0;
byte flowPin       = 2;
int relayPin       = 4;
float calibrationFactor = 2;
volatile byte pulseCount;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;




void setup() {


  pinMode(flowPin, INPUT);
  pinMode(relayPin, OUTPUT);



  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  Ethernet.begin(mac, ip);
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {



  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return;
  }




  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }


  delay(2000);

  if (client.connect(serverName, 80))
  {
    Serial.println("connected...");
    Serial.println("ARDUINO: forming HTTP request message");
    client.print("GET /rfidration_kerosene/add.php?temp=");
    for (byte i = 0; i < mfrc522.uid.size; i++) {

      client.print(mfrc522.uid.uidByte[i], HEX);

    }
    client.println();
    client.println(" HTTP/1.1");
    client.println("Host: localhost");
    client.println();
    Serial.println("ARDUINO: HTTP message sent");

    String c = client.readString();
    if (c != "")
    {
      temp = c.toFloat();
      Serial.println(temp);
      delay(1000);
      client.stop();
      liquid();
    }
    temp = 0;
  }
  else
  {
    Serial.println("connection failure");
  }


}




void liquid()
{
  value = temp * 1000;
  digitalWrite(relayPin, LOW);
  {
    pulseCount        = 0;
    flowRate          = 0.0;
    flowMilliLitres   = 0;
    totalMilliLitres  = 0;
    oldTime           = 0;
    if (value > 0)
    {
      Serial.println("Started");
      digitalWrite(relayPin, HIGH);
      while (value > totalMilliLitres)
      {
        if ((millis() - oldTime) > 1000)   // Only process counters once per second
        {
          detachInterrupt(sensorInterrupt);
          flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
          oldTime = millis();
          flowMilliLitres = (flowRate / 60) * 1000;
          totalMilliLitres += flowMilliLitres;
          Serial.print("Quantity: ");
          Serial.print(totalMilliLitres);
          Serial.println(" mL");
          pulseCount = 0;
          attachInterrupt(sensorInterrupt, pulseCounter, RISING);
        }
      }
      Serial.println("Done\n");
    }
    value = 0;
    digitalWrite(relayPin, LOW);
  }
}
/*
Insterrupt Service Routine
 */
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}









