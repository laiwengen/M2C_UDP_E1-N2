#include "lcd.h"
#include "string.h"
#include "lcd_meshs.h"
#include "frameworks/tick.h"
#include "frameworks/time.h"
#include "frameworks/screen.h"
#include "frameworks/qrcode.h"
#include "frameworks/esp8266.h"
#include "mainconfig.h"
#include "stdlib.h"
//#include "time.h"


static uint16_t g_lcd_rotation = UINT16_MAX;
uint16_t xOffset=LCD_LEFT_X,yOffset=LCD_TOP_Y;

uint8_t g_lcd_data_buffer[LCD_DATA_SIZE] = {0};
uint8_t g_lcd_data_cache[LCD_DATA_SIZE] = {0};

// consts


const int16_t g_lcd_const_titleHeight = 30;
const int16_t g_lcd_const_navigateHeight = 26;
// title
const uint32_t g_lcd_const_batteryBlinkDuration = 1000;
const int16_t g_lcd_const_batterySteps = 6;
const uint32_t g_lcd_const_batteryChargingDuration = 1000;
// main
const int32_t g_lcd_frame_aqi[] = {0, 51, 101, 151, 201, 301, 401};

const uint16_t g_lcd_color_dust[] = {rgb(1,240,1),rgb(241,240,0),rgb(240,104,64),rgb(240,0,1),rgb(160,0,0),rgb(89,72,64)};
const int32_t g_lcd_frame_pm2d5[] = {0, 3600, 7600, 11600, 15100, 25100, 30000};

const uint16_t g_lcd_color_ch2o[] = {rgb(1,240,1),rgb(241,240,0),rgb(240,0,1)};
const int32_t g_lcd_frame_ch2o[] = {0, 101, 301};

//const uint16_t g_lcd_color_co2[] = {rgb(93,218,133),rgb(168,215,74),rgb(251,209,126),rgb(251,152,155),rgb(238,91,95)};
//const int32_t g_lcd_frame_co2[] = {0, 800, 1700, 3000, 5500, 8000};


const uint16_t g_lcd_color_temperature[] = {rgb(142,211,249),rgb(186,227,250),rgb(93,218,133),rgb(251,209,126),rgb(238,91,95)};
const int32_t g_lcd_frame_temperature[] = {4, 18, 28, 37};
const uint16_t g_lcd_color_humidity[] = {rgb(251,209,126),rgb(168,215,74),rgb(93,218,133),rgb(186,227,250)};
const int32_t g_lcd_frame_humidity[] = {20, 40, 65};

//const int32_t g_lcd_frame_so2[] = {51,151,476,801,1601};
//const int32_t g_lcd_frame_no2[] = {41,81,181,281,566};
//const int32_t g_lcd_frame_co[] = {2001,4001,14001,24001,36001};
//const int32_t g_lcd_frame_o3[] = {101,161,216,266,801};

// air
// util functions

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#define abs(a) ((a)<0?(-(a)):(a))

#define lcd_copy(k, n) buffer_set(g_lcd_data_cache,LCD_DATA_##k,(buffer_get(g_lcd_data_buffer,LCD_DATA_##k,n)),n)
#define lcd_check(k, n) (buffer_get(g_lcd_data_cache,LCD_DATA_##k,n) == buffer_get(g_lcd_data_buffer,LCD_DATA_##k,n))
#define lcd_cnc(k, n) (checkAndCopy(g_lcd_data_buffer, g_lcd_data_cache,LCD_DATA_##k,n))

#define lcd_const(name) (g_lcd_const_##name)

#define lcd_mesh(class, name) (&g_lcd_mesh_##class##_##name##_mesh)
#define lcd_meshs(class, name) (g_lcd_mesh_##class##_##name##_meshs)
#define lcd_meshss(class, name) (g_lcd_mesh_##class##_##name##_meshss)

// #define lcd_meshL(class, name) (&g_lcd_mesh_##class##_##name##_mesh)
// #define lcd_meshsL(class, name) (g_lcd_mesh_##class##_##name##_meshs)
// #define lcd_meshssL(class, name) (g_lcd_mesh_##class##_##name##_meshss)

//#define lcd_meshL(class, name) (lcd_get(language) == 0?(&g_lcd_mesh_##class##_##name##_mesh):(&g_lcd_mesh_##class##_##name##_en_mesh))
//#define lcd_meshsL(class, name) (lcd_get(language) == 0?(g_lcd_mesh_##class##_##name##_meshs):(g_lcd_mesh_##class##_##name##_en_meshs))
//#define lcd_meshssL(class, name) (lcd_get(language) == 0?(g_lcd_mesh_##class##_##name##_meshss):(g_lcd_mesh_##class##_##name##_en_meshss))

#define lcd_getC(k) (g_lcd_data_cache[LCD_DATA_##k])

static inline uint8_t checkAndCopy(uint8_t* from, uint8_t* to, uint16_t k, int8_t size){
  if (buffer_get(from,k,size) == buffer_get(to,k,size)) {
    return 1;
  } else {
    buffer_set(to,k,buffer_get(from,k,size),size);
    return 0;
  }
}

enum lcd_theme_colors_t {
  LCD_THEME_COLOR_background,
  LCD_THEME_COLOR_primary,
  LCD_THEME_COLOR_secondary,
  LCD_THEME_COLOR_tertiary,
  LCD_THEME_COLOR_font,
  LCD_THEME_COLOR_cursor,
  LCD_THEME_COLOR_selected,
  LCD_THEME_COLOR_SIZE,
};
const uint16_t g_themeColorBlue[] = {rgb(16,66,156), rgb(33,82,173), rgb(99,140,222), rgb(100,100,100), rgb(255,255,255), rgb(99,140,222), rgb(50,150,50)};
const uint16_t g_themeColorBlack[] = {rgb(0,0,0), rgb(12,12,12), rgb(50,50,50), rgb(100,100,100), rgb(255,255,255), rgb(44,44,44), rgb(0,50,0)};
const uint16_t g_themeColorWhite[] = {rgb(201,201,201), rgb(238,238,238), rgb(201,201,201), rgb(100,100,100), rgb(32,32,32), rgb(201,201,201), rgb(160,160,160)};
const uint16_t g_themeColorPink[] = {rgb(234,105,162), rgb(225,85,146), rgb(208,48,120), rgb(100,100,100), rgb(255,255,255), rgb(184,55,112), rgb(235,47,131)};
const uint16_t g_themeColorOrange[] = {rgb(238,124,36), rgb(231,113,23), rgb(201,90,0), rgb(201,90,0), rgb(255,255,255), rgb(199,87,3), rgb(200,104,30)};
const uint16_t g_themeColorPurple[] = {rgb(96,1,255), rgb(82,1,217), rgb(68,0,181), rgb(82,1,217), rgb(255,255,255), rgb(68,0,183), rgb(127,49,255)};
static const uint16_t * const g_themeColors[] = {g_themeColorBlue, g_themeColorBlack, g_themeColorWhite,g_themeColorPink, g_themeColorOrange, g_themeColorPurple};
static inline const uint16_t* getThemeColors(void) {
  uint8_t theme = lcd_get(theme);
  if (theme >= sizeof(g_themeColors)/sizeof(void*)) {
    theme = 0;
  }
  return g_themeColors[theme];
}
#if 0
enum lcd_time_parse_t {
  LCD_TIME_PARSE_second,
  LCD_TIME_PARSE_minute,
  LCD_TIME_PARSE_hour,
  LCD_TIME_PARSE_day,
  LCD_TIME_PARSE_month,
  LCD_TIME_PARSE_year,
  LCD_TIME_PARSE_weakday,
};
#endif

void lcd_testNumber(int16_t number, int8_t position);
/// block for part Rendering

enum lcd_render_title_bits_t {
//  LCD_RENDER_TITLE_BIT_logo,
//  LCD_RENDER_TITLE_BIT_hold,
//  LCD_RENDER_TITLE_BIT_time,
  LCD_RENDER_TITLE_BIT_wifi,
  LCD_RENDER_TITLE_BIT_batteryStatic,
	LCD_RENDER_TITLE_BIT_battery,
  LCD_RENDER_TITLE_BIT_SIZE,
};

static void renderTitle(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
//  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
//  uint16_t backColor = themeColors[LCD_THEME_COLOR_primary];
	
	uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];

  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
	
  screen_rect_t rect = {0};
  uint32_t bit = 1;
  
  const screen_mesh_t* meshs[10] = {0};
  screen_sequence_t sequence = {
    &rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor
  };
//  rect.x = 147 + xOffset;
//	rect.y = 32 + yOffset;
//	rect.h = 25;
//  rect.w = 13;
	#if 0
  // logo
	
  rect.w = 108;
  if (updateBits & bit) {
    meshs[0] = lcd_mesh(title, logo);
    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = 1;
    int8_t xoffsets[10] = {0};
    sequence.xoffsets = xoffsets;
    xoffsets[0] = 8;
    screen_drawSequence(&sequence);
    sequence.xoffsets = 0;
  }
  bit<<=1;
  rect.x += rect.w;
  // hold
  rect.w = 10;
  if (updateBits & bit) {
    meshs[0] = lcd_get(holding)?lcd_mesh(title, hold):lcd_mesh(title, run);
    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = lcd_get(holding)?1:0;
    // sequence.size = 1;
    screen_drawSequence(&sequence);
  }

  bit<<=1;
  rect.x += rect.w;
  // time
  rect.w = 45;

  if (updateBits & bit) {
    uint32_t timestamp = lcd_get32(time);
    time_t time;
    time_fromSecond(timestamp, &time);
    screen_dec_t dec = {time.hour, lcd_meshs(number,5), 0, 2, 2};
    screen_fetchDec(&dec,meshs);
    dec.data = time.minute;
    screen_fetchDec(&dec,meshs+3);
    meshs[2] = lcd_mesh(title, timeConnector);
    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = 5;

    int8_t xoffsets[10] = {0};
    fors (10) {
      xoffsets[i] = 2;
    }
    sequence.xoffsets = xoffsets;
    screen_drawSequence(&sequence);
    sequence.xoffsets = 0;
  }

  bit<<=1;
  rect.x += rect.w;

  // gprs
  rect.w = 40;
  if (updateBits & bit) { 
    screen_mesh_t dummyMesh = {SCREEN_MESH_NULL,lcd_mesh(title, uploading)->w,lcd_mesh(title, downloading)->h };
    int8_t xoffsets[10] = {0};
    if (lcd_get(pin)) {
      meshs[0] = lcd_get(uploading)?lcd_mesh(title, uploading):&dummyMesh;
      meshs[1] = lcd_get(downloading)?lcd_mesh(title, downloading):&dummyMesh;
      meshs[2] = lcd_meshs(title, gprs)[lcd_get(rssi)];
      sequence.size = 3;
      xoffsets[0] = 0;
      xoffsets[1] = 0;
      xoffsets[2] = 2;
    } else {
      meshs[0] = lcd_mesh(title,card);
      meshs[1] = lcd_mesh(title,sim);
      sequence.size = 2;
      xoffsets[0] = 0;
      xoffsets[1] = 1;
    }
    sequence.xoffsets = xoffsets;
    sequence.align = SCREEN_ALIGN_CENTER;
    screen_drawSequence(&sequence);
    sequence.xoffsets = 0;
  }
	#endif
//  bit<<=1;
//  rect.x += rect.w;
//	if (updateBits & (1<<1))
//	{
//		rect.x = 147 + xOffset;
//		rect.y = 32 + yOffset;
//		rect.h = 13;
//		rect.w = 25;
//		meshs[0] = lcd_meshs(title, battery)[0];
//			
//		sequence.align = SCREEN_ALIGN_LEFT;
//		sequence.size = 1;
//		sequence.xoffsets = 0;
//		screen_drawSequence(&sequence);	
		
//			esp8266_status_t wifiStatus = (esp8266_status_t)lcd_get(wifiStatus);
//		if (wifiStatus == ESP8266_STATUS_smarting)
//		{
//			meshs[0] = lcd_mesh(settings, wifiStatus1Label);
//		}
//		else if (wifiStatus == ESP8266_STATUS_connectWifi || wifiStatus == ESP8266_STATUS_getIP)
//		{
//			meshs[0] = lcd_mesh(settings, wifiStatus2Label);
//		}
//		else if (wifiStatus == ESP8266_STATUS_ok)
//		{
//			meshs[0] = lcd_mesh(settings, wifiStatus3Label);
//		}
//		else if (wifiStatus <= ESP8266_STATUS_initOK)
//		{
//			meshs[0] = lcd_mesh(settings, wifiStatus4Label);
//		}
//		else 
//		{
//			screen_mesh_t dummyMesh = {SCREEN_MESH_FILL, 104, 12};
//			meshs[0] = &dummyMesh;
//			sequence.frontColor = backColor;
//		}
//	}	

//wifi
//LCD_RENDER_TITLE_BIT_wifi
	if (updateBits & (1<<LCD_RENDER_TITLE_BIT_wifi)) {
		rect.x = 74 + xOffset;
		rect.y = 30 + yOffset;
		rect.w = 21;		
		rect.h = 18;

		meshs[0] = lcd_mesh(title, wifi);
			
		sequence.align = SCREEN_ALIGN_LEFT;
		sequence.size = 1;
		sequence.xoffsets = 0;
		screen_drawSequence(&sequence);
			
		uint8_t wifiStatus = esp8266_getStatus();
		if (wifiStatus != ESP8266_STATUS_ok)
		{
			meshs[0] = lcd_mesh(title, noWifi);
			rect.x = 91 + xOffset;
			rect.y = 39 + yOffset;
			rect.h = 8;
			rect.w = 9;

				
			sequence.align = SCREEN_ALIGN_LEFT;
			sequence.size = 1;
			sequence.xoffsets = 0;
			screen_drawSequence(&sequence);		
		}
	}
	
  // battery
//	uint8_t isBlend = lcd_get(batteryBlend);
	rect.x = 147 + xOffset;
	rect.y = 32 + yOffset;
	rect.h = 13;
  rect.w = 25;
	if (updateBits & (1<<LCD_RENDER_TITLE_BIT_batteryStatic)) {


		
//	screen_mesh_t clearBatteryMesh = {SCREEN_MESH_NULL,lcd_mesh(title, battery0)->w,lcd_mesh(title, battery0)->h };		
//	meshs[0] = isBlend?&clearBatteryMesh:lcd_meshs(title, battery)[0];
	meshs[0] = lcd_meshs(title, battery)[0];
	sequence.align = SCREEN_ALIGN_LEFT;
	sequence.size = 1;
	sequence.xoffsets = 0;
	screen_drawSequence(&sequence);
		
	}
	uint8_t index = lcd_get(batteryIndex);
  if (updateBits & (1<<LCD_RENDER_TITLE_BIT_battery)) {
//		int8_t xoffsets[10] = {0};
//		int8_t yoffsets[10] = {0};
//		xoffsets[0] = 1;
//		yoffsets[0] = 1;
//		sequence.xoffsets = xoffsets;
//		sequence.yoffsets = yoffsets;
		//LCD_DATA_batteryCharging
//meshs[0] = lcd_get(batteryBlend)?&batteryMesh:lcd_meshs(title, battery)[index];
//	if (!isBlend) {
		rect.x +=2;
		rect.y +=2;
		rect.h = 9;
		rect.w = 19;
		
		uint8_t batteryWidth = (index * (rect.w) / 100) + 1;
		batteryWidth = min(19,batteryWidth);
		uint8_t isCharging = lcd_get(batteryCharging);
		if (isCharging)
		{
			sequence.frontColor = rgb(0,240,1);
		}
		else
		{
			if (index < 20)
			{
				sequence.frontColor = rgb(250,0,0);
			}
			else 
			{
				sequence.frontColor = rgb(96,96,96);
			}
		}
		screen_mesh_t dummyMesh = {SCREEN_MESH_FILL,batteryWidth,rect.h };
		
		meshs[0] = &dummyMesh;
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;		
//		sequence.frontColor = rgb(0,240,1);
		screen_drawSequence(&sequence);
	}
  // battery
