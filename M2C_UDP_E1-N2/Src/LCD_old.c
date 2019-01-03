#include "LCD.h"
#include "stdlib.h"
#include "stm32f0xx_hal.h"
#include "math.h"
#include "QRencode.h"
#include "stdio.h"
#include "lcd_img.h"
#include "mainConfig.h"
#include "string.h"
#include "qrcode.h"
//#define max(a,b) (a>b?a:b)
//#define min(a,b) (a>b?b:a)
lcd_block_t* g_lcd_start = NULL;

uint16_t g_lcd_xOrigin = 0, g_lcd_yOrigin = 0, g_lcd_rotation = UINT16_MAX;
uint16_t g_lcd_printColor = 0, g_lcd_backColor = 0xffff;
__IO uint8_t g_main_is_warmUp = 1;
uint16_t xOffset=LCD_LEFT_X,yOffset=LCD_TOP_Y;
void lcd_drawGradualBMP(const uint8_t * org,Rect_t * position,uint16_t Startcolor,uint16_t Endcolor,int8_t scale);
//extern	uint8_t RUN_MODE;
static inline void hw_wdata(uint16_t value)
{
  GPIOF->BSRRH = GPIO_PIN_1;
  GPIOF->BSRRL = GPIO_PIN_0;
	GPIOC->ODR = value;
  GPIOA->BSRRH = GPIO_PIN_0;
  GPIOA->BSRRL = GPIO_PIN_0;
  GPIOF->BSRRL = GPIO_PIN_1;
}
static inline void hw_wcmd(uint16_t value)
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
	
	hw_wcmd(0XE1);    //Set Gamma 
	hw_wdata(0x00);
	hw_wdata(0x19);
	hw_wdata(0x1B);
	hw_wdata(0x04);
	hw_wdata(0x10);
	hw_wdata(0x07);
	hw_wdata(0x2A);
	hw_wdata(0x47);
	hw_wdata(0x39);
	hw_wdata(0x03);
	hw_wdata(0x06);
	hw_wdata(0x06);
	hw_wdata(0x30);
	hw_wdata(0x38);
	hw_wdata(0x0F);
	
	hw_wcmd(0x36);    //Memory Access Control
	hw_wdata(0x68);	
		
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
#else //WK28109
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
#if 0 
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
	
	hw_wcmd(0XE1);    //Set Gamma 
	hw_wdata(0x00);
	hw_wdata(0x19);
	hw_wdata(0x1B);
	hw_wdata(0x04);
	hw_wdata(0x10);
	hw_wdata(0x07);
	hw_wdata(0x2A);
	hw_wdata(0x47);
	hw_wdata(0x39);
	hw_wdata(0x03);
	hw_wdata(0x06);
	hw_wdata(0x06);
	hw_wdata(0x30);
	hw_wdata(0x38);
	hw_wdata(0x0F);
	
	hw_wcmd(0x36);    //Memory Access Control
	hw_wdata(0x68);	
		
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
#else //WK28109
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
//static void hw_setScroll(uint16_t top, uint16_t bottom, uint16_t scroll)
//{
//	hw_wcmd(0x33);
//	hw_wdata(top>>8);
//	hw_wdata(top&0xff);
//	hw_wdata(scroll>>8);
//	hw_wdata(scroll&0xff);
//	hw_wdata(bottom>>8);
//	hw_wdata(bottom&0xff);
//	
//	hw_wcmd(0x37);
//	hw_wdata(50>>8);
//	hw_wdata(50&0xff);
//}

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
#ifdef LCD_DRIVER_ST7789
	hw_wdata((mymxmv<<5)|0x0);	    //  000
#else
	hw_wdata((mymxmv<<5)|0x08);	    //  000
	#endif
}

static void hw_setOrigin(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	//used
	#
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
//static void hw_drawOneBitUint(lcd_unit_t* lu)
//{
//	for(uint16_t i = 0; i<lu->height; i++)
//	{
//		for(uint16_t j = 0; j<lu->realWidth; j++)
//		{
//			uint16_t data;
//			if (j>=lu->width)
//			{
//				data = g_lcd_backColor;
//			}
//			else
//			{
//				uint16_t dataPerLine = (lu->width + 0x07)>>3;
//				uint16_t dataIndex = (i*dataPerLine) + (j>>3);
//				uint16_t dataMask = 0x80>>(j&0x07);
//				if ((*(lu->data + dataIndex))&dataMask)
//				{
//					data = g_lcd_printColor;
//				}
//				else
//				{
//					data = g_lcd_backColor;
//				}
//			}
//			hw_wdata(data);
//		}
//	}
//}

//static void hw_drawRealColorUint(lcd_unit_t* lu)
//{
//	for(uint16_t i = 0; i<lu->height; i++)
//	{
//		for(uint16_t j = 0; j<lu->width; j++)
//		{
//			uint16_t data;
//			uint16_t dataIndex = (i * lu->width + j)<<1;
//			data = *(((uint16_t*)(lu->data)) + dataIndex);
//			hw_wdata(data);
//		}
//	}
//}


void lcd_init(void)
{
	#if NEW_M2_PCB
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);//power	

#else	
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_SET);//power
	#endif
	hw_init();


	
}

void lcd_deinit(void)
{
	hw_deinit();
}

//static lcd_block_t* getBlock(uint16_t groupId)
//{
//	lcd_block_t* b = g_lcd_start;
//	while(b)
//	{
//		if(b->groupId == groupId)
//		{
//			return b;
//		}
//		b=b->next;
//	}
//	return NULL;
//}

//static lcd_block_t* newBlock(uint16_t groupId, int16_t type)
//{
//	lcd_block_t* lb = (lcd_block_t*)malloc(sizeof(lcd_block_t));
//	lb->first = NULL;
//	lb->groupId = groupId;
//	lb->type = type;
//	lb->next = g_lcd_start;
//	g_lcd_start = lb;
//	return lb;
//}

//static lcd_unit_t* getUnit(lcd_block_t* lb, uint16_t key)
//{
//	lcd_unit_t* u = lb->first;
//	while(u)
//	{
//		if(u->key == key)
//		{
//			return u;
//		}
//		u=u->next;
//	}
//	return NULL;
//}

//void lcd_regester(uint16_t groupId, int16_t type, lcd_unit_t* uint)
//{
//	lcd_block_t* lb = getBlock(groupId);
//	if (!lb)
//	{
//		lb = newBlock(groupId,type);
//	}
//	if ( lb->type != type)
//	{
//		return;
//	}
//	uint->next = lb->first;
//	lb->first = uint;
//}

void lcd_setOrigin(uint16_t x, uint16_t y)
{
	g_lcd_xOrigin = x;
	g_lcd_yOrigin = y;
}

uint8_t lcd_setRotation(uint16_t degree)
{
	if (degree != g_lcd_rotation)
	{
		g_lcd_rotation = degree;
		hw_setDir(degree);
		return 1;
	}
	return 0;
}

//void lcd_setColor(uint16_t* pPrintColor, uint16_t* pBackColor)
//{
//	if (pPrintColor)
//	{
//		g_lcd_printColor = *pPrintColor;
//	}
//	if (pBackColor)
//	{
//		g_lcd_backColor = *pBackColor;
//	}
//}

//void lcd_fill(uint16_t width, uint16_t height)
//{
//	hw_setOrigin(g_lcd_xOrigin,g_lcd_yOrigin,width,height);
//	hw_fill(width,height,g_lcd_printColor);
//}

void lcd_erase(uint16_t width, uint16_t height)
{
	#ifndef LCD_DRIVER_ST7789
	hw_setOrigin(g_lcd_xOrigin,g_lcd_yOrigin,width,height);
	hw_fill(width,height,g_lcd_backColor);
	#else
	hw_setOrigin(g_lcd_xOrigin,g_lcd_yOrigin,height,width);
	hw_fill(height,width,g_lcd_backColor);	
	#endif
}
void lcd_drawRectangle(Rect_t *pos,uint16_t color)
{
	
	hw_setOrigin(pos->x,pos->y,pos->w,pos->h);
	hw_fill(pos->w,pos->h,color);
	
}

//void lcd_print(uint16_t groupId, char* str, uint8_t anchor)
//{
//	if(str == NULL)
//	{
//		return;
//	}
//	lcd_block_t* lb = getBlock(groupId);
//	if (!lb)
//	{
//		return;
//	}
//	if (str == NULL)
//	{
//		return;
//	}
//	if ((anchor&0xf0) == 0x20 || (anchor&0xf0) == 0x10 )
//	{
//		uint16_t xOffset = 0;
//		char*strt = str;
//		while(*strt != '\0')
//		{
//			uint16_t key = *strt;
//			lcd_unit_t* lu = getUnit(lb,key);
//			if(lu)
//			{
//				xOffset += lu->realWidth;
//			}
//			strt ++;
//		}
//		if ((anchor&0xf0) == 0x20)
//		{
//			xOffset>>=1;
//		}
//		g_lcd_xOrigin -= xOffset;
//	}
//	
//	while(*str != '\0')
//	{
//		uint16_t key = *str;
//		lcd_unit_t* lu = getUnit(lb,key);
//		if(lu)
//		{
//			if(g_lcd_xOrigin<(UINT16_MAX>>1))
//			{
//				hw_setOrigin(g_lcd_xOrigin,g_lcd_yOrigin,lu->realWidth,lu->height);
//				if (lb->type == 1)
//				{
//					hw_drawOneBitUint(lu);
//				}
//				else if(lb->type == 16)
//				{
//					hw_drawRealColorUint(lu);
//				}
//			}
//			g_lcd_xOrigin += lu->realWidth;
//		}
//		str ++;
//	}
//}

//uint16_t lcd_printDirect(uint16_t groupId, char* str, uint8_t anchor)
//{
//	lcd_block_t* lb = getBlock(groupId);
//	uint16_t width = 0;
//	if (lb == NULL)
//	{
//		return 0;
//	}
//	if (str == NULL)
//	{
//		return 0;
//	}
//	lcd_unit_t* lu = lb->first;
//	if (lu == NULL)
//	{
//		return 0;
//	}
//	lcd_unit_t* lut = malloc(sizeof(lcd_unit_t));
//	if (lut == NULL)
//	{
//		return 0;
//	}
//	{
//		char*strt = str;
//		while(*strt != '\0')
//		{
//			uint16_t key = *strt;
//			lut->data = lu->data+((lu->width+7>>3)*lu->height*(key-lu->key));
//			lut->width = lu->width;
//			lut->height = lu->height;
//			lut->realWidth = lcd_getRealWidth(lut)+1;
//			width += lut->realWidth;
//			strt ++;
//		}
//		if ((anchor&0xf0) == 0x20)
//		{
//			g_lcd_xOrigin -= width>>1;
//		}
//		else if ((anchor&0xf0) == 0x10 )
//		{
//			g_lcd_xOrigin -= width;
//		}
//	}
//	while(*str != '\0')
//	{
//		uint16_t key = *str;
//		lut->data = lu->data+((lu->width+7>>3)*lu->height*(key-lu->key));
//		lut->width = lu->width;
//		lut->height = lu->height;
//		lut->realWidth = lcd_getRealWidth(lut)+1;
//		
//		if(g_lcd_xOrigin<(UINT16_MAX>>1))
//		{
//			hw_setOrigin(g_lcd_xOrigin,g_lcd_yOrigin,lut->realWidth,lut->height);
//			hw_drawOneBitUint(lut);
//		}
//		g_lcd_xOrigin += lut->realWidth;
//		str ++;
//	}
//	free(lut);
//	return width;
//}

