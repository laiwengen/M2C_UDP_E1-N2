#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "frameworks/strings.h"
#include "frameworks/number.h"
#include "frameworks/schedule.h"
#include "frameworks/esp8266.h"

// defines
#define ESP8266_RSSI_CHECKER_SCHEDULE_ID 0xf93201ae
#define ESP8266_STATUS_CHECKER_SCHEDULE_ID 0x4ccd3725
#define ESP8266_RESET_SCHEDULE_ID 0x4cde5375
#define ESP8266_LISTERNER_SCHEDULE_ID 0x6c888a9d
#define ESP8266_SEND_TOP_SCHEDULE_ID 0x4d33485f
#define ESP8266_STARTUP_SCHEDULE_ID 0xadb3584b

enum esp8266_socket_type_t {
	ESP8266_SOCKET_TCP,
	ESP8266_SOCKET_UDP
};

// consts


char const * const g_esp8266_statusCheckCommands[] = {
	"AT+CWMODE_DEF?\n",
	"AT+CSQ\n",
	"AT+CREG?\n",
	"AT+CGATT?\n",
	"AT+CIFSR\n",
};
// declears

void esp8266_stream4D(stream_t* stream, stream_listener_t* listener, uint8_t* buffer, int16_t size);
uint8_t sime800_postsend4R(void** bytes, void** params);
void esp8266_startup4D(void** params);

// globals
esp8266_t* g_esp8266_main;

stream_listener_t g_esp8266_lineListener = {
	STREAM_LISTENER_TYPE_endByte,
	esp8266_stream4D,
	{.endWith = '\n'},
	{0}
};
stream_listener_t g_esp8266_tickListener = {
	STREAM_LISTENER_TYPE_tick,
	esp8266_stream4D,
	{.ticks = 100},
	{0}
};
// statis functions
static uint8_t setStatus(uint8_t status) {
	esp8266_t* esp8266 = g_esp8266_main;
	if (esp8266->status != status) {
		if (status < esp8266->status) {
			if (esp8266->status > ESP8266_STATUS_cgatt && status <= ESP8266_STATUS_cgatt) {
				esp8266->apnStarted = 0;
			}
		}
		esp8266->status = status;
		return 1;
	}
	return 0;
}

static uint8_t incStatus(uint8_t status, uint8_t to) {
	return max(status, to);
}

static uint8_t decStatus(uint8_t status, uint8_t to) {
	return min(status, to);
}

static uint8_t paramMatch(const char* str, int16_t index, const char* toMatch) {
	char** out;
	int16_t size = string_split(str, ',', &out);
	uint8_t match = 0;
	if (size > index) {
		char* t = string_trim(out[index]);
		if (t) {
			if (string_equal(t, toMatch)) {
				match = 1;
			}
			free(t);
		}
	}
	for (int16_t i = 0; i < size; i++) {
		free(out[i]);
	}
	free(out);
	return match;
}

static int16_t parseDecs(const char* str, char splitter, int32_t** outDec) {
	char** out;
	int16_t size = string_split(str, splitter, &out);
	*outDec = 0;
	if (size) {
		int32_t* buffer = (int32_t*) malloc(size*sizeof(int32_t));
		if (buffer) {
			*outDec = buffer;
			int8_t dp;
			for (int16_t i = 0; i < size; i++) {
				char* t = string_trim(out[i]);
				if (t) {
					number_fromDecString(t, strlen(t), &buffer[i], &dp);
					free(t);
				}
			}
		}
	}
	for (int16_t i = 0; i < size; i++) {
		free(out[i]);
	}
	free(out);
	return size;
}

static void doSend(uint8_t* buffer, int16_t size) {
	esp8266_t* esp8266 = g_esp8266_main;
	stream_write(esp8266->outStream, buffer, size);
	stream_flush(esp8266->outStream);
}

