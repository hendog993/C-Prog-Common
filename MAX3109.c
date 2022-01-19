/* Hardware Implementation Layer for MAX3109 SPI to UART Chip. 
 * Author : Henry Gilbert
 * Date: 12 December 2021 
 * 
 * Master Todo List: Add SPI Burst access , touch up interrupts, fix disable and enable tx FIFO, implement Rx timeout
 * 
 * Module Description: All access to the MAX3109 UART chip involves reading and writing to specified registers based on
 * the users desired functionality. All reads and writes are performed in 16 bit SPI mode. A write operation involves
 * creating a command byte with bit 7 toggled high, the UART channel on bit 5, and the address in the following 5 bits 4-0.
 * The value to write is the second byte of the 16 bit SPI message. A read operation is performed by setting bit 7 low, and the 
 * desired channel and register based on the format below. The last 8 bits of the 16 bit SPI message sent during a READ cycle
 * are irrelevant. The byte used as a return parameter is the 8 bit value read back from the SPI RxBuf. 
 * 
 * Note: All SPI read operations return the desired 8 bit value, along with 2 extra status bytes of the interrupt registers.
 * 
 *                           7    6      5         4      3      2      1      0    
 *SPI Command byte format - W/R   0   Channel   Bit 4   Bit 3  Bit 2  Bit 1   Bit 0  -> append second 8 bits with data for write, zeros for read. */

#include "MAX3109.h"
#include "newSPI.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#define maxRetryAttempts 3
#define IRQREADMASK 0x300 // Bitmask for fast read to take the 12 byte message 
#define CLKSourceMASK 0x8C

static uint16_t dummy; // Used as dummy for receiving junk data from SPI - should be local to this file and never used elsewhere.

/* Local Function Prototypes */
static uint8_t MAXreadRegisterValue( const MAX3109_UART_SELECTION channel,
                                     const MAX3109_REGISTER_ADDRESS_VALUE maxRegister );
static uint8_t MAXwriteRegisterValue( const MAX3109_UART_SELECTION channel, /* 0 is ADC , 1 is GPS UART*/
                                      const MAX3109_REGISTER_ADDRESS_VALUE maxRegister,
                                      const uint8_t value /*value to write */ );
static inline bool UARTChannelIsInvalid( const MAX3109_UART_SELECTION channel );

/* Function: Initialize_MAX3109()
 * Description - Performs initial read known register value, writes registers from configuration data, performs readback tests on written registers.
 * Returns 1 on success, 0 on failure. 
 * 3 retry attempts for entire chip, 3 retry attemps for each read back register  */