//void lcd_setPixelColor(uint16_t x, uint16_t y, uint16_t color, uint16_t alpha)
//{
//	hw_setOrigin(x,y,1,1);
//	if (alpha<0x100)
//	{
//		uint16_t oldColor = hw_rdata();
//		color = (color*(uint32_t)alpha + oldColor*(uint32_t)(0x100-alpha))>>8;
//	}
//	hw_wdata(color);
//}

void lcd_test(uint16_t data)
{
	hw_setOrigin(0,0,100,50);
	for(uint16_t i = 0 ; i < 5000;i++)
	{
		hw_wdata(0xf800);
	}
}

/* draw a single color bmp *///2015Äê10ÔÂ14ÈÕ09:41:28
Rect_t pos_pm25 =
{
	.x = 33,
	.y = 102,
	.w = 24,
	.h = 45,
};
//static void lcd_drawREFline(void)
//{
//		Rect_t pos_line = 
//{
//	.x = 120+xOffset,
//	.y = 0+yOffset,
//	.w = 1,
//	.h = 240,
//};
//lcd_drawRectangle(&pos_line,0xffff);
//		Rect_t pos_line2 = 
//{
//	.x = 0+xOffset,
//	.y = 	120+yOffset,
//	.w = 240,
//	.h = 1,
//};
//lcd_drawRectangle(&pos_line2,0xffff);
//}
static void setLevelColor(uint8_t dataType,uint32_t sourceData,uint16_t *color,uint8_t *level)
{
	uint32_t data = sourceData;
	switch (dataType)
	{
		case GAS_TVOC:
		{
		//	data = sourceData/100;
			if (data<=600)
			{
				*level = 0;
				*color = rgb2c(0,228,0);
			}
			else if (data<=1000)
			{
				*level = 2;
				*color = rgb2c(255,255,0);
			}
			else if (data<=2500)
			{
				*level = 3;
				*color = rgb2c(255,64,0);
			}
			else
			{
				*level = 4;
				*color = rgb2c(126,0,35);
			}
		break;
		}		
		case GAS_CH2O:
		{
		//	data = sourceData/100;
			if (data<= 30)
			{
				*level = 0;
				*color = rgb2c (0,255,0);		
			}
			else if (data <= 80)
			{
				*level = 0;
				*color = rgb2c (0,255,0);	
			}
			else if (data <= 300)
			{
				*level = 2;
				*color = rgb2c(255,255,0);
			}
			else if (data <= 500)
			{
				*level = 3;
				*color = rgb2c (255,126,0);
			}
			else if (data <= 750)
			{
				*level = 4;
				*color = rgb2c(255,0,0);			
			}
			else 
			{
				*level = 5;
				*color = rgb2c(153,0,76);			
			}
		break;	
		}
		case TEMPRATURE:
		{
			data = sourceData;
		//	{rgb2c(0,120,255),rgb2c(52,170,220),rgb2c(0,255,0),rgb2c(255,255,0),rgb2c(255,0,0)};
			//uint16_t colorBar[] = {rgb2c(225,0,186),rgb2c(71,144,255),rgb2c(0,228,0),rgb2c(255,64,0),rgb2c(255,0,0)};	
			if (data<= 500)
			{
				*level = 0;
				*color = rgb2c(225,0,186);		
			}
			else if (data <= 1000)
			{
				*level = 1;
				*color = rgb2c(71,144,255);
			}
			else if (data <= 1600)
			{
				*level = 2;
				*color = rgb2c(0,0,228);
			}
			else if (data <= 2400)
			{
				*level = 3;
				*color = rgb2c(0,228,0);
			}
			else if (data <= 3000)
			{
				*level = 4;
				*color = rgb2c(255,255,0);
			}
			else
			{
				*level = 5;
				*color = rgb2c(255,0,0);
			}
		
		break;
		}
		case HUMIDITY:
		{
			data = sourceData;
		//	{rgb2c(0,120,255),rgb2c(52,170,220),rgb2c(0,255,0),rgb2c(255,255,0),rgb2c(255,0,0)};
		//uint16_t colorBar[] = {rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(0,228,0),rgb2c(71,144,255)};	
			if (data <= 4000)
			{
				*level = 0;
				*color = rgb2c(255,64,0);
			}
			else if (data <= 7000)
			{
				*level = 1;
				*color = rgb2c(0,228,0);
			}
			else
			{
				*level = 2;
				*color = rgb2c(71,144,255);					
			}		
		break;
		}
	}	
}
//static void lcd_showLine(Rect_t *pos)
//{
//	hw_setOrigin(pos->x + xOffset+pos->w,pos->y + yOffset + pos->h/2,30,1);
//	for (uint8_t i=0;i<10;i++)
//	{
//		hw_wdata(0xffff);
//	}
//		for (uint8_t i=0;i<10;i++)
//	{
//		hw_wdata(0);
//	}
//		for (uint8_t i=0;i<10;i++)
//	{
//		hw_wdata(0xffff);
//	}

//}
void lcd_drawSpecialBMP(const uint8_t * org,Rect_t * position,uint16_t Fcolor,uint16_t Bcolor,uint16_t percent)
{	
	int8_t scale = 0;
	uint16_t x = position->x, y = position->y,w = (position->w),h = (position->h) ;
	__IO uint16_t bitperline = ((((w -1)>>3)+1)<<3)>>scale;
//	__IO uint16_t x_bytes;
//	x_bytes = ceil(width*1.0/8);
//	bitperline = (ceil(w*1.0/8))*(8>>scale);
	
	if (scale>1)
	{
	bitperline = 32;
	}
	hw_setOrigin(x,y,w,h);
	for (__IO uint32_t i=0;i<(h);i++)
	{
		
	//	uint32_t row_pixel_count = 0;		
		for (__IO uint8_t j=0;j<(w);j++)
	{
	//	for (uint8_t k = 0;k<8;k++)
		{
//			__IO uint8_t mask = 0x80>>k;
			uint16_t  mapindex = (((i>>scale)*bitperline)+(j>>scale))>>3;
			__IO uint8_t mask = 0x80>>((j>>scale)&0x07);
			__IO uint16_t nchar = mapindex;
			nchar = *(org+nchar);
			if (nchar&mask)
			{
				hw_wdata(g_lcd_backColor);		
			}
			else
			{
				if (j<(w*percent/100))
				{
				hw_wdata(Fcolor);
				}
				else
				{
				hw_wdata(Bcolor);
				}
				
				
			}
//			row_pixel_count++;
//			if (row_pixel_count>=width)
//			{
//				break;
//			}
		}	
	}	
	}

}
void lcd_drawBMP(const uint8_t * org,Rect_t * position,uint16_t Fcolor,uint16_t Bcolor,int8_t scale)
{	
	uint16_t x = position->x, y = position->y,w = (position->w),h = (position->h) ;
//	__IO uint16_t bitperline = ((((w -1)>>3)+1)<<3)>>scale;
//	__IO uint16_t x_bytes;
//	x_bytes = ceil(width*1.0/8);
//	bitperline = (ceil(w*1.0/8))*(8>>scale);
//	if (scale==2)
//	{
//	bitperline = w;
//	}
	uint32_t bitperline ;
	if (scale==2)
	{
	bitperline = 32;
	}	
	else
	{
		bitperline = ((((w -1)>>(3+scale)))+1)<<(3+scale)>>scale;
	}
	hw_setOrigin(x,y,w,h);
	for (__IO uint32_t i=0;i<(h);i++)
	{
		
	//	uint32_t row_pixel_count = 0;		
			for (__IO uint8_t j=0;j<(w);j++)
		{
		//	for (uint8_t k = 0;k<8;k++)
			{
		//			__IO uint8_t mask = 0x80>>k;
				uint16_t  mapindex = (((i>>scale)*bitperline)+(j>>scale))>>3;
				__IO uint8_t mask = 0x80>>((j>>scale)&0x07);
				__IO uint16_t nchar = mapindex;
				nchar = *(org+nchar);
				if (nchar&mask)
				{
					hw_wdata(Fcolor);		
				}
				else
				{
					hw_wdata(Bcolor);
					
				}
		//			row_pixel_count++;
		//			if (row_pixel_count>=width)
		//			{
		//				break;
		//			}
			}	
		}	
	}

}
void lcd_drawCompressBMP(const uint8_t * org,Rect_t * position,uint16_t Fcolor,uint16_t Bcolor,int8_t scale,uint8_t percent)
{	
	uint16_t x = position->x, y = position->y,w = (position->w),h = (position->h) ;
	__IO uint16_t bitperline = ((((w -1)>>3)+1)<<3)>>scale;
//	__IO uint16_t x_bytes;
//	x_bytes = ceil(width*1.0/8);
//	bitperline = (ceil(w*1.0/8))*(8>>scale);
	if (scale>1)
	{
	bitperline = 32;
	}
	hw_setOrigin(x,y,w,h);
	__IO uint32_t  readAddress = 0;
	__IO uint32_t totalCount = 0;
	for (__IO uint32_t i=0;i<(h);i++)
	{		
		uint16_t rowCount = 0;
	//	uint32_t row_pixel_count = 0;		
		for (__IO uint8_t j=0;j<(w);j++)
	{
	//	for (uint8_t k = 0;k<8;k++)
		{
//			__IO uint8_t mask = 0x80>>k;
			
			__IO uint8_t mask = 0x80>>((totalCount>>scale)&0x07);
			__IO uint16_t nchar = 0;
			static uint16_t ffCount = 0;
			nchar = *(org+readAddress);
			static uint32_t normalStart = 0;			
			
			if (nchar == 0xff)
			{
				if (ffCount == 0)
				{
					ffCount = *(org+readAddress+1)<<3;
				}
				
				//__IO uint16_t looptimes = ffCount;
				
				//for (uint16_t i=j;i<looptimes;i++)
					if (percent==0xff)
					{
						if (nchar&mask)
						{
							hw_wdata(Bcolor);		
						}
						else
						{
							hw_wdata(Fcolor);
							
						}
					}
					else
					{
						
						if (nchar&mask)
						{
							hw_wdata(g_lcd_backColor);		
						}
						else
						{
							hw_wdata(Fcolor);
							
						}
					}
//						else
//						{
//							if (rowCount<(w*percent/100))
//							{
//							hw_wdata(Bcolor);
//							}
//							else
//							{
//							hw_wdata(Fcolor);
//							}
//							
//							
//						}
//					
//					}
					ffCount --;
					totalCount++;
//					j++;
//					if (i>=w)
//					{
//					break;
//					}
				rowCount++;	
				if (ffCount==0)
				{
					readAddress += 2;
					normalStart = 0;
				}	
			}
			else
			{
				
//				if (nchar&mask)
//				{
//					hw_wdata(Bcolor);		
//				}
//				else
//				{
//					hw_wdata(Fcolor);
//					
//				}
				if (percent==0xff)
					{
						if (nchar&mask)
						{
							hw_wdata(Bcolor);		
						}
						else
						{
							hw_wdata(Fcolor);
							
						}
					}
					else
					{
						
						if (nchar&mask)
						{
							hw_wdata(g_lcd_backColor);		
						}
						else
						{
							if (rowCount<(w*percent/100))
							{
							hw_wdata(Fcolor);
							}
							else
							{
							hw_wdata(Bcolor);
							}
							
							
						}
					
					}
				totalCount++;
				rowCount++;	
				normalStart++;
				if (normalStart >=8)
				{
				readAddress += normalStart>>3;
					normalStart = 0;
				}
				
			}

//			row_pixel_count++;
//			if (row_pixel_count>=width)
//			{
//				break;
//			}
		}	
	}	
	}

}