//  rect.w = 40;
//  if (updateBits & bit) {
//    screen_mesh_t dummyMesh = {SCREEN_MESH_NULL,lcd_mesh(title, charging)->w,lcd_mesh(title, charging)->h };
//    screen_mesh_t batteryMesh = {SCREEN_MESH_NULL,lcd_mesh(title, battery0)->w,lcd_mesh(title, battery0)->h };
//    uint8_t index = lcd_get(batteryIndex);
//    meshs[0] = lcd_get(batteryBlend)?&batteryMesh:lcd_meshs(title, battery)[index];
//    meshs[1] = lcd_get(batteryCharging)?lcd_mesh(title, charging):&dummyMesh;
//    sequence.align = SCREEN_ALIGN_LEFT;
//    sequence.size = 2;
//    int8_t xoffsets[10] = {0};
//    sequence.xoffsets = xoffsets;
//    xoffsets[0] = 0;
//    xoffsets[1] = 2;
//    screen_drawSequence(&sequence);
//    sequence.xoffsets = 0;
//  }
//  bit<<=1;
//  rect.x += rect.w;
//  


  //
}

//static void renderNavigate(const screen_mesh_t ** meshs, uint16_t frontColor, uint16_t primaryColor) {
//  screen_rect_t rect = {0};
//  screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, 0,0,0,1,frontColor,primaryColor};
//  rect.x = 0;
//  rect.y = 320-lcd_const(navigateHeight);
//  rect.w = 79;
//  rect.h = lcd_const(navigateHeight);
//  sequence.align = SCREEN_ALIGN_CENTER;
//  sequence.meshs = meshs;
//  sequence.size = (*sequence.meshs)?1:0;
//  screen_drawSequence(&sequence);
//  rect.x += rect.w + 1;
//  sequence.meshs = meshs + 1;
//  sequence.size = (*sequence.meshs)?1:0;
//  screen_drawSequence(&sequence);
//  rect.x += rect.w + 1;
//  rect.w = 80;
//  sequence.meshs = meshs + 2;
//  sequence.size = (*sequence.meshs)?1:0;
//  screen_drawSequence(&sequence);
//}

//static int16_t calculateFramePercent(const int32_t* frames, int8_t size, int32_t value) {
//  if (size < 2) {
//    return 500;
//  }
//  for (int8_t i = 0; i < size; i++) {
//    if (value < frames[i]) {
//      if (i == 0) {
//        return 0;
//      } else {
//        int32_t base = frames[i] - frames[i-1];
//        int32_t over = value - frames[i-1];
//        return ((i - 1) * base + over) * 1000/(base * (size - 1));
//      }
//    }
//  }
//  return 1000;
//}
static int8_t calculateFrameLevel(const int32_t* frames, int8_t size, int32_t value) {
  for (int8_t i = 0; i < size; i++) {
    if (value < frames[i]) {
      return i;
    }
  }
  return size;
}

enum lcd_render_main_bits_t {
//  LCD_RENDER_MAIN_BIT_pm2d5Static,
//  LCD_RENDER_MAIN_BIT_pm2d5Data,
  LCD_RENDER_MAIN_BIT_ch2oStatic,
  LCD_RENDER_MAIN_BIT_ch2oPre,
  LCD_RENDER_MAIN_BIT_ch2oData,
  LCD_RENDER_MAIN_BIT_minorStatic,
  LCD_RENDER_MAIN_BIT_minorData,
  LCD_RENDER_MAIN_BIT_hold,
  LCD_RENDER_MAIN_BIT_SIZE,
};
static void renderPm2d5(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];

  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  screen_rect_t rect = {0};
  const screen_mesh_t* meshs[10] = {0};
  screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
  // pm2d5Static
  if (1) {
		
    rect.x = 45+xOffset;
    rect.y = 90+yOffset;
    rect.w = 150;
    rect.h = 60;

    screen_dec_t dec = {lcd_get32(valuePm2d5), lcd_meshs(number,1), 2, 3, 3};
    uint8_t size = screen_fetchDec(&dec,meshs);

    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = size;
    int8_t xoffsets[10] = {0};
    fors (10) {
      xoffsets[i] = 2;
    }
    xoffsets[0] = 0;
    sequence.xoffsets = xoffsets;
    screen_drawSequence(&sequence); //pm2d5 number
		
		frontColor = themeColors[LCD_THEME_COLOR_font];

		backColor = themeColors[LCD_THEME_COLOR_background];
		
    rect.x = 22 + xOffset;
    rect.y = 125 + yOffset;
    rect.w = 9;
    rect.h = 10;
    meshs[0] = lcd_mesh(pm2d5, arrowLeft);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
		sequence.frontColor = backColor;
		sequence.backColor = frontColor;
    screen_drawSequence(&sequence);//draw arrowL
		
		
    rect.x = 218 + xOffset;
    rect.y = 125 + yOffset;
    rect.w = 9;
    rect.h = 10;
    meshs[0] = lcd_mesh(pm2d5, arrowRight);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw arrowR
		
		
    rect.x = 70 + xOffset;
    rect.y = 69 + yOffset;
    rect.w = 46;
    rect.h = 9;
    meshs[0] = lcd_mesh(main2, pm2d5Title);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw pm2.5 icon
		
		
    rect.x = 127 + xOffset;
    rect.y = 67 + yOffset;
    rect.w = 36;
    rect.h = 12;
    meshs[0] = lcd_mesh(main2, ugm3);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw ug/m3
		
		
		
    rect.x = 30 + xOffset;
    rect.y = 165 + yOffset;
    rect.w = 49;
    rect.h = 12;
    meshs[0] = lcd_mesh(pm2d5, pm1d0Title);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw pm1.0 icon
		
		
    rect.x = 140 + xOffset;
    rect.y = 165 + yOffset;
    rect.w = 44;
    rect.h = 12;
    meshs[0] = lcd_mesh(pm2d5, pm10Title);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw pm10 icon
		
		
    sequence.xoffsets = 0;
		int8_t level = calculateFrameLevel(g_lcd_frame_pm2d5 + 1, 3, lcd_get32(valuePm2d5));		
		rect.x = 0 + xOffset;
		rect.y = 202 + yOffset;
		rect.w = 240;
		rect.h = 38;
		
		screen_mesh_t dummyMesh = {SCREEN_MESH_FILL, 240, 38};
		meshs[0] = &dummyMesh;
		sequence.align = SCREEN_ALIGN_LEFT;
		sequence.size = 1;
		sequence.frontColor = g_lcd_color_dust[level];
		screen_drawSequence(&sequence);//level down
		
	}
	
}


static void renderMain2(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];

  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  screen_rect_t rect = {0};
  const screen_mesh_t* meshs[10] = {0};
  screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
  // pm2d5Static
  if (1) {
		
    rect.x = 45+xOffset;
    rect.y = 90+yOffset;
    rect.w = 90;
    rect.h = 60;

    screen_dec_t dec = {lcd_get32(valuePm2d5), lcd_meshs(number,2), 2, 3, 3};
    uint8_t size = screen_fetchDec(&dec,meshs);

    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = size;
    int8_t xoffsets[10] = {0};
    fors (10) {
      xoffsets[i] = 2;
    }
    xoffsets[0] = 0;
    sequence.xoffsets = xoffsets;
    screen_drawSequence(&sequence); //pm2d5 number
		
		
    rect.x = 62 + xOffset+8;
    rect.y = 65 + yOffset;
    rect.w = 16;
    rect.h = 12;

    screen_dec_t tempNumber = {lcd_get16(valueTemperature), lcd_meshs(number,3), 0, 0, 2};
    size = screen_fetchDec(&tempNumber,meshs);

    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = size;
    screen_drawSequence(&sequence); //temp number
		
		
    rect.x = 146 + xOffset+8;
    rect.y = 65 + yOffset;
    rect.w = 16;
    rect.h = 12;

    screen_dec_t humNumber = {lcd_get16(valueHumidity), lcd_meshs(number,3), 0, 0, 2};
    size = screen_fetchDec(&humNumber,meshs);

    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = size;
    screen_drawSequence(&sequence); //hum number
		
		
    rect.x = 87 + xOffset+12;
    rect.y = 151 + yOffset;
    rect.w = 32;
    rect.h = 12;

    screen_dec_t ch2oNumber = {lcd_get16( valueCh2o ), lcd_meshs(number,3), 3, 4, 4};
    size = screen_fetchDec(&ch2oNumber,meshs);

    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = size;
    screen_drawSequence(&sequence); //ch2o number
		
		
    rect.y += 25;
    rect.w = 32;
    rect.h = 12;

    screen_dec_t tvocNumber = {lcd_get16(valueCh2o), lcd_meshs(number,3), 3, 4, 4};
    size = screen_fetchDec(&tvocNumber,meshs);

    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = size;
    screen_drawSequence(&sequence); //tvoc number
		
		
    rect.y += 25;
    rect.w = 32;
    rect.h = 12;

    screen_dec_t co2Number = {lcd_get16(valueCo2), lcd_meshs(number,3), 0, 4, 4};
    size = screen_fetchDec(&co2Number,meshs);

    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = size;
    screen_drawSequence(&sequence); //co2 number
		
		
    rect.x = 59 + xOffset;
    rect.y = 62 + yOffset;
//    rect.w = 46;
//    rect.h = 9;
//    meshs[0] = lcd_mesh(main2, pm2d5Title);
//    sequence.align = SCREEN_ALIGN_LEFT;
//    sequence.size = 1;
//    screen_drawSequence(&sequence);//draw pm25
		
    rect.x = 59 + xOffset+90;
    rect.y = 82 + yOffset+30;
    rect.w = 36;
    rect.h = 12;
    meshs[0] = lcd_mesh(main2, ugm3);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
		sequence.frontColor = backColor;
		sequence.backColor = frontColor;
    screen_drawSequence(&sequence);//draw ugm3
		
    rect.x = 58 + xOffset;
    rect.y = 65 + yOffset;
    rect.w = 4;
    rect.h = 13;
    meshs[0] = lcd_mesh(main2, degree);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw temp
		
		
		
    rect.x += 40;
    rect.w = 12;
    rect.h = 13;
    meshs[0] = lcd_mesh(main2, temp);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw degree
		
		
		
    rect.x = 131 + xOffset;
    rect.y = 65 + yOffset;
    rect.w = 15;
    rect.h = 13;
    meshs[0] = lcd_mesh(main2, rh);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw rh
		
		
    rect.x += 50;
    rect.w = 11;
    rect.h = 13;
    meshs[0] = lcd_mesh(main2, percent);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw percent
		
		
		
		
		
    rect.x = 55 + xOffset;
    rect.y = 151+yOffset;
    rect.w = 32;
    rect.h = 12;
    meshs[0] = lcd_mesh(main2, ch2oIcon);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw ch2o
		
    rect.y += 25;
    rect.w = 32;
    rect.h = 12;
    meshs[0] = lcd_mesh(main2, ch2oIcon);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw tvoc
		
    rect.y += 25;
    rect.w = 24;
    rect.h = 12;
    meshs[0] = lcd_mesh(main2, co2Icon);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw co2
		
    rect.x = 145+xOffset;
    rect.y = 151+yOffset;
    rect.w = 36;
    rect.h = 12;
    meshs[0] = lcd_mesh(main2, mgm3);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw mgm3
		
    rect.y += 25;
    rect.w = 36;
    rect.h = 12;
    meshs[0] = lcd_mesh(main2, mgm3);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw mgm3
		
    rect.y += 25;
    rect.w = 36;
    rect.h = 12;
    meshs[0] = lcd_mesh(main2, mgm3);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw mgm3
		
	}
	
}

static void renderMain(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];

  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  screen_rect_t rect = {0};
  const screen_mesh_t* meshs[10] = {0};
  screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
  // pm2d5Static
  if (1) {
    rect.x = 59 + xOffset;
    rect.y = 62 + yOffset;
    rect.w = 42;
    rect.h = 14;
    meshs[0] = lcd_mesh(main, ch2oLabel);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);//draw hcho