static void deleteCommand(int16_t index) {
	esp8266_t* esp8266 = g_esp8266_main;
	esp8266_command_t* c = (esp8266_command_t*)list_removeByIndex(&esp8266->commands, index);
	if (c) {
		free(c->bytes);
		free(c);
	}
}
static void sendTop(void** params) { // params is not used
	esp8266_t* esp8266 = g_esp8266_main;
	if (list_size(&esp8266->commands) > 0) {
		esp8266_command_t* c = (esp8266_command_t*)list_peekFirst(&esp8266->commands);
		if (c->retries > 0) {
			c->retries--;
			doSend(c->bytes, c->size);
			if (c->function == sime800_postsend4R) { // TODO
				if (esp8266->functions.sending) {
					esp8266->functions.sending(esp8266, c->params, c->bytes, c->size);
				}
			}
			schedule_once(ESP8266_SEND_TOP_SCHEDULE_ID, sendTop, c->timeout, params);
		} else {
			//drop it
			deleteCommand(0);
			sendTop(params);
		}
	}
}

static void onCommandReceived(void** params) {
	schedule_cancel(ESP8266_SEND_TOP_SCHEDULE_ID);
	// esp8266_t* esp8266 = g_esp8266_main;
	uint8_t error = (uint32_t) params;
	if (!error) {
		deleteCommand(0);
	}
	sendTop(0);
}

static void sendCommand(esp8266_command_t* c) {
	esp8266_t* esp8266 = g_esp8266_main;
	list_t* cl = (list_t*)list_peekLast(&esp8266->commands);
	list_add(&esp8266->commands, c);
	if (list_size(&esp8266->commands) == 1) {
		sendTop(0);
	}
}

static void reset(void** params) {
	esp8266_t* esp8266 = g_esp8266_main;
	esp8266->rssi = -114;
	if (esp8266->functions.rssi) {
		esp8266->functions.rssi(esp8266, esp8266->rssi);
	}
	schedule_cancel(ESP8266_RSSI_CHECKER_SCHEDULE_ID);
	schedule_cancel(ESP8266_STATUS_CHECKER_SCHEDULE_ID);
	schedule_cancel(ESP8266_RESET_SCHEDULE_ID);
	schedule_cancel(ESP8266_SEND_TOP_SCHEDULE_ID);
	schedule_cancel(ESP8266_STARTUP_SCHEDULE_ID);
	schedule_once(ESP8266_STARTUP_SCHEDULE_ID, esp8266_startup4D, 2000, 0);
	schedule_once(ESP8266_RESET_SCHEDULE_ID, reset,30000,0);
	setStatus(0);
	while (list_size(&esp8266->commands) > 0) {
		deleteCommand(0);
	}
	while (list_size(&esp8266->sockets) > 0) {
		wireless_socket_t* socket = list_removeFirst(&esp8266->sockets);
		if (socket) {
			free(socket->host);
			free(socket);
		}
	}
	esp8266_hw_reset(esp8266);
}
// 4Rs

uint8_t esp8266_common4R(uint8_t* buffer, int16_t size) {
	static int8_t errorCount = 0;
	if (string_startWith((const char*)buffer, "OK")) {
		errorCount = 0;
		return 1;
	} else if (string_startWith((char*)buffer, "ERROR")) {
		if (errorCount > 8) {
			errorCount = 0;
			schedule_once(ESP8266_RESET_SCHEDULE_ID,reset,0,0);
		} else {
			errorCount ++;
		}
		return 2;
	}
	return 0;
}

uint8_t sime800_connected4R(void** bytes, void** params) {
	// esp8266_t* esp8266 = g_esp8266_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	wireless_socket_t* socket = (wireless_socket_t*)params;
	if (socket) {
		char* line = string_duplicate((char const*)buffer, size);
		if (line) {
			uint8_t match = paramMatch(line, 1, "CONNECT OK") || paramMatch(line, 1, "ALREADY CONNECT");
			free(line);
			if (match) {
				socket->connected = 1;
				return 1;
			}
		}
		if (socket->type == ESP8266_SOCKET_UDP) {
			if (string_startWith((const char*)buffer, "OK")) {
				socket->connected = 1;
				return 1;
			}
		}
	}
	return esp8266_common4R(buffer, size);
}