void lcd_drawRHNumber(Rect_t * position,uint16_t color,uint32_t number,uint8_t dot_position,uint8_t width)
{
//	uint8_t firstzero = 1;
	uint8_t nbit[5]={0};
//	uint8_t dotShowed = 0;
	const static uint16_t div_nums[] = {1000,100,10,1};
	//const uint8_t* pImage[135]={gImage_midNumber,gImage_smallNumber};
	//const uint8_t* dotImage[]={gImage_dot,gImage_smallDot};
  __IO uint16_t currentx = 1;
	 Rect_t nclearArea;
	 uint8_t i, numshownd = 0;
	 uint8_t validDigital = 0;
//	static uint8_t lastDigitals = 0;
	Rect_t tops = {.x = (position->x)+xOffset, .y = (position->y)+yOffset, .w = position->w, .h = position->h};
	
	for (i=0; i<sizeof(nbit)/sizeof(uint8_t); i++)
	{
		nbit[i] = (number/div_nums[i])%10;		
	}
//	width = MIN(width, sizeof(nbit)/sizeof(uint8_t));
	validDigital = 0;//´¢´æÎ»Êý
	uint32_t temp=1;
	do
	{
		++validDigital;
		temp *= 10;
	}while(temp<=number);
	if (dot_position<(width-1))
	{
	validDigital = MAX(2,validDigital);
	}
	else if (validDigital ==0)
	{
	validDigital = 1;
	}
	if (dot_position == 0)
	{
	validDigital = width;
	}
	
	
		
	if (validDigital<width)
	{
		currentx = (tops.w * width  - (validDigital)*tops.w)>>1;
	}
	
//	if (lastDigitals!=validDigital)
//	if (isSmall)
//	{
	nclearArea.h = 19;	
//	}	
//	else
//	{
//	nclearArea.h = 49;
//	}
	nclearArea.w = currentx-1;
	
	nclearArea.x = tops.x+1;
	nclearArea.y = tops.y;
	lcd_drawRectangle(&nclearArea,g_lcd_backColor);		
		
	
	
	for (i = 4 - validDigital;i<4; i++)
	{
		tops.x = position->x + currentx + xOffset;
//		if (i >= dot_position)
//		{
//			firstzero = 0;
//		}
//		if (!firstzero||(nbit[i])!=0)
		{
			//		oled_getdataFromPgm(tmp,notes[nbit[i]]);
	//			if (isSmall)
			{
				lcd_drawBMP(gImage_smallNumber[nbit[i]], &tops,color,g_lcd_backColor,0);
			}
//				else
//			{
//				lcd_drawBMP(gImage_bigNumber[nbit[i]], &tops,color,0x0000,0);
//			}
//			
			numshownd ++;
//			firstzero = 0;
			currentx += tops.w;
		}
		if (numshownd == validDigital)
		{
			break;
		}
		if (i == dot_position &&  dot_position != (width-1))
		{
			Rect_t dot_pos;// = {.w = 4,.h = 4,.y=position->y+24};
							
							
//			if (isSmall)
			{
				dot_pos.h = 19;
				dot_pos.w = 8;	
				dot_pos.y=tops.y;
				dot_pos.x = tops.x + tops.w;	
				lcd_drawBMP(gImage_smallDot,&dot_pos,color,g_lcd_backColor,0);
			}
//			else
//			{
//				dot_pos.w = 12;	
//				dot_pos.h = 49;
//				dot_pos.y=tops.y;
//				dot_pos.x = tops.x + tops.w;	
//				lcd_drawBMP(gImage_bigdot,&dot_pos,0x0000,color,0);
//			}

			currentx += dot_pos.w;
			
//			dotShowed = 1;
		}
	}
//	if (lastDigitals!=validDigital)
	{
		nclearArea.w = tops.w * (width - numshownd) - ((tops.w * width  - (validDigital)*tops.w)>>1)+1;
//		nclearArea.h = 45;
		nclearArea.x = position->x + currentx+xOffset;
		//nclearArea.y = position->y+yOffset;
		lcd_drawRectangle(&nclearArea,g_lcd_backColor);
	}
//	lastDigitals = validDigital;	
}

//show x.xx xx.x xxx number (input max 99999->999,1234->12.3,345->3.45),always have "width" digital display
void lcd_drawBigNumber(Rect_t * position,uint16_t color,uint32_t number,uint8_t width,uint8_t type,uint8_t dot_position)
{
//	uint8_t firstzero = 1;
	uint8_t nbit[5]={0};
//	uint8_t dot_position=0xff;
//	uint8_t dotShowed = 0;
	const static uint16_t div_nums[] = {10000,1000,100,10,1};
  __IO int16_t currentx = 1;
	 Rect_t nclearArea;
	Rect_t dot_pos;
	 __IO uint8_t i, numshownd = 0;
	 uint8_t validDigital = 0;
//	static uint8_t lastDigitals = 0;
	if (type==0)		
	{
		dot_pos.w = 12;	
//		position->w = 38;
//		position->h = 71;
	}
	else if (type==1)		
	{
		dot_pos.w = 12;	
//		position->w = 24;
//		position->h = 49;		
	}
	else if (type==2)		
	{
		dot_pos.w = 8;	
//		position->w = 22;
//		position->h = 43;		
	}	
	else if (type==3)		
	{
		dot_pos.w = 8;	
//		position->w = 10;
//		position->h = 19;		
	}	
	Rect_t tops = {.x = (position->x)+xOffset, .y = (position->y)+yOffset, .w = position->w, .h = position->h};	
	for (i=0; i<sizeof(nbit)/sizeof(uint8_t); i++)
	{
		nbit[i] = (number/div_nums[i])%10;		
	}
	validDigital = 0;//´¢´æÎ»Êý
	uint32_t temp=1;
	do
	{
		++validDigital;
		temp *= 10;
	}while(temp<=number);
	if (validDigital>4)
	{
	//	dot_position = 0xff;
	}	
	else
	{
		//dot_position = 2;
	}
	if (validDigital>4)
	{
	currentx+= dot_pos.w>>1;
	}
		
	
	nclearArea.h = position->h;
	nclearArea.w = currentx;	
	nclearArea.x = tops.x+1;
	nclearArea.y = tops.y;
	lcd_drawRectangle(&nclearArea,g_lcd_backColor);		

	for (i = MIN(5-width,width+dot_position-validDigital);i<5; i++)
	{
		tops.x = position->x + currentx + xOffset;
//		if (i >= dot_position)
//		{
//			firstzero = 0;
//		}
//		if (!firstzero||(nbit[i])!=0)
		if (type==0)
		{
		lcd_drawBMP(gImage_hugeNumber[nbit[i]], &tops,color,g_lcd_backColor,0);
		}
		else if (type==1)
		{
		lcd_drawBMP(gImage_bigNumber[nbit[i]], &tops,color,g_lcd_backColor,0);
		}
		else if (type==2)
		{
		lcd_drawBMP(gImage_midNumber[nbit[i]], &tops,color,g_lcd_backColor,0);
		}
		else if (type==3)
		{
		lcd_drawBMP(gImage_smallNumber[nbit[i]], &tops,color,g_lcd_backColor,0);
		}		
			numshownd ++;
//			firstzero = 0;
			currentx += tops.w;
		
		if (numshownd == width)
		{
			break;
		}
		if (i == dot_position )
		{				
			dot_pos.h = tops.h;
			dot_pos.y=tops.y;
			dot_pos.x = tops.x + tops.w;	
			if (type==0)		
			{					
				lcd_drawBMP(gImage_Hdot,&dot_pos,g_lcd_backColor,color,0);
			}
			else if (type==1)		
			{				
				lcd_drawBMP(gImage_bigdot,&dot_pos,g_lcd_backColor,color,0);
			}
			else if (type==2)		
			{			
				lcd_drawBMP(gImage_middot,&dot_pos,g_lcd_backColor,color,0);
			}
			else if (type==3)		
			{			
				lcd_drawBMP(gImage_smallDot,&dot_pos,g_lcd_backColor,color,0);
			}						
			currentx += dot_pos.w;
			
//			dotShowed = 1;
		}
	}
	if (validDigital>4)
	{
		nclearArea.w = dot_pos.w>>1;

		nclearArea.x = position->x + currentx+xOffset;

		lcd_drawRectangle(&nclearArea,g_lcd_backColor);
	}
//	lastDigitals = validDigital;	

}









Rect_t pos_temp =
{
	.x = 60,
	.y = 181,
	.w = 16,
	.h = 22,
};
Rect_t pos_hum =
{
	.x = 131,
	.y = 181,
	.w = 16,
	.h = 22,
};
static void lcd_showTemprature(uint32_t data,uint16_t color)
{
	lcd_drawRHNumber( &pos_temp,g_lcd_printColor,data,1,2);	
}
static void lcd_showHumidity(uint32_t data,uint16_t color)
{
	lcd_drawRHNumber( &pos_hum,g_lcd_printColor,data,1,2);	
}


static void lcd_showGrade(uint16_t startX, uint16_t startY, uint8_t grade,uint8_t needClear,uint16_t color)
{
	//const uint16_t color[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,126,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};

	uint16_t tColor = color;
	Rect_t pos_exllent = {.x = startX+20+xOffset,.y = startY+yOffset,.w = 13,.h = 14,};	
	Rect_t pos_pollute = {.x = startX+2 +xOffset,.y = startY+yOffset,.w = 13,.h = 14, };
	Rect_t pos_clear 	=  {.x = startX-19+xOffset,.y = startY+yOffset,.w = 80,.h = 15,};
	Rect_t pos_dot = {.x = startX -17 + xOffset,.y = startY + yOffset,.w = 15,.h = 15,};
	if (needClear)
	{
		lcd_drawRectangle(&pos_clear,g_lcd_backColor);
	}
	lcd_drawBMP(gImage_levelDot,&pos_dot,g_lcd_backColor,tColor,0);
	tColor = g_lcd_printColor;
	switch (grade)
	{
		case 0:
		case 1:	
		{			
			const uint8_t* index[] = {gImage_grade[grade]};
			lcd_drawBMP(index[0], &pos_exllent,tColor,g_lcd_backColor,0);	
				break;
		}
		case 2:
		case 3:
		case 4:			
		{			
			const uint8_t* index[] = {gImage_grade[grade],gImage_grade[5],gImage_grade[7],gImage_grade[8]};
			for (uint8_t i=0;i<4;i++)
			{
				lcd_drawBMP(index[i], &pos_pollute,tColor,g_lcd_backColor,0);	
				pos_pollute.x += 16 ;
			}
			break;
		}
		case 5:
		{			
			const uint8_t* index[] = {gImage_grade[6],gImage_grade[4],gImage_grade[7],gImage_grade[8]};
			for (uint8_t i=0;i<4;i++)
			{
				lcd_drawBMP(index[i], &pos_pollute,tColor,g_lcd_backColor,0);	
				pos_pollute.x += 16 ;
			}
			break;
		}
		default:
			break;
	}

	
}

