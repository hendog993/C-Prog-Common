#ifndef SPI_TO_UART_H
#define SPI_TO_UART_H

#include "MAX3109.h"
#include "CircularBuffer.h"

uint16_t ReadDataFromUARTBuffer( const MAX3109_UART_SELECTION channel, circBuffer_t * cb );
uint16_t WriteDataToUARTTransmitBuffer(const MAX3109_UART_SELECTION channel, circBuffer_t * txBuf, uint8_t numBytesToWrite  );

void InitializeUART3(circBuffer_t* uart3RxCircBuff, circBuffer_t* uart3TxCircBuff );
void InitializeUART4(circBuffer_t* uart4RxCircBuff, circBuffer_t* uart4TxCircBuff );

#endif 

 