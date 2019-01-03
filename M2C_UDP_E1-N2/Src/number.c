#include "number.h"

uint8_t number_bitGet(uint8_t const* buffer, int16_t pos) {
	return (buffer[pos >> 3] & (1 << (pos & 7))) ? 1 : 0;
}
uint8_t number_bitGetBE(uint8_t const* buffer, int16_t pos) {
	return (buffer[pos >> 3] & (0x80 >> (pos & 7))) ? 1 : 0;
}


static int16_t fetchDecBytes(int32_t value, int8_t* buffer) {
	int16_t length;
	length = 0;
	while (value != 0) {
		int32_t d = value / 10;
		buffer[length++] = value - d * 10;
		value = d;
	}
	return length;
}

int16_t number_toDecString(int32_t value, int8_t dotPosition, char* buffer, int16_t bufferSize) {
	uint8_t negative = 0;
	if (value < 0) {
		value = -value;
		negative = 1;
	}
	int8_t offsets[10] = {0};
	int16_t length = fetchDecBytes(value, offsets);
	int16_t index = 0;
	if(negative) {
		buffer[index++] = '-';
	}

	int16_t di = max(length - dotPosition - 1, 0);
	int16_t size = max(length, dotPosition + 1);

	for (int16_t i = 0; i < size; i++) {
		int16_t ri = size - 1 - i;
		if (ri < length) {
			buffer[index++] = '0' + offsets[ri];
			// if (offsets[ri] != 0 || i >= di) {
			// }
		} else {
			buffer[index++] = '0';
		}
		if (index >= bufferSize - 1) {
			break;
		}
		if (i == di && i != size - 1) {
			buffer[index++] = '.';
			if (index >= bufferSize - 1) {
				break;
			}
		}
	}
	buffer[index] = 0;
	return index;
}

int16_t number_fromDecString(const char* buffer, int16_t bufferSize, int32_t* value, int8_t* dotPosition) {
	int32_t v = 0;
	int16_t dp = 0;
	int16_t convertSize = bufferSize;
	uint16_t negative = 0;
	uint16_t decimal = 0;
	for (int16_t i = 0; i < bufferSize; ++i) {
		char c = buffer[i];
		if (i == 0 && c == '-') {
			negative = 1;
		} else if (c == '.' && decimal == 0) {
			decimal = 1;
			dp = i;
		} else if (c >= '0' && c <= '9') {
			v = v * 10 + (c - '0');
		} else {
			convertSize = i;
			break;
		}
	}
	*value = negative ? -v : v;
	*dotPosition = decimal ? (convertSize - dp - 1) : 0;
	return convertSize;
}

int16_t number_toHexString(uint32_t value, char* out, int16_t bufferSize, uint8_t bigLetter) {
	for (int16_t i = 0; i < bufferSize; i++) {
		int8_t v = (value >> ((bufferSize - i - 1) * 4)) & 0xf;
		if (v < 10) {
			out[i] = '0' + v;
		} else {
			out[i] = (bigLetter ? 'A' : 'a') + v - 10;
		}
	}
	return bufferSize;
}

int32_t number_fromHexString(const char* buffer, int16_t size) {
	uint8_t hexBuffer[8] = {0};
	for (int16_t i = 0; i < min(size, 16); i++) {
		uint8_t v = buffer[i];
		if (v >= 'a') {
			v -= 'a' - 10;
		} else if (v >= 'A') {
			v -= 'A' - 10;
		} else {
			v -= '0';
		}
		if ((i & 1) == 0) {
			v <<= 4;
		}
		hexBuffer[i >> 1] += v;
	}
	return number_valueInt32(hexBuffer, (size + 1) >> 1);
}

int32_t number_valueInt32(uint8_t const* buffer, int16_t size) {
	int32_t value = 0;
	const uint8_t maxSize = 4;
	uint8_t negative = 0;
	for (int16_t i = 0; i < min(size, maxSize); i++) {
		uint8_t v = buffer[i];
		value |= (v) << (i * 8);
		negative = (v & 0x80) != 0;
	}
	if (negative && size < maxSize) {
		value -= (UINT32_C(1) << (size * 8));
	}
	return value;
}
int64_t number_valueInt64(uint8_t const* buffer, int16_t size) {
	int64_t value = 0;
	const uint8_t maxSize = 8;
	uint8_t negative = 0;
	for (int16_t i = 0; i < min(size, maxSize); i++) {
		uint8_t v = buffer[i];
		value |= (v) << (i * 8);
		negative = (v & 0x80) != 0;
	}
	if (negative && size < maxSize) {
		value -= (INT64_C(1) << (size * 8));
	}
	return value;
}

