#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "frameworks/number.h"
#include "frameworks/package.h"

#if 1

// declear functions
static uint8_t package_allPairsFound(package_element_t* e, list_t* list);

static uint16_t deserialize(uint8_t const* buffer, package_element_t** ep);

// consts
#endif
/*********** INLINES **********/
#if 1

#define package_mallocElement(s) ((package_element_t*) malloc(sizeof(package_element_common_t) + (s)))

#if PACKAGE_ENABLE_INT64
#define valueIntMax(buffer, size) number_valueInt64(buffer, size)
#else
#define valueIntMax(buffer, size) number_valueInt32(buffer, size)
#endif
#endif
/*********** UTILS **********/
#if 1
#endif


/*********** NEW **********/
#if 1

package_element_t* package_newElement(uint8_t type, uint16_t size) {
	// size = max((uint16_t) g_package_sizeByType[type], size);
	package_element_t* e = package_mallocElement(size);
	if (e) {
		e->type = type;
		e->flag = 0;
		e->size = size;
	}
	return e;
}

package_element_t* package_newNull() {
	package_element_t* e = package_newElement(PACKAGE_TYPE_null, 0);
	return e;
}

package_element_t* package_newBoolean(uint8_t value) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_boolean, 1);
	if (e) {
		package_elementBuffer(e, int8_t, 0) = value ? 1 : 0;
	}
	return e;
}

package_element_t* package_newInt(const uint8_t* buffer, uint16_t size) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_integer, size);
	for (int16_t i = 0; i < size; i++) {
		package_elementBuffer(e, uint8_t, i) = buffer[i];
	}
	return e;
}

package_element_t* package_newInt8(int8_t value) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_integer, 1);
	if (e) {
		package_elementBuffer(e, int8_t, 0) = value;
	}
	return e;
}

package_element_t* package_newInt16(int16_t value) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_integer, 2);
	if (e) {
		package_elementBuffer(e, int16_t, 0) = value;
	}
	return e;
}

package_element_t* package_newInt32(int32_t value) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_integer, 4);
	if (e) {
		package_elementBuffer(e, int32_t, 0) = value;
	}
	return e;
}

package_element_t* package_newInt64(int64_t value) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_integer, 8);
	if (e) {
		package_elementBuffer(e, int32_t, 0) = (int32_t) value;
		package_elementBuffer(e, int32_t, 4) = (int32_t) (value >> 32);
	}
	return e;
}

package_element_t* package_newDec8(int8_t value, int8_t e10) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_dec, 2);
	if (e) {
		package_elementBuffer(e, int8_t, 0) = value;
		package_elementBuffer(e, int8_t, 1) = e10;
	}
	return e;
}

package_element_t* package_newDec16(int16_t value, int8_t e10) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_dec, 3);
	if (e) {
		package_elementBuffer(e, int16_t, 0) = value;
		package_elementBuffer(e, int8_t, 2) = e10;
	}
	return e;
}

package_element_t* package_newDec32(int32_t value, int8_t e10) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_dec, 5);
	if (e) {
		package_elementBuffer(e, int32_t, 0) = value;
		package_elementBuffer(e, int8_t, 4) = e10;
	}
	return e;
}

package_element_t* package_newDec64(int64_t value, int8_t e10) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_dec, 9);
	if (e) {
		package_elementBuffer(e, int32_t, 0) = (int32_t) value;
		package_elementBuffer(e, int32_t, 4) = (int32_t) (value >> 32);
		package_elementBuffer(e, int8_t, 8) = e10;
	}
	return e;
}

#if PACKAGE_ENABLE_FLOAT
package_element_t* package_newFloat(float value) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_float, 4);
	if (e) {
		package_elementBuffer(e, float, 0) = value;
	}
	return e;
}
#endif
#if PACKAGE_ENABLE_DOUBLE
package_element_t* package_newDouble(double value) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_double, 8);
	if (e) {
		package_elementBuffer(e, double, 0) = value;
	}
	return e;
}
#endif

package_element_t* package_newString(char const* str) {
	int16_t size = strlen(str);
	package_element_t* e = package_newElement(PACKAGE_TYPE_string, size + 1);
	if (e) {
		memcpy(package_elementPointer(e, char, 0), str, size + 1);
	}
	return e;
}

package_element_t* package_newEmptyString(int16_t size) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_string, size + 1);
	if (e) {
		package_elementBuffer(e, char, 0) = 0;
	}
	return e;
}

package_element_t* package_newStringWithSize(char const* str, int16_t size) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_string, size + 1);
	if (e) {
		memcpy(package_elementPointer(e, uint8_t, 0), str, size);
		package_elementBuffer(e, uint8_t, size) = 0;
	}
	return e;
}

package_element_t* package_newBytes(uint8_t const* bytes, int16_t size) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_bytes, size);
	if (e) {
		memcpy(package_elementPointer(e, uint8_t, 0), bytes, size);
	}
	return e;
}

package_element_t* package_newEmptyBytes(int16_t size) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_bytes, size);
	return e;
}


package_element_t* package_newObject(void) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_object, sizeof(list_t*));
	if (e) {
		package_elementBuffer(e, list_t*, 0) = 0;
	}
	return e;
}

package_element_t* package_newArray(void) {
	package_element_t* e = package_newElement(PACKAGE_TYPE_array, sizeof(list_t*));
	if (e) {
		package_elementBuffer(e, list_t*, 0) = 0;
	}
	return e;
}

package_pair_t* package_newPair(package_element_t* key, package_element_t* value) {
	package_pair_t* pair = (package_pair_t*) malloc(sizeof(package_pair_t));
	if (pair) {
		pair->key = key;
		pair->value = value;
		pair->flag = 0;
	}
	return pair;
}

#endif

/*********** ADD **********/
#if 1
package_pair_t* package_setPair(package_element_t* e, package_pair_t* pair) {
	//assert(e && e->type == PACKAGE_TYPE_object);
  if (e->type != PACKAGE_TYPE_object) {
    return 0;
  }
	if (pair == 0) {
		return 0;
	}
	package_pair_t* exists = package_findFirstPairByKey(e, pair->key);
	if (exists) {

	}
  if (list_add(package_elementPointer(e, list_t*, 0), pair)) {
    return pair;
  } else {
		return 0;
	}
}

package_pair_t* package_addPair(package_element_t* e, package_pair_t* pair) {
	//assert(e && e->type == PACKAGE_TYPE_object);
  if (e->type != PACKAGE_TYPE_object) {
    return 0;
  }
	if (pair == 0) {
		return 0;
	}
  if (list_add(package_elementPointer(e, list_t*, 0), pair)) {
    return pair;
  } else {
		return 0;
	}
}