//    rect.x = 30;
//    rect.y = 98+lcd_const(titleHeight);
//    rect.w = 180;
//    rect.h = 9;
//    screen_colorbar_t bar = {&rect,SCREEN_ALIGN_TOP,0,0,180,9,g_lcd_color_dust,6,backColor};
//    screen_drawColorbar(&bar);
  }

  // pm2d5Data

  if (updateBits & (1<<LCD_RENDER_MAIN_BIT_ch2oData)) {
    rect.x = 44+xOffset;
    rect.y = 89+yOffset;
    rect.w = 150;
    rect.h = 60;

    screen_dec_t dec = {lcd_get16(valueCh2o), lcd_meshs(number,1), 3, 3, 4};
    uint8_t size = screen_fetchDec(&dec,meshs);

    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = size;
    int8_t xoffsets[10] = {0};
    fors (10) {
      xoffsets[i] = 2;
    }
    xoffsets[0] = 0;
    sequence.xoffsets = xoffsets;
    screen_drawSequence(&sequence); //big number
		
    sequence.xoffsets = 0;
		int8_t level = calculateFrameLevel(g_lcd_frame_ch2o + 1, 3, lcd_get16(valueCh2o));		
		rect.x = 0 + xOffset;
		rect.y = 202 + yOffset;
		rect.w = 240;
		rect.h = 38;
		
		screen_mesh_t dummyMesh = {SCREEN_MESH_FILL, 240, 38};
		meshs[0] = &dummyMesh;
		sequence.align = SCREEN_ALIGN_LEFT;
		sequence.size = 1;
		sequence.frontColor = g_lcd_color_ch2o[level];
		screen_drawSequence(&sequence);//level down
			
		rect.x = 133 + xOffset;
    rect.y = 61 + yOffset;
    rect.w = 60;
    rect.h = 17;
    meshs[0] = lcd_meshs(icon, ch2o)[level];
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
		sequence.frontColor = rgb(64,64,64);
    screen_drawSequence(&sequence);//level word
    
//    int16_t percent = calculateFramePercent(g_lcd_frame_pm2d5, 7, lcd_get32(valuePm2d5));
//    int8_t level = calculateFrameLevel(g_lcd_frame_pm2d5 + 1, 5, lcd_get32(valuePm2d5));

//    rect.x = 19;
//    rect.y = 90+lcd_const(titleHeight);
//    rect.w = 200;
//    rect.h = 8;
//    screen_slider_t slider = {&rect,SCREEN_ALIGN_BOTTOM,0,0,180,5,percent,lcd_mesh(main,slider),frontColor,backColor};
//    screen_drawSlider(&slider);

//    rect.x = 163;
//    rect.y = 12+lcd_const(titleHeight);
//    rect.w = 60;
//    rect.h = 17;
//    meshs[0] = lcd_meshsL(icon, pm2d5)[level];
//    sequence.align = SCREEN_ALIGN_LEFT;
//    sequence.size = 1;
//    sequence.frontColor = g_lcd_color_dust[level];
//    screen_drawSequence(&sequence);
//    sequence.frontColor = frontColor;
  }
	#if 0
  // co2Static
  if (updateBits & (1<<LCD_RENDER_MAIN_BIT_co2Static)) {
    rect.x = 24;
    rect.y = 120+lcd_const(titleHeight);
    rect.w = 134;
    rect.h = 17;
    meshs[0] = lcd_mesh(main, co2Label);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);

  }

  // co2Pre
  if (updateBits & (1<<LCD_RENDER_MAIN_BIT_co2Pre)) {
    if (!lcd_get(co2Preparing)) {
      rect.x = 30;
      rect.y = 108+98+lcd_const(titleHeight);
      rect.w = 180;
      rect.h = 9;
      screen_colorbar_t bar = {&rect,SCREEN_ALIGN_TOP,0,0,180,9,g_lcd_color_co2,5,backColor};
      screen_drawColorbar(&bar);
    }
  }

  // co2Data

  if (updateBits & (1<<LCD_RENDER_MAIN_BIT_co2Data)) {
    //number
    rect.x = 30;
    rect.y = 108+28+lcd_const(titleHeight);
    rect.w = 180;
    rect.h = 55;
    int32_t value;
    if (!lcd_get(co2Preparing)) {
      value = lcd_get16(valueCo2);
      value = min(value, 9999);
    } else {
      value = lcd_get(co2PreTime);
    }
    screen_dec_t dec = {value, lcd_meshs(number,1), 0, 2, 4};
    uint8_t size = screen_fetchDec(&dec,meshs);
    int8_t xoffsets[10] = {0};
    fors (10) {
      xoffsets[i] = 2;
    }
    sequence.xoffsets = xoffsets;
    if (!lcd_get(co2Preparing)) {
      xoffsets[0] = 0;
    } else {
      const screen_mesh_t* mesh = lcd_meshL(main,miao);
      meshs[size] = mesh;
      xoffsets[0] = -(mesh->w>>1) - 5;
      xoffsets[size] = 5;
      size++;
    }
    sequence.align = SCREEN_ALIGN_BOTTOM;
    sequence.size = size;
    screen_drawSequence(&sequence);
    sequence.xoffsets = 0;

    if (!lcd_get(co2Preparing)) {
      // slider
      int16_t percent = calculateFramePercent(g_lcd_frame_co2, 6, value);
      int8_t level = calculateFrameLevel(g_lcd_frame_co2 + 1, 4, value);
      rect.x = 19;
      rect.y = 108+90+lcd_const(titleHeight);
      rect.w = 200;
      rect.h = 8;
      screen_slider_t slider = {&rect,SCREEN_ALIGN_BOTTOM,0,0,180,5,percent,lcd_mesh(main,slider),frontColor,backColor};
      screen_drawSlider(&slider);

      // icon
      rect.x = 123;
      rect.y = 108+12+lcd_const(titleHeight);
      rect.w = 100;
      rect.h = 17;
      meshs[0] = lcd_meshsL(icon, co2)[level];
      sequence.align = SCREEN_ALIGN_RIGHT;
      sequence.size = 1;
      sequence.frontColor = g_lcd_color_co2[level];
      screen_drawSequence(&sequence);
      sequence.frontColor = frontColor;
    } else {
      // statubar
      int16_t percent;
      {
        percent = 1000 - lcd_get(co2PreTime)*1000/30;
      }
      rect.x = 19;
      rect.y = 108+90+lcd_const(titleHeight);
      rect.w = 200;
      rect.h = 17;
      screen_statubar_t bar = {&rect,SCREEN_ALIGN_BOTTOM,0,0,180,9,percent,3,primaryColor,rgb(0,0xff,0),backColor,backColor};
      screen_drawStatubar(&bar);

      // prepairing
      rect.x = 123;
      rect.y = 108+12+lcd_const(titleHeight);
      rect.w = 100;
      rect.h = 17;
      meshs[0] = lcd_meshL(main, prewarm);
      sequence.align = SCREEN_ALIGN_RIGHT;
      sequence.size = 1;
      screen_drawSequence(&sequence);
    }
  }
	// minorStatic
  if (updateBits & (1<<LCD_RENDER_MAIN_BIT_minorStatic)) {
    const screen_mesh_t* labelMeshs[] = {
//      lcd_mesh(main,pm10Label),
//      lcd_mesh(main,pm1d0Label),
      lcd_meshL(main,temperatureLabel),
      lcd_meshL(main,humidityLabel)
    };
    for (uint8_t i = 0; i < 2; i++) {
      rect.x = 84 + i*82+ xOffset;
      rect.y = 173+ yOffset;
      rect.w = 56;
      rect.h = 24;
      meshs[0] = labelMeshs[i];
      sequence.align = SCREEN_ALIGN_CENTER;
      sequence.size = 1;
      screen_drawSequence(&sequence); //level word
    }
  }

	#endif 

  // minorData
  if (updateBits & (1<<LCD_RENDER_MAIN_BIT_minorData)) {
    int32_t values[] = {lcd_get16(valueTemperature)/100, lcd_get16(valueHumidity)/100};

		
    const screen_mesh_t* units[] = {lcd_mesh(main,temperatureUnit),lcd_mesh(main,humidityUnit)};

    for (uint8_t i = 0; i < 2; i++) {
      rect.x = 49 + i*82 + xOffset;
      rect.y = 163+yOffset;
      rect.w = 60;
      rect.h = 28;

      int32_t value = values[i];
      screen_dec_t dec = {value, lcd_meshs(number,2), 0, 1, 4};
      uint8_t size = screen_fetchDec(&dec,meshs);
      if (units[i]) {
        meshs[size] = units[i];
        size ++ ;
      }

      sequence.align = SCREEN_ALIGN_BOTTOM;
      sequence.size = size;
      int8_t xoffsets[10] = {0};
      fors (10) {
        xoffsets[i] = 3;
      }
			xoffsets[size -1] = 8;
      xoffsets[0] = 0;
      sequence.xoffsets = xoffsets;
      screen_drawSequence(&sequence);//temp and hum
      sequence.xoffsets = 0;

			
//      rect.y += rect.h;
//      rect.h = 8;
//      screen_mesh_t dummyMesh = {SCREEN_MESH_FILL, 30, 4};
//      meshs[0] = &dummyMesh;
//      sequence.align = SCREEN_ALIGN_CENTER;
//      sequence.size = 1;

//      screen_drawSequence(&sequence);

    }
    //int16_t percent = calculateFramePercent(g_lcd_frame_pm2d5, 7, lcd_get32(valuePm2d5));

  }
	
}
// air
#if 0
enum lcd_render_air_bits_t {
  LCD_RENDER_AIR_BIT_static,
  LCD_RENDER_AIR_BIT_data,
  LCD_RENDER_AIR_BIT_SIZE,
};
static void renderAir(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];
  screen_rect_t rect = {0};
  const screen_mesh_t* meshs[10] = {0};
  screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
  // static
  if (updateBits & (1<<LCD_RENDER_AIR_BIT_static)) {
    // aqi label
    rect.x = 20;
    rect.y = 5+lcd_const(titleHeight);
    rect.w = 84;
    rect.h = 24;
    meshs[0] = lcd_meshL(air, aqiLabel);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);

    // color bar
    rect.x = 20;
    rect.y = 35+lcd_const(titleHeight);
    rect.w = 96;
    rect.h = 2;
    screen_colorbar_t bar = {&rect,SCREEN_ALIGN_TOP,0,0,96,2,g_lcd_color_dust,6,backColor};
    screen_drawColorbar(&bar);

    //detail
    rect.x = 20;
    rect.y = 49+lcd_const(titleHeight);
    rect.w = 50;
    rect.h = 20;
    meshs[0] = lcd_meshL(air, detailLabel);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);

    //grid
    rect.x = 20;
    rect.y = 72+lcd_const(titleHeight);
    rect.w = 200;
    rect.h = 181;
    int16_t rowSpaces[] = {29,29,29,29,29,29};
    screen_grid_t grid = {&rect, SCREEN_ALIGN_CENTER,0,0,200,181,rowSpaces,6,0,1,1,1,secondaryColor,secondaryColor, backColor};
    screen_drawGrid(&grid);

    //navigate
    const screen_mesh_t* navigateMeshs[] = {lcd_meshL(navigate,preview),lcd_meshL(navigate,weather),lcd_meshL(navigate,nextview)};
    renderNavigate(navigateMeshs, frontColor, primaryColor);
  }

  if (updateBits & (1<<LCD_RENDER_AIR_BIT_data)) {
    uint32_t airUpdateTime = lcd_get32(airUpdateTime);
    const screen_rect_t numberRect = {152,8+lcd_const(titleHeight),70,32};
    const screen_rect_t sliderRect = {20,30+lcd_const(titleHeight),96,5};
    const screen_rect_t iconRect = {101,10+lcd_const(titleHeight),50,15};
    const screen_rect_t updateTimeRect = {80,53+lcd_const(titleHeight),140,13};
    if (airUpdateTime != 0 && airUpdateTime != UINT32_MAX){
      // number
      {
        sequence.rect = &numberRect;
        uint8_t size;
        if (lcd_get16(valueAirAqi) >= 0) {
          screen_dec_t dec = {lcd_get16(valueAirAqi), lcd_meshs(number,2), 0, 1, 3};
          size = screen_fetchDec(&dec,meshs);
        } else {
          fors(3) {
            meshs[i] = lcd_meshs(number,2)[11];
          }
          size = 3;
        }
        sequence.align = SCREEN_ALIGN_RIGHT;
        sequence.size = size;
        int8_t xoffsets[10] = {0};
        for (int8_t i = 0; i < sequence.size; i++) {
          xoffsets[i] = 2;
        }
        xoffsets[0] = 0;
        sequence.xoffsets = xoffsets;
        screen_drawSequence(&sequence);
        sequence.xoffsets = 0;
      }
      // slider
      {
        int16_t percent = calculateFramePercent(g_lcd_frame_aqi, 7, lcd_get16(valueAirAqi));
        percent = min(percent,1000);
        percent = max(percent,0);
        screen_slider_t slider = {&sliderRect,SCREEN_ALIGN_BOTTOM,0,0,96,5,percent,lcd_mesh(air,slider),frontColor,backColor};
        screen_drawSlider(&slider);
      }
      // icon
      {
        sequence.rect = &iconRect;

        if (lcd_get16(valueAirAqi) >= 0) {
          uint16_t level = calculateFrameLevel(g_lcd_frame_aqi + 1, 5, lcd_get16(valueAirAqi)); //(lcd_get16(valueAirAqi)+1)/50;
          level = min(level,5);
          level = max(level,0);
          meshs[0] = lcd_meshsL(icon, air)[level];
          sequence.size = 1;
					sequence.frontColor = g_lcd_color_dust[level];
        } else {
          sequence.size = 0;
        }
        sequence.align = SCREEN_ALIGN_CENTER;
        screen_drawSequence(&sequence);
        sequence.frontColor = frontColor;
      }
      // update time
      {
        sequence.rect = &updateTimeRect;
        time_t time;
        time_fromSecond(airUpdateTime, &time);
        const screen_mesh_t* updateTimeMeshs[15] = {0};
        int8_t xoffsets[15] = {0};
        for (int8_t i = 0; i < 15; i++) {
          xoffsets[i] = 2;
        }
        int8_t yoffsets[15] = {0};
        for (int8_t i = 0; i < 15; i++) {
          yoffsets[i] = 0;
        }
        uint8_t size = 0;
        screen_dec_t dec = {0, lcd_meshs(number,6), 0, 1, 4};
        if (lcd_get(language) == 0) {
          dec.data = time.month + 1;
          size += screen_fetchDec(&dec,updateTimeMeshs + size);
          updateTimeMeshs[size++] = lcd_mesh(air,calendarMonth);
          dec.data = time.day + 1;
          size += screen_fetchDec(&dec,updateTimeMeshs + size);
          updateTimeMeshs[size++] = lcd_mesh(air,calendarDay);
          xoffsets[size] = 6;
        } else {
          updateTimeMeshs[size++] = lcd_meshs(date,month_en)[time.month];
          updateTimeMeshs[size++] = lcd_meshs(date,day)[time.day];
          xoffsets[size] = 6;
        }
        dec.minSize = 2;
        dec.data = time.hour;
        size += screen_fetchDec(&dec,updateTimeMeshs + size);
        updateTimeMeshs[size++] = lcd_mesh(air,calendarHour);
        dec.data = time.minute;
        size += screen_fetchDec(&dec,updateTimeMeshs + size);
        if (lcd_get(language) != 0) {
          xoffsets[size] = 2;
          yoffsets[size] = 1;
        }
        updateTimeMeshs[size++] = lcd_meshL(air,calendarMinute);
        sequence.align = SCREEN_ALIGN_RIGHT;
        sequence.size = size;
        sequence.meshs = updateTimeMeshs;
        sequence.xoffsets = xoffsets;
        sequence.yoffsets = yoffsets;
        screen_drawSequence(&sequence);
        sequence.meshs = meshs;
        sequence.xoffsets = 0;
        sequence.yoffsets = 0;
      }
    }
    else {
      // number
      {
        sequence.rect = &numberRect;
        sequence.align = SCREEN_ALIGN_RIGHT;
        sequence.size = 3;
        int8_t xoffsets[10] = {0};
        for (int8_t i = 0; i < sequence.size; i++) {
          meshs[i] = lcd_meshs(number,2)[11];
          xoffsets[i] = 2;
        }
        xoffsets[0] = 0;
        sequence.xoffsets = xoffsets;
        screen_drawSequence(&sequence);
        sequence.xoffsets = 0;
      }
      // slider
      {
        sequence.rect = &sliderRect;
        sequence.size = 0;
        screen_drawSequence(&sequence);
      }
      // icon
      {
        sequence.rect = &iconRect;
        sequence.size = 0;
        screen_drawSequence(&sequence);
      }
      // update time
      {
        sequence.rect = &updateTimeRect;
        meshs[0] = lcd_meshL(air,notUpdated);
        sequence.size = 1;
        sequence.align = SCREEN_ALIGN_RIGHT;
        screen_drawSequence(&sequence);
      }
      // const screen_rect_t* rects[] = {&numberRect, &sliderRect, &iconRect, &updateTimeRect};
      // for (int8_t i = 0; i < sizeof(rects)/sizeof(screen_rect_t*); i++) {
      //   sequence.rect = rects[i];
      //   sequence.size = 0;
      //   screen_drawSequence(&sequence);
      // }
    }

    sequence.rect = &rect;
    // grid
    sequence.backColor = primaryColor;
    int8_t xoffsets[10] = {0};
    int8_t yoffsets[10] = {0};
    sequence.xoffsets = xoffsets;
    sequence.yoffsets = yoffsets;
    int16_t values[6] = {
      lcd_get16(valueAirPm2d5),
      lcd_get16(valueAirPm10),
      lcd_get16(valueAirSo2),
      lcd_get16(valueAirNo2),
      lcd_get16(valueAirCo),
      lcd_get16(valueAirO3),
    };
    int8_t levels[6] = {0};
    {
      int8_t i = 0;
      levels[i++] = calculateFrameLevel(g_lcd_frame_pm2d5 + 1, 5, values[i] * 100);
      levels[i++] = calculateFrameLevel(g_lcd_frame_pm10, 5, values[i]);
      levels[i++] = calculateFrameLevel(g_lcd_frame_so2, 5, values[i]);
      levels[i++] = calculateFrameLevel(g_lcd_frame_no2, 5, values[i]);
      levels[i++] = calculateFrameLevel(g_lcd_frame_co, 5, values[i]);
      levels[i++] = calculateFrameLevel(g_lcd_frame_o3, 5, values[i]);
    }

    // int16_t values[6] = {1,2,3,4,5,6};
    const int16_t xGrids[] = {1,58,122,164,199};
    const int16_t xBase = 20;
    const int16_t yOffset = 1;
    // const int16_t nodataValue = -1;
    for (int8_t i = 0; i < 6; i++) {
      int8_t xIndex = 0;
      rect.y = 73 + lcd_const(titleHeight) + i*30;
      rect.h = 29;
      // label
      rect.x = xBase + xGrids[xIndex];
      rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
      meshs[0] = lcd_meshs(air, labels)[i];
      xoffsets[0] = 7;
      sequence.align = SCREEN_ALIGN_LEFT;
      sequence.size = 1;
      yoffsets[0] = yOffset;
      screen_drawSequence(&sequence);
      xIndex ++;
      if (airUpdateTime != 0 && airUpdateTime != UINT32_MAX && values[i] >= 0) {
        // icon
        rect.x = xBase + xGrids[xIndex];
        rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
        meshs[0] = lcd_meshsL(icon, air)[levels[i]];
        xoffsets[0] = 0;
        sequence.align = SCREEN_ALIGN_CENTER;
        sequence.size = 1;
        sequence.frontColor = g_lcd_color_dust[levels[i]];
        yoffsets[0] = yOffset;
        screen_drawSequence(&sequence);
        sequence.frontColor = frontColor;
        xIndex ++;

        //number
        rect.x = xBase + xGrids[xIndex];
        rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
        int16_t value = values[i];
        screen_dec_t dec = {value, lcd_meshs(number,3), i==4?3:0, 1, 4};
        uint8_t size = screen_fetchDec(&dec,meshs);
        fors (10) {
          xoffsets[i] = 2;
          yoffsets[i] = yOffset;
        }
        xoffsets[0] = 0;
        sequence.align = SCREEN_ALIGN_RIGHT;
        sequence.size = size;
        screen_drawSequence(&sequence);
        xIndex ++;

        // unit
        rect.x = xBase + xGrids[xIndex];
        rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
        meshs[0] = lcd_meshs(air, units)[i];
        xoffsets[0] = 7;
        sequence.align = SCREEN_ALIGN_LEFT;
        sequence.size = 1;
        yoffsets[0] = yOffset + 2;
        screen_drawSequence(&sequence);
        xIndex ++;
      }
      else {
        // nodata
        rect.x = xBase + xGrids[xIndex];
        rect.w = xGrids[xIndex + 3] - xGrids[xIndex];
        meshs[0] = lcd_meshs(number,3)[11];
        meshs[1] = lcd_meshs(number,3)[11];
        screen_mesh_t dummyMesh = {SCREEN_MESH_NULL, 20, 0};
        meshs[2] = &dummyMesh;
        xoffsets[0] = 0;
        xoffsets[1] = 2;
        yoffsets[0] = yOffset;
        yoffsets[1] = yOffset;
        sequence.align = SCREEN_ALIGN_RIGHT;
        sequence.size = 3;
        screen_drawSequence(&sequence);
        xIndex += 3;
      }
    }
    sequence.xoffsets = 0;
    sequence.yoffsets = 0;
  }
}
/// end
//static uint8_t g_weatherLevel2IconIndex_en[40] = {0 ,1 ,2 ,2 ,3 ,3 ,3 ,3 ,3 ,4 ,\
//												 5 ,5 ,5 ,6 ,6 ,6 ,7 ,7 ,7 ,6,\
//												 8 ,9 ,9, 9 ,9 ,9 ,10,11,12,13,\
//												 14,15,16,17,18,7 ,19,20,21};
//static uint8_t g_weatherLevel2IconIndex[40] = {0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,2 ,\
//														 3 ,4 ,4 ,5 ,6 ,7 ,8 ,9 ,9 ,10,\
//														 11,12,13,14,15,16,17,18,19,19,\
//														 20,21,22,23,24,25,26,27,28};
//static uint16_t g_weatherIndex2Color[] = {rgb(0,255,24),rgb(182,182,182),rgb(103,103,103),rgb(0,174,255),rgb(2,138,242),rgb(2,92,242),rgb(2,52,195),rgb(0,25,152),rgb(3,11,114),rgb(7,13,91),\
// rgb(255,255,255),rgb(255,255,255),rgb(255,255,255),rgb(255,255,255),rgb(255,255,255),rgb(255,255,255),rgb(240,255,0),rgb(255,216,0),rgb(255,126,0),rgb(255,54,0),\
// rgb(255,0,18),rgb(126,0,255),rgb(91,0,204),rgb(78,7,166),rgb(60,6,126),rgb(32,5,73),rgb(0,255,150),rgb(231,136,255)};
// static uint8_t toWeatherIconIndex(uint8_t level) {
//   return g_weatherLevel2IconIndex[level];
// }
//static uint8_t weather_l2ii( uint8_t level) {
//  level = min(level,39);
//  return lcd_get(language) == 0 ? g_weatherLevel2IconIndex[level] : g_weatherLevel2IconIndex_en[level];
//}
//static uint16_t weather_l2c( uint8_t level) {
//  level = min(level,39);
//  return g_weatherIndex2Color[g_weatherLevel2IconIndex[level]];
//}