void lcd_showAlignSidesWords (Rect_t* pos_word,const uint8_t** wordsList,uint8_t size,uint8_t totalWidth,uint16_t Fcolor,uint16_t Bcolor)
{
//	const uint8_t  * wordList_p = *wordsList;
//	Rect_t pos_firstWord = {.x =pos_word->x,.y = pos_word->y,.w = pos_word->w,.h = pos_word->h,};	
//	uint8_t gap = ((pos_gradeArea->w)-(pos_word->w * size))/(size + 1)
//	pos_firstWord.x += gap;
//	for (uint8_t i=0;i<size;i++)
//	{
//		lcd_drawBMP((*wordsList) + i, &pos_firstWord,Fcolor,Bcolor,0);	
//		pos_firstWord.x += gap + pos_word->w  ;
//	}	
	
}
void lcd_showHistoryGrade(uint16_t centerX, uint16_t startY, uint16_t length,uint8_t grade,uint8_t needRedraw,uint16_t color,uint16_t mode)
{
//length = 69
//		Rect_t pos_grade = {.x =114+xOffset,.y = 197+yOffset,.w = 15,.h = 14,};	
	
	Rect_t pos_grade = {.x = centerX + xOffset,.y = startY+yOffset,.w = 13,.h = 14,};	
	Rect_t pos_clear 	=  {.x = centerX - (length>>1) + xOffset,.y = startY-2+yOffset,.w = length ,.h = 20,};
	Rect_t pos_roundL =  {.x = centerX - (length>>1) - 7 +xOffset,.y = startY-2+yOffset,.w = 7,.h = 20,};
	Rect_t pos_roundR =  {.x = centerX + (length>>1)  + xOffset,.y = startY-2+yOffset,.w = 7,.h = 20,};
	uint16_t tCenterX = centerX + xOffset;
	
	if (needRedraw)
	{
		lcd_drawBMP(gImage_roundLeft, &pos_roundL,g_lcd_backColor,color,0);	
		lcd_drawBMP(gImage_roundRight, &pos_roundR,g_lcd_backColor,color,0);	
		lcd_drawRectangle(&pos_clear,color);
	}
	uint16_t allLength = length + 2;
	uint16_t startX = centerX - (allLength >> 1);
	uint16_t spacePixel = 0;
	uint8_t halfWord = (pos_grade.w)>>1;
//	if (length >= ((pos_grade.w) + 1)<<2)
//	{
//		spacePixel = (length - ((pos_grade.w)<<2))/5; 
//	}
	if (mode == TEMPRATURE)
	{
		switch (grade)
		{
/*"Êæ",0*//*"ÊÊ",1*//*"Ñ×",2*//*"ÈÈ",3*//*"º®",4*//*"Àä",5*//*"¸É",6*//*"Ôï",7*//*"³±",8*//*"Êª",9*//*"Á¹",10*/
			case 0://º®Àä
			{
				spacePixel = (allLength - ((pos_grade.w)<<1))/3;
				pos_grade.x = startX + spacePixel + xOffset ;
				const uint8_t* index[] = {gImage_tRHgrade[4],gImage_tRHgrade[5]};
				for (uint8_t i=0;i<2;i++)
				{
					lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
					pos_grade.x += (spacePixel + pos_grade.w);
				}
				break;
			}
			case 1://Àä
			{	
				pos_grade.x = centerX + xOffset - halfWord - 1;
				const uint8_t* index[] = {gImage_tRHgrade[5]};
				lcd_drawBMP(index[0], &pos_grade,0xffff,color,0);	
				break;
			}
			case 2://Á¹
			{
				pos_grade.x = centerX + xOffset - halfWord - 1;
				const uint8_t* index[] = {gImage_tRHgrade[10]};
				lcd_drawBMP(index[0], &pos_grade,0xffff,color,0);	
				break;
			}
			case 3://ÊæÊÊ
			{
				spacePixel = (allLength - ((pos_grade.w)<<1))/3;
				pos_grade.x = startX + spacePixel + xOffset ;
				const uint8_t* index[] = {gImage_tRHgrade[0],gImage_tRHgrade[1]};
				for (uint8_t i=0;i<2;i++)
				{
					lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
					pos_grade.x += (spacePixel + pos_grade.w) ;
				}	
				break;
			}
			case 4://ÈÈ
			{
				pos_grade.x = centerX + xOffset - halfWord - 1;
				const uint8_t* index[] = {gImage_tRHgrade[3]};
				lcd_drawBMP(index[0], &pos_grade,0xffff,color,0);	
				break;
			}
			case 5://Ñ×ÈÈ
			{
				spacePixel = (allLength - ((pos_grade.w)<<1))/3;
				pos_grade.x = startX + spacePixel + xOffset ;
				const uint8_t* index[] = {gImage_tRHgrade[2],gImage_tRHgrade[3]};
				for (uint8_t i=0;i<2;i++)
				{
					lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
					pos_grade.x += (spacePixel + pos_grade.w) ;
				}
				break;
			}				
		}
	
	}
	else if (mode == HUMIDITY)
	{
			switch (grade)
		{
			case 0://¸ÉÔï
			{
				spacePixel = (allLength - ((pos_grade.w)<<1))/3;
				pos_grade.x = startX + spacePixel + xOffset ;
				const uint8_t* index[] = {gImage_tRHgrade[6],gImage_tRHgrade[7]};
				for (uint8_t i=0;i<2;i++)
				{
					lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
					pos_grade.x += (spacePixel + pos_grade.w) ;
				}
				break;
			}
			case 1://ÊæÊÊ
			{
				spacePixel = (allLength - ((pos_grade.w)<<1))/3;
				pos_grade.x = startX + spacePixel + xOffset ;
				const uint8_t* index[] = {gImage_tRHgrade[0],gImage_tRHgrade[1]};
				for (uint8_t i=0;i<2;i++)
				{
					lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
					pos_grade.x += (spacePixel + pos_grade.w) ;
				}	
				break;
			}
			case 2://³±Êª
			{
				spacePixel = (allLength - ((pos_grade.w)<<1))/3;
				pos_grade.x = startX + spacePixel + xOffset ;
				const uint8_t* index[] = {gImage_tRHgrade[8],gImage_tRHgrade[9]};
				for (uint8_t i=0;i<2;i++)
				{
					lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
					pos_grade.x += (spacePixel + pos_grade.w);
				}
				break;
			}
			
		}
	
	}
	else 
	{
		//grade = 2;
		switch (grade)
		{
			case 0:
			case 1:	
			{			
				pos_grade.x = centerX + xOffset - halfWord - 1;
				const uint8_t* index[] = {gImage_grade[grade]};
				lcd_drawBMP(index[0], &pos_grade,0xffff,color,0);	
				break;
			}
			case 2:
			case 3:
			case 4:			
			{	
				spacePixel = (allLength - ((pos_grade.w)<<2))/5;
				pos_grade.x = (tCenterX -(allLength >> 1)) + spacePixel;
//				pos_grade.x = tCenterX - (spacePixel<<1) - (spacePixel>>1) - ((pos_grade.w)<<1)  + xOffset ;
//				pos_grade.x = 84 + xOffset;
				//pos_grade.x = centerX - 24 + xOffset;
				const uint8_t* index[] = {gImage_grade[grade],gImage_grade[5],gImage_grade[7],gImage_grade[8]};
				for (uint8_t i=0;i<4;i++)
				{
					lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
					pos_grade.x += spacePixel + pos_grade.w ;
				}
				break;
			}
			case 5:
			{	
				spacePixel = (allLength - ((pos_grade.w)<<2))/5;
				pos_grade.x = (tCenterX -(allLength >> 1)) + spacePixel;
				const uint8_t* index[] = {gImage_grade[6],gImage_grade[4],gImage_grade[7],gImage_grade[8]};
				for (uint8_t i=0;i<4;i++)
				{
					lcd_drawBMP(index[i], &pos_grade,0xffff,color,0);	
					pos_grade.x += spacePixel + pos_grade.w ;
				}
				break;
			}
			default:
				break;
		}
	}
	

//
	
}

void lcd_showBattery(uint8_t level,uint8_t is_charging,uint8_t isPoweroff)
{
//	const uint8_t *batterys[] = {gImage_battery0,gImage_battery1,gImage_battery2,gImage_battery3,gImage_battery4};
	Rect_t pos_innerbattery = 
	{
		.x = 83+xOffset,
		.y = 29+yOffset,
		.w = 19,
		.h = 7,
	};
//	Rect_t pos_battery = 
//	{
//		.x = 79+xOffset,
//		.y = 27+yOffset,
//		.w = 25,
//		.h = 11,
//	};
	static uint8_t showlevel = 100;
	if (is_charging == 1)
	{
	//	uint8_t currentlevel = level;
		showlevel +=4;
		if (showlevel>103)
		{
		showlevel = level;
		}
		if (isPoweroff)
		{
			uint16_t color = rgb2c(0,0,128);
			Rect_t pos_logo =
			{
				.x = 22+xOffset,
				.y = 101+yOffset,
				.w = 200,
				.h = 44,
			};
			lcd_drawCompressBMP(gImage_logo_compressed, &pos_logo,color,rgb2c(32,32,32),0,showlevel);
		//	lcd_drawSpecialBMP(gImage_logo, &pos_logo,color,rgb2c(32,32,32),showlevel);


		}	
		else
		{	
		lcd_drawSpecialBMP(gImage_inBattery,&pos_innerbattery,g_lcd_printColor,0xffff,showlevel);
		}
		
		
	}
	else if (is_charging == 0)
	{
		showlevel = level;
		lcd_drawSpecialBMP(gImage_inBattery,&pos_innerbattery,g_lcd_printColor,0xffff,showlevel);

	}
	else
	{
//		Rect_t pos_battery_notify =
//		{
//			.x = 107+xOffset,
//			.y = 30+yOffset,
//			.w = 30,
//			.h = 16,
//		};

		{
		lcd_drawRectangle(&pos_innerbattery,rgb2c(0,200,0));
//		lcd_drawRectangle();
//		lcd_drawBMP(gImage_inBattery,&pos_innerbattery,rgb2c(0,200,0),g_lcd_backColor,0);
		}
	}



}
//void lcd_showWifiIco(uint8_t status)
//{
//		Rect_t pos_wifi = 
//{
//	.x = 104+xOffset,
//	.y = 23+yOffset,
//	.w = 11,
//	.h = 11,
//};
//if (status == 3)
//{
//lcd_drawBMP(gImage_wifi,&pos_wifi,0,0xffff,0);