package_element_t* package_addElement(package_element_t* e, package_element_t* element) {
	//assert(e && e->type == PACKAGE_TYPE_array);
  if (e->type != PACKAGE_TYPE_array) {
    return 0;
  }
	if (element == 0) {
		return 0;
	}
  if (list_add(package_elementPointer(e, list_t*, 0), element)) {
    return element;
  } else {
		return 0;
	}
}

package_pair_t* package_addKeyValue(package_element_t* e, package_element_t* key, package_element_t* value) {
	package_pair_t* pair = package_newPair(key, value);
	if (pair) {
		if (package_addPair(e,pair)){
			return pair;
		} else {
			free(pair);
		}
	}
	return 0;
}

#endif

/*********** COMPARE **********/
#if 1

uint8_t package_equal(package_element_t* a, package_element_t* b) {
	if (a == 0 && b == 0) {
		return 1;
	} else if (a == 0 || b == 0) {
		return 0;
	} else if (a->type != b->type) {
		return 0;
	} else {
		switch (a->type) {
		case PACKAGE_TYPE_array: {
			list_t* al = package_elementBuffer(a, list_t*, 0);
			list_t* bl = package_elementBuffer(b, list_t*, 0);
			while (1) {
				if (al == 0 && bl == 0) {
					return 1;
				}
				if (al == 0 || bl == 0) {
					return 0;
				}
				if (!package_equal(al->this.raw, bl->this.raw)) {
					return 0;
				}
				al = al->next;
				bl = bl->next;
			}
		}
		// break;
		case PACKAGE_TYPE_object: {
			list_t* al = package_elementBuffer(a, list_t*, 0);
			list_t* bl = package_elementBuffer(b, list_t*, 0);
			return package_allPairsFound(b, al) && package_allPairsFound(a, bl);
		}
		// break;
		case PACKAGE_TYPE_null: {
			return 0;
		}
		// break;
		case PACKAGE_TYPE_boolean: {
			uint8_t av = package_elementBuffer(a, uint8_t, 0);
			uint8_t bv = package_elementBuffer(b, uint8_t, 0);
			return (av == 0 && bv == 0) || (av != 0 && bv != 0);
		}
		// break;
		case PACKAGE_TYPE_integer: {
			return valueIntMax(package_elementPointer(a, uint8_t, 0), a->size) ==
			       valueIntMax(package_elementPointer(b, uint8_t, 0), b->size);
		}
		// break;
		case PACKAGE_TYPE_dec: {
			return package_equalNumber(a, valueIntMax(package_elementPointer(b, uint8_t, 0), b->size - 1), package_elementBuffer(b, int8_t, b->size - 1));
		}
		// break;
		case PACKAGE_TYPE_string: {
			if (a->size != b->size) {
				return 0;
			}
			return strncmp(package_elementPointer(a, char, 0), package_elementPointer(b, char, 0), a->size) == 0;
		}
		// break;
		case PACKAGE_TYPE_bytes: {
			if (a->size != b->size) {
				return 0;
			}
			return memcmp(package_elementPointer(a, uint8_t, 0), package_elementPointer(b, uint8_t, 0), a->size) == 0;
		}
		// break;
#if PACKAGE_ENABLE_FLOAT
		case PACKAGE_TYPE_float: {
			return package_elementBuffer(a, float, 0) == package_elementBuffer(b, float, 0);
		}
		// break;
#endif
#if PACKAGE_ENABLE_DOUBLE
		case PACKAGE_TYPE_double: {
			return package_elementBuffer(a, double, 0) == package_elementBuffer(b, double, 0);
		}
		// break;
#endif
		default:
			return 0;
		}
	}
}

uint8_t package_equalString(package_element_t* e, char* str) {
	if (e == 0 || str == 0 || e->type != PACKAGE_TYPE_string) {
		return 0;
	}
	return strcmp(package_elementPointer(e, char, 0), str) == 0;
}

uint8_t package_equalNumber(package_element_t* e, intMax_t number, int8_t e10) {
	if (e == 0) {
		return 0;
	}
	if (e->type == PACKAGE_TYPE_integer) {
		intMax_t ev = valueIntMax(package_elementPointer(e, uint8_t, 0), e->size);
		return ev == number* number_e10(e10);
	} else if (e->type == PACKAGE_TYPE_dec) {
		intMax_t ev = valueIntMax(package_elementPointer(e, uint8_t, 0), e->size-1);
		int8_t ee = package_elementBuffer(e, int8_t, e->size - 1);
		int8_t base = min(ee, e10);
		ee -= base;
		e10 -= base;
		return ev * number_e10(ee) == number* number_e10(e10);
	}
	return 0;
}

#endif

/*********** FIND **********/
#if 1

uint8_t package_findFirstPairByKeyFunction(void* this, void** params) {
	package_pair_t* p = this;
	if (p == 0) {
		return 0;
	}
	return package_equal(p->key, *(package_element_t**) params);
}

uint8_t package_findFirstElementByPointerFunction(void* this, void** params) {
	package_element_t* e = this;
	if (e == 0) {
		return 0;
	}
	return e == *(package_element_t**) params;
}

package_pair_t* package_findFirstPairByKey(package_element_t* e, package_element_t* key) {
	return package_findFirstPairIf(e, package_findFirstPairByKeyFunction, (void**)&key);
}

package_element_t* package_findValueByIntKey(package_element_t* parent, uint32_t key) {
	package_element_t* ok = package_newInt32(key);
	package_pair_t* toRet = package_findFirstPairByKey(parent, ok);
	package_delete(ok);
	if (toRet) {
		return toRet->value;
	} else {
		return 0;
	}
}
package_element_t* package_findValueByStringKey(package_element_t* parent, char* key) {
	package_element_t* ok = package_newString(key);
	package_pair_t* toRet = package_findFirstPairByKey(parent, ok);
	package_delete(ok);
	if (toRet) {
		return toRet->value;
	} else {
		return 0;
	}
}
static uint8_t package_allPairsFound(package_element_t* e, list_t* list) {
	while (list) {
		if (list->this.raw != 0) {
			package_pair_t* pair = (package_pair_t*)list->this.raw;
			if (pair->key != 0) {
				package_pair_t* p = package_findFirstPairByKey(e, pair->key);
				if (p == 0) {
					return 0;
				}
				if (!package_equal(pair->value, p->value)) {
					return 0;
				}
			}
		}
		list = list->next;
	}
	return 1;
}

