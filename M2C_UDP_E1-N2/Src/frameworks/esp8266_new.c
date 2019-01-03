#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "frameworks/strings.h"
#include "frameworks/tick.h"
#include "frameworks/wireless.h"
#include "frameworks/esp8266.h"
#include "frameworks/number.h"
#include "stm32f0xx_hal.h"



void esp8266_connect(wireless_socket_t* socket);
void esp8266_disconnect(wireless_socket_t* socket);
//void esp8266_send(wireless_socket_t* socket, char* data);
uint8_t esp8266_send(uint8_t type, char const* host, uint16_t port, uint8_t* buffer, uint16_t size);
static void esp8266_startup4D(void** params);
void esp8266_sendHttp(wireless_socket_t socket, esp8266_httpRequest_t* request);
static uint8_t esp8266Package4D(uint8_t* buffer, int16_t size, void** params);
esp8266_t* g_esp8266_main;
#define ESP8266_SEND_TOP_SCHEDULE_ID 0x3485

// uint32_t* g_esp8266_listenerIdGenerator = 0;

#define ESP8266_STATUS_CHECKER_SCHEDULE_ID 0x54cc8266
#define ESP8266_LISTERNER_SCHEDULE_ID 0xc8888266
#define ESP8266_STARTUP_SCHEDULE_ID 0xdb358266

//uint8_t esp8266_packageListener(uint8_t* buffer, uint16_t size, void* params) {
//	return 0;
//}

//uint8_t esp8266_smartLinkListener(uint8_t* buffer, uint16_t size, void* params) {
//	return 0;
//}

//uint8_t esp8266_statusListener(uint8_t* buffer, uint16_t size, void* params) {
//	return 0;
//}
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
void doSend(uint8_t* buffer, int16_t size) {
	esp8266_t* esp_8266 = g_esp8266_main; 
	stream_write(esp_8266->outStream, (const uint8_t*)buffer, size);
	stream_flush(esp_8266->outStream);
}
static void deleteCommand(int16_t index) {
	esp8266_t* esp8266 = g_esp8266_main;
	esp8266_command_t* c = (esp8266_command_t*)list_removeByIndex(&esp8266->commands, index);
	if (c) {
		free(c->bytes);
		free(c);
	}
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
esp8266_status_t esp8266_getStatus(void)
{
	return (g_esp8266_main->status);
}
uint8_t esp8266_setStatus(esp8266_status_t status)
{
	if (g_esp8266_main->status != status) {
	
		g_esp8266_main->status = status;
		return 1;
	}
	return 0;
}
void esp8266_reset(void** params)
{
	esp8266_t* esp8266 = g_esp8266_main;
	
	if (esp8266->status != ESP8266_STATUS_smarting)
	{
		schedule_cancel(ESP8266_STATUS_CHECKER_SCHEDULE_ID);
		schedule_cancel(ESP8266_RESET_SCHEDULE_ID);
		schedule_cancel(ESP8266_SEND_TOP_SCHEDULE_ID);
		schedule_cancel(ESP8266_STARTUP_SCHEDULE_ID);
		schedule_cancel(MAIN_ESP8266_AUTO_RESET_SCHEDULE_ID);
		esp8266_setStatus(ESP8266_STATUS_starting);
		
		schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 60000, 0);
		schedule_once(ESP8266_STARTUP_SCHEDULE_ID, esp8266_startup4D, 1000, 0);
	//	schedule_once(ESP8266_STARTUP_SCHEDULE_ID, esp8266_startup4D, 1500, 0);
	//	schedule_once(esp8266_RESET_SCHEDULE_ID, esp8266_reset,30000,0);
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
		esp8266_hw_reset();
		
	}
}
//void esp8266_reset(void)
//{
//	schedule_once(ESP8266_RESET_SCHEDULE_ID,esp8266_reset,0,0);
//}
enum esp8266_socket_type_t {
	ESP8266_SOCKET_TCP,
	ESP8266_SOCKET_UDP
};

