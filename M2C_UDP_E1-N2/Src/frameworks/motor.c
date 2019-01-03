#include "frameworks/list.h"
#include "frameworks/schedule.h"
#include "frameworks/plc.h"
#include "frameworks/tick.h"
#include "frameworks/number.h"
#include "frameworks/motor.h"

static uint32_t g_resetTick;
static int32_t g_rpm;

static int32_t ready(plc_t* plc) {
  return motor_hw_peekCounter()>50 || ((int32_t)(tick_ms() - g_resetTick)) > 1000;
}
static int32_t getRpm(plc_t* plc) {
  uint32_t tick = tick_ms();
  if (!ready(plc) || tick == g_resetTick) {
    return g_rpm;
  }
  int32_t toRet = motor_hw_getCounter();
  toRet = toRet * (1000 * 60 / 4)/(int32_t)(tick-g_resetTick);
  g_rpm = toRet;
  g_resetTick = tick;
  return toRet;
}

int32_t motor_hw_rpm2pwm(int32_t rpm);

static int32_t rpm2pwm(plc_t* plc, int32_t rpm) {
	return motor_hw_rpm2pwm(rpm);
}

static int32_t setPwm(plc_t* plc, int32_t pwm) {
  pwm = max(min(pwm,100000),0);
  motor_hw_setPwm(pwm/100);
  // motor_hw_setPwm(200);
  return pwm;
}

static plc_t g_plc = {
  4000,
  0,
  getRpm,
  setPwm,
  rpm2pwm,
  ready,
};


void motor_setTarget(int32_t rpm) {
  g_plc.target = rpm;
}

int32_t motor_getRpm(void) {
  return g_rpm;
}
void motor_enable(uint8_t enable) {
  motor_hw_enable(enable);
  g_resetTick = tick_ms();
  motor_hw_getCounter();
  plc_enable(enable);
}

void motor_init(int32_t pwm) {
  g_plc.set = pwm*100;
  motor_hw_init();
  plc_add(&g_plc);
}