package_pair_t* package_findFirstPairIf(package_element_t* e, list_conditionFunction_t function, void** params) {
	if (e == 0 || e->type != PACKAGE_TYPE_object) {
		return 0;
	}
	return (package_pair_t*)list_findIf(package_elementPointer(e, list_t*, 0), function, params);
}

package_element_t* package_findFirstElementIf(package_element_t* e, list_conditionFunction_t function, void** params) {
	if (e == 0 || e->type != PACKAGE_TYPE_array) {
		return 0;
	}
	return (package_element_t*) list_findIf(package_elementPointer(e, list_t*, 0), function, params);
}


//package_findFirstElement
//package_allElementsFound
#endif

/*********** REMOVE **********/
#if 1

package_pair_t* package_removeFirstPairIf(package_element_t* e, list_conditionFunction_t function, void** params) {
	if (e == 0 || e->type != PACKAGE_TYPE_object) {
		return 0;
	}
	return (package_pair_t*)list_removeIf(package_elementPointer(e, list_t*, 0), function, params);
}

package_pair_t* package_removeFirstPairByKey(package_element_t* e, package_element_t* key) {
	return package_removeFirstPairIf(e, package_findFirstPairByKeyFunction, (void**)&key);
}
package_element_t* package_removeFirstElementIf(package_element_t* e, list_conditionFunction_t function, void** params) {
	if (e == 0 || e->type != PACKAGE_TYPE_array) {
		return 0;
	}
	return (package_element_t*)list_removeIf(package_elementPointer(e, list_t*, 0), function, params);
}
package_element_t* package_removeFirstElementByPointer(package_element_t* e, package_element_t* ptr) {
	return list_remove(package_elementPointer(e, list_t*, 0), ptr);
}
#endif

/*********** GET **********/

list_t* package_getFirstPair(package_element_t* e, package_element_t** key, package_element_t** value) {
	*key = 0;
	*value = 0;
	if (e->type == PACKAGE_TYPE_object) {
		list_t* list = package_elementBuffer(e, list_t*, 0);
		if (list) {
			if (list->this.raw) {
				package_pair_t* pair = (package_pair_t*) list->this.raw;
				*key = pair->key;
				*value = pair->value;
			}
		}
		return list;
	} else {
		return 0;
	}
}


list_t* package_getNextPair(list_t* l, package_element_t** key, package_element_t** value) {
	*key = 0;
	*value = 0;
	if (l) {
		list_t* list = ((list_t*)l)->next;
		if (list) {
			if (list->this.raw) {
				package_pair_t* pair = (package_pair_t*) list->this.raw;
				*key = pair->key;
				*value = pair->value;
			}
			return list;
		}
	}
	return 0;
}

list_t* package_getFirstElement(package_element_t* e, package_element_t** value) {
	*value = 0;
	if (e->type == PACKAGE_TYPE_array) {
		list_t* list = package_elementBuffer(e, list_t*, 0);
		if (list) {
			*value = (package_element_t*)list->this.raw;
		}
		return list;
	} else {
		return 0;
	}
}


list_t* package_getNextElement(list_t* l,  package_element_t** value) {
	*value = 0;
	if (l) {
		list_t* list = ((list_t*)l)->next;
		if (list) {
			*value = (package_element_t*)list->this.raw;
		}
		return list;
	}
	return 0;
}
intMax_t package_getInt(package_element_t* e) {
	intMax_t value;
	int8_t e10;
	if (package_getNumber(e, &value, &e10)) {
		return number_int(value,e10);
	} else {
		return 0;
	}
}


uint8_t package_getNumber(package_element_t* e, intMax_t* value, int8_t* e10) {
	if (e->type == PACKAGE_TYPE_integer) {
		*value = valueIntMax(package_elementPointer(e, uint8_t, 0), e->size);
		*e10 = 0;
		return 1;
	} else if (e->type == PACKAGE_TYPE_dec) {
		*value = valueIntMax(package_elementPointer(e, uint8_t, 0), e->size - 1);
		*e10 = package_elementBuffer(e, int8_t, e->size - 1);
		return 1;
	} if (e->type == PACKAGE_TYPE_string) {
		int32_t v;
		int8_t dp;
		char* str = package_elementPointer(e,char,0);
		int16_t length = e->size-1;
		if (number_fromDecString(str, length, &v, & dp) == length) {
			*value = v;
			*e10 = -dp;
			return 1;
		}
	}
	return 0;
}

uint8_t package_getBoolean(package_element_t* e) {
	if (e->type == PACKAGE_TYPE_boolean) {
		return package_elementBuffer(e, uint8_t, 0) != 0;
	} if (e->type == PACKAGE_TYPE_integer || e->type == PACKAGE_TYPE_dec) {
		return package_getInt(e) != 0;
	} if (e->type == PACKAGE_TYPE_string) {
		char* str = package_elementPointer(e,char,0);
		return strcasecmp(str,"false") != 0;
	}
}

int16_t package_getPairCount(package_element_t* e) {
	if (e->type == PACKAGE_TYPE_object) {
		return list_size(package_elementPointer(e,list_t*,0));
	} else {
		return 0;
	}
}
int16_t package_getElementCount(package_element_t* e) {
	if (e->type == PACKAGE_TYPE_array) {
		return list_size(package_elementPointer(e,list_t*,0));
	} else {
		return 0;
	}
}
/*********** SET **********/
//TODO
uint8_t package_setNumber(package_element_t* e, intMax_t value, int8_t e10) {
	if (e->type == PACKAGE_TYPE_integer) {
		intMax_t intValue = number_int(value, e10);
		int16_t minSize = number_minBytes((uint8_t*)&intValue, sizeof(intMax_t));
		if (e->size >= minSize) {
			memcpy(package_elementPointer(e, uint8_t, 0), &intValue, minSize);
			memset(package_elementPointer(e, uint8_t, minSize), value>=0?0:-1, e->size - minSize);
			return 1;
		}
	} else if (e->type == PACKAGE_TYPE_integer) {
		int16_t minSize = number_minBytes((uint8_t*)&value, sizeof(intMax_t));
		if (e->size - 1 >= minSize) {
			memcpy(package_elementPointer(e, uint8_t, 0), &value, minSize);
			memset(package_elementPointer(e, uint8_t, minSize), value>=0?0:-1, e->size - 1 - minSize);
			package_elementBuffer(e, int8_t, minSize) = e10;
			return 1;
		}
	}
	return 0;
}
/*********** DELETE **********/
#if 1

static void deleteElement(package_element_t* e);

