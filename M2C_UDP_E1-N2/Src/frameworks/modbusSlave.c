#include "stdlib.h"
#include "frameworks/modbus.h"
#include "frameworks/crc.h"
#include "frameworks/stream.h"

static list_t* g_head;

static modbusSlave_t* findModbusSlaveByInStream(stream_t* stream) {
  modbusSlave_t* modbus = 0;
  fortl(modbusSlave_t*, &g_head) {
    if (v->inStream == stream) {
      modbus = v;
      break;
    }
  }
  return modbus;
}

static void modbus_stream4D(stream_t* stream, stream_listener_t* listener, uint8_t* buffer, int16_t size) {
  if (modbus_check(buffer, size)){
    modbusSlave_t* modbus = findModbusSlaveByInStream(stream);
    if (modbus && modbus->slaveAddress == buffer[0]) {
      fortl(modbus_listener_t*, &modbus->listeners) {
        if (v->functionCode == buffer[1]) {
          v->function(modbus, v, buffer, size);
        }
      }
    }
  }
}

uint8_t modbus_response(modbusSlave_t* modbus, uint8_t* bytes, int16_t size) {
  modbus_hw_preSend();
  stream_write(modbus->outStream, bytes, size);
  stream_flush(modbus->outStream);
  return 1;
}

static stream_listener_t g_streamTickListener = {
	STREAM_LISTENER_TYPE_tick,
	modbus_stream4D,
	{.ticks = 3},
	{0}
};

void modbus_addListener(modbusSlave_t* modbus, uint8_t functionCode, modbus_receivedFunction_t function, void** params) {
  modbus_listener_t* l = (modbus_listener_t*) malloc(sizeof(modbus_listener_t));
  if (l) {
    l->functionCode = functionCode;
    l->function = function;
    l->params = params;
    list_add(&modbus->listeners, l);
  }
}

void modbus_initSlave(modbusSlave_t* modbus) {
  list_add(&g_head, modbus);
  modbus_hw_initSlave(modbus);
	stream_add(modbus->inStream);
	stream_add(modbus->outStream);
  stream_addListener(modbus->inStream, &g_streamTickListener);
}