enum lcd_render_weather_bits_t {
  LCD_RENDER_WEATHER_BIT_static,
  LCD_RENDER_WEATHER_BIT_latestData,
  LCD_RENDER_WEATHER_BIT_forecastData,
  LCD_RENDER_WEATHER_BIT_SIZE,
};
static void renderWeather(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];
  screen_rect_t rect = {0};
  const screen_mesh_t* meshs[10] = {0};
  screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};

  // static
  if (updateBits & (1<<LCD_RENDER_AIR_BIT_static)) {
    // aqi label
    rect.x = 20;
    rect.y = 5+lcd_const(titleHeight);
    rect.w = 84;
    rect.h = 24;
    meshs[0] = lcd_meshL(weather, weatherLabel);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);

    //forecastLabel
    rect.x = 20;
    rect.y = 49+lcd_const(titleHeight);
    rect.w = 50;
    rect.h = 20;
    meshs[0] = lcd_meshL(weather, forecastLabel);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);

    //grid
    rect.x = 20;
    rect.y = 72+lcd_const(titleHeight);
    rect.w = 200;
    rect.h = 181;
    int16_t rowSpaces[] = {29,29,29,29,29,29};
    screen_grid_t grid = {&rect, SCREEN_ALIGN_CENTER,0,0,200,181,rowSpaces,6,0,1,1,1,secondaryColor,secondaryColor, backColor};
    screen_drawGrid(&grid);

    //navigate
    const screen_mesh_t* navigateMeshs[] = {lcd_meshL(navigate,back),0,0};
    renderNavigate(navigateMeshs, frontColor, primaryColor);
  }

  if (updateBits & (1<<LCD_RENDER_WEATHER_BIT_latestData)) {
    uint32_t weatherUpdateTime = lcd_get32(weatherUpdateTime);
    const screen_rect_t numberRect = {140,8+lcd_const(titleHeight),80,32};
    const screen_rect_t iconRect = {98,10+lcd_const(titleHeight),55,15};
		const screen_rect_t windRect = {20,34+lcd_const(titleHeight),125,11};
    const screen_rect_t updateTimeRect = {80,53+lcd_const(titleHeight),140,13};
    if (weatherUpdateTime != 0 && weatherUpdateTime != UINT32_MAX){
      // number
      {
				int8_t xoffsets[10] = {0};
				int8_t yoffsets[5] = {0};
        sequence.rect = &numberRect;
				for (int8_t i = 0; i < sequence.size; i++) {
          xoffsets[i] = 0;
        }
        screen_dec_t dec = {(int8_t)lcd_get(weatherTemperature), lcd_meshs(number,2), 0, 1, 3};
        uint8_t size = screen_fetchDec(&dec,meshs);
				yoffsets[size] = 4;
				xoffsets[size] = 1;
				meshs[size++] = lcd_mesh(weather, tempratureUnit);
        sequence.align = SCREEN_ALIGN_RIGHT;
        sequence.size = size;
        sequence.xoffsets = xoffsets;
				sequence.yoffsets = yoffsets;
        screen_drawSequence(&sequence);
        sequence.xoffsets = 0;
				sequence.yoffsets = 0;
      }
      // icon
      {
        sequence.rect = &iconRect;
        uint8_t weatherLevel = min((uint8_t)lcd_get(weatherLevel),40);
				uint8_t weatherIconIndex = weather_l2ii(weatherLevel);
        uint16_t color = weather_l2c(weatherLevel);
				weatherIconIndex = min(weatherIconIndex, sizeof(lcd_meshs(icon, weather))/ sizeof(void*));
        meshs[0] = lcd_meshsL(icon, weather)[weatherIconIndex];
        sequence.align = SCREEN_ALIGN_LEFT;
        sequence.size = 1;
        sequence.frontColor = color;
        screen_drawSequence(&sequence);
        sequence.frontColor = frontColor;
      }
			//wind
			{
				sequence.rect = &windRect;
				int8_t xoffsets[10] = {0};
				for (int8_t i = 0; i < 10; i++) {
          xoffsets[i] = 1;
        }
				uint8_t size = 0;
				meshs[size ++] = lcd_meshL(weather, windLevelLabel);

        if (lcd_get(language) == 0) {
  				screen_dec_t dec = {lcd_get(weatherWindLevel), lcd_meshs(number,6), 0, 1, 2};
  				size += screen_fetchDec(&dec,meshs + size);
  				meshs[size++] = lcd_meshL(weather, windLevelUnit);
        } else {
          meshs[size++] = lcd_meshL(weather, windLevelUnit);
          screen_dec_t dec = {lcd_get(weatherWindLevel), lcd_meshs(number,6), 0, 1, 2};
          size += screen_fetchDec(&dec,meshs + size);
        }
        if (lcd_get(language) == 0) {
				  xoffsets[size] = 2;
		      meshs[size++] = lcd_mesh(weather, windDirectionLabel);
        } else {
				  xoffsets[size] = 5;
        }
				meshs[size++] = lcd_meshsL(weather, windDirections)[lcd_get(weatherWindDirection) & 0x07];
        if (lcd_get(language) == 0) {
          meshs[size++] = lcd_mesh(weather, windDirectionUnit);
        }
				sequence.align = SCREEN_ALIGN_LEFT;
				sequence.size = size;
				sequence.xoffsets = xoffsets;
				screen_drawSequence(&sequence);
				sequence.xoffsets = 0;
			}
      // update time
      {
        sequence.rect = &updateTimeRect;
        time_t time;
        time_fromSecond(weatherUpdateTime, &time);
        const screen_mesh_t* updateTimeMeshs[15] = {0};
        int8_t xoffsets[15] = {0};
        for (int8_t i = 0; i < 15; i++) {
          xoffsets[i] = 2;
        }
        int8_t yoffsets[15] = {0};
        for (int8_t i = 0; i < 15; i++) {
          yoffsets[i] = 0;
        }
        uint8_t size = 0;
        screen_dec_t dec = {0, lcd_meshs(number,6), 0, 1, 4};
        if (lcd_get(language) == 0) {
          dec.data = time.month + 1;
          size += screen_fetchDec(&dec,updateTimeMeshs + size);
          updateTimeMeshs[size++] = lcd_mesh(air,calendarMonth);
          dec.data = time.day + 1;
          size += screen_fetchDec(&dec,updateTimeMeshs + size);
          updateTimeMeshs[size++] = lcd_mesh(air,calendarDay);
          xoffsets[size] = 6;
        } else {
          updateTimeMeshs[size++] = lcd_meshs(date,month_en)[time.month];
          updateTimeMeshs[size++] = lcd_meshs(date,day)[time.day];
          xoffsets[size] = 6;
        }
        dec.minSize = 2;
        dec.data = time.hour;
        size += screen_fetchDec(&dec,updateTimeMeshs + size);
        updateTimeMeshs[size++] = lcd_mesh(air,calendarHour);
        dec.data = time.minute;
        size += screen_fetchDec(&dec,updateTimeMeshs + size);
        if (lcd_get(language) != 0) {
          xoffsets[size] = 2;
          yoffsets[size] = 1;
        }
        updateTimeMeshs[size++] = lcd_meshL(air,calendarMinute);
        sequence.align = SCREEN_ALIGN_RIGHT;
        sequence.size = size;
        sequence.meshs = updateTimeMeshs;
        sequence.xoffsets = xoffsets;
        sequence.yoffsets = yoffsets;
        screen_drawSequence(&sequence);
        sequence.meshs = meshs;
        sequence.xoffsets = 0;
        sequence.yoffsets = 0;
      }
    }
    else {
      // number
      {
        sequence.rect = &numberRect;
        sequence.align = SCREEN_ALIGN_RIGHT;
        sequence.size = 3;
        int8_t xoffsets[10] = {0};
        for (int8_t i = 0; i < sequence.size; i++) {
          meshs[i] = lcd_meshs(number,2)[11];
          xoffsets[i] = 2;
        }
        xoffsets[0] = 0;
        sequence.xoffsets = xoffsets;
        screen_drawSequence(&sequence);
        sequence.xoffsets = 0;
      }
      // icon
      {
        sequence.rect = &iconRect;
        sequence.size = 0;
        screen_drawSequence(&sequence);
      }
      // update time
      {
        sequence.rect = &updateTimeRect;
        meshs[0] = lcd_meshL(weather,notUpdated);
        sequence.size = 1;
        sequence.align = SCREEN_ALIGN_RIGHT;
        screen_drawSequence(&sequence);
      }
      const screen_rect_t* rects[] = {&iconRect};
      for (int8_t i = 0; i < sizeof(rects)/sizeof(screen_rect_t*); i++) {
        sequence.rect = rects[i];
        sequence.size = 0;
        screen_drawSequence(&sequence);
      }
    }

  }
	if (updateBits & (1<<LCD_RENDER_WEATHER_BIT_forecastData)) {
    uint32_t forecastUpdateTime = lcd_get32(forecastUpdateTime);
    sequence.rect = &rect;
    // grid
    sequence.backColor = primaryColor;
    int8_t xoffsets[10] = {0};
    int8_t yoffsets[10] = {0};
    sequence.xoffsets = xoffsets;
    sequence.yoffsets = yoffsets;
    const int16_t xGrids[] = {1,58,115,165,184,199};
    const int16_t xBase = 20;
    const int16_t yOffset = 1;
    int8_t lastDayTemperature;
    uint8_t gotLastDayTemperature = 0;
    fors (6) {
      int8_t xIndex = 0;
			foris(j, 10) {
				xoffsets[j] = 0;
				yoffsets[j] = 0;
			}
      rect.y = 73 + lcd_const(titleHeight) + i*30;
      rect.h = 29;
      // data

      uint8_t weatherCode = lcd_get(forecastDay1Weather + i*4);
      // label
      rect.x = xBase + xGrids[xIndex];
      rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
			if(i<3) {
				meshs[0] = lcd_meshsL(weather, forecasttop3DayLabels)[i];
			} else {
        uint8_t dayIndex = lcd_get(forecastDay1Weekday + i*4);
        if (dayIndex > sizeof(lcd_meshs(weather, forecastDayLabels))/sizeof(void*)) {
          meshs[0] = lcd_meshsL(weather, forecastDayLabels)[i];
        } else {
          meshs[0] = lcd_meshsL(weather, forecastDayLabels)[dayIndex];
        }
      }

      if (lcd_get(language) == 0) {
        xoffsets[0] = 7;
      } else {
        xoffsets[0] = 4;
      }
      sequence.align = SCREEN_ALIGN_LEFT;
      sequence.size = 1;
      yoffsets[0] = yOffset;
      screen_drawSequence(&sequence);
      xIndex ++;
			// icon
			rect.x = xBase + xGrids[xIndex];
      rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
			if(weatherCode != 0xFF) {
        uint8_t weatherIndex = weather_l2ii(weatherCode);
        uint16_t iconColor = weather_l2c(weatherCode);
        meshs[0] = lcd_meshsL(icon, weather)[weatherIndex];
        xoffsets[0] = 0;
        sequence.align = SCREEN_ALIGN_CENTER;
        sequence.size = 1;
        sequence.frontColor = iconColor;
        yoffsets[0] = yOffset;
        screen_drawSequence(&sequence);
        sequence.frontColor = frontColor;
      } else {
        sequence.size = 0;
        screen_drawSequence(&sequence);
      }
      xIndex ++;
      // number
      rect.x = xBase + xGrids[xIndex];
      rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
			foris (j, 10) {
        xoffsets[j] = 2;
        yoffsets[j] = yOffset;
      }
      xoffsets[0] = 0;

  		uint8_t size = 0;
      if(weatherCode != 0xFF) {
        int8_t lvalue = lcd_get(forecastDay1TempratureL + i*4);
        int8_t hvalue = lcd_get(forecastDay1TempratureH + i*4);
        screen_dec_t decl = {lvalue, lcd_meshs(number, 3), 0, 1, 3};
        size = screen_fetchDec(&decl, meshs);
				yoffsets[size] = yOffset;
				meshs[size++] = lcd_mesh(weather, forecastTempratureConnector);
				screen_dec_t dech = {hvalue, lcd_meshs(number, 3), 0, 1, 3};
				size += screen_fetchDec(&dech, meshs+size);
			} else {
        meshs[size++] = lcd_meshs(number,3)[11];
        meshs[size++] = lcd_meshs(number,3)[11];
        meshs[size++] = lcd_mesh(weather,forecastTempratureConnector);
        meshs[size++] = lcd_meshs(number,3)[11];
        meshs[size++] = lcd_meshs(number,3)[11];
      }
      sequence.size = size;
      sequence.align = SCREEN_ALIGN_RIGHT;
      screen_drawSequence(&sequence);
      xIndex ++;
      //unit
      rect.x = xBase + xGrids[xIndex];
      rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
      meshs[0] = lcd_mesh(weather,forecastTempratureUnit);
      xoffsets[0] = 4;
      yoffsets[0] = 1;
      sequence.align = SCREEN_ALIGN_LEFT;
      sequence.size = 1;
      screen_drawSequence(&sequence);
      xIndex ++;
      // inc or dec
      rect.x = xBase + xGrids[xIndex];
      rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
      if(weatherCode != 0xFF) {
        int8_t lvalue = lcd_get(forecastDay1TempratureL + i*4);
        int8_t hvalue = lcd_get(forecastDay1TempratureH + i*4);
        if (gotLastDayTemperature == 0){
          meshs[0] = lcd_mesh(weather, forecastTempratureEquel);
          sequence.frontColor = rgb(29,218,133);
        } else {
          int8_t temp = lvalue + hvalue;
          if (temp > lastDayTemperature + 1) {
            meshs[0] = lcd_mesh(weather, forecastTempratureUp);
            sequence.frontColor = rgb(251,209,126);
          } else if (temp < lastDayTemperature - 1) {
            meshs[0] = lcd_mesh(weather, forecastTempratureDown);
            sequence.frontColor = rgb(186,227,250);
          } else {
            meshs[0] = lcd_mesh(weather, forecastTempratureEquel);
            sequence.frontColor = rgb(29,218,133);
          }
        }
        sequence.size = 1;

        lastDayTemperature = lvalue + hvalue;
        gotLastDayTemperature = 1;
      } else {
        sequence.size = 0;
      }
      xoffsets[0] = 0;
      sequence.align = SCREEN_ALIGN_LEFT;
      screen_drawSequence(&sequence);
      sequence.frontColor = frontColor;
      xIndex ++;
    }
	}
}