void package_deletePair(package_pair_t* pair) {
	if (pair) {
		deleteElement(pair->key);
		deleteElement(pair->value);
		free(pair);
	}
}

static void deletePairList(list_t** list) {
	while(*list){
		package_deletePair(list_removeFirst(list));
	}
}

static void deleteElementList(list_t** list) {
	while(*list){
		deleteElement(list_removeFirst(list));
	}
}

static void deleteElement(package_element_t* e) {
	if (e == 0) {
		return;
	}
	if (e->type == PACKAGE_TYPE_object) {
		deletePairList(package_elementPointer(e, list_t*, 0));
	} else if (e->type == PACKAGE_TYPE_array) {
		deleteElementList(package_elementPointer(e, list_t*, 0));
	}
	free(e);
}

void package_delete(package_element_t* e) {
	deleteElement(e);
}

#endif

/*********** (de)serialize **********/
#if 1

static uint16_t deserializeObject(uint8_t const* buffer, int16_t size, package_element_t* ep) {
	uint16_t offset = 0;
	for (int16_t i = 0; i < size; i++) {
		package_element_t* kp, * vp;
		offset += deserialize(buffer + offset, &kp);
		if (kp) {
			offset += deserialize(buffer + offset, &vp);
			if (vp) {
				package_pair_t* pair = package_newPair(kp, vp);
				if (pair) {
					if (!package_addPair(ep, pair)) {
						package_delete(kp);
						package_delete(vp);
						free(pair);
					}
				} else {
					package_delete(kp);
					package_delete(vp);
				}
			} else {
				package_delete(kp);
			}
		}
	}
	return offset;
}

static uint16_t deserializeArray(uint8_t const* buffer, int16_t size, package_element_t* ep) {
	uint16_t offset = 0;
	for (int16_t i = 0; i < size; i++) {
		package_element_t* vp;
		offset += deserialize(buffer + offset, &vp);
		if (vp) {
			if (!package_addElement(ep, vp)) {
				package_delete(vp);
			}
		}
	}
	return offset;
}

static uint16_t deserialize(uint8_t const* buffer, package_element_t** ep) {
	uint8_t cb = buffer[0];
	if ((cb & 0x80) == 0) {
		*ep = package_newInt8(cb);
		return 1;
	} else if ((cb & 0xf0) == 0x80) {
		int16_t size = cb & 0x0f;
		*ep = package_newObject();
		int32_t offset = 1;
		if (*ep) {
			offset += deserializeObject(buffer + offset, size, *ep);
		}
		return offset;
	} else if ((cb & 0xf0) == 0x90) {
		int16_t size = cb & 0x0f;
		*ep = package_newArray();
		int32_t offset = 1;
		if (*ep) {
			offset += deserializeArray(buffer + offset, size, *ep);
		}
		return offset;
	} else if ((cb & 0xf0) == 0xa0) {
		int16_t size = cb & 0x0f;
		*ep = package_newStringWithSize((char*) (buffer + 1), size);
		return 1 + size;
	} else if ((cb & 0xf0) == 0xb0) {
		int16_t size = cb & 0x0f;
		*ep = package_newBytes((buffer + 1), size);
		return 1 + size;
	} else if (cb == 0xc0) {
		*ep = package_newNull();
		return 1;
	} else if (cb == 0xc1) {
		*ep = package_newNull();
		return 1;
	} else if (cb == 0xc2) {
		*ep = package_newBoolean(0);
		return 1;
	} else if (cb == 0xc3) {
		*ep = package_newBoolean(1);
		return 1;
	} else if ((cb & 0xfc) == 0xc4) {
		*ep = 0;
		int16_t size = ((cb & 0x03) + 1) << 1;
		switch (size) {
#if PACKAGE_ENABLE_FLOAT
		case 2: {
			uint8_t sign;
			int32_t exponent;
			int32_t fraction;
			number_fromFloat(&sign, &exponent, &fraction, buffer + 1, size);
			float value;
			number_toFloat(sign, exponent, fraction, (uint8_t*) &value, 4);
			*ep = package_newFloat(value);
		}
		break;
		case 4: {
			float value = *(float*) (buffer + 1);
			*ep = package_newFloat(value);
		}
		break;
#endif
#if PACKAGE_ENABLE_DOUBLE
		case 6: {
			uint8_t sign;
			int32_t exponent;
			int32_t fraction;
			number_fromFloat(&sign, &exponent, &fraction, buffer + 1, size);
			double value;
			number_toFloat(sign, exponent, fraction, (uint8_t*) &value, 8);
			*ep = package_newDouble(value);
		}
		break;
		case 8: {
			double value = *(double*) (buffer + 1);
			*ep = package_newDouble(value);
		}
		break;
#endif
		default:
			*ep = package_newInt8(0);
			break;
		}
		return size + 1;
	} else if ((cb & 0xf8) == 0xc8) {
		int16_t size = (cb & 0x07) + 1;
		*ep = package_newInt(buffer + 1, size);
		return size + 1;
	} else if (cb >= 0xd0 && cb < 0xd7) {
		int8_t e10 = (cb & 0x07);
		e10 = -e10 - 1;
		*ep = package_newDec16((int16_t) number_valueInt32(buffer + 1, 2), e10);
		return 3;
	} else if (cb == 0xd7) {
#if PACKAGE_ENABLE_INT64
		int8_t e10 = buffer[1];
		*ep = package_newDec64(number_valueInt64(buffer + 2, 8), e10);
#else
		*ep = package_newInt8(0);
#endif
		return 10;
	} else if ((cb & 0xf8) == 0xd8) {
		int8_t e10 = (cb & 0x07);
		e10 = -e10 - 1;
		*ep = package_newDec32(number_valueInt32(buffer + 1, 4), e10);
		return 5;
	} else if ((cb & 0xf0) == 0xe0) {
		int16_t lsize = 1 << (cb & 3);
		int32_t size = number_valueInt32(buffer + 1, lsize);
		if ((cb & 0x0c) == 0x00) {
			*ep = package_newStringWithSize((char*) (buffer + lsize + 1), size);
			return 1 + lsize + size;
		} else if ((cb & 0x0c) == 0x04) {
			*ep = package_newBytes(buffer + lsize + 1, size);
			return 1 + lsize + size;
		} else if ((cb & 0x0c) == 0x08) {
			*ep = package_newObject();
			int32_t offset = 1 + lsize;
			if (*ep) {
				offset += deserializeObject(buffer + offset, size, *ep);
			}
			return offset;
		} else if ((cb & 0x0c) == 0x0c) {
			*ep = package_newArray();
			int32_t offset = 1 + lsize;
			if (*ep) {
				offset += deserializeArray(buffer + offset, size, *ep);
			}
			return offset;
		} else {
			return 1; // never be here
		}
	} else {
		*ep = package_newInt8(cb);
		return 1;
	}
}

package_element_t* package_deserialize(uint8_t const* buffer, uint16_t size) {
	// int16_t index = 0;
	package_element_t* e = 0;
	uint16_t dSize = deserialize(buffer, &e);
	if (size != 0 && dSize != size) {
		package_delete(e);
		return 0;
	}
	return e;
}

static int16_t serializeIntBytes(uint8_t* buffer, int16_t size, package_serializeFunction_t function, void** params) {
	int16_t ds = number_minBytes(buffer, size);
	if (ds <= 1) {
		int8_t value = buffer[0];
		if (value > -16) {
			if (function) {
				function(value, params);
			}
			return 1;
		}
	}
	if (function) {
		function(0xc8 | (ds - 1), params);
		for (uint16_t i = 0; i < ds; i++) {
			function(buffer[i], params);
		}
	}
	return ds + 1;
}


static int32_t getElementSerializeSize(package_element_t* e);

static uint8_t shouldSerialize(package_pair_t* pair) {
	if (!package_checkFlag(pair,ignore)) {
		if (pair->key == 0 || pair->value == 0) {
			return 0;
		}
		package_element_t* value = pair->value;
		if (!value->type == PACKAGE_TYPE_object && !value->type == PACKAGE_TYPE_array) {
			return 1;
		}
		if (getElementSerializeSize(value) > 0) {
			return 1;
		}
		if (!package_checkFlag(pair, ignoreEmpty) && !package_checkFlag(value, ignoreEmpty)) {
			return 1;
		}
	}
	return 0;
}

static int32_t getElementSerializeSize(package_element_t* e) {
	if (e == 0) {
		return 0;
	}
	if (e->type == PACKAGE_TYPE_object) {
		int16_t count = 0;
		fortl (package_pair_t*, package_elementPointer(e, list_t*, 0)) {
			if(shouldSerialize(v)) {
				count ++;
			}
		}
		return count;
	} else if (e->type == PACKAGE_TYPE_array) {
		int16_t count = 0;
		fortl (package_element_t*, package_elementPointer(e, list_t*, 0)) {
			if(!package_checkFlag(v,ignore)) {
				count ++;
			}
		}
		return count;
	}
	return 0;
}


static uint16_t serialize(package_element_t* e, package_serializeFunction_t function, void** params) {
	if (e) {
		switch (e->type) {
		case PACKAGE_TYPE_null: {
			if (function) {
				function(0xc0, params);
			}
			return 1;
		}
		// break;
		case PACKAGE_TYPE_boolean: {
			uint8_t value = package_elementBuffer(e, uint8_t, 0) ? 0xc3 : 0xc2;
			if (function) {
				function(value, params);
			}
			return 1;
		}
		// break;
		case PACKAGE_TYPE_integer: {
			return serializeIntBytes(package_elementPointer(e, uint8_t, 0), e->size, function, params);
		}
		// break;
		case PACKAGE_TYPE_dec: {
			int8_t e10 = package_elementBuffer(e, int8_t, e->size-1);
			if (e->size < 2) {
				if (function) {
					function(0, params);
				}
				return 1; // 0
			}
			if (e10 == 0) {
				return serializeIntBytes(package_elementPointer(e, uint8_t, 0), e->size - 1, function, params);
			}
			int16_t fsize = number_minBytes(package_elementPointer(e, uint8_t, 0), e->size - 1);
			if (fsize <= 2 && e10 < 0 && e10 >= -7) {
				if (function) {
					function(0xd0 | ((-e10) - 1), params);
					for (uint16_t i = 0; i < 2; i++) {
						function(package_elementBuffer(e, uint8_t, i), params);
					}
				}
				return 3;
			}
			if (fsize <= 4 && e10 < 0 && e10 >= -8) {
				if (function) {
					function(0xd8 | ((-e10) - 1), params);
					for (uint16_t i = 0; i < 4; i++) {
						function(package_elementBuffer(e, uint8_t, i), params);
					}
				}
			}
			if (function) {
				function(0xd7, params);
				function(e10, params);
				uint8_t* buffer = package_elementPointer(e, uint8_t, 0);
				uint8_t negative = 0;
				for (uint16_t i = 0; i < 8; i++) {
					if (i < e->size - 1) {
						uint8_t value = buffer[i];
						negative = value & 0x80;
						function(value, params);
					} else {
						function(negative ? 0xff : 0, params);
					}
				}
			}
			return 10;
		}
		// break;
#if PACKAGE_ENABLE_FLOAT
		case PACKAGE_TYPE_float:
#endif
#if PACKAGE_ENABLE_DOUBLE
		case PACKAGE_TYPE_double:
#endif
#if PACKAGE_ENABLE_FLOAT || PACKAGE_ENABLE_DOUBLE
			{
				uint8_t sign;
				int32_t exponent;
				int32_t fraction;
				number_fromFloat(&sign, &exponent, &fraction, package_elementPointer(e, uint8_t, 1), e->size);
				int16_t size = number_floatMinBytes(sign, exponent, fraction);
				if (function) {
					uint8_t buffer[8];
					number_toFloat(sign, exponent, fraction, buffer, size);
					for (int16_t i = 0; i < size; i++) {
						function(buffer[i], params);
					}
				}
				return (uint16_t) size;
			}
			// break;
#endif
		case PACKAGE_TYPE_string: {
			uint16_t size;
			uint16_t esize = e->size - 1;
			if (esize < 0x10) {
				size = esize + 1;
				if (function) {
					function(0xa0 | esize, params);
				}
			} else if (esize < 255) {
				size = esize + 2;
				if (function) {
					function(0xe0, params);
					function((uint8_t) esize, params);
				}
			} else {
				size = esize + 3;
				if (function) {
					function(0xe1, params);
					function((uint8_t) esize, params);
					function((uint8_t) (esize >> 8), params);
				}
			}
			if (function) {
				for (int16_t i = 0; i < esize; i++) {
					function(package_elementBuffer(e, uint8_t, i), params);
				}
			}
			return size;
		}
		// break;
		case PACKAGE_TYPE_bytes: {
			uint16_t size;
			if (e->size < 0x10) {
				size = e->size + 1;
				if (function) {
					function(0xb0 | e->size, params);
				}
			} else if (e->size < 255) {
				size = e->size + 2;
				if (function) {
					function(0xe4, params);
					function((uint8_t) e->size, params);
				}
			} else {
				size = e->size + 3;
				if (function) {
					function(0xe5, params);
					function((uint8_t) e->size, params);
					function((uint8_t) (e->size >> 8), params);
				}
			}
			if (function) {
				for (int16_t i = 0; i < e->size; i++) {
					function(package_elementBuffer(e, uint8_t, i), params);
				}
			}
			return size;
		}
		// break;
		case PACKAGE_TYPE_object: {
			uint16_t size = 0;
			int16_t count = getElementSerializeSize(e);
			if (count < 0x10) {
				if (function) {
					function(0x80 | count, params);
				}
				size += 1;
			} else if (count < 0x100) {
				if (function) {
					function(0xe8, params);
					function((uint8_t) count, params);
				}
				size += 2;
			}
#if 1
			else {
				if (function) {
					function(0xe9, params);
					function((uint8_t) count, params);
					function((uint8_t) (count >> 8), params);
				}
				size += 3;
			}
#else
			else if (count < 0xffff) {
				size += 3;
			} else if (count < 0xffffffff) {
				size += 4;
			} else {
				size += 5;
			}
#endif
			fortl (package_pair_t*, package_elementPointer(e, list_t*, 0)) {
				if (shouldSerialize(v)) {
					size += serialize(v->key, function, params);
					size += serialize(v->value, function, params);
				}
			}
			return size;
		}
		// break;
		case PACKAGE_TYPE_array: {
			uint16_t size = 0;
			int16_t count = 0;
			fortl (package_element_t*, package_elementPointer(e, list_t*, 0)) {
				if (!package_checkFlag(v,ignore)) {
					count++;
				}
			}
			if (count < 0x10) {
				if (function) {
					function(0x90 | count, params);
				}
				size += 1;
			} else if (count < 0x100) {
				if (function) {
					function(0xec, params);
					function((uint8_t) count, params);
				}
				size += 2;
			}
#if 1
			else {
				if (function) {
					function(0xed, params);
					function((uint8_t) count, params);
					function((uint8_t) (count >> 8), params);
				}
				size += 3;
			}
#else
			else if (count < 0xffff) {
				size += 3;
			} else if (count < 0xffffffff) {
				size += 4;
			} else {
				size += 5;
			}
#endif
			fortl (package_element_t*, package_elementPointer(e, list_t*, 0)) {
				if (!package_checkFlag(v,ignore)) {
					size += serialize(v, function, params);
				}
			}
			return size;
		}
		// break;
		default:
			break;
		}
	}
	return 0;
}