uint8_t sime800_presend4R(void** bytes, void** params) {
	// esp8266_t* esp8266 = g_esp8266_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	if (string_startWith((const char*)buffer, "> ") && size == 2) {
		return 1;
	}
	uint8_t done = esp8266_common4R(buffer, size);
	if (done & 2) {
		deleteCommand(1);
	}
	return done;
}

uint8_t sime800_postsend4R(void** bytes, void** params) {
	// esp8266_t* esp8266 = g_esp8266_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	char* line = string_duplicate((char const*)buffer, size);
	if (line) {
		uint8_t match = paramMatch(line, 1, "SEND OK");
		free(line);
		if (match) {
			return 1;
		}
	}
	return esp8266_common4R(buffer, size);
}

uint8_t esp8266_close4R(void** bytes, void** params) {
	// esp8266_t* esp8266 = g_esp8266_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	char* line = string_duplicate((char const*)buffer, size);
	if (line) {
		uint8_t match = paramMatch(line, 1, "CLOSE OK");
		free(line);
		if (match) {
			return 1;
		}
	}
	return esp8266_common4R(buffer, size);
}

uint8_t sime800_ip4R(void** bytes, void** params) {
	esp8266_t* esp8266 = g_esp8266_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	char* line = string_duplicate((char const*)buffer, size);
	if (line) {
		char* t = string_trim(line);
		free(line);
		if (t) {
			int32_t* decs;
			int16_t decCount = parseDecs(t, '.', &decs);
			free(t);
			free(decs);
			if (decCount == 4) {
				setStatus(incStatus(esp8266->status,  ESP8266_STATUS_cifsr + 1));
				return 1;
			}
		}
	}
	return esp8266_common4R(buffer, size);
}

uint8_t sime800_rssi4R(void** bytes, void** params) {
	esp8266_t* esp8266 = g_esp8266_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	char* line = string_duplicate((char const*)buffer, size);
	uint8_t match = 0;
	if (line) {
		char** out;
		int16_t size = string_split(line, ':', &out);
		if (size > 1) {
			char* command = string_trim(out[0]);
			char* code = string_trim(out[1]);
			if (command && code) {
				if (string_equal(command, "+CSQ")) {
					match = 1;
					int32_t* decs;
					int16_t decCount = parseDecs(code, ',', &decs);
					if (decCount > 0) {
						if (decs[0] != 99) {
							int8_t rssi = -114 + decs[0] * 2;
							if (esp8266->rssi != rssi ){
								esp8266->rssi = rssi;
								if (functions.rssi) {
									esp8266->functions.rssi(esp8266, esp8266->rssi);
								}
							}
						}
						free(decs);
					}
				}
			}
			free(command);
			free(code);
		}
		for (int16_t i = 0; i < size; i++) {
			free(out[i]);
		}
		free(out);
		free(line);
	}
	if (match) {
		return 1;
	}
	return esp8266_common4R(buffer, size);
}

// sends
void esp8266_command(const char* cmd, uint16_t timeout, uint16_t retries, esp8266_comandFunction_t function, void** params) {
	esp8266_command_t* c = (esp8266_command_t*)malloc(sizeof(esp8266_command_t));
	if (c) {
		c->bytes = (uint8_t*)string_duplicate(cmd, -1);
		c->size = strlen(cmd);
		c->timeout = timeout;
		c->retries = retries;
		c->function = function;
		c->params = params;
		sendCommand(c);
	}
}

uint8_t esp8266_ready(void) {
	esp8266_t* esp8266 = g_esp8266_main;
	if (esp8266->status != ESP8266_STATUS_ok) {
		return 0;
	}
	return 1;
}

