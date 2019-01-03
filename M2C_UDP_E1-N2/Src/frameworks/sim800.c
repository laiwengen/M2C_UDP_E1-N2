#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "frameworks/strings.h"
#include "frameworks/number.h"
#include "frameworks/schedule.h"
#include "frameworks/sim800.h"

// defines
#define SIM800_RSSI_CHECKER_SCHEDULE_ID 0xef93201a
#define SIM800_STATUS_CHECKER_SCHEDULE_ID 0x54ccd372
#define SIM800_RESET_SCHEDULE_ID 0x54cde537
#define SIM800_LISTERNER_SCHEDULE_ID 0xc888a9d6
#define SIM800_SEND_TOP_SCHEDULE_ID 0xf4d33485
#define SIM800_STARTUP_SCHEDULE_ID 0xdb3584ba

enum sim800_socket_type_t {
	SIM800_SOCKET_TCP,
	SIM800_SOCKET_UDP
};

// consts

char const * const g_sim800_statusCheckCommands[] = {
	"AT+CPIN?\n",
	"AT+CSQ\n",
	"AT+CREG?\n",
	"AT+CGATT?\n",
	"AT+CIFSR\n",
};
// declears

void sim800_stream4D(stream_t* stream, stream_listener_t* listener, uint8_t* buffer, int16_t size);
uint8_t sime800_postsend4R(void** bytes, void** params);
void sim800_startup4D(void** params);

// globals
sim800_t* g_sim800_main;

stream_listener_t g_sim800_lineListener = {
	STREAM_LISTENER_TYPE_endByte,
	sim800_stream4D,
	{.endWith = '\n'},
	{0}
};
stream_listener_t g_sim800_tickListener = {
	STREAM_LISTENER_TYPE_tick,
	sim800_stream4D,
	{.ticks = 100},
	{0}
};
// statis functions
static uint8_t setStatus(uint8_t status) {
	sim800_t* sim800 = g_sim800_main;
	if (sim800->status != status) {
		if (status < sim800->status) {
			if (sim800->status > SIM800_STATUS_cgatt && status <= SIM800_STATUS_cgatt) {
				sim800->apnStarted = 0;
			}
		}
		sim800->status = status;
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
	sim800_t* sim800 = g_sim800_main;
	stream_write(sim800->outStream, buffer, size);
	stream_flush(sim800->outStream);
}

static void deleteCommand(int16_t index) {
	sim800_t* sim800 = g_sim800_main;
	sim800_command_t* c = (sim800_command_t*)list_removeByIndex(&sim800->commands, index);
	if (c) {
		free(c->bytes);
		free(c);
	}
}
static void sendTop(void** params) { // params is not used
	sim800_t* sim800 = g_sim800_main;
	if (list_size(&sim800->commands) > 0) {
		sim800_command_t* c = (sim800_command_t*)list_peekFirst(&sim800->commands);
		if (c->retries > 0) {
			c->retries--;
			doSend(c->bytes, c->size);
			if (c->function == sime800_postsend4R) { // TODO
				if (sim800->sendingFunction) {
					sim800->sendingFunction(sim800, c->params, c->bytes, c->size);
				}
			}
			schedule_once(SIM800_SEND_TOP_SCHEDULE_ID, sendTop, c->timeout, params);
		} else {
			//drop it
			deleteCommand(0);
			sendTop(params);
		}
	}
}

static void onCommandReceived(void** params) {
	schedule_cancel(SIM800_SEND_TOP_SCHEDULE_ID);
	// sim800_t* sim800 = g_sim800_main;
	uint8_t error = (uint32_t) params;
	if (!error) {
		deleteCommand(0);
	}
	sendTop(0);
}

static void sendCommand(sim800_command_t* c) {
	sim800_t* sim800 = g_sim800_main;
	list_t* cl = (list_t*)list_peekLast(&sim800->commands);
	list_add(&sim800->commands, c);
	if (list_size(&sim800->commands) == 1) {
		sendTop(0);
	}
}

static void reset(void** params) {
	sim800_t* sim800 = g_sim800_main;
	sim800->rssi = -114;
	if (sim800->rssiFunction) {
		sim800->rssiFunction(sim800, sim800->rssi);
	}
	// if (sim800->pinFunction) {
	// 	sim800->pinFunction(sim800, 0);
	// }
	schedule_cancel(SIM800_RSSI_CHECKER_SCHEDULE_ID);
	schedule_cancel(SIM800_STATUS_CHECKER_SCHEDULE_ID);
	schedule_cancel(SIM800_RESET_SCHEDULE_ID);
	schedule_cancel(SIM800_SEND_TOP_SCHEDULE_ID);
	schedule_cancel(SIM800_STARTUP_SCHEDULE_ID);
	schedule_once(SIM800_STARTUP_SCHEDULE_ID, sim800_startup4D, 2000, 0);
	schedule_once(SIM800_RESET_SCHEDULE_ID, reset,30000,0);
	setStatus(0);
	while (list_size(&sim800->commands) > 0) {
		deleteCommand(0);
	}
	while (list_size(&sim800->sockets) > 0) {
		wireless_socket_t* socket = list_removeFirst(&sim800->sockets);
		if (socket) {
			free(socket->host);
			free(socket);
		}
	}
	sim800_hw_reset();
}
// 4Rs

uint8_t sim800_common4R(uint8_t* buffer, int16_t size) {
	static int8_t errorCount = 0;
	if (string_startWith((const char*)buffer, "OK")) {
		errorCount = 0;
		return 1;
	} else if (string_startWith((char*)buffer, "ERROR")) {
		if (errorCount > 8) {
			errorCount = 0;
			schedule_once(SIM800_RESET_SCHEDULE_ID,reset,0,0);
		} else {
			errorCount ++;
		}
		return 2;
	}
	return 0;
}

uint8_t sime800_connected4R(void** bytes, void** params) {
	// sim800_t* sim800 = g_sim800_main;
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
		if (socket->type == SIM800_SOCKET_UDP) {
			if (string_startWith((const char*)buffer, "OK")) {
				socket->connected = 1;
				return 1;
			}
		}
	}
	return sim800_common4R(buffer, size);
}

