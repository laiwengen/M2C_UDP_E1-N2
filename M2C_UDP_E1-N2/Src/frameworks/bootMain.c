#include "stm32f0xx.h"
#include "stdint.h"

#define BOOT_VERSION ("0.0.1")
#define BOOT_ENCODE_ENABLE 0
#define BOOT_ENCODE_XOR 0x57
#define BOOT_APP_START_OFFSET UINT32_C(0x00001000)

#define BOOT_UART_SYNC_DELAY 40
#define BOOT_UART_WAIT 200
#define BOOT_UART_KEY 'Y'
#define BOOT_UART_PASS 'N'
#define BOOT_UART_BAND 921600
#define BOOT_UART_AUTODETECT 0

// #define BOOT_GPIO GPIOC
// #define BOOT_PIN_TX PIN_4
// #define BOOT_PIN_RX PIN_5
#define BOOT_UART USART3
#define BOOT_DMA DMA1
#define BOOT_DMA_CHANNLE DMA1_Channel3

#define BOOT_FLASH_BOOT 0x0000
#define BOOT_FLASH_CODE 0x1000
#define BOOT_FLASH_FPE 0x1F800
#define BOOT_FLASH_INFO 0x20000
#define BOOT_FLASH_UPGRADE 0x21000

#define BOOT_MEMORY_SIZE 0x8000
#define BOOT_GLOBAL_START 0x7000


typedef struct
{
	uint8_t length;
	uint16_t address;
	uint8_t type;
	uint8_t data[0x10];
	uint8_t verify;
} intelHex_t;

#define g_boot_readBuffer ((volatile const uint8_t*)(SRAM_BASE + BOOT_GLOBAL_START + 0x400))
#define g_boot_readIndex *((volatile uint8_t*)(SRAM_BASE + BOOT_GLOBAL_START + 0x500))
#define g_boot_addressBase *((volatile uint32_t*)(SRAM_BASE + BOOT_GLOBAL_START + 0x504))
#define g_boot_obRdp *((volatile uint32_t*)(SRAM_BASE + BOOT_GLOBAL_START + 0x508))
#define g_boot_writeIndex (uint8_t)(0x100 - (BOOT_DMA_CHANNLE->CNDTR))
#define g_boot_vector (volatile uint8_t*)SRAM_BASE


#if 1 // put stuff
static inline void uputc(uint8_t c)
{
	while((BOOT_UART->ISR & UART_FLAG_TXE) == 0);
	BOOT_UART->TDR = c;
}

static void uputs(uint8_t* str)
{
	while(*str)
	{
		uputc(*str);
		str++;
	}
}

static void puthex(uint32_t address, uint8_t bits)
{
	for(uint8_t i = 0; i < bits; i += 4)
	{
		uint8_t c = (((uint32_t)address << i >> (bits - 4)) & 0x0f);
		if(c > 9)
		{
			c += 'A' - '0' - 10;
		}
		uputc('0' + c);
	}
}

static void puti(intelHex_t* ih)
{
	uputc(':');
	puthex(ih->length, 8);
	puthex(ih->address, 16);
	puthex(ih->type, 8);
	for(uint8_t i = 0; i < ih->length; i++)
	{
		puthex(*(ih->data + i), 8);
	}
	puthex(ih->verify, 8);
	uputs("\r\n");
}
#endif
#if 1 // buffer operation

static inline uint8_t ascii2oct(uint8_t c)
{
	if(c >= 'a')
	{
		c = 0x0a + (c - 'a');
	}
	else if(c >= 'A')
	{
		c = 0x0a + (c - 'A');
	}
	else if(c >= '0')
	{
		c = c - '0';
	}
	else
	{
		c = 0;
	}
	return c & 0x0f;
}

static inline uint8_t ascii2hex(uint8_t c0, uint8_t c1)
{
	return ascii2oct(c0) | (ascii2oct(c1) << 4);
}

static inline uint32_t msbBytes(uint8_t* buffer, uint8_t size)
{
	uint32_t value = 0;
	while(size--)
	{
		value <<= 8;
		value |= *buffer;
		buffer++;
	}
	return value;
}


