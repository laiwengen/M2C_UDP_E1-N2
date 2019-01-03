#include "stm32f0xx_hal.h"
#include "frameworks/number.h"
#include "frameworks/list.h"


// data1 -> pf0
// data2 -> pb9
// data3 -> pb10
// data4 -> pb13

void ds18b20_hw_delay(uint32_t us) {
  fors(us) {
    foris(24);
    nop();
  }
}
void ds18b20_hw_write(uint32_t id, uint8_t set) {

}
uint8_t ds18b20_hw_read(uint32_t id) {
  return 0;
}
void ds18b20_hw_init(uint32_t id) {

}
