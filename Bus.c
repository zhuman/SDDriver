#include <p24Hxxxx.h>
#include "sd.h"

//#include <stdio.h>

// SD_Bus_WriteByte -- write a single byte to the SD card
void SD_Bus_WriteByte(unsigned char c)
{
	unsigned char dummy;
	
//	printf("[SPI] Write byte: 0x%.2X\r\n", c);
	
	// write byte
	SD_SPIBUF = c;
	
	// wait for transmit to finish
	while(!SD_SPISTATbits.SPIRBF);
	
	// dummy read to prevent overflow
	dummy = SD_SPIBUF;
}

// SD_Bus_ReadByte -- read a single byte from the SD card
unsigned char SD_Bus_ReadByte()
{
	unsigned char c;
	
	// dummy write
	SD_SPIBUF = 0xFF;
	
	// wait for transmit to finish
	while(!SD_SPISTATbits.SPIRBF);
	
	// read byte
	c = SD_SPIBUF;
	
//	printf("[SPI] Read byte: 0x%.2X\r\n", c);
	
	return c;
}

// SD_Bus_DummyClock -- provides dummy clock pulses to the SD card by transmitting meaningless data
//   n - number of clocks to transmit / 8 (clocks transmitted = n * 8)
void SD_Bus_DummyClock(unsigned int n)
{
	unsigned i;
	for(i=0;i<n;i++)
		SD_Bus_WriteByte(0xFF);
}

// SD_Bus_Init -- initialize SPI bus and SD pins
void SD_Bus_Init(void)
{	
	// *** SPI initialization ***
	// Reset SPI hardware.
	SD_SPISTAT = 0;
	SD_SPICON1 = 0;
	SD_SPICON2 = 0;
	
	// Master mode, idle level is high, all other settings are default.
	SD_SPICON1 = 0x60 | SD_SPIDIV_INITIALIZE;
	
	SD_CS_Deassert();  // initialize CS deasserted
	SD_PIN_CS_T = 0;  // CS output
	SD_SPISTATbits.SPIEN = 1;  // enable module
	
#ifdef SD_PIN_CD_PU
	SD_PIN_CD_PU = 1;	// enable CD pull-up
#endif
#ifdef SD_PIN_WP_PU
	SD_PIN_WP_PU = 1;	// enable write-protect pull-up
#endif
#ifdef SD_PIN_CD_INT
	SD_PIN_CD_INT = 1;  // select pin for CN interrupt
#endif
#ifdef SD_CD_INTERRUPT_HANDLER
	IFS1bits.CNIF = 0;
	IEC1bits.CNIE = 1;	// enable CN interrupt
#endif
}

#ifdef SD_CD_INTERRUPT_HANDLER
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void)
{
	SD_CD_Interrupt_Hook();
	IFS1bits.CNIF = 0;
}
#endif
