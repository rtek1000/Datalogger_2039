/*
 * BMP_to_USB.c
 *
 *  Created on: 13 de nov de 2021
 *      Author: r
 */

#include "BMP_to_USB.h"

#include "main.h"

const uint16_t w = 320; // image width in pixels
const uint16_t h = 240; // " height

const uint32_t imgSize = 76800; // 320 * 240 = 76800

uint32_t rowSize = 960;

uint32_t fileSize = 230454; // 230454 = (76800 * 3) + 54

void BMP_to_USB_header(uint8_t *header_buf);

void BMP_to_USB_header(uint8_t *header_buf) {

	// THE BMP FILE FORMAT:
	// http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm

	// Based on the analysis (using Okteta hex viewer) of the file created with KolourPaint in Xubuntu 20.10:
	// 320x240 (256 colors) all pixels blue

	// Some explanation from (but not working):
	// https://forum.arduino.cc/t/writing-binary-file-to-sd-creating-bmp-files/110331/7

	unsigned char bmpFileHeader[14] = { 0 };

	unsigned char bmpInfoHeader[40] = { 0 };

	bmpFileHeader[0] = 'B';
	bmpFileHeader[1] = 'M';
	bmpFileHeader[2] = (unsigned char) (fileSize); // FileSize
	bmpFileHeader[3] = (unsigned char) (fileSize >> 8); // FileSize
	bmpFileHeader[4] = (unsigned char) (fileSize >> 16); // FileSize
	bmpFileHeader[5] = (unsigned char) (fileSize >> 24); // FileSize

	for (uint8_t i = 6; i < 14; i++) {
		bmpFileHeader[i] = 0;
	}

	bmpFileHeader[10] = 54; // (14 + 40 = 54)

	bmpInfoHeader[0] = 40;
	bmpInfoHeader[1] = 0;
	bmpInfoHeader[2] = 0;
	bmpInfoHeader[3] = 0;

	bmpInfoHeader[4] = (unsigned char) (w);
	bmpInfoHeader[5] = (unsigned char) (w >> 8);
	bmpInfoHeader[6] = (unsigned char) (w >> 16);
	bmpInfoHeader[7] = (unsigned char) (w >> 24);

	bmpInfoHeader[8] = (unsigned char) (h);
	bmpInfoHeader[9] = (unsigned char) (h >> 8);
	bmpInfoHeader[10] = (unsigned char) (h >> 16);
	bmpInfoHeader[11] = (unsigned char) (h >> 24);

	bmpInfoHeader[12] = 1; // Planes
	bmpInfoHeader[13] = 0; // Planes

	bmpInfoHeader[14] = 24; // Bits Per Pixel
	bmpInfoHeader[15] = 0; // Bits Per Pixel

	bmpInfoHeader[16] = 0; // Compression
	bmpInfoHeader[17] = 0; // Compression
	bmpInfoHeader[18] = 0; // Compression
	bmpInfoHeader[19] = 0; // Compression

	bmpInfoHeader[20] = 0; // ImageSize
	bmpInfoHeader[21] = 44; // ImageSize
	bmpInfoHeader[22] = 1; // ImageSize
	bmpInfoHeader[23] = 0; // ImageSize

	bmpInfoHeader[24] = 196; // XpixelsPerM
	bmpInfoHeader[25] = 14; // XpixelsPerM
	bmpInfoHeader[26] = 0; // XpixelsPerM
	bmpInfoHeader[27] = 0; // XpixelsPerM

	bmpInfoHeader[28] = 196; // YpixelsPerM
	bmpInfoHeader[29] = 14; // YpixelsPerM
	bmpInfoHeader[30] = 0; // YpixelsPerM
	bmpInfoHeader[31] = 0; // YpixelsPerM

	bmpInfoHeader[32] = 0; // Colors Used
	bmpInfoHeader[33] = 1; // Colors Used
	bmpInfoHeader[34] = 0; // Colors Used
	bmpInfoHeader[35] = 0; // Colors Used

	bmpInfoHeader[36] = 0; // Important Colors
	bmpInfoHeader[37] = 0; // Important Colors
	bmpInfoHeader[38] = 0; // Important Colors
	bmpInfoHeader[39] = 0; // Important Colors

	for (uint8_t i = 0; i < 14; i++) {
		header_buf[i] = bmpFileHeader[i];
	}

	for (uint8_t i = 0; i < 40; i++) {
		header_buf[i + 14] = bmpInfoHeader[i];
	}
}