uint8_t MAXInitializeMAX3109( const uint8_t maxPLLConfiguration,
                           const uint8_t maxClockConfig,
                           const uint8_t uartLineConfig )
{

    /* Perform a Master reset on the chip and reset FIFOs */
    MAXwriteRegisterValue( UART_0, max3109_MODE2, 0x01 );
    MAXwriteRegisterValue( UART_1, max3109_MODE2, 0x01 );
    MAXwriteRegisterValue( UART_0, max3109_MODE2, 0x00 );
    MAXwriteRegisterValue( UART_1, max3109_MODE2, 0x00 );

    // Disable both receive FIFOs during initialization 

    uint8_t readRegisterResult;

    /* Read known register values back at startup. These should be 1 on read */
    uint8_t retryCounterA = maxRetryAttempts;
    while (retryCounterA > 0)
    {
        readRegisterResult = 1;
        readRegisterResult &= MAXreadRegisterValue( UART_0, max3109_DIVLSB );
        readRegisterResult &= MAXreadRegisterValue( UART_0, max3109_PLLConfig );
        readRegisterResult &= MAXreadRegisterValue( UART_1, max3109_DIVLSB );
        readRegisterResult &= MAXreadRegisterValue( UART_1, max3109_PLLConfig );

        if (1 == readRegisterResult)
        {
            break; // Successful readback
        }
        retryCounterA--;
    }

    if (0 == retryCounterA)
    {
        return 0; // Couldn't read back known default values, readback test fails. 
    }

    uint8_t retryCounterB = maxRetryAttempts;
    uint8_t isMAXstartupValid;
    while (retryCounterB > 0)
    {
        isMAXstartupValid = 1;

        /* Generic UART Config - these apply to both UARTs - note: generic can only be written to UART0 */
        MAXwriteRegisterValue( UART_0, max3109CLKSource, maxClockConfig ); /* Setup clock source for both UARTS*/
        MAXwriteRegisterValue( UART_0, max3109_PLLConfig, maxPLLConfiguration ); /* Configure BAUD rates for both UARTS */

        /* UART0 Configuration */
//        MAXwriteRegisterValue( UART_0, max3109_IRQEn, uart0InterruptEnable ); /* Setup interrupt enables */
        MAXwriteRegisterValue( UART_0, max3109_LCR, uartLineConfig ); /* Setup start, stop, and parity configuration */
//        MAXwriteRegisterValue( UART_0, max3109_LSRIntEn, uart0LineStatusInterruptEnable ); /* Setup Line status interrupt enable */
//        MAXwriteRegisterValue( UART_0, max3109_RxTimeOut, uart0RxTimeout ); /* Setup receiver timeout interrupt valie */

        /* UART1 Configuration */
//        MAXwriteRegisterValue( UART_1, max3109_IRQEn, uart1InterruptEnable ); /* Setup interrupt enables */
        MAXwriteRegisterValue( UART_1, max3109_LCR, uartLineConfig ); /* Setup start, stop, and parity configuration */
//        MAXwriteRegisterValue( UART_1, max3109_LSRIntEn, uart1LineStatusInterruptEnable ); /* Setup Line status interrupt enable */
//        MAXwriteRegisterValue( UART_1, max3109_RxTimeOut, uart1RxTimeout ); /* Setup receiver timeout interrupt valie */

        /* Read back all values written and verify they equal the function inputs from configuration */
        isMAXstartupValid &= (maxClockConfig == ((MAXreadRegisterValue( UART_0, max3109CLKSource )) & CLKSourceMASK)); // Why does this read back when I wrote over it? 
        isMAXstartupValid &= (maxPLLConfiguration == MAXreadRegisterValue( UART_0, max3109_PLLConfig ));

//        isMAXstartupValid &= (uart0InterruptEnable == MAXreadRegisterValue( UART_0, max3109_IRQEn ));
        isMAXstartupValid &= (uartLineConfig == MAXreadRegisterValue( UART_0, max3109_LCR ));
//        isMAXstartupValid &= (uart0LineStatusInterruptEnable == MAXreadRegisterValue( UART_0, max3109_LSRIntEn ));
//        isMAXstartupValid &= (uart0RxTimeout == MAXreadRegisterValue( UART_0, max3109_RxTimeOut ));


//        isMAXstartupValid &= (uart1InterruptEnable == MAXreadRegisterValue( UART_1, max3109_IRQEn ));
        isMAXstartupValid &= (uartLineConfig == MAXreadRegisterValue( UART_1, max3109_LCR ));
//        isMAXstartupValid &= (uart1LineStatusInterruptEnable == MAXreadRegisterValue( UART_1, max3109_LSRIntEn ));
//        isMAXstartupValid &= (uart1RxTimeout == MAXreadRegisterValue( UART_1, max3109_RxTimeOut ));
        
        if (1 == isMAXstartupValid)
        {
            break;
        }
        retryCounterB--;
    }
    return isMAXstartupValid;
}

/* If the input UART channel is not from the enum, return true (channel IS invalid) */
static inline bool UARTChannelIsInvalid( const MAX3109_UART_SELECTION channel )
{
    return ( (channel != UART_0) && (channel != UART_1)) ? true : false;
}

/* Writes a specified 8 bit value to the user's channel and register of choice */
static uint8_t MAXwriteRegisterValue( const MAX3109_UART_SELECTION channel,
                                      const MAX3109_REGISTER_ADDRESS_VALUE maxRegister,
                                      const uint8_t value )
{
    if (UARTChannelIsInvalid( channel ))
    {
        return 1; // Error, invalid params 
    }

    uint16_t spiMsg = WRITE_MAX | channel | maxRegister | value;
    SPIreadWriteWord( spiMsg, &dummy );
    return 0;
}