uint8_t esp8266_send(uint8_t type, char const* host, uint16_t port, uint8_t* buffer, uint16_t size) {
	esp8266_t* esp8266 = g_esp8266_main;
	if (esp8266->status != ESP8266_STATUS_ok) {
		return 0;
	}
	if (list_size(&esp8266->commands) > 10) {
		return 0;
	}
	int8_t socketCount = list_size(&esp8266->sockets);
	wireless_socket_t* socket = 0;
	char idBuffer[2];
	// find if socket alread exists
	list_t* l = esp8266->sockets;
	while (l) {
		if (l->this.raw) {
			wireless_socket_t* s = (wireless_socket_t*) l->this.raw;
			if (s->type == type && string_equal(host, s->host) && port == s->port) {
				socket = s;
				break;
			}
		}
		l = l->next;
	}
	uint8_t needDisconnect = 0;
	if (socket == 0) {
		// start a new one if not exists
		if (socketCount < 6) {
			socket = (wireless_socket_t*) malloc(sizeof(wireless_socket_t));
			if (socket) {
				socket->id = socketCount;
			}
		} else {
			socket = list_removeFirst(&esp8266->sockets);
			if (socket) {
				if (socket->connected) {
					needDisconnect = 1;
				}
				free(socket->host);
			}
		}
		if (socket) {
			socket->connected = 0;
		}
	}
	// init id string
	if (socket) {
		number_toDecString(socket->id, 0, idBuffer, sizeof(idBuffer));
	}
	// fill id buffer for future use
	if (socket) {
		if (needDisconnect) {
			char const* concats[] = {"AT+CIPCLOSE=", idBuffer, "\n"};
			char* str = string_concat(concats, sizeof(concats) / sizeof(char*));
			if (str) {
				esp8266_command(str, 200, 2, esp8266_close4R, 0);
				free(str);
			}
		}
	}
	//TODO add setuped for UDP?
	// check if connected
	// even if udp do not need the CONNECT. but this just make sure it has sent the CIPSTART for host and port.
	if (socket && socket->connected == 0) {
		list_add(&esp8266->sockets, socket);
		socket->type = type;
		socket->host = string_duplicate(host, -1);
		socket->port = port;
		char portBuffer[6];
		number_toDecString(port, 0, portBuffer, sizeof(portBuffer));
		char const* concats[] = {"AT+CIPSTART=", idBuffer, ",\"", type == ESP8266_SOCKET_TCP ? "TCP" : "UDP", "\",\"", host, "\",\"", portBuffer, "\"\n"};
		char* str = string_concat(concats, sizeof(concats) / sizeof(char*));
		if (str) {
			// esp8266_command("AT+CGATT?\n",2000,5,0,0);
			esp8266_command(str, 2000, 1, sime800_connected4R, (void**)socket);
			free(str);
		}
	}
	// send data
	if (socket) {
		char lengthBuffer[6];
		number_toDecString(size, 0, lengthBuffer, sizeof(lengthBuffer));
		char const* concats[] = {"AT+CIPSEND=", idBuffer, ",", lengthBuffer, "\n"};
		char* str = string_concat(concats, sizeof(concats) / sizeof(char*));
		if (str) {
			// esp8266_command("AT+CGATT?\n",2000,5,0,0); //
			esp8266_command(str, 200, 1, sime800_presend4R, (void**)socket);
			free(str);
			esp8266_command_t* c = (esp8266_command_t*)malloc(sizeof(esp8266_command_t));
			uint8_t* bytes = (uint8_t*)malloc(size);
			if (c && bytes) {
				memcpy(bytes, buffer, size);
				c->bytes = bytes;
				c->size = size;
				c->timeout = 200;
				c->retries = 1;
				c->function = sime800_postsend4R;
				c->params = socket;
				sendCommand(c);
			} else {
				free(c);
				free(bytes);
			}
		}
	}
	return 0;
}

// schedule

void esp8266_rssiChecker(void** params) {
	esp8266_t* esp8266 = g_esp8266_main;
	if (esp8266->status > ESP8266_STATUS_csq) {
		if (list_size(&esp8266->commands) > 10) {
			return;
		}
		esp8266_command("AT+CSQ\n", 2000, 2, sime800_rssi4R, 0);
	}
}

