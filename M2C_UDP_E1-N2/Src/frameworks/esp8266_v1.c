#inlistenerude "string.h"
#inlistenerude "stdio.h"
#inlistenerude "framework/strings.h"
#inlistenerude "framework/list.h"
#inlistenerude "framework/schedule.h"
#inlistenerude "framework/stream.h"
#inlistenerude "framework/wireless.h"
#inlistenerude "framework/esp8266.h"
#inlistenerude "stm32f0xx_hal.h"

void esp8266_hw_rest(void) {

}

typedef void(*esp8266_receivePackageFunction_t)(wireless_socket_t* socket, char* package);
typedef uint8_t(*esp8266_listenerFunction_t)(uint8_t* bytes, int16_t size);
typedef void(*esp8266_smartLinkFunction_t)(char* ssid, char password);
typedef void(*esp8266_comandFunction_t)(void* params);

typedef struct esp8266_t {
	stream_t* outStream;
	stream_t* inStream;
	list_t* listeners;
	uint32_t smartNoApTick;
	uint32_t resetForCrashTick;
	esp8266_smartLinkFunction_t smartLinkFunction;
	list_t* commands;
	list_t* sockets;
} esp8266_t;

typedef struct esp8266_listener_t {
	uint32_t id;
	esp8266_listenerFunction_t function;
	void* params;
} esp8266_listener_t;

typedef struct esp8266_command_t {
	char* bytes;
	uint16_t size;
	uint16_t timeout;
	uint16_t retries;
	esp8266_comandFunction_t function;
	void* params;
} esp8266_listener_t;

void esp8266_connect(wireless_socket_t* socket);
void esp8266_disconnect(wireless_socket_t* socket);
void esp8266_send(wireless_socket_t* socket, char* data);
void esp8266_sendHttp(wireless_socket_t socket, esp8266_httpRequest_t* request);

esp8266_t* g_esp8266_main;
#define ESP8266_SEND_TOP_SCHEDULE_ID 3485
// uint32_t* g_esp8266_listenerIdGenerator = 0;
void esp8266_init(esp8266_t* esp8266) {
	g_esp8266_main = esp8266;
}

uint8_t esp8266_packageListener(uint8_t* buffer, uint16_t size, void* params) {
	return 0;
}

uint8_t esp8266_smartLinkListener(uint8_t* buffer, uint16_t size, void* params) {
	return 0;
}

uint8_t esp8266_statusListener(uint8_t* buffer, uint16_t size, void* params) {
	return 0;
}

void esp8266_doSend(char* str, int16_t size) {
	stream_write(console->outStream, str, size);
	stream_flush(console->outStream);
}

void esp8266_sendTop(void* params) {
	esp8266_t* esp8266 = g_esp8266_main;
	if (list_size(&esp8266->commands)>0){
		esp8266_command_t* c = (esp8266_command_t*)list_peekFirst(&esp8266->commands);
		esp8266_doSend(c->bytes, c->size);
		if (c->retries > 0) {
			c->retries --;
			schedule_once(ESP8266_SEND_TOP_SCHEDULE_ID, esp8266_sendTop, c->timeout, params);
		} else {
			//drop it
			list_removeFirst(&esp8266->commands);
			esp8266_command_t* c = esp8266_sendTop(params);
			if (c) {
				free(c->bytes);
				free(c);
			}
			esp8266_sendTop(params);
		}
	}
}

void esp8266_onCommandSent(void* params) {
	schedule_cancel(ESP8266_SEND_TOP_SCHEDULE_ID);
	esp8266_t* esp8266 = g_esp8266_main;
	esp8266_command_t* c = (esp8266_command_t*)list_removeFirst(&esp8266->commands);
	if (c) {
		if (c->function) {
			c->function(c->params);
		}
		free(c->bytes);
		free(c);
	}
	esp8266_sendTop(params);
}

void esp8266_out(esp8266_command_t* c) {
	esp8266_t* esp8266 = g_esp8266_main;
	list_add(&esp8266->commands, c);
	if (list_size(&esp8266->commands) == 1) {
		esp8266_sendTop(0);
	}
}