static inline uint32_t lsbBytes(uint8_t* buffer, uint8_t size)
{
	uint32_t value = 0;
	uint8_t shift = 0;
	while(size--)
	{
		value |= ((uint32_t)(*buffer)) << shift;
		shift += 8;
		buffer++;
	}
	return value;
}
static uint8_t checkAscii(uint8_t index)
{
	uint8_t c1 = *(g_boot_readBuffer + index);
	uint8_t c0 = *(g_boot_readBuffer + ((index + 1) & 0xff));
	return ascii2hex(c0, c1);
}
static uint16_t findByteIndex(uint8_t start, uint8_t byteToFind)
{
	uint16_t byte;
	while(1)
	{
		byte = *((g_boot_readBuffer + start));
		if ((start) == g_boot_writeIndex)
		{
			return UINT16_MAX;
		}
		if (byte == byteToFind)
		{
			return start;
		}
		start++;
	}
}

static uint8_t readLine(intelHex_t* ih)
{
	uint8_t error = 1;
	uint16_t startIndex = findByteIndex(g_boot_readIndex, ':');
	if (startIndex != UINT16_MAX)
	{
		uint16_t endIndex = findByteIndex(startIndex, '\n');
		if (endIndex != UINT16_MAX)
		{
			error = 2;
			g_boot_readIndex = endIndex;
			if ((((uint8_t)((endIndex - 1) - (startIndex + 1)) >> 1)) == 1 + 2 + 1 + 1 + checkAscii(startIndex + 1))
			{
				startIndex += 1;
				endIndex = (endIndex - 1) & UINT8_MAX;
				uint8_t sum = 0;
				for(uint8_t i = startIndex; i != endIndex && i != endIndex + 1; i += 2)
				{
					sum += (uint16_t)checkAscii(i);
				}
				if (sum == 0)
				{
					ih->length = checkAscii(startIndex + 0);
					ih->address = (uint16_t)checkAscii(startIndex + 4) | (((uint16_t)checkAscii(startIndex + 2)) << 8);
					ih->type = checkAscii(startIndex + 6);
					for(uint8_t i = 0; i < ih->length; i++)
					{
						ih->data[i] = checkAscii(startIndex + 8 + (i << 1));
					}
					ih->verify = checkAscii(startIndex + 8 + (ih->length << 1));
					error = 0;
				}
			}
		}
	}
	return error;
}


#endif

#if 1 // flash operation

static void flash_erase2(uint32_t address)
{
	while(FLASH->SR & FLASH_SR_BSY);
	if (address >= OB_BASE)
	{
		FLASH->CR |= FLASH_CR_OPTER;
	}
	else
	{
		FLASH->CR |= FLASH_CR_PER;
	}
	FLASH->AR = address;
	FLASH->CR |= FLASH_CR_STRT;
	while(FLASH->SR & FLASH_SR_BSY);
	FLASH->CR &= ~FLASH_CR_OPTER;
	FLASH->CR &= ~FLASH_CR_PER;
}

static void flash_erase(uint16_t page)
{
	flash_erase2(g_boot_addressBase + page);
}