uint8_t esp8266_common4R(uint8_t* buffer, int16_t size) {
	static int8_t errorCount = 0;
	if (string_startWith((const char*)buffer, "OK")) {
		errorCount = 0;
		return 1;
	} else if (string_startWith((char*)buffer, "ERROR")) {
		if (errorCount > 8) {
			errorCount = 0;
			schedule_once(ESP8266_RESET_SCHEDULE_ID,esp8266_reset,0,0);
		} else {
			errorCount ++;
		}
		return 2;
	}
	return 0;
}
uint8_t esp8266_presend4R(void** bytes, void** params) {
	// sim800_t* sim800 = g_sim800_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	if (string_startWith((const char*)buffer, "> ") && size == 2) {
		return 1;
	}
	
	//busy s...
	uint8_t done = esp8266_common4R(buffer, size);
	if (done == 2) {
		deleteCommand(1);
		return done;
	}
	else 
	{
		return 0;
	}
 
}
void esp8266_resetLineListener(void** params)
{
	g_esp8266_lineListener.type = STREAM_LISTENER_TYPE_endByte;
}
uint8_t esp8266_postsend4R(void** bytes, void** params) {
	// esp8266_t* esp8266 = g_esp8266_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	char* line = string_duplicate((char const*)buffer, size);
	if (line) {
		uint8_t match = string_startWith(line, "SEND OK");
//		uint8_t match = string_startWith(line, "Recv ");
		free(line);
		if (match) {
			g_esp8266_lineListener.type = STREAM_LISTENER_TYPE_ignore;
			schedule_once(0,esp8266_resetLineListener,5000,0);
			//esp8266Package4D(buffer, size, 0);
			//continue to other 4D
			return 1;
		}
//		uint8_t match = string_startWith(line, "bus");
//		free(line);
//		if (match) {
//			return 1;
//		}		
	}
	return esp8266_common4R(buffer, size);
}

