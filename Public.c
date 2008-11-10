#include "sd.h"
#include "public.h"
#include "..\Z-OS\Z-OS.h"

Int16 SDDeviceId;
Int8 SectorBuffer[512]; // A sector buffer
UInt64 SectorBufferNum = ~(UInt64)0;
Bool BufferDirty = False;

Int16 SDRead(Int16 id, UInt64 pos, UInt8* buffer, UInt16 bufferLen)
{
	if (id == SDDeviceId)
	{
		puts("SDRead called...\r\n");
		
		// Divide by 512 (the number of bytes in a sector)
		UInt64 sectorBegin = pos >> 9;
		UInt16 i = sectorBegin;
		pos -= sectorBegin << 9; // pos is now the pos into the first sector needed
		
		while (1)
		{
			UInt16 bytesToCopy;
			if (i != SectorBufferNum)
			{
				if (BufferDirty)
				{
					// Write back the sector
					if (SD_Sector_Write((unsigned char*)SectorBuffer,SectorBufferNum)) return ErrorWriting;
					BufferDirty = False;
				}
				if (SD_Sector_Read((unsigned char*)SectorBuffer,i)) return ErrorReading;
				SectorBufferNum = i;
				BufferDirty = False;
			}
			bytesToCopy = Min(bufferLen,512 - pos);
			memcpy(buffer,SectorBuffer + pos,bytesToCopy);
			bufferLen -= bytesToCopy;
			// Have we finished?
			if (bufferLen == 0) break;
			// Move to the next buffer
			i++;
			buffer += bytesToCopy;
			pos = 0;
		}
		puts("SDRead finished.\r\n");
		return ErrorSuccess;
	}
	return ErrorUnknownDevice;
}

Int16 SDWrite(Int16 id, UInt64 pos, UInt8* buffer, UInt16 bufferLen)
{
	if (id == SDDeviceId)
	{
		// Divide by 512 (the number of bytes in a sector)
		UInt64 sectorBegin = pos >> 9;
		UInt16 i = sectorBegin;
		pos -= sectorBegin << 9; // pos is now the pos into the first sector needed
		
		while (1)
		{
			UInt16 bytesToCopy = Min(bufferLen,512 - pos);
			if (i != SectorBufferNum)
			{
				if (BufferDirty)
				{
					// Write back the sector
					if (SD_Sector_Write((unsigned char*)SectorBuffer,SectorBufferNum)) return ErrorWriting;
					BufferDirty = False;
				}
				// If the user is writing a whole sector, don't worry 
				// about reading in the current version of the sector.
				if (bytesToCopy != 512 || pos != 0)
				{
					if (SD_Sector_Read((unsigned char*)SectorBuffer,i)) return ErrorReading;
				}
				SectorBufferNum = i;
				BufferDirty = False;
			}
			memcpy(SectorBuffer + pos,buffer,bytesToCopy);
			bufferLen -= bytesToCopy;
			BufferDirty = True;
			// Have we finished?
			if (bufferLen == 0) break;
			// Move to the next buffer
			i++;
			buffer += bytesToCopy;
			pos = 0;
		}
		return ErrorSuccess;
	}
	return ErrorUnknownDevice;
}

Int16 SDCommand(Int16 id, Int16 cmd, UInt8* buffer, UInt16 bufferLen)
{
	return ErrorUnimplemented;
}

Int16 SDGetAvailableBytes(Int16 id, UInt64* bytes)
{
	if (SD_GetTotalSize(bytes)) return ErrorUnknown;
	return ErrorSuccess;
}

Int16 SDFlush(Int16 id)
{
	if (BufferDirty)
	{
		// Write back the sector
		if (SD_Sector_Write((unsigned char*)SectorBuffer,SectorBufferNum)) return ErrorWriting;
		BufferDirty = False;
	}
	return ErrorSuccess;
}

void InitSD(void)
{
	DeviceInfo info = {0};
	DeviceFuncs funcs = {0};
	
	info.CanRead = True;
	info.CanWrite = True;
	info.CanSeek = True;
	info.CanMount = True;
	info.IsSectored = True;
	info.SectorSize = 512;
	
	funcs.Read = SDRead;
	funcs.Write = SDWrite;
	funcs.Command = SDCommand;
	funcs.GetAvailableBytes = SDGetAvailableBytes;
	funcs.Flush = SDFlush;
	
	RegisterDevice("SD", info, funcs, &SDDeviceId);
}
