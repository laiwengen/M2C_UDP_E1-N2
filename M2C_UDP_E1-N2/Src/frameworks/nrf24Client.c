#include "stdlib.h"
#include "string.h"
#include "frameworks/number.h"
#include "frameworks/package.h"
#include "frameworks/tick.h"
#include "frameworks/wireless.h"
#include "frameworks/nrf24Client.h"

const uint8_t g_nrf24_boardcastAddress[] = {0x4e, 0x23, 0xcf, 0x87, 0x83};
const uint8_t g_nrf24_p2pAddressLastBytes[] = {0x4e, 0x23, 0xcf, 0x87, 0x83};


#if 1 //cache
  #define cacheBuffer(c, i) (((uint8_t*)(c)) + sizeof(nrf24_cache_t) + i)

  static uint8_t cacheValid(nrf24_cache_t* cache) {
    return cache->gotPages == cache->totalPages;
  }

  static uint16_t cacheSize(nrf24_cache_t* cache) {
    return (cache->totalPages-1) * NRF24_PAGE_DATA_SIZE + cache->lastPageSize;
  }

  static nrf24_cache_t* cacheCreate(uint8_t* bytes, int8_t size) {
    nrf24_page_t* page = (nrf24_page_t*) bytes;
    nrf24_cache_t* cache = (nrf24_cache_t*) malloc(sizeof(nrf24_cache_t) + page->total * NRF24_PAGE_DATA_SIZE);
    cache->gotPages = 0;
    cache->totalPages = page->total;
    cache->line = page->line;
    cache->tick = tick_ms();
    return cache;
  }

  static uint8_t cacheMatch(nrf24_cache_t* cache, uint8_t* bytes, int8_t size) {
    nrf24_page_t* page = (nrf24_page_t*) bytes;
    if (cache && page && page->index == cache->gotPages && page->total == cache->totalPages && page->line == cache->line) {
      if (page->index == page->total - 1 || size == NRF24_PAGE_MAX_SIZE) {
        return 1;
      }
    }
    return 0;
  }

  // need check matched
  static void cachePush(nrf24_cache_t* cache, uint8_t* bytes, int8_t size) {
    nrf24_page_t* page = (nrf24_page_t*) bytes;
    memcpy(cacheBuffer(cache, page->index*NRF24_PAGE_DATA_SIZE), &page->data, size - NRF24_PAGE_INFO_SIZE);
    if (page->index == page->total - 1) {
      cache->lastPageSize = size - NRF24_PAGE_INFO_SIZE;
    }
    cache->gotPages++;
    cache->tick = tick_ms();
  }

  static uint8_t cachePnG(nrf24_cache_t** cache, uint8_t* bytes, int8_t size, uint8_t** outBuffer, int16_t* outSize) {
    nrf24_page_t* page = (nrf24_page_t*) bytes;
    if (page) {
      if (page->total <= 1) {
        *outBuffer = bytes + NRF24_PAGE_INFO_SIZE;
        *outSize = size - NRF24_PAGE_INFO_SIZE;
        return 1;
      } else {
        if (!cacheMatch(*cache, bytes, size)){
          free(*cache);
          *cache = cacheCreate(bytes, size);
        }
        if (*cache) {
          cachePush(*cache, bytes, size);
          if (cacheValid(*cache)) {
            *outBuffer = cacheBuffer(*cache,0);
            *outSize = cacheSize(*cache);
            return 1;
          }
        }
      }
    }
    return 0;
  }
#endif

