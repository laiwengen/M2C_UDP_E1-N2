#include "stdlib.h"
#include "string.h"
#include "frameworks/number.h"
#include "libs/cQRCode/cQRCode.h"

#define VERSION 8
#define SIZE (4 * VERSION + 17)
#define ECC ECC_MEDIUM

uint8_t* qrcode_convert(uint8_t* buffer, int16_t size) {
  QRCode cqr;
  uint8_t* bytes = (uint8_t*) malloc(qrcode_getBufferSize(VERSION));
  if (bytes) {
    if (qrcode_initBytes(&cqr, bytes, VERSION, ECC, buffer, size) == 0) {
      // memset(bytes, 0xaa, qrcode_getBufferSize(VERSION));
      return bytes;
    }
    free(bytes);
  }
  return 0;
}

uint8_t qrcode_get(uint8_t* buffer, int16_t x, int16_t y) {
  if (buffer == 0 || x < 0 || x >= SIZE || y < 0 || y >= SIZE) {
      return 0;
  }
  return number_bitGetBE(buffer, x + y * SIZE);
}
