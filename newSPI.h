#ifndef NEW_SPI
#define NEW_SPI

#include <stdint.h>
#include "p30F6014A.h"

void InitializeSPI(uint16_t u16ModeConfig,
        uint16_t u16SPI1StatusConfig);

uint8_t SPIreadWriteWord(uint16_t writeData,
        uint16_t * readData);

#endif