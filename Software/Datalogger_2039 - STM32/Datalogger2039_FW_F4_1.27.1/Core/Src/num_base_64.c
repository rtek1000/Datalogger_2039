/*
 * num_base_64.c
 *
 *  Created on: 9 de mai de 2022
 *      Author: r
 */
#include "main.h"
#include "num_base_64.h"
//#include <stdio.h>

void conv_b10_b64(int64_t input, char *output) {
	int64_t num10 = input;
	int64_t R;
	int64_t Q;
	int J = 0;
	int k = 0;
	uint8_t is_negative = 0;
	char map[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char tmp[20];

	if (num10 < 0) {
		is_negative = 1;
		num10 = num10 * (-1);
	}

	if (num10 != 0) {
		Q = num10;

		while (Q > 0) {
			Q = 0;

			while (num10 >= 64) {
				num10 = num10 - 64;
				Q = Q + 1;
			}

			R = num10;

			tmp[J] = map[R];

			num10 = Q;

			J++;
		}

		tmp[J] = 0;

		if (is_negative == 1) {
			output[0] = 45; // "-"
		}

		k = 0;

		while (tmp[k] != 0) {
			k++;
		}

		for (int i = 0; i < k; i++) {
			if (is_negative != 1) {
				output[i] = tmp[(k - 1) - i];
			} else {
				output[i + 1] = tmp[(k - 1) - i];
			}
		}

		if (is_negative != 1) {
			output[J] = 0;
		} else {
			output[J + 1] = 0;
		}
	} else {
		output[0] = map[0];
		output[1] = 0;
	}
}