#if 1 // sequence
  static void initSequence(nrf24_sequence_t* sequence, nrf24_sequenceType_t type) {
    sequence->type = type;
    sequence->doTimes = 0;
    sequence->totalTimes = 1;
    sequence->beforeDoFunction = 0;
    sequence->onTimeoutFunction = 0;
  }

  static void initTransmit(nrf24_transmit_t* transmit, uint32_t address, uint8_t* bytes, int16_t size) {
    if (address == 0) {
      memcpy(transmit->address, g_nrf24_boardcastAddress, 5);
    } else {
      memcpy(transmit->address, &address, 4);
      transmit->address[4] = g_nrf24_p2pAddressLastBytes[0];
    }
    transmit->timeout = 250;
    transmit->bytes = bytes;
    transmit->size = size;
    transmit->noack = 1;
    transmit->ackTimeout = 0;
    transmit->retransmitTimes = 0;
  }

  static void initReceive(nrf24_receive_t* receive, uint32_t timeout, uint8_t pipe, nrf24_dataFunction_t onReceived, void** params) {
    receive->timeout = timeout;
    receive->pipe = pipe;
    receive->onReceived = onReceived;
    receive->params = params;
  }

  static void initGroup(nrf24_group_t* group) {
    group->stage = 0;
    group->subsequences = 0;
    group->stageChangedFunction = 0;
  }

  static int16_t disassembleToPages(uint8_t id, uint8_t line, uint8_t* buffer, int16_t size, uint8_t*** outBytes, int16_t** outSizes) {
    int16_t pageCount = max((size - 1) / NRF24_PAGE_DATA_SIZE + 1, 1);
    uint8_t** bytes = (uint8_t**) malloc (pageCount * sizeof(uint8_t*));
    int16_t* sizes = (int16_t*) malloc (pageCount * sizeof(int16_t));
    if (!bytes || !sizes){
      free(bytes);
      free(sizes);
      return 0;
    }
    fors(pageCount) {
      int8_t dataSize;
      if ( i == pageCount - 1 ) {
        dataSize = size % NRF24_PAGE_DATA_SIZE;
        if (dataSize == 0) {
          dataSize = NRF24_PAGE_DATA_SIZE;
        }
      } else {
        dataSize = NRF24_PAGE_DATA_SIZE;
      }
      uint8_t* buffer = (uint8_t*) malloc (dataSize + NRF24_PAGE_INFO_SIZE);
      if (buffer) {
        nrf24_page_t* page = (nrf24_page_t*) buffer;
        page->id = id;
        page->index = i;
        page->total = pageCount;
        page->line = line;
        memcpy(&page->data, buffer+i*NRF24_PAGE_DATA_SIZE, dataSize);
      } else {
        foris(j, i) {
          free(bytes[j]);
        }
        free(bytes);
        free(sizes);
        return 0;
      }
    }
    //TODO
    *outBytes = bytes;
    *outSizes = sizes;
    return 1;
  }

  nrf24_sequence_t* generateTransmitSequence(uint32_t address, uint8_t id, uint8_t* line, uint8_t* buffer, int16_t size) {
    nrf24_sequence_t* sequence = (nrf24_sequence_t*) malloc(sizeof(nrf24_sequence_t));
    if (!sequence) {
      return 0;
    }
    uint8_t** bytes;
    int16_t* sizes;
    int16_t pageSize = disassembleToPages(id, *line, buffer, size, &bytes, &sizes);
    if (pageSize > 0) {
      if (pageSize == 1) {
        //
        initSequence(sequence, NRF24_SEQUENCE_TYPE_transmit);
        initTransmit(&sequence->data.transmit, address, bytes[0], sizes[0]);
      } else{
        initSequence(sequence, NRF24_SEQUENCE_TYPE_group);
        initGroup(&sequence->data.group);
        fors(pageSize) {
          nrf24_sequence_t* s = (nrf24_sequence_t*) malloc(sizeof(nrf24_sequence_t));
          if (s) {
            initSequence(sequence, NRF24_SEQUENCE_TYPE_transmit);
            initTransmit(&sequence->data.transmit, address, bytes[i], sizes[i]);
            if (!list_add(&sequence->data.group.subsequences, s)) {
              nrf24_deleteSequence(s);
            }
          } else {
            free(bytes[i]);
          }
        }
      }
      free(bytes);
      free(sizes);
    }
    (*line) ++;
    return sequence;
  }
  nrf24_sequence_t* generateReceiveSequence(nrf24_client_t* client, nrf24_dataFunction_t onReceived, uint32_t timeout, uint8_t pipe) {
    nrf24_sequence_t* sequence = (nrf24_sequence_t*) malloc(sizeof(nrf24_sequence_t));
    if (!sequence) {
      return 0;
    }
    initSequence(sequence, NRF24_SEQUENCE_TYPE_receive);
    initReceive(&sequence->data.receive, timeout, pipe, onReceived, (void**)client);
    return sequence;
  }
  nrf24_sequence_t* generateGroupSequence(int8_t doTimes, nrf24_sequence_t** sequences, int16_t size) {
    nrf24_sequence_t* sequence = (nrf24_sequence_t*) malloc(sizeof(nrf24_sequence_t));
    if (!sequence) {
      return 0;
    }
    initSequence(sequence, NRF24_SEQUENCE_TYPE_group);
    sequence->totalTimes = doTimes;
    initGroup(&sequence->data.group);
    fors(size) {
      if (!list_add(&sequence->data.group.subsequences, sequences[i])) {
        nrf24_deleteSequence(sequences[i]);
      }
    }
    return sequence;
  }
  nrf24_sequence_t* generateWaitSequence(uint32_t timeout) {
    nrf24_sequence_t* sequence = (nrf24_sequence_t*) malloc(sizeof(nrf24_sequence_t));
    if (!sequence) {
      return 0;
    }
    initSequence(sequence, NRF24_SEQUENCE_TYPE_wait);
    sequence->totalTimes = 1;
    sequence->data.wait.timeout = timeout;
    return sequence;
  }

