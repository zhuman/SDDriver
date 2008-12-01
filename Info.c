#include "..\Z-OS\Z-OS.h"
#include "SD.h"

int SD_GetTotalSize(UInt64* size)
{
	SD_Cmd cmd;
	SD_R1 r;
	SD_CSD csd;
	int ret = 0;
	int i;
	
	SD_SanityCheck();
	
	// CMD9 - Send card specific data (CSD)
	cmd.Cmd.Command = 9;
	cmd.Cmd.Arg.l = 0;
	SD_SendCommand(cmd,&r,R1m);
	
	if(r.Byte[0] != 0)
	{
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
	
	// If no errors occurred, read in the data
	ret = SD_Data_Read((unsigned char*)&csd, 16);
	
	// deselect chip
	SD_CS_Deassert();
	// SD spec requires 8 clock cycles after finishing any command
	SD_Bus_DummyClock(1);
			
	if(ret == ERR_NO_COMM)
		SD_Invalidate_State();
		
	if (ret != 0) return ret;
	
	for (i = 0; i < sizeof(csd); i++)
	{
		printf("%x ",((unsigned char*)&csd)[i]);
	}
	puts("\r\n");
	
	if (csd.V1.CSD_STRUCTURE == 0)
	{
		UInt64 C_SIZE_MULT = (((UInt64)csd.Bytes[9] & 0b11) << 1) & ((UInt64)csd.Bytes[10] >> 7);
		UInt64 C_SIZE = (((UInt64)csd.Bytes[6] & 0b11) << 10) & ((UInt64)csd.Bytes[7] << 2) & ((UInt64)csd.Bytes[8] >> 6);
		UInt64 READ_BL_LEN = csd.Bytes[5] & 0b1111;
		
		UInt64 mult = 1 << (C_SIZE_MULT + 2);
		UInt64 blocknr = (C_SIZE + 1) * mult;
		UInt64 blocklen = 1 << READ_BL_LEN;
		
		puts("Reading CSD structure version 0\r\n");
		printf("Block length: %llu\r\n",blocklen);
		*size = blocknr * blocklen;
		return ERR_SUCCESS;
	}
	else if (csd.V1.CSD_STRUCTURE == 1)
	{
		puts("Reading CSD structure version 1 (high-capacity)\r\n");
		*size = ((UInt64)csd.V2.C_SIZE + (UInt64)1) * ((UInt64)512 * (UInt64)1024);
		return ERR_SUCCESS;
	}
	
	return ERR_SUCCESS;
}