uint8_t sime800_presend4R(void** bytes, void** params) {
	// sim800_t* sim800 = g_sim800_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	if (string_startWith((const char*)buffer, "> ") && size == 2) {
		return 1;
	}
	if (string_startWith((const char*)buffer, "ERROR") ) {//gzh add 2018-04-12
			init(0);
	}
	uint8_t done = sim800_common4R(buffer, size);
	if (done & 2) {
		deleteCommand(1);
	}
	return done;
}

uint8_t sime800_postsend4R(void** bytes, void** params) {
	// sim800_t* sim800 = g_sim800_main;
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
	return sim800_common4R(buffer, size);
}

uint8_t sim800_close4R(void** bytes, void** params) {
	// sim800_t* sim800 = g_sim800_main;
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
	return sim800_common4R(buffer, size);
}

uint8_t sime800_ip4R(void** bytes, void** params) {
	sim800_t* sim800 = g_sim800_main;
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
				setStatus(incStatus(sim800->status,  SIM800_STATUS_cifsr + 1));
				return 1;
			}
		}
	}
	return sim800_common4R(buffer, size);
}

uint8_t sime800_rssi4R(void** bytes, void** params) {
	sim800_t* sim800 = g_sim800_main;
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
							if (sim800->rssi != rssi ){
								sim800->rssi = rssi;
								if (sim800->rssiFunction) {
									sim800->rssiFunction(sim800, sim800->rssi);
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
	return sim800_common4R(buffer, size);
}

// sends
void sim800_command(const char* cmd, uint16_t timeout, uint16_t retries, sim800_comandFunction_t function, void** params) {
	sim800_command_t* c = (sim800_command_t*)malloc(sizeof(sim800_command_t));
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

uint8_t sim800_ready(void) {
	sim800_t* sim800 = g_sim800_main;
	if (sim800->status != SIM800_STATUS_ok) {
		return 0;
	}
	return 1;
}

uint8_t sim800_send(uint8_t type, char const* host, uint16_t port, uint8_t* buffer, uint16_t size) {
	sim800_t* sim800 = g_sim800_main;
	if (sim800->status != SIM800_STATUS_ok) {
		return 0;
	}
	if (list_size(&sim800->commands) > 10) {
		return 0;
	}
	int8_t socketCount = list_size(&sim800->sockets);
	wireless_socket_t* socket = 0;
	char idBuffer[2];
	// find if socket alread exists
	list_t* l = sim800->sockets;
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
			socket = list_removeFirst(&sim800->sockets);
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
				sim800_command(str, 200, 2, sim800_close4R, 0);
				free(str);
			}
		}
	}
	//TODO add setuped for UDP?
	// check if connected
	// even if udp do not need the CONNECT. but this just make sure it has sent the CIPSTART for host and port.
	if (socket && socket->connected == 0) {
		list_add(&sim800->sockets, socket);
		socket->type = type;
		socket->host = string_duplicate(host, -1);
		socket->port = port;
		char portBuffer[6];
		number_toDecString(port, 0, portBuffer, sizeof(portBuffer));
		char const* concats[] = {"AT+CIPSTART=", idBuffer, ",\"", type == SIM800_SOCKET_TCP ? "TCP" : "UDP", "\",\"", host, "\",\"", portBuffer, "\"\n"};
		char* str = string_concat(concats, sizeof(concats) / sizeof(char*));
		if (str) {
			// sim800_command("AT+CGATT?\n",2000,5,0,0);
			sim800_command(str, 2000, 1, sime800_connected4R, (void**)socket);
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
			// sim800_command("AT+CGATT?\n",2000,5,0,0); //
			sim800_command(str, 200, 1, sime800_presend4R, (void**)socket);
			free(str);
			sim800_command_t* c = (sim800_command_t*)malloc(sizeof(sim800_command_t));
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

void sim800_rssiChecker(void** params) {
	sim800_t* sim800 = g_sim800_main;
	if (sim800->status > SIM800_STATUS_csq) {
		if (list_size(&sim800->commands) > 10) {
			return;
		}
		sim800_command("AT+CSQ\n", 2000, 2, sime800_rssi4R, 0);
	}
}

void sim800_statusChecker(void** params) {
	static uint32_t index;
	static int16_t pdt = 0;
	sim800_t* sim800 = g_sim800_main;
	if (!sim800_hw_isPowerOn()) {
		if (pdt > 2) {
			pdt = 0;
			reset(0);
		} else {
			pdt ++;
		}
		return;
	} else {
		pdt = 0;
	}
	if (list_size(&sim800->commands) > 10) {
		return;
	}
	uint8_t status = sim800->status;
	if (status > 1) {
		if (status != SIM800_STATUS_ok || (index & 0x0f) == 0) {
			sim800_comandFunction_t function = 0;
			if (status == SIM800_STATUS_cifsr + 1) {
				function = sime800_ip4R;
			}
			sim800_command(g_sim800_statusCheckCommands[status - 2], 200, 1, function, 0); //
		}
	}
	if (status == SIM800_STATUS_cgatt + 1 && !sim800->apnStarted) {
		sim800_command("AT+CIPMUX=1\n", 200, 1, 0, 0); //
		sim800_command("AT+CSTT=\"CMNET\"\n", 200, 1, 0, 0);
		sim800_command("AT+CIICR\n", 5000, 2, 0, 0);
		sim800->apnStarted = 1;
	}
	if (status != SIM800_STATUS_ok && status > 0) {
		sim800_comandFunction_t function = 0;
		if (status == SIM800_STATUS_cifsr) {
			function = sime800_ip4R;
		}
		sim800_command(g_sim800_statusCheckCommands[status - 1], 200, 1, function, 0); //
	}
	if (status != SIM800_STATUS_ok) {
		if (!schedule_exists(SIM800_RESET_SCHEDULE_ID)) {
			schedule_once(SIM800_RESET_SCHEDULE_ID, reset,30000,0);
		}
	} else {
		schedule_cancel(SIM800_RESET_SCHEDULE_ID);
	}
	static uint8_t pint;
	if (status == SIM800_STATUS_cpin) {
		if (pint > 3) {
			pint = 0;
			if (sim800->pin != 0){
				sim800->pin = 0;
				if (sim800->pinFunction) {
					sim800->pinFunction(sim800, sim800->pin);
				}
			}
			schedule_once(SIM800_RESET_SCHEDULE_ID, reset,0,0);
		} else {
			pint ++;
		}
	} else {
		pint = 0;
	}
	index++;
}

void sim800_comandInit(void** params) {
	sim800_t* sim800 = g_sim800_main;
	setStatus(incStatus(sim800->status, SIM800_STATUS_starting + 1));
	schedule_repeat(SIM800_STATUS_CHECKER_SCHEDULE_ID, sim800_statusChecker, 1000, 0);
	schedule_repeat(SIM800_RSSI_CHECKER_SCHEDULE_ID, sim800_rssiChecker, 16000, 0);
}

// 4D

uint8_t sim800_command4D(uint8_t* buffer, int16_t size, void** params) {
	sim800_t* sim800 = g_sim800_main;
	uint8_t done = 0;
	sim800_command_t* c = (sim800_command_t*)list_peekFirst(&sim800->commands);
	if (c) {
		if (c->function) {
			void* ps[] = {&buffer, &size};
			done = c->function(ps, c->params);
		} else {
			done = sim800_common4R(buffer,size);
		}
	}
	if (done) {
		schedule_once(SIM800_SEND_TOP_SCHEDULE_ID, onCommandReceived, 100, (void**)((done & 2) != 0));
		return 1;
	}
	return 0;
}

uint8_t sim800_connected4D(uint8_t* buffer, int16_t size, void** params) {
	return 0;
}

static void resetListenerType(void ** params) {
	schedule_cancel(SIM800_LISTERNER_SCHEDULE_ID);
	g_sim800_lineListener.type = STREAM_LISTENER_TYPE_endByte;
}

uint8_t sim800_package4D(uint8_t* buffer, int16_t size, void** params) {
	//"+RECEIVE,id,length:\r\ndata"
	sim800_t* sim800 = g_sim800_main;
	static uint32_t id = 0;
	static int16_t length = 0;
	if (length && length == size) {
		wireless_socket_t* socket = (wireless_socket_t*)list_findById(&sim800->sockets, id);
		if (sim800->receivedFunction) {
			sim800->receivedFunction(sim800, socket, buffer, size);
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
			g_sim800_lineListener.type = STREAM_LISTENER_TYPE_ignore;
			schedule_once(SIM800_LISTERNER_SCHEDULE_ID,resetListenerType,5000,0);
			return 1;
		}
	}
	return 0;
}

uint8_t sim800_status4D(uint8_t* buffer, int16_t size, void** params) {
	sim800_t* sim800 = g_sim800_main;
	uint8_t status = sim800->status;
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
							status = incStatus(status, SIM800_STATUS_cpin + 1);
							pin = 1;
						} else {
							status = decStatus(status, SIM800_STATUS_cpin);
							pin = 0;
						}
						if (sim800->pin != pin){
							sim800->pin = pin;
							if (sim800->pinFunction) {
								sim800->pinFunction(sim800, sim800->pin);
							}
						}
					} else if (string_equal(command, "+CSQ")) {
						match = 1;
						int32_t* decs;
						int16_t decCount = parseDecs(code, ',', &decs);
						if (decCount > 0) {
							if (decs[0] != 99) {
								int8_t rssi = -114 + decs[0] * 2;
								if (rssi > -90) {
									status = incStatus(status, SIM800_STATUS_csq + 1);
								} else {
									status = decStatus(status, SIM800_STATUS_csq);
								}
								if (sim800->rssi != rssi ){
									sim800->rssi = rssi;
									if (sim800->rssiFunction) {
										sim800->rssiFunction(sim800, sim800->rssi);
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
								status = incStatus(status, SIM800_STATUS_creg + 1);
							} else {
								status = decStatus(status, SIM800_STATUS_creg);
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
								status = incStatus(status, SIM800_STATUS_cgatt + 1);
							} else {
								status = decStatus(status, SIM800_STATUS_cgatt);
							}
						}
						free(decs);
					} else if (string_equal(command, "+PDP")) {
						match = 1;
						if (string_equal(code, "DEACT")) {
							// status = decStatus(status, 0);
							schedule_once(SIM800_RESET_SCHEDULE_ID,reset,0,0);
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
		schedule_reset(SIM800_STATUS_CHECKER_SCHEDULE_ID, 0);
	}
	// "+CPIN: READY" -> SIM card
	// "+CSQ: 0,0" -> signal strength
	// "+CREG: 0,1" ->  web status
	// "+CGATT: 1" -> gprs
	return match;
}

// listeners

sim800_listener_t* sim800_addListener(sim800_listenerFunction_t function, void** params) {
	sim800_t* sim800 = g_sim800_main;
	sim800_listener_t* listener = (sim800_listener_t*)malloc(sizeof(sim800_listener_t));
	if (listener) {
		// listener->id = g_sim800_listenerIdGenerator++;
		listener->function = function;
		listener->params = params;
		if (!list_addLast(&sim800->listeners, listener)) {
			free (listener);
			return 0;
		}
	}
	return listener;
}

void sim800_removeListener(sim800_listener_t* listener) {
	sim800_t* sim800 = g_sim800_main;
	sim800_listener_t* removed = list_remove(&sim800->listeners, listener);
	free(removed);
}

void sim800_stream4D(stream_t* stream, stream_listener_t* listener, uint8_t* buffer, int16_t size) {
	sim800_t* sim800 = g_sim800_main;
	list_t* list = sim800->listeners;
	while (list) {
		if (list->this.raw) {
			sim800_listener_t* listener = (sim800_listener_t*) list->this.raw;
			if (listener->function(buffer, size, listener->params)) {
				break;
			}
		}
		list = list->next;
	}
}

void sim800_startup4D(void** params) {
	if (sim800_hw_isPowerOn()) {
		schedule_cancel(SIM800_RESET_SCHEDULE_ID);
		schedule_once(SIM800_STARTUP_SCHEDULE_ID, sim800_comandInit, 1000, params);
	} else {
		schedule_once(SIM800_STARTUP_SCHEDULE_ID, sim800_startup4D, 100, params);
	}
}

void sim800_init(sim800_t* sim800) {
	g_sim800_main = sim800;

	sim800_hw_init();

	stream_add(sim800->inStream);
	stream_add(sim800->outStream);
	stream_addListener(sim800->inStream, &g_sim800_lineListener);
	stream_addListener(sim800->inStream, &g_sim800_tickListener);

	sim800_addListener(sim800_package4D, 0);
	sim800_addListener(sim800_command4D, 0);
	sim800_addListener(sim800_connected4D, 0);
	sim800_addListener(sim800_status4D, 0);
	reset(0);
}

void sim800_print(const char* str) {
	sim800_t* sim800 = g_sim800_main;
	stream_write(sim800->outStream, (const uint8_t*)str, strlen(str));
	stream_flush(sim800->outStream);
}