void esp8266_statusChecker(void** params) {
	// static uint32_t index;
	// static int16_t pdt = 0;
	// esp8266_t* esp8266 = g_esp8266_main;
	// if (!esp8266_hw_isPowerOn()) {
	// 	if (pdt > 2) {
	// 		pdt = 0;
	// 		reset(0);
	// 	} else {
	// 		pdt ++;
	// 	}
	// 	return;
	// } else {
	// 	pdt = 0;
	// }
	// if (list_size(&esp8266->commands) > 10) {
	// 	return;
	// }
	// uint8_t status = esp8266->status;
	// if (status > 1) {
	// 	if (status != ESP8266_STATUS_ok || (index & 0x0f) == 0) {
	// 		esp8266_comandFunction_t function = 0;
	// 		if (status == ESP8266_STATUS_cifsr + 1) {
	// 			function = sime800_ip4R;
	// 		}
	// 		esp8266_command(g_esp8266_statusCheckCommands[status - 2], 200, 1, function, 0); //
	// 	}
	// }
	// if (status == ESP8266_STATUS_cgatt + 1 && !esp8266->apnStarted) {
	//
	// 	esp8266_command("AT+CWMODE_DEF=1\n",2000,5,0,0); // 1: client,2: host, 3: both
	// 	esp8266_command("AT+CWAUTOCONN=1\n",2000,5,0,0); //
	// 	esp8266_command("AT+CIPMODE=0\n",2000,5,0,0);
	// 	esp8266_command("AT+SAVETRANSLINK=0\n",2000,5,0,0);
	// 	esp8266_command("AT+CIPMUX=1\n",2000,5,0,0);
	//
	//
	//
	// 	esp8266_command("AT+CIPMUX=1\n", 200, 1, 0, 0); //
	// 	esp8266_command("AT+CSTT=\"CMNET\"\n", 200, 1, 0, 0);
	// 	esp8266_command("AT+CIICR\n", 5000, 2, 0, 0);
	// 	esp8266->apnStarted = 1;
	// }
	// if (status != ESP8266_STATUS_ok && status > 0) {
	// 	esp8266_comandFunction_t function = 0;
	// 	if (status == ESP8266_STATUS_cifsr) {
	// 		function = sime800_ip4R;
	// 	}
	// 	esp8266_command(g_esp8266_statusCheckCommands[status - 1], 200, 1, function, 0); //
	// }
	// if (status != ESP8266_STATUS_ok) {
	// 	if (!schedule_exists(ESP8266_RESET_SCHEDULE_ID)) {
	// 		schedule_once(ESP8266_RESET_SCHEDULE_ID, reset,30000,0);
	// 	}
	// } else {
	// 	schedule_cancel(ESP8266_RESET_SCHEDULE_ID);
	// }
	// static uint8_t pint;
	// if (status == ESP8266_STATUS_cpin) {
	// 	if (pint > 3) {
	// 		pint = 0;
	// 		if (esp8266->pin != 0){
	// 			esp8266->pin = 0;
	// 			if (esp8266->pinFunction) {
	// 				esp8266->pinFunction(esp8266, esp8266->pin);
	// 			}
	// 		}
	// 		schedule_once(ESP8266_RESET_SCHEDULE_ID, reset,0,0);
	// 	} else {
	// 		pint ++;
	// 	}
	// } else {
	// 	pint = 0;
	// }
	// index++;
}

void esp8266_comandInit(void** params) {
	esp8266_t* esp8266 = g_esp8266_main;
	setStatus(incStatus(esp8266->status, ESP8266_STATUS_starting + 1));
	schedule_repeat(ESP8266_STATUS_CHECKER_SCHEDULE_ID, esp8266_statusChecker, 1000, 0);
	schedule_repeat(ESP8266_RSSI_CHECKER_SCHEDULE_ID, esp8266_rssiChecker, 16000, 0);
}

// 4D

uint8_t esp8266_command4D(uint8_t* buffer, int16_t size, void** params) {
	esp8266_t* esp8266 = g_esp8266_main;
	uint8_t done = 0;
	esp8266_command_t* c = (esp8266_command_t*)list_peekFirst(&esp8266->commands);
	if (c) {
		if (c->function) {
			void* ps[] = {&buffer, &size};
			done = c->function(ps, c->params);
		} else {
			done = esp8266_common4R(buffer,size);
		}
	}
	if (done) {
		schedule_once(ESP8266_SEND_TOP_SCHEDULE_ID, onCommandReceived, 100, (void**)((done & 2) != 0));
		return 1;
	}
	return 0;
}

