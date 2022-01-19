/* File: newSPI
 * Author: Henry Gilbert
 * Description : Basic function calls for using SPI port.
 */

#include "newSPI.h"
#include "pic_h/p30F6014A.h"

/* Local Function Prototypes */
static inline void csLow( );
static inline void csHigh( );

/* Initializes the SPI port based on configuration mode and status config - sets chip select high */
void InitializeSPI( uint16_t uModeConfig,
                    uint16_t SPI1StatusConfig )
{
    /* Chip Select pins configured as outputs */
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB4 = 0;

    /* Clear the Interrupt flag, setup interrupt enable, set interrupt priority */
    IFS0bits.SPI1IF = 0;
    IPC2bits.SPI1IP = 0;
    IEC0bits.SPI1IE = 0;
    //    SPI1STATbits.SPIROV = 0;

    SPI1CON = uModeConfig;
    SPI1STAT = SPI1StatusConfig;
    csHigh( );
}

/* Reads and writes a single 16 bit word to MOSI.  */
uint8_t SPIreadWriteWord( uint16_t writeData,
                          uint16_t * readData )
{
    csLow( );
    SPI1BUF = writeData;
    while (!SPI1STATbits.SPIRBF);
    *readData = SPI1BUF;
    csHigh( );
    return 0;
}

/* These are specific to the AFC004 project - must be updated per implementation */
static inline void csLow( )
{
    LATBbits.LATB4 = 0;
    LATBbits.LATB2 = 0;
}

static inline void csHigh( )
{
    LATBbits.LATB4 = 1;
    LATBbits.LATB2 = 1;
}