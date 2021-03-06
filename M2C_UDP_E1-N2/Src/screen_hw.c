#include "stm32f0xx_hal.h"
#include "frameworks/screen.h"
#include "mainConfig.h"


static void hw_wdata(uint16_t value)
{
  GPIOF->BSRRH = GPIO_PIN_1;
  GPIOF->BSRRL = GPIO_PIN_0;
	GPIOC->ODR = value;
  GPIOA->BSRRH = GPIO_PIN_0;
  GPIOA->BSRRL = GPIO_PIN_0;
  GPIOF->BSRRL = GPIO_PIN_1;
}

void lcd_wdata(uint16_t value) {
	hw_wdata(value);
}

static void hw_wcmd(uint16_t value)
{
  GPIOF->BSRRH = GPIO_PIN_1;
  GPIOF->BSRRH = GPIO_PIN_0;
	GPIOC->ODR = value;
  GPIOA->BSRRH = GPIO_PIN_0;
  GPIOA->BSRRL = GPIO_PIN_0;
  GPIOF->BSRRL = GPIO_PIN_1;
}
//static inline uint16_t hw_rdata(void)
//{
//	uint16_t value;
//	GPIOC->MODER = 0;
//  GPIOF->BSRRH = GPIO_PIN_1;
//  GPIOF->BSRRL = GPIO_PIN_0;
//  GPIOA->BSRRH = GPIO_PIN_1;
//  GPIOA->BSRRL = GPIO_PIN_1;
//	value = GPIOC->IDR;
//  GPIOF->BSRRL = GPIO_PIN_1;
//	GPIOC->MODER = 0x55555555;
//	return value;
//}
static void hw_deinit(void)
{
	
  GPIO_InitTypeDef GPIO_InitStruct;
	
	// pc0-pc15
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	// pf0 pf1 pf4 pf5 pf6
	
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
		  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	#if NEW_M2_PCB
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);//power
		#else
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_RESET);//power
	#endif
//	GPIOF->BRR= (uint32_t)GPIO_PIN_4;//BSRR reset fail..
	
}
#if 1
static void hw_init(void)
{
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_0,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_1,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_SET);//power
#if NEW_M2_PCB
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET); //reset
	HAL_Delay(2);
	
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET); //reset
	HAL_Delay(2);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET); //reset
	HAL_Delay(20);
#else
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET); //reset
	HAL_Delay(2);
	
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_RESET); //reset
	HAL_Delay(2);
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET); //reset
	HAL_Delay(20);	
#endif
#ifndef LCD_DRIVER_ST7789	
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
	
		hw_wcmd(0x36);    //Memory Access Control
	hw_wdata(0xC8);	
	
	hw_wcmd(0x3A);   
	hw_wdata(0x55); 	
	
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
	hw_wdata(0x19); 
	hw_wdata(0x18); 
	hw_wdata(0x0A); 
	hw_wdata(0x0C); 
	hw_wdata(0x07); 
	hw_wdata(0x4D); 
	hw_wdata(0X84); 
	hw_wdata(0x41); 
	hw_wdata(0x09); 
	hw_wdata(0x15); 
	hw_wdata(0x07); 
	hw_wdata(0x12); 
	hw_wdata(0x06); 
	hw_wdata(0x00); 	
	
	hw_wcmd(0XE1);    //Set Gamma 
	hw_wdata(0x00);
	hw_wdata(0x1A);
	hw_wdata(0x1B);
	hw_wdata(0x03);
	hw_wdata(0x0F);
	hw_wdata(0x05);
	hw_wdata(0x33);
	hw_wdata(0x15);
	hw_wdata(0x47);
	hw_wdata(0x02);
	hw_wdata(0x0b);
	hw_wdata(0x0A);
	hw_wdata(0x35);
	hw_wdata(0x38);
	hw_wdata(0x0F);
	

	hw_wcmd(0x2A); 
	hw_wdata(0x00);
	hw_wdata(0x00);
	hw_wdata(0x00);
	hw_wdata(0xef);	
	
	hw_wcmd(0x2B); 
	hw_wdata(0x00);
	hw_wdata(0x00);
	hw_wdata(0x01);
	hw_wdata(0x3F);
		


	hw_wcmd(0x11); //Exit Sleep
//	Delayms(120);
	HAL_Delay(120);
	hw_wcmd(0x29); //display on
	hw_wcmd(0x2c);
#else //ST7789V

hw_wcmd(0x0011); 
HAL_Delay(120); 
hw_wcmd(0x00B2);  
hw_wdata(0x000C); 
hw_wdata(0x000C); 
hw_wdata(0x0000); 
hw_wdata(0x0033);
hw_wdata(0x0033);

hw_wcmd(0x00B7);  
hw_wdata(0x0035); 

hw_wcmd(0x00BB);  
hw_wdata(0x0015);    //1f

hw_wcmd(0x00C0);  
hw_wdata(0x002C); 

hw_wcmd(0x00C2);  
hw_wdata(0x0001); 
 
