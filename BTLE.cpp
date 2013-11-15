/*
 * Copyright (C) 2013 Florian Echtler <floe@butterbrot.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
*/

#include <BTLE.h>
#include <btle.inc>


const uint8_t channel[3]   = {37,38,39};  // logical BTLE channel number (37-39)
const uint8_t frequency[3] = { 2,26,80};  // physical frequency (2400+x MHz)


// This is a rather convoluted hack to extract the month number from the build date in
// the __DATE__ macro using a small hash function + lookup table. Since all inputs are
// const, this can be fully resolved by the compiler and saves over 200 bytes of code.
#define month(m) month_lookup[ (( ((( (m[0] % 24) * 13) + m[1]) % 24) * 13) + m[2]) % 24 ]
const uint8_t month_lookup[24] = { 0,6,0,4,0,1,0,17,0,8,0,0,3,0,0,0,18,2,16,5,9,0,1,7 };


// constructor
BTLE::BTLE( RF24* _radio ):
	radio(_radio),
	current(0)
{ }

// set BTLE-compatible radio parameters
void BTLE::begin( const char* _name ) {

	name = _name;
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

// set the current channel (from 37 to 39)
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
bool BTLE::advertise( void* buf, uint8_t buflen ) {

	// name & total payload size
	uint8_t namelen = strlen(name);
	uint8_t pls = 0;

	// insert pseudo-random MAC address
	buffer.mac[0] = ((__TIME__[6]-0x30) << 4) | (__TIME__[7]-0x30);
	buffer.mac[1] = ((__TIME__[3]-0x30) << 4) | (__TIME__[4]-0x30);
	buffer.mac[2] = ((__TIME__[0]-0x30) << 4) | (__TIME__[1]-0x30);
	buffer.mac[3] = ((__DATE__[4]-0x30) << 4) | (__DATE__[5]-0x30);
	buffer.mac[4] = month(__DATE__);
	buffer.mac[5] = ((__DATE__[9]-0x30) << 4) | (__DATE__[10]-0x30);

	// add device descriptor chunk
	chunk(buffer,pls)->size = 0x02;  // chunk size: 2
	chunk(buffer,pls)->type = 0x01;  // chunk type: device flags
	chunk(buffer,pls)->data[0]= 0x05;  // flags: LE-only, limited discovery mode
	pls += 3;

	// add "complete name" chunk
	chunk(buffer,pls)->size = namelen+1;  // chunk size
	chunk(buffer,pls)->type = 0x09;       // chunk type
	for (uint8_t i = 0; i < namelen; i++)
		chunk(buffer,pls)->data[i] = name[i];
	pls += namelen+2;

	// add custom data, if applicable
	if (buflen > 0) {
		chunk(buffer,pls)->size = buflen+1;  // chunk size
		chunk(buffer,pls)->type = 0xFF;      // chunk type
		for (uint8_t i = 0; i < buflen; i++)
			chunk(buffer,pls)->data[i] = ((uint8_t*)buf)[i];
		pls += buflen+2;
	}

	// add CRC placeholder
	buffer.payload[pls++] = 0x55;
	buffer.payload[pls++] = 0x55;
	buffer.payload[pls++] = 0x55;

	// total payload size must be 24 bytes or less
	if (pls > 24)
		return false;

	// assemble header
	buffer.pdu_type = 0x42;    // PDU type: ADV_NONCONN_IND, TX address is random
	buffer.pl_size = pls + 3;  // set final payload size in header incl. MAC excl. CRC

	// encode for current logical channel, flush buffers, send
	btLePacketEncode( (uint8_t*)&buffer, pls+8, channel[current] );

	whiten( (uint8_t*)&buffer, pls+8 );
	for (int i = 0; i < pls+8; i++) ((uint8_t*)&buffer)[i] = swapbits(((uint8_t*)&buffer)[i]);

	radio->stopListening();
	radio->write( (uint8_t*)&buffer, pls+8 );

	return true;
}

// listen for advertisement packets
bool BTLE::listen() {

	radio->startListening();
	delay(20);

	if (!radio->available())
		return false;

	bool done = false;
	uint8_t total_size = 0;
	uint8_t* inbuf = (uint8_t*)&buffer;

	while (!done) {

		// fetch the payload, and check if there are more left
		done = radio->read( inbuf, sizeof(buffer) );

		// decode: swap bit order, un-whiten
		for (uint8_t i = 0; i < sizeof(buffer); i++) inbuf[i] = swapbits(inbuf[i]);
		whiten( inbuf, sizeof(buffer) );
		
		// size is w/o header+CRC -> add 2 bytes header
		total_size = inbuf[1]+2;
		uint8_t crc[3] = { 0x55, 0x55, 0x55 };

		// calculate & compare CRC
		btLeCrc( inbuf, total_size, crc );
		for (uint8_t i = 0; i < 3; i++)
			if (inbuf[total_size+i] != swapbits(crc[i]))
				return false;
	}

	return true;
}


// see BT Core Spec 4.0, Section 6.B.3.2
void BTLE::whiten( uint8_t* buf, uint8_t len ) {

	// initialize LFSR with current channel, set bit 6
	uint8_t lfsr = channel[current] | 0x40;

	while (len--) {
		uint8_t res = 0;
		// LFSR in "wire bit order"
		for (uint8_t i = 1; i; i <<= 1) {
			if (lfsr & 0x01) {
				lfsr ^= 0x88;
				res |= i;
			}
			lfsr >>= 1;
		}
		*(buf++) ^= res;
	}
}

