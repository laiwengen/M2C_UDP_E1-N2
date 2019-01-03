#include "frameworks/list.h"
#include "frameworks/schedule.h"
#include "frameworks/plc.h"

static list_t* g_head;

static void timer(void** parmas) {
  fortl(plc_t*, &g_head) {
    if (v->readyFunction) {
      int32_t resault = v->resaultFunction(v);
      int32_t detal = v->processFunction(v, resault - v->target);
      v->set = v->setFunction(v, v->set+detal);
    }
  }
}
#define TIMER_SCHEDULE_ID 0x5107b4c1

void plc_add(plc_t* plc) {
  list_addLast(&g_head, plc);
}

void plc_enable(uint8_t enable) {
  if (enable){
    schedule_repeat(TIMER_SCHEDULE_ID,timer,1,0);
  } else {
    schedule_cancel(TIMER_SCHEDULE_ID);
  }
}
