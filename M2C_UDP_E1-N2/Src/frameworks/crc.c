#include "frameworks/crc.h"
#include "libs/mbcrc/mbcrc.h"

uint16_t crc_modbus(uint8_t* buffer, int16_t size) {
  return usMBCRC16(buffer, size);
}
