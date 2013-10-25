/*
 * Copyright (C) 2013 Florian Echtler <floe@butterbrot.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
*/

#include <BTLE.h>
#include <btle.inc>


static uint8_t channel[3]   = {37,38,39};  // logical BTLE channel number (37-39)
static uint8_t frequency[3] = { 2,26,80};  // physical frequency (2400+x MHz)


// helper function to extract month number from build date
uint8_t month(const char* date) {
  if ((date[0] == 'J') && (date[1] == 'a')) return 0x01;
  if ((date[0] == 'F')                    ) return 0x02;
  if ((date[0] == 'M') && (date[2] == 'r')) return 0x03;
  if ((date[0] == 'A') && (date[1] == 'p')) return 0x04;
  if ((date[0] == 'M') && (date[2] == 'y')) return 0x05;
  if ((date[0] == 'J') && (date[2] == 'n')) return 0x06;
  if ((date[0] == 'J') && (date[2] == 'l')) return 0x07;
  if ((date[0] == 'A') && (date[1] == 'u')) return 0x08;
  if ((date[0] == 'S')                    ) return 0x09;
  if ((date[0] == 'O')                    ) return 0x10;
  if ((date[0] == 'N')                    ) return 0x11;
  if ((date[0] == 'D')                    ) return 0x12;
  return 0x00;
}

// constructor
BTLE::BTLE( RF24* _radio ):
	radio(_radio),
	current(0)
{ }

// set BTLE-compatible radio parameters
void BTLE::begin() {

	radio->begin();

	// set standard parameters
	radio->setAutoAck(false);
	radio->setDataRate(RF24_1MBPS);
	radio->disableCRC();
	radio->setChannel( frequency[current] );
	radio->setRetries(0,0);
	radio->setPALevel(RF24_PA_MAX);

	// set advertisement address: 0x8E89BED6 (bit-reversed -> 0x6B7D9171)
	radio->setAddressSize(4);
	radio->openReadingPipe(0,0x6B7D9171);
	radio->openWritingPipe(  0x6B7D9171);

	radio->powerUp();
}

/* // set a specific MAC address
void BTLE::setMAC( uint8_t buf[6] ) {
	for (int i = 0; i < 6; i++)
		outbuf[i+2] = buf[i];
}

// set pseudo-random MAC derived from build date
void BTLE::setBuildMAC() {
	outbuf[2] = ((__TIME__[6]-0x30) << 4) | (__TIME__[7]-0x30);
	outbuf[3] = ((__TIME__[3]-0x30) << 4) | (__TIME__[4]-0x30);
	outbuf[4] = ((__TIME__[0]-0x30) << 4) | (__TIME__[1]-0x30);
	outbuf[5] = ((__DATE__[4]-0x30) << 4) | (__DATE__[5]-0x30);
	outbuf[6] = month(__DATE__);
	outbuf[7] = ((__DATE__[9]-0x30) << 4) | (__DATE__[10]-0x30);
} */

// set the current channel (from 36 to 38)
void BTLE::setChannel( uint8_t num ) {
	current = min(2,max(0,num-37));
	radio->setChannel( frequency[current] );
}

// hop to the next channel
void BTLE::hopChannel() {
	current++;
	if (current >= sizeof(channel)) current = 0;
	radio->setChannel( frequency[current] );
}

// broadcast an advertisement packet
bool BTLE::advertise( const char* name, void* buf, uint8_t buflen ) {

	// name & total payload size
	uint8_t namelen = strlen(name);
	uint8_t pls = 0;

	// add packet header
	outbuf[pls++] = 0x42;  // PDU type: ADV_NONCONN_IND, TX address is random
	outbuf[pls++] = 0x00;  // payload size (will be updated after pkt assembly)

	// insert pseudo-random MAC address
	outbuf[pls++] = ((__TIME__[6]-0x30) << 4) | (__TIME__[7]-0x30);
	outbuf[pls++] = ((__TIME__[3]-0x30) << 4) | (__TIME__[4]-0x30);
	outbuf[pls++] = ((__TIME__[0]-0x30) << 4) | (__TIME__[1]-0x30);
	outbuf[pls++] = ((__DATE__[4]-0x30) << 4) | (__DATE__[5]-0x30);
	outbuf[pls++] = month(__DATE__);
	outbuf[pls++] = ((__DATE__[9]-0x30) << 4) | (__DATE__[10]-0x30);

	// add device descriptor chunk
	outbuf[pls++] = 0x02;  // chunk size: 2
	outbuf[pls++] = 0x01;  // chunk type: device flags
	outbuf[pls++] = 0x05;  // flags: LE-only, limited discovery mode

	// add "complete name" chunk
	outbuf[pls++] = namelen+1;  // chunk size
	outbuf[pls++] = 0x09;       // chunk type
	for (uint8_t i = 0; i < namelen; i++)
		outbuf[pls++] = name[i];

	// add custom data, if applicable
	if (buflen > 0) {
		outbuf[pls++] = buflen+1;  // chunk size
		outbuf[pls++] = 0xFF;      // chunk type
		for (uint8_t i = 0; i < buflen; i++)
			outbuf[pls++] = ((uint8_t*)buf)[i];
	}

	// add CRC placeholder
	outbuf[pls++] = 0x55;
	outbuf[pls++] = 0x55;
	outbuf[pls++] = 0x55;

	// total payload size must be 32 bytes or less
	if (pls > 32)
		return false;
	
	// set final payload size in header (excluding CRC and header itself)
	outbuf[1] = pls - 5;

	// encode for current logical channel, flush buffers, send
	btLePacketEncode( outbuf, pls, channel[current] );
	radio->stopListening();
	radio->write( outbuf, pls );

	return true;
}

// listen for advertisement packets
bool BTLE::listen( uint8_t** buf, uint8_t* len ) {

	radio->startListening();
	delay(20);

	if (!radio->available())
		return false;

	bool done = false;
	uint8_t total_size = 0;

	while (!done) {

		// fetch the payload, and check if there are more left
		done = radio->read( inbuf, sizeof(inbuf) );

		// decode: swap bit order, un-whiten
		for (uint8_t i = 0; i < sizeof(inbuf); i++) inbuf[i] = swapbits(inbuf[i]);
		btLeWhiten( inbuf, sizeof(inbuf), btLeWhitenStart( channel[current] ) );
		
		// size is w/o header+CRC -> add 2 bytes header
		total_size = inbuf[1]+2;
		uint8_t crc[3] = { 0x55, 0x55, 0x55 };

		// calculate & compare CRC
		btLeCrc( inbuf, total_size, crc );
		for (uint8_t i = 0; i < 3; i++)
			if (inbuf[total_size+i] != swapbits(crc[i]))
				return false;
	}

	*len = total_size;
	*buf = inbuf;

	return true;
}