static void flash_write2(uint32_t address, uint16_t* values, uint32_t size) {
	while(FLASH->SR & FLASH_SR_BSY);
	FLASH->CR |= FLASH_CR_PG;
	for (uint32_t i = 0; i < size; i++) {
		while(FLASH->SR & FLASH_SR_BSY);
		*(((uint16_t*)address) + i) = values[i];
	}
	while(FLASH->SR & FLASH_SR_BSY);
	FLASH->CR &= ~FLASH_CR_PG;
}
static void flash_write(uint16_t offset, uint8_t* data, uint8_t size)
{
	offset &= ~1;
	while(FLASH->SR & FLASH_SR_BSY);
	if (g_boot_addressBase + offset >= OB_BASE)
	{
		FLASH->CR |= FLASH_CR_OPTPG;
	}
	else
	{
		FLASH->CR |= FLASH_CR_PG;
	}
	for(uint8_t i = 0; i < size; i += 2)
	{
		if (((g_boot_addressBase) + offset + i) >= FLASH_BASE + BOOT_APP_START_OFFSET)
		{
			uint16_t value = lsbBytes(data + i, 2);
			uint32_t addr = (g_boot_addressBase) + offset + i;
			while(FLASH->SR & FLASH_SR_BSY);
			if ( addr == (uint32_t)&(OB->RDP))
			{
				if ((value & 0xff) == 0xaa && g_boot_obRdp != 0xaa)
				{
					continue;
				}
			}
			*(__IO uint16_t*)(addr) = value;
		}
	}
	FLASH->CR &= ~FLASH_CR_OPTPG;
	FLASH->CR &= ~FLASH_CR_PG;
}
static void flash_read(uint16_t offset, uint8_t* data, uint8_t size)
{
	for(uint8_t i = 0; i < size; i++)
	{
		*(data + i) = *(__IO uint8_t*)((g_boot_addressBase) + offset + i);
	}
}



#endif

#if 1 //code stuff
static inline uint8_t decode(uint8_t byte, uint32_t address)
{
	#if BOOT_ENCODE_ENABLE
	return (byte ^ BOOT_ENCODE_XOR) + (uint8_t)address;
	#else
	return byte;
	#endif
}

static inline uint8_t encode(uint8_t byte, uint32_t address)
{
	#if BOOT_ENCODE_ENABLE
	return (byte - (uint8_t)address) ^ BOOT_ENCODE_XOR;
	#else
	return byte;
	#endif
}

static void encodeBuffer(uint32_t address, uint8_t* buffer, uint8_t size)
{
	for(uint8_t i = 0; i < size; i++)
	{
		*(buffer + i) = encode(*(buffer + i), address++);
	}
}

static void decodeBuffer(uint32_t address, uint8_t* buffer, uint8_t size)
{
	for(uint8_t i = 0; i < size; i++)
	{
		*(buffer + i) = decode(*(buffer + i), address++);
	}
}

#endif
static void intelHexVerify(intelHex_t* ih)
{
	uint8_t sum = 0;
	sum += ih->length;
	sum += ih->address >> 8;
	sum += ih->address;
	sum += ih->type;
	for(uint8_t i = 0; i < ih->length; i++)
	{
		sum += *(ih->data + i);
	}
	ih->verify = -sum;
}

static void parseLine(intelHex_t* ih) {
	switch (ih->type)
	{
	case 0x00:
		//program flash
		decodeBuffer(ih->address, ih->data, ih->length);
		flash_write(ih->address, ih->data, ih->length);
		//puthex(ih->address,16);
		uputs(".\r\n");
		break;
	case 0x01:
		//program end
		uputs("Chip reset.\r\n");
		{
			uint16_t i = 8000;
			while(i--)
			{
				__NOP();
			}
		}
		NVIC_SystemReset();
		break;
	case 0xa1:
		//ob lanch
		uputs("OB lanched.\r\n");
		{
			uint16_t i = 8000;
			while(i--)
			{
				__NOP();
			}
		}
		FLASH->CR |= FLASH_CR_OBL_LAUNCH;
		break;
	case 0x04:
		//set address base
		if (ih->length == 2)
		{
			g_boot_addressBase = msbBytes(ih->data, ih->length) << 16;
			uputs("Address offset is: 0x");
			puthex(g_boot_addressBase, 32);
			uputs(".\r\n");
		}
		break;
	case 0x10:
		//erase flash
		if ( ih->length > 0 && ih->length <= 2)
		{
			uint16_t length = msbBytes(ih->data, ih->length);
			for(uint32_t address = ih->address & (~(FLASH_PAGE_SIZE - 1)); address < ih->address + length; address += FLASH_PAGE_SIZE)
			{
				flash_erase(address);
			}
			uputs("BOOT Erease pages, address is: 0x");
			puthex(ih->address, 16);
			uputs(" length: 0x");
			puthex(length, 16);
			uputs(".\r\n");
		}
		break;
	case 0x11:
		//read flash

		if ( ih->length > 0 && ih->length <= 2)
		{
			ih->type = 0x00;
			uint16_t length = msbBytes(ih->data, ih->length);
			uputs("BOOT Read flash, address is: 0x");
			puthex(ih->address, 16);
			uputs(" length: 0x");
			puthex(length, 16);
			uputs(":\r\n");
			while(length)
			{
				uint16_t thisl = length;
				if (thisl > 0x10)
				{
					thisl = 0x10;
				}
				flash_read(ih->address, ih->data, thisl);
				encodeBuffer(ih->address, ih->data, thisl);
				ih->length = thisl;
				intelHexVerify(ih);
				puti(ih);
				length -= thisl;
				ih->address += thisl;
			}
			uputs("Read done.\r\n");
		}
		break;
	case 0x20:
		//get boot version
		uputs("BOOT Version:");
		uputs(BOOT_VERSION);
		uputs(".\r\n");
		break;
	default:
		uputs("Ignore record.\r\n");
		break;
	}
}

