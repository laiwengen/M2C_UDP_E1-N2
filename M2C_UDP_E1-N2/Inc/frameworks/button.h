#ifndef BUTTON_H__
#define BUTTON_H__
#include "stdint.h"
#include "frameworks/list.h"

#define BUTTON_TICK_PER_MS 1
#define BUTTON_MS_click 10
#define BUTTON_MS_doubleInterval 300
#define BUTTON_MS_hold 1000
#define BUTTON_MS_longHold 3000

#define button_ticks(k) (BUTTON_MS_##k * BUTTON_TICK_PER_MS)

typedef uint8_t (*button_pressedFunction_t)(uint32_t);
typedef void(*button_eventFunction_t)(void**);

typedef enum{
	BUTTON_EVENT_NONE,
	BUTTON_EVENT_CLICKED,
	BUTTON_EVENT_DOUBLE_CLICKED,
	BUTTON_EVENT_CLICKED_NO_NEXT,
	BUTTON_EVENT_HOLD,
	BUTTON_EVENT_LONG_HOLD,
	BUTTON_EVENT_CLICKED_HOLD,
	BUTTON_EVENT_PRESSED,
	BUTTON_EVENT_RELEASED,
	BUTTON_EVENT_SIZE,
} button_event_t;

void button_init(uint32_t id, button_pressedFunction_t function);
void button_tick(void** params);
void button_run(void** params);
void button_addListener(uint32_t buttonId, button_event_t event, button_eventFunction_t function, void** params);
void button_removeListener(uint32_t buttonId, button_event_t event);

#endif