/* Returns a masked 8 bit register value of the user's specified channel and register */
static uint8_t MAXreadRegisterValue( const MAX3109_UART_SELECTION channel,
                                     const MAX3109_REGISTER_ADDRESS_VALUE maxRegister )
{
    if (UARTChannelIsInvalid( channel ))
    {
        return 1; // Error, invalid params - how to make this more obvious of an error? 1 could be a register value, can't tell. 
    }

    uint16_t spiMsg = READ_MAX | channel | maxRegister;
    uint16_t registerValue;
    SPIreadWriteWord( spiMsg, &registerValue );
    uint8_t readValue = registerValue & 0xFF;
    return (uint8_t) readValue; // TODO verify this type cast is required.. 
}


/* Basic FIFO Implementations */

/* Pops a single value from the UART RxFIFO. CAREFUL - popping value while receiving can cause a double. Possibly disable while w/r-ing !!!!! */
uint8_t MAXPopSingleValueFromUARTRxFIFO( const MAX3109_UART_SELECTION channel )
{
    if (UARTChannelIsInvalid( channel ))
    {
        return 1; // Error, invalid params 
    }
    uint8_t dataFromBuffer = MAXreadRegisterValue( channel, max3109_TRxHR );
    return dataFromBuffer;
}

/* Writes a single value to the desired UART TxFIFO*/
uint8_t MAXPushSingleValueToUARTTxFIFO( const MAX3109_UART_SELECTION channel,
                                        const uint8_t valueToWrite )
{
    if (UARTChannelIsInvalid( channel ))
    {
        return 1; // Error, invalid params 
    }
    return MAXwriteRegisterValue( channel, max3109_TRxHR, valueToWrite ); // matches writeRegVal return val: 1 is failure, 0 is success
}

/* Returns the FIFO fill level of the desired transmit or receive buffer, at the desired UART port */

/* Returning 0xFF on error ensures the return val is not mistaken for a valid fill level - fill level max value is 128  */
uint8_t MAXGetUARTFIFOLevel( const MAX3109_UART_SELECTION channel,
                             const MAX3109_REGISTER_ADDRESS_VALUE fifoBuffer )
{
    if (((fifoBuffer != (max3109_TxFIFOLvl) &&
            fifoBuffer != (max3109_RxFIFOLvl))) ||
            (UARTChannelIsInvalid( channel )))
    {
        return 0xFF; // Invalid desired register address - should only be fifo fill levels
    }

    uint8_t fifoLevel = MAXreadRegisterValue( channel, fifoBuffer );
    fifoLevel = (fifoLevel > MAXIMUM_MAX3109_FIFO_SIZE_IN_BYTES) ? 0xFF : fifoLevel;
    return fifoLevel;
}

/* Reads the receiver timeout status and fifo empty flag. The user can read the receive
 buffer if BOTH a receiver timeout has occurred and there is data in the UART. This condition,
 based on our available knowledge, will avoid the problem of reading out data while receiving 
 data. Can eventually implement this with interrupts */
bool MAXIsUARTReceiveReadyToRead( const MAX3109_UART_SELECTION channel )
{
    if (UARTChannelIsInvalid( channel ))
    {
        return false;
    }

    /* Valid to read if BOTH the receive FIFO is NOT empty and the rx Timeout is True */
    bool DoesRxFIFOHaveData = (0x40 == (MAXreadRegisterValue( channel, max3109_ISR_Status ) & 0x40)) ? true : false ; // This is cleared every time it's read. If you receive a constrant stream of data, 
                                                                                                                        // zeroes will be read consistently after that 
    bool IsRxTimeout = (0x01 == (MAXreadRegisterValue( channel, max3109_LSR ) & 0x01)) ? true : false;

    return ( (true == DoesRxFIFOHaveData) && (true == IsRxTimeout));
}