//}
//else
//{
//lcd_drawRectangle(&pos_wifi,0);
//}

//}
static void lcd_showWifiSignal(void)
{
	Rect_t pos_hcho_title = 
	{
		.x = 143+xOffset,
		.y = 28+yOffset,
		.w = 22,
		.h = 10,
	};	
	lcd_drawBMP(gImage_wifiSignal, &pos_hcho_title,g_lcd_backColor,g_lcd_printColor,0);
}
void lcd_showBackGround(void)
{

	Rect_t pos_batt = 
	{
		.x = 79+xOffset,
		.y = 27+yOffset,
		.w = 25,
		.h = 11,
	};
	lcd_drawBMP(gImage_battery, &pos_batt,g_lcd_backColor,g_lcd_printColor,0);
	lcd_showWifiSignal();
	


//	lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,0,0xffff,0);

//pos_pm25_unit.x -= 13;
//pos_pm25_unit.y += 40;
//pos_pm25_unit.y += 30;
//lcd_drawBMP(gImage_ugm3,&pos_pm25_unit,0,0xffff,0);


}

static uint16_t qrcode4D(int16_t x, int16_t y,uint8_t* buffer) {
	uint16_t g_qrScale = (256 + 2)>>1;
  int16_t xs = (x * g_qrScale) >>8;
  int16_t ys = (y * g_qrScale) >>8;
  return qrcode_get(buffer,xs,ys)?g_lcd_printColor:g_lcd_backColor;
}
void lcd_showQR(char *data,uint8_t needRefresh)
{
	static uint8_t generated=0;
	uint8_t qrVersion = 8;
#ifdef 	QR_VERSION_6
	Rect_t QR_tag =
	{
		.x = 72+xOffset,
		.y = 45+yOffset,
		.w = (4 * qrVersion + 17)*2,
		.h =  (4 * qrVersion + 17)*2,
	};

	Rect_t wifi_outline =
	{
		.x = QR_tag.x - 2,
		.y = QR_tag.y - 2,
		.w = QR_tag.w + 4,
		.h = QR_tag.h + 4,
//		.x = 0+xOffset,
//		.y = 0+yOffset,
//		.w = 240,
//		.h =  160,
	};	
		Rect_t wifi_clear =
	{
//		.x = QR_tag.x - 2,
//		.y = QR_tag.y - 2,
//		.w = QR_tag.w + 4,
//		.h = QR_tag.h + 4,
		.x = 0+xOffset,
		.y = 0+yOffset,
		.w = 240,
		.h =  165,
	};	
	#endif

//	#define FPE_UID_ADDRESS 0x00110000

	char qrString[DATA_CODE_BYTE];

	static uint8_t* QRbuffer = NULL;
	uint16_t 	pixes_pre_row = (4 * qrVersion + 17);
	if (!generated )
	{
		lcd_drawRectangle(&wifi_clear,g_lcd_printColor);
		sprintf(qrString,"http://app.hw99lt.com?deviceId=%s&hardware=E1_1.0.0_bluesky&software=E1_1.0.0_release",data);
		//char* testString = "123";
//		sprintf(qrString,"http://app.hw99lt.com?deviceId=%s",data);
//		sprintf(testString,"a");
//		 EncodeData(testString);
		//QRbuffer = qrcode_convert((uint8_t*)qrString, strlen(qrString));
		QRbuffer = qrcode_convert((uint8_t*)qrString, strlen(qrString));

//		QRbuffer = QR_stringToBuffer(testString);
	//	QRbuffer = QRcode_encodeString(testString, 5, QR_ECLEVEL_L,QR_MODE_8, 0);
//		for (uint8_t i = 0; i<BYTE_PRE_ROW*PIXES_PRE_ROW; i++)
//		{
//			*(QRbuffer+i) = ~*(QRbuffer+i);
//		}		
//		#else
//		
//		sprintf(testString,"http://weather.hwlantian.com/redirect/intr");
//		QRbuffer = QR_stringToBuffer(testString);
//		for (uint8_t i = 0; i<BYTE_PRE_ROW*PIXES_PRE_ROW; i++)
//		{
//			*(QRbuffer+i) = ~*(QRbuffer+i);
//		}	
//		#endif

	
	}
	if (needRefresh)
	{
		lcd_drawRectangle(&wifi_clear,g_lcd_printColor);
		lcd_drawRectangle(&wifi_outline,g_lcd_backColor);
		uint16_t color = 0;
		hw_setOrigin(QR_tag.x,QR_tag.y,pixes_pre_row*2,pixes_pre_row*2);
		//hw_fill(pos->w,pos->h,color);
		for(uint16_t y = 0; y<pixes_pre_row*2; y++)
			for(uint16_t x = 0; x<pixes_pre_row*2; x++)
		{
			 color = qrcode4D((x),(y),QRbuffer);		
			{
				hw_wdata(color);
			}
		}
	}
//	if (QRbuffer)
//	{
//		free(QRbuffer);
//	}
	
//	lcd_drawGradualBMP(QRbuffer, &QR_tag,rgb2c(0,0,255),0,2);
	generated = 1;		
		


	
}
static uint16_t lcd_drawMutiWords(Rect_t *firstWord,const uint8_t ** org,uint8_t size)
{
	
	for (uint8_t i = 0; i < size ; i++)
	{
		lcd_drawBMP(*(org+i),firstWord,g_lcd_printColor,g_lcd_backColor,0);
		firstWord->x += firstWord->w;
	
	}
	return firstWord->x;

}
void lcd_showWifiStatus(uint8_t wifistatus,uint8_t statusChanged,uint32_t smartingTick)
{

	Rect_t pos_leftnumber =
	{
	.x = 112+xOffset,
	.y = 175+yOffset+15,
	.w = 6,
	.h = 8
	};
	Rect_t pos_rightnumber =
	{
	.x = 120+xOffset,
	.y = 175+yOffset+15,
	.w = 6,
	.h = 8
	};

	Rect_t pos_singleWord =
	{
	.x = 84+xOffset,
	.y = 157+yOffset+15,
	.w = 13,
	.h = 12
	};
	Rect_t pos_clear =
	{
	.x = 40+xOffset,
	.y = 153+yOffset+17,
	.w = 190,
	.h = 56
	};
	//if (statusChanged)
//	{
//		//if (wifistatus ==5 || wifistatus == 1 || wifistatus == 4)
//		{
//			lcd_showQR((char *)data,statusChanged);	
//		}	
//	}
	static uint8_t lastStatus = 0;
	if (lastStatus!=wifistatus)
	{
		lcd_drawRectangle(&pos_clear,g_lcd_backColor);
	
	//	lcd_drawRectangle(&pos_connecting,0);
	}
	lastStatus = wifistatus;
	//lcd_drawBMP(gImage_littlenumber[wifistatus],&pos_leftnumber,0XFFFF,0,0);
	switch (wifistatus)
	{
		case 1: 
		{
			uint8_t s = smartingTick/1000/10;
			uint8_t g = smartingTick/1000%10;
			pos_singleWord.x = 84+xOffset;
		//	lcd_drawBMP(img_phone,&pos_left_phone,rgb2c(0,0,255),0,0);
//			lcd_drawBMP(gImage_wifiword[0],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[1],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[8],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[9],&pos_singleWord,0XFFFF,0,0);
			const uint8_t* imageList[] = {gImage_wifiword[0],gImage_wifiword[1],gImage_wifiword[8],gImage_wifiword[9]};
			pos_singleWord.x = lcd_drawMutiWords(&pos_singleWord,imageList,4);
			Rect_t pos_wifi =
			{
			.x = pos_singleWord.x,
			.y = pos_singleWord.y+2,
			.w = 27,
			.h = 9
			};
			lcd_drawBMP(gImage_wifi,&pos_wifi,g_lcd_backColor,g_lcd_printColor,0);
			lcd_drawBMP(gImage_littlenumber[s],&pos_leftnumber,g_lcd_printColor,g_lcd_backColor,0);
			lcd_drawBMP(gImage_littlenumber[g],&pos_rightnumber,g_lcd_printColor,g_lcd_backColor,0);
		break;
		}
		
//			lcd_drawBMP(gImage_smarting,&pos_connecting,0,rgb2c(0,0,255),0);
//		
//			lcd_drawBMP(img_phone,&pos_left_phone,rgb2c(0,0,255),0,0);
////			lcd_drawBMP(img_connectTips,&pos_connect_tip1,rgb2c(0,0,255),0,0);
////			lcd_drawBMP(img_connectTips2,&pos_connect_tip2,rgb2c(0,0,255),0,0);
//		break;	
		
		case 4: 
			break;

		
		
		//	lcd_drawBMP(img_phone,&pos_left_phone,rgb2c(0,0,255),0,0);
//			lcd_drawBMP(gImage_cloudDown,&pos_cloudDown,0XFFFF,0,0);
		
		case 5:
		{
			pos_singleWord.x = 88+xOffset;
			pos_singleWord.y += 9;
//			lcd_drawBMP(gImage_wifiword[12],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[13],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[14],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
			const uint8_t* imageList[] = {gImage_wifiword[12],gImage_wifiword[13],gImage_wifiword[14]};
			pos_singleWord.x = lcd_drawMutiWords(&pos_singleWord,imageList,3);			
			Rect_t pos_wifi =
			{
				.x = pos_singleWord.x,
				.y = pos_singleWord.y+2,
				.w = 27,
				.h = 9
			};
			lcd_drawBMP(gImage_wifi,&pos_wifi,g_lcd_backColor,g_lcd_printColor,0);
			break;
		}
		case 2:	
		case 3:
		{
			pos_singleWord.x = 88+xOffset;
			lcd_drawBMP(gImage_wifiword[2],&pos_singleWord,g_lcd_printColor,g_lcd_backColor,0);
			pos_singleWord.x += pos_singleWord.w;
			lcd_drawBMP(gImage_wifiword[3],&pos_singleWord,g_lcd_printColor,g_lcd_backColor,0);
			pos_singleWord.x += pos_singleWord.w;
			lcd_drawBMP(gImage_wifiword[4],&pos_singleWord,g_lcd_printColor,g_lcd_backColor,0);
			pos_singleWord.x += pos_singleWord.w;
			Rect_t pos_wifi =
			{
			.x = pos_singleWord.x,
			.y = pos_singleWord.y+2,
			.w = 27,
			.h = 9
			};
			
			lcd_drawBMP(gImage_wifi,&pos_wifi,g_lcd_backColor,g_lcd_printColor,0);
			pos_singleWord.y += pos_singleWord.h+3;
			pos_singleWord.x = 78+xOffset;	
//			lcd_drawBMP(gImage_wifiword[0],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[1],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[3],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[4],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[5],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[6],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[7],&pos_singleWord,0XFFFF,0,0);
//			break;
						const uint8_t* imageList[] = {gImage_wifiword[0],gImage_wifiword[1],gImage_wifiword[3],gImage_wifiword[4],gImage_wifiword[5],gImage_wifiword[6],gImage_wifiword[7]};
			pos_singleWord.x = lcd_drawMutiWords(&pos_singleWord,imageList,7);
			break;			
		}
		
		case 6:
		{
			pos_singleWord.x = 81+xOffset;
			pos_singleWord.y += 9;
//			lcd_drawBMP(gImage_wifiword[2],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[3],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[4],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[5],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[6],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[7],&pos_singleWord,0XFFFF,0,0);
						const uint8_t* imageList[] = {gImage_wifiword[2],gImage_wifiword[3],gImage_wifiword[4],gImage_wifiword[5],gImage_wifiword[6],gImage_wifiword[7]};
			pos_singleWord.x = lcd_drawMutiWords(&pos_singleWord,imageList,6);
			break;
		}
		
		case 0:
		//	lcd_drawBMP(img_phone,&pos_left_phone,rgb2c(0,0,255),0,0);
//			lcd_drawBMP(gImage_isSmarting,&pos_smarting,0XFFFF,0,0);
		default:

			break;
	}
		
			pos_singleWord.y = 190+yOffset +15;
//	for (uint8_t i=0;i<9;i++)
	{
//			lcd_drawBMP(gImage_wifiword[i+15],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[3],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[4],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[5],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[6],&pos_singleWord,0XFFFF,0,0);
//			pos_singleWord.x += pos_singleWord.w;
//			lcd_drawBMP(gImage_wifiword[7],&pos_singleWord,0XFFFF,0,0);
	}
#if MAC2UID
				pos_singleWord.x = 70+xOffset;
			const uint8_t* imageList[] = {gImage_wifiword[15],gImage_wifiword[16],gImage_wifiword[17],gImage_wifiword[18],gImage_wifiword[19],gImage_wifiword[20],gImage_wifiword[21],gImage_wifiword[22]};
	lcd_drawMutiWords(&pos_singleWord,imageList,8);
			#else
			pos_singleWord.x = 64+xOffset;
			const uint8_t* imageList[] = {gImage_wifiword[15],gImage_wifiword[16],gImage_wifiword[17],gImage_wifiword[18],gImage_wifiword[19],gImage_wifiword[20],gImage_wifiword[21],gImage_wifiword[22],gImage_wifiword[23]};
	lcd_drawMutiWords(&pos_singleWord,imageList,9);
				#endif
			
}
void lcd_showlogo(uint16_t color,uint8_t mode)
{
	if (mode == 0)
	{
	Rect_t pos_logo =
	{
		.x = 22+xOffset,
		.y = 92+yOffset,
		.w = 200,
		.h = 44,
	};
			Rect_t pos_TYPE =
	{
		.x = 60+xOffset,
		.y = 182+yOffset,
		.w = 123,
		.h = 23,
	};
		#if M1_MODE
		lcd_drawBMP(gImage_n2,&pos_TYPE,g_lcd_backColor,rgb2c(80,255,255),0);
		#else
		lcd_drawBMP(gImage_n2, &pos_TYPE,g_lcd_backColor,g_lcd_printColor,0);
		#endif
	
	//lcd_drawBMP(gImage_logo, &pos_logo,0,color,0);
	lcd_drawCompressBMP(gImage_logo_compressed, &pos_logo,color,g_lcd_backColor,0,0xff);
	
	}
	else
	{
		Rect_t pos_battery_notify; 
		{
			pos_battery_notify.x = 92+xOffset;
			pos_battery_notify.y = 106+yOffset;
			pos_battery_notify.w = 60;
			pos_battery_notify.h = 32;
			lcd_drawBMP(gImage_charge,&pos_battery_notify,g_lcd_backColor,g_lcd_printColor,1);
		}	
	}
			Rect_t pos_version =
	{
		.x = 98+xOffset,
		.y = 220+yOffset,
		.w = 6,
		.h = 8,
	};	
	uint8_t s[4];
	uint16_t ncolor = rgb2c(64,64,64);
	for(uint8_t i = 0;i < strlen(DEVICE_VERSION);i ++)
	{
		char* string = DEVICE_VERSION;
		if (*(string + i) == ' '|| *(string + i) == '.')
		{
			uint8_t c = *(string + i + 1) - '0';
			pos_version.x+= 7;
			lcd_drawBMP(gImage_littlenumber[c],&pos_version,ncolor,g_lcd_backColor,0);
		}
	}

}

