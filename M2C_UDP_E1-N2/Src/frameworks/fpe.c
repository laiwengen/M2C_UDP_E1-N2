#include "frameworks/fpe.h"
#include "string.h"
#include "stm32f0xx_hal.h"

uint32_t g_fpe_page1, g_fpe_page2;

#define flashAt(addr) *(volatile uint32_t *)(addr)


static uint32_t getCurrenPage()
{
	if(flashAt(g_fpe_page1) != UINT32_MAX)
	{
		return g_fpe_page1;
	}
	else
	{
		return g_fpe_page2;
	}
}

static uint32_t getFirstFreeOffset(uint32_t base)
{
	for(uint16_t i = 8; i < FLASH_PAGE_SIZE; i+= 8)
	{
		if(flashAt(base + i) == UINT32_MAX)
		{
			return i;
		}
	}
	return FLASH_PAGE_SIZE;
}

static uint8_t readInPage(uint32_t base, uint32_t address, uint32_t* pValue)
{
	//* for each backward
	for (int16_t i = FLASH_PAGE_SIZE - 8; i>=8; i-=8)
	{
		if (flashAt(base + i) == address)
		{
			if (pValue)
			{
				*pValue = flashAt(base + i+4);
			}
			return 1;
		}
	}
	return 0;
}

static uint8_t writeInPage(uint32_t base, uint32_t address, uint32_t value)
{
	//* read if already the value
	uint32_t rValue;
	if ( readInPage(base,address,&rValue))
	{
		if ( rValue == value)
		{

			return 1;
		}
	}
	else
	{
		//* or is erase the value
		if (value == UINT32_MAX)
		{
			return 1;
		}
	}
	//* check enough space
	uint32_t offset = getFirstFreeOffset(base);
	if (offset>= FLASH_PAGE_SIZE)
	{
		return 0;
	}
	//* program flash
	HAL_FLASH_Program(2,base + offset, address);
	while((FLASH->SR & FLASH_SR_BSY)!=0);
	HAL_FLASH_Program(2,base + offset+4, value);
	while((FLASH->SR & FLASH_SR_BSY)!=0);
	return 1;
}

static void reorder(uint32_t from, uint32_t to)
{
	//* erase the "to" page
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = 0;
	eraseInit.PageAddress = to;
	eraseInit.NbPages = 1;
	uint32_t error;
	HAL_FLASHEx_Erase(&eraseInit,&error);
	//* copy the data to the "to" page, backward.
	uint16_t toIndex = 8;
	for (int16_t i = FLASH_PAGE_SIZE - 8; i>=8; i-=8)
	{
		uint32_t addr = flashAt(from+i);
		uint32_t value = flashAt(from+i+4);
		uint32_t fromValue = 0;
		readInPage(from,addr,&fromValue);
		if (addr != UINT32_MAX && value!=UINT32_MAX && !readInPage(to,addr,NULL) && fromValue!=UINT32_MAX)
		{
			HAL_FLASH_Program(2,to + toIndex, addr);
			HAL_FLASH_Program(2,to + toIndex+4, value);
			toIndex += 8;
		}
	}
	// * set the flage of the "to" page
	HAL_FLASH_Program(2,to, 0);
	//* erase the "from" page, so that the "from" flag would be clear.
	eraseInit.PageAddress = from;
	HAL_FLASHEx_Erase(&eraseInit,&error);
}


void fpe_init(uint32_t firstPage, uint32_t sencondPage)
{
	g_fpe_page1 = FLASH_BASE + FLASH_PAGE_SIZE*firstPage;
	g_fpe_page2 = FLASH_BASE + FLASH_PAGE_SIZE*sencondPage;
}
uint32_t fpe_read(uint32_t address)
{
	uint32_t currentPage = getCurrenPage();
	uint32_t value = UINT32_MAX;
	readInPage(currentPage,address,&value);
	return value;
}
uint32_t fpe_readOr(uint32_t address,uint32_t def)
{
	uint32_t currentPage = getCurrenPage();
	uint32_t value = UINT32_MAX;
	readInPage(currentPage,address,&value);
	if (value == UINT32_MAX)
	{
		return def;
	}
	return value;
}
void fpe_write(uint32_t address, uint32_t value)
{
	HAL_FLASH_Unlock();
	//* try to write to current page
	uint32_t currentPage = getCurrenPage();
	if ( !writeInPage(currentPage,address,value))
	{
		uint32_t nextPage;
		if ( currentPage == g_fpe_page1)
		{
			nextPage = g_fpe_page2;
		}
		else
		{
			nextPage = g_fpe_page1;
		}
		reorder(currentPage,nextPage);
		//* write again
		writeInPage(nextPage,address,value);
	}
	//* current is full, need reorder
	HAL_FLASH_Lock();
}

uint8_t fpe_readBytes(uint32_t address, uint8_t* bytes, uint16_t size) {
	uint32_t currentPage = getCurrenPage();
	uint32_t value = 0;
	uint8_t toRet = 0;
	for(uint16_t i = 0; i<size; i++) {
		uint8_t shift = i & 0x03;
		if (shift == 0) {
			value = UINT32_MAX;
			readInPage(currentPage,address,&value);
			address+=1;
		}
		bytes[i] = ((value)>>(shift*8))&0xff;
		if (bytes[i] != 0xff) {
			toRet = 1;
		}
	}
	return toRet;
}

void fpe_writeBytes(uint32_t address, uint8_t* bytes, int16_t size)
{
	HAL_FLASH_Unlock();
	//* try to write to current page
	uint32_t currentPage = getCurrenPage();
	uint32_t value = 0;
	uint8_t reordered = 0;
	for(uint16_t i = 0; i<size; i++)
	{
		uint8_t shift = i&0x03;
		value|=(bytes[i])<<(shift*8);
		if (shift == 3 || i == size-1)
		{
			if (!writeInPage(currentPage,address,value))
			{
				if(reordered)
				{
					break;
				}
				uint32_t nextPage;
				if (currentPage == g_fpe_page1)
				{
					nextPage = g_fpe_page2;
				}
				else
				{
					nextPage = g_fpe_page1;
				}
				reorder(currentPage,nextPage);
				//* write again
				writeInPage(nextPage,address,value);
				reordered = 1;
			}
			value = 0;
			address += 1;
		}
	}
	//* current is full, need reorder
	HAL_FLASH_Lock();
}