void esp8266_command(char* cmd, uint6_t timeout, uint16_t retries, esp8266_comandFunction_t function, void* params) {
	esp8266_command_t* c = (esp8266_command_t*)malloc(sizeof(esp8266_command_t));
	if (c) {
		c->bytes = string_copy(cmd);
		c->size = strlen(cmd);
		c->timeout = timeout;
		c->retries = retries;
		c->function = function;
		c->params = params;
		esp8266_out(c);
	}
}

enum esp8266_socket_type_t {
	ESP8266_SOCKET_TCP,
	ESP8266_SOCKET_UDP
},
uint8_t esp8266_send(uint8_t type, char* host, uint16_t port, uint8_t* buffer, uint16_t size) {
	esp8266_t* esp8266 = g_esp8266_main;
	int8_t socketCount = list_size(&esp8266->sockets);
	wireless_socket_t* socket = 0;
	char idBuffer[2];
	// find if socket alread exists
	list_t *l = esp8266->sockets;
	while (l) {
		if (l->this){
			wireless_socket_t* s = (wireless_socket_t*) l->this;
			if (s->type == type && strcmp(host,s->host) == 0 && port == s->port) {
				socket = s;
				break;
			}
		}
		l = l->next;
	}
	// start a new one if not exists
	uint8_t needDisconnect = 0;
	if (socket == 0) {
		if (socketCount < 5) {
			socket = (wireless_socket_t*) malloc(sizeof(wireless_socket_t));
			socket->id = socketCount + 1;
		} else {
			socket = list_removeFirst(&esp8266->sockets);
			if (socket) {
				if (socket->connected) {
					needDisconnect = true;
				}
			}
		}
		if (socket) {
			socket->connected = 0;
		}
	}
	// fill id buffer for future use
	if (socket) {
		number_toDecString(socket->id, 0, idBuffer, sizeof(idBuffer));
		if (needDisconnect) {
			uint8_t id = socket->id;
			char idBuffer[2];
			number_toDecString(id, 0, idBuffer, sizeof(idBuffer));
			char* concats[] = {"AT+CIPCLOSE=",idBuffer,"\n"};
			char* c = string_concat(concats,sizeof(concats)/sizof(char*));
			if (c) {
				esp8266_command(c,2000,5,0,0);
				free(c);
			}
		}
	}
	//TODO add setuped for UDP?
	// check if connected
	// even if udp do not need the CONNECT. but this just make sure it has sent the CIPSTART for host and port.
	if (socket && socket->connected == 0) {
		list_add(&esp8266->sockets, socket);
		uint8_t id = socket->id;
		socket->type == type;
		socket->host = string_copy(host);
		socket->port = port;
		char portBuffer[6];
		number_toDecString(port, 0, portBuffer, sizeof(portBuffer));
		char* concats[] = {"AT+CIPSTART=",idBuffer,",\"",type==ESP8266_SOCKET_TCP?"TCP":"UDP","\",\"",host,"\",",portBuffer,"\n"};
		char* c = string_concat(concats,sizeof(concats)/sizof(char*));
		if (c) {
			esp8266_command(c,2000,5,0,0);
			free(c);
		}
	}
	// send data
	if (socket) {
		{
			char lengthBuffer[6];
			number_toDecString(size, 0, lengthBuffer, sizeof(lengthBuffer));
			char* concats[] = {"AT+CIPSENDEX=",idBuffer,",",lengthBuffer,"\n"};
			char* c = string_concat(concats,sizeof(concats)/sizof(char*));
			if (c) {
				esp8266_command(c,2000,5,0,0);
				free(c);
			}
		}
		{
			esp8266_command_t* c = (esp8266_command_t*)malloc(sizeof(esp8266_command_t));
			char* bytes = (char*)malloc(size);
			if (c && bytes) {
				memcpy(bytes, buffer, size);
				c->bytes = bytes;
				c->size = size;
				c->timeout = 2000;
				c->retries = 5;
				c->function = 0;
				c->params = 0;
				esp8266_out(c);
			}
		}
	}

}

