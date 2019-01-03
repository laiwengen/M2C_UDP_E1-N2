
#include "stdlib.h"
#include "string.h"
#include "frameworks/list.h"
#include "frameworks/number.h"
#include "frameworks/strings.h"
#include "frameworks/uota.h"

int16_t uota_stringToBytes(char* str, uint8_t* out) {
	int16_t slen = strlen(str);
	if (slen == 0 || (slen & 1) == 0) {
		return 0;
	}
	if (str[0] != ':') {
		return 0;
	}
	fors (slen >> 1) {
		out[i] = number_fromHexString(str + i * 2 + 1, 2);
	}
  return slen >> 1;
}

uint8_t uota_bytesToObject(uint8_t* bytes, int16_t size, uota_line_t* out) {
  if (size != bytes[0] + 5) {
    return 0;
  }
  uint8_t sum = 0;
  fors(size) {
    sum += bytes[i];
  }
  if (sum != 0) {
    return 0;
  }
  out->length = bytes[0];
  out->address = (bytes[1] << 8) + bytes[2];
  out->type = bytes[3];
  memcpy(out->data, bytes + 4, out->length);
  out->verify = bytes[size-1];
  return 1;
}

static uota_t* g_uota = {0};

void uota_init(uota_t* uota) {
  g_uota = uota;
}

static void init(char* version, int32_t total) {
  if (g_uota != 0) {
    free(g_uota->version);
    g_uota->version = string_duplicate(version, strlen(version));
    g_uota->total = total;
    g_uota->base = 0;
    uota_hw_save(g_uota);
  }
}

static void setBase(uint32_t base) {
  if (g_uota != 0) {
    g_uota->base = base;
    uota_hw_save(g_uota);
  }
}

static uint8_t load(void) {
  return uota_hw_load(g_uota);
}

uint8_t uota_push(char* version, int32_t line, int32_t total, uint8_t* bytes, int16_t size) {
  if (g_uota->version == 0) {
    load();
  }
  if (g_uota->version == 0 || strcmp(g_uota->version, version) != 0) {
    if (line!=0) {
      return 0;
    }
    init(version, total);
    g_uota->resetFunction(g_uota);
  }
  uota_line_t uota_line;
  if (uota_bytesToObject(bytes, size, &uota_line)) {
    switch (uota_line.type) {
      case 0x00: //write
        g_uota->writeFunction(g_uota, uota_line.address + g_uota->base, uota_line.data, uota_line.length);
      break;
      case 0x01: //finish
        g_uota->finishFunction(g_uota);
        uota_hw_clear(g_uota);
      break;
      case 0x04: //setaddress
        setBase((uota_line.data[0] << 24) | (uota_line.data[1] << 16));
      break;
      case 0x05: //ob
        //TODO?
      break;
      default:
      break;
    }
    return 1;
  }
  return 0;
}