enum lcd_render_dust_bits_t {
  LCD_RENDER_DUST_BIT_static,
  LCD_RENDER_DUST_BIT_data,
  LCD_RENDER_DUST_BIT_SIZE,
};
static void renderDust(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();

  const int16_t xGrids[] = {1,58,144,199};
  const int16_t xBase = 20;
  const int16_t yOffset = 1;
  const int16_t nodataValue = -1;

  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];
  screen_rect_t rect = {0};
  const screen_mesh_t* meshs[10] = {0};
  screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
	  // static
  if (updateBits & (1<<LCD_RENDER_DUST_BIT_static)) {
    // aqi label
    rect.x = 20;
    rect.y = 5+lcd_const(titleHeight);
    rect.w = 84;
    rect.h = 24;
    meshs[0] = lcd_mesh(dust, pm2d5Label);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);

    // color bar
    rect.x = 20;
    rect.y = 35+lcd_const(titleHeight);
    rect.w = 96;
    rect.h = 2;
    screen_colorbar_t bar = {&rect,SCREEN_ALIGN_TOP,0,0,96,2,g_lcd_color_dust,6,backColor};
    screen_drawColorbar(&bar);
    //grid
    rect.x = 20;
    rect.y = 42+lcd_const(titleHeight);
    rect.w = 200;
    rect.h = 211;
    int16_t rowSpaces[] = {29,29,29,29,29,29,29};
    screen_grid_t grid = {&rect, SCREEN_ALIGN_CENTER,0,0,200,211,rowSpaces,7,0,1,1,1,secondaryColor,secondaryColor, backColor};
    screen_drawGrid(&grid);
    // grid
    sequence.backColor = primaryColor;
    int8_t xoffsets[10] = {0};
    int8_t yoffsets[10] = {0};
    sequence.xoffsets = xoffsets;
    sequence.yoffsets = yoffsets;
    // int8_t levels[6] = {0};
    int16_t values[7] = {0};
    for (int8_t i = 0; i < 7; i++) {
      int8_t xIndex = 0;
      rect.y = 43 + lcd_const(titleHeight) + i*30;
      rect.h = 29;
      // label
      rect.x = xBase + xGrids[xIndex];
      rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
      meshs[0] = lcd_meshs(dust, labels)[i];
      xoffsets[0] = 7;
      sequence.align = SCREEN_ALIGN_LEFT;
      sequence.size = 1;
      yoffsets[0] = yOffset;
      screen_drawSequence(&sequence);
      xIndex ++;
      if (values[i] != nodataValue){
        //number
        // rect.x = xBase + xGrids[xIndex];
        // rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
        // int16_t value = values[i];
        // screen_dec_t dec = {value, lcd_meshs(number,3), 1, 1, 4};
        // uint8_t size = screen_fetchDec(&dec,meshs);
        // fors (10) {
        //   xoffsets[i] = 2;
        //   yoffsets[i] = yOffset;
        // }
        // xoffsets[0] = 0;
        // sequence.align = SCREEN_ALIGN_RIGHT;
        // sequence.size = size;
        // screen_drawSequence(&sequence);
        xIndex ++;

        // unit
        rect.x = xBase + xGrids[xIndex];
        rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
        meshs[0] = lcd_meshs(dust, units)[i];
        xoffsets[0] = 20;
        sequence.align = SCREEN_ALIGN_LEFT;
        sequence.size = 1;
        yoffsets[0] = yOffset + 2;
        screen_drawSequence(&sequence);
        xIndex ++;
      }
    }
    sequence.xoffsets = 0;
    sequence.yoffsets = 0;
    sequence.backColor = backColor;
    //navigate
    const screen_mesh_t* navigateMeshs[] = {lcd_meshL(navigate,preview),lcd_meshL(navigate,history),lcd_meshL(navigate,nextview)};
    renderNavigate(navigateMeshs, frontColor, primaryColor);
  }

  if (updateBits & (1<<LCD_RENDER_DUST_BIT_data)) {
    const screen_rect_t numberRect = {122,8+lcd_const(titleHeight),90,32};
    const screen_rect_t sliderRect = {17,30+lcd_const(titleHeight),104,5};
    // const screen_rect_t iconRect = {101,10+lcd_const(titleHeight),50,15};
    // const screen_rect_t updateTimeRect = {80,53+lcd_const(titleHeight),140,13};
      // number
		{
			sequence.rect = &numberRect;
			screen_dec_t dec = {lcd_get32(valuePm2d5), lcd_meshs(number,2), 2, 1, 4};
			uint8_t size = screen_fetchDec(&dec,meshs);
			sequence.align = SCREEN_ALIGN_RIGHT;
			sequence.size = size;
			int8_t xoffsets[10] = {0};
			for (int8_t i = 0; i < sequence.size; i++) {
				xoffsets[i] = 2;
			}
			xoffsets[0] = 0;
			sequence.xoffsets = xoffsets;
			screen_drawSequence(&sequence);
			sequence.xoffsets = 0;
		}
		// slider
		{
      int16_t percent = calculateFramePercent(g_lcd_frame_pm2d5, 7, lcd_get32(valuePm2d5));
			percent = min(percent,1000);
			percent = max(percent,0);
			screen_slider_t slider = {&sliderRect,SCREEN_ALIGN_BOTTOM,0,0,93,5,percent,lcd_mesh(dust,slider),frontColor,backColor};
			screen_drawSlider(&slider);
		}
    sequence.rect = &rect;
    // grid
    sequence.backColor = primaryColor;
    int8_t xoffsets[10] = {0};
    int8_t yoffsets[10] = {0};
    sequence.xoffsets = xoffsets;
    sequence.yoffsets = yoffsets;
    //int8_t levels[6] = {0};
    int32_t values[7] = {
      lcd_get32(valuePm1d0),
      lcd_get32(valuePm10),
      lcd_get32(valuePc0d3),
      lcd_get32(valuePc1d0),
      lcd_get32(valuePc2d5),
      lcd_get32(valuePc5d0),
      lcd_get32(valuePc10),
    };
    for (int8_t i = 0; i < 7; i++) {
      int8_t xIndex = 0;
      rect.y = 43 + lcd_const(titleHeight) + i*30;
      rect.h = 29;
      // label
      // rect.x = xBase + xGrids[xIndex];
      // rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
      // meshs[0] = lcd_meshs(dust, labels)[i];
      // xoffsets[0] = 7;
      // sequence.align = SCREEN_ALIGN_LEFT;
      // sequence.size = 1;
      // yoffsets[0] = yOffset;
      // screen_drawSequence(&sequence);
      xIndex ++;
      if (values[i] != nodataValue){
        //number
        rect.x = xBase + xGrids[xIndex];
        rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
        int32_t value = values[i];
        screen_dec_t dec = {value, lcd_meshs(number,3), i<2?2:0, 1, 10};
        uint8_t size = screen_fetchDec(&dec,meshs);
        fors(size) {
          xoffsets[i] = 2;
          yoffsets[i] = yOffset;
        }
        xoffsets[0] = 0;
        sequence.align = SCREEN_ALIGN_RIGHT;
        sequence.size = size;
        screen_drawSequence(&sequence);
        xIndex ++;

        // unit
        // rect.x = xBase + xGrids[xIndex];
        // rect.w = xGrids[xIndex + 1] - xGrids[xIndex];
        // meshs[0] = lcd_meshs(dust, units)[i];
        // xoffsets[0] = 20;
        // sequence.align = SCREEN_ALIGN_LEFT;
        // sequence.size = 1;
        // yoffsets[0] = yOffset + 2;
        // screen_drawSequence(&sequence);
        xIndex ++;
      }
    }
    sequence.xoffsets = 0;
    sequence.yoffsets = 0;
  }
}

enum lcd_render_history_bits_t {
  LCD_RENDER_HISTORY_BIT_static,
  LCD_RENDER_HISTORY_BIT_timeType,
  LCD_RENDER_HISTORY_BIT_dataType,
  LCD_RENDER_HISTORY_BIT_figure,
  LCD_RENDER_HISTORY_BIT_SIZE,
};

#define LCD_FIGURE_WIDTH 169
#define LCD_FIGURE_HEIGHT 150

static inline int32_t getFifoData(int32_t* buffer, int16_t index, int16_t size) {
  if (index >= size) {
    index -= size;
  }
  return buffer[index];
}

static int16_t fetchHistoryData(history_t* h, uint32_t startTime, int32_t maxData, int16_t* xBuffer, int16_t* yBuffer) {
  int16_t size = lcd_get32(historyTimeRange)/max(h->intervalTime,1);
  int32_t xMulti = ((LCD_FIGURE_WIDTH - 1) << 8)/(size - 1);
  int32_t yMulti = (LCD_FIGURE_HEIGHT << 20)/maxData;
  for (int16_t i = 0; i < size; i++) {
    xBuffer[i] = (i * xMulti) >> 8;
    uint32_t t = startTime + i * h->intervalTime;
    int16_t v = 0;
    history_getData(h, t, &v);
    yBuffer[i] = v<0?v:(v * yMulti) >> 20;
  }
  return size;
}

