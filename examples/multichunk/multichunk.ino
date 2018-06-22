#include <BTLE.h>
#include <RF24.h>

RF24 radio(9,10);
BTLE btle(&radio);


struct battery_level_data {
  uint16_t service_uuid;
  uint8_t battery_percentage;
};

void setup() {
  Serial.begin(57600);
  Serial.println("BTLE multichunk sender");
  btle.begin("RD"); //can be emptystring
}

void loop() {

  battery_level_data battery_data;
  battery_data.service_uuid = NRF_BATTERY_SERVICE_UUID;
  battery_data.battery_percentage = 80; //100 is maximum

  nrf_service_data temp_data;
  temp_data.service_uuid = NRF_TEMPERATURE_SERVICE_UUID;
  temp_data.value = BTLE::to_nRF_Float(35);
  
  btle.preparePacket();
  if(!btle.addChunk(0x16, sizeof(battery_data), &battery_data)) {
    Serial.println("Battery level does not fit");
  }
  if(!btle.addChunk(0x16, sizeof(temp_data), &temp_data)) {
    Serial.println("Temperature does not fit");
  }
  btle.transmitPacket();
  btle.hopChannel();
  
  delay(100);
}