#endif

#if 1// driver
  static void setAddress(nrf24_client_t* client, uint32_t address) {
    client->address = address;
    uint8_t buffer[5];
    memcpy(buffer, &address, 4);
    buffer[4] = g_nrf24_p2pAddressLastBytes[0];
    nrf24_setAddress1(client->nrf24, buffer);
  }
#endif

#if 1 // end
  static uint8_t endRegister4R(nrf24_t* nrf24, nrf24_sequence_t* sequence, uint8_t* bytes, int8_t size, void** params) {
    int8_t pipe = (nrf24->status & 0x0e) >> 1;
    nrf24_page_t* page = (nrf24_page_t*) bytes;
    nrf24_client_t* client = (nrf24_client_t*)params;
    if (pipe == NRF24_PIPE_broadcast) {
      package_element_t* root = package_deserialize(&page->data, size - NRF24_PAGE_INFO_SIZE);
      if (root) {
        client->asEnd.id = package_getInt(package_findValueByIntKey(root, 0));
        client->asEnd.routerAddress = package_getInt(package_findValueByIntKey(root, 2));
        setAddress(client, package_getInt(package_findValueByIntKey(root, 1)));
        package_delete(root);
        return 1;
      }
    }
    return 0;
  }

  static void endRegister(nrf24_client_t* client, nrf24_registerConfig_t* config) {
    package_element_t* root = package_newObject();
    if (!root) {
      return;
    }
    package_addKeyValue(root, package_newInt8(0), package_newInt8(config->priorityBase));
    package_addKeyValue(root, package_newInt8(1), package_newInt8(config->priorityRange));
    if (config->usePreferRouterAddress) {
      package_addKeyValue(root, package_newInt8(2), package_newInt32(config->preferRouterAddress));
    }
    if (config->receivingWhileIdle) {
      package_addKeyValue(root, package_newInt8(3), package_newBoolean(config->receivingWhileIdle));
    }
    uint8_t* serialized;
    int16_t size = package_serialize(root, &serialized);
    package_delete(root);
    if (size == 0) {
      return;
    }
    nrf24_sequence_t* tsequence = generateTransmitSequence(0, 0, &client->line, serialized, size);
    free(serialized);
    nrf24_sequence_t* rsequence = generateReceiveSequence(client, endRegister4R, (config->priorityRange+1)*250, 1<<NRF24_PIPE_broadcast);
    nrf24_sequence_t* sequences[] = {tsequence, rsequence};
    nrf24_sequence_t* gsequence = generateGroupSequence(5, sequences, 2);
    if (tsequence && rsequence && gsequence) {
      nrf24_addSequence(client->nrf24, gsequence);
    } else {
      nrf24_deleteSequence(tsequence);
      nrf24_deleteSequence(rsequence);
      nrf24_deleteSequence(gsequence);
    }
  }

  static uint8_t endCommunicate4R(nrf24_t* nrf24, nrf24_sequence_t* sequence, uint8_t* bytes, int8_t size, void** params) {
    int8_t pipe = (nrf24->status & 0x0e) >> 1;
    nrf24_page_t* page = (nrf24_page_t*) bytes;
    nrf24_client_t* client = (nrf24_client_t*)params;
    if (pipe == NRF24_PIPE_peer2peer) {
      if (page->id == 0 && page->line != client->asEnd.line) {
        uint8_t* buffer;
        int16_t bSize;
        if (cachePnG(&client->asEnd.cache, bytes, size, &buffer, &bSize)) {
          client->asEnd.line = page->line;
          client->asEnd.onCommunicated(client, buffer, bSize);
          return 1;
        } else {
          nrf24_setTimeout(nrf24, sequence->data.receive.timeout);
          // TODO reset timeout
        }
      }
    }
    return 0;
  }

  static void endCommunicate(nrf24_client_t* client, uint8_t* bytes, int16_t size) {
    nrf24_sequence_t* tsequence = generateTransmitSequence(client->asEnd.routerAddress, 0, &client->line, bytes, size);
    nrf24_sequence_t* rsequence = generateReceiveSequence(client, endCommunicate4R, 2*250, 1<<NRF24_PIPE_peer2peer);
    nrf24_sequence_t* sequences[] = {tsequence, rsequence};
    nrf24_sequence_t* gsequence = generateGroupSequence(5, sequences, 2);
    if (tsequence && rsequence && gsequence) {
      nrf24_addSequence(client->nrf24, gsequence);
    } else {
      nrf24_deleteSequence(tsequence);
      nrf24_deleteSequence(rsequence);
      nrf24_deleteSequence(gsequence);
    }
  }
