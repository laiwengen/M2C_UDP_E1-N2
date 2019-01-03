#include "libs/cMD5/cMD5.h"
#include "stdlib.h"

uint8_t md5_convert(uint8_t* buffer, int16_t size, uint8_t* out) {
  MD5_CTX* ctx = (MD5_CTX*) malloc (sizeof(MD5_CTX));
  if (!ctx) {
    return 0;
  }
  MD5_Init(ctx);
  MD5_Update(ctx, buffer, size);
  MD5_Final(out, ctx);
  free(ctx);
	return 1;
}
