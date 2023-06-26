/*
 * key_map.c
 *
 *  Created on: Jun 25, 2023
 *      Author: user
 */

#include "key_map.h"

#define KEY_CAPS_LOCK                          0x39
#define KEY_SCROLL_LOCK                        0x47

uint8_t ABNT2_map_in[] = { //
		(uint8_t) '`', // '\'
				(uint8_t) '~',  // '"'
				(uint8_t) '\\', // ']'
				(uint8_t) '|',  // '}'
				(uint8_t) ';',  // 'ç'
				(uint8_t) ':',  // 'Ç'
				(uint8_t) '\'', // '~'
				(uint8_t) '"',  // '^'
				(uint8_t) '[',  // '´'
				(uint8_t) '{',  // '`'
				(uint8_t) ']',  // '['
				(uint8_t) '}',  // '{'
				(uint8_t) '/',  // ';'
				(uint8_t) '?',  // ':'
		};

uint8_t ABNT2_map_out[] = { //
		(uint8_t) '\'', // '
				(uint8_t) '"',  // "
				(uint8_t) ']',  // ]
				(uint8_t) '}',  // }
				(uint8_t) 'c',  // ç
				(uint8_t) 'C',  // Ç
				(uint8_t) '~',  // ~
				(uint8_t) '^',  // ^
				(uint8_t) '\'', // ´
				(uint8_t) '`',  // `
				(uint8_t) '[',  // [
				(uint8_t) '{',  // {
				(uint8_t) ';',  // ;
				(uint8_t) ':',  // :
		};

uint8_t ABNT2_map_AltGR_in[] = { //
		(uint8_t) 0x14, // '/'
				(uint8_t) 0x1A  // '?'
		};

uint8_t ABNT2_map_AltGR_out[] = {  //
		(uint8_t) '/', // '/'
				(uint8_t) '?'  // '?'
		};

uint8_t get_ABNT2(HID_KEYBD_Info_TypeDef *info) {
	uint8_t size_in = sizeof(ABNT2_map_in);
	uint8_t ascii_tmp = info->key_ascii;
	uint8_t key_code = info->keys[0];

//USBH_UsrLog("get_ABNT2");

	if (info->ralt == 1U) {

		//USBH_UsrLog("AltGr: key (0x%02X)", key_code);

		size_in = sizeof(ABNT2_map_AltGR_in);

		if (size_in == sizeof(ABNT2_map_AltGR_out)) {
			for (uint8_t i = 0; i < size_in; i++) {
				if (key_code == ABNT2_map_AltGR_in[i]) {
					ascii_tmp = ABNT2_map_AltGR_out[i];
					//USBH_UsrLog("ascii_tmp found");
					break;
				}
			}
		} else {
			USBH_UsrLog("Error, ABNT2_map_in != ABNT2_map_AltGR_out");
		}
	} else {
		if (size_in == sizeof(ABNT2_map_out)) {
			if (key_code == 0x63) {
				//USBH_UsrLog("Direct output");

				if (info->led_caps_lock == 1) {
					ascii_tmp = 'C'; // Ç not supported (UTF8)
				} else {
					ascii_tmp = 'c'; // ç not supported (UTF8)
				}
			} else {
				for (uint8_t i = 0; i < size_in; i++) {
					if (ascii_tmp == ABNT2_map_in[i]) {
						ascii_tmp = ABNT2_map_out[i];
						break;
					}
				}
			}
		} else {
			USBH_UsrLog("Error, ABNT2_map_in != ABNT2_map_out");
		}
	}

	return ascii_tmp;
}

//uint8_t getKeyMapABNT2(HID_KEYBD_Info_TypeDef *info) {
//	uint8_t output = 0;
//	if ((info->lshift == 1U) || (info->rshift)) {
//		output =
//				HID_KEYBRD_ShiftKey_Custom[HID_KEYBRD_Codes_Custom[info->keys[0]]];
//	} else if (info->ralt == 1U) {
//		//USBH_UsrLog("AltGr");
//
//		if (info->keys[0] == KEY_W) {
//			output = '?';
//		} else if (info->keys[0] == KEY_Q) {
//			output = '/';
//		}
//	} else {
//		if (info->led_caps_lock == 1) {
//			// USBH_UsrLog("caps_lock_state = true");
//			if ((info->keys[0] >= KEY_A) & (info->keys[0] <= KEY_Z)) {
//				//USBH_UsrLog("(info->keys[0] >= 65) & (info->keys[0] <= 90)");
//				output =
//						HID_KEYBRD_ShiftKey_Custom[HID_KEYBRD_Codes_Custom[info->keys[0]]];
//			} else {
//				output =
//						HID_KEYBRD_Key_Custom[HID_KEYBRD_Codes_Custom[info->keys[0]]];
//			}
//		} else {
//			output =
//					HID_KEYBRD_Key_Custom[HID_KEYBRD_Codes_Custom[info->keys[0]]];
//		}
//	}
//	return output;
//}