uint8_t esp8266_connected4D(uint8_t* buffer, int16_t size, void** params) {
	return 0;
}

static void resetListenerType(void ** params) {
	schedule_cancel(ESP8266_LISTERNER_SCHEDULE_ID);
	g_esp8266_lineListener.type = STREAM_LISTENER_TYPE_endByte;
}

uint8_t esp8266_package4D(uint8_t* buffer, int16_t size, void** params) {
	//"+RECEIVE,id,length:\r\ndata"
	esp8266_t* esp8266 = g_esp8266_main;
	static uint32_t id = 0;
	static int16_t length = 0;
	if (length && length == size) {
		wireless_socket_t* socket = (wireless_socket_t*)list_findById(&esp8266->sockets, id);
		if (esp8266->functions.received) {
			esp8266->functions.received(esp8266, socket, buffer, size);
		}
		length = 0;
		resetListenerType(0);
		return 1;
	} else {
		if (string_startWith((char const*)buffer, "+RECEIVE")) {
			char* line = string_duplicate((char const*)buffer, size);
			if (line) {
				int32_t* decs;
				int16_t decCount = parseDecs(line, ',', &decs);
				free(line);
				if (decCount == 3) {
					id = decs[1];
					length = decs[2];
				}
				free(decs);
			}
			g_esp8266_lineListener.type = STREAM_LISTENER_TYPE_ignore;
			schedule_once(ESP8266_LISTERNER_SCHEDULE_ID,resetListenerType,5000,0);
			return 1;
		}
	}
	return 0;
}

uint8_t esp8266_status4D(uint8_t* buffer, int16_t size, void** params) {
	esp8266_t* esp8266 = g_esp8266_main;
	uint8_t status = esp8266->status;
	uint8_t match = 0;
	if (string_startWith((char const*)buffer, "+")) {
		char* line = string_duplicate((char const*)buffer, size);
		if (line) {
			char** out;
			int16_t size = string_split(line, ':', &out);
			if (size>1) {
				char* command = string_trim(out[0]);
				char* code = string_trim(out[1]);
				if (command && code) {
					if (string_equal(command, "+CPIN")) {
						match = 1;
						uint8_t pin;
						if (string_equal(code, "READY")) {
							status = incStatus(status, ESP8266_STATUS_cpin + 1);
							pin = 1;
						} else {
							status = decStatus(status, ESP8266_STATUS_cpin);
							pin = 0;
						}
						if (esp8266->pin != pin){
							esp8266->pin = pin;
							// if (esp8266->pinFunction) {
							// 	esp8266->pinFunction(esp8266, esp8266->pin);
							// }
						}
					} else if (string_equal(command, "+CSQ")) {
						match = 1;
						int32_t* decs;
						int16_t decCount = parseDecs(code, ',', &decs);
						if (decCount > 0) {
							if (decs[0] != 99) {
								int8_t rssi = -114 + decs[0] * 2;
								if (rssi > -90) {
									status = incStatus(status, ESP8266_STATUS_csq + 1);
								} else {
									status = decStatus(status, ESP8266_STATUS_csq);
								}
								if (esp8266->rssi != rssi ){
									esp8266->rssi = rssi;
									if (esp8266->functions.rssi) {
										esp8266->functions.rssi(esp8266, esp8266->rssi);
									}
								}
							}
						}
						free(decs);
					} else if (string_equal(command, "+CREG")) {
						match = 1;
						int32_t* decs;
						int16_t decCount = parseDecs(code, ',', &decs);
						if (decCount > 1) {
							int32_t regStat = decs[1];
							if (regStat == 1) {
								status = incStatus(status, ESP8266_STATUS_creg + 1);
							} else {
								status = decStatus(status, ESP8266_STATUS_creg);
							}
						}
						free(decs);
					} else if (string_equal(command, "+CGATT")) {
						match = 1;
						int32_t* decs;
						int16_t decCount = parseDecs(code, ',', &decs);
						if (decCount > 0) {
							int32_t gatt = decs[0];
							if (gatt == 1) {
								status = incStatus(status, ESP8266_STATUS_cgatt + 1);
							} else {
								status = decStatus(status, ESP8266_STATUS_cgatt);
							}
						}
						free(decs);
					} else if (string_equal(command, "+PDP")) {
						match = 1;
						if (string_equal(code, "DEACT")) {
							// status = decStatus(status, 0);
							schedule_once(ESP8266_RESET_SCHEDULE_ID,reset,0,0);
						}
					}
					//+PDP: DEACT
				}
				free(command);
				free(code);
			}
			for (int16_t i = 0; i < size; i++) {
				free(out[i]);
			}
			free(out);
			free(line);
		}
	}
	if (setStatus(status)) {
		schedule_reset(ESP8266_STATUS_CHECKER_SCHEDULE_ID, 0);
	}
	// "+CPIN: READY" -> SIM card
	// "+CSQ: 0,0" -> signal strength
	// "+CREG: 0,1" ->  web status
	// "+CGATT: 1" -> gprs
	return match;
}