static void pollingCommands(void) {
	g_boot_readIndex = 0;
	g_boot_addressBase = FLASH_BASE;
	uputs("BOOT has initalized.\r\n");
	g_boot_obRdp = (OB->RDP) & 0xff;
	intelHex_t ih;
	while(1)
	{
		if(readLine(&ih) == 0)
		{
			parseLine(&ih);
		}
	}
}

static uint8_t waitUartSync(uint32_t delayms, uint32_t waitms) {
	while(delayms--)
	{
		uint16_t i = 6000;
		while(i--)
		{
			__NOP();
		}
	}
	uputc('U');
	while(waitms--)
	{
		uint16_t i = 6000;
		while(i--)
		{
			__NOP();
		}
		if (BOOT_UART->RDR == BOOT_UART_KEY)
		{
			return 1;
		}
		if (BOOT_UART->RDR == BOOT_UART_PASS)
		{
			return 0;
		}
	}
	return 0;
}
static void clockInit(void) {
	{

	  /* Reset the RCC clock configuration to the default reset state ------------*/
	  /* Set HSION bit */
	  RCC->CR |= (uint32_t)0x00000001U;

	#if defined (STM32F051x8) || defined (STM32F058x8)
	  /* Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE and MCOSEL[2:0] bits */
	  RCC->CFGR &= (uint32_t)0xF8FFB80CU;
	#else
	  /* Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE, MCOSEL[2:0], MCOPRE[2:0] and PLLNODIV bits */
	  RCC->CFGR &= (uint32_t)0x08FFB80CU;
	#endif /* STM32F051x8 or STM32F058x8 */

	  /* Reset HSEON, CSSON and PLLON bits */
	  RCC->CR &= (uint32_t)0xFEF6FFFFU;

	  /* Reset HSEBYP bit */
	  RCC->CR &= (uint32_t)0xFFFBFFFFU;

	  /* Reset PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
	  RCC->CFGR &= (uint32_t)0xFFC0FFFFU;

	  /* Reset PREDIV[3:0] bits */
	  RCC->CFGR2 &= (uint32_t)0xFFFFFFF0U;

	#if defined (STM32F072xB) || defined (STM32F078xx)
	  /* Reset USART2SW[1:0], USART1SW[1:0], I2C1SW, CECSW, USBSW and ADCSW bits */
	  RCC->CFGR3 &= (uint32_t)0xFFFCFE2CU;
	#elif defined (STM32F071xB)
	  /* Reset USART2SW[1:0], USART1SW[1:0], I2C1SW, CECSW and ADCSW bits */
	  RCC->CFGR3 &= (uint32_t)0xFFFFCEACU;
	#elif defined (STM32F091xC) || defined (STM32F098xx)
	  /* Reset USART3SW[1:0], USART2SW[1:0], USART1SW[1:0], I2C1SW, CECSW and ADCSW bits */
	  RCC->CFGR3 &= (uint32_t)0xFFF0FEACU;
	#elif defined (STM32F030x6) || defined (STM32F030x8) || defined (STM32F031x6) || defined (STM32F038xx) || defined (STM32F030xC)
	  /* Reset USART1SW[1:0], I2C1SW and ADCSW bits */
	  RCC->CFGR3 &= (uint32_t)0xFFFFFEECU;
	#elif defined (STM32F051x8) || defined (STM32F058xx)
	  /* Reset USART1SW[1:0], I2C1SW, CECSW and ADCSW bits */
	  RCC->CFGR3 &= (uint32_t)0xFFFFFEACU;
	#elif defined (STM32F042x6) || defined (STM32F048xx)
	  /* Reset USART1SW[1:0], I2C1SW, CECSW, USBSW and ADCSW bits */
	  RCC->CFGR3 &= (uint32_t)0xFFFFFE2CU;
	#elif defined (STM32F070x6) || defined (STM32F070xB)
	  /* Reset USART1SW[1:0], I2C1SW, USBSW and ADCSW bits */
	  RCC->CFGR3 &= (uint32_t)0xFFFFFE6CU;
	  /* Set default USB clock to PLLCLK, since there is no HSI48 */
	  RCC->CFGR3 |= (uint32_t)0x00000080U;
	#else
	 #warning "No target selected"
	#endif

	  /* Reset HSI14 bit */
	  RCC->CR2 &= (uint32_t)0xFFFFFFFEU;

	  /* Disable all interrupts */
	  RCC->CIR = 0x00000000U;
	}
	{
		/* (1) Test if PLL is used as System clock */
		/* (2) Select HSI as system clock */
		/* (3) Wait for HSI switched */
		/* (4) Disable the PLL */
		/* (5) Wait until PLLRDY is cleared */
		/* (6) Set the PLL multiplier to 6 */
		/* (7) Enable the PLL */
		/* (8) Wait until PLLRDY is set */
		/* (9) Select PLL as system clock */
		/* (10) Wait until the PLL is switched on */
		if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) /* (1) */
		{
			RCC->CFGR &= (uint32_t) (~RCC_CFGR_SW); /* (2) */
			while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) /* (3) */
			{
			/* For robust implementation, add here time-out management */
			}
		}
		RCC->CR &= (uint32_t)(~RCC_CR_PLLON);/* (4) */
		while((RCC->CR & RCC_CR_PLLRDY) != 0) /* (5) */
		{
		/* For robust implementation, add here time-out management */
		}
		RCC->CFGR = RCC->CFGR & (~RCC_CFGR_PLLMUL) | (RCC_CFGR_PLLMUL6); /* (6) */
		RCC->CR |= RCC_CR_PLLON; /* (7) */
		while((RCC->CR & RCC_CR_PLLRDY) == 0) /* (8) */
		{
		/* For robust implementation, add here time-out management */
		}
		RCC->CFGR |= (uint32_t) (RCC_CFGR_SW_PLL); /* (9) */
		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) /* (10) */
		{
		/* For robust implementation, add here time-out management */
		}
	}
}

