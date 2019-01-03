#ifndef LEDSCREEN_H__
#define LEDSCREEN_H__

#include "stdint.h"

typedef struct ledScreen_package_t ledScreen_package_t;

struct ledScreen_package_t {
	struct ledScreen_packageHead_t{
		uint16_t descAddress;
		uint16_t sourceAddress;
		uint32_t reserved0;
		uint8_t checkMode;
		uint8_t displayMode;
		uint8_t deviceType;
		uint8_t protocolVersion;
		uint16_t dataLength;
	}__attribute__((packed))head;
	struct ledScreen_packageInfo_t{
		uint8_t commandGroup;
		uint8_t command;
		uint8_t response;
		uint8_t processMode;
		uint8_t reserved0;
		uint8_t deleteAreaNumber;
		uint8_t areaNumber;
		uint16_t areaDataLength;//
		uint8_t areaType;
		uint16_t areaX;
		uint16_t areaY;
		uint16_t areaWidth;
		uint16_t areaHeight;
		uint8_t dynamicAreaLoc;
		uint8_t lineSize;
		uint8_t runmode;
		uint16_t timeout;
		uint8_t soundMode;
		uint16_t reserved1;
		uint8_t singleLine;
		uint8_t newLine;
		uint8_t displayModeEx;
		uint8_t exitMode;
		uint8_t speed;
		uint8_t stayTime;
		uint32_t dataLength;
	}__attribute__((packed))info;
	uint8_t* data;
}__attribute__((packed));


void ledScreen_initPackage(struct ledScreen_package_t* pacakge);
void ledScreen_setData(ledScreen_package_t* pacakge, uint8_t* data, uint32_t size);

uint32_t ledScreen_seralizeSize(struct ledScreen_package_t * pacakge);
uint32_t ledScreen_seralize(struct ledScreen_package_t* package, uint8_t* out);

uint8_t ledScreen_display(struct ledScreen_package_t* pacakge);

#endif