int16_t number_minBytes(uint8_t* buffer, int16_t size) {
	uint8_t negative = (uint8_t) ((buffer[size - 1] & 0x80) ? 1 : 0);
	int16_t ms = 1;
	for (int16_t i = 0; i < size; i++) {
		uint8_t value = buffer[i];
		if (negative) {
			if ((value & 0x80) == 0) {
				ms = i + 2;
			} else if (value != 0xff) {
				ms = i + 1;
			}
		}
		else {
			if ((value & 0x80)) {
				ms = i + 2;
			} else if (value != 0) {
				ms = i + 1;
			}
		}
	}
	return ms;
}

int16_t number_minBits(uint8_t* buffer, int16_t size) {
	uint8_t negative = number_bitGet(buffer, size);
	int16_t ms = 1;
	for (int8_t i = 0; i < size; i++) {
		if (negative) {
			if (!number_bitGet(buffer, size)) {
				ms = i + 2;
			}
		}
		else {
			if (number_bitGet(buffer, size)) {
				ms = i + 2;
			}
		}
	}
	return ms;
}

int32_t number_e10(int16_t e) {
	int32_t value = 1;
	while (e--) {
		value *= 10;
	}
	return value;
}

int32_t number_int(int32_t value, int8_t e10) {
	if (e10<0) {
		return value / number_e10(-e10);
	} else {
		return value * number_e10(e10);
	}
}

const int8_t g_number_floatExponentBits[] = {5, 8, 9, 11};
void number_toFloat(uint8_t sign, int32_t exponent, intFraction_t fraction, uint8_t* buffer, int16_t size) {
	int16_t feb = g_number_floatExponentBits[size >> 1];
	int16_t ffb = (size * 8) - feb - 1;
	int16_t ffbi = ffb >> 3;
	int16_t ffbb = (ffb & 0x7);
	for (int16_t i = 0; i < size; i++) {
		if (i < ffbi) {
			buffer[i] = (fraction >> (i * 8));
		} else if (i == ffbi) {
			uint8_t mask = ((1 << ffbb) - 1);
			buffer[i] = ((fraction >> (i * 8)) & mask) | ((exponent << ffbb) & ~mask);
		} else {
			buffer[i] = exponent >> ((i - ffbi) * 8 - ffbb);
		}
	}
	if (sign) {
		buffer[size - 1] |= 0x80;
	} else {
		buffer[size - 1] &= 0x7f;
	}
}
void number_fromFloat(uint8_t* sign, int32_t* exponent, intFraction_t* fraction, uint8_t const* buffer, int16_t size) {
	int16_t feb = g_number_floatExponentBits[size >> 1];
	int16_t ffb = (size * 8) - feb - 1;
	int16_t ffbi = ffb >> 3;
	int16_t ffbb = (ffb & 0x7);
	uint8_t* eb = (uint8_t*)exponent;
	uint8_t* fb = (uint8_t*)fraction;

	for (int16_t i = 0; i < size; i++) {
		if (i < ffbi) {
			fb[i] = buffer[i];
		} else if (i == ffbi) {
			uint8_t mask = ((1 << ffbb) - 1);
			fb[i] = buffer[i] & mask;
			eb[0] = buffer[i] >> ffbb;
		} else {
			eb[i - ffb - 1] |= buffer[i] << (8 - ffbi);
			eb[i - ffb] = buffer[i] >> ffbb;
		}
	}
	*exponent &= (1 << feb) - 1;
	*fraction &= (NUMBER_FRACTION_C(1) << ffb) - 1;
	*sign = (buffer[size - 1] & 0x80) ? 1 : 9;
}

int16_t number_floatMinBytes(uint8_t sign, int32_t exponent, intFraction_t fraction) {
	int16_t ieb = number_minBits((uint8_t*)&exponent, 32);
	int16_t ifb = number_minBits((uint8_t*)&fraction, NUMBER_FRACTION_BTYES * 8) - 1;
	for (int16_t i = 0; i < 4; i++) {
		int16_t eb = g_number_floatExponentBits[i];
		int16_t fb = (i * 16) - eb - 1;
		if (ieb <= eb && ifb <= fb) {
			return (i + 1) * 2;
		}
	}
	return 8;
}

//int16_t number_floatToDecString(float f, char* buffer, int16_t bufferSize) {
//	uint8_t sign;
//	int32_t exponent;
//	intFraction_t fraction;
//	number_fromFloat(&sign, &exponent, &fraction, (uint8_t*)&f, 4);
//	exponent -= 127;
//	fraction += 0x00800000;
//
//}

int32_t number_circleCompare(int32_t a, int32_t b, uint32_t size) {
	int32_t hs = size >> 1;
	int32_t r = a - b;
	if (r > hs) {
		return r - size;
	} else if (r > 0) {
		return r;
	} else if (r > -hs) {
		return r;
	} else {
		return r + size;
	}
}
