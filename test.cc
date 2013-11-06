#include <stdint.h>
#include <stdio.h>
#include "btle.inc"

uint8_t testbuf[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };


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
