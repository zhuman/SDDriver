#include <p24Hxxxx.h>
#include "sd.h"

//#include <stdio.h>

// SD_SendCommand -- sends a command to the SD card and reads the response
//   cmd - SD command to send
//	 response - pointer to the response struct to fill
//	 responsetype - indicates the type of response to be received
void SD_SendCommand(SD_Cmd cmd, void *response, int responsetype)
{
	unsigned len, timeout;
	unsigned char *p;
	
	// fill in SD start/end fields
	cmd.Cmd.StartField = 1;
	cmd.Cmd.EndField = 1;
	
	// fill in CRC field
	cmd.Cmd.CRC = SD_CRC7_Calculate(&cmd);
	
//	printf("[SD] Write cmd (cmd: 0x%.2X data: 0x%.2X 0x%.2X 0x%.2X 0x%.2X crc: 0x%.2X)\r\n", cmd.Cmd.Command, cmd.Cmd.Arg.c0, cmd.Cmd.Arg.c1, cmd.Cmd.Arg.c2, cmd.Cmd.Arg.c3, cmd.Cmd.CRC);
	
	// select chip
	SD_CS_Assert();

	do {
		p = response;
		switch(responsetype)
		{
			case R1:
			case R1b:
			case R1m:
				len = sizeof (SD_R1);
				break;
			case R2:
				len = sizeof (SD_R2);
				break;
			case R3:
				len = sizeof (SD_R3);
				break;
			case R7:
				len = sizeof (SD_R7);
				break;
		}

		// write command
		SD_Bus_WriteByte(cmd.Byte[0]);
		SD_Bus_WriteByte(cmd.Byte[4]);	// little-endian to big-endian conversion
		SD_Bus_WriteByte(cmd.Byte[3]);
		SD_Bus_WriteByte(cmd.Byte[2]);
		SD_Bus_WriteByte(cmd.Byte[1]);
		SD_Bus_WriteByte(cmd.Byte[5]);
	
		// read response
		timeout = SD_CMD_TIMEOUT;
		do {
			*p = SD_Bus_ReadByte();
			timeout--;
		} while((timeout > 0) && ((*p) & 0x80));
		for(len--;len > 0;len--)		// read remaining bytes
			*(++p) = SD_Bus_ReadByte();

// this check is actually unnecessary because we check for a busy signal at the beginning of every top-level function
// this code is only here for example documentation
//		if(responsetype == R1b)
//		{
//			// look for a busy signal and wait for it to pass
//			while(SD_Bus_ReadByte() != 0xFF);
//		}

#ifdef SD_STATS
		if((*((unsigned char *) response) & 0x88) == 0x8)
			// log CRC error
			SD_Stats.com_crc++;
#endif

	} while((*((unsigned char *) response) & 0x88) == 0x8);  // is valid response && bad CRC, repeat transmission
	
	if(responsetype != R1m)
	{
		// deselect chip
		SD_CS_Deassert();
	
		// SD spec requires 8 clock cycles after finishing any command
		SD_Bus_DummyClock(1);
	}
	
//	printf("[SD] Read response (R1: 0x%.2X)\r\n", *((unsigned char *) response));
}
