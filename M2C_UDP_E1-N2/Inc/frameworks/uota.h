#ifndef UOTA_H__
#define UOTA_H__

#include "stdint.h"

typedef struct uota_t uota_t;
typedef struct uota_line_t uota_line_t;
typedef void (*uota_writeFunction_t) (uota_t*, uint32_t, uint8_t*, int16_t);
typedef void (*uota_commonFunction_t) (uota_t*);

struct uota_t {
  uota_writeFunction_t writeFunction;
  uota_commonFunction_t resetFunction;
  uota_commonFunction_t finishFunction;
  char* version;
  uint32_t total;
  uint32_t base;
};
struct uota_line_t {
  uint8_t length;
  uint16_t address;
  uint8_t type;
  uint8_t data[16];
	uint8_t verify;
};

int16_t uota_stringToBytes(char* str, uint8_t* out);
uint8_t uota_bytesToObject(uint8_t* bytes, int16_t size, uota_line_t* out);
uint8_t uota_push(char* version, int32_t line, int32_t total, uint8_t* bytes, int16_t size);
void uota_init(uota_t* uota);

void uota_hw_save(uota_t* uota);
void uota_hw_clear(uota_t* uota);
uint8_t uota_hw_load(uota_t* uota);

#endif