static void renderHistory(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];
  screen_rect_t rect = {0};
  const screen_mesh_t* meshs[10] = {0};
  screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};

  if (updateBits & (1<<LCD_RENDER_HISTORY_BIT_dataType)){
    //navigate
    const screen_mesh_t* navigateMeshs[] = {lcd_meshL(navigate,back),lcd_get(historyDataType) == 0?lcd_meshL(navigate,co2):lcd_meshL(navigate,pm2d5),lcd_meshL(navigate,switch)};
    renderNavigate(navigateMeshs, frontColor, primaryColor);
  }
  if ((updateBits & (1<<LCD_RENDER_HISTORY_BIT_timeType)) || (updateBits & (1<<LCD_RENDER_HISTORY_BIT_dataType))){
    //title
    uint8_t type = lcd_get(historyTimeType);
    screen_rect_t titleRect = {10,10 + lcd_const(titleHeight),220,32};
    meshs[0] = lcd_get(historyDataType) == 0?lcd_meshL(history,pm2d5Label):lcd_meshL(history,co2Label);
    meshs[1] = lcd_meshL(history,historyLabel);
    meshs[2] = lcd_meshsL(history,perioLabels)[type];
    sequence.size = 3;
    sequence.rect = &titleRect;
    int8_t xoffsets[10] = {0};
    fors(sizeof(xoffsets)) {
      xoffsets[i] = 4;
    }
    sequence.xoffsets = xoffsets;
    screen_drawSequence(&sequence);
    sequence.rect = &rect;
    sequence.xoffsets = 0;
  }


  if (updateBits & (1<<LCD_RENDER_HISTORY_BIT_figure)) {
    screen_rect_t figureRect = {44,70 + lcd_const(titleHeight),LCD_FIGURE_WIDTH,LCD_FIGURE_HEIGHT};
    uint16_t xGridSizeLS8;
    uint16_t xGridCount;
    uint16_t yGridInterval;
    uint16_t yGridCount;
    uint16_t yGridSizeLS8;
    history_t* history1 = (history_t*)lcd_get32(historyData1);
    history_t* history2 = (history_t*)lcd_get32(historyData2);
    uint32_t endTime = lcd_get32(historyEndTime);
    int32_t timeRange = lcd_get32(historyTimeRange);
    uint32_t startTime = endTime - timeRange;

    uint32_t inDoorColor, outDoorColor;
    if (history2) {
      outDoorColor = rgb(0,255,0);
      inDoorColor = rgb(200,100,0);
    } else {
      inDoorColor = rgb(200,100,0);
    }

    //x axis
    {
      int8_t gridCount = lcd_get(historyXGridCount);
      int32_t gridSizeLS8 = ((LCD_FIGURE_WIDTH-1) << 8)/gridCount;
      xGridSizeLS8 = gridSizeLS8;
      xGridCount = gridCount;

      //draw
      int8_t xoffsets[10] = {0};
      fors(sizeof(xoffsets)) {
        xoffsets[i] = 1;
      }
      sequence.xoffsets = xoffsets;
      screen_rect_t xAxisRect = {10,figureRect.y + figureRect.h + 7,220,10};
      rect.y = xAxisRect.y;
      rect.h = xAxisRect.h;
      rect.x = xAxisRect.x;
      int32_t timeInterval = (timeRange)/gridCount;
      for (int8_t i = 0; i < gridCount + 1; i++) {
        int16_t x = figureRect.x + (i*gridSizeLS8 >> 8);
        uint32_t timestamp = startTime + timeInterval * i;
        time_t time;
        time_fromSecond(timestamp, &time);
        int8_t valueLeft;
        int8_t valueRight;
        int8_t minsize;
        const screen_mesh_t* connectMesh;
        if (timeRange > 24*60*60) {
          valueLeft = time.month + 1;
          valueRight = time.day + 1;
          connectMesh = lcd_mesh(history,connectorSlash);
          minsize = 1;
        } else if (timeRange > 60*60) {
          valueLeft = time.hour;
          valueRight = time.minute;
          connectMesh = lcd_mesh(history,connectorColon);
          minsize = 2;
        } else if (timeRange > 60) {
          valueLeft = time.hour;
          valueRight = time.minute;
          connectMesh = lcd_mesh(history,connectorColon);
          minsize = 2;
        } else {
          valueLeft = time.minute;
          valueRight = time.second;
          connectMesh = lcd_mesh(history,connectorColon);
          minsize = 2;
        }
        screen_dec_t dec = {valueLeft, lcd_meshs(number,6), 0,minsize,2};
        int8_t meshSize = 0;
        meshSize += screen_fetchDec(&dec,meshs + meshSize);
        meshs[meshSize ++] = connectMesh;
        dec.data = valueRight;
        meshSize += screen_fetchDec(&dec,meshs + meshSize);
        sequence.size = meshSize;
        int16_t meshsWidth = screen_fetchSequenceWidth(&sequence);
        if (rect.x < x - (meshsWidth>>1)) {
          rect.w = x - (meshsWidth>>1) - rect.x;
          sequence.size = 0;
          screen_drawSequence(&sequence);
        }
        {
          rect.x = x - (meshsWidth>>1);
          rect.w = meshsWidth;
          sequence.size = meshSize;
          screen_drawSequence(&sequence);
          rect.x += meshsWidth;
        }
      }
      if (rect.x < xAxisRect.x + xAxisRect.w) {
        rect.w = xAxisRect.x + xAxisRect.w - rect.x;
        sequence.size = 0;
        screen_drawSequence(&sequence);
      }
      sequence.xoffsets = 0;
    }
    //y axis
    {

      int32_t maxData = 0, minData = 0;
      history_getMaxMinData(history1, startTime, startTime + timeRange, &maxData, &minData);
      if (history2) {
        int32_t max2 = 0, min2 = 0;
        history_getMaxMinData(history2, startTime, startTime + timeRange, &max2, &min2);
        maxData = max(maxData, max2);
        minData = min(minData, min2);
      }

      // int16_t size1 = lcd_get(historyData1DataSize);
      // int16_t bufferSize1 = lcd_get(historyData1BufferSize);
      // int16_t offset1 = lcd_get16(historyData1BufferOffset);
      // int16_t size2 = lcd_get(historyData2DataSize);
      // int16_t bufferSize2 = lcd_get(historyData2BufferSize);
      // int16_t offset2 = lcd_get16(historyData2BufferOffset);
      // for (int16_t i = 0; i < size1; i++) {
      //   int32_t data = 0;
      //   history_getData(history1, )
      //    = getFifoData(buffer1,offset1 + i,bufferSize1);
      //   if (maxData < data) {
      //     maxData = data;
      //   }
      //   if (minData > data) {
      //     minData = data;
      //   }
      // }
      // if (history2) {
      //   for (int16_t i = 0; i < size2; i++) {
      //     int32_t data = getFifoData(buffer2,offset2 + i,bufferSize2);
      //     if (maxData < data) {
      //       maxData = data;
      //     }
      //     if (minData > data) {
      //       minData = data;
      //     }
      //   }
      // }
      maxData += maxData >> 3;
      maxData = max(maxData, 20);
      int16_t gridCount;
      uint32_t gridInterval = screen_fetchGridInterval(0,maxData,8,&gridCount);
      uint32_t gridSizeLS8 = ((LCD_FIGURE_HEIGHT-1) << 8)/gridCount;

      yGridInterval = gridInterval;
      yGridCount = gridCount;
      yGridSizeLS8 = gridSizeLS8;


      // draw
      int16_t x0 = max(figureRect.x - 50,0);

      screen_rect_t yAxisRect = {x0,figureRect.y -5,figureRect.x - 5 - x0,figureRect.h + 10};
      int8_t xoffsets[10] = {0};
      fors (sizeof(xoffsets)) {
        xoffsets[i] = 1;
      }
      sequence.xoffsets = xoffsets;
      sequence.align = SCREEN_ALIGN_RIGHT;
      rect.x = yAxisRect.x;
      rect.w = yAxisRect.w;
      rect.y = yAxisRect.y;
      for (int8_t i = 0; i < gridCount + 1; i++) {
        int16_t y = figureRect.y + (i*gridSizeLS8 >> 8) + 1;
        uint32_t value = gridInterval * (gridCount - i);
        screen_dec_t dec = {value, lcd_meshs(number,6), 0,0,4};
        int8_t size = screen_fetchDec(&dec,meshs);
        int16_t height = meshs[0]->h;
        if (rect.y < y - (height>>1)) {
          rect.h = y - (height>>1) - rect.y;
          sequence.size = 0;
          screen_drawSequence(&sequence);
        }
        {
          rect.y = y - (height>>1);
          rect.h = height;
          sequence.size = size;
          screen_drawSequence(&sequence);
          rect.y += height;
        }
      }
      if (rect.y < yAxisRect.y + yAxisRect.h) {
        rect.h = yAxisRect.y + yAxisRect.h - rect.y;
        sequence.size = 0;
        screen_drawSequence(&sequence);
      }
      sequence.xoffsets = 0;
    }
    //figure
    {
      int16_t lineDataUpper1[LCD_FIGURE_WIDTH] = {0};
      int16_t lineDataLowwer1[LCD_FIGURE_WIDTH] = {0};
      int16_t lineDataUpper2[LCD_FIGURE_WIDTH] = {0};
      int16_t lineDataLowwer2[LCD_FIGURE_WIDTH] = {0};

      for (int16_t i = 0; i < LCD_FIGURE_WIDTH; i++) {
        lineDataLowwer1[i] = INT16_MAX;
        lineDataUpper1[i] = INT16_MIN;
        lineDataLowwer2[i] = INT16_MAX;
        lineDataUpper2[i] = INT16_MIN;
      }

      // int16_t size1 = lcd_get32(historyTimeRange)/max(history1->intervalTime,1);
      // int16_t size2 = lcd_get32(historyTimeRange)/max(history2->intervalTime,1);
      int32_t maxData = yGridInterval * yGridCount;
      int16_t xBuffer[61] = {0};
      int16_t yBuffer[61] = {0};
      { // line
        history_t* h;
        int16_t* upper;
        int16_t* lowwer;
        if (lcd_get(historyTimeType) > 1) {
          h = history2;
          upper = lineDataUpper2;
          lowwer = lineDataLowwer2;
        } else {
          h = history1;
          upper = lineDataUpper1;
          lowwer = lineDataLowwer1;
        }
        if (h) {
          int16_t size = fetchHistoryData(h, startTime, maxData, xBuffer, yBuffer);
          int32_t xMulti = ((LCD_FIGURE_WIDTH - 1) << 8)/(size - 1);
          for (int16_t i = 0; i < size - 1; i++) {
            if (yBuffer[i]>0 && yBuffer[i + 1]>0) {
              screen_fetchThickLine(upper, lowwer, xBuffer[i], yBuffer[i], xBuffer[i + 1], yBuffer[i + 1], 2);
            } else if (i != 0 && yBuffer[i-1] <= 0 && yBuffer[i]>0 && yBuffer[i + 1] <= 0){
              screen_fetchThickLine(upper, lowwer, xBuffer[i] - (xMulti>>9), yBuffer[i], xBuffer[i] + (xMulti>>9) - 1, yBuffer[i], 2);
            } else if (i == size - 2 && yBuffer[i] <= 0 && yBuffer[i + 1]>0){
              screen_fetchThickLine(upper, lowwer, xBuffer[i + 1] - (xMulti>>9), yBuffer[i + 1], xBuffer[i + 1], yBuffer[i + 1], 2);
            }
          }
        }
      }
      { // column

        history_t* h;
        int16_t* upper;
        int16_t* lowwer;
        if (lcd_get(historyTimeType) > 1) {
          h = history1;
          upper = lineDataUpper1;
          lowwer = lineDataLowwer1;
        } else {
          h = 0;
        }
        if (h) {
          int16_t size = fetchHistoryData(h, startTime, maxData, xBuffer, yBuffer);
          for (int16_t i = 1; i < size; i++) {
            if (yBuffer[i] > 0) {
              screen_fetchColumn(upper, lowwer, xBuffer[i-1] + (2), yBuffer[i], xBuffer[i] - xBuffer[i-1] - 1,0);
            }
          }
          if (yBuffer[0] > 0) {
            screen_fetchColumn(upper, lowwer, 0, yBuffer[0], 1, 0);
          }
        }
      }
      // {
      //   if (history2){
      //
      //   } else {
      //     int16_t* upper = lineDataUpper1;
      //     int16_t* lowwer = lineDataLowwer1;
      //     for (int16_t i = 0; i < size1 - 1; i++) {
      //       if (yBuffer[i]>0 && yBuffer[i + 1]>0) {
      //         screen_fetchThickLine(upper, lowwer, xBuffer[i], yBuffer[i], xBuffer[i + 1], yBuffer[i + 1], 2);
      //       } else if (i != 0 && yBuffer[i-1] <= 0 && yBuffer[i]>0 && yBuffer[i + 1] <= 0){
      //         screen_fetchThickLine(upper, lowwer, xBuffer[i] - (xMulti>>9), yBuffer[i], xBuffer[i] + (xMulti>>9) - 1, yBuffer[i], 2);
      //       } else if (i == size1 - 2 && yBuffer[i] <= 0 && yBuffer[i + 1]>0){
      //         screen_fetchThickLine(upper, lowwer, xBuffer[i + 1] - (xMulti>>9), yBuffer[i + 1], xBuffer[i + 1], yBuffer[i + 1], 2);
      //       }
      //     }
      //   }
      // }
      //
      //
      // if (history2) {
      //   xMulti = ((LCD_FIGURE_WIDTH - 1) << 8)/(size2 - 1);
      //   for (int16_t i = 0; i < size2; i++) {
      //     xBuffer[i] = (i * xMulti) >> 8;
      //     uint32_t t = startTime + i * history2->intervalTime;
      //     int16_t v = 0;
      //     history_getData(history2, t, &v);
      //     yBuffer[i] = (v * yMulti) >> 20;
      //   }
      //   int16_t* upper = lineDataUpper2;
      //   int16_t* lowwer = lineDataLowwer2;
      //   for (int16_t i = 0; i < size2 - 1; i++) {
      //     if (yBuffer[i]>0 && yBuffer[i + 1]>0) {
      //       screen_fetchThickLine(upper, lowwer, xBuffer[i], yBuffer[i], xBuffer[i + 1], yBuffer[i + 1], 2);
      //     } else if (i != 0 && yBuffer[i-1] <= 0 && yBuffer[i]>0 && yBuffer[i + 1] <= 0){
      //       screen_fetchThickLine(upper, lowwer, xBuffer[i] - (xMulti>>9), yBuffer[i], xBuffer[i] + (xMulti>>9) - 1, yBuffer[i], 2);
      //     } else if (i == size1 - 2 && yBuffer[i] <= 0 && yBuffer[i + 1]>0){
      //       screen_fetchThickLine(upper, lowwer, xBuffer[i + 1] - (xMulti>>9), yBuffer[i + 1], xBuffer[i + 1], yBuffer[i + 1], 2);
      //     }
      //     // screen_fetchThickLine(lineDataUpper2, lineDataLowwer2, xBuffer[i], yBuffer[i], xBuffer[i + 1], yBuffer[i + 1], 2);
      //   }
      // }
        //
      screen_lineData_t lineData1 = {lineDataUpper1, lineDataLowwer1};
      screen_lineData_t lineData2 = {lineDataUpper2, lineDataLowwer2};
      screen_lineData_t* lines[2] = {&lineData2,&lineData1};
      uint16_t lineColors[2] = {outDoorColor, inDoorColor};
      screen_figure_t figure = {&figureRect, SCREEN_ALIGN_CENTER,0,0,LCD_FIGURE_WIDTH,LCD_FIGURE_HEIGHT,
        lines,2,
        1 << 8, xGridSizeLS8-(1<<8),
        1 << 8, yGridSizeLS8-(1<<8),
        4<<8, 0,
        lineColors,secondaryColor,primaryColor,backColor
      };
      screen_drawFigure(&figure);
      //
      //
      //
      // lcd_testNumber(offset1,0);
      // lcd_testNumber(maxData,1);
    }
    {
      //tips
      screen_rect_t tipRect = {figureRect.x+20, figureRect.y-20, figureRect.w-20, 15};
      rect.y = tipRect.y;
      rect.h = tipRect.h;
      sequence.size = 1;
      sequence.align = SCREEN_ALIGN_CENTER;
      sequence.rect = &rect;
      rect.x = tipRect.x + tipRect.w;
      int16_t space = 4;

      //
      if (history2) {
        meshs[0] = lcd_mesh(history,tagOutDoor);
        rect.w = meshs[0]->w  + space;
        rect.x -= rect.w;
        sequence.frontColor = outDoorColor;
        screen_drawSequence(&sequence);

        meshs[0] = lcd_meshL(history,outDoorLabel);
        rect.w = meshs[0]->w  + space;
        rect.x -= rect.w;
        sequence.frontColor = frontColor;
        screen_drawSequence(&sequence);
      }

      meshs[0] = history2?lcd_mesh(history,tagInDoor):lcd_mesh(history,tagOutDoor);
      rect.w = meshs[0]->w + space;
      rect.x -= rect.w;
      sequence.frontColor = inDoorColor;
      screen_drawSequence(&sequence);

      meshs[0] = lcd_meshL(history,inDoorLabel);
      rect.w = meshs[0]->w + space;
      rect.x -= rect.w;
      sequence.frontColor = frontColor;
      screen_drawSequence(&sequence);

      rect.w = rect.x - tipRect.x;
      rect.x = tipRect.x;
      sequence.frontColor = frontColor;
      sequence.size = 0;
      screen_drawSequence(&sequence);

      sequence.size = 1;
      sequence.align = SCREEN_ALIGN_LEFT;
      rect.x = figureRect.x - 10;
      rect.y = figureRect.y - 20;
      rect.w = 30;
      rect.h = 10;
      meshs[0] = lcd_get(historyDataType) == 0 ? lcd_mesh(history,pm2d5Unit) : lcd_mesh(history,co2Unit);
      screen_drawSequence(&sequence);

    }
  }
}