void esp8266_comandInit() {
	esp8266_command("AT+CWMODE_DEF=1\n",2000,5,0,0); // 1: client,2: host, 3: both
	esp8266_command("AT+CWAUTOCONN=1\n",2000,5,0,0); //
	esp8266_command("AT+CIPMODE=0\n",2000,5,0,0);
	esp8266_command("AT+SAVETRANSLINK=0\n",2000,5,0,0);
	esp8266_command("AT+CIPMUX=1\n",2000,5,0,0);
}

uint8_t esp8266_commandListener(uint8_t* buffer, uint16_t size, void* params) {
	uint8_t done = 0;
	if (done) {
		schedule_once(ESP8266_SEND_TOP_SCHEDULE_ID, esp8266_onCommandSent, 100, 0);
		return 1;
	}
	return 0;
}

void esp8266_streamFunction(stream_t* stream, stream_listener_t* listener, uint8_t* buffer, int16_t size) {
	esp8266_t* esp8266 = g_esp8266_main;
	size = removeBackspace(buffer,size);
	uint8_t type = listener->type;
	switch (listener->type) {
		case STREAM_LISTENER_TRIGGER_BIT_endByte: {
			list_t* list = esp8266->listeners;
			while (list) {
				if (list->this) {
					esp8266_listener_t listener = (esp8266_listener_t) list->this;
					if (listener->function(buffer, size, listener->params)){
						break;
					}
				}
				list = list->next;
			}
		}
		break;
	}
}

