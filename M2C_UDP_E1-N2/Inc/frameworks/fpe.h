#ifndef FPE_H__
#define FPE_H__
#include "stdint.h"

typedef enum
{
	PAGE_PMCORRECT,
	PAGE_CONFIGURE,
	PAGE_HISTORY,
}page_type_t;

void fpe_init(uint32_t firstPage, uint32_t sencondPage);
uint32_t fpe_read(uint32_t address);
uint32_t fpe_readOr(uint32_t address,uint32_t def);
void fpe_write(uint32_t address, uint32_t value);
uint8_t fpe_readBytes(uint32_t address, uint8_t* bytes, uint16_t size);
void fpe_writeBytes(uint32_t address, uint8_t* bytes, int16_t size);

#endif
