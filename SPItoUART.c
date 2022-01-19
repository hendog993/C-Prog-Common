/*
 File: Holds UART implementation layer for SPI to UART chip - should be used for MAX3109 Uart 
 Author: Henry Gilbert
 Date: 12 December 2021
 */

#include "SPItoUART.h"
#include <stdbool.h> 

/* Global circular buffer definitions */
circBuffer_t* pUART3rxCircBuff ;
circBuffer_t* pUART3txCircBuff ;

circBuffer_t* pUART4rxCircBuff ;
circBuffer_t* pUART4txCircBuff ;

bool isUART3initialized = false ;
bool isUART4initialized = false ;

/* Function Prototypes*/

/* The UART line configs are already setup during MAX chip setup. Only need setup for Circular Buffer */
void InitializeUART3 ( circBuffer_t* uart3RxCircBuff, circBuffer_t* uart3TxCircBuff )
{
   if ( ( NULL == uart3RxCircBuff ) ||
        ( NULL == uart3TxCircBuff ) )
   {
      return ; // Error, invalid params
   }

   pUART3rxCircBuff = uart3RxCircBuff ;
   pUART3txCircBuff = uart3TxCircBuff ;
   isUART3initialized = true ;
   return ;
}

void InitializeUART4 ( circBuffer_t* uart4RxCircBuff, circBuffer_t* uart4TxCircBuff )
{
   if ( ( NULL == uart4RxCircBuff ) ||
        ( NULL == uart4TxCircBuff ) )
   {
      return ; // Error, invalid params
   }
   pUART4rxCircBuff = uart4RxCircBuff ;
   pUART4txCircBuff = uart4TxCircBuff ;
   isUART4initialized = true ;
   return ;
}

/* Get FIFO fill level, pop values from FIFO to circular buffer */
uint16_t ReadDataFromUARTBuffer ( const MAX3109_UART_SELECTION channel, circBuffer_t * rxBuf )
{
   if ( NULL == rxBuf )
   {
      return 0 ;
   }
   
   uint8_t numBytesToRead = MAXGetUARTFIFOLevel ( channel,
           max3109_RxFIFOLvl ) ;
   
   if ( ( numBytesToRead > MAXIMUM_MAX3109_FIFO_SIZE_IN_BYTES ) ||
        ( 0 == numBytesToRead ) )
   {
      return 0 ; // Error, or nothing to read.
   }


   uint8_t counter ;
   for ( counter = 0 ; counter < numBytesToRead ; counter++ )
   {
      uint8_t poppedValue = MAXPopSingleValueFromUARTRxFIFO ( channel ) ;

      cb_push ( rxBuf, poppedValue ) ; // TODO replace this with CB flush in 
   }
   
   return counter ; // Return num of bytes read 
}

uint16_t WriteDataToUARTTransmitBuffer ( const MAX3109_UART_SELECTION channel, circBuffer_t * txBuf, uint8_t numBytesToWrite )
{
   uint8_t counter ;
   uint8_t numBytesWritten = 0 ;
   uint8_t errorCounter = 0 ;
   
   /* Disable transmit buffer while writing */
   
   for ( counter = 0 ; counter < numBytesToWrite ; counter++ )
   {
      uint8_t dataToWrite = cb_peek ( txBuf, counter ) ;
      errorCounter += MAXPushSingleValueToUARTTxFIFO ( channel, dataToWrite ) ;
      numBytesWritten++ ;
   }
   /* Enable transmission when finished writing to buffer */
   
   
   cb_advance_tail ( txBuf, numBytesWritten ) ;
   return ( 0 == errorCounter ) ? ( numBytesWritten ) : 0 ;
}