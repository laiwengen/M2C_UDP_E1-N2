#include "frameworks/nrf24.h"
#include "stdlib.h"
#include "string.h"

#define istype(s, t) (s->type == NRF24_SEQUENCE_TYPE_##t)

static inline uint8_t cnc(uint8_t* current, uint8_t value) {
  if (*current == value) {
    return 0;
  } else {
    *current = value;
    return 1;
  }
}

static inline uint8_t scnc(uint8_t* current, uint8_t sets, uint8_t clears) {
  return cnc(current, *current|sets&(~clears));
}

#if 1 // hw stuff
  static void writeRegisterMultiBytes(nrf24_t* nrf24, uint8_t address, uint8_t* bytes, int8_t size) {
    nrf24_hw_write(nrf24, 0x20|address, bytes, size);
  }
  static void writeRegister(nrf24_t* nrf24, uint8_t address, uint8_t value) {
    writeRegisterMultiBytes(nrf24, address, &value, 1);
  }

  static int8_t readRxPayload(nrf24_t* nrf24, uint8_t* bytes) {
    nrf24_hw_read(nrf24, 0x60, bytes, 1);
    int8_t size = bytes[0];
    nrf24_hw_read(nrf24, 0x61, bytes, size);
    return size;
  }

  static void writeTxPayload(nrf24_t* nrf24, uint8_t* bytes, int8_t size, uint8_t noack) {
    nrf24_hw_write(nrf24, noack?0xb0:0xa0, bytes, size);
  }
  static uint8_t readStatus(nrf24_t* nrf24) {
    return nrf24_hw_write(nrf24, 0xff, 0, 0);
  }

  static void setTimeout(nrf24_t* nrf24, uint32_t us) {
    nrf24_hw_setTimer(nrf24, us, 1);
  }

  static void setWakeup(nrf24_t* nrf24, uint32_t us) {
    nrf24_hw_setTimer(nrf24, us, 0);
  }

  static void cancelTimeout(nrf24_t* nrf24) {
    nrf24_hw_cancelTimer(nrf24);
  }

#endif

uint8_t nrf24_addSequence(nrf24_t* nrf24, nrf24_sequence_t* sequence) {
  return list_add(&nrf24->sequences, sequence) != 0;
}

void nrf24_deleteSequence(nrf24_sequence_t* sequence) {
  if (istype(sequence,transmit)) {
    free(sequence->data.transmit.bytes);
  } else if (istype(sequence,group)) {
    while(list_size(&sequence->data.group.subsequences)) {
      nrf24_deleteSequence(list_removeFirst(&sequence->data.group.subsequences));
    }
  }
  free(sequence);
}

#if 1 // sequence done?
  static uint8_t isSequenceDone(nrf24_t* nrf24, uint8_t flag, nrf24_sequence_t* sequence) {
    uint8_t toReturn = 0;
    uint8_t timeout = (flag & 0x01) ? 1 : 0;
    uint8_t wake = (flag & 0x02) ? 1 : 0;
    uint8_t callTimeoutFunction = 0;
    if (sequence) {
      if (istype(sequence, group)) {
        nrf24_sequence_t* sub = list_peekIndex(&sequence->data.group.subsequences, sequence->data.group.stage);
        uint8_t subDone;
        if (sub) {
          subDone = isSequenceDone(nrf24, flag, sub);
        } else {
          subDone = 1;
        }
        if (subDone) {
          uint8_t resetGroup = 0;
          if (timeout) {
            resetGroup = 1;
            callTimeoutFunction = 1;
          } else {
            sequence->data.group.stage ++;
            int32_t stages = list_size(&sequence->data.group.subsequences);
            if (sequence->data.group.stage >= stages) {
              resetGroup = 1;
            }
          }
          if (resetGroup) {
            sequence->data.group.stage = 0;
            fortl(nrf24_sequence_t*, &sequence->data.group.subsequences) {
              v->doTimes = 0;
            }
            sequence->doTimes ++;
          }
          if (sequence->data.group.stageChangedFunction) {
            sequence->data.group.stageChangedFunction(nrf24, sequence);
          }
        }
      }
      else {
        if (timeout) {
          sequence->doTimes ++;
          callTimeoutFunction = 1;
        } else if (wake) {
        } else {
          if (istype(sequence, receive)) {
            uint8_t buffer[32];
            toReturn = sequence->data.receive.onReceived(nrf24, sequence, buffer, readRxPayload(nrf24, buffer), sequence->data.receive.params);
            // TODO back to idle once received?
          } else {
            toReturn = 1;
          }
        }
      }
    } else {
      toReturn = 1; //null pointer
    }
    if (callTimeoutFunction && sequence->onTimeoutFunction) {
      sequence->onTimeoutFunction(nrf24, sequence);
    }
    if (sequence->doTimes >= sequence->totalTimes) {
      toReturn = 1;
    }
    return toReturn;
  }

  static void freeSequence(nrf24_sequence_t* sequence) {
    if (sequence) {
      if (istype(sequence, group)) {
        while (list_size(&sequence->data.group.subsequences)) {
          freeSequence(list_removeFirst(&sequence->data.group.subsequences));
        }
      } else if (istype(sequence, transmit)) {
        free(sequence->data.transmit.bytes);
      } else if (istype(sequence, receive)) {
        free(sequence->data.receive.params);
      }
      free(sequence);
    }
  }

  static void removeTop(nrf24_t* nrf24) {
    freeSequence((nrf24_sequence_t*)list_removeFirst(&nrf24->sequences));
  }
