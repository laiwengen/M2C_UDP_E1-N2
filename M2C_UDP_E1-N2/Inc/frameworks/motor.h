#ifndef MOTOR_H__
#define MOTOR_H__

void motor_setTarget(int32_t rpm);
int32_t motor_getRpm(void);
void motor_tick(void** params);
void motor_enable(uint8_t enable);
void motor_init(int32_t pwm);

void motor_hw_setPwm(uint32_t pwm);
int32_t motor_hw_peekCounter(void);
int32_t motor_hw_getCounter(void);
void motor_hw_init(void);
void motor_hw_enable(uint8_t enable);

#endif
