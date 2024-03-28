#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"

uint16_t CRC16(uint8_t *dat, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif
