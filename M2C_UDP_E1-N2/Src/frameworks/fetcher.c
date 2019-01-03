#include "string.h"
#include "frameworks/fetcher.h"

static void run(fetcher_t* f) {
  // fortl(fetcherTask_t*, &f->tasks) {
  for(list_t* l = f->tasks; l; l = l->next) {
    if (l->this.raw) {
      fetcherTask_t* v = (fetcherTask_t*)l->this.raw;
      if (!v->done) {
        f->fetching = 1;
        f->function(f,v);
        break;
      }
    }
  }
}

uint8_t fetcher_addTask(fetcher_t* f, fetcherTask_t* t) {
  return list_add(&f->tasks, t) != 0;
}

uint8_t fetcher_get(fetcher_t* f, fetcherTask_t* t, uint8_t* buffer) {
  uint8_t fetched = 0;
  if (t->done) {
    if (buffer) {
      memcpy(buffer,t->buffer,t->size);
    }
    t->done = 0;
    fetched = 1;
  }
  if (!f->fetching) {
    run(f);
  }
  return fetched;
}

void fetcher_done(fetcher_t* f, fetcherTask_t* t){
  f->fetching = 0;
	if(t) {
		t->done = 1;
	}
  run(f);
}

void fetcher_start(fetcher_t* f) {
  if (!f->fetching) {
    run(f);
  }
}
