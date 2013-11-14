/*
 * Copyright (C) 2013 Florian Echtler <floe@butterbrot.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 as published by the Free Software Foundation.
*/

#ifndef _BTLE_H_
#define _BTLE_H_

#include "Arduino.h"
#include <RF24.h>


// advertisement PDU
struct btle_adv_pdu {

	// packet header
	uint8_t pdu_type; // PDU type
	uint8_t pl_size;  // payload size

	// MAC address
	uint8_t mac[6];

	// payload (including 3 bytes for CRC)
	uint8_t payload[24];
};

// payload chunk in advertisement PDU payload
struct btle_pdu_chunk {
	uint8_t size;
	uint8_t type;
	uint8_t data[];
};

// helper macro to access chunk at specific offset
#define chunk(x,y) ((btle_pdu_chunk*)(x.payload+y))


class BTLE {

	public:

		BTLE( RF24* _radio );

		void begin( const char* _name ); // set BTLE-compatible radio parameters & name

		void setChannel( uint8_t num ); // set the current channel (from 36 to 38)
		void hopChannel();              // hop to the next channel

		bool advertise( void* buf, uint8_t len ); // broadcast an advertisement packet with optional payload
		bool listen();                            // listen for advertisement packets (if true: result = buffer)

		btle_adv_pdu buffer;  // buffer for received BTLE packet (also used for outgoing!)

	private:

		RF24* radio;       // pointer to the RF24 object managing the radio
		uint8_t current;   // current channel index
    const char* name;  // name of local device
};

#endif // _BTLE_H_

