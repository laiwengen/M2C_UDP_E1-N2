#include "stdlib.h"
#include "frameworks/modbus.h"
#include "frameworks/crc.h"
#include "frameworks/stream.h"

static list_t* g_head;

int16_t modbus_serialize(uint8_t slaveAddress, uint8_t functionCode, uint16_t address, uint16_t value, uint8_t* buffer) {
  int16_t index = 0;
  buffer[index++] = slaveAddress;
  buffer[index++] = functionCode;
  buffer[index++] = (address>>8)&0xff;
  buffer[index++] = (address>>0)&0xff;
  buffer[index++] = (value>>8)&0xff;
  buffer[index++] = (value>>0)&0xff;
  uint16_t crc = crc_modbus(buffer,6);
  buffer[index++] = (crc>>0)&0xff;
  buffer[index++] = (crc>>8)&0xff;
  return index;
}

uint8_t modbus_send(uint32_t id, uint8_t* bytes, int16_t size) {
  modbusMaster_t* modbus = (modbusMaster_t*)list_findById(&g_head, id);
  if (!modbus) {
    return 0;
  }
  return modbus_hw_send(modbus, bytes, size);
}

uint8_t modbus_receive(uint32_t id, uint8_t* bytes, int16_t size) {
  modbusMaster_t* modbus = (modbusMaster_t*)list_findById(&g_head, id);
  if (!modbus) {
    return 0;
  }
  // TODO CRC here?
  return modbus_hw_receive(modbus, bytes, size);
}
void modbus_initMaster(modbusMaster_t* modbus) {
  list_add(&g_head, modbus);
  modbus_hw_initMaster(modbus);
}
