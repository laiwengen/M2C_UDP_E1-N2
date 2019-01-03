#include "string.h"
#include "stdlib.h"
#include "frameworks/strings.h"
#include "frameworks/list.h"
#include "frameworks/number.h"


static uint8_t isspace(char c) {
	return c==' ' || c=='\t' || c=='\r' || c=='\n';
}

// check
uint8_t string_startWith(const char* str, const char* p) {
	int16_t pl = strlen(p);
	int16_t sl = strlen(str);
	if (sl<pl) {
		return 0;
	}
	return strncasecmp(str,p,pl) == 0;
}

uint8_t string_equal(const char* a, const char* b) {
	if (a == 0 || b == 0) {
		return 0;
	}
	return strcasecmp(a,b) == 0;
}

// modify
int16_t string_removeBackspace(char* str, int16_t size) {
	uint16_t i = 0, j = 0;
	for(; i< size+1; i++)
	{
		if (*(str + i) == '\b')
		{
			if(j>0)
			{
				j--;
			}
		}
		else
		{
			*(str+j) = *(str+i);
			j++;
		}
	}
	*(str+j) = 0;
	return j;
}

// new
char* string_duplicate(const char* str, int16_t size) {
	if (size<0) {
		size = strlen(str);
	}
	char* s = (char*)malloc(size + 1);
	if (s) {
		memcpy(s,str,size);
		s[size] = 0;
	}
	return s;
}

char* string_concat(char const** concats, int16_t size){
	int16_t length = 0;
	for (int16_t i = 0; i < size; i++) {
		length += strlen(concats[i]);
	}
	char* s = (char*) malloc(length+1);
	if (s) {
		int16_t offset = 0;
		for (int16_t i = 0; i < size; i++) {
			int16_t l = strlen(concats[i]);
			memcpy(s+offset, concats[i], l);
			offset += l;
		}
		s[offset] = 0;
	}
  return s;
}

int16_t string_parseAT(const char* str, int16_t size, const char** command, char const** params) {
	int16_t offset = 0;
	if (string_startWith(str+offset,"AT+")) {
		offset += 3;
	} else {
		return 0;
	}
	int16_t ts = 0;
	int16_t count = 0;
	while(1) {
		char c = str[offset + ts];
		uint8_t shouldBreak = c == '\0' || c == '\n' || c == '\r' || offset + ts == size - 1;
		if (shouldBreak || c == ((count==0)?'=':',')) {
			char* s = string_duplicate(str+offset, ts);
			if (count ==0){
				*command = s;
			} else {
				*(params + count-1) = s;
			}
			count ++;
			if (shouldBreak) {
				break;
			}
			offset += ts + 1;
			ts = 0;
		} else {
			ts ++;
		}
	}
	return count;
}

int16_t tokenCount(const char* str, char token) {
	int16_t offset = 0;
	int16_t ts = 0;
	int16_t count = 0;
	while(1) {
		char c = str[offset + ts];
		uint8_t shouldBreak = c == '\0' || c == '\n' || c == '\r';
		if (shouldBreak || c == token) {
			count ++;
			if (shouldBreak) {
				break;
			}
			offset += ts + 1;
			ts = 0;
		} else {
			ts ++;
		}
	}
	return count;
}

int16_t string_split(const char* str, char splitter, char*** out) {
	int16_t offset = 0;
	int16_t ts = 0;
	int16_t count = 0;
	int16_t size = tokenCount(str, splitter);
	char** buffer = (char**) malloc(sizeof(void*)*size);
	*out = buffer;
	if (!buffer) {
		return 0;
	}
	while(1) {
		char c = str[offset + ts];
		uint8_t shouldBreak = c == '\0' || c == '\n' || c == '\r';
		if (shouldBreak || c == splitter) {
			char* s = string_duplicate(str+offset, ts);
			buffer[count] = s;
			count ++;
			if (shouldBreak) {
				break;
			}
			offset += ts + 1;
			ts = 0;
		} else {
			ts ++;
		}
	}
	return count;
}

char* string_trim(const char* str) {
	int16_t length = strlen(str);
	int16_t start = length, end = start;
	for (int16_t i = 0; i < length; i++) {
		char c = str[i];
		if (!isspace(c)) {
			start = i;
			break;
		}
	}
	for (int16_t i = length - 1; i >= start; i--) {
		char c = str[i];
		if (!isspace(c)) {
			end = i + 1;
			break;
		}
	}
	return string_duplicate(str + start, end-start);
}

void string_trimS(char const** strp, int16_t* size) {
	int16_t length = *size;
	int16_t start = length, end = start;
	char const* str = *strp;
	for (int16_t i = 0; i < length; i++) {
		char c = str[i];
		if (!isspace(c)) {
			start = i;
			break;
		}
	}
	for (int16_t i = length - 1; i >= start; i--) {
		char c = str[i];
		if (!isspace(c)) {
			end = i + 1;
			break;
		}
	}
	*strp = str + start;
	*size = end - start;
}

static const char g_urlChars[] = {' '};
static const uint8_t g_urlValues[] = {0x20};

char* string_urlencode(char const* input) {
	int16_t size = 0;
	fortas(char, input, strlen(input)) {
		foris(j, sizeof(g_urlChars)) {
			if (g_urlChars[j] == v) {
				size += 2;
				break;
			}
		}
		size += 1;
	}
	char* toRet = (char*) malloc(size+1);
	int16_t k = 0;
	if (toRet) {
		fortas(char, input, strlen(input)) {
			uint8_t match = 0;
			uint8_t value;
			foris(j, sizeof(g_urlChars)) {
				if (g_urlChars[j] == v) {
					match = 1;
					value = g_urlValues[j];
					break;
				}
			}
			if (match) {
				toRet[k++] = '%';
				k+=number_toHexString(value, &toRet[k], 2, 1);
			} else {
				toRet[k++] = v;
			}
		}
		toRet[k] = 0;
	}
	return toRet;
}
