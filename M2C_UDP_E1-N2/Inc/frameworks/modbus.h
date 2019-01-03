#ifndef MODBUS_H__
#define MODBUS_H__
#include "stdint.h"
#include "frameworks/stream.h"
#include "frameworks/crc.h"

typedef struct modbusMaster_t modbusMaster_t;

struct modbusMaster_t {
  uint32_t id;
};
typedef struct modbusSlave_t modbusSlave_t;
typedef struct modbus_listener_t modbus_listener_t;
typedef void(*modbus_receivedFunction_t)(modbusSlave_t* modbus, modbus_listener_t* listener, uint8_t* buffer, int16_t size);

struct modbusSlave_t {
  uint32_t id;
  uint8_t slaveAddress;
  stream_t* outStream;
  stream_t* inStream;
  list_t* listeners;
};

struct modbus_listener_t {
  uint8_t functionCode;
  modbus_receivedFunction_t function;
  void** params;
};


static inline uint8_t modbus_check(uint8_t* bytes, int16_t size) {
  if (size < 2) {
    return 0;
  }
  uint16_t crc = crc_modbus(bytes,size - 2);
  if (bytes[size - 2] == ((crc>>0)&0xff) && bytes[size - 1] == ((crc>>8)&0xff) ) {
    return 1;
  }
  return 0;
}


int16_t modbus_serialize(uint8_t slaveAddress, uint8_t functionCode, uint16_t address, uint16_t value, uint8_t* buffer);
uint8_t modbus_send(uint32_t id, uint8_t* bytes, int16_t size);
uint8_t modbus_receive(uint32_t id, uint8_t* bytes, int16_t size);
uint8_t modbus_response(modbusSlave_t* modbus, uint8_t* bytes, int16_t size);
uint8_t modbus_check(uint8_t* bytes, int16_t size);
void modbus_initMaster(modbusMaster_t* modbus);
void modbus_initSlave(modbusSlave_t* modbus);
void modbus_addListener(modbusSlave_t* modbus, uint8_t functionCode, modbus_receivedFunction_t function, void** params);

uint8_t modbus_hw_receive(modbusMaster_t* modbus, uint8_t* buffer, int16_t size);
uint8_t modbus_hw_send(modbusMaster_t* modbus, uint8_t* buffer, int16_t size);
uint8_t modbus_hw_preSend(void);
uint8_t modbus_hw_postSend(void);
void modbus_hw_initMaster(modbusMaster_t* modbus);
void modbus_hw_initSlave(modbusSlave_t* modbus);

#endif
