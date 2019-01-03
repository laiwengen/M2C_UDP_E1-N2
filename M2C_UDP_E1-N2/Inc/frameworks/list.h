#ifndef LIST_H__
#define LIST_H__
#include "stdint.h"

typedef uint8_t(*list_conditionFunction_t)(void* this, void** params);

typedef struct package_element_t package_element_t;
typedef struct package_pair_t package_pair_t;
typedef struct thread_t thread_t;

typedef union list_item_t {
  thread_t* thread;
  package_element_t* pe;
  package_pair_t* pp;
  void* raw;
} list_item_t;

typedef struct list_t {
  list_item_t this;
  struct list_t* next;
} list_t;

typedef struct list_loop_t {
  list_t* l;
  list_t* n;
  uint8_t k;
} list_loop_t;

#define fortas(type, array, size) for (int16_t i = 0, _k = 1, _s = (size); _k && i < _s; _k = !_k, i++) \
  for(type v = *(((type*)array) + i); _k; _k = !_k)
#define forta(type, array) fortas(type, array, sizeof(array)/ sizeof(type))
#define fors(size) for(int16_t i = 0, _s = (size); i < _s; i++)
#define foris(i,size) for(int16_t i = 0, _s = (size); i < _s; i++)
#define forsei(start,end,inc) for(int16_t i = (start), _e = (end), _i = (inc); (inc>0)?(i < _e):(i > _e); i += _i)

#define fortl(type, list) for (list_loop_t _ = {*(list), (*(list))->next, 1}; _.k && _.l; _.k = !_.k, _.l = _.n, _.n = _.n->next) \
  for(type v = (type)_.l->this.raw; _.k; _.k = !_.k) \
    if (v)

#define forl(list) for (list_t* l = *(list), *_n = l->next; l; l = _n, _n = _n->next)

list_t* list_add(list_t** list, void* e);
#define list_addLast(list, e) list_add(list,e)

list_t* list_addFirst(list_t** list, void* e);

#define list_add0(list, e) list_addFirst(list,e)

list_t* list_addAfter(list_t** list, void* e, void* after);

void* list_remove(list_t** list, void* e);

void* list_removeByIndex(list_t** list, int16_t n);

void* list_removeFirst(list_t** list);

void* list_removeLast(list_t** list);

void* list_removeIf(list_t** list, list_conditionFunction_t function, void** params);

void* list_findIf(list_t** list, list_conditionFunction_t function, void** params);

void* list_findByBytes(list_t** list, uint8_t* bytes, int16_t start, int16_t size);
/////////!!!!! ID MUST BE THE FIRST 4BYTE 32 INT
void* list_findById(list_t** list, uint32_t id);

int16_t list_size(list_t** list);

void* list_peekFirst(list_t** list);

void* list_peekLast(list_t** list);

void* list_peekIndex(list_t** list, int16_t index);

uint32_t list_generatId(list_t** list, uint32_t* generator);

#endif
