
#ifndef ESP8266_H_
#define ESP8266_H_

#include "stdint.h"
#include "stm32f0xx_hal.h"
#include "frameworks/list.h"
#include "frameworks/schedule.h"
#include "frameworks/stream.h"
#include "frameworks/wireless.h"

#define ESP8266_RESET_SCHEDULE_ID 0x54cd8266
#define MAIN_ESP8266_AUTO_RESET_SCHEDULE_ID 0xfec08266
typedef struct esp8266_t esp8266_t;
typedef void(*esp8266_sendingFuntion_t)(esp8266_t* esp8266,wireless_socket_t* socket,uint8_t* bytes,int16_t size);
typedef void(*esp8266_receivedFunction_t)(esp8266_t* esp8266, wireless_socket_t* socket, uint8_t* buffer, int16_t size);
typedef void(*esp8266_receivePackageFunction_t)(wireless_socket_t* socket, char* package);
typedef uint8_t(*esp8266_listenerFunction_t)(uint8_t* bytes, int16_t size,void** params);
typedef void(*esp8266_smartLinkFunction_t)(esp8266_t* esp8266);
//typedef void(*esp8266_comandFunction_t)(void* params);
typedef uint8_t(*esp8266_comandFunction_t)(void** commonParams, void** indivParams);


typedef struct esp8266_listener_t {
	uint32_t id;
	esp8266_listenerFunction_t function;
	void* params;
} esp8266_listener_t;

typedef struct esp8266_command_t {
	uint8_t* bytes;
	uint16_t size;
	uint16_t timeout;
	uint16_t retries;
	esp8266_comandFunction_t function;
	void* params;
} esp8266_command_t;
typedef struct esp8266_httpRequest_t {
	char* path;
	uint8_t method;
	list_t* headers;
	uint8_t* body;
	int16_t size;
} esp8266_httpRequest_t;
typedef enum esp8266_status_t {
	ESP8266_STATUS_starting,
	ESP8266_STATUS_initOK,
	ESP8266_STATUS_smarting,
	ESP8266_STATUS_connectWifi,
	ESP8266_STATUS_getIP,
	ESP8266_STATUS_connectServer,
	ESP8266_STATUS_ok,
	ESP8266_STATUS_SIZE,
}esp8266_status_t;
typedef struct esp8266_t {
	stream_t* outStream;
	stream_t* inStream;
	list_t* listeners;
	char* wifiSsid;
	esp8266_status_t status;
	uint32_t smartNoApTick;
//	uint32_t resetForCrashTick;
	esp8266_smartLinkFunction_t smartLinkFunction;
	esp8266_smartLinkFunction_t ipChangedFunction;
	esp8266_sendingFuntion_t sendingFunction;
	esp8266_receivedFunction_t receivedFunction;
	list_t* commands;
	list_t* sockets;
} esp8266_t;
void esp8266_init(esp8266_t* esp8266);
void esp8266_deinit(esp8266_t* esp8266);
void esp8266_smartStop(void);
void esp8266_smart(uint32_t lastingMs);
void esp8266_hw_init(void);
void esp8266_hw_powerOff(void** params);
esp8266_status_t esp8266_getStatus(void);
uint8_t esp8266_ready(void);
void esp8266_hw_reset(void);
uint8_t esp8266_hw_isPowerOn(void);
uint8_t esp8266_send(uint8_t type, char const* host, uint16_t port, uint8_t* buffer, uint16_t size);
void flushEsp8266TransmitBuffer(void);
uint8_t esp8266_common4R(uint8_t* buffer, int16_t size);
void esp8266_command(const char* cmd, uint16_t timeout, uint16_t retries, esp8266_comandFunction_t function, void* params);
void esp8266_reset(void** params);
uint8_t esp8266_setStatus(esp8266_status_t status);
void esp8266_delayStatusChecker(void** params);
#endif
