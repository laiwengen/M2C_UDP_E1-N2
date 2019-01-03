#ifndef PLC_H__
#define PLC_H__


typedef struct plc_t plc_t;

typedef int32_t(*plc_getFuction_t)(plc_t*);
typedef int32_t(*plc_setFuction_t)(plc_t*, int32_t);
typedef int32_t(*plc_processFuction_t)(plc_t*, int32_t);

struct plc_t {
  int32_t target;
  int32_t set;
  plc_getFuction_t resaultFunction;
  plc_setFuction_t setFunction;
  plc_processFuction_t processFunction;
  plc_getFuction_t readyFunction;
};


void plc_add(plc_t* plc);
void plc_enable(uint8_t enable);

#endif