#endif

#if 1 // router

  static uint8_t generateId(nrf24_client_t* client) {
    while (1) {
      uint8_t id = ++ client->asRouter.registerIdGenerator;
      if (id != 0 && list_findById(&client->asRouter.registereds, id) == 0) {
        return id;
      }
    }
  }

  static uint8_t validAddress(uint32_t address) {
    return address != 0;
  }

  static uint32_t generateAddress(nrf24_client_t* client, uint32_t preferAddress) {
    uint32_t address;
    if (preferAddress == 0) {
      address = rand();
    } else {
      address = preferAddress;
    }
    while (1) {
      if (validAddress(address) && list_findByBytes(&client->asRouter.registereds, (uint8_t*)&address, 8, 4) == 0) {
        return address;
      }
      address = rand();
    }
  }

  static nrf24_register_t* findRegisterById(nrf24_client_t* client, uint32_t id) {
    return (nrf24_register_t*)list_findById(&client->asRouter.registereds, id);
  }

  static uint8_t routerRegister4B(nrf24_client_t* client, uint8_t const* buffer, int8_t size) {
    // generate address and register device id;
    package_element_t* e2r = package_deserialize(buffer, size);
    nrf24_registerConfig_t config = {0};
    config.priorityBase = package_getInt(package_findValueByIntKey(e2r, 0));
    config.priorityRange = package_getInt(package_findValueByIntKey(e2r, 1));
    package_element_t* addressE = package_findValueByIntKey(e2r, 2);
    if (addressE) {
      config.usePreferRouterAddress = 1;
      config.preferRouterAddress = package_getInt(addressE);
    } else {
      config.usePreferRouterAddress = 0;
    }
    package_element_t* rwiE = package_findValueByIntKey(e2r, 3);
    if (rwiE) {
      config.receivingWhileIdle = package_getBoolean(rwiE);
    } else {
      config.receivingWhileIdle = 0; //default
    }
    uint8_t preferAddressMatched = config.usePreferRouterAddress && client->address == config.preferRouterAddress;
    if (preferAddressMatched || client->asRouter.priority >= config.priorityBase && client->asRouter.priority < config.priorityBase + config.priorityRange) {
      nrf24_register_t* r = (nrf24_register_t*) malloc (sizeof(nrf24_register_t));
      if (r) {
        r->id = generateId(client);
        r->tick = tick_ms();
        r->address = generateAddress(client, config.usePreferRouterAddress?config.preferRouterAddress:0);
        r->cache = 0;
        r->line = 0;
        r->receivingWhileIdle = config.receivingWhileIdle;
        r->upload = 0;
        client->asRouter.initRegister(client,r);
        list_addFirst(&client->asRouter.registereds, r);
        package_element_t* r2e = package_newObject();
        if (r2e) {
          package_addKeyValue(r2e, package_newInt8(0), package_newInt8(r->id));
          package_addKeyValue(r2e, package_newInt8(1), package_newInt32(r->address));
          package_addKeyValue(r2e, package_newInt8(2), package_newInt32(client->address));
          uint8_t* serialized;
          int16_t size = package_serialize(r2e, &serialized);
          package_delete(r2e);
          if (size) {
            nrf24_sequence_t* tsequence = generateTransmitSequence(0, 0, &client->line, serialized, size);
            free(serialized);
            if (tsequence) {
              if (!preferAddressMatched) {
                nrf24_sequence_t* wsequence = generateWaitSequence((config.priorityBase + config.priorityRange - client->asRouter.priority) * 250);
                if (wsequence) {
                  nrf24_addSequence(client->nrf24,wsequence);
                  nrf24_addSequence(client->nrf24,tsequence);
                }
              } else {
                nrf24_addSequence(client->nrf24,tsequence);
              }
            }
          }
        }
      }
    }
    return 0;
  }

  static void packageMega(package_element_t* from, package_element_t* to) {
    list_t** list = package_elementPointer(from, list_t*, 0);
    fortl(package_pair_t*, list) {
      package_pair_t* pair = list_remove(list, v);
      package_deletePair(package_removeFirstPairByKey(to, pair->key));
      package_addPair(to, pair);
    }
    package_delete(from);
  }

  static uint8_t isIdentified(package_element_t* root) {
    if (package_findValueByIntKey(root, wireless_keyInt(root, sessionId))) {
      return 1;
    } else if (
      package_findValueByIntKey(root, wireless_keyInt(root, deviceId))
      && package_findValueByIntKey(root, wireless_keyInt(root, version))
    ) {
      return 1;
    }
    return 0;
  }

  static void ignoreIdentifies(package_element_t* root) {
    fortl(package_pair_t*, package_elementPointer(root,list_t*,0)) {
      if (package_getInt(v->key)<0x10) {
        package_setFlag(v, ignore);
      }
    }
  }

  static uint8_t routerCommunicate(nrf24_client_t* client, nrf24_register_t* reg, uint8_t noack) {
    package_element_t* download = reg->download;
    if (download == 0) {
      download = package_newObject();
    }
    if (download) {
      ignoreIdentifies(download);
      uint8_t* serialized;
      int16_t size = package_serialize(download, &serialized);
      if (size) {
        nrf24_sequence_t* tsequence = generateTransmitSequence(reg->address, 0, &client->line, serialized, size);
        free(serialized);
        if (tsequence) {
          if (!noack) {
            tsequence->totalTimes = 3;
          }
          nrf24_addSequence(client->nrf24,tsequence);
          return 1;
        }
      }
    }
    return 0;
  }

  static uint8_t routerCommunicate4P(nrf24_client_t* client, nrf24_register_t* reg, uint8_t* buffer, int8_t size) {
    package_element_t* e2r = package_deserialize(buffer, size);
    // (package_element_t *e, package_element_t** key, package_element_t** value);
    if (e2r) {
      package_element_t* upload = reg->upload;
      if (upload) {
        packageMega(e2r, upload);
      }
    }
    if (!isIdentified(reg->upload)) {
      package_element_t* r2e = package_newObject();
      if (r2e) {
        package_element_t* errorE = package_newObject();
        if (errorE) {
          package_addKeyValue(errorE, package_newInt8(wireless_keyInt(error, code)), package_newInt32(WIRELESS_ERROR_code_sessionIdTimeout));
          package_addKeyValue(r2e, package_newInt8(wireless_keyInt(root, error)), errorE);
        }
        uint8_t* serialized;
        int16_t size = package_serialize(r2e, &serialized);
        package_delete(r2e);
        if (size) {
          nrf24_sequence_t* tsequence = generateTransmitSequence(reg->address, 0, &client->line, serialized, size);
          free(serialized);
          if (tsequence) {
            nrf24_addSequence(client->nrf24,tsequence);
            return 1;
          }
        }
      }
    } else {
      if (routerCommunicate(client, reg, 1)) {
        return 1;
      }
    }
    return 0;
  }

