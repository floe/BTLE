/*
 * Copyright (C) 2012 Dmitry Grinberg
 *
 * http://dmitry.gr/index.php?r=05.Projects&proj=11.%20Bluetooth%20LE%20fakery
 *
 * All the code as well as the research that went into this and is published
 * here is under this license: you may use it in any way you please if and
 * only if it is for non-commercial purposes, you must provide a link to this
 * page as well. Any commercial use must be discussed with me.
 *
 * Some additional comments by Florian Echtler <floe@butterbrot.org>
*/

uint8_t swapbits(uint8_t a) {

	uint8_t v = 0;

	if (a & 0x80) v |= 0x01;
	if (a & 0x40) v |= 0x02;
	if (a & 0x20) v |= 0x04;
	if (a & 0x10) v |= 0x08;
	if (a & 0x08) v |= 0x10;
	if (a & 0x04) v |= 0x20;
	if (a & 0x02) v |= 0x40;
	if (a & 0x01) v |= 0x80;

	return v;
}

 
// see BT Core Spec 4.0, Section 6.B.3.1.1
void btLeCrc(const uint8_t* data, uint8_t len, uint8_t* dst) {

	uint8_t v, t, d;

	while (len--) {

		d = *data++;
		for (v = 0; v < 8; v++, d >>= 1) {

			// t = bit 23 (highest-value) 
			t = dst[0] >> 7;

			// left-shift the entire register by one
			// (dst[0] = bits 23-16, dst[1] = bits 15-8, dst[2] = bits 7-0
			dst[0] <<= 1;
			if(dst[1] & 0x80) dst[0] |= 1;
			dst[1] <<= 1;
			if(dst[2] & 0x80) dst[1] |= 1;
			dst[2] <<= 1;

			// if the bit just shifted out (former bit 23)
			// and the incoming data bit are not equal:
			// => bit_out ^ bit_in = 1
			if (t != (d & 1)) {
				// toggle register bits (=XOR with 1) according to CRC polynom
				dst[2] ^= 0x5B; // 0b01011011 - x^6+x^4+x^3+x+1
				dst[1] ^= 0x06; // 0b00000110 - x^10+x^9
			}
		}       
	}
}


// see BT Core Spec 4.0, Section 6.B.3.2
void btLeWhiten(uint8_t* data, uint8_t len, uint8_t whitenCoeff) {

	uint8_t  m;

	while (len--) {

		for (m = 1; m; m <<= 1) {

			if (whitenCoeff & 0x80) {
 
				whitenCoeff ^= 0x11;
				(*data) ^= m;
			}
			whitenCoeff <<= 1;
		}
		data++;
	}
}


static inline uint8_t btLeWhitenStart(uint8_t chan) {
	//the value we actually use is what BT'd use left shifted one...makes our life easier
	return swapbits(chan) | 2;
}


void btLePacketEncode(uint8_t* packet, uint8_t len, uint8_t chan) {
	//length is of packet, including crc. pre-populate crc in packet with initial crc value!
	uint8_t i, dataLen = len - 3;

	btLeCrc(packet, dataLen, packet + dataLen);
	for (i = 0; i < 3; i++, dataLen++) packet[dataLen] = swapbits(packet[dataLen]);
	btLeWhiten(packet, len, btLeWhitenStart(chan));
	for (i = 0; i < len; i++) packet[i] = swapbits(packet[i]);
}

