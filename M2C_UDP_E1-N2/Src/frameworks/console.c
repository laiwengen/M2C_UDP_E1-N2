#include "stdlib.h"
#include "string.h"
#include "frameworks/strings.h"
#include "frameworks/list.h"
#include "frameworks/stream.h"
#include "frameworks/console.h"
#include "frameworks/schedule.h"
static list_t* g_head = 0;

static console_t* stream2console(stream_t* stream) {
	fortl(console_t*, &g_head) {
		if (v->inStream == stream) {
			return v;
		}
	}
	return 0;
}

console_listener_t* console_addListener(console_t* console, const char* command, console_listenerFunction_t function, void* params) {
	console_listener_t* cl = (console_listener_t*)malloc(sizeof(console_listener_t));
	if (cl) {
		cl->command = command;
		cl->function = function;
		cl->params = params;
		if (!list_add0(&console->listeners,cl)){
			free (cl);
			return 0;
		}
	}
	return cl;
}

static uint8_t removeListenerIfCommandMatch(void* ptr, void** params) {
	console_listener_t* listener = (console_listener_t*) ptr;
	const char* command = *((char**) params);
	return strcasecmp(listener->command, command) == 0;
}

void console_removeListener(console_t* console, const char* command) {
	void* ptr = list_removeIf(&console->listeners, removeListenerIfCommandMatch, (void**)&command);
	free(ptr);
}

static void lineFunction(void ** ps) {
	stream_t* stream = (stream_t*)ps[0];
	uint8_t* buffer = (uint8_t*)ps[2];
	int16_t size = (int32_t)ps[3];
	console_t* console = stream2console(stream);
	size = string_removeBackspace((char*)buffer,size);
	char const* command;
	char const* params[10];
	int16_t tokenCount = string_parseAT((char*)buffer, size, &command, params);
	uint8_t match = 0;
	if (tokenCount > 0){
		if (command) {
			fortl(console_listener_t*, &console->listeners) {
				if (v->command && strcasecmp(command,v->command) == 0) {
					if (v->function(v, command, params, tokenCount-1)) {
						match = 1;
					}
				}
			}
		}
		free((void*)command);
		for (int16_t i = 0; i<tokenCount-1; i++) {
			free((void*)params[i]);
		}
	}
	free(buffer);
	free(ps);
	if (match) {
//		console_print(console, "at+ok\n");
	} else {
//		console_print(console, "at+error\n");
	}
}

static void console_streamLineFunction(stream_t* stream, stream_listener_t* listener, uint8_t* buffer, int16_t size) {
	void** params = (void**) malloc(4*4);
	if (params) {
		params[0] = stream;
		params[1] = listener;
		params[2] = string_duplicate((char*)buffer,size);
		params[3] = (void*)size;
		schedule_once(0,lineFunction,5,params);
	}
}

static void console_streamEchoFunction(stream_t* stream, stream_listener_t* listener, uint8_t* buffer, int16_t size) {
	console_t* console = stream2console(stream);
	stream_write(console->outStream,buffer,size);
	stream_flush(console->outStream);
}
static stream_listener_t g_console_lineListener = {
	STREAM_LISTENER_TYPE_endByte,
	console_streamLineFunction,
	{.endWith = '\n'}
};

static stream_listener_t g_console_echoListener = {
	STREAM_LISTENER_TYPE_peek,
	console_streamEchoFunction
};

void console_init(console_t* console) {
	list_add0(&g_head, console);
	console_hw_init(console);
	stream_add(console->inStream);
	stream_add(console->outStream);
	stream_addListener(console->inStream, &g_console_lineListener);
	stream_addListener(console->inStream, &g_console_echoListener);
}

void console_print(console_t* console, const char* str) {
	stream_write(console->outStream,(uint8_t*)str,strlen(str));
	stream_flush(console->outStream);
}

void console_out(console_t* console, uint8_t* str) {
	stream_write(console->outStream,(uint8_t*)str,1);
	stream_flush(console->outStream);
}
