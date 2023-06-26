/*
MIT License

Copyright (c) 2021 Kazuki Ota

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include "main.h"
#include "base64.h"
#include <stdio.h>

extern void string2byte(const char *input, uint32_t inputLength,
		uint8_t *output);
extern void byte2string(const uint8_t *input, uint32_t inputLength,
		char *output);
extern uint32_t sizeof_array(const char *input);

//const char alphaTable[64] = { 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
//		0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54,
//		0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
//		0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
//		0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x30, 0x31, 0x32, 0x33,
//		0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2B, 0x2F };

const char alphaTable[64] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
		'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
		'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

const uint8_t charTable[128] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E,
		0xFF, 0xFF, 0xFF, 0x3F, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
		0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02,
		0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
		0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
		0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
		0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

void a3to4(uint8_t *a3, uint8_t *a4) {
	a4[0] = (a3[0] & 0xFC) >> 2;
	a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xF0) >> 4);
	a4[2] = ((a3[1] & 0x0F) << 2) + ((a3[2] & 0xC0) >> 6);
	a4[3] = a3[2] & 0x3F;
}

void a4to3(uint8_t *a4, uint8_t *a3) {
	a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
	a3[1] = ((a4[1] & 0x0F) << 4) + ((a4[2] & 0x3C) >> 2);
	a3[2] = ((a4[2] & 0x03) << 6) + a4[3];
}

void BASE64_encode(const char *input, char *output) {
	uint32_t i = 0;
	uint32_t j = 0;
	uint8_t a3[3];
	uint8_t a4[4];
	uint32_t inputLength = sizeof_array(input);
	uint8_t buff[inputLength];
	string2byte(input, inputLength, buff);

	while (inputLength--) {
		a3[i++] = buff[j++];

		if (i == 3) {
			a3to4(a3, a4);

			for (i = 0; i < 4; i++) {
				*output++ = alphaTable[a4[i]];
			}

			i = 0;
		}
	}

	if (i) {
		for (uint32_t j = i; j < 3; j++) {
			a3[j] = '\0';
		}

		a3to4(a3, a4);

		for (uint32_t j = 0; j < i + 1; j++) {
			*output++ = alphaTable[a4[j]];
		}

		while (i++ < 3) {
			*output++ = '=';
		}
	}

	*output = '\0';
}

uint32_t BASE64_encodeLength(uint32_t inputLength) {
	return (inputLength + 2 - ((inputLength + 2) % 3)) / 3 * 4 + 1;
}

void BASE64_decode(const char *input, char *output) {
	size_t inputLength = strlen(input);
	uint32_t i = 0;
	uint8_t a3[3];
	uint8_t a4[4];

	while (inputLength--) {
		if (*input == '=') {
			break;
		}

		a4[i++] = *input++;

		if (i == 4) {
			for (i = 0; i < 4; i++) {
				a4[i] = charTable[a4[i]];
			}

			a4to3(a4, a3);

			for (i = 0; i < 3; i++) {
				*output++ = a3[i];
			}

			i = 0;
		}
	}

	if (i) {
		for (uint32_t j = i; j < 4; j++) {
			a4[j] = '\0';
		}

		for (uint32_t j = 0; j < 4; j++) {
			a4[j] = charTable[a4[j]];
		}

		a4to3(a4, a3);

		for (uint32_t j = 0; j < i - 1; j++) {
			*output++ = a3[j];
		}
	}

	*output = '\0';
}

uint32_t BASE64_decodeLength(const char *input) {
	uint32_t inputLength = strlen(input);
	uint32_t eq = 0;

	for (uint32_t i = inputLength - 1; input[i] == '='; i--) {
		eq++;
	}

	return 6 * inputLength / 8 - eq;
}
