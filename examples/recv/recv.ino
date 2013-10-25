#include <SPI.h>
#include <RF24.h>
#include <BTLE.h>

RF24 radio(9,10);

BTLE btle(&radio);

void setup() {

  Serial.begin(9600);
  while (!Serial) { }
  Serial.println("BTLE advertisement receiver");

  btle.begin();
}

void loop() {
  
  uint8_t len;
  uint8_t* buf;
  
  Serial.print("Listening... ");
  
  if (btle.listen(&buf,&len)) {
    Serial.print("Got payload: ");
    for (uint8_t i = 0; i < len; i++) { Serial.print(buf[i],HEX); Serial.print(" "); }
  }
  
  Serial.println("done.");
  btle.hopChannel();
}