static void uartInit(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

	GPIOC->MODER |= 0x00000a00;
	GPIOC->AFR[0] |= 0x00110000;

	BOOT_UART->CR1 = 0x0000000C;
	#if BOOT_UART_AUTODETECT
	BOOT_UART->CR2 = USART_CR2_ABRMODE_0 | USART_CR2_ABREN;
	#endif
	BOOT_UART->BRR = (48000000 / BOOT_UART_BAND);
	BOOT_UART->CR1 |= 0x00000001;
}

static void uartDeinit(void) {
	GPIOC->MODER &= ~0x00000a00;
	GPIOC->AFR[0] &= ~0x00110000;

	BOOT_UART->CR1 &= ~0x00000001;
	BOOT_UART->CR1 = 0;
	#if BOOT_UART_AUTODETECT
	BOOT_UART->CR2 = 0;
	#endif
	BOOT_UART->BRR = 0;

	RCC->APB2ENR &= ~RCC_APB2ENR_SYSCFGEN;
  RCC->AHBENR &= ~RCC_AHBENR_GPIOCEN;
  RCC->APB1ENR &= ~RCC_APB1ENR_USART3EN;

}

static void copyInterrupts (void) {
	for(uint16_t i = 0; i < 0x10; i += 2)
	{
		*(uint16_t*)(g_boot_vector + i) = *(uint16_t*)(FLASH_BASE + BOOT_APP_START_OFFSET + i);
	}
	for(uint16_t i = 0x10; i < 0xc0; i += 2)
	{
		*(uint16_t*)(g_boot_vector + i) = *(uint16_t*)(FLASH_BASE + BOOT_APP_START_OFFSET + i);
	}
		__HAL_REMAPMEMORY_SRAM();
}