//20180627 TODO TEST status4R
static uint8_t esp8266Status4R(void** bytes, void** params) {
//	 esp8266_t* esp8266 = g_esp8266_main;
	//+CIPSTATUS:1,"UDP","47.95.28.196",59732,42300,0
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	esp8266_status_t lastStatus = esp8266_getStatus();
	char* line = string_duplicate((char const*)buffer, size);
	if (line) {
		if (string_startWith((const char*)buffer, "STATUS:"))
		{
			int32_t* decs;
			int16_t decCount;
			decCount = parseDecs((const char*)buffer,':',&decs);
			if (decCount > 0) {
				int32_t status = decs[1];
				if (lastStatus != ESP8266_STATUS_smarting)
				{
					if (lastStatus != ESP8266_STATUS_ok)
					{
						if (status == 2)
						{
							esp8266_setStatus(ESP8266_STATUS_getIP);	
							
							schedule_reset(ESP8266_RESET_SCHEDULE_ID,30000);
						}
						else if (status == 3)
						{
							esp8266_setStatus(ESP8266_STATUS_connectWifi);	
						}
					}
					if (status == 4 || status == 5)
					{
						esp8266_setStatus(ESP8266_STATUS_connectWifi);	
						schedule_once(ESP8266_RESET_SCHEDULE_ID,esp8266_reset,30000,0);	
					}
//					if (lastStatus == ESP8266_STATUS_getIP || lastStatus == ESP8266_STATUS_connectWifi)
//					{//+CIPSTATUS:0,"UDP","47.95.28.196",59732,3925,0
//						
//						esp8266_setStatus(ESP8266_STATUS_connectWifi);
//					}
					
				}
				free(decs);
			}
		}
//		else if (string_startWith((const char*)buffer, "+CIPSTATUS:"))
//		{
//			
//		}
		free(line);
	}
	return esp8266_common4R(buffer, size);
}	
//#define ESP8266_SEND_TOP_SCHEDULE_ID 0xf4d3348a
static void sendTop(void** params) { // params is not used
	esp8266_t* esp8266 = g_esp8266_main;
	if (list_size(&esp8266->commands) > 0) {
		esp8266_command_t* c = (esp8266_command_t*)list_peekFirst(&esp8266->commands);
		if (c->retries > 0) {
			c->retries--;
			doSend(c->bytes, c->size);
			if (c->function == esp8266_postsend4R) { // TODO
				if (esp8266->sendingFunction) {
					esp8266->sendingFunction(esp8266,c->params, c->bytes, c->size);
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
uint8_t esp8266_connected4R(void** bytes, void** params) {
//	 esp8266_t* esp8266 = g_esp8266_main;
	uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	wireless_socket_t* socket = (wireless_socket_t*)params;
	if (socket) {
		char* line = string_duplicate((char const*)buffer, size);
		if (line) {
			uint8_t match = paramMatch(line, 1, "CONNECT") || string_startWith((const char*)buffer, "ALREADY CONNECT");
			free(line);
			if (match) {
				socket->connected = 1;
//				if (esp8266->smartLinkFunction)
//				{
//					esp8266->smartLinkFunction(esp8266);
//				}
				return 1;
			}
		}
//		if (socket->type == ESP8266_SOCKET_UDP) {
//			if (string_startWith((const char*)buffer, "OK")) {
//				socket->connected = 1;
//				return 1;
//			}
//		}
	}
	return esp8266_common4R(buffer, size);
}
//static void sendCommand(sim800_command_t* c) {
//	sim800_t* sim800 = g_sim800_main;
//	list_t* cl = (list_t*)list_peekLast(&sim800->commands);
//	list_add(&sim800->commands, c);
//	if (list_size(&sim800->commands) == 1) {
//		sendTop(0);
//	}
//}
void esp8266_commandOut(esp8266_command_t* c) {
	esp8266_t* esp8266 = g_esp8266_main;
	list_t* cl = (list_t*)list_peekLast(&esp8266->commands);
	list_add(&esp8266->commands, c);
	if (list_size(&esp8266->commands) == 1) {
		sendTop(0);
	}
}

void esp8266_command(const char* cmd, uint16_t timeout, uint16_t retries, esp8266_comandFunction_t function, void* params) {
	esp8266_command_t* c = (esp8266_command_t*)malloc(sizeof(esp8266_command_t));
	if (c) {
		c->bytes = (uint8_t*)string_duplicate(cmd, -1);
		c->size = strlen(cmd);
		c->timeout = timeout;
		c->retries = retries;
		c->function = function;
		c->params = params;
		esp8266_commandOut(c);
	}
}


uint8_t esp8266_send(uint8_t type, char const* host, uint16_t port, uint8_t* buffer, uint16_t size) {
	esp8266_t* esp8266 = g_esp8266_main;
		if (esp8266->status < ESP8266_STATUS_getIP) {
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
	// start a new one if not exists
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
			char const* concats[] = {"AT+CIPCLOSE=", idBuffer, "\r\n"};
			char* str = string_concat(concats, sizeof(concats) / sizeof(char*));
			if (str) {
				esp8266_command(str, 200, 2, 0, 0);
				free(str);
			}
		}
	}
	//TODO add setuped for UDP?
	// check if connected
	// even if udp do not need the CONNECT. but this just make sure it has sent the CIPSTART for host and port.
	if (socket && socket->connected == 0 && esp8266->wifiSsid ) {
		list_add(&esp8266->sockets, socket);
		socket->type = type;
		socket->host = string_duplicate(host, -1);
		socket->port = port;
		char portBuffer[6];
		number_toDecString(port, 0, portBuffer, sizeof(portBuffer));
		char const* concats[] = {"AT+CIPSTART=",idBuffer,",\"",type==ESP8266_SOCKET_TCP?"TCP":"UDP","\",\"",host,"\",",portBuffer,"\r\n"};
		char* str = string_concat(concats,sizeof(concats)/sizeof(char*));
		if (str) {
			schedule_reset(ESP8266_STATUS_CHECKER_SCHEDULE_ID,4000);
			esp8266_command(str, 2000, 1, esp8266_connected4R, (void**)socket);
			free(str);
		}
	}
	// send data
	if (socket && esp8266->wifiSsid && socket->connected) {
		char lengthBuffer[6];
		number_toDecString(size, 0, lengthBuffer, sizeof(lengthBuffer));
		char const* concats[] = {"AT+CIPSEND=", idBuffer, ",", lengthBuffer, "\r\n"};
		char* str = string_concat(concats, sizeof(concats) / sizeof(char*));
		if (str) {
			// sim800_command("AT+CGATT?\n",2000,5,0,0); //
			esp8266_command(str, 200, 1, esp8266_presend4R, (void**)socket);
			free(str);
			esp8266_command_t* c = (esp8266_command_t*)malloc(sizeof(esp8266_command_t));
			uint8_t* bytes = (uint8_t*)malloc(size);
			if (c && bytes) {
				memcpy(bytes, buffer, size);
				c->bytes = bytes;
				c->size = size;
				c->timeout = 200;
				c->retries = 1;
				c->function = esp8266_postsend4R;
				c->params = socket;
				esp8266_commandOut(c);
			} else {
				free(c);
				free(bytes);
			}
		}
	}
	return 0;
}
//static uint8_t incStatus(uint8_t status, uint8_t to) {
//	return max(status, to);
//}

//static uint8_t decStatus(uint8_t status, uint8_t to) {
//	return min(status, to);
//}

uint8_t esp8266_ready(void)
{
	esp8266_t* esp8266 = g_esp8266_main;
	if (esp8266->status >= ESP8266_STATUS_getIP)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
static uint8_t esp8266Info4R(void** bytes, void** params) {
		esp8266_t* esp8266 = g_esp8266_main;
		uint8_t* buffer = *(uint8_t**)bytes[0];
		int16_t size = *(int16_t*)bytes[1];
		if ((uint32_t)params == 0 ) {
			if (size <256) {
				//+ CWJAP_CUR:"ssid",<bssid>
				if (string_startWith((const char*)buffer, "No AP"))
				{
					esp8266_setStatus(ESP8266_STATUS_connectWifi);
				}
				else if (string_startWith((const char*)buffer,"+CWJAP_CUR:"))
				{
					int16_t length = strlen("+CWJAP_CUR:");
					char* ssidBuffer = string_duplicate((const char*)(buffer + length),size-length);
					if (ssidBuffer)
					{	
						char** ssid = NULL;
						int16_t splitSize = string_split((const char*)ssidBuffer,',',&ssid);
						if (splitSize>1)
						{
							int16_t ssidLength = strlen(ssid[0]) - 2;
							if (esp8266->wifiSsid) {
								free(esp8266->wifiSsid);
								esp8266->wifiSsid = NULL;
							}
							char* tssid = malloc(ssidLength + 1);
							*(ssid[0]+ssidLength+1) = '\0';
							if (tssid) {
								strcpy(tssid,ssid[0]+1);
								esp8266->wifiSsid = tssid;//del  double quotes
								if (esp8266->ipChangedFunction) {
									esp8266->ipChangedFunction(esp8266);
								}					
							}

							
							//return 1;
						}
							for (int16_t i = 0; i < splitSize; i++) {
							free(ssid[i]);
						}
						free(ssid);
						free(ssidBuffer);
					}
				}
			}
		}
		// 867793034663403
		// schedule_cancel(MAIN_esp8266_INFO_SCHEDULE_ID);
		return esp8266_common4R(buffer, size);
}
static uint8_t esp8266Status4D(uint8_t* buffer, int16_t size, void** params) {
	esp8266_t* esp8266 = g_esp8266_main;
	esp8266_status_t esp8266Status = esp8266->status;
	uint8_t match = 0;
	if (string_startWith((char const*)buffer, "AT+")) {
		char* line = string_duplicate((char const*)buffer, size);
		if (line) {
			char** out;
			int16_t splitSize = string_split(line, '+', &out);
			if (size>1) {
				char* command = string_trim(out[1]);
				//char* status = string_trim(out[1]);
				if (command) {
//					if (string_startWith((char const*)command, "CWSTARTSMART")) {
////						int32_t* decs;
////						int16_t decCount = parseDecs(code, ',', &decs);
//						esp8266Status = ESP8266_STATUS_smarting;
//					
//					}
//					else 
					if (string_equal((char const*)command, "CWSTOPSMART")) {
						schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 30000, 0);
						esp8266Status = ESP8266_STATUS_initOK;
					
					}
				}
				free(command);

			}
			for (int16_t i = 0; i < splitSize; i++) {
				free(out[i]);
			}
			free(out);
			free(line);
		}
	}
	else if (string_startWith((char const*)buffer, "WIFI CONNECTED"))
	{
//		esp8266Status = ESP8266_STATUS_connectWifi;
		schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 60000, 0);
	}
	else if (string_startWith((char const*)buffer, "Smart get wifi info"))	
	{
			if (esp8266->smartLinkFunction) {
				esp8266->smartLinkFunction(esp8266);
			}		
	}
	else if (string_startWith((char const*)buffer, "WIFI GOT IP"))
	{
		esp8266Status = ESP8266_STATUS_getIP;
		schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 30000, 0);
		esp8266_command("AT+CWJAP_CUR?\r\n",2000,2,esp8266Info4R,0);
//		esp8266_command("AT+CWJAP_CUR?\r\n",2000,2,esp8266Info4R,0);
	}
	else if (string_startWith((char const*)buffer, "WIFI DISCONNECT") && esp8266Status != ESP8266_STATUS_smarting)
	{
		esp8266Status = ESP8266_STATUS_connectWifi;
		schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 30000, 0);
	}
	else if (string_startWith((char const*)buffer, "READY") && esp8266Status < ESP8266_STATUS_initOK)
	{
		esp8266Status = ESP8266_STATUS_initOK;
		schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 30000, 0);
	}
	else if (string_startWith((char const*)buffer, "DHCP TIMEOUT") )
	{
		if (esp8266Status == ESP8266_STATUS_smarting)
		{
			esp8266_smartStop();
			esp8266Status = ESP8266_STATUS_connectWifi;
		}
		schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 30000, 0);
	}
	else if (string_startWith((char const*)buffer, "DNS Fail") )
	{
	//	esp8266Status = ESP8266_STATUS_initOK;
		schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 30000, 0);
	}