#endif

#if 1 // do sequence
  static void doPower(nrf24_t* nrf24, uint8_t on) {
    if (on) {
      if (scnc(&nrf24->config.config,1<<1,0)) {
        writeRegister(nrf24, 0x00, nrf24->config.config);
        setWakeup(nrf24, 5000);
      }
    } else {
      if (scnc(&nrf24->config.config,0,1<<1)) {
        writeRegister(nrf24, 0x00, nrf24->config.config);
        setWakeup(nrf24, 1000);
      }
    }
  }

  static void doTransmit(nrf24_t* nrf24, nrf24_transmit_t* transmit) {
    // power check
    doPower(nrf24, 1);
    // step 1: set address
    if (memcmp(nrf24->config.txAddress, transmit->address, 5)){
      memcpy(nrf24->config.txAddress, transmit->address, 5);
      writeRegisterMultiBytes(nrf24, 0x10, nrf24->config.txAddress, 5);
    }
    // step 2: ack and retransmit
    if (transmit->noack) {
    } else {
      uint8_t retransmission = (transmit->ackTimeout << 3) | (transmit->retransmitTimes);
      if (cnc(&nrf24->config.retransmission, retransmission)) {
        writeRegister(nrf24, 0x04, nrf24->config.retransmission);
      }
    }
    // step 3: set to tx mode
    if (scnc(&nrf24->config.config,0,1<<0)) {
      writeRegister(nrf24, 0x00, nrf24->config.config);
    }
    // step 4: push data to fifo
    writeTxPayload(nrf24, transmit->bytes, transmit->size, transmit->noack);

    // step 5: interrupt status
    writeRegister(nrf24, 0x07, 0x70);

    // step 6: clear and set timeout
    setTimeout(nrf24, transmit->timeout);

    // step 7: start transmit by set ce pin
    nrf24_hw_ce(nrf24, 1);
  }

  static void doReceive(nrf24_t* nrf24, nrf24_receive_t* receive) {
    // power check
    doPower(nrf24, 1);
    // step: set to rx mode
    if (scnc(&nrf24->config.config,1<<0,0)) {
      writeRegister(nrf24, 0x00, nrf24->config.config);
    }

    // step: interrupt status
    writeRegister(nrf24, 0x07, 0x70);

    // step: clear and set timeout
    setTimeout(nrf24, receive->timeout);

    // step: start receive by set ce pin
    nrf24_hw_ce(nrf24, 1);
  }

  static void doSequence(nrf24_t* nrf24, nrf24_sequence_t* sequence) {
    if (sequence) {
      if (sequence->beforeDoFunction) {
        sequence->beforeDoFunction(nrf24, sequence);
      }
      if (istype(sequence, group)) {
        nrf24_sequence_t* sub = list_peekIndex(&sequence->data.group.subsequences, sequence->data.group.stage);
        doSequence(nrf24, sub);
      } else if (istype(sequence, transmit)) {
        doTransmit(nrf24, &sequence->data.transmit);
      } else if (istype(sequence, receive)) {
        doReceive(nrf24, &sequence->data.receive);
      }
    }
  }

  static void doTop(nrf24_t* nrf24) {
    doSequence(nrf24, (nrf24_sequence_t*)list_peekFirst(&nrf24->sequences));
  }
