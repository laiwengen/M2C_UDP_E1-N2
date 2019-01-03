#ifndef NRF24_H__
#define NRF24_H__

#include "frameworks/list.h"

typedef struct nrf24_t nrf24_t;
typedef struct nrf24_sequence_t nrf24_sequence_t;
typedef uint8_t (*nrf24_dataFunction_t)(nrf24_t* nrf24, nrf24_sequence_t* sequence, uint8_t* bytes, int8_t size, void** params);
typedef void (*nrf24_sequenceFunction_t)(nrf24_t* nrf24, nrf24_sequence_t* sequence);

typedef enum {
  NRF24_SEQUENCE_TYPE_transmit,
  NRF24_SEQUENCE_TYPE_receive,
  NRF24_SEQUENCE_TYPE_group,
  NRF24_SEQUENCE_TYPE_wait,
  NRF24_SEQUENCE_TYPE_SIZE,
} nrf24_sequenceType_t;

typedef enum {
  NRF24_IDLE_TYPE_receive,
  NRF24_IDLE_TYPE_standby,
  NRF24_IDLE_TYPE_poweroff,
  NRF24_IDLE_TYPE_SIZE,
} nrf24_idleType_t;

typedef enum {
  NRF24_FLAG_interrupted,
  NRF24_FLAG_SIZE,
} nrf24_flag_t;

typedef enum {
  NRF24_STATUS_txFull,
  NRF24_STATUS_pipeNumber0,
  NRF24_STATUS_pipeNumber1,
  NRF24_STATUS_pipeNumber2,
  NRF24_STATUS_retansmits,
  NRF24_STATUS_dataSent,
  NRF24_STATUS_dataReady,
  NRF24_STATUS_SIZE,
} nrf24_status_t;

typedef struct nrf24_transmit_t {
  uint8_t address[5];
  uint32_t timeout;
  uint8_t* bytes;
  int8_t size;
  uint8_t noack;
  uint8_t ackTimeout; //250us - 4000us
  uint8_t retransmitTimes; // 0 - 15
} nrf24_transmit_t;

typedef struct nrf24_receive_t {
  uint32_t timeout;
  uint8_t pipe;
  nrf24_dataFunction_t onReceived;
  void** params;
} nrf24_receive_t;

typedef struct nrf24_group_t {
  int32_t stage;
  list_t* subsequences;
  nrf24_sequenceFunction_t stageChangedFunction;
} nrf24_group_t;

typedef struct nrf24_wait_t {
  uint32_t timeout;
} nrf24_wait_t;

struct nrf24_sequence_t {
  nrf24_sequenceType_t type;
  int8_t doTimes;
  int8_t totalTimes;
  nrf24_sequenceFunction_t beforeDoFunction;
  nrf24_sequenceFunction_t onTimeoutFunction;
  union {
    nrf24_transmit_t transmit;
    nrf24_receive_t receive;
    nrf24_group_t group;
    nrf24_wait_t wait;
  } data;
};

typedef struct nrf24_idle_t {
  nrf24_idleType_t type;
  union {
    nrf24_receive_t receive;
  } data;
} nrf24_idle_t;

struct nrf24_t {
  struct {
    uint8_t config;
    uint8_t retransmission;  // 7:4  retransmit delay, 250us - 4000us. 3:0 retransmit times, 0 - 15.
    uint8_t rfCHannel;  // 2.4GHz + n MHz

    uint8_t rxAddress0[5];
    uint8_t rxAddress1[5];
    uint8_t rxAddress2_6[4];
    uint8_t txAddress[5];
  } config;
  nrf24_idle_t idle;
  uint8_t status; // nrf24_status_t
  list_t* sequences;
};

// uint8_t nrf24_transmit(uint8_t* bytes, int16_t size);
uint8_t nrf24_addSequence(nrf24_t* nrf24, nrf24_sequence_t* sequence);
void nrf24_init(nrf24_t* nrf24);
void nrf24_enable(nrf24_t* nrf24, uint8_t enable);
void nrf24_deleteSequence(nrf24_sequence_t* sequence);
void nrf24_setAddress1(nrf24_t* nrf24, uint8_t* address);

void nrf24_setTimeout(nrf24_t* nrf24, uint32_t us);
void nrf24_interrputed(nrf24_t* nrf24, uint8_t flag);


uint8_t nrf24_hw_write(nrf24_t* nrf24, uint8_t command, uint8_t const* data, int8_t size);
uint8_t nrf24_hw_read(nrf24_t* nrf24, uint8_t command, uint8_t* data, int8_t size);


void nrf24_hw_setTimer(nrf24_t* nrf24, uint32_t us, uint8_t timeout);
void nrf24_hw_cancelTimer(nrf24_t* nrf24);
void nrf24_hw_ce(nrf24_t* nrf24, uint8_t value);

void nrf24_hw_init(nrf24_t* nrf24);


#endif
