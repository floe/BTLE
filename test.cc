#include <stdint.h>
#include <stdio.h>
#include "btle.inc"

uint8_t testbuf[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };


// see BT Core Spec 4.0, Section 6.B.3.1.1
void myCrc(const uint8_t* data, uint8_t len, uint8_t* dst) {

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

/* uint8_t swapbits(uint8_t a) {
	uint8_t res;
	asm volatile(
		"ldi        r25, 0x80 \n"
		"rotate_bit:          \n"
		"rol        %0        \n"
		"ror        r25       \n"
		"brcc       rotate_bit\n"
		"mov        %0, r25   \n":
		"=r" (res):
		"0" (a)
	);
	return res;
} */


//const uint8_t nibbles = { 0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15 };
//uint8_t swapbits( uint8_t in ) { return nibbles[in>>4] | (nibbles[in&0x0F]<<4); }

/*uint16_t lfsr;
uint8_t mywhiten() {
	for (uint8_t i = 0; i < 8; i++) {
		if (lfsr & 0x80)
			lfsr ^= 0x11;
		lfsr <<= 1;
	}
	return swapbits(lfsr >> 8);
}

uint16_t lfsr2;
uint8_t mywhiten2() {
	for (uint8_t i = 0; i < 8; i++) {
		if (lfsr2 & 0x100)
			lfsr2 ^= 0x8800;
		lfsr2 >>= 1;
	}
	return lfsr2 & 0xFF;
}*/

uint8_t lfsr3;
uint8_t mywhiten3() {
	uint8_t res = 0;
	for (uint8_t i = 1; i; i <<= 1) {
		if (lfsr3 & 0x01) {
			lfsr3 ^= 0x88;
			res |= i;
		}
		lfsr3 >>= 1;
	}
	return res;
}

int main() {

	//lfsr = swapbits(39 | 0x40);
	//lfsr2 = (39 | 0x40) << 8;
	lfsr3 = (39 | 0x40);

	//for (int i = 0; i < 32; i++) printf("%02hhx ",mywhiten()); printf("\n");
	//for (int i = 0; i < 32; i++) printf("%02hhx ",mywhiten2()); printf("\n");

	for (int i = 0; i < 32; i++) printf("%02hhx ",mywhiten3()); printf("\n");

	btLeWhiten(testbuf, 32, btLeWhitenStart(39));
	
	for (int i = 0; i < 32; i++) printf("%02hhx ",testbuf[i]); printf("\n");
}