// listeners

esp8266_listener_t* esp8266_addListener(esp8266_listenerFunction_t function, void** params) {
	esp8266_t* esp8266 = g_esp8266_main;
	esp8266_listener_t* listener = (esp8266_listener_t*)malloc(sizeof(esp8266_listener_t));
	if (listener) {
		// listener->id = g_esp8266_listenerIdGenerator++;
		listener->function = function;
		listener->params = params;
		if (!list_addLast(&esp8266->listeners, listener)) {
			free (listener);
			return 0;
		}
	}
	return listener;
}

void esp8266_removeListener(esp8266_listener_t* listener) {
	esp8266_t* esp8266 = g_esp8266_main;
	esp8266_listener_t* removed = list_remove(&esp8266->listeners, listener);
	free(removed);
}

void esp8266_stream4D(stream_t* stream, stream_listener_t* listener, uint8_t* buffer, int16_t size) {
	esp8266_t* esp8266 = g_esp8266_main;
	list_t* list = esp8266->listeners;
	while (list) {
		if (list->this.raw) {
			esp8266_listener_t* listener = (esp8266_listener_t*) list->this.raw;
			if (listener->function(buffer, size, listener->params)) {
				break;
			}
		}
		list = list->next;
	}
}

void esp8266_startup4D(void** params) {
	// if (esp8266_hw_isPowerOn()) {
	// 	schedule_cancel(ESP8266_RESET_SCHEDULE_ID);
	// 	schedule_once(ESP8266_STARTUP_SCHEDULE_ID, esp8266_comandInit, 1000, params);
	// } else {
	// 	schedule_once(ESP8266_STARTUP_SCHEDULE_ID, esp8266_startup4D, 100, params);
	// }
}

void esp8266_init(esp8266_t* esp8266) {
	g_esp8266_main = esp8266;

	esp8266_hw_init(esp8266);

	stream_add(esp8266->inStream);
	stream_add(esp8266->outStream);
	stream_addListener(esp8266->inStream, &g_esp8266_lineListener);
	stream_addListener(esp8266->inStream, &g_esp8266_tickListener);

	esp8266_addListener(esp8266_package4D, 0);
	esp8266_addListener(esp8266_command4D, 0);
	esp8266_addListener(esp8266_connected4D, 0);
	esp8266_addListener(esp8266_status4D, 0);
	reset(0);
}

void esp8266_print(const char* str) {
	esp8266_t* esp8266 = g_esp8266_main;
	stream_write(esp8266->outStream, (const uint8_t*)str, strlen(str));
	stream_flush(esp8266->outStream);
}

uint8_t esp8266_setSsidAndPassword(char *ssid, char *password)
{
	// char rssid[34], rpassword[34];
	// sprintf(rssid,"\"%s\"",ssid);
	// sprintf(rpassword,"\"%s\"",password);
	// setCommandWait(1,"CWJAP_DEF",rssid,rpassword,NULL);
	// return waitUntilOk(20000)>0;
	return 0;
}
