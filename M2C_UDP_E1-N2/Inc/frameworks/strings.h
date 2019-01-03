#ifndef STRINGS_H__
#define STRINGS_H__
#include "stdint.h"

uint8_t string_startWith(const char* str, const char* p);
uint8_t string_equal(const char* a, const char* b);

int16_t string_removeBackspace(char* str, int16_t size);

char* string_duplicate(const char* str, int16_t size);
char* string_concat(char const** concats, int16_t size);
char* string_trim(const char* str);
void string_trimS(char const** strp, int16_t* size);

int16_t string_parseAT(const char* str, int16_t size, const char** command, char const** params);
int16_t string_split(const char* str, char splitter, char*** out);
char* string_urlencode(char const* input);

#endif
