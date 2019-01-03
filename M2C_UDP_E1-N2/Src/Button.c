#include "stdlib.h"
#include "frameworks/button.h"

typedef struct button_t {
	uint32_t id;
	button_pressedFunction_t pressedFunction;
	uint32_t riseTick1;
	uint32_t riseTick0;
	uint32_t fallTick1;
	uint32_t fallTick0;
	uint32_t clearTick;
} button_t;

typedef struct buttonListener_t {
	uint32_t buttonId;
	button_event_t event;
	button_eventFunction_t function;
	void** params;
} buttonListener_t;

list_t* g_button_buttonHead = 0;
list_t* g_button_listenerHead = 0;
volatile uint32_t g_button_ticks = 0;

static button_t* getButtonById(uint32_t id) {
	return (button_t*)list_findById(&g_button_buttonHead, id);
}

void button_init(uint32_t id, button_pressedFunction_t function) {
	button_t* button = getButtonById(id);
	if (button) {
		button = list_remove(&g_button_buttonHead, button);
	} else {
		button = malloc(sizeof(button_t));
	}
	if (button) {
		button->id = id;
		if (function(id)){
			button->riseTick1 = g_button_ticks-4;
			button->fallTick1 = g_button_ticks-3;
			button->riseTick0 = g_button_ticks-2;
			button->fallTick0 = g_button_ticks-1;
		} else {
			button->fallTick1 = g_button_ticks-4;
			button->riseTick1 = g_button_ticks-3;
			button->fallTick0 = g_button_ticks-2;
			button->riseTick0 = g_button_ticks-1;
		}
		button->clearTick = g_button_ticks;
		button->pressedFunction = function;
		if (!list_addFirst(&g_button_buttonHead, button)) {
			free(button);
		}
	}
}
void button_deinit(uint32_t id) {
	button_t* button = getButtonById(id);
	if (button) {
		button = list_remove(&g_button_buttonHead, button);
		free(button);
	}
}

#define downtime0(b) (g_button_ticks-b->fallTick0)
#define uptime0(b) (g_button_ticks-b->riseTick0)
#define downtime1(b) (b->riseTick0-b->fallTick0)
#define uptime1(b) (b->fallTick0-b->riseTick0)
#define downtime2(b) (b->riseTick0-b->fallTick1)
#define uptime2(b) (b->fallTick0-b->riseTick1)
#define inSequence(f,t) ((t)-(f)<(UINT32_MAX>>1))
#define pressed(b) (inSequence(b->riseTick0,b->fallTick0) && inSequence(b->fallTick0,g_button_ticks))
#define released(b) (inSequence(b->fallTick0,b->riseTick0) && inSequence(b->riseTick0,g_button_ticks))
#define returnIfNotCleared(b, t) if (inSequence(b->clearTick + 1, (t))) {\
	return g_button_ticks - (t);\
}

