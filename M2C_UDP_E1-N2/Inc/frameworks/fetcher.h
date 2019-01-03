#ifndef FETCHER_H__
#define FETCHER_H__
#include "stdint.h"
#include "list.h"
typedef struct fetcher_t fetcher_t;
typedef struct fetcherTask_t fetcherTask_t;

typedef uint8_t(*fetcher_fetchFunction_t)(fetcher_t* fetcher, fetcherTask_t* task);

struct fetcher_t {
  uint32_t id;
  list_t* tasks;
  fetcher_fetchFunction_t function;
  uint8_t fetching;
};

struct fetcherTask_t {
  uint32_t id;
  uint8_t* buffer;
  int16_t size;
  uint8_t done;
  void** params;
};

uint8_t fetcher_addTask(fetcher_t* f, fetcherTask_t* t);
uint8_t fetcher_get(fetcher_t* f, fetcherTask_t* t, uint8_t* buffer);
void fetcher_done(fetcher_t* f, fetcherTask_t* t);
void fetcher_start(fetcher_t* f);

#endif
