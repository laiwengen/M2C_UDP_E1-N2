#include "stdlib.h"
#include "frameworks/thread.h"

list_t* g_thread_head = 0;

static threadGroup_t * findGroupById(uint32_t groupId) {
	return (threadGroup_t*) list_findById(&g_thread_head, groupId);
}

static uint32_t generatId(uint32_t groupId) {
	threadGroup_t* tg = findGroupById(groupId);
	if (tg) {
		return list_generatId(&tg->threads, &tg->idGenerator);
	}
	return 0;
}

threadGroup_t* thread_init(uint32_t groupId, thread_fetchTickFunction_t function) {
	threadGroup_t* tg = findGroupById(groupId);
	if (tg) {
		return tg;
	}
	tg = (threadGroup_t*)malloc(sizeof(threadGroup_t));
	if (tg) {
		tg->id = groupId;
		tg->threads = 0;
		tg->fetchTickFunction = function;
		tg->idGenerator = 0;
		list_add0(&g_thread_head, tg);
	}
	return tg;
}
void thread_deinit(uint32_t groupId) {
	threadGroup_t* tg = findGroupById(groupId);
	if (tg) {
		thread_t* t = list_removeFirst(&tg->threads);
		while (t) {
			free(t);
			t = list_removeFirst(&tg->threads);
		}
		tg = list_remove(&g_thread_head, tg);
		if (tg) {
			free(tg);
		}
	}
}

thread_t* thread_exists(uint32_t groupId, uint32_t tid) {
	threadGroup_t* tg = findGroupById(groupId);
	if (tg) {
		return (thread_t*) list_findById(&tg->threads, tid);
	}
	return 0;
}

uint8_t thread_add(uint32_t groupId, thread_t* t, uint32_t followId) {
	list_t* list = 0;
	threadGroup_t* tg = findGroupById(groupId);
	if (tg) {
		if (t && t->id == 0) {
			t->id = generatId(groupId);
		}
		if (followId == 0) {
			list = list_addLast(&tg->threads,t);
		} else {
			thread_t* follow = (thread_t*) list_findById(&tg->threads, followId);
			if (follow) {
				list = list_addAfter(&tg->threads,t,follow);
			} else {
				list = list_addLast(&tg->threads,t); ///// First or Last?
			}
		}
	}
	return list != 0;
}
uint32_t thread_quickAdd(uint32_t groupId, thread_executeFunction_t function, uint32_t interval, void** params) {
	threadGroup_t* tg = findGroupById(groupId);
	if (tg) {
		thread_t* t = (thread_t*) malloc(sizeof(thread_t));
		if (t) {
			t->id = generatId(groupId);
			t->remainTimes = 0;
			t->intervalTick = interval;
			t->executeTick = 0;
			t->function = function;
			t->params = params;
			if (list_add(&tg->threads,t) == 0) {
				free(t);
			} else {
				return t->id;
			}
		}
	}
	return 0;
}
uint8_t thread_remove(uint32_t groupId, uint32_t tid) {
	threadGroup_t* tg = findGroupById(groupId);
	if (tg) {
		thread_t* t = list_findById(&tg->threads, tid);
		if (t) {
			// t = list_remove(&tg->threads,t);
			t->id = 0;
			return 1;
		}
	}
	return 0;
}

static void execute(threadGroup_t* tg, thread_t* t, uint32_t currentTick) {
	if (tg && t) {
		thread_executeFunction_t function = t->function;
		void** params = t->params;
		if (t->remainTimes == 1) {
			// list_remove(&tg->threads,t);
			// free(t);
			t->id = 0;
			// t->function = 0;
		} else {
			if (t->remainTimes > 0) {
				t->remainTimes --;
			}
			t->executeTick = currentTick + t->intervalTick;
		}
		function(params);
		// if (toRemove && t->function != 0) {
		// 	toRemove = 0; // re added in call
		// }
	}
	// return toRemove;
}

void thread_execute(uint32_t groupId, uint32_t tid) {
	threadGroup_t* tg = findGroupById(groupId);
	if (tg) {
		thread_t* t = list_findById(&tg->threads, tid);
		if (t) {
			execute(tg,t,tg->fetchTickFunction());
		}
	}
}

void thread_run(uint32_t groupId) {
	threadGroup_t* tg = findGroupById(groupId);
	if (tg) {
		uint32_t currentTick = tg->fetchTickFunction();
		fortl(thread_t*, &tg->threads) {
			if (v->id) {
				if ((currentTick - v->executeTick) < INT32_MAX) {
					execute(tg,v,currentTick);
				}
			}
		}
		// list_t ** pl = &tg->threads;
		// fortl(thread_t*, &tg->threads) {
		// 	if (v->id == 0) {
		// 		*pl = _.l->next;
		// 		free(_.l);
		// 		free(v);
		// 		// free(list_remove(&tg->threads,v));
		// 	} else {
		// 		pl = &_.l->next;
		// 	}
		// }
		fortl(thread_t*, &tg->threads) {
			if (v->id == 0) {
				free(list_remove(&tg->threads,v));
			}
		}
	}
}
