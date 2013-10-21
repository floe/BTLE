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

class BTLE {

	public:

		BTLE( RF24* _radio );

		void begin(); // set BTLE-compatible radio parameters 

		/*void setMAC( uint8_t buf[6] ); // set a specific MAC address
		void setBuildMAC();            // set pseudo-random MAC derived from build date */

		void setChannel( uint8_t num ); // set the current channel (from 36 to 38)
		void hopChannel();              // hop to the next channel

		bool advertise( const char* name, void* buf, uint8_t len ); // broadcast an advertisement packet
		bool listen( void* buf, uint8_t* len );   // listen for advertisement packets

	protected:

	private:

		RF24* radio;  // pointer to the RF24 object managing the radio

		uint8_t outbuf[32];  // buffer for outgoing BTLE packet
		uint8_t inbuf[32];   // buffer for incoming BTLE packet

		uint8_t baselen;  // length of header + name
		uint8_t current;  // current channel index
};

#endif // _BTLE_H_