#endif

#if 0
enum lcd_render_menu_bits_t {
  LCD_RENDER_MENU_BIT_static,
  LCD_RENDER_MENU_BIT_background,
  LCD_RENDER_MENU_BIT_itemCursor,
  LCD_RENDER_MENU_BIT_itemOther,
};
static void renderMenu(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];
  uint16_t cursorColor = themeColors[LCD_THEME_COLOR_cursor];
  //title
  if (updateBits & (1<<LCD_RENDER_MENU_BIT_static)){
    screen_rect_t titleRect = {30,5 + lcd_const(titleHeight),180,36};
    const screen_mesh_t* meshs[10] = {0};
    screen_sequence_t sequence = {&titleRect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
    meshs[0] = lcd_meshL(settings, menuTitleLabel);
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);
  }

  //navigate
  const screen_mesh_t* navigateMeshs[] = {lcd_meshL(navigate,back),lcd_meshL(navigate,submit),lcd_meshL(navigate,switch)};
  renderNavigate(navigateMeshs, frontColor, primaryColor);
  //menu
  if (updateBits & ((1<<LCD_RENDER_MENU_BIT_itemCursor) | (1<<LCD_RENDER_MENU_BIT_itemOther))){
    screen_rect_t menuRect = {17,43 + lcd_const(titleHeight),206,206};
    uint16_t meshColors[10];
    uint16_t backColors[10];
    uint8_t updateBitArray[2] = {0};
    int8_t size = lcd_get(settingsMenuSize);
    int8_t cursorIndex = lcd_get(settingsMenuCursorIndex);
    int8_t lastCursorIndex = lcd_getC(settingsMenuCursorIndex);
    for (int8_t i = 0; i < size; i++) {
      meshColors[i] = frontColor;
      if (i == cursorIndex) {
        backColors[i] = cursorColor;
      } else {
        backColors[i] = backColor;
      }
      uint8_t updateCursor = (updateBits & (1<<LCD_RENDER_MENU_BIT_itemCursor))?1:0;
      if ((i == cursorIndex || i == lastCursorIndex) && updateCursor) {
        bit_set(updateBitArray,i);
      } else if (updateBits & (1<<LCD_RENDER_MENU_BIT_itemOther)) {
        bit_set(updateBitArray,i);
      }
    }
    //memset(updateBitArray,0xff,2);
    screen_menu_t menu = {&menuRect, SCREEN_ALIGN_LEFT,0,0,206,206, size, 7, 18, SCREEN_ALIGN_LEFT, 13, 0,
       lcd_meshsL(settings,menuLabels),size,
       meshColors, backColors, backColor,updateBitArray, (updateBits & (1<<LCD_RENDER_MENU_BIT_background))?1:0};
    screen_drawMenu(&menu);
  }
}

enum lcd_render_select_bits_t {
  LCD_RENDER_SELECT_BIT_static,
  LCD_RENDER_SELECT_BIT_background,
  LCD_RENDER_SELECT_BIT_itemCursor,
  LCD_RENDER_SELECT_BIT_itemSelected,
  LCD_RENDER_SELECT_BIT_itemOther,
};
static void renderSelect(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];
  uint16_t cursorColor = themeColors[LCD_THEME_COLOR_cursor];
  uint16_t selectedColor = themeColors[LCD_THEME_COLOR_selected];
  screen_rect_t rect = {0};
  const screen_mesh_t* meshs[10] = {0};
  // screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};

  int8_t menuIndex = lcd_get(settingsMenuCursorIndex);
  //title
  if (updateBits & (1<<LCD_RENDER_SELECT_BIT_static)){
    screen_rect_t titleRect = {30,5 + lcd_const(titleHeight),180,36};
    screen_sequence_t sequence = {&titleRect, SCREEN_ALIGN_CENTER, lcd_meshsL(settings,selectMainLabels) + menuIndex,0,0,1,frontColor,backColor};
    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = 1;
    screen_drawSequence(&sequence);
  }

  //navigate
  const screen_mesh_t* navigateMeshs[] = {lcd_meshL(navigate,back),lcd_meshL(navigate,submit),lcd_meshL(navigate,switch)};
  renderNavigate(navigateMeshs, frontColor, primaryColor);
  //menu
  if (updateBits & ((1<<LCD_RENDER_SELECT_BIT_itemCursor) | (1<<LCD_RENDER_SELECT_BIT_itemSelected) | (1<<LCD_RENDER_SELECT_BIT_itemOther))){
    const screen_mesh_t* const* optionMeshs = (const screen_mesh_t**)*((uint32_t*)lcd_meshssL(settings,selectOptionsLabels) + menuIndex);
    screen_rect_t menuRect = {17,43 + lcd_const(titleHeight),206,206};
    uint16_t meshColors[24];
    uint16_t backColors[24];
    uint8_t updateBitArray[4] = {0};
    int8_t size = lcd_get(settingsSelectSize);
    int8_t rowSize = lcd_get(settingsMenuSize);
    int8_t cursorIndex = lcd_get(settingsSelectCursorIndex);
    int8_t lastCursorIndex = lcd_getC(settingsSelectCursorIndex);
    int8_t selectedIndex = lcd_get(settingsSelectSelectedIndex);
    int8_t lastSelectedIndex = lcd_getC(settingsSelectSelectedIndex);
    for (int8_t i = 0; i < size; i++) {
      meshColors[i] = frontColor;
      if (i == cursorIndex & i== selectedIndex) {
        backColors[i] = overlayRGB(cursorColor, selectedColor, backColor);
      } else if (i == cursorIndex) {
        backColors[i] = cursorColor;
      } else if (i == selectedIndex) {
        backColors[i] = selectedColor;
      } else {
        backColors[i] = backColor;
      }
      uint8_t updateCursor = (updateBits & (1<<LCD_RENDER_SELECT_BIT_itemCursor))?1:0;
      uint8_t updateSelected = (updateBits & (1<<LCD_RENDER_SELECT_BIT_itemSelected))?1:0;
      if ((i == cursorIndex || i == lastCursorIndex) && updateCursor) {
        bit_set(updateBitArray,i);
      } else if ((i == selectedIndex || i == lastSelectedIndex) && updateSelected) {
        bit_set(updateBitArray,i);
      } else if (updateBits & (1<<LCD_RENDER_SELECT_BIT_itemOther)) {
        bit_set(updateBitArray,i);
      }
    }
    //memset(updateBitArray,0xff,2);
    screen_menu_t menu = {&menuRect, SCREEN_ALIGN_LEFT,0,0,206,206, size, 7, 18, SCREEN_ALIGN_LEFT, 13, 0,
       optionMeshs,rowSize,
       meshColors, backColors, backColor,updateBitArray, (updateBits & (1<<LCD_RENDER_MENU_BIT_background))?1:0};
    screen_drawMenu(&menu);
  }
}
#endif

enum lcd_render_qrcode_bits_t {
//  LCD_RENDER_MAIN_BIT_pm2d5Static,
//  LCD_RENDER_MAIN_BIT_pm2d5Data,
  LCD_RENDER_QRCODE_BIT_qr,
  LCD_RENDER_QRCODE_BIT_wifiStatus,
  LCD_RENDER_QRCODE_BIT_SIZE,
};
static uint8_t* g_qrBuffer;
static uint16_t g_qrScale;
static uint16_t g_qrFrontColor;
static uint16_t g_qrBackColor;
static uint16_t qrcode4D(int16_t x, int16_t y) {
  int16_t xs = (x * g_qrScale) >>8;
  int16_t ys = (y * g_qrScale) >>8;
  return qrcode_get(g_qrBuffer,xs,ys)?g_qrFrontColor:g_qrBackColor;
}
static void renderQrcode(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  const screen_mesh_t* meshs[10] = {0};
  //navigate
//  const screen_mesh_t* navigateMeshs[] = {lcd_mesh(navigate,back),0,0};
//  renderNavigate(navigateMeshs, frontColor, primaryColor);
  //image
	if (updateBits & (1<<LCD_RENDER_QRCODE_BIT_qr))
	{
		g_qrBuffer = (uint8_t*)lcd_get32(settingsQrBuffer);
		if (g_qrBuffer) {
			screen_rect_t qrRect = {70 + xOffset,70 + yOffset,104,104};
			screen_rect_t tipRect = {71 + xOffset,32 + yOffset,104,28};
			g_qrFrontColor = rgb(0,0,0);
			g_qrBackColor = backColor;
			screen_dynamic_t dynamic = {&qrRect, SCREEN_ALIGN_CENTER, 0, 0, (17+8*4)*2, (17+8*4)*2, qrcode4D, g_qrBackColor};
			g_qrScale = (256 + (2))/ 2;
			screen_drawDynamicMesh(&dynamic);
			//description
			//description
			screen_sequence_t sequence = {&tipRect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
			meshs[0] = lcd_mesh(settings, qrcodeTips1Label);
			screen_drawSequence(&sequence);
			
			

		}
//		if (lcd_get(language) == 0){
//			meshs[1] = lcd_mesh(settings, qrcodeTips2Label);
//			sequence.size ++;
//		}
    
  }
	if (updateBits & (1<<LCD_RENDER_QRCODE_BIT_wifiStatus))
	{
			screen_rect_t statusRect = {71+ xOffset,182 + yOffset,104,12};
			screen_sequence_t sequence = {&statusRect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
			
			
			esp8266_status_t wifiStatus = (esp8266_status_t)lcd_get(wifiStatus);
			if (wifiStatus == ESP8266_STATUS_smarting)
			{
				meshs[0] = lcd_mesh(settings, wifiStatus1Label);
			}
			else if ( wifiStatus == ESP8266_STATUS_getIP)
			{
				meshs[0] = lcd_mesh(settings, wifiStatus2Label);
			}
			else if (wifiStatus == ESP8266_STATUS_ok)
			{
				meshs[0] = lcd_mesh(settings, wifiStatus3Label);
			}
			else if (wifiStatus <= ESP8266_STATUS_connectWifi)
			{
				meshs[0] = lcd_mesh(settings, wifiStatus4Label);
			}
			else {
				screen_mesh_t dummyMesh = {SCREEN_MESH_FILL, 104, 12};
				meshs[0] = &dummyMesh;
				sequence.frontColor = backColor;

			}

			screen_drawSequence(&sequence);
  }
  //TODO
}

static uint8_t c2i(uint8_t v) {
  uint8_t index;
  if (v>= 'a') {
    index = 12+(v-'a');
  } else if (v>= 'A') {
    index = 12+(v-'A');
  } else if (v>= '0') {
    index = 0+(v-'0');
  } else {
    index = 11;
  }
  return index;
}
#if 0
static void renderInfo(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];
  screen_rect_t rect = {0};
  const screen_mesh_t* meshs[40] = {0};
  int8_t xoffsets[40] = {0};
  screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
  //title

  screen_rect_t titleRect = {30,5 + lcd_const(titleHeight),180,36};
  meshs[0] = lcd_meshL(settings, selectAboutMainLabel);
  sequence.rect = &titleRect;
  sequence.align = SCREEN_ALIGN_LEFT;
  sequence.size = 1;
  screen_drawSequence(&sequence);
  //lefts
  sequence.rect = &rect;
  rect.h = 10;
  rect.x = 30;
  rect.w = 90;
  sequence.align = SCREEN_ALIGN_LEFT;
  const uint8_t lines = 6;
  fors (lines) {
    rect.y = 48 + lcd_const(titleHeight) + i * 25;
    meshs[0] = *(lcd_meshsL(settings, selectAboutInfoLabel) + i);
    screen_drawSequence(&sequence);
  }
  //rights
  rect.h = 10;
  rect.x = 150;
  rect.w = 60;
  sequence.xoffsets = xoffsets;
  sequence.align = SCREEN_ALIGN_RIGHT;
  fors (40) {
    xoffsets[i] = 1;
  }
  fors (lines + 1) {
    rect.y = 48 + lcd_const(titleHeight) + i * 25;
    int8_t mIndex = 0;
    if (i == 2) {
      foris(j,3){
        screen_dec_t dec = {lcd_get(settingsVersion1 + j), lcd_meshs(number,6), 0, 1, 3};
        mIndex += screen_fetchDec(&dec, meshs+mIndex);
        if (j != 2) {
          meshs[mIndex++] = lcd_meshs(number,6)[10];
        }
      }
    } else if (i == 3) {
      uint8_t upgradePercent = lcd_get(settingsUpgradePercent);
      if (upgradePercent == 0xff) {
        meshs[mIndex++] = lcd_meshL(settings,noNewVersion);
      } else {
        screen_dec_t dec = {upgradePercent, lcd_meshs(number,6), 0, 1, 3};
        mIndex += screen_fetchDec(&dec, meshs+mIndex);
        meshs[mIndex++] = lcd_mesh(settings,updatePercentUnit);
      }
    } else if (i == 4) {
//      uint8_t pin = lcd_get(pin);
//      if (pin) {
//        char* str = (char*)lcd_get32(settingsImsi);
//        fortas(uint8_t, str, strlen(str)) {
//          meshs[mIndex++] = lcd_meshs(number,6)[c2i(v)];
//        }
//      } else {
//        meshs[mIndex++] = lcd_meshL(settings,noSIMCard);
//      }
    } else if (i == 5) {
      meshs[mIndex++] = lcd_mesh(settings, deviceIDHead);
      char* str = (char*)lcd_get32(settingsDeviceId);
      fors(16) {
        meshs[mIndex++] = lcd_meshs(number,6)[c2i(str[3+i])];
      }
      sequence.align = SCREEN_ALIGN_BOTTOMRIGHT;
    } else if (i == 6) {
      rect.y -= 10;
      sequence.align = SCREEN_ALIGN_BOTTOMRIGHT;
      char* str = (char*)lcd_get32(settingsDeviceId);
      fors(16) {
        meshs[mIndex++] = lcd_meshs(number,6)[c2i(str[3+i+16])];
      }
      sequence.align = SCREEN_ALIGN_BOTTOMRIGHT;
    } else {
      meshs[mIndex++] = *(lcd_meshsL(settings, selectAboutInfoValue) + i);
    }
    sequence.size = mIndex;
    screen_drawSequence(&sequence);
  }
  //navigate
  const screen_mesh_t* navigateMeshs[] = {lcd_meshL(navigate,back),0,0};
  renderNavigate(navigateMeshs, frontColor, primaryColor);
}


#endif
static void startNShut(int16_t alpha) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
//  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
//  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];

    const int16_t heights[] = {40,20,20,20};
    const int16_t spaces[] = {100,40,20,40};
    uint8_t index = 0;

    screen_rect_t rect = {20+xOffset,0+yOffset,200,0};
    const screen_mesh_t* meshs[10] = {0};
    screen_sequence_t sequence = {
      &rect,
      SCREEN_ALIGN_CENTER,
      meshs,
      0,
      0,
      1,
      rgb(255,0,0),
      backColor,
    };

    uint8_t colorBuffer[3];
    parseRBG(frontColor,colorBuffer);
    uint16_t color = color_mix(frontColor, backColor, alpha);
    sequence.frontColor = color;
    // sequence.backColor = rgb(255,0,0);

    // banner
    rect.y += rect.h + spaces[index];
    rect.h = heights[index];
		rect.x = 72 + xOffset;
		rect.y = 59 + yOffset;
		rect.w = 98;
		rect.h = 14;		
		//sequence.frontColor = rgb(128,128,128);
    meshs[0] = lcd_mesh(boot,banner);
    screen_drawSequence(&sequence);


		rect.x = 82 + xOffset;
		rect.y = 90 + yOffset;
		rect.w = 60;
		rect.h = 50;
		
		screen_dec_t dec = {lcd_get(ch2oPreTime), lcd_meshs(number,1), 0, 2, 2};
    uint8_t size = screen_fetchDec(&dec,meshs);

    sequence.align = SCREEN_ALIGN_LEFT;
    sequence.size = size;
    int8_t xoffsets[10] = {0};
    fors (10) {
      xoffsets[i] = 2;
    }
    xoffsets[0] = 0;
    sequence.xoffsets = xoffsets;
    screen_drawSequence(&sequence);
		
	
		rect.x = 20 + xOffset;
		rect.y = 161 + yOffset;
		rect.w = 196;
		rect.h = 15;
		sequence.align = SCREEN_ALIGN_CENTER;
		//sequence.frontColor = rgb(128,128,128);
//		static uint8_t gernerated = 0;
		static uint8_t randIndex = 0xff; 
		if (randIndex == 0xff)
		{
			srand((ADC1->DR) + SysTick->VAL);
			randIndex = rand()&0x07;
		}
			sequence.frontColor = rgb(32,32,32);
//			int8_t yoffsets[2] = {0};
			meshs[0] = lcd_meshs(boot,ch2oTips)[randIndex<<1];
//			meshs[1] = lcd_meshs(boot,ch2oTips)[(randIndex<<1)+1];
//			yoffsets[1] = 3;
//			sequence.yoffsets = yoffsets;
			sequence.size = 1;
			screen_drawSequence(&sequence);			
//			meshs[0] = lcd_meshs(boot,ch2oTips)[randIndex<<1];
			rect.y += 20;
			meshs[0] = lcd_meshs(boot,ch2oTips)[(randIndex<<1)+1];
			screen_drawSequence(&sequence);	
//    // name

//    rect.y += rect.h + spaces[index];
//    rect.h = heights[index];
//    meshs[0] = lcd_mesh(boot,deviceNum); //TODO location
//    screen_drawSequence(&sequence);
//    index ++;

//    // company
//    rect.y += rect.h + spaces[index];
//    rect.h = heights[index];
//    meshs[0] = lcd_mesh(boot,company); //TODO location
//    screen_drawSequence(&sequence);
//    index ++;

    // rect.y += rect.h + spaces[index];
    // rect.h = heights[index];
    // screen_statubar_t bar = {&rect,0,0,0,160,5,percent,0,frontColor,frontColor,backColor,backColor};
    // screen_drawStatubar(&bar);
    // index ++;
}
static void renderStart(uint32_t updateBits) {
  uint32_t percent = (tick_ms() - lcd_get32(animationStartTime))/4;
  int16_t alpha = ((percent) * 5) >> 3;
  alpha = min(max(alpha, 0), 255);
  startNShut(alpha);
}
//static void renderShut(uint32_t updateBits) {
//  uint32_t percent = (tick_ms() - lcd_get32(animationStartTime))/4;
//  int16_t alpha = ((1000 - percent) * 3) >> 3;
//  alpha = min(max(alpha, 0), 255);
//  startNShut(alpha);
//}


