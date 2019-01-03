#ifndef PACKAGE_H__
#define PACKAGE_H__
#include <stdint.h>
#include "frameworks/list.h"

#define PACKAGE_ENABLE_INT64 0
#define PACKAGE_ENABLE_FLOAT 0
#define PACKAGE_ENABLE_DOUBLE 0

#if PACKAGE_ENABLE_INT64
typedef int64_t intMax_t;
#else
typedef int32_t intMax_t;
#endif
enum package_flag_t {
  PACKAGE_FLAG_ignore,
  PACKAGE_FLAG_ignoreEmpty,
  PACKAGE_FLAG_SIZE,
};
typedef struct {
  uint8_t type;
  uint8_t flag;
  uint16_t size;
} package_element_common_t;

typedef struct package_element_t{
  uint8_t type;
  uint8_t flag;
  uint16_t size;
  union {
    list_t* list;
    int32_t v32;
    int16_t v16;
    int8_t v8;
    // char str[16];
    uint8_t bytes[16];
  } value;
} package_element_t;

typedef struct package_pair_t{
    package_element_t *key;
    package_element_t *value;
    uint8_t flag;
} package_pair_t;

typedef void(*package_serializeFunction_t)(uint8_t, void**);

enum package_type_t {
  PACKAGE_TYPE_object,
  PACKAGE_TYPE_array,
  PACKAGE_TYPE_string,
  PACKAGE_TYPE_bytes,
  PACKAGE_TYPE_integer,
  PACKAGE_TYPE_dec,
  PACKAGE_TYPE_null,
  PACKAGE_TYPE_boolean,
#if PACKAGE_ENABLE_FLOAT
  PACKAGE_TYPE_float,
#endif
#if PACKAGE_ENABLE_DOUBLE
  PACKAGE_TYPE_double,
#endif
};

/*********** UTILS **********/
#if 1
#define package_elementBuffer(e, type, i) (*(type*)(((uint8_t*)(e)) + sizeof(package_element_common_t) + (i)))
#define package_elementPointer(e, type, i) ((type*)(((uint8_t*)(e)) + sizeof(package_element_common_t) + (i)))
#define package_setFlag(e, f) ((e)->flag |= (1) << (PACKAGE_FLAG_##f))
#define package_clearFlag(e, f) ((e)->flag &= ~(1 << (PACKAGE_FLAG_##f)))
#define package_checkFlag(e, f) (((e)->flag & (1 << (PACKAGE_FLAG_##f))) ? 1:0)
#endif

/*********** NEW **********/
#if 1

package_element_t *package_newElement(uint8_t type, uint16_t size);

package_element_t *package_newNull(void);

package_element_t *package_newBoolean(uint8_t value);

package_element_t *package_newInt(const uint8_t *buffer, uint16_t size);

package_element_t *package_newInt8(int8_t value);

package_element_t *package_newInt16(int16_t value);

package_element_t *package_newInt32(int32_t value);

package_element_t *package_newInt64(int64_t value);

package_element_t *package_newDec8(int8_t value, int8_t e10);

package_element_t *package_newDec16(int16_t value, int8_t e10);

package_element_t *package_newDec32(int32_t value, int8_t e10);

package_element_t *package_newDec64(int64_t value, int8_t e10);

package_element_t *package_newFloat(float value);

package_element_t *package_newDouble(double value);

package_element_t *package_newString(char const* str);

package_element_t *package_newEmptyString(int16_t size);

package_element_t *package_newStringWithSize(char const* str, int16_t size);

package_element_t *package_newBytes(uint8_t const* bytes, int16_t size);

package_element_t *package_newEmptyBytes(int16_t size);

package_element_t *package_newObject(void);

package_element_t *package_newArray(void);

package_pair_t *package_newPair(package_element_t *key, package_element_t *value);
#endif

/*********** ADD **********/
#if 1

package_pair_t *package_addPair(package_element_t *e, package_pair_t *pair);
package_pair_t* package_addKeyValue(package_element_t* e, package_element_t* key, package_element_t* value);
package_element_t *package_addElement(package_element_t *e, package_element_t *element);

#endif

/*********** COMPARE **********/
#if 1

uint8_t package_equal(package_element_t *a, package_element_t *b);
uint8_t package_equalString(package_element_t* e, char* str);
uint8_t package_equalNumber(package_element_t* e, intMax_t number, int8_t e10);

#endif

/*********** FIND **********/
#if 1

package_pair_t *package_findFirstPairIf(package_element_t *e, list_conditionFunction_t function, void** params);
package_pair_t *package_findFirstPairByKey(package_element_t *e, package_element_t *key);
package_element_t *package_findValueByIntKey(package_element_t *parent, uint32_t key);
package_element_t *package_findValueByStringKey(package_element_t *parent, char* key);
package_element_t *package_findFirstElementIf(package_element_t *e, list_conditionFunction_t function, void** params);

#endif

/*********** REMOVE **********/
#if 1

package_pair_t *package_removeFirstPairIf(package_element_t *e, list_conditionFunction_t function, void** params);
package_pair_t *package_removeFirstPairByKey(package_element_t *e, package_element_t *key);

package_element_t *package_removeFirstElementByPointer(package_element_t *e, package_element_t *ptr);
package_element_t *package_removeFirstElementIf(package_element_t *e, list_conditionFunction_t function, void** params);

#endif

/*********** GET **********/

list_t* package_getFirstPair(package_element_t *e, package_element_t** key, package_element_t** value);
list_t* package_getNextPair(list_t *l, package_element_t** key, package_element_t** value);

list_t* package_getFirstElement(package_element_t *e, package_element_t** value);
list_t* package_getNextElement(list_t *l,  package_element_t** value);


intMax_t package_getInt(package_element_t* e);
uint8_t package_getNumber(package_element_t* e, intMax_t* value, int8_t* e10);

uint8_t package_getBoolean(package_element_t* e);

int16_t package_getPairCount(package_element_t* e);
int16_t package_getElementCount(package_element_t* e);

//TODO
/*********** SET **********/
uint8_t package_setNumber(package_element_t* e, intMax_t value, int8_t e10);

/*********** DELETE **********/
#if 1

void package_deletePair(package_pair_t* pair);
void package_delete(package_element_t *e);

#endif

/*********** (de)serialize **********/
#if 1
package_element_t *package_deserialize(uint8_t const* buffer, uint16_t size);
//free return
uint16_t package_serialize(package_element_t *e, uint8_t **out);
uint16_t package_serializeD(package_element_t *e, package_serializeFunction_t function, void** params);

#endif
/*********** Json **********/
#if 1
package_element_t *package_parseJson(char const* buffer, uint16_t size);
uint16_t package_toJson(package_element_t *e, char **out);
#endif

#endif
