#ifndef __MAINCONFIG_H
#define __MAINCONFIG_H

#define SYSTEM_ATUOPOFF_MS (600000) //10min
#define RESET_WHILE_NO_DATEGET 60000

#define PM_LOW_MODIFED 0
#define M1_MODE 0
#define HCHO_MAX 6250
#if (HCHO_MAX == 2000)
#define HCHO_LOW_FIX 0
#else
#define HCHO_LOW_FIX 1
#endif
#define HCHO_FIX_INDEX 0.4
#define ENABLE_HCHO_STABLE 1
#define MAC2UID 1
#define USE_TEST_SERVER 0
#define UART_FIX_MODE 1
#define UART_FIX_HCHO 1
#define FPE_UID_ADDRESS 0x00110000
//#if MAC2UID  //@×¢ÒâºìÉ«N2 °æ±¾ºÅ
//#define DEVICE_VERSION "E3 1.0.1.0"
////#define DEVICE_VERSION "N2 2.0.0.7"
//#else
//#define DEVICE_VERSION "E3 1.0.0.7"
//#endif
//#define DEVICE_TYPE "E3"
#define FPE_NEVERUSED_ADDRESS 0x00001000
#define MAIN_STATUS_ADDRESS 0x00000100
#define PM_RATIO_ADDRESS 0x00000200
#define FPE_CH2O_GROUND 0x00243852
#define FPE_HCHO_RATIO_ADDRESS 0x00000300
//#define MIN(a,b) (((a)<(b))?(a):(b))
//#define MAX(a,b) (((a)>(b))?(a):(b))
//#define ABS(a) (((a)>(0))?(a):(-a))

//#define LCD_DRIVER_ST7789
#define UDP_BORADCAST 0
#define NEW_M2_PCB 0
enum STATUS_ID
{
	STATUS_ALL_SHOW = 0,	
//	STATUS_OUTDOOR,
	STATUS_SINGLE_SHOW,
	STATUS_HISTORY,
	STATUS_WIFI ,
	STATUS_POWEROFF,
};
#if MAC2UID
#define QR_VERSION_6 
#else
#define QR_VERSION_3
#endif

#ifdef QR_VERSION_6

	#define DATA_CODE_BYTE 136
	#define ALL_CODE_BYTE 172
	#define PIXES_PRE_ROW 41
	#define BYTE_PRE_ROW ((PIXES_PRE_ROW+7)>>3)
	#define FORMAT_INFO 0x08
	#define ERROR_CORRECT_BLOCK 2
	#if (ERROR_CORRECT_BLOCK>1)
		#define BLOCK1_ALL_CODE_BYTE 86
		#define BLOCK1_DATA_CODE_BYTE 68
	#endif
#endif
#ifdef QR_VERSION_3
//QR_VERSION 3
#define DATA_CODE_BYTE 55
#define ALL_CODE_BYTE 70
#define PIXES_PRE_ROW 29
#define BYTE_PRE_ROW ((PIXES_PRE_ROW+7)>>3)
#define FORMAT_INFO 0x08
#define ERROR_CORRECT_BLOCK 1
#endif
#ifdef QR_VERSION_5
//QR_VERSION 3
#define DATA_CODE_BYTE 108
#define ALL_CODE_BYTE 134
#define PIXES_PRE_ROW 37
#define BYTE_PRE_ROW ((PIXES_PRE_ROW+7)>>3)
#define FORMAT_INFO 0x08
#define ERROR_CORRECT_BLOCK 1
#endif

#define LCD_LEFT_X   0
#define LCD_TOP_Y   68
#define LCD_CIRCLE_D 240

#endif
