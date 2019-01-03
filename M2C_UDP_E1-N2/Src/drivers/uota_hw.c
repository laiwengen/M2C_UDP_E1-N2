#include "frameworks/fpe.h"
#include "frameworks/uota.h"
#include "frameworks/list.h"
#include "frameworks/strings.h"
#include "string.h"

#define UOTA_FPE_ADDRESS 0x1f37ab0c
#define UOTA_VERSION_MAX_SIZE 32

void uota_hw_save(uota_t* uota) {
  fpe_writeBytes(UOTA_FPE_ADDRESS, (uint8_t*) uota->version, strlen(uota->version) + 1);
  fpe_write(UOTA_FPE_ADDRESS + 0x100, uota->total);
  fpe_write(UOTA_FPE_ADDRESS + 0x101, uota->base);
}
void uota_hw_clear(uota_t* uota) {
  fors(UOTA_VERSION_MAX_SIZE/4) {
    fpe_write(UOTA_FPE_ADDRESS + i, UINT32_MAX);
  }
  fpe_write(UOTA_FPE_ADDRESS + 0x100, UINT32_MAX);
  fpe_write(UOTA_FPE_ADDRESS + 0x101, UINT32_MAX);
}
uint8_t uota_hw_load(uota_t* uota) {
  char buffer[UOTA_VERSION_MAX_SIZE];
  uota->total = fpe_readOr(UOTA_FPE_ADDRESS + 0x100, 0);
  uota->base = fpe_readOr(UOTA_FPE_ADDRESS + 0x101, 0);
  if (fpe_readBytes(UOTA_FPE_ADDRESS, (uint8_t*)buffer, UOTA_VERSION_MAX_SIZE)) {
    buffer[UOTA_VERSION_MAX_SIZE-1] = 0;
    uota->version = string_duplicate(buffer, strlen(buffer));
		return 1;
  }
	return 0;
}
