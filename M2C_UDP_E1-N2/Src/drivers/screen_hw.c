#include "stm32f0xx_hal.h"
#include "frameworks/screen.h"

static void hw_wdata(uint16_t value)
{
	GPIOD->BSRR	= GPIO_PIN_2;  //rdx
	GPIOC->BRR	= GPIO_PIN_13; //csx
  GPIOC->BSRR = GPIO_PIN_14; //d/cx

	GPIOB->ODR  = value;
  GPIOC->BRR  = GPIO_PIN_12; //rwx
  GPIOC->BSRR  = GPIO_PIN_12;
  GPIOC->BSRR	= GPIO_PIN_13;
}

void lcd_wdata(uint16_t value) {
	hw_wdata(value);
}

static void hw_wcmd(uint16_t value)
{
	GPIOD->BSRR	= GPIO_PIN_2;
	GPIOC->BRR	= GPIO_PIN_13;
  GPIOC->BRR = GPIO_PIN_14;
	GPIOB->ODR  = value;
  GPIOC->BRR  = GPIO_PIN_12;
  GPIOC->BSRR  = GPIO_PIN_12;
  GPIOC->BSRR	= GPIO_PIN_13;
}
static inline uint16_t hw_rdata(void)
{
	uint16_t value;
	GPIOB->MODER = 0;
  GPIOC->BSRR = GPIO_PIN_12;
	GPIOC->BRR	= GPIO_PIN_13;
  GPIOC->BSRR = GPIO_PIN_14; //d/cx
	GPIOD->BRR	= GPIO_PIN_2;
	GPIOD->BSRR	= GPIO_PIN_2;
	value = GPIOB->IDR;
	GPIOB->MODER = 0x55555555;
	return value;
}


static void hw_init(void)
{
//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_SET);//power
	hw_wcmd(0x00);  //nop

	hw_wcmd(0xCF);  //Power control B
	hw_wdata(0x00);
	hw_wdata(0xd9);
	hw_wdata(0X30);

	hw_wcmd(0xED);  //Power on sequence control
	hw_wdata(0x64);
	hw_wdata(0x03);
	hw_wdata(0X12);
	hw_wdata(0X81);

	hw_wcmd(0xE8);  //Driver timing control A
	hw_wdata(0x85);
	hw_wdata(0x00);
	hw_wdata(0x78);

	hw_wcmd(0xCB);  //Power control A
	hw_wdata(0x39);
	hw_wdata(0x2C);
	hw_wdata(0x00);
	hw_wdata(0x34);
	hw_wdata(0x02);

	hw_wcmd(0xF7);  //Pump ratio control
	hw_wdata(0x20);

	hw_wcmd(0xEA);  //Driver timing control B
	hw_wdata(0x00);
	hw_wdata(0x00);

	hw_wcmd(0xC0);    //Power control
	hw_wdata(0x1B);   //VRH[5:0]

	hw_wcmd(0xC1);    //Power control
	hw_wdata(0x13);   //SAP[2:0];BT[3:0]

	hw_wcmd(0xC5);    //VCM control
	hw_wdata(0x3f); 	 //3F
	hw_wdata(0x3c); 	 //3C

	hw_wcmd(0xC7);    //VCM control2
	hw_wdata(0X96);

	hw_wcmd(0xB1);   //Frame Rate Control
	hw_wdata(0x00);
	hw_wdata(0x10);

	hw_wcmd(0xB6);    // Display Function Control
	hw_wdata(0x0A);
	hw_wdata(0xA2);

	hw_wcmd(0xF2);    // 3Gamma Function Disable
	hw_wdata(0x00);

	hw_wcmd(0x26);    //Gamma curve selected
	hw_wdata(0x01);

	hw_wcmd(0xE0);    //Set Gamma
	hw_wdata(0x0F);
	hw_wdata(0x26);
	hw_wdata(0x24);
	hw_wdata(0x0B);
	hw_wdata(0x0E);
	hw_wdata(0x09);
	hw_wdata(0x54);
	hw_wdata(0XA8);
	hw_wdata(0x46);
	hw_wdata(0x0C);
	hw_wdata(0x17);
	hw_wdata(0x09);
	hw_wdata(0x0F);
	hw_wdata(0x07);
	hw_wdata(0x00);

	// hw_wcmd(0XE1);    //Set Gamma
	// hw_wdata(0x00);
	// hw_wdata(0x19);
	// hw_wdata(0x1B);
	// hw_wdata(0x04);
	// hw_wdata(0x10);
	// hw_wdata(0x07);
	// hw_wdata(0x2A);
	// hw_wdata(0x47);
	// hw_wdata(0x39);
	// hw_wdata(0x03);
	// hw_wdata(0x06);
	// hw_wdata(0x06);
	// hw_wdata(0x30);
	// hw_wdata(0x38);
	// hw_wdata(0x0F);
	hw_wcmd(0XE1);    //Set Gamma
	hw_wdata(0x00);//1
	hw_wdata(0x19);
	hw_wdata(0x1B);
	hw_wdata(0x04);
	hw_wdata(0x10);
	hw_wdata(0x0f);//6
	hw_wdata(0x3A);
	hw_wdata(0x57);
	hw_wdata(0x59);
	hw_wdata(0x0a);
	hw_wdata(0x06);//11
	hw_wdata(0x06);
	hw_wdata(0x30);
	hw_wdata(0x38);
	hw_wdata(0x0F);
	
	hw_wcmd(0x36);    //Memory Access Control
	hw_wdata(0xC8);

	hw_wcmd(0x2A);
	hw_wdata(0x00);
	hw_wdata(0x00);
	hw_wdata(0x01);
	hw_wdata(0x3F);

	hw_wcmd(0x2B);
	hw_wdata(0x00);
	hw_wdata(0x00);
	hw_wdata(0x00);
	hw_wdata(0xEF);

	hw_wcmd(0x3A);
	hw_wdata(0x55);

	hw_wcmd(0x11); //Exit Sleep
//	Delayms(120);
	HAL_Delay(20);
	hw_wcmd(0x29); //display on
	hw_wcmd(0x2c);
}


