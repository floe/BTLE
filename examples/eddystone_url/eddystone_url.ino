#include <BTLE.h>
#include <RF24.h>

RF24 radio(9,10);
BTLE btle(&radio);

void setup() {
  btle.begin(""); //empty name can save several bytes and allow longer link
}

void loop() {
  eddystone_url_service_data beacon_data;
  beacon_data.service_uuid = NRF_EDDYSTONE_SERVICE_UUID;
  beacon_data.frame_type = EDDYSTONE_URL_FRAME_TYPE;
  beacon_data.tx_power = -25;
  beacon_data.url_scheme = EDDYSTONE_HTTPS_URL_SCHEME;
  beacon_data.encoded_url[0] = 'q';
  beacon_data.encoded_url[1] = 'r';
  beacon_data.encoded_url[2] = 0x03; // dot net
  beacon_data.encoded_url[3] = 'f';
  beacon_data.encoded_url[4] = 'N';
  beacon_data.encoded_url[5] = 'A';
  beacon_data.encoded_url[6] = '8';
  beacon_data.encoded_url[7] = 'Y';
  beacon_data.encoded_url[8] = 'Z';

  btle.preparePacket();
  btle.addChunk(0x16, EDDYSTONE_URL_HEADER_LENGTH + 9, &beacon_data);
  btle.transmitPacket();
  btle.hopChannel();
  
  delay(100);
}

