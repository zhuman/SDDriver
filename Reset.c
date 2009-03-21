#include <p24Hxxxx.h>
#include "sd.h"

// card state information -- stores information about the currently inserted card
struct SD_Card_State_t SD_Card_State;
// SD stats information
#ifdef SD_STATS
  struct SD_Stats_t SD_Stats;
#endif

// SD_Reset -- resets a newly inserted SD card
int SD_Reset(void)
{
	SD_Cmd c;
	SD_R1 r;
	SD_R3 r3;
	SD_R7 r7;
	
	SD_Invalidate_State();
	
	if(SD_PIN_CD == CARD_REMOVED) /* check for inserted card */
		return ERR_NO_CARD;
		
	SD_Bus_Init();

	// Power-on delay (100ms)
//	__delay32(100000L * 40);
	
//	SD_CS_Assert();			// assert CS during the 74 clock cycles required to power the card up
	SD_Bus_DummyClock(10);	// give the SD card at least 74 clock cycles to start up
//	SD_CS_Deassert();		// deassert CS during 16 additional clock cycles
//	SD_Bus_DummyClock(2);	// 16 more clock cycles
	
	// send CMD0 -- reset card
	c.Cmd.Command = 0;
	c.Cmd.Arg.l = 0;
	SD_SendCommand(c, &r, R1);
	if(r.Byte[0] != 0x1)  // last bit (idle state) should be the only error bit set after CMD0
		return ERR_RESET_FAILED;

	// send CMD59 -- CRC on
	c.Cmd.Command = 59;
	c.Cmd.Arg.c3 = 1;
	SD_SendCommand(c, &r, R1);
	
	do {
		// send CMD8 -- send operating conditions
		c.Cmd.Command = 8;
		c.Cmd.Arg.c2 = 1;		// VHS - 0x1 = 2.7-3.6v
		c.Cmd.Arg.c3 = 0xAA;	// check pattern - echoed back
		SD_SendCommand(c, &r7, R7);
		if(!IsValidResponse(r7)) return ERR_RESET_FAILED;
		if(r7.Response.IllegalCmd) break;
	} while(r7.Response.CheckPattern != 0xAA);
	
	c.Cmd.Arg.l = 0;
	
	if(!r7.Response.IllegalCmd)
	{
		// spec version 2.0
		do
		{
			// send CMD55 -- APP_CMD, precedes all ACMDs
			c.Cmd.Command = 55;
			SD_SendCommand(c, &r, R1);
			if(!IsValidResponse(r)) return ERR_RESET_FAILED;
			
			// send ACMD41 -- initialize card
			c.Cmd.Command = 41;
			c.Cmd.Arg.c0 = 0x40;  // host HCS is true
			SD_SendCommand(c, &r, R1);
			if(!IsValidResponse(r)) return ERR_RESET_FAILED;
		} while(r.Response.IdleState);  // repeat ACMD41 until card is ready
		
		// send CMD58 -- read OCR register
		c.Cmd.Command = 58;
		SD_SendCommand(c, &r3, R3);
		if(!IsValidResponse(r)) return ERR_RESET_FAILED;
		if(r3.Response.OCR.c[0] & 0x40)
			// high capacity card
			SD_Card_State.high_capacity = 1;
		else
			SD_Card_State.high_capacity = 0;
	}
	else
	{
		// spec version 1.x
		do
		{
			// send APP_CMD, precedes all ACMDs
			c.Cmd.Command = 55;
			SD_SendCommand(c, &r, R1);
			if(!IsValidResponse(r)) return ERR_RESET_FAILED;
			
			// send ACMD41 -- initialize card
			c.Cmd.Command = 41;
			SD_SendCommand(c, &r, R1);
			if(!IsValidResponse(r)) return ERR_RESET_FAILED;
		} while(r.Response.IdleState);  // repeat ACMD41 until card is ready
		
		SD_Card_State.high_capacity = 0;
	}
	
	// send CMD16 -- set block length to 512 (this is the default anyway)
//	c.Cmd.Command = 16;
//	c.Cmd.Arg.l = 512;
//	SD_SendCommand(c, &r, R1);
//	if(!IsValidResponse(r)) return ERR_RESET_FAILED;

	// set SPI clock to something a little faster
	SD_SPISTATbits.SPIEN = 0;  // disable module
	SD_SPICON1 = (SD_SPICON1 & ~0x1F) | SD_SPIDIV_NORMAL;
	SD_SPISTATbits.SPIEN = 1;  // enable module

	SD_Card_State.valid = 1;
	
	return ERR_SUCCESS;
}