#endif

static uint8_t routerIdle4R(nrf24_t* nrf24, nrf24_sequence_t* sequence, uint8_t* bytes, int8_t size, void** params) {
  int8_t pipe = (nrf24->status & 0x0e) >> 1;
  nrf24_page_t* page = (nrf24_page_t*) bytes;
  nrf24_client_t* client = (nrf24_client_t*)params;
  if (pipe == NRF24_PIPE_broadcast) {
    if (page->id == 0) {
      routerRegister4B(client, bytes + NRF24_PAGE_INFO_SIZE, size - NRF24_PAGE_INFO_SIZE);
      return 0;
    }
  } else if (pipe == NRF24_PIPE_peer2peer) {
    if (page->id != 0) {
      nrf24_register_t* r = findRegisterById(client, page->id);
      if (r && r->line != page->line) {
        uint8_t* buffer;
        int16_t bSize;
        r->tick = tick_ms();
        if (cachePnG(&r->cache, bytes, size, &buffer, &bSize)) {
          r->line = page->line;
          routerCommunicate4P(client, r, buffer, bSize);
          return 0;
        }
      }
    }
  }
  return 0;
}

uint8_t nrf24Client_routerDownloadUpdated(nrf24_client_t* client, nrf24_register_t*reg) {
  return routerCommunicate(client, reg, 0);
}

void nrf24Client_init(nrf24_client_t* client, uint8_t receivingWhileIdle) {
  nrf24_t* nrf24 = client->nrf24;
  nrf24_init(nrf24);
  if (receivingWhileIdle) {
    nrf24->idle.type = NRF24_IDLE_TYPE_receive;
    initReceive(&nrf24->idle.data.receive, 0, 0x3F, routerIdle4R, (void**)client);
  } else {
    nrf24->idle.type = NRF24_IDLE_TYPE_standby;
  }
}
void nrf24Client_enable(nrf24_client_t* client, uint8_t enable) {
  nrf24_enable(client->nrf24, enable);
  // TODO reset state
}
