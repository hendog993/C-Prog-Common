/*
File: MAX3109 Header File
Author: Henry Gilbert
Description: Typedefs and generic parameters for the MAX3109 SPI to UART chip. Describes the register 
    addresses used in 16bit spi mode, defines a typedef for the decoding interrupts, differentiates read and 
    write mode, creates a typedef for the two UART channels, and declares function prototypes. 
 */

#ifndef MAX_3109_H
#define MAX_3109_H
#include <stdint.h>
#include<stdbool.h>

#define MAXIMUM_MAX3109_FIFO_SIZE_IN_BYTES 128 

/* Register addresses all configured in 16bit SPI mode */
typedef enum MAX3109_REGISTER_ADDRESS_VALUE_t {
    max3109_TRxHR = 0x0000, // Transmitter and Receiver Hold Register
    max3109_IRQEn = 0x0100,
    max3109_ISR_Status = 0x0200,
    max3109_LSRIntEn = 0x0300,
    max3109_LSR = 0x0400,
    max3109_SpclChrIntEn = 0x0500,
    max3109_SpclCharInt = 0x0600,
    max3109_STSIntEn = 0x0700,
    max3109_STSInt = 0x0800,
    max3109_MODE1 = 0x0900,
    max3109_MODE2 = 0x0A00,
    max3109_LCR = 0x0B00,
    max3109_RxTimeOut = 0x0C00,
    max3109_HDplxDelay = 0x0D00,
    max3109_IrDA = 0x0E00,
    max3109_FlowLvl = 0x0F00,
    max3109_FIFOTrgLvl = 0x1000,
    max3109_TxFIFOLvl = 0x1100,
    max3109_RxFIFOLvl = 0x1200,
    max3109_PLLConfig = 0x1A00,
    max3109_BRGConfig = 0x1B00,
    max3109_DIVLSB = 0x1C00,
    max3109_DIVMSB = 0x1D00,
    max3109CLKSource = 0x1E00,
    max3109_GlobalIRQ = 0x1F00,
    max3109_GloblComnd = 0x1F00,
    max3109_TxSync = 0x2000,
    max3109_SynchDelay1 = 0x2100,
    max3109_SynchDelay2 = 0x2200,
    max3109_Timer1 = 0x2300,
    max3109_Timer2 = 0x2400,
    max3109_RevID = 0x2500
} MAX3109_REGISTER_ADDRESS_VALUE;

/* UART Selections for 16 bit SPI mode. For AFC004, 0 is ADC, 1 is GPS */
typedef enum UART_SELECTION_t {
    UART_0 = 0x0000,
    UART_1 = 0x2000,
} MAX3109_UART_SELECTION;

typedef enum READ_WRITE_MODE_t {
    READ_MAX = 0x0,
    WRITE_MAX = 0x8000 // Write is active HIGH on 16th bit of cmd word 
} MAX3109_READ_WRITE_MODE;


/* Function: Initialize_MAX3109()
 * Description - Performs initial read known register value, writes registers from configuration data, performs readback tests on written registers.
 * Returns 0 on success, 1 on failure. */
uint8_t MAXInitializeMAX3109( const uint8_t maxPLLConfiguration,
                           const uint8_t maxClockConfig,
                           const uint8_t uartLineConfig );

uint8_t MAXGetUARTFIFOLevel(const MAX3109_UART_SELECTION channel,
                            const MAX3109_REGISTER_ADDRESS_VALUE buffer);

uint8_t MAXPopSingleValueFromUARTRxFIFO(const MAX3109_UART_SELECTION channel);

uint8_t MAXPushSingleValueToUARTTxFIFO(const MAX3109_UART_SELECTION channel,
                                const uint8_t valueToWrite);

bool MAXIsUARTReceiveReadyToRead( const MAX3109_UART_SELECTION channel );
#endif 