void lcd_showSingle(uint32_t data_pm,uint8_t gas_type,uint16_t data_battery,uint8_t is_charging,uint8_t needRedraw)
{
		uint8_t level = 0;
		uint16_t color = 0xffff;
		Rect_t pos_allclear =
	{
		.x = 34+xOffset,
		.y = 62+yOffset,
		.w = 180,
		.h = 151,
	};
		Rect_t pos_pm25 =
	{
		.x = 57,
		.y = 96,
		.w = 36,
		.h = 49,
	};
		Rect_t pos_arrowL = 
	{
		.x = 22+xOffset,
		.y = 125+yOffset,
		.w = 9,
		.h = 10,
	};
		Rect_t pos_arrowR = 
	{
		.x = 218+xOffset,
		.y = 125+yOffset,
		.w = 9,
		.h = 10,
	};
		Rect_t pos_pm25_unit = 
	{
		.x = 148+xOffset,
		.y = 168+yOffset,
		.w = 36,
		.h = 12,
	};


	Rect_t pos_RHtitle = 
	{
		.x = 96+xOffset,
		.y = 64+yOffset,
		.w = 13,
		.h = 14,
	};
	Rect_t pos_block = 
	{
		.x = 12+xOffset,
		.y = 116+yOffset,
		.w = 7,
		.h = 12,
	};	
		Rect_t pos_line = 
	{
		.x = 121+xOffset,
		.y = 166+yOffset,
		.w = 1,
		.h = 14,
	};
	uint8_t gradeNeedClear = 0;
	const uint16_t grayColor = rgb2c(32,32,32);
	if (needRedraw)
	{
		gradeNeedClear = 1;
		lcd_showBackGround();		
		lcd_drawRectangle(&pos_allclear,g_lcd_backColor);
	}
	lcd_drawRectangle(&pos_block,rgb2c(128,128,128));
	pos_block.x = 223+xOffset;
	lcd_drawRectangle(&pos_block,rgb2c(128,128,128));
	lcd_drawRectangle(&pos_line,grayColor);
//	lcd_drawBMP(gImage_pm25title,&pos_pm25title,0,0xffff,0);
	switch (gas_type)
	{
		case GAS_CH2O:
		{
			pos_RHtitle.x = 62+xOffset;
			pos_RHtitle.y = 166+yOffset;
			lcd_drawBMP(gImage_jia,&pos_RHtitle,g_lcd_printColor,g_lcd_backColor,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_quan,&pos_RHtitle,g_lcd_printColor,g_lcd_backColor,0);
			lcd_drawBMP(gImage_MGM3,&pos_pm25_unit,g_lcd_backColor,grayColor,0);
			if (g_main_is_warmUp == 1)
			{
				Rect_t pos_preheat = 
				{	
					.x = 100 + xOffset,
					.y = 115 +yOffset,
					.w = 48,
					.h = 16,
				};
				lcd_drawBMP(gImage_preheat, &pos_preheat,g_lcd_backColor,g_lcd_printColor,0);
			}
			else
			{
				setLevelColor(GAS_CH2O,data_pm,&color,&level);
				pos_pm25.x -= pos_pm25.w>>1;
				lcd_drawBigNumber(&pos_pm25,g_lcd_printColor,data_pm,4,0,1);
			}
			//lcd_drawBigNumber(&pos_pm25,color,data_pm,3,0,2);
			break;
		}
			case GAS_TVOC:
		{
			pos_RHtitle.x = 62+xOffset;
			pos_RHtitle.y = 168+yOffset;
			pos_RHtitle.w = 39;
			pos_RHtitle.h = 10;
			lcd_drawBMP(gImage_tvocTitle, &pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);
			lcd_drawBMP(gImage_MGM3,&pos_pm25_unit,g_lcd_backColor,grayColor,0);
			if (g_main_is_warmUp == 1)
			{
				Rect_t pos_preheat = 
				{	
					.x = 100 + xOffset,
					.y = 115 + yOffset,
					.w = 48,
					.h = 16,
				};
				lcd_drawBMP(gImage_preheat, &pos_preheat,g_lcd_backColor,g_lcd_printColor,0);
			}
			else
			{
				setLevelColor(GAS_CH2O,data_pm,&color,&level);
				pos_pm25.x -= pos_pm25.w>>1;
				lcd_drawBigNumber(&pos_pm25,g_lcd_printColor,data_pm,4,0,1);
			}
			//lcd_drawBigNumber(&pos_pm25,color,data_pm,3,0,2);
			break;
		}
		case TEMPRATURE:
		{
			pos_RHtitle.x = 62+xOffset;
			pos_RHtitle.y = 166+yOffset;
			lcd_drawBMP(gImage_wen,&pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_du,&pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);

			setLevelColor(TEMPRATURE,data_pm,&color,&level);	
			lcd_drawBigNumber(&pos_pm25,g_lcd_printColor,data_pm,3,0,2);
			
			
				Rect_t pos_temp_unit = 
			{
				.x = 158+xOffset,
				.y = 168+yOffset,
				.w = 14,
				.h = 9,
			};				
			lcd_drawBMP(gImage_sheshidu,&pos_temp_unit,g_lcd_backColor,g_lcd_printColor,0);
		

		break;
		}
		case HUMIDITY:
		{
			pos_RHtitle.x = 62+xOffset;
			pos_RHtitle.y = 166+yOffset;
			lcd_drawBMP(gImage_shi,&pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_du,&pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);			
			setLevelColor(HUMIDITY,data_pm,&color,&level);
			lcd_drawBigNumber(&pos_pm25,g_lcd_printColor,data_pm,3,0,2);					
			Rect_t pos_rh_unit = 
			{
				.x = 158+xOffset,
				.y = 168+yOffset,
				.w = 10,
				.h = 10,
			};
			
			lcd_drawBMP(gImage_percent2,&pos_rh_unit,g_lcd_backColor,g_lcd_printColor,0);		
		break;
		}	
		default: break;
	
	}
	

		static uint8_t lastLevel = 0;
		if (lastLevel != level)
		{
			needRedraw = 1;
		}
		lastLevel = level;
		
		lcd_showHistoryGrade(120,198,70,level,needRedraw,color,gas_type);
		
	//	setLevelColor(GAS_PM1,data_pm[1],&color,&level);
		
		//lcd_showBackGround();		
	
//		lcd_drawBMP(gImage_arrowLeft,&pos_arrowL,g_lcd_backColor,g_lcd_printColor,0);
//		lcd_drawBMP(gImage_arrowRight,&pos_arrowR,g_lcd_backColor,g_lcd_printColor,0);
		lcd_showBattery(data_battery,is_charging,0);
//			lcd_showWifiIco(wifistatus);

}