//	else if (string_startWith((char const*)buffer, "smartconfig connected wifi") || )
//	{

//	}
	if (esp8266_setStatus(esp8266Status)) {
		schedule_reset(ESP8266_STATUS_CHECKER_SCHEDULE_ID, 0);
	}
//	if (setStatus(status)) {
//		schedule_reset(esp8266_STATUS_CHECKER_SCHEDULE_ID, 0);
//	}
	// "+CPIN: READY" -> SIM card
	// "+CSQ: 0,0" -> signal strength
	// "+CREG: 0,1" ->  web status
	// "+CGATT: 1" -> gprs
	return match;
}


	
void esp8266_sendBytes(uint8_t* buffer,uint16_t size)
{
	//char* str="test\r\n";
	  stream_write(g_esp8266_main->outStream,buffer,size);//将字符串str写入dma的发送buffer里,注意如果发送字节流不可用strlen
    stream_flush(g_esp8266_main->outStream);//调用定义的flushFunction函数
}
char const * const g_esp8266_statusCheckCommands[] = {
	"AT+CIPSTATUS\r\n",
};
void esp8266_statusChecker(void** params) {
	static uint32_t index;
	static int16_t pdt = 0;
	esp8266_t* esp8266 = g_esp8266_main;
	if (!esp8266_hw_isPowerOn()) {
		if (pdt > 2) {
			pdt = 0;
			esp8266_reset(0);
		} else {
			pdt ++;
		}
		return;
	} else {
		pdt = 0;
	}
	if (list_size(&esp8266->commands) > 10) {
		return;
	}
	uint8_t status = esp8266->status;
	
//	if (status >= ESP8266_STATUS_getIP && !(esp8266->wifiSsid)) {
////			esp8266_comandFunction_t function = 0;
//////			function = esp8266_status4R;
////			esp8266_command(g_esp8266_statusCheckCommands[0], 200, 1, function, 0); //
//		esp8266_command("AT+CWJAP_CUR?\r\n",2000,2,esp8266Info4R,0);
//	}
	if ((status != ESP8266_STATUS_smarting) && ((index & 0x0f) == 0)) {
		esp8266_command("AT+CIPSTATUS\r\n",2000,2,esp8266Status4R,0);
	}

	if (status != ESP8266_STATUS_ok) {
		if (!schedule_exists(ESP8266_RESET_SCHEDULE_ID)) {
			schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset,60000,0);
		}
	} else {
		schedule_cancel(ESP8266_RESET_SCHEDULE_ID);
	}
	index++;
	uint32_t now = tick_ms();
	if ((status == ESP8266_STATUS_smarting) && ((esp8266->smartNoApTick) - now > (UINT32_MAX>>1)) )
	{
		esp8266_smartStop();
		//esp8266_setStatus(ESP8266_STATUS_initOK);
		esp8266_reset(0);
	}	
	
}
void esp8266_delayStatusChecker(void** params){
	if (schedule_exists(ESP8266_STATUS_CHECKER_SCHEDULE_ID)) {
		schedule_reset(ESP8266_STATUS_CHECKER_SCHEDULE_ID,5000);
	}
}
void esp8266_comandInit(void** params) {

	esp8266_sendBytes("+++",3);
	esp8266_command("AT+CIPMODE=0\r\n",2000,5,0,0);
	esp8266_command("AT+CIPMUX=1\r\n",2000,5,0,0);
	esp8266_command("AT+CWMODE_DEF=1\r\n",2000,5,0,0); // 1: client,2: host, 3: both
	esp8266_command("AT+CWAUTOCONN=1\r\n",2000,5,0,0); //
	esp8266_command("AT+SAVETRANSLINK=0\r\n",2000,5,0,0);
	
	schedule_repeat(ESP8266_STATUS_CHECKER_SCHEDULE_ID, esp8266_statusChecker, 1000, 0);
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
static uint8_t esp8266Command4D(uint8_t* buffer, int16_t size, void** params) {
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
//static void resetListenerType(void ** params) {
//	schedule_cancel(ESP8266_LISTERNER_SCHEDULE_ID);
//	g_esp8266_lineListener.type = STREAM_LISTENER_TYPE_endByte;
//}
static uint8_t esp8266Package4D(uint8_t* buffer, int16_t size, void** params) {
	//"+IPD,id,length:data" to do 
//#error "not complete"
	esp8266_t* esp8266 = g_esp8266_main;
	uint32_t id = 0;
	int16_t length = 0;
	int16_t headLength = 0;
	__IO int16_t validSize = size;
	
	uint8_t match = 0;
//	if (packStarted && receivedPackLength > 0 && receivedPackLength < length)
//	{
//		memc
//	}
//	else 
	char* startAddr = 0;
	startAddr = strstr((char const*)buffer, "+IPD");
	if (startAddr != NULL) {
		//validSize -= 2;
		char* line = string_duplicate((char const*)(startAddr), validSize);
		if (line) {
			
			char** out = NULL;
			volatile int16_t splitSize = string_split(line, ':', &out);
			free(line);
			if (splitSize>1)
			{
				headLength = strlen(out[0]) + 1;
				int32_t* decs;
				int16_t decCount = parseDecs(out[0], ',', &decs);
				
				if (decCount > 2) {
					id = decs[1];
					length = decs[2];
				}
				free(decs);
				for (int16_t i = 0; i < splitSize; i++) {
					free(out[i]);
				}
				free(out);
				
				uint8_t* bufferToDeal = NULL;
				bufferToDeal = (uint8_t*)(buffer + 2 + headLength);
				
				if (length + headLength <=validSize ) {
					wireless_socket_t* socket = (wireless_socket_t*)list_findById(&esp8266->sockets, id);
					if (esp8266->receivedFunction) {
						esp8266->receivedFunction(esp8266, socket, bufferToDeal, length);
					}
					//resetListenerType(0);
					
					match = 1;
				}
				else
				{
					match = 0;
				}
			}
		}
		esp8266_resetLineListener(0);
		return match;
	}
	return 0;
}

esp8266_listener_t* esp8266_addListener(esp8266_listenerFunction_t function, void* params) {
	esp8266_listener_t* listener = (esp8266_listener_t*)malloc(sizeof(esp8266_listener_t));
	if (listener) {
		// listener->id = g_esp8266_listenerIdGenerator++;
		listener->function = function;
		listener->params = params;
		if (!list_addLast(&g_esp8266_main->listeners,listener)){
			free (listener);
			return 0;
		}
	}
	return listener;
}

uint8_t esp8266_removeListenerFunction(void* ptr, void* params) {
	esp8266_listener_t* listener = (esp8266_listener_t*) ptr;
	esp8266_listener_t* paramListener = *((esp8266_listener_t**) params);
	return listener == paramListener;
}
void esp8266_removeListener(esp8266_listener_t* listener) {
	esp8266_t* esp8266 = g_esp8266_main;
	esp8266_listener_t* removed = list_remove(&esp8266->listeners, listener);
	free(removed);
}
//void esp8266_removeListener(esp8266_t* console, esp8266_listener_t* listener) {
//	void* ptr = list_removeIf(esp8266_removeListenerFunction, &listener);
//	free(ptr);
//}

static void esp8266_startup4D(void** params)
{
	if (esp8266_hw_isPowerOn())
	{
		schedule_cancel(ESP8266_STARTUP_SCHEDULE_ID);
		schedule_once(ESP8266_STARTUP_SCHEDULE_ID, esp8266_comandInit, 500, 0);
		
	}
	else
	{
		schedule_once(ESP8266_STARTUP_SCHEDULE_ID, esp8266_startup4D, 500, 0);
	}

}
static uint8_t esp8266Smart4R(void** bytes, void**params) {
	esp8266_t* esp8266 = g_esp8266_main;
		uint8_t* buffer = *(uint8_t**)bytes[0];
	int16_t size = *(int16_t*)bytes[1];
	esp8266_status_t esp8266Status = esp8266->status;
	uint8_t err = 0;
	uint8_t match = 0;
	err = esp8266_common4R(buffer, size);
	if (err < 3) {

		esp8266Status = ESP8266_STATUS_smarting;
		match = 1;		
	}
	else if (err == 3)
	{
		//match = 0;
		schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 0, 0);
	}
	
	esp8266_setStatus(esp8266Status);
//	else if (string_equal((char const*)command, "CWSTOPSMART")) {

//		schedule_once(ESP8266_RESET_SCHEDULE_ID, esp8266_reset, 30000, 0);
//		esp8266Status = ESP8266_STATUS_initOK;
//	}
	return match;

}
void esp8266_smartStop(void)
{
	esp8266_command("AT+CWSTOPSMART\r\n",1000,2,0,0);
}
void esp8266_smart(uint32_t lastingMs)
{
	//esp8266_t* esp8266 = g_esp8266_main;
	g_esp8266_main->smartNoApTick = tick_ms() + lastingMs;
//	esp8266_command("AT+CIPCLOSE=0\r\n",1000,2,0,0);
//	esp8266->sockets->
	esp8266_command("AT+CWSTARTSMART=3\r\n",2000,3,esp8266Smart4R,0); 
}

void esp8266_init(esp8266_t* esp8266) {
	g_esp8266_main = esp8266;
	esp8266_hw_init();
	
	stream_add(esp8266->inStream);
	stream_add(esp8266->outStream);
	stream_addListener(esp8266->inStream, &g_esp8266_lineListener);
	stream_addListener(esp8266->inStream, &g_esp8266_tickListener);

	esp8266_addListener(esp8266Status4D, 0);
	esp8266_addListener(esp8266Command4D, 0);
	esp8266_addListener(esp8266Package4D, 0);
//	esp8266_addListener(esp8266_connected4D, 0);
//	esp8266_addListener(esp8266Status4D, 0);
	esp8266_reset(0);
}
void esp8266_deinit(esp8266_t* esp8266) {
	
//	esp8266_reset(0);
//	stream_removeListener
	g_esp8266_main = NULL;
	esp8266_hw_powerOff(0);
	
}
