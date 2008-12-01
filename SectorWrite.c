#include "..\Z-OS\Z-OS.h"
#include "sd.h"

// SD_Sector_Write -- writes buf to the sector sector_num
int SD_Sector_Write(unsigned char *buf, unsigned long sector_num)
{
	SD_Cmd c;
	SD_R1 r;
	int ret;
	
	SD_SanityCheck();
	
#ifdef SD_HONOR_WP
	if(SD_PIN_WP == WP_PROTECTED)
		return ERR_WRITE_PROTECTED;
#endif

	do
	{
		// CMD24 -- write single block
		c.Cmd.Command = 24;
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
		
		ret = SD_Data_Write(buf, 512);
		
#ifdef SD_STATS
		if(ret == ERR_BAD_CRC)
			// log crc error
			SD_Stats.dw_crc++;
#endif

	} while(ret == ERR_BAD_CRC);
	
	// deselect chip
	SD_CS_Deassert();
	// SD spec requires 8 clock cycles after finishing any command
	SD_Bus_DummyClock(1);
			
	if(ret == ERR_NO_COMM)
		SD_Invalidate_State();
		
	return ret;
}