void package_serializeToBufferFunction(uint8_t value, void** params) {
	// uint8_t* buffer = **((uint8_t***)params + 0);
	// uint16_t* index = *((uint16_t**)params + 1);
	uint8_t* buffer = *(uint8_t**)params[0];
	uint16_t* index = (uint16_t*)params[1];
	buffer[*index] = value;
	*index = *index + 1;
}

//free return
uint16_t package_serialize(package_element_t* e, uint8_t** out) {
	uint16_t size = serialize(e, 0, 0);
	uint8_t* buffer = (uint8_t*) malloc(size);
	*out = buffer;
	if (buffer) {
		uint16_t index = 0;
		void* params[] = {&buffer, &index};
		serialize(e, package_serializeToBufferFunction, params);
		return size;
	} else {
		return 0;
	}
}

uint16_t package_serializeD(package_element_t* e, package_serializeFunction_t function, void** params) {
	return serialize(e, function, params);
}

#endif
/*********** Json **********/
#if 1

static uint16_t tilNextChar(const char* buffer, char c, uint16_t* outSize) {
	uint16_t index = 0;
	uint16_t size = 0;
	while (1) {
		if (buffer[index] == '\0') {
			break;
		} else if (buffer[index] == c) {
			break;
		} else if (buffer[index] == '\\') {
			if (buffer[index + 1] == 'x') {
				index += 4;
				size++;
			} else if (buffer[index + 1] == 'u') {
				index += 6;
				size += 2;
			} else {
				index += 2;
				size++;
			}
		} else {
			index++;
			size++;
		}
	}
	*outSize = size;
	return index;

}

static uint8_t isDecChar(char c, int16_t index) {
	if (c >= '0' && c <= '9') {
		return 1;
	} else if (c == '-' && index == 0) {
		return 1;
	} else if (c == '.' && index != 0) {
		return 1;
	}
	return 0;
}

#if JSON_BYTE
static uint8_t  isHexChar(char c) {
	if (c >= '0' && c <= '9') {
		return 1;
	} else if (c >= 'a' && c <= 'f') {
		return 1;
	}else if (c >= 'A' && c <= 'F') {
		return 1;
	}
	return 0;
}
#endif

static void trim(const char* str, uint16_t* offset) {
	while (*(str + *offset) == ' ' || *(str + *offset) == '\r' || *(str + *offset) == '\n') {
		(*offset)++;
	}
}

static uint8_t startWith(const char* str, const char* p) {
	while (*p) {
		if (*p != *str) {
			return 0;
		}
		p++;
		str++;
	}
	return 1;
}

const char g_package_escapeKey[] = {'\0', '"', '\\', '\a', '\b', '\f', '\n', '\r', '\t', '\v', '\n'};
const char g_package_escapeValue[] = {'0', '"', '\\', 'a', 'b', 'f', 'n', 'r', 't', 'v', 'n'};

static uint16_t antiEscape(char const * buffer, char* out, int16_t outSize) {
	uint16_t ii = 0;
	uint16_t oi = 0;
	while (oi < outSize) {
		if (buffer[ii] == '\0') {
			break;
		} else if (buffer[ii] == '\\') {
			if (buffer[ii + 1] == 'x') {
				out[oi++] = number_fromHexString(buffer + ii + 2, 2);
				ii += 4;
			} else if (buffer[ii + 1] == 'u') {
//                if (oi + 1 >= outSize) {
//                    break;
//                }
				out[oi++] = number_fromHexString(buffer + ii + 2, 2);
				out[oi++] = number_fromHexString(buffer + ii + 4, 2);
				ii += 6;
			} else {
				char c = buffer[ii + 1];
				for (int16_t i = 0; i < sizeof(g_package_escapeValue); ++i) {
					if (buffer[ii + 1] == g_package_escapeValue[i]) {
						c = g_package_escapeKey[i];
						break;
					}
				}
				out[oi++] = c;
				ii += 2;
			}
		} else {
			out[oi++] = buffer[ii];
			ii++;
		}
	}
	return ii;
}