#endif

#if 1 // do idle
  static void doIdle(nrf24_t* nrf24) {
    nrf24_hw_ce(nrf24, 0);
  }
#endif

void nrf24_setTimeout(nrf24_t* nrf24, uint32_t us) {
  setTimeout(nrf24, us);
}

void nrf24_interrputed(nrf24_t* nrf24, uint8_t flag) {
  nrf24_sequence_t* top = (nrf24_sequence_t*)list_peekFirst(&nrf24->sequences);
  nrf24->status = readStatus(nrf24);
  if (isSequenceDone(nrf24, flag, top)) {
    removeTop(nrf24);
    cancelTimeout(nrf24);
  }
  if (list_size(&nrf24->sequences)) {
    doTop(nrf24);
  } else {
    doIdle(nrf24);
  }
}

static void commandInit(nrf24_t* nrf24) {
  nrf24->config.config |= 0x0c;
  writeRegister(nrf24,0, nrf24->config.config);
  writeRegister(nrf24,1, 0x3f);
  writeRegister(nrf24,2, 0x3f);
  writeRegister(nrf24,3, 0x03);

  writeRegister(nrf24,4, nrf24->config.retransmission);
  writeRegister(nrf24,5, nrf24->config.rfCHannel);
  writeRegister(nrf24,6, 0x0e);
  writeRegister(nrf24,7, 0xff);
  {
    uint8_t setaddress = 0;
    fors(5) {
      if (nrf24->config.rxAddress0[i]) {
        setaddress = 1;
        break;
      }
    }
    if (setaddress) {
      writeRegisterMultiBytes(nrf24,0x0a, nrf24->config.rxAddress0, 5);
    }
  }
  {
    uint8_t setaddress = 0;
    fors(5) {
      if (nrf24->config.rxAddress1[i]) {
        setaddress = 1;
        break;
      }
    }
    if (setaddress) {
      writeRegisterMultiBytes(nrf24,0x0b, nrf24->config.rxAddress1, 5);
      fors(4) {
        writeRegister(nrf24,0x0c + i, nrf24->config.rxAddress2_6[i]);
      }
    }
  }
  {
    uint8_t setaddress = 0;
    fors(5) {
      if (nrf24->config.txAddress[i]) {
        setaddress = 1;
        break;
      }
    }
    if (setaddress) {
      writeRegisterMultiBytes(nrf24,0x10, nrf24->config.txAddress, 5);
    }
  }
  writeRegister(nrf24,0x1c, 0x3F);
  writeRegister(nrf24,0x1d, 0x05);
}

void nrf24_setAddress1(nrf24_t* nrf24, uint8_t* address) {
  uint8_t setaddress = 0;
  fors(5) {
    if (nrf24->config.rxAddress1[i] != address[i]) {
      setaddress = 1;
      break;
    }
  }
  if (setaddress) {
    memcmp(nrf24->config.rxAddress1, address, 5);
    writeRegisterMultiBytes(nrf24,0x0b, nrf24->config.rxAddress1, 5);
  }
}

void nrf24_init(nrf24_t* nrf24) {
  nrf24_hw_init(nrf24);
  commandInit(nrf24);
}

void nrf24_enable(nrf24_t* nrf24, uint8_t enable) {
  //nrf24_hw_enable(nrf24, enable);
  if (enable) {
    // schedule_repeat(INTERRUPT_SCHEDULE_ID, sequenceChecker, 1, (void**) nrf24);
  } else {
    // schedule_cancel(INTERRUPT_SCHEDULE_ID);
  }
}
