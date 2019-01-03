#ifndef SIM800_H__
#define SIM800_H__
#include "stdint.h"
#include "frameworks/list.h"
#include "frameworks/stream.h"
#include "frameworks/wireless.h"
typedef struct sim800_t sim800_t;
typedef void(*sim800_receivePackageFunction_t)(wireless_socket_t* socket, char* package);
typedef uint8_t(*sim800_listenerFunction_t)(uint8_t* bytes, int16_t size, void** params);
typedef uint8_t(*sim800_comandFunction_t)(void** commonParams, void** indivParams);
typedef void(*sim800_sendingFunction_t)(sim800_t* sim800, wireless_socket_t* socket, uint8_t* buffer, int16_t size);
typedef void(*sim800_receivedFunction_t)(sim800_t* sim800, wireless_socket_t* socket, uint8_t* buffer, int16_t size);
typedef void(*sim800_rssiFunction_t)(sim800_t* sim800, int16_t rssi);
typedef void(*sim800_pinFunction_t)(sim800_t* sim800, uint8_t pin);

enum sim800_status_t {
	SIM800_STATUS_starting,
	SIM800_STATUS_cpin,
	SIM800_STATUS_csq,
	SIM800_STATUS_creg,
	SIM800_STATUS_cgatt,
	SIM800_STATUS_cifsr,
	SIM800_STATUS_ok,
	SIM800_STATUS_SIZE,
};

typedef struct sim800_t {
	stream_t* outStream;
	stream_t* inStream;
	list_t* listeners;
	list_t* commands;
	list_t* sockets;
	uint8_t status;
	int8_t rssi;
	uint8_t pin;
	uint8_t apnStarted;
	sim800_sendingFunction_t sendingFunction;
	sim800_receivedFunction_t receivedFunction;
	sim800_rssiFunction_t rssiFunction;
	sim800_pinFunction_t pinFunction;
} sim800_t;

typedef struct sim800_listener_t {
	uint32_t id;
	sim800_listenerFunction_t function;
	void* params;
} sim800_listener_t;

typedef struct sim800_command_t {
	uint8_t* bytes;
	uint16_t size;
	uint16_t timeout;
	uint16_t retries;
	sim800_comandFunction_t function;
	void* params;
} sim800_command_t;

// void sim800_hw_initDMA(stream_t* stream);
// void sim800_hw_initFlushFuntion(stream_t* stream);
uint8_t sim800_common4R(uint8_t* buffer, int16_t size);
void sim800_command(const char* cmd, uint16_t timeout, uint16_t retries, sim800_comandFunction_t function, void** params);

void sim800_hw_init(void);

uint8_t sim800_hw_isPowerOn(void);
uint8_t sim800_hw_isIniting(void) ;

void sim800_hw_powerOn(void);
void sim800_hw_reset(void);
void sim800_hw_powerOff(void);

void sim800_init(sim800_t* sim800);
uint8_t sim800_ready(void);

uint8_t sim800_send(uint8_t type, char const* host, uint16_t port, uint8_t* buffer, uint16_t size);

#endif
