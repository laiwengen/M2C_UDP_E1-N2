#include "frameworks/servo.h"

void servo_hw_init(void);
void servo_hw_set(uint8_t id, int32_t p);


void servo_init(void) {
  servo_hw_init();
}
void servo_set(uint8_t id, int32_t degree10) {
  servo_hw_set(id, degree10 + 1350);
}
