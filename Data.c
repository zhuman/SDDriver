#include "..\Z-OS\Z-OS.h"
#include "sd.h"

extern const unsigned int SD_CRC16_Table[];

// SD_Data_Read -- performs a data read of len bytes into buf
int SD_Data_Read(unsigned char *buf, unsigned len)
{
	unsigned timeout, i, crc = 0;
	unsigned char c;
	
	timeout = SD_CMD_TIMEOUT;
	
	// wait for data start token
	do {
		c = SD_Bus_ReadByte();
		if(c < 0x10) return ERR_DATA_FAILED;  // data error token received
		timeout--;
	} while(timeout > 0 && c != 0xFE);
	
	if(c != 0xFE) return ERR_NO_COMM;
	
	len--;
	// kick off the process by getting the first byte
	c = SD_Bus_ReadByte();
	
	// for the heart of this function, we won't be using SPI_* or the CRC16 function.
	// instead, we'll overlap SPI transactions and CRC accumulation to minimize wasted processor time.
	for(i=0;i<len;i++)
	{
		SD_SPIBUF = 0xFF;  // start next transaction
		crc = (crc << 8) ^ SD_CRC16_Table[(crc >> 8) ^ c];  // calculate crc from last byte
		buf[i] = c;
		while(!SD_SPISTATbits.SPIRBF);  // wait for this transaction to finish up, if necessary
		c = SD_SPIBUF;
	}
	
	// process the last byte
	crc = (crc << 8) ^ SD_CRC16_Table[(crc >> 8) ^ c];  // calculate crc from last byte
	buf[i] = c;
	
	// read CRC from card
	i = SD_Bus_ReadByte() << 8;  // MSB first
	i |= SD_Bus_ReadByte();  // LSB second
	
	if(i == crc)
		return ERR_SUCCESS;
	
	return ERR_BAD_CRC;
}

// SD_Data_Write -- performs a data write of len bytes from buf
int SD_Data_Write(unsigned char *buf, unsigned len)
{
	unsigned timeout, i, crc = 0;
	unsigned char c;
	
	timeout = SD_CMD_TIMEOUT;
	
	SD_Bus_WriteByte(0xFE);  // send start block token
	
	// for the heart of this function, we won't be using SPI_* or the CRC16 function.
	// instead, we'll overlap SPI transactions and CRC accumulation to minimize wasted processor time.
	for(i=0;i<len;i++)
	{
		c = buf[i];
		SD_SPIBUF = c;  // send byte
		crc = (crc << 8) ^ SD_CRC16_Table[(crc >> 8) ^ c];  // calculate crc from byte
		while(!SD_SPISTATbits.SPIRBF);  // wait for this transaction to finish up, if necessary
		c = SD_SPIBUF;  // dummy read
	}
	
	// write out CRC
	SD_Bus_WriteByte(crc >> 8);  // MSB first
	SD_Bus_WriteByte(crc & 0xFF);  // LSB second
	
	// wait for data response token
	do {
		c = SD_Bus_ReadByte();
		timeout--;
	} while(timeout > 0 && (c & 0x11) != 1);
	
	switch(c & 0x1F)
	{
		case 0x5:
			return ERR_SUCCESS;
		case 0xB:
			return ERR_BAD_CRC;
		case 0xD:
			return ERR_DATA_FAILED;
		default:
			return ERR_NO_COMM;
	}
}
