#ifndef LCD_H__
#define LCD_H__
#include "stdint.h"

#define LCD_LEFT_X   0
#define LCD_TOP_Y   67
#define LCD_CIRCLE_D 240
typedef struct Rect
{
	uint16_t x,y,w,h;
} Rect_t;
typedef struct lcd_unit
{
	uint8_t key;
	uint8_t width;
	uint8_t realWidth;
	uint8_t height;
	uint8_t* data;
	struct lcd_unit * next;
} lcd_unit_t;

typedef struct lcd_block
{
	int16_t type;
	uint16_t groupId;
	struct lcd_unit * first;
	struct lcd_block * next;
}lcd_block_t;
enum DATATYPE_ID
{
	GAS_CH2O = 0,
	GAS_TVOC,
//	GAS_PM1,
	TEMPRATURE,
	HUMIDITY,	
	MAX_GAS_TYPES,
};

void lcd_init(void); 
void lcd_deinit(void);
//void lcd_regester(uint16_t groupId, int16_t type, lcd_unit_t* uint);
void lcd_setOrigin(uint16_t x, uint16_t y);
uint8_t lcd_setRotation(uint16_t degree);
//void lcd_setColor(uint16_t* pPrintColor, uint16_t* pBackColor);
//void lcd_fill(uint16_t width, uint16_t height);
void lcd_erase(uint16_t width, uint16_t height);
//void lcd_setPixelColor(uint16_t x, uint16_t y, uint16_t color, uint16_t alpha);
//void lcd_print(uint16_t groupId, char* str, uint8_t anchor);
//uint16_t lcd_printDirect(uint16_t groupId, char* str, uint8_t anchor);
void lcd_test(uint16_t data);

void lcd_loadData(void);
uint16_t lcd_getRealWidth(lcd_unit_t* lu);
void lcd_drawBigNumber(Rect_t * position,uint16_t color,uint32_t number,uint8_t width,uint8_t type,uint8_t dot_position);
void lcd_drawHistoryNumber(Rect_t * position,uint16_t color,uint32_t number,uint8_t dot_position,uint8_t width,uint8_t isSmall);

void lcd_showCo2(uint32_t co2,uint16_t color);

void lcd_showBattery(uint8_t level,uint8_t is_charging,uint8_t isPoweroff);
void lcd_showBackGround(void);

void lcd_showOutDoor(uint32_t data_pm,uint32_t outDoor_pm25,uint16_t data_battery,uint8_t is_charging,uint8_t needRedraw);
void lcd_showlogo(uint16_t color,uint8_t mode);
void lcd_showSingle(uint32_t data_pm,uint8_t gas_type,uint16_t data_battery,uint8_t is_charging,uint8_t needRedraw);
void lcd_showAll(uint32_t* data_pm,uint16_t data_battery,uint8_t is_charging,uint8_t needRedraw);
void lcd_showQR(char *data,uint8_t needRefresh);
void lcd_showhistory(uint32_t* data,uint16_t size ,uint8_t Mode,uint32_t mainNumber,uint8_t needRedraw);
void lcd_showWifiStatus(uint8_t wifistatus,uint8_t statusChanged,uint32_t smartingTick);
void lcd_showWifiIco(uint8_t status);
void lcd_showTest(uint32_t number,uint16_t color);
void lcd_Compress(char *buffer,uint16_t *afterSize);
static inline uint16_t rgb2c(uint16_t r, uint16_t g, uint16_t b)
{
	return (r>>3<<11)|(g>>2<<5)|(b>>3);
}
#endif