static void hw_setDir(uint16_t degree)
{
	uint8_t mymxmv = 0;
	switch (degree)
	{
		case 0:
			mymxmv = 0x00;
			break;
		case 90:
			mymxmv = 0x05;
			break;
		case 180:
			mymxmv = 0x06;
			break;
		case 270:
			mymxmv = 0x03;
			break;
		default:
			break;
	}
	hw_wcmd(0x36);    //Memory Access Control
	hw_wdata((mymxmv<<5)|0x8);	    //  000
}

static void hw_setOrigin(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	//used
	hw_wcmd(0x2a);
	hw_wdata(x>>8);
	hw_wdata(x&0xff);
	hw_wdata((x+w-1)>>8);
	hw_wdata((x+w-1)&0xff);

	hw_wcmd(0x2b);
	hw_wdata(y>>8);
	hw_wdata(y&0xff);
	hw_wdata((y+h-1)>>8);
	hw_wdata((y+h-1)&0xff);

	hw_wcmd(0x3A);
	hw_wdata(0x55);
	hw_wcmd(0x2c);
}

static void hw_setVerticalScroll(uint16_t top,uint16_t area,uint16_t buttom)
{
	hw_wcmd(0x33);
	hw_wdata(top>>8);
	hw_wdata(top&0xff);
	hw_wdata(area>>8);
	hw_wdata(area&0xff);
	hw_wdata(buttom>>8);
	hw_wdata(buttom&0xff);
}
static void hw_fill(uint16_t width, uint16_t height, uint16_t data)
{
	for (uint16_t i = 0 ; i < width; i++)
	{
		for(uint16_t j = 0 ; j < height; j++)
		{
			hw_wdata(data);
		}
	}
}
static void hw_powerOff()
{
	hw_wcmd(0x28);
}

static void hw_powerOn()
{
	hw_wcmd(0x29);
}


void screen_init(void) {
  hw_init();
	screen_fill(0,0,240,320,0);
}

void screen_powerOff(void) {
	hw_powerOff();
}

void screen_fill(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	hw_setOrigin(x,y,w,h);
  for (int16_t i = 0; i < h; i++) {
    for (int16_t j = 0; j < w; j++) {
      hw_wdata(color);
    }
  }
}

void screen_draw(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t frontColor, uint16_t backColor, const uint8_t* buffer) {
	hw_setOrigin(x,y,w,h);
  for (int16_t i = 0; i < h; i++) {
    for (int16_t j = 0; j < w; j++) {
      int16_t index = ((w+7)>>3)*i+(j>>3);
      int16_t bit = 0x80 >> (j&0x07);
      lcd_wdata((buffer[index]&bit)!=0?frontColor:backColor); //
    }
  }
}
void screen_drawD(int16_t x, int16_t y, int16_t w, int16_t h, screen_xy2cFunction_t fcn) {
	hw_setOrigin(x,y,w,h);
  for (int16_t i = 0; i < h; i++) {
    for (int16_t j = 0; j < w; j++) {
      lcd_wdata(fcn(j,i));
    }
  }
}

void screen_point(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  screen_fill(x, y, 1, 1, color);
}
