#include <p24Hxxxx.h>
#include "sd.h"
#include <stdio.h>

// SD_Sector_Read -- reads the sector sector_num from the card into buf
int SD_Sector_Read(unsigned char *buf, unsigned long sector_num)
{
	SD_Cmd c;
	SD_R1 r;
	int ret;
	
	puts("SD_Sector_Read called.\r\n");
	
	SD_SanityCheck();
	
	do
	{
		// CMD17 -- read single block
		c.Cmd.Command = 17;
		if(SD_Card_State.high_capacity)
			c.Cmd.Arg.l = sector_num;  // high capacity cards receive a sector address
		else
			c.Cmd.Arg.l = sector_num << 9;  // standard cards receive a byte address aligned to a sector boundary
		SD_SendCommand(c, &r, R1m);
		
		if(r.Byte[0] != 0) {
			// deselect chip
			SD_CS_Deassert();
			// SD spec requires 8 clock cycles after finishing any command
			SD_Bus_DummyClock(1);
			if(!IsValidResponse(r)) {
				SD_Invalidate_State();
				return ERR_NO_COMM;
			}
			return ERR_DATA_FAILED;
		}
		
		ret = SD_Data_Read(buf, 512);
		
#ifdef SD_STATS
		if(ret == ERR_BAD_CRC)
			// log crc error
			SD_Stats.dr_crc++;
#endif

	} while(ret == ERR_BAD_CRC);
	
	// deselect chip
	SD_CS_Deassert();
	// SD spec requires 8 clock cycles after finishing any command
	SD_Bus_DummyClock(1);
			
	if(ret == ERR_NO_COMM)
		SD_Invalidate_State();
	
	puts("SD_Sector_Read finished.\r\n");
	return ret;
}
