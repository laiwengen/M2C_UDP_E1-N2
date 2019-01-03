#include "frameworks/fetcher.h"
#include "frameworks/ad.h"
#include "stdlib.h"

#define AD_FETCHER_ID 0xa38cf5e8
#define offset(buffer, lenght, type) (type)(((uint8_t*)(buffer))+(lenght))

static uint8_t fetch4D(fetcher_t* f, fetcherTask_t* t);

static fetcher_t g_fetcher = {
  AD_FETCHER_ID,
  0,
  fetch4D,
  0,
};
static uint32_t g_idGenerator = 0;
static fetcherTask_t* g_lastTask = 0;

static uint8_t fetch4D(fetcher_t* f, fetcherTask_t* t) {
  void** params = t->params;
  int16_t size;
  if (params) {
    uint8_t channel = *(uint8_t*)params[0];
    uint8_t resolution = *(uint8_t*)params[1];
    uint8_t sampling = *(uint8_t*)params[2];
    ad_hw_config(channel,resolution,sampling);
    size = t->size >> (resolution>8?1:0);
  } else {
    ad_hw_config(0,12,0);
    size = t->size;
  }
  g_lastTask = t;
  return ad_hw_convert(t->buffer, size);
}

static uint32_t add(uint8_t* buffer, int16_t size, uint8_t channel, uint8_t resolution, uint8_t sampling) {
  fetcherTask_t* task = (fetcherTask_t*) malloc (sizeof(fetcherTask_t) + 16);
  if (task) {
    void** params = offset(task, sizeof(fetcherTask_t),void**);
    task->id = list_generatId(&g_fetcher.tasks, &g_idGenerator);
    task->buffer = buffer;
    task->size = size;
    task->done = 0;
    task->params = params;
    params[0] = offset(params, 12, void*);
    params[1] = offset(params, 13, void*);
    params[2] = offset(params, 14, void*);
    *(uint8_t*)params[0] = channel;
    *(uint8_t*)params[1] = resolution;
    *(uint8_t*)params[2] = sampling;
    fetcher_addTask(&g_fetcher, task);
    fetcher_start(&g_fetcher);
    return task->id;
  }
  return 0;
}

void ad_done(void) {
	fetcherTask_t* done = g_lastTask;
  g_lastTask = 0;
  fetcher_done(&g_fetcher, done);
}

void ad_init(void) {
	static uint8_t inited = 0;
	if (!inited) {
		inited = 1;
		ad_hw_init();
		ad_hw_calibration();
	}
}
void ad_enable(uint8_t enable) {
  ad_hw_enable(enable);
}
uint32_t ad_addSingle(uint8_t channel, uint8_t resolution, uint8_t sampling) {
  int16_t size = 1+(resolution>8?1:0);
  uint8_t* buffer = (uint8_t*) malloc(size);
  if (buffer) {
    uint32_t id = add(buffer, size, channel, resolution, sampling);
    if (id) {
      return id;
    } else {
      free(buffer);
    }
  }
  return 0;
}

uint32_t ad_add(uint8_t* buffer, int16_t size, uint8_t channel, uint8_t resolution, uint8_t sampling) {
  return add(buffer, size, channel, resolution, sampling);
}

int16_t ad_getSingle(uint32_t id) {
  fetcherTask_t* task = list_findById(&g_fetcher.tasks, id);
  if (task) {
    int16_t value = 0;
    if (fetcher_get(&g_fetcher, task, (uint8_t*)&value)) {
      return value;
    }
  }
  return -1;
}

uint8_t* ad_peek(uint32_t id) {
  fetcherTask_t* task = list_findById(&g_fetcher.tasks, id);
  if (task && task->done) {
    return task->buffer;
  }
	return 0;
}

void ad_drop(uint32_t id) {
  fetcherTask_t* task = list_findById(&g_fetcher.tasks, id);
  if (task && task->done) {
    fetcher_get(&g_fetcher, task, 0);
  }
}

void ad_start(void) {
	if (!ad_hw_converting()) {
		fetcher_start(&g_fetcher);
	}
}

uint8_t ad_converting(void) {
  return g_fetcher.fetching;
}
