
#include "frameworks/ds18b20.h"
#include "frameworks/list.h"

void ds18b20_hw_delay(uint32_t us);
void ds18b20_hw_write(uint32_t id, uint8_t set);
uint8_t ds18b20_hw_read(uint32_t id);
void ds18b20_hw_init(uint32_t id);


static uint8_t reset(uint32_t id) {
  ds18b20_hw_write(id, 0);
  ds18b20_hw_delay(480);
  ds18b20_hw_write(id, 1);
  ds18b20_hw_delay(80);
  uint8_t flag = 0;
  flag = ds18b20_hw_read(id);
  ds18b20_hw_delay(400);
  return flag;
}

static void write(uint32_t id, uint8_t data) {
  fors (8) {
    ds18b20_hw_write(id, 0);
    ds18b20_hw_delay(4);
    ds18b20_hw_write(id, (data & (1<<i))?1:0);
    ds18b20_hw_delay(60);
  }
  ds18b20_hw_write(id, 1);
}

static uint8_t read(uint32_t id) {
  uint8_t toRet = 0;
  fors (8) {
    ds18b20_hw_write(id, 0);
    ds18b20_hw_delay(4);
    ds18b20_hw_write(id, 1);
    ds18b20_hw_delay(8);
    if (ds18b20_hw_read(id)) {
      toRet |= 1<<i;
    }
    ds18b20_hw_delay(60);
  }
  ds18b20_hw_write(id, 1);
  return toRet;
}

static void init(uint32_t id) {
  reset(id);
  write(id, 0xcc);
  write(id, 0x4e);
  write(id, 0x20);
  write(id, 0x7f);
  // reset(id);
}

static uint16_t get(uint32_t id) {
  uint16_t toRet = 0;
  reset(id);
  write(id, 0xcc);
  write(id, 0x44);
  reset(id);
  write(id, 0xcc);
  write(id, 0xbe);
  toRet = read(id);
  toRet |= read(id) << 8;
  return toRet;
}

uint16_t ds18b20_get(uint32_t id) {
  return get(id);
}

void ds18b20_init(uint32_t id) {
  ds18b20_hw_init(id);
  init(id);
}


void ds18b20_enable(uint32_t id, uint8_t enable) {
  ds18b20_hw_enable(id, enable);
}