static void lcd_drawColorBlock(uint16_t startX,uint16_t startY,uint16_t color,uint8_t length)
{
			Rect_t pos_Lcircle =
		{
			.x = startX + xOffset,
			.y = startY + yOffset,
			.w = 3,
			.h = 7,
		};
		Rect_t pos_Rcircle =
		{
			.x = startX + length + 3 + xOffset,
			.y = startY  + yOffset,
			.w = 3,
			.h = 7,
		};
		Rect_t pos_rectangle =
		{
			.x = startX + 3 + xOffset,
			.y = startY  + yOffset,
			.w = length,
			.h = 7,
		};		
	
	lcd_drawRectangle(&pos_rectangle,color);
	lcd_drawBMP(gImage_leftCircle,&pos_Lcircle,color,g_lcd_backColor,0);
	lcd_drawBMP(gImage_rightCircle,&pos_Rcircle,color,g_lcd_backColor,0);
}

void lcd_showAll(uint32_t* data_pm,uint16_t data_battery,uint8_t is_charging,uint8_t needRedraw)
{
		uint8_t level = 0;
		uint16_t color = 0;
		Rect_t pos_mainnumber =
		{
			.x = 22,
			.y = 78,
			.w = 26,
			.h = 36,
		};
	Rect_t pos_hcho_title = 
	{
		.x = 154+xOffset,
		.y = 79+yOffset,
		.w = 30,
		.h = 12,
	};
	Rect_t pos_tvoc_title = 
	{
		.x = 151+xOffset,
		.y = 136+yOffset,
		.w = 39,
		.h = 10,
	};
	Rect_t pos_hcho_unit = 
	{
		.x = 194+xOffset,
		.y = 82+yOffset,
		.w = 36,
		.h = 12,
	};
	Rect_t pos_tvoc_unit = 
	{
		.x = 194+xOffset,
		.y = 137+yOffset,
		.w = 36,
		.h = 12,
	};


	Rect_t pos_temp_unit = 
	{
		.x = 97+xOffset,
		.y = 191+yOffset,
		.w = 12,
		.h = 13,
	};

	Rect_t pos_rh_unit = 
	{
		.x = 170+xOffset,
		.y = 191+yOffset,
		.w = 11,
		.h = 13,
	};
	


		//lcd_drawRHNumber(&pos_pm25,color,data_pm[0],1,3,0);
		
	//		lcd_showGrade(&pos_grade,level,needRedraw,color,GAS_PM25);
			
//			if (RUN_MODE == 'N')
		uint8_t gradeNeedClear = 0;
		if (needRedraw)
		{
			gradeNeedClear = 1;
			lcd_showBackGround();		
		}
		
	uint16_t fColor = rgb2c(64,64,64);
//lcd_drawBMP(gImage_pm10ico, &pos_pm10_ico,0,0xffff,0);

	lcd_drawBMP(gImage_tvocTitle, &pos_tvoc_title,g_lcd_backColor,fColor,0);
	lcd_drawBMP(gImage_jqtitle, &pos_hcho_title,g_lcd_backColor,fColor,0);
	
	fColor = rgb2c(128,128,128);
	lcd_drawBMP(gImage_MGM3,&pos_hcho_unit,g_lcd_backColor,fColor,0);
	lcd_drawBMP(gImage_MGM3,&pos_tvoc_unit,g_lcd_backColor,fColor,0);	

//	lcd_drawBMP(gImage_ttt,&pos_ttt,0,0xffff,0);
	lcd_drawBMP(gImage_temp,&pos_temp_unit,g_lcd_backColor,fColor,0);
//	lcd_drawBMP(gImage_rh,&pos_rh,0,0xffff,0);
	lcd_drawBMP(gImage_percent,&pos_rh_unit,g_lcd_backColor,fColor,0);
	if (g_main_is_warmUp == 1)
	{
		Rect_t pos_preheat = 
		{	
			.x = 84 + xOffset,
			.y = 84 +yOffset,
			.w = 48,
			.h = 16,
		};
		lcd_drawBMP(gImage_preheat, &pos_preheat,g_lcd_backColor,g_lcd_printColor,0);
	}
	else
	{
		static uint8_t lastCh2oLevel,lastTvocLevel;
		setLevelColor(GAS_CH2O,data_pm[0],&color,&level);	
		if (lastCh2oLevel != level)
		{
			lastCh2oLevel = level;
			gradeNeedClear = 1;
		}
		lcd_drawBigNumber(&pos_mainnumber,g_lcd_printColor,data_pm[0],4,1,1);
		lcd_showGrade(166,98,level,gradeNeedClear,color);
		
		//lcd_drawBMP(gImage_leftCircle,&pos_Lcircle,0,tColor,0);
		
		pos_mainnumber.y = 132;
		setLevelColor(GAS_TVOC,data_pm[1],&color,&level);	
		if (lastTvocLevel != level)
		{
			lastTvocLevel = level;
			gradeNeedClear = 1;
		}
		lcd_drawBigNumber(&pos_mainnumber,g_lcd_printColor,data_pm[1],4,1,1);
		lcd_showGrade(166,154,level,gradeNeedClear,color);
		
	}
	setLevelColor(TEMPRATURE,data_pm[2],&color,&level);				
	lcd_showTemprature(data_pm[2]/100,color);
	lcd_drawColorBlock(56,209,color,48);
	
	setLevelColor(HUMIDITY,data_pm[3],&color,&level);	
	lcd_showHumidity(data_pm[3]/100,color);				
	lcd_drawColorBlock(128,209,color,48);

		Rect_t pos_line = 
	{
		.x = 11+xOffset,
		.y = 122+yOffset,
		.w = 4,
		.h = 1,
	};
	for(uint16_t x=pos_line.x;x < 225+xOffset;x += 6)
	{
		pos_line.x = x;
		lcd_drawRectangle(&pos_line,rgb2c(64,64,64));
	

	}
	lcd_showBattery(data_battery,is_charging,0);
//		Rect_t pos_cirlce = 
//	{	
//		.x = 58 + xOffset,
//		.y = 209 +yOffset,
//		.w = 4,
//		.h = 7,
//	};		

////			lcd_showWifiIco(wifistatus);

}
void lcd_showMark(uint16_t *array,uint8_t size,uint8_t dot_postion)
{
		Rect_t pos_colorBar = 
	{
		.x = 195+xOffset,
		.y = 70+yOffset,
		.w = 7,
		.h = 120/(size+1),
	};
	Rect_t pos_barMark = 
	{
		.x = 203+xOffset,
		.y = 82+yOffset,
		.w = 6,
		.h = 8,
	};
	Rect_t pos_barMarkBase = 
	{
		.x = 204+xOffset,
		.y = 70+yOffset,
		.w = 6,
		.h = 8,
	};
	
		for (uint8_t i=0;i<size;i++)
	{
		__IO uint8_t bw = *(array+i)/100;
		__IO uint8_t sw = *(array+i)%100/10;
		__IO uint8_t gw = *(array+i)%10;
		uint8_t firstshowed = 0;
				pos_barMark.x = pos_barMarkBase.x;
		pos_barMark.y = pos_barMarkBase.y + pos_colorBar.h - pos_barMark.h;
		if (bw!=0 || dot_postion != 0xff)
		{
			
			firstshowed = 1;
			lcd_drawBMP(gImage_littlenumber[bw],&pos_barMark,g_lcd_printColor,g_lcd_backColor,0);
			pos_barMark.x += pos_barMark.w;
			if (dot_postion == 0)
			{
				Rect_t pos_dot=
				{
					.x = pos_barMark.x ,
					.y = pos_barMark.y + pos_barMark.h -1,
					.w = 1,
					.h = 1,
				};
				lcd_drawRectangle(&pos_dot,g_lcd_printColor);
				pos_barMark.x += pos_barMark.w>>1;
			
			}
		}

		if (sw!=0 || firstshowed ||  dot_postion == 1)
		{
			firstshowed = 1;
			lcd_drawBMP(gImage_littlenumber[sw],&pos_barMark,g_lcd_printColor,g_lcd_backColor,0);
			pos_barMark.x += pos_barMark.w;
			if (dot_postion == 1)
			{
				Rect_t pos_dot=
				{
					.x = pos_barMark.x,
					.y = pos_barMark.y + pos_barMark.h-1 ,
					.w = 1,
					.h = 1,
				};
				lcd_drawRectangle(&pos_dot,g_lcd_printColor);
				pos_barMark.x += pos_barMark.w>>1;
			
			}
		}
		if (gw!=0 || firstshowed)
		{
				lcd_drawBMP(gImage_littlenumber[gw],&pos_barMark,g_lcd_printColor,g_lcd_backColor,0);
		}
		pos_barMarkBase.y+= pos_colorBar.h;
	
	}
		


}
uint8_t lcd_showColorGrade(uint32_t value,uint16_t startX,uint16_t startY,uint16_t width ,uint16_t* colorList,uint8_t size)
{

	uint8_t numberOfColor = size;

	Rect_t pos_colorBar = 
	{
//		.x = 40,
//		.y = 157,
		.w = 7,
	};	
//	if (width < 80)
//	{
//		pos_colorBar.w = 4;
//	}
	pos_colorBar.h = width/size;
	pos_colorBar.x = startX;
	pos_colorBar.y = startY;	
	
//	pos_colorBar.h = barHeight/6;
	for (uint8_t i=0;i<numberOfColor;i++)
	{
		lcd_drawRectangle(&pos_colorBar,colorList[numberOfColor-i-1]);
		pos_colorBar.y += pos_colorBar.h;
//		lcd_drawRectangle(&pos_outLine2,gapColor);
//		pos_outLine2.x = pos_colorBar.x ;
	//	lcd_drawRectangle(&pos_outLine2,0xffff);		
	}


	uint8_t i = 1;
//	for (; i < numberOfColor ; i ++)
//	{
//		if (value < gradeList[i] )
//		{
//			break;
//		}
//	}

	return (i-1);
}
void lcd_showhistory(uint32_t* data,uint16_t size ,uint8_t Mode,uint32_t mainNumber,uint8_t needRedraw)
{
//		Rect_t pos_ttt = 
//	{
//		.x = 158+xOffset,
//		.y = 85+yOffset,
//		.w = 4,
//		.h = 13,
//	};
//	Rect_t pos_rh = 
//	{
//		.x = 158+xOffset,
//		.y = 161+yOffset,
//		.w = 15,
//		.h = 13,
//	};

//	Rect_t pos_title =
//	{
//		.x = 174+xOffset,
//		.y = 142+yOffset,
//		.w = 36,
//		.h = 10,
//	};
	Rect_t pos_allclear =
	{
		.x = 29+xOffset,
		.y = 67+yOffset,
		.w = 195,
		.h = 174,
	};
	Rect_t pos_main_number =
	{
		.x = 46 ,
		.y = 182,
		.w = 16,
		.h = 22,
	};
	Rect_t pos_triangle = 
	{
		.x = 173+xOffset,
		.y = 154+yOffset,
		.w = 10,
		.h = 9,
	};
	Rect_t pos_pm25_unit = 
	{
		.x = 98+xOffset,
		.y = 219+yOffset,
		.w = 36,
		.h = 12,
	};


		Rect_t pos_RHtitle = 
	{
		.x = 137+xOffset,
		.y = 172+yOffset,
		.w = 13,
		.h = 14,
	};
	Rect_t pos_colorBar = 
	{
		.x = 195+xOffset,
		.y = 70+yOffset,
		.w = 7,
		.h = 120,
	};
	Rect_t pos_line = 
	{
		.x = 30+xOffset,
		.y = 152+yOffset,
		.w = 4,
		.h = 1,
	};
	uint16_t color = 0;
	uint8_t level = 0;

	if (needRedraw)
	{
		lcd_drawRectangle(&pos_allclear,g_lcd_backColor);
		lcd_showBackGround();		
	}
	
	{		
	//	lcd_drawBMP(pImage[Mode],&pos_title,0,0xffff,0);
		lcd_drawBMP(gImage_arrowup,&pos_triangle,g_lcd_backColor,g_lcd_printColor,0);

	for(uint16_t x=pos_line.x;x < 180+xOffset;x += 5)
	{
		pos_line.x = x;
		lcd_drawRectangle(&pos_line,rgb2c(64,64,64));
	

	}
		
		
		
		
		 uint32_t tempbuffer[size];
	
		 uint32_t max=0;

	 if (Mode == TEMPRATURE)
		{

			max = 3000;
			uint16_t colorBar[] = {rgb2c(225,0,186),rgb2c(71,144,255),rgb2c(0,0,228),rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,0,0)};	
			lcd_showColorGrade(mainNumber,pos_colorBar.x,pos_colorBar.y,120 ,colorBar,6);	
//			pos_colorBar.h = barHeight/6;	
//			for (uint8_t i=0;i<6;i++)
//			{
//			lcd_drawRectangle(&pos_colorBar,colorBar[5-i]);
//				pos_colorBar.y += pos_colorBar.h;
//				
//			}
			uint16_t mark[5] = {30,24,16,10,5};
			lcd_showMark(mark,5,0xff);			
			lcd_drawBMP(gImage_wen,&pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_du,&pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);
			//uint32_t showNumber = mainNumber;
			setLevelColor(Mode,mainNumber,&color,&level);
//			//pos_main_number.x += pos_main_number.w;
//			lcd_drawBigNumber(&pos_main_number,color,mainNumber,3,2,2);
		//	lcd_drawBMP(gImage_sheshidu,&pos_temp_unit,g_lcd_backColor,rgb2c(64,64,64),0);
//			for (uint8_t i=0;i<size;i++)
//			{			
//			tempbuffer[i] = (*(data+i));
//			//	__IO uint32_t testnumber = tempbuffer[i];
//			}
		}
		else if (Mode == HUMIDITY)
		{

			max = 6500;
			uint16_t colorBar[] = {rgb2c(255,64,0),rgb2c(0,228,0),rgb2c(71,144,255)};	
			lcd_showColorGrade(mainNumber,pos_colorBar.x,pos_colorBar.y,120 ,colorBar,3);	

			uint16_t mark[2] = {70,40};
			lcd_showMark(mark,2,0xff);			
			lcd_drawBMP(gImage_shi,&pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_du,&pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);
			//uint32_t showNumber = (mainNumber&0xff);
			setLevelColor(Mode,mainNumber,&color,&level);
			//lcd_drawBMP(gImage_percent2,&pos_rh_unit,g_lcd_backColor,rgb2c(64,64,64),0);
//			for (uint8_t i=0;i<size;i++)
//			{			
//			tempbuffer[i] = (*(data+i));
//			}			
		}
		else if (Mode == GAS_CH2O)
		{
			uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(255,0,0),rgb2c(153,0,76),rgb2c(126,0,35)};
//			pos_colorBar.h = barHeight/5;
//			for (uint8_t i=0;i<5;i++)
//			{
//				lcd_drawRectangle(&pos_colorBar,colorBar[4-i]);
//				pos_colorBar.y += pos_colorBar.h;
//				
//			}
			lcd_showColorGrade(mainNumber,pos_colorBar.x,pos_colorBar.y,120 ,colorBar,5);	
			uint16_t mark[4] = {75,50,30,8};
			lcd_showMark(mark,4,0);
			
			pos_RHtitle.x = 137+xOffset;
			pos_RHtitle.y = 172+yOffset;
			lcd_drawBMP(gImage_jia,&pos_RHtitle,g_lcd_printColor,g_lcd_backColor,0);
			pos_RHtitle.x += 21;
			lcd_drawBMP(gImage_quan,&pos_RHtitle,g_lcd_printColor,g_lcd_backColor,0);
			//lcd_drawBMP(gImage_MGM3,&pos_pm25_unit,g_lcd_backColor,rgb2c(64,64,64),0);
			
			if (g_main_is_warmUp == 1)
			{
					Rect_t pos_preheat = 
					{	
						.x = 60 + xOffset,
						.y = 185 +yOffset,
						.w = 48,
						.h = 16,
					};
					lcd_drawBMP(gImage_preheat, &pos_preheat,g_lcd_backColor,g_lcd_printColor,0);
					setLevelColor(Mode,0,&color,&level);
			}	
			else
			{
				pos_main_number.x -= pos_main_number.w>>1;
				setLevelColor(Mode,mainNumber,&color,&level);
				lcd_drawBigNumber(&pos_main_number,g_lcd_printColor,mainNumber,4,3,1);
			}
			max = 300;
		}
		else if (Mode == GAS_TVOC)
		{
			uint16_t colorBar[] = {rgb2c(0,228,0),rgb2c(255,255,0),rgb2c(255,64,0),rgb2c(153,0,76)};
//			pos_colorBar.h = barHeight/5;
//			for (uint8_t i=0;i<5;i++)
//			{
//				lcd_drawRectangle(&pos_colorBar,colorBar[4-i]);
//				pos_colorBar.y += pos_colorBar.h;
//				
//			}
			lcd_showColorGrade(mainNumber,pos_colorBar.x,pos_colorBar.y,120 ,colorBar,4);	
			uint16_t mark[3] = {250,100,60};
			lcd_showMark(mark,3,0);
			
			pos_RHtitle.x = 135+xOffset;
			pos_RHtitle.y = 173+yOffset;
			pos_RHtitle.w = 39;
			pos_RHtitle.h = 10;
			lcd_drawBMP(gImage_tvocTitle,&pos_RHtitle,g_lcd_backColor,g_lcd_printColor,0);
			//lcd_drawBMP(gImage_MGM3,&pos_pm25_unit,g_lcd_backColor,rgb2c(64,64,64),0);
			
			if (g_main_is_warmUp == 1 )
			{
					Rect_t pos_preheat = 
					{	
						.x = 60 + xOffset,
						.y = 185 +yOffset,
						.w = 48,
						.h = 16,
					};
					lcd_drawBMP(gImage_preheat, &pos_preheat,g_lcd_backColor,g_lcd_printColor,0);
				setLevelColor(Mode,0,&color,&level);
			}	
			else
			{
				pos_main_number.x -= pos_main_number.w>>1;
				setLevelColor(Mode,mainNumber,&color,&level);
				lcd_drawBigNumber(&pos_main_number,g_lcd_printColor,mainNumber,4,3,1);
			}

			max = 300;

		}
	//	lcd_showTips(Mode,mainNumber,level);
		static uint8_t lastLevel = 0;
		if (lastLevel != level)
		{
			needRedraw = 1;
		}
		lastLevel = level;
	//	setLevelColor(Mode,mainNumber,&color,&level);
		lcd_showHistoryGrade(153,190,50,level,needRedraw,color,Mode);		
					
			//pos_main_number.x += pos_main_number.w;
		if (Mode != GAS_CH2O && Mode != GAS_TVOC )
		{
			lcd_drawBigNumber(&pos_main_number,g_lcd_printColor,mainNumber,3,3,2);
		}
		//__IO uint16_t tempbuffer[size];		
		

			Rect_t pos_area =
		{
			.x = 180+xOffset,
			.y = 147+yOffset,
			.w = 150,
			.h = 60,
		};
			Rect_t pos_col =
		{
			.x = 180+xOffset,
			.y = 147+yOffset,
			.w = 4,
			.h = pos_area.h,
		};
		Rect_t pos_clear =
		{
			.x = 198+xOffset,
			.y = 85+yOffset,
			.w = 4,
			.h = 20,
		};
		
		__IO uint32_t tHeight = 0;

		for (uint8_t i=0;i<size;i++)
		{
			tempbuffer[i] = *(data+i);
			if (max<(tempbuffer[i]))
			{
			max = tempbuffer[i];
			}
		}
		for (uint8_t i=0;i<size;i++)
		{
			//tempbuffer[i] = fmax(1,(*(data+i)*1.0)/max * pos_area.h) ;
			tHeight = ((tempbuffer[i]<<4)/max * pos_area.h)>>4;
			tHeight = MAX(1,tHeight) ;
//		}			
//		for (uint8_t i=0;i<size;i++)
//		{
			pos_col.x -= pos_col.w + 1;		
			pos_col.y = pos_area.y- tHeight;		
			pos_col.h = tHeight;
			
			pos_clear.x = pos_col.x;
			pos_clear.y = pos_area.y - pos_area.h;
			pos_clear.h =  pos_area.h- tHeight;
			setLevelColor(Mode,tempbuffer[i],&color,&level);
			lcd_drawRectangle(&pos_col,color);
			lcd_drawRectangle(&pos_clear,g_lcd_backColor);
		}
		

			
		
	}
}	

#if COMPRESS_BMP
//to compress bmp with losts of 0xff
//void lcd_bmpCompress(const unsigned char *str,char *dest,uint16_t size,uint16_t *afterSize)
//{
//	 uint16_t length = size;
//	 uint16_t ffSum = 0;
//	uint16_t destCount = 0;
// for (uint16_t i=0;i<length-1;)
//	{
//		 uint8_t  currentByte = *(str+i);
//		
//		if (currentByte ==0xff )
//		{
//			__IO uint8_t  nextByte = *(str+i+1);
//			ffSum++;
//			*(dest + destCount)= 0xff;
//			destCount++;
//			
//			while (nextByte == 0xff)
//			{			
//				ffSum++;
//				nextByte = *(str+i+ffSum);
//			}
//			*(dest +destCount)= ffSum;
//			destCount++;
//			i = i+ffSum;
//			ffSum = 0;
//		}
//		else
//		{
//			i++;
//		*(dest + destCount) = currentByte;
//			destCount++;
//		}
//	
//	}
//	*afterSize = destCount;

//}
//void lcd_Compress(char *buffer,uint16_t *afterSize)
//{

//lcd_bmpCompress(gImage_logo,buffer,1100,afterSize);
//}
#endif
