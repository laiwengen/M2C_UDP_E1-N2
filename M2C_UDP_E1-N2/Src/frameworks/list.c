#include "string.h"
#include "stdlib.h"
#include "frameworks/list.h"

list_t* list_add(list_t** list, void* e) {
  list_t* el = (list_t*)malloc(sizeof(list_t));
  if (el == 0) {
    return 0;
  }
  el->this.raw = e;
  el->next = 0;
  list_t* l = *(list);
  if (l == 0) {
    *(list) = el;
  } else {
    while (l->next) {
      l = l->next;
    }
    l->next = el;
  }
  return el;
}

list_t* list_addFirst(list_t** list, void* e) {
  list_t* el = (list_t*)malloc(sizeof(list_t));
  if (el == 0) {
    return 0;
  }
  el->this.raw = e;
  el->next = *(list);
  *(list) = el;
  return el;
}


list_t* list_addAfter(list_t** list, void* e, void* after) {
  list_t* el = (list_t*)malloc(sizeof(list_t));
  if (el == 0) {
    return 0;
  }
  el->this.raw = e;
  el->next = 0;
  list_t* l = *(list);
  if (l == 0) {
    *(list) = el;
  } else {
    while (l->next) {
      if (l->this.raw == after) {
        el->next = l->next;
        l->next = el;
        return el;
      }
      l = l->next;
    }
    l->next = el;
  }
  return el;
}


void* list_remove(list_t** list, void* e) {
  list_t* l = *(list);
  list_t** ll = list;
  while (l) {
    if (l->this.raw == e) {
      *ll = l->next;
      void* toRet = l->this.raw;
      free(l);
      return toRet;
    }
    ll = &(l->next);
    l = l->next;
  }
  return 0;
}

void* list_removeFirst(list_t** list) {
  list_t* l = *(list);
  list_t** ll = list;
  if (l) {
    *ll = l->next;
    void* toRet = l->this.raw;
    free(l);
    return toRet;
  }
  return 0;
}

void* list_removeLast(list_t** list) {
  list_t* l = *(list);
  list_t** ll = list;
  if (l) {
    void* toRet;
    if (l->next == 0) {
      *ll = 0;
      toRet = l->this.raw;
      free(l);
    } else {
      while (l->next->next) {
        l = l->next;
      }
      toRet = l->next->this.raw;
      free(l->next);
      l->next = 0;
    }
    return toRet;
  }
  return 0;
}


void* list_peekIndex(list_t** list, int16_t n) {
  list_t* l = *(list);
  if (n<0) {
    n += list_size(list);
  }
  int16_t i = 0;
  while (l) {
    if (i == n) {
      return l->this.raw;
    }
    l = l->next;
    i ++;
  }
  return 0;
}

void* list_removeByIndex(list_t** list, int16_t n) {
  list_t* l = *(list);
  list_t** ll = list;
  if (n<0) {
    n += list_size(list);
  }
  int16_t i = 0;
  while (l) {
    if (i == n) {
      *ll = l->next;
      void* toRet = l->this.raw;
      free(l);
      return toRet;
    }
    ll = &(l->next);
    l = l->next;
    i ++;
  }
  return 0;
}

void* list_removeIf(list_t** list, list_conditionFunction_t function, void** params) {
  list_t* l = *(list);
  list_t** ll = list;
  while (l) {
    if (function(l->this.raw, params)) {
      *ll = l->next;
      void* toRet = l->this.raw;
      free(l);
      return toRet;
    }
    ll = &(l->next);
    l = l->next;
  }
  return 0;
}

void* list_findIf(list_t** list, list_conditionFunction_t function, void** params) {
  list_t* l = *(list);
  while (l) {
    if (function(l->this.raw, params)) {
      return l->this.raw;
    }
    l = l->next;
  }
  return 0;
}

uint8_t list_findByBytes4D(void* this, void** params){
  uint8_t* bytes = *((uint8_t**)params[0]);
  int16_t start = *((uint16_t*)params[1]);
  int16_t size = *((int16_t*)params[2]);
  return memcmp((uint8_t*)this+start, bytes, size) == 0;
}

void* list_findByBytes(list_t** list, uint8_t* bytes, int16_t start, int16_t size) {
  void* params[] = {&bytes,&start,&size};
  return list_findIf(list, list_findByBytes4D, params);
}

/////////!!!!! ID MUST BE THE FIRST 4BYTE 32 INT
void* list_findById(list_t** list, uint32_t id) {
  return list_findByBytes(list, (uint8_t*)&id, 0, 4);
}

int16_t list_size(list_t** list) {
  int16_t size = 0;
  list_t* l = *(list);
  while (l) {
    size ++;
    l = l->next;
  }
  return size;
}

void* list_peekFirst(list_t** list) {
  list_t* l = *(list);
  if (l) {
    return l->this.raw;
  }
  return 0;
}

void* list_peekLast(list_t** list) {
  list_t* l = *(list);
  if (l) {
    while (l->next) {
      l = l->next;
    }
    return l->this.raw;
  }
  return 0;
}


uint32_t list_generatId(list_t** list, uint32_t* generator) {
  while (1) {
    uint32_t id = ++ *generator;
    if (id != 0 && list_findById(list, id) == 0) {
      return id;
    }
  }
}