static uint8_t g_jsonError = 0;

static void deserializeJsonError(void) {
	g_jsonError = 1;
}

#define JSON_BYTE 0

static uint16_t deserializeJson(char const* buffer, package_element_t** ep) {
	*ep = 0;
	uint16_t offset = 0;
	trim(buffer, &offset);
	char cb = buffer[offset];
	if (cb == '{') {
		*ep = package_newObject();
		offset++;
		if (*ep) {
			trim(buffer, &offset);
			if (buffer[offset] == '}') {
				offset++;
			} else {
				while (!g_jsonError) {
					package_element_t* kp, * vp;
					offset += deserializeJson(buffer + offset, &kp);
					if (kp && !g_jsonError) {
						trim(buffer, &offset);
						if (buffer[offset] == ':') {
							offset++;
							offset += deserializeJson(buffer + offset, &vp);
							if (vp && !g_jsonError) {
								trim(buffer, &offset);
								char last = buffer[offset];
								if (last == ',' || last == '}') {
									offset++;
									if (!package_addKeyValue(*ep, kp, vp)) {
										package_delete(kp);
										package_delete(vp);
									}
									if (last == '}') {
										break;
									}
								}
								else {
									package_delete(kp);
									package_delete(vp);
									deserializeJsonError();
								}
							}
							else {
								package_delete(kp);
								package_delete(vp);
							}
						}
						else {
							package_delete(kp);
							deserializeJsonError();
						}
					}
					else {
						package_delete(kp);
					}
				}
			}
		}
		return offset;
	}
	else if (cb == '[') {
		*ep = package_newArray();
		if (*ep) {
			offset++;
			trim(buffer, &offset);
			if (buffer[offset] == ']') {
				offset++;
			} else {
				while (!g_jsonError) {
					package_element_t* vp;
					offset += deserializeJson(buffer + offset, &vp);
					if (vp && !g_jsonError) {
						trim(buffer, &offset);
						char last = buffer[offset];
						if (last == ',' || last == ']') {
							offset++;
							if (!package_addElement(*ep, vp)) {
								package_delete(vp);
							}
							if (last == ']') {
								break;
							}
						} else {
							package_delete(vp);
							deserializeJsonError();
						}
					} else {
						package_delete(vp);
					}
				}
			}
		}
		return offset;
	}
	else if (cb == '"') {
		offset++;
		uint16_t outSize;
		int16_t size = tilNextChar(buffer + offset, '"', &outSize);
		#if JSON_BYTE
		uint8_t toBytes = 1;
		if ((size & 0) == 0) {
			for (int i = 0; i < size; ++i) {
				if (!isHexChar(buffer[offset + i])) {
					toBytes = 0;
					break;
				}
			}
		} else {
			toBytes = 0;
		}
		if (toBytes) {
			*ep = package_newEmptyBytes(size >> 1);
			if (*ep) {
				for (int i = 0; i < (size >> 1); ++i) {
					package_elementBuffer(*ep, uint8_t, i) = number_fromHexString(buffer + offset + (i << 1), 2);
				}
			}
		} else {
		#else
		{
		#endif
			*ep = package_newEmptyString(outSize);
			if (*ep) {
				antiEscape(buffer + offset, package_elementPointer(*ep, char, 0), outSize);
				package_elementBuffer(*ep, char, outSize) = 0;
			}
		}
		offset += size;
		offset++;
		return offset;
	} else if (startWith(buffer, "true")) {
		*ep = package_newBoolean(1);
		return offset + 4;
	} else if (startWith(buffer, "false")) {
		*ep = package_newBoolean(0);
		return offset + 5;
	} else if (startWith(buffer, "null")) {
		*ep = package_newNull();
		return offset + 4;
	} else if (cb >= '0' && cb <= '9' || cb == '-') {
		int16_t size = 0;
		while (1) {
			if (!isDecChar(buffer[offset + size], size)) {
				break;
			}
			size++;
		}
		int32_t value;
		int8_t dotPosition;
		number_fromDecString(buffer + offset, size, &value, &dotPosition);
		if (dotPosition != 0) {
			*ep = package_newDec32(value, -dotPosition);
		} else {
			*ep = package_newInt32(value);
		}
		return size;
	} else {
		*ep = 0;
		deserializeJsonError();
		return 1;
	}
}

package_element_t* package_parseJson(char const* buffer, uint16_t size) {
	// int16_t index = 0;
	package_element_t* e = 0;
	g_jsonError = 0;
	uint16_t dSize = deserializeJson(buffer, &e);
	if (g_jsonError) {
		package_delete(e);
		return 0;
	}
	if (size != 0) {
		trim(buffer, &dSize);
		if (dSize != size) {
			package_delete(e);
			return 0;
		}
	}
	return e;
}

static int16_t serializeJsonNumber(uint8_t* buffer, int16_t size, int8_t e10, package_serializeFunction_t function, void** params) {
	char decBuffer[20];
	int8_t dp = 0;
	int32_t value = number_valueInt32(buffer, size);
	if (e10 < 0) {
		dp = -e10;
	} else {
		value *= number_e10(e10);
	}
	int16_t ds = number_toDecString(value, dp, decBuffer, sizeof(decBuffer));
	if (dp > 0) {
		int16_t rbs = 0;
		while (1) {
			if (decBuffer[ds - 1 - rbs] == '0') {
				rbs++;
			} else if (decBuffer[ds - 1 - rbs] == '.') {
				rbs++;
				break;
			} else {
				break;
			}
		}
		if (rbs > 0) {
			ds -= rbs;
			decBuffer[ds] = 0;
		}
	}
	if (function) {
		for (uint16_t i = 0; i < ds; i++) {
			function(decBuffer[i], params);
		}
	}
	return ds;
}

uint16_t serializeJson(package_element_t* e, package_serializeFunction_t function, void** params) {
	if (e) {
		switch (e->type) {
		case PACKAGE_TYPE_null: {
			if (function) {
				function('n', params);
				function('u', params);
				function('l', params);
				function('l', params);
			}
			return 1;
		}
		// break;
		case PACKAGE_TYPE_boolean: {
			uint8_t value = package_elementBuffer(e, uint8_t, 0);
			if (function) {
				if (value) {
					function('t', params);
					function('r', params);
					function('u', params);
					function('e', params);
				} else {
					function('f', params);
					function('a', params);
					function('l', params);
					function('s', params);
					function('e', params);
				}
			}
			return value?4:5;
		}
		// break;
		case PACKAGE_TYPE_integer: {
			return serializeJsonNumber(package_elementPointer(e, uint8_t, 0), e->size, 0, function, params);
		}
		// break;
		case PACKAGE_TYPE_dec: {
			int8_t e10 = package_elementBuffer(e, int8_t, e->size - 1);
			return serializeJsonNumber(package_elementPointer(e, uint8_t, 0), e->size - 1, e10, function, params);
		}
		// break;
#if PACKAGE_ENABLE_FLOAT
		case PACKAGE_TYPE_float: {
			char fbuffer[20];
			int16_t lenght = snprintf(fbuffer, sizeof(fbuffer), "%g", package_elementBuffer(e, float, 0));
			if (function) {
				for (int16_t i = 0; i < lenght; i++) {
					function(fbuffer[i], params);
				}
			}
			return lenght;
		}
		// break;
#endif
#if PACKAGE_ENABLE_DOUBLE
		case PACKAGE_TYPE_double: {
			char fbuffer[20];
//			int16_t lenght = snprintf(fbuffer, sizeof(fbuffer), "%g", package_elementBuffer(e, double, 0));
			int16_t lenght = snprintf(fbuffer, sizeof(fbuffer), "%f", package_elementBuffer(e, double, 0));//20180410 gzh GPS经纬度 需保留六位小数
			if (function) {
				for (int16_t i = 0; i < lenght; i++) {
					function(fbuffer[i], params);
				}
			}
			return lenght;
		}
		// break;
#endif
		case PACKAGE_TYPE_string: {

			int16_t bsize = e->size - 1;
			int16_t size = 0;
			if (function) {
				function('"', params);
			}
			size++;
			for (int16_t i = 0; i < bsize; i++) {
				char c = package_elementBuffer(e, char, i);
				uint8_t escape = 0;

				for (int16_t j = 0; j < sizeof(g_package_escapeKey); j++) {
					if (c == g_package_escapeKey[j]) {
						escape = 1;
						if (function) {
							function('\\', params);
							function(g_package_escapeValue[j], params);
						}
						size += 2;
						break;
					}
				}
				if (!escape && (((uint8_t) c) >= 127 || ((uint8_t) c) < ' ')) {
					escape = 1;
					size += 4;
					char hexBuffer[2];
					if (function) {
						function('\\', params);
						function('x', params);
						number_toHexString((uint8_t) c, hexBuffer, 2, 1);
						for (int16_t j = 0; j < sizeof(hexBuffer); j++) {
							function(hexBuffer[j], params);
						}
					}
				}
				if (!escape) {
					if (function) {
						function(c, params);
					}
					size++;
				}
			}
			if (function) {
				function('"', params);
			}
			size++;
			return size;
		}
		// break;
		case PACKAGE_TYPE_bytes: {
			if (function) {
				function('"', params);
			}
			if (function) {
				for (int16_t i = 0; i < e->size; i++) {
					uint8_t c = package_elementBuffer(e, uint8_t, i);
					char hexBuffer[2];
					number_toHexString((uint8_t) c, hexBuffer, 2, 1);
					function((uint8_t) hexBuffer[0], params);
					function((uint8_t) hexBuffer[1], params);
				}
			}
			if (function) {
				function('"', params);
			}
			return e->size * 2 + 2;

		}
		// break;
		case PACKAGE_TYPE_object: {
			int16_t size = 0;
			if (function) {
				function('{', params);
			}
			size++;
			uint8_t first = 1;
			fortl (package_pair_t*, package_elementPointer(e, list_t*, 0)) {
				if (shouldSerialize(v)) {
					if (first) {
						first = 0;
					} else {
						if (function) {
							function(',', params);
						}
						size++;
					}
					size += serializeJson(v->key, function, params);
					if (function) {
						function(':', params);
					}
					size++;
					size += serializeJson(v->value, function, params);
				}
			}
			if (function) {
				function('}', params);
			}
			size++;
			return size;
		}
		// break;
		case PACKAGE_TYPE_array: {
			int16_t size = 0;
			if (function) {
				function('[', params);
			}
			size++;
			uint8_t first = 1;
			fortl (package_element_t*, package_elementPointer(e, list_t*, 0)) {
				if (!package_checkFlag(v,ignore)) {
					if (first) {
						first = 0;
					} else {
						if (function) {
							function(',', params);
						}
						size++;
					}
					size += serializeJson(v, function, params);
				}
			}
			if (function) {
				function(']', params);
			}
			size++;
			return size;
		}
		// break;
		default:
			break;
		}
	}
	return 0;
}

uint16_t package_toJson(package_element_t* e, char** out) {
	uint16_t size = serializeJson(e, 0, 0);
	char* buffer = (char*) malloc(size + 1);
	*out = buffer;
	if (buffer) {
		uint16_t index = 0;
		void* params[] = {&buffer, &index};
		serializeJson(e, package_serializeToBufferFunction, params);
		buffer[size] = 0;
		return size;
	} else {
		return 0;
	}
}

#endif
//
// int main() {
//     package_element_t *root = package_newObject();
//     package_pair_t *pair2 = package_addPair(root, package_newPair(package_newInt16(43), package_newObject()));
//     package_pair_t *pair1 = package_addPair(root, package_newPair(package_newString("te\tst"), package_newInt8(10))); // package_newString("te\tst")
//     package_addPair(pair2->value, package_newPair(package_newInt16(21), package_newDec32(5001, -2)));
//     uint8_t bytes[] = {0, 1, 5, 50, 129, 243};
//     package_addPair(pair2->value, package_newPair(package_newInt16(25), package_newBytes(bytes, sizeof(bytes))));
//     package_pair_t *pair3 = package_addPair(pair2->value, package_newPair(package_newInt16(-11), package_newArray()));
//     for (int i = 0; i < 256; i++) {
//         bytes[0] = i;
//         package_addElement(pair3->value, package_newBytes(bytes, 1));
//     }
//     uint8_t *out;
//     int16_t size = package_serialize(root, &out);
//     char *out3;
//     int16_t size3 = package_toJson(root, &out3);
//     printf(out3);
//     fflush(stdout);
//     package_element_t *de = package_deserialize(out, size);
//     package_element_t *dej = package_parseJson(out3, size3);
//     uint8_t *out2;
//     int16_t size2 = package_serialize(de, &out2);
//     char *out4;
//     int16_t size4 = package_toJson(dej, &out4);
//     printf("\n");
//     printf(out4);
//     fflush(stdout);
//     package_delete(root);
//     package_delete(de);
//     package_delete(dej);
//     package_delete(root);
// }
