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
  uint8_t buf[32];
  btle.listen(buf,&len);
  btle.hopChannel();
}