static const screen_mesh_t* g_chargingMesh;
static uint16_t g_chargingFrontColor;
static uint16_t g_chargingBackColor;
static uint16_t g_chargingBackColor2;
static uint16_t g_chargingThresholdHeight;
//static uint16_t charging4D(int16_t x, int16_t y) {
//  int16_t xs = (x * 21) >> 8;
//  int16_t ys = (y * 32) >> 8;
//  return screen_meshAt(g_chargingMesh,ys,xs)?g_chargingFrontColor:g_chargingBackColor;
//}
static uint16_t charging4D(int16_t x, int16_t y) {
  int16_t xs = ((x) * 64) >> 8;
  int16_t ys = ((y) * 64) >> 8;
  return screen_meshAt(g_chargingMesh,xs,ys)?g_chargingFrontColor:g_chargingBackColor;
}

static void renderCharging(uint32_t updateBits) {

  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
//  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
//	uint16_t secondaryColor = rgb(0,240,1);
	
	uint8_t index = lcd_get(batteryIndex);
	//index = 30;
	
	const screen_mesh_t* meshs[6] = {0};
	#if 1
	screen_rect_t rect;
	
//	
//	    rect.x = 51+xOffset;
//    rect.y = 93+yOffset;
//    rect.w = 100;
//    rect.h = 50;
	
		rect.x = 0 + xOffset;
    rect.y = 0 + yOffset;
    rect.w = 240;
    rect.h = 240;
		
		g_chargingBackColor2 = rgb(0,240,1);
		g_chargingBackColor = backColor;
		g_chargingThresholdHeight = (100 - index)*240/100 + yOffset;
//		g_chargingThresholdHeight = 180;
    int8_t xoffsets[6] = {0};
    fors (6) {
      xoffsets[i] = 2;
    }
		screen_sequence_t sequence = {&rect, SCREEN_ALIGN_CENTER, meshs,0,0,1,frontColor,backColor};
    sequence.xoffsets = xoffsets;
    screen_dec_t dec = {lcd_get(batteryRealIndex), lcd_meshs(number, 1), 0, 2, 3};
    uint8_t size = screen_fetchDec(&dec,meshs);
		xoffsets[size] = 10;
		meshs[size++] = lcd_mesh(number,percent);
    sequence.align = SCREEN_ALIGN_CENTER;
    sequence.size = size;
    xoffsets[0] = 0;
//    yoffsets[0] = 0;
//		screen_dynamic_t dynamic = {&rect, SCREEN_ALIGN_CENTER, 0, 0, 96, 168, charging4D, backColor};
//		sequence.backColor = 
//    screen_drawSequence(&sequence);
		
		screen_drawSequenceDynamicBackground(&sequence,g_chargingBackColor2,g_chargingThresholdHeight);
	
//		rect.x = 158 + xOffset;
//		rect.y = 96 + yOffset;
//		rect.h = 47;
//		rect.w = 52;
//		meshs[0] = lcd_mesh(number,percent);
//		
//		sequence.align = SCREEN_ALIGN_LEFT;
//		sequence.size = 1;
//		sequence.xoffsets = 0;
//		
//		screen_drawSequenceDynamicBackground(&sequence,g_chargingBackColor2,g_chargingThresholdHeight);

		//screen_drawSequence(&sequence);
#endif
		
//  screen_rect_t rect = {51 + xOffset,93 + yOffset,100,50};

//  screen_dynamic_t dynamic = {&rect, SCREEN_ALIGN_CENTER, 0, 0, 96, 168, charging4D, backColor};

// // uint8_t index = lcd_get(batteryIndex);
//	screen_dec_t dec = {index, lcd_meshs(number,1), 0, 2, 3};
//  uint8_t size = screen_fetchDec(&dec,meshs);
//			
//  g_chargingMesh = meshs[0];

//  uint8_t alpha = lcd_get(chargingAlpha);

//  g_chargingFrontColor = color_mix(frontColor, backColor, alpha);
//  g_chargingBackColor = backColor;

//  screen_drawDynamicMesh(&dynamic);
}
static void renderLowPower(uint32_t updateBits) {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
//  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
//  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];

  screen_rect_t rect = {0 + xOffset, 0 + yOffset, 240,240};

  screen_dynamic_t dynamic = {&rect, SCREEN_ALIGN_CENTER, 0, 0, 25*4, 13*4, charging4D, backColor};

  g_chargingMesh = lcd_meshs(title, battery)[0];
  uint8_t alpha = lcd_get(chargingAlpha);
  g_chargingFrontColor = color_mix(frontColor, backColor, alpha);
  g_chargingBackColor = backColor;
  screen_drawDynamicMesh(&dynamic);
}
void lcd_setOffset(uint16_t rotation)
{
	//#if AIRJ_LCD_DISPLAY
	switch (rotation)
	{
		case 0:
			xOffset = LCD_LEFT_X;
			yOffset = LCD_TOP_Y;
			break;
		case 90:
			xOffset = 320-LCD_TOP_Y-LCD_CIRCLE_D;
			yOffset = LCD_LEFT_X;
			break;
		case 180:
			xOffset = 240-LCD_LEFT_X-LCD_CIRCLE_D;
			yOffset = 320-LCD_TOP_Y-LCD_CIRCLE_D;

			break;
		case 270:
			xOffset = LCD_TOP_Y;
			yOffset = 240-LCD_LEFT_X-LCD_CIRCLE_D;

			break;
		default:
			return;
	}
//	#endif
}
uint8_t lcd_setRotation(uint16_t degree)
{
	if (degree != g_lcd_rotation)
	{
		g_lcd_rotation = degree;
		screen_setDir(degree);
		lcd_setOffset(degree);
		return 1;
		
	}
	return 0;
}



void lcd_init() {
  g_lcd_data_cache[LCD_DATA_page] = 0xff;
  screen_init();
	lcd_setRotation(0);
	lcd_setOffset(g_lcd_rotation);
}

void lcd_update() {
  const uint16_t* themeColors = getThemeColors();
  uint16_t frontColor = themeColors[LCD_THEME_COLOR_font];
  uint16_t backColor = themeColors[LCD_THEME_COLOR_background];
  uint16_t primaryColor = themeColors[LCD_THEME_COLOR_primary];
  uint16_t secondaryColor = themeColors[LCD_THEME_COLOR_secondary];

  uint8_t pageChanged = 0;
  uint8_t themeChanged = 0;
  uint8_t directionChanged = 0;
  if (!lcd_cnc(page,1)) {
    pageChanged = 1;
  }
  if (!lcd_cnc(theme,1)) {
    themeChanged = 1;
  }
  if (!lcd_cnc(direction,1)) {
    directionChanged = 1;
  }
  uint8_t renderAll = pageChanged || themeChanged || directionChanged;
  if (renderAll) {
		//screen_fill_circle(120 + xOffset, 120 + yOffset, 120, backColor);
    screen_fill(0+xOffset,0+yOffset,240,240,backColor); // except statu bar on the top.
  }
  if (lcd_get(page)== LCD_PAGE_main || lcd_get(page)== LCD_PAGE_all || lcd_get(page)== LCD_PAGE_pm2d5) {
    if (renderAll) {
      renderTitle(UINT32_MAX);
    } else {
      uint32_t bits = 0;
//      if (!lcd_cnc(holding,1)) {
//        bits |= 1<<LCD_RENDER_TITLE_BIT_hold;
//      }
//      if (!lcd_cnc(time,4)) {
//        bits |= 1<<LCD_RENDER_TITLE_BIT_time;
//      }
      if (!lcd_cnc(wifiStatus,1)) {
        bits |= 1<<LCD_RENDER_TITLE_BIT_wifi;
      }
      if (!lcd_cnc(batteryBlend,1) || !lcd_cnc(batteryCharging,1) || !lcd_cnc(batteryIndex,1)) {
        bits |= 1<<LCD_RENDER_TITLE_BIT_battery;
      }
      renderTitle(bits);
    }
  } 
//	else {
//    if (renderAll) {
//			screen_fill(0+xOffset,0+yOffset,240,240,backColor); //statu bar on the top.
//    }
//  }
  switch (lcd_get(page)) {
    case LCD_PAGE_start:{
//      renderStart(UINT32_MAX);
      break;
		}
//     case LCD_PAGE_shut: {
//     // renderShut(UINT32_MAX);
//      break;
//    }
		case LCD_PAGE_low_power: {
			
      renderLowPower(UINT32_MAX);
      break;
    }
		case LCD_PAGE_battery: {
      if (!lcd_cnc(batteryBlend,1) || !lcd_cnc(batteryCharging,1) || !lcd_cnc(batteryIndex,1) || directionChanged || pageChanged) {
        renderCharging(UINT32_MAX);
      }
      break;
    } 
		case LCD_PAGE_main: {
      uint32_t bits = 0;
      if (renderAll) {
        bits = UINT32_MAX;
      }
      if (!lcd_cnc(valueCh2o,2)) {
        bits |= 1<<LCD_RENDER_MAIN_BIT_ch2oData;
      }
      if (!lcd_cnc(valueTemperature,2) || !lcd_cnc(valueHumidity,2)) {
        bits |= 1<<LCD_RENDER_MAIN_BIT_minorData;
      }
      if (bits) {
        renderMain(bits);
      }
      break;
		}
		case LCD_PAGE_all: {
			
			lcd_set(theme, 1);//black
      uint32_t bits = 0;
      if (renderAll) {
        bits = UINT32_MAX;
      }
      if (!lcd_cnc(valueCh2o,2)) {
        bits |= 1<<LCD_RENDER_MAIN_BIT_ch2oData;
      }
      if (!lcd_cnc(valueTemperature,2) || !lcd_cnc(valueHumidity,2)) {
        bits |= 1<<LCD_RENDER_MAIN_BIT_minorData;
      }
      if (bits) {
        renderMain2(bits);
      }
      break;
		}
		case LCD_PAGE_pm2d5: {
			
			lcd_set(theme, 1);//black
      uint32_t bits = 0;
      if (renderAll) {
        bits = UINT32_MAX;
      }
      if (!lcd_cnc(valueCh2o,2)) {
        bits |= 1<<LCD_RENDER_MAIN_BIT_ch2oData;
      }
      if (!lcd_cnc(valueTemperature,2) || !lcd_cnc(valueHumidity,2)) {
        bits |= 1<<LCD_RENDER_MAIN_BIT_minorData;
      }
      if (bits) {
        renderPm2d5(bits);
      }
      break;
		}
		
			case LCD_PAGE_settings_qrcode: {
			uint32_t bits = 0;

			if (renderAll)
			{
				bits = UINT32_MAX;
			}
			if (!lcd_cnc(wifiStatus,1))
			{
				 bits |= 1<<LCD_RENDER_QRCODE_BIT_wifiStatus;
			}
			renderQrcode(UINT32_MAX);
      break;
			} 
//			case LCD_PAGE_settings_info: {
//      if (!lcd_cnc(settingsUpgradePercent,1) || renderAll) {
//        renderInfo(UINT32_MAX);
//      }
//      break;
//    }
  }
}
