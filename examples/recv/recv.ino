#include <SPI.h>
#include <RF24.h>
#include <BTLE.h>

RF24 radio(9,10);

BTLE btle(&radio);

void setup() {

  Serial.begin(9600);
  while (!Serial) { }
  Serial.println("BTLE advertisement receiver");

  btle.begin("");
}

void loop() {
    
  Serial.print("Listening... ");
  
  if (btle.listen()) {
    Serial.print("Got payload: ");
    for (uint8_t i = 0; i < (btle.buffer.pl_size)-6; i++) { Serial.print(btle.buffer.payload[i],HEX); Serial.print(" "); }
  }
  
  Serial.println("done.");
  btle.hopChannel();
}