static void unlockFlash(void) {
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	FLASH->OPTKEYR = FLASH_OPTKEY1;
	FLASH->OPTKEYR = FLASH_OPTKEY2;
}

static void initDMA(void) {
	RCC->AHBENR |= (RCC_AHBENR_DMA1EN);

	BOOT_UART->CR1 &= ~0x00000001;
	BOOT_UART->CR3 = 0x00000040;
	BOOT_UART->CR1 |= 0x00000001;

	BOOT_DMA->CSELR |= 0x00000a00;

	BOOT_DMA_CHANNLE->CNDTR = 0x00000100;
	BOOT_DMA_CHANNLE->CPAR = (uint32_t)&(BOOT_UART->RDR);
	BOOT_DMA_CHANNLE->CMAR = (uint32_t)g_boot_readBuffer;
	BOOT_DMA_CHANNLE->CCR = 0x000000af;
}

static uint8_t uotaReady(void) {
	return *(uint32_t*)(FLASH_BASE + BOOT_FLASH_INFO) != 0xffffffff;
}
static uint32_t uotaSize(void) {
	return *(uint32_t*)(FLASH_BASE + BOOT_FLASH_INFO);
}

static void uotaStart(void) {
	uint32_t size = uotaSize();
	unlockFlash();
	// erase
	for(uint32_t offset = 0; offset < BOOT_FLASH_FPE - BOOT_FLASH_CODE && offset < size; offset += FLASH_PAGE_SIZE) {
		flash_erase2(FLASH_BASE + BOOT_FLASH_CODE + offset);
	}
	// program
	for(uint32_t offset = 0; offset < BOOT_FLASH_FPE - BOOT_FLASH_CODE && offset < size; offset += FLASH_PAGE_SIZE) {
		flash_write2(FLASH_BASE + BOOT_FLASH_CODE + offset, (uint16_t*)(FLASH_BASE + BOOT_FLASH_UPGRADE + offset), FLASH_PAGE_SIZE >> 1);
	}
	// clear info
	flash_erase2(FLASH_BASE + BOOT_FLASH_INFO);
	// erase old
	for(uint32_t offset = 0; offset < BOOT_FLASH_FPE - BOOT_FLASH_CODE && offset < size; offset += FLASH_PAGE_SIZE) {
		flash_erase2(FLASH_BASE + BOOT_FLASH_UPGRADE + offset);
	}
	NVIC_SystemReset();
}

void boot_main(void)
{
	#if 1
	//TODO check if reset from standby or stop mode
	__set_PRIMASK(1); //disable all interrupt;
	// __set_MSP(SRAM_BASE + BOOT_MEMORY_SIZE); //set stack
	clockInit();
	uartInit();
	#if 1
	if (waitUartSync(BOOT_UART_SYNC_DELAY, BOOT_UART_WAIT)) {
		initDMA(); // start dma
		unlockFlash();
		pollingCommands();
	} else if (uotaReady()) {
		uotaStart();
	} else {
	#else
	{
	#endif
		uartDeinit(); // deint uart
	#else
	{
		#endif
		copyInterrupts();
		void((*app_start)()) = (void((*)()))(*((volatile uint32_t*)(FLASH_BASE + BOOT_APP_START_OFFSET + 4)));
		__set_MSP(*(volatile uint32_t*)(FLASH_BASE + BOOT_APP_START_OFFSET));
		__set_PRIMASK(0);
		(*app_start)();
	}
}