uint32_t button_ticksFromLastEvent(uint32_t id, button_event_t event) {
	button_t* b = getButtonById(id);
	if (!b)
	{
		return 0;
	}
	switch(event)
	{
		case (BUTTON_EVENT_CLICKED): {
			if (released(b) && downtime1(b) > button_ticks(click) && downtime1(b) <= button_ticks(hold)) {
				returnIfNotCleared(b, b->riseTick0);
			}
		}
		break;
		case (BUTTON_EVENT_DOUBLE_CLICKED): {
//			if (released(b) && downtime0(b) > button_ticks(click) && downtime0(b) <= button_ticks(hold) && 
//			uptime1(b) <= button_ticks(doubleInterval) && downtime2(b) > button_ticks(click) && downtime2(b) <= button_ticks(hold)) {
	if (released(b) && downtime0(b) > button_ticks(click) && downtime0(b) <= button_ticks(hold) && \
			uptime2(b) <= button_ticks(doubleInterval) && downtime2(b) > button_ticks(click) && downtime2(b) <= button_ticks(hold)) {

				returnIfNotCleared(b, b->riseTick0);
			}
		}
		break;
		case (BUTTON_EVENT_CLICKED_NO_NEXT): {
			if (released(b) && uptime0(b) > button_ticks(doubleInterval) && downtime1(b) > button_ticks(click) && downtime1(b) <= button_ticks(hold))
			{
				//returnIfNotCleared(b, b->riseTick0 + button_ticks(doubleInterval));
				returnIfNotCleared(b, b->riseTick0);
			}
		}
		break;
		case (BUTTON_EVENT_HOLD): {
			if (pressed(b) && downtime0(b) > button_ticks(hold)) {
				returnIfNotCleared(b, b->fallTick0 + button_ticks(hold))
			}
		}
		break;
		case (BUTTON_EVENT_LONG_HOLD): {
			if (pressed(b) && downtime0(b) > button_ticks(longHold)) {
				returnIfNotCleared(b, b->fallTick0 + button_ticks(longHold))
			}
		}
		break;
		case (BUTTON_EVENT_CLICKED_HOLD): {
			if (pressed(b) && downtime2(b) > button_ticks(click) && downtime2(b) <= button_ticks(hold)
					&& uptime1(b) <= button_ticks(doubleInterval)
					&& downtime0(b) > button_ticks(hold)) {
				returnIfNotCleared(b, b->fallTick0 + button_ticks(hold))
			}
		}
		break;
		case (BUTTON_EVENT_PRESSED): {
			if (pressed(b)) {
				returnIfNotCleared(b, b->fallTick0);
			}
		}
		break;
		case (BUTTON_EVENT_RELEASED): {
			if (released(b) && (downtime0(b) <= button_ticks(click) || downtime0(b) > button_ticks(hold))) {
				returnIfNotCleared(b, b->fallTick0);
			}
		}
		break;
	}
	return 0;
}

void button_clearStatus(uint32_t id, uint32_t delayed) {
	button_t* b = getButtonById(id);
	if (b) {
		b->clearTick = g_button_ticks - delayed;
	}
}

void button_tick(void** params) {
	g_button_ticks++;
	list_t* l = g_button_buttonHead;
	while(l) {
		button_t* b = (button_t*)l->this.raw;
		if (b) {
			uint8_t lastPressed = pressed(b);
			uint8_t pressed = b->pressedFunction(b->id);
			if (pressed && !lastPressed) {
				b->fallTick1 = b->fallTick0;
				b->fallTick0 = g_button_ticks;
			} else if (!pressed && lastPressed) {
				b->riseTick1 = b->riseTick0;
				b->riseTick0 = g_button_ticks;
			}
		}
		l = l->next;
	}
}
void button_run(void** params) {
	list_t* l = g_button_listenerHead;
	while(l) {
		buttonListener_t* listener = (buttonListener_t*)l->this.raw;
		if (listener) {
			uint32_t delayed = button_ticksFromLastEvent(listener->buttonId,listener->event);
			if(delayed>0) {
				button_clearStatus(listener->buttonId,delayed);
				listener->function(listener->params);
			}
		}
		l=l->next;
	}
}

buttonListener_t* getListener(uint32_t buttonId, button_event_t event) {
	list_t* l = g_button_listenerHead;
	while(l) {
		buttonListener_t* listener = (buttonListener_t*)l->this.raw;
		if (listener) {
			if (listener->buttonId == buttonId && listener->event == event) {
				return listener;
			}
		}
		l=l->next;
	}
	return 0;
}

void button_addListener(uint32_t buttonId, button_event_t event, button_eventFunction_t function, void** params) {
	buttonListener_t* l = getListener(buttonId,event);
	if(l) {
		l = list_remove(&g_button_listenerHead, l);
	} else {
		l = malloc(sizeof(buttonListener_t));
	}
	if (l) {
		l->function = function;
		l->buttonId = buttonId;
		l->event = event;
		l->params = params;
		if (!list_addFirst(&g_button_listenerHead, l)) {
			free(l);
		}
	}
}

void button_removeListener(uint32_t buttonId, button_event_t event) {
	buttonListener_t* l = getListener(buttonId,event);
	if (l) {
		list_remove(&g_button_listenerHead, l);
	}
}
