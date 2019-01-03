#ifndef CONSOLE_H__
#define CONSOLE_H__
#include "stdint.h"
#include "frameworks/stream.h"
typedef struct console_listener_t console_listener_t;
typedef struct console_t console_t;

typedef uint8_t(*console_listenerFunction_t)(console_listener_t* listener, char const* command, char const** params, int16_t paramCount);

struct console_listener_t {
	const char* command;
	console_listenerFunction_t function;
	void* params;
};

struct console_t {
	stream_t* outStream;
	stream_t* inStream;
	uint8_t echo;
	list_t* listeners;
};

#include "stdlib.h"
#include "string.h"
#include "frameworks/strings.h"
#include "frameworks/list.h"
#include "frameworks/stream.h"
#include "frameworks/console.h"

console_listener_t* console_addListener(console_t* console, const char* command, console_listenerFunction_t function, void* params);

void console_removeListener(console_t* console, const char* command);
void console_init(console_t* console);

void console_print(console_t* console, const char* str);
void console_hw_init(console_t* console);

#endif
