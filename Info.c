#include "SD.h"
#include "..\Z-OS\Z-OS.h"

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
		UInt64 mult = (UInt64)1 << (UInt64)(csd.V1.C_SIZE_MULT + 2);
		UInt64 blocklen = (UInt64)1 << (UInt64)(csd.V1.READ_BL_LEN);
		UInt64 blocknr = (UInt64)((UInt64)csd.V1.C_SIZE + (UInt64)1) * mult;
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