hw_wcmd(0x00C3);  
hw_wdata(0x0017); 

hw_wcmd(0x00C4);  
hw_wdata(0x0020);		  //20

hw_wcmd(0x00C6);  
hw_wdata(0x0006);			//0f

//hw_wcmd(0x00CA);  
//hw_wdata(0x000F);

//hw_wcmd(0x00C8);  
//hw_wdata(0x0008);


//hw_wcmd(0x0055);  
//hw_wdata(0x0090);		//90


hw_wcmd(0x00D0);  
hw_wdata(0x00A4); 
hw_wdata(0x00A1); 
 
hw_wcmd(0x00E0);    //Set Gamma 
hw_wdata(0x00d0); 
hw_wdata(0x0000); 
hw_wdata(0x0014); 
hw_wdata(0x0015); 
hw_wdata(0x0013); 
hw_wdata(0x002c); 
hw_wdata(0x0042); 
hw_wdata(0x0043); 
hw_wdata(0x004e); 
hw_wdata(0x0009); 
hw_wdata(0x0016); 
hw_wdata(0x0014); 
hw_wdata(0x0018);
hw_wdata(0x0021); 
 
 
hw_wcmd(0x00E1);    //Set Gamma 
hw_wdata(0x00d0); 
hw_wdata(0x0000); 
hw_wdata(0x0014); 
hw_wdata(0x0015); 
hw_wdata(0x0013); 
hw_wdata(0x000b); 
hw_wdata(0x0043); 
hw_wdata(0x0055); 
hw_wdata(0x0053); 
hw_wdata(0x000c); 
hw_wdata(0x0017); 
hw_wdata(0x0014); 
hw_wdata(0x0023); 
hw_wdata(0x0020); 

hw_wcmd(0x0036);  
hw_wdata(0x0000); 
hw_wcmd(0x003A); 
hw_wdata(0x0005); 

hw_wcmd(0x002A); 
hw_wdata(0x0000);
hw_wdata(0x0000);
hw_wdata(0x0000);
hw_wdata(0x00ef);	

hw_wcmd(0x002B); 
hw_wdata(0x0000);
hw_wdata(0x0000);
hw_wdata(0x0001);
hw_wdata(0x003f);



hw_wcmd(0x0029); //display on
hw_wcmd(0x002c);
#endif
	
}
#endif


void screen_setDir(uint16_t degree)
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
#ifdef LCD_DRIVER_ST7789
	hw_wdata((mymxmv<<5)|0x0);	    //  000
#else
	hw_wdata((mymxmv<<5)|0x08);	    //  000
	#endif
}

static void hw_setOrigin(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	//used
//	#ifndef LCD_DRIVER_ST7789
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
//	#else
//	hw_wcmd(0x2a);
//	hw_wdata(x>>8);
//	hw_wdata(x&0xff);
//	hw_wdata((x+h-1)>>8);
//	hw_wdata((x+h-1)&0xff);

//	hw_wcmd(0x2b);   
//	hw_wdata(y>>8);
//	hw_wdata(y&0xff);
//	hw_wdata((y+w-1)>>8);
//	hw_wdata((y+w-1)&0xff);
//		
//	hw_wcmd(0x3A);   
//	hw_wdata(0x55);
//	hw_wcmd(0x2c);	
//	
//	
//	#endif
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
	screen_fill(0,0,320,240,rgb(0,0,0));
}

void screen_powerOff(void) {
	hw_deinit();
//	hw_powerOff();
}

void screen_fill(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	hw_setOrigin(x,y,w,h);
  for (int16_t i = 0; i < h; i++) {
    for (int16_t j = 0; j < w; j++) {
      hw_wdata(color);
    }
  }
}
void screen_fill_circle(int16_t x, int16_t y,int16_t r, uint16_t color) {
	int16_t h = r<<1;// x2 + y2 = r2;
	int16_t w = r<<1;// x2 + y2 = r2;
	
	hw_setOrigin(x - r,y - r,h,w);
  for (int16_t i = 0; i < h; i++) {
    for (int16_t j = 0; j < w; j++) {
			if (i <= r && j <= r)
			if (((r - i)*(r - i) + (r - j)*(r - j))<= r*r )
			{
				hw_wdata(rgb (255,0,0));
			}

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
void screen_draw4DH(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t frontColor, uint16_t backColor ,uint16_t backColor2, uint16_t boundryH,const uint8_t* buffer) {
	hw_setOrigin(x,y,w,h);
	uint16_t bColor = 0;
	bColor = backColor;
  for (int16_t i = 0; i < h; i++) {
		if (i + y > boundryH) {
		//if (i > 10) {
			bColor = backColor2;
		}
    for (int16_t j = 0; j < w; j++) {
      int16_t index = ((w+7)>>3)*i+(j>>3);
      int16_t bit = 0x80 >> (j&0x07);
      lcd_wdata((buffer[index]&bit)!=0?frontColor:bColor); //
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
