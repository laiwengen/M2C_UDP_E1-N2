#ifndef SERVO_H__
#define SERVO_H__
#include "stdint.h"


void servo_init(void);
void servo_set(uint8_t id, int32_t degree10);


#endif