esp8266_listener_t* esp8266_addListener(esp8266_listenerFunction_t function, void* params) {
	esp8266_listener_t* listener = (esp8266_listener_t*)malloc(sizeof(esp8266_listener_t));
	if (listener) {
		// listener->id = g_esp8266_listenerIdGenerator++;
		listener->function = function;
		listener->params = params;
		if (!list_add0(&console->listeners,listener)){
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

void esp8266_removeListener(esp8266_t* console, esp8266_listener_t* listener) {
	void* ptr = list_removeIf(esp8266_removeListenerFunction, &listener);
	free(ptr);
}

uint8_t esp8266_disconncet(uint8_t force)
{
	if (!g_esp8266_smartStarted)
	{
		uint8_t status = getStatus(0);
		if(force || status == 3)
		{
			setCommandWait(0,"CIPCLOSE",NULL);
			if(waitUntilOk(500))
			{
				return 1;
			}
		}
	}
	return 0;
}

void esp8266_reset(void)
{
	reset();
}
char* esp8266_getSSID(void)
{
	if (g_esp8266_ssidString == NULL)
	{
		if(!g_esp8266_smartStarted)
		{
			char* get = getCommand(200,"CWJAP_DEF?");
			if(get)
			{
				char* start,*end;
				start = strstr(get,"\"");
				end = strstr(get,"\",\"");
				if(start && end && start+1<end)
				{
					char* ssid = malloc(end-start);
					*end = '\0';
					strcpy(ssid,start+1);
					g_esp8266_ssidString = ssid;
				}
				free(get);
			}
		}
	}
	if (g_esp8266_ssidString)
	{
		char* ssid = malloc(strlen(g_esp8266_ssidString)+1);
		strcpy(ssid,g_esp8266_ssidString);
		return ssid;
	}
	return NULL;
}
char* esp8266_getReceiveString(void)
{
	char* str = g_esp8266_receiveString;
	g_esp8266_receiveString = NULL;
	return str;
}
void esp8266_test(void)
{
	setCommandWait(1000,"GMR",NULL);
//	char* ssid = "\"AIRJ_NB\"";
//	setCommandWait(20000,"CWJAP_DEF",ssid,"\"airj2123\"",NULL); //default ssid.
//	setCommandWait(0,"CIUPDATE",NULL);
//	if(waitUntilStrcmp(5000,"+CIPUPDATE:4"))
//	{
//		waitUntilStrstr(60000,"jump to run user");
//	}

//	setCommandWait(0,"CWSAP","\"AIRJESPAIRJ\"","\"airj2123\"","11","3",NULL);
}

uint8_t esp8266_setSsidAndPassword(char *ssid, char *password)
{
	char rssid[34], rpassword[34];
	sprintf(rssid,"\"%s\"",ssid);
	sprintf(rpassword,"\"%s\"",password);
	setCommandWait(1,"CWJAP_DEF",rssid,rpassword,NULL);
	return waitUntilOk(20000)>0;
}










setCommandWait(5000,"CWMODE_DEF","1",NULL);  //sta mode.   ��˼�ǣ�at+CWMODE_DEF=1
//*
//	char* ssid = "\"AIRJ_NB\"";
//	setCommandWait(20000,"CWJAP_DEF?",NULL);
//	setCommandWait(20000,"CWJAP_DEF",ssid,"\"airj2123\"",NULL); //default ssid.
//*/
setCommandWait(5000,"CWAUTOCONN","1",NULL);  //auto connect after reset.
//		setCommandWait(5000,"CIPSTART","\"TCP\"",g_esp8266_tcpServerIP,g_esp8266_tcpServerPort,NULL);  //set tcp target
setCommandWait(5000,"CIPMODE","0",NULL);  //
setCommandWait(5000,"SAVETRANSLINK","0",NULL);  //
setCommandWait(5000,"CIPMUX","0",NULL);  //
reset();//����ģ������ǰ��������wifi����wifiģ��Ӳ����������ʹģ������wifi�����ã�������ǰû������wifi����ʲô����ִ��
//Step 4: wait until the "ready" appeared.
hw_delay(500);
waitUntilRxStopped(5000,100);//MCU�ڵȴ�ʱ��ms��û�н�������ʱ������0����û�еȹ�msʱ���ڣ������ν������ݼ���ʱ������intevalʱ������ʣ���ɵȴ�ʱ��timeOutTick - currentTick
readFocus();//?????�������յ�������
//Step 5: check if in TM mode.
printf("\n");
hw_delay(1000);
uint8_t test = lastIndex();//???
char c = bufferLastChar();//????
if (c != '\n' )
{
	//Setp 5.1: if not in TM mode. Set the default properties.
	setATMode();//���Ǻ��⣿��
}
g_esp8266_isAtMode = 0;//�Ƿ���͸��ģʽ�£��Ѳ��ã��Լ���������������
//		reset();  // reset to enter tm mode.
//		waitUntilRxStopped(waitUntilBlankLine(5000),25);
//		readFocus();




cJSON * g_esp8266_responseJson = NULL;

static uint32_t waitUntilBlankLine(uint32_t ms)
{
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if (strlen(line) == 0)
			{
				free(line);
				return timeOutTick - currentTick;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 0;
}


static uint32_t waitUntilRxStopped(uint32_t ms, uint16_t interval)//������˵��ms��wait������ʱ�䣬interval������ģ�鷵�ؼ���������ʱ�䣻��MCU����msʱ��û�н�������ʱ������0��
													//��û�еȹ�msʱ���ڣ������ν������ݼ���ʱ������intevalʱ�����أ�����ֵΪʣ���ɵȴ�ʱ��timeOutTick - currentTick
{
	uint32_t timeOutTick = getTick() + ms;//ֻ����һ�Σ��������к�Ϊ��ֵ������������������һ��������getTick�йأ�����Щ�����ǲ��ϸ��µģ�����currentTick = getTick();
	uint32_t lastRxTick = getTick();
	uint32_t currentTick = getTick();
	uint16_t i = lastIndex(),j;
	while(timeOutTick - currentTick< (UINT32_MAX>>1))//�൱��currentTickһֱ�ڸ��£�ֱ�����µ�timeOutTickС��currentTickΪֹ
	{
		j = i;
		i = lastIndex();//lastIndex()=��ESP8266_BUFFER_SIZE-*(g_esp8266_dmaRemain)-1��& ESP8266_BUFFER_MASK;*(g_esp8266_dmaRemain)Ϊ���յ����ݵ�������
										//i����һ���̶��ϱ������ݽ�����������������Ϊ�ο���ô��ʾ�����󣿣���
		if(i!=j)//�����д����Ľ��գ���buffer��Ϊ�������ݵĿռ䷢���仯����i!=j,������lastRxTick
		{
			lastRxTick = getTick();
		}
		if (currentTick - (lastRxTick + interval) < (UINT32_MAX>>1))//����currentTickһֱ���£������ϴν������ݵ�ʱ��Ҳ��һֱ���µģ�ֻ�е��ϴν�������ʱ�䲻�ٸ��£�
																																//Ҳ���ǲ��ٽ�������ʱ������������Ч�����ԣ���code��˼�����δ�������ʱ������intervalʱ�����صȴ�ʱ����ʣ��
		{
			return timeOutTick - currentTick;
		}
		currentTick = getTick();//�����Ƕ�currentTick�ĸ���
	}
	return 0;
}

static uint8_t waitUntilOk(uint32_t ms)//getTick�ǰ����ڻ�ȡ��ʱ��ʱ��      //�˺���������ȷ��ָ��ͳɹ������ɹ��򷵻ط���ֵ�������ɹ����򷵻���ֵ
{
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if (strcmp(line,"OK") == 0 || strcmp(line,"SEND OK") == 0)
			{
				free(line);
				return timeOutTick - currentTick;
			}
			if (strcmp(line,"ERROR") == 0 || strcmp(line,"busy s...") == 0)
			{
				free(line);
				return 0;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)//ֱ��д��3�����𣿣���
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 0;
}
static uint8_t waitUntilStrcmp(uint32_t ms, char* str)
{
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if (strcmp(line,str) == 0)
			{
				free(line);
				return timeOutTick - currentTick;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 0;
}

static uint8_t waitUntilStrstr(uint32_t ms, char* str)
{
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if (strstr(line,str) != 0)
			{
				free(line);
				return timeOutTick - currentTick;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 0;
}
static void vSetCommand(char * commond,va_list argptr)
{
	char* connect = "=";
	char * para;
	if (commond!=NULL)
	{
		printf("AT+%s",commond);
		while ((para = va_arg(argptr,char*))!=NULL)
		{
			printf("%s%s",connect,para);
			connect = ",";
		}
	}
	else
	{
		printf("AT");
	}
	printf("\r\n");
}
static void setCommand(char* commond, ...)
{
	va_list argptr;
	va_start(argptr, commond);
	vSetCommand(commond,argptr);
	va_end(argptr);
}

static void setCommandWait(uint32_t ms, char * commond,...)
{
	waitUntilRxStopped(10,2);
	readFocus();
	va_list argptr;
	va_start(argptr, commond);
	vSetCommand(commond,argptr);
	va_end(argptr);
	waitUntilRxStopped(waitUntilBlankLine(ms),25);
}
char* getCommand(uint32_t ms, char* commond)
{
	printf("AT+%s\r\n",commond);
	uint32_t timeOutTick = getTick() + ms;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if(line[0] == '+')
			{
				uint16_t i;
				for(i = 1; i<strlen(line);i++)
				{
					if(line[i] == ':' || line[i] == '=')
					{
						break;
					}
				}
				uint16_t size = strlen(line) - i;
				if (size)
				{
					char* rst = malloc(size);
					if (rst)
					{
						strcpy(rst,line+i+1);
					}
					free(line);
					return rst;
				}
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return NULL;
}

static void setATMode(void)
{
	hw_delay(500);
	readFocus();
	printf("+++");
	hw_delay(500);
	printf("\n");
	setCommandWait(5000,NULL);
	setCommandWait(5000,NULL);
	g_esp8266_isAtMode = 1;
}

static void reset(void)//����ģ������ǰ��������wifi����wifiģ��Ӳ����������ʹģ������wifi�����ã�������ǰû������wifi����ʲô����ִ��
{
	hw_reset();//ע��g_esp8266_smartStarted��wifi���õı�־λ����ʾ�ǵ��ǵ�wifi����ʱ��־λΪ1�������Ǳ�ʾ����־λΪ1ʱ����wifi���ã����Ծ���ģ��Ӳ���ã�����־λ�Ծ�Ϊ1��ֻ��
						 //wifi���õĺ����Ѿ��ж���
	if (g_esp8266_isAtMode)//g_esp8266_isAtMode��ʲô��־������
	{
	//	setATMode();
	}
	if (g_esp8266_smartStarted)
	{
		uint32_t smartRemain = g_esp8266_smartTick - getTick();//ע����������hw_reset����ֻ��wifiģ����Ӳ�����������ᵼ��MCU��ʧ���ݣ���������ǰ��wifi���ú���smart�����е�
																													 //����ʱ��ms���ɴ��ڣ�Ҳ�������ڼ������� smartRemain = g_esp8266_smartTick - getTick()�����Խ������ٴ�����smart����ʱ������esp8266_smart(smartRemain)����
		if (smartRemain<(UINT32_MAX>>1) && smartRemain>0)
		{
			esp8266_smart(smartRemain);//
		}
	}
	g_esp8266_resetTick = getTick();
}

uint8_t getStatus(uint8_t force)
{
	if (!force)
	{
		if (g_esp8266_status == 3)
		{
			return g_esp8266_status;
		}
	}
	setCommand("CIPSTATUS",NULL);
	uint32_t timeOutTick = getTick() + 1000;
	uint32_t currentTick = getTick();
	while(timeOutTick - currentTick< (UINT32_MAX>>1))
	{
		char* line = readNewLine();
		if (line)
		{
			if(strstr(line,"STATUS:") && strlen(line) == 8)
			{
				g_esp8266_status = line[7] - '0';
				free(line);
				return g_esp8266_status;
			}
			for (uint8_t i = 0; i< sizeof(g_esp8266_focus)/sizeof(focus_t); i++)
			{
				if (g_esp8266_focus[i](line))
				{
					break;
				}
			}
			free(line);
		}
		currentTick = getTick();
	}
	return 5;

}
void esp8266_run(void)
{
	readFocus();//?????what's the meaning?
	if(g_esp8266_smartStarted)//Ҳ����һֱ�ڼ���smart�����Ƿ���ʱ������ʱ��ֹͣsmart����ˢ��g_esp8266_resetTick = getTickʱ��
	{
		if ((getTick() - g_esp8266_smartTick)<(UINT32_MAX>>1))//g_esp8266_smartTick=getTick+ms,�ӿ�ʼ���Ͳ���ˢ�£����ԣ�������ʾ����smartlink���̳�ʱ
		{
			esp8266_smartStop();//ע�⣬�˾�����ֹͣsmart�����Ժ󣬻���g_esp8266_smartStarted��0
			reset();//����esp8266_smartStop()����g_esp8266_smartStarted��0����������codeֻ��ˢ��g_esp8266_resetTick��ʱ��
		}
	}
	//esp8266_resetForCrash(60000);
}

void esp8266_sendHTTPRequest(char* path, cJSON* json)
{
	if (g_esp8266_isAtMode)
	{
		return;
	}
	char * str = cJSON_PrintUnformatted(json);
	if (str)
	{
		uint16_t length = strlen(str);
		char* head = "POST %s HTTP/1.1\r\n"
		"Content-Length: %i\r\n"
		"Content-Type: text/plain\r\n"
		"Host: api.hanwanglantian.com\r\n"
		"Connection: keep-alive\r\n"
		"User-Agent: konggan\r\n"
		"Accept: \r\n"
		"\r\n";
		printf(head,path,length);
		printf(str);
//	printf("\r\n");
		free(str);
	}
}

cJSON* esp8266_getHTTPResponse(void)
{
	if (g_esp8266_responseJson)
	{
		cJSON* json = g_esp8266_responseJson;
		g_esp8266_responseJson = NULL;
		return json;
	}
	else
	{
		return NULL;
	}
}

static void softwareInit(void)
{
	//Step 3: reset
	reset();//����ģ������ǰ��������wifi����wifiģ��Ӳ����������ʹģ������wifi�����ã�������ǰû������wifi����ʲô����ִ��
	//Step 4: wait until the "ready" appeared.
	hw_delay(500);
	waitUntilRxStopped(5000,100);//MCU�ڵȴ�ʱ��ms��û�н�������ʱ������0����û�еȹ�msʱ���ڣ������ν������ݼ���ʱ������intevalʱ������ʣ���ɵȴ�ʱ��timeOutTick - currentTick
	readFocus();//?????�������յ�������
	//Step 5: check if in TM mode.
	printf("\n");
	hw_delay(1000);
	uint8_t test = lastIndex();//???
	char c = bufferLastChar();//????
	if (c != '\n' )
	{
		setATMode();
	}
	setCommandWait(5000,"CWMODE_DEF","1",NULL);  //sta mode.   ��˼�ǣ�at+CWMODE_DEF=1
	//*
//	char* ssid = "\"AIRJ_NB\"";
//	setCommandWait(20000,"CWJAP_DEF?",NULL);
//	setCommandWait(20000,"CWJAP_DEF",ssid,"\"airj2123\"",NULL); //default ssid.
	//*/
	setCommandWait(5000,"CWAUTOCONN","1",NULL);  //auto connect after reset.
//		setCommandWait(5000,"CIPSTART","\"TCP\"",g_esp8266_tcpServerIP,g_esp8266_tcpServerPort,NULL);  //set tcp target
	setCommandWait(5000,"CIPMODE","0",NULL);  //
	setCommandWait(5000,"SAVETRANSLINK","0",NULL);  //
	setCommandWait(5000,"CIPMUX","0",NULL);  //
	g_esp8266_isAtMode = 0;//�Ƿ���͸��ģʽ�£��Ѳ��ã��Լ���������������
//		reset();  // reset to enter tm mode.
//		waitUntilRxStopped(waitUntilBlankLine(5000),25);
//		readFocus();
}
static uint8_t smart(int8_t type)//wifi���ú���
{
	if (type == 0)
	{
		setCommandWait(0,"CWSTARTSMART",NULL);
	}
	else
	{
		char buffer[4];
		sprintf(buffer,"%d",type);
		setCommandWait(0,"CWSTARTSMART",buffer,NULL);
	}
	if (waitUntilOk(5000))
	{
		return 1;
	}
	else
	{
		type--;
		if(type == 4 || type == -1)
		{
			return 0;
		}
		else
		{
			return smart(type);//����������Ƕ�ף���type=0,1,2,3ʱ��ѭ��ִ�б�����,����waitUntilOk�з��أ���return1�������޷��أ�����3��1��ʽ��ִ��һ��
		}
	}
}
void esp8266_smart(uint32_t ms)//����wifi����smart(3)�����ѱ�־λg_esp8266_smartStarted��1���趨����ʱ��ms
{
	if(smart(3))//����3��ʽͨѶ����wifi��ESP8266ָ��pdf����û����Ӧ��ָ������Ϊ����pdf�汾̫��
	{
		g_esp8266_smartStarted = 1;//wifi���õı�־λ
		g_esp8266_smartTick = getTick()+ms;//

	}
	readFocus();
}

void esp8266_smartStop(void)//ֹͣsmart���̣�����ָֹͣ����ҵȴ�5s�����Ƿ����ͳɹ���
{
	setCommandWait(5000,"CWSTOPSMART",NULL);
	if (waitUntilOk(5000))
	{
		g_esp8266_smartStarted = 0;
	}
}

uint8_t esp8266_smarting(void)
{
	return g_esp8266_smartStarted;
}
uint8_t esp8266_getStatus(void)
{
	if (g_esp8266_smartStarted)
	{
		return 1;
	}
	if (g_esp8266_status == 2)
	{
		return 4;
	}
	return g_esp8266_status;
}
/*
void esp8266_connectTCP(char* address, uint16_t port)
{
	char buffer[0x10];
	sprintf(buffer,"%d",port);
	char* addressForAT = (char*)malloc(strlen(address)+3);
	sprintf(addressForAT,"\"%s\"",address);
	setCommandWait(5000,"CIPSTART","\"TCP\"",address,buffer,NULL);
	free(addressForAT);
}

uint8_t esp8266_tcp(char * str)
{
//	char* status = getCommand(1000,"CIPSTATUS");
	if (!g_esp8266_smartStarted)
	{
		uint8_t status = getStatus(0);
		if(status == 2 || status == 4 || status == 3)
		{
			esp8266_connectTCP("192.168.2.89",82);
		}
		else if (status == 3)
		{
			char buffer[0x10];
			sprintf(buffer,"%d",strlen(str));
			setCommandWait(0,"CIPSENDEX",buffer,NULL);
			if(waitUntilOk(500))
			{
				printf(str);
				waitUntilOk(5000);
			}
		}
	}
	return esp8266_getStatus();
}*/
