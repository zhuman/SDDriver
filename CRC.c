#include "..\Z-OS\Z-OS.h"
#include "sd.h"

/* SD CRC16 table implementation     *
 * Polynomial: x^16 + x^12 + x^5 + 1 */
const unsigned int SD_CRC16_Table[] = 
{
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF,
	0x1231,0x0210,0x3273,0x2252,0x52B5,0x4294,0x72F7,0x62D6,0x9339,0x8318,0xB37B,0xA35A,0xD3BD,0xC39C,0xF3FF,0xE3DE,
	0x2462,0x3443,0x0420,0x1401,0x64E6,0x74C7,0x44A4,0x5485,0xA56A,0xB54B,0x8528,0x9509,0xE5EE,0xF5CF,0xC5AC,0xD58D,
	0x3653,0x2672,0x1611,0x0630,0x76D7,0x66F6,0x5695,0x46B4,0xB75B,0xA77A,0x9719,0x8738,0xF7DF,0xE7FE,0xD79D,0xC7BC,
	0x48C4,0x58E5,0x6886,0x78A7,0x0840,0x1861,0x2802,0x3823,0xC9CC,0xD9ED,0xE98E,0xF9AF,0x8948,0x9969,0xA90A,0xB92B,
	0x5AF5,0x4AD4,0x7AB7,0x6A96,0x1A71,0x0A50,0x3A33,0x2A12,0xDBFD,0xCBDC,0xFBBF,0xEB9E,0x9B79,0x8B58,0xBB3B,0xAB1A,
	0x6CA6,0x7C87,0x4CE4,0x5CC5,0x2C22,0x3C03,0x0C60,0x1C41,0xEDAE,0xFD8F,0xCDEC,0xDDCD,0xAD2A,0xBD0B,0x8D68,0x9D49,
	0x7E97,0x6EB6,0x5ED5,0x4EF4,0x3E13,0x2E32,0x1E51,0x0E70,0xFF9F,0xEFBE,0xDFDD,0xCFFC,0xBF1B,0xAF3A,0x9F59,0x8F78,
	0x9188,0x81A9,0xB1CA,0xA1EB,0xD10C,0xC12D,0xF14E,0xE16F,0x1080,0x00A1,0x30C2,0x20E3,0x5004,0x4025,0x7046,0x6067,
	0x83B9,0x9398,0xA3FB,0xB3DA,0xC33D,0xD31C,0xE37F,0xF35E,0x02B1,0x1290,0x22F3,0x32D2,0x4235,0x5214,0x6277,0x7256,
	0xB5EA,0xA5CB,0x95A8,0x8589,0xF56E,0xE54F,0xD52C,0xC50D,0x34E2,0x24C3,0x14A0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xA7DB,0xB7FA,0x8799,0x97B8,0xE75F,0xF77E,0xC71D,0xD73C,0x26D3,0x36F2,0x0691,0x16B0,0x6657,0x7676,0x4615,0x5634,
	0xD94C,0xC96D,0xF90E,0xE92F,0x99C8,0x89E9,0xB98A,0xA9AB,0x5844,0x4865,0x7806,0x6827,0x18C0,0x08E1,0x3882,0x28A3,
	0xCB7D,0xDB5C,0xEB3F,0xFB1E,0x8BF9,0x9BD8,0xABBB,0xBB9A,0x4A75,0x5A54,0x6A37,0x7A16,0x0AF1,0x1AD0,0x2AB3,0x3A92,
	0xFD2E,0xED0F,0xDD6C,0xCD4D,0xBDAA,0xAD8B,0x9DE8,0x8DC9,0x7C26,0x6C07,0x5C64,0x4C45,0x3CA2,0x2C83,0x1CE0,0x0CC1,
	0xEF1F,0xFF3E,0xCF5D,0xDF7C,0xAF9B,0xBFBA,0x8FD9,0x9FF8,0x6E17,0x7E36,0x4E55,0x5E74,0x2E93,0x3EB2,0x0ED1,0x1EF0
};

// SD_CRC16_Calculate -- calculates the CRC for the given data block
unsigned int SD_CRC16_Calculate(unsigned char *buf, unsigned len)
{
	unsigned int i, crc = 0;
	for(i=0;i<len;i++)
		crc = (crc << 8) ^ SD_CRC16_Table[(crc >> 8) ^ buf[i]];
	return crc;
}


/* SD CRC7 table implementation *
 * Polynomial: x^7 + x^3 + 1    */
 
/* CRC7 table implementation and table generator courtesy of
   http://www.pololu.com/docs/0J1?section=5.f */
const unsigned char SD_CRC7_Table[] =
{
  0,  9, 18, 27, 36, 45, 54, 63, 72, 65, 90, 83,108,101,126,119,
 25, 16, 11,  2, 61, 52, 47, 38, 81, 88, 67, 74,117,124,103,110,
 50, 59, 32, 41, 22, 31,  4, 13,122,115,104, 97, 94, 87, 76, 69,
 43, 34, 57, 48, 15,  6, 29, 20, 99,106,113,120, 71, 78, 85, 92,
100,109,118,127, 64, 73, 82, 91, 44, 37, 62, 55,  8,  1, 26, 19,
125,116,111,102, 89, 80, 75, 66, 53, 60, 39, 46, 17, 24,  3, 10,
 86, 95, 68, 77,114,123, 96,105, 30, 23, 12,  5, 58, 51, 40, 33,
 79, 70, 93, 84,107, 98,121,112,  7, 14, 21, 28, 35, 42, 49, 56,
 65, 72, 83, 90,101,108,119,126,  9,  0, 27, 18, 45, 36, 63, 54,
 88, 81, 74, 67,124,117,110,103, 16, 25,  2, 11, 52, 61, 38, 47,
115,122, 97,104, 87, 94, 69, 76, 59, 50, 41, 32, 31, 22, 13,  4,
106, 99,120,113, 78, 71, 92, 85, 34, 43, 48, 57,  6, 15, 20, 29,
 37, 44, 55, 62,  1,  8, 19, 26,109,100,127,118, 73, 64, 91, 82,
 60, 53, 46, 39, 24, 17, 10,  3,116,125,102,111, 80, 89, 66, 75,
 23, 30,  5, 12, 51, 58, 33, 40, 95, 86, 77, 68,123,114,105, 96,
 14,  7, 28, 21, 42, 35, 56, 49, 70, 79, 84, 93, 98,107,112,121
};

// SD_CRC7_Calculate -- calculates the CRC for the SD command cmd
unsigned char SD_CRC7_Calculate(SD_Cmd *cmd)
{
    unsigned char CRC;

	CRC = SD_CRC7_Table[cmd->Byte[0]];
	CRC = SD_CRC7_Table[(CRC << 1) ^ cmd->Byte[4]];  // endian reversal
	CRC = SD_CRC7_Table[(CRC << 1) ^ cmd->Byte[3]];
	CRC = SD_CRC7_Table[(CRC << 1) ^ cmd->Byte[2]];
	CRC = SD_CRC7_Table[(CRC << 1) ^ cmd->Byte[1]];

    return CRC;
}


/* Old CRC7 implementation below */
/* CRC7 lookup table, this was generated using the following code fragment:

  int i;
  printf("{\n");
  for(i=0;i<128;i++)
  {
    int di = i << 7;
    int dv = 0x89 << 6;
    int pv = 13;
    for(;pv>=7;di^=(di&(1<<pv)?dv:0),dv>>=1,pv--);
    printf("%3d,",di);
  }
  printf("\n}");
*/

//const unsigned char SD_CRC7_Table[] =
//{
//  0,  9, 18, 27, 36, 45, 54, 63, 72, 65, 90, 83,108,101,126,119,
// 25, 16, 11,  2, 61, 52, 47, 38, 81, 88, 67, 74,117,124,103,110,
// 50, 59, 32, 41, 22, 31,  4, 13,122,115,104, 97, 94, 87, 76, 69,
// 43, 34, 57, 48, 15,  6, 29, 20, 99,106,113,120, 71, 78, 85, 92,
//100,109,118,127, 64, 73, 82, 91, 44, 37, 62, 55,  8,  1, 26, 19,
//125,116,111,102, 89, 80, 75, 66, 53, 60, 39, 46, 17, 24,  3, 10,
// 86, 95, 68, 77,114,123, 96,105, 30, 23, 12,  5, 58, 51, 40, 33,
// 79, 70, 93, 84,107, 98,121,112,  7, 14, 21, 28, 35, 42, 49, 56
//};
//
// SD_CRC7_Calculate -- calculates the CRC for the SD command cmd
//unsigned char SD_CRC7_Calculate(SD_Cmd *cmd)
//{
//	// 40 bits are protected by this CRC, we look up patterns of 7 bits at a time
//	unsigned char crc;
//	register unsigned char b1  asm ("w8") = cmd->Byte[0];
//	register unsigned char b2  asm ("w9") = cmd->Byte[1];
//	register unsigned char b3 asm ("w10") = cmd->Byte[2];
//	register unsigned char b4 asm ("w11") = cmd->Byte[3];
//	register unsigned char b5 asm ("w12") = cmd->Byte[4];
//	
//	unsigned char piece = (b1 & 0xF8) >> 3;  // effectively pad the bit stream with 2 leading zeroes; first piece is the first 5 bits right-justified
//	crc = SD_CRC7_Table[piece];  // crc first piece
//	
//	piece = ((b1 & 0x7) << 4) | ((b2 & 0xF0) >> 4);  // next piece is the last 3 bits of the first byte and the first 4 bits of the second byte
//	crc = SD_CRC7_Table[piece ^ crc];  // crc second piece
//	
//	piece = ((b2 & 0xF) << 3) | ((b3 & 0xE0) >> 5);  // next piece is the last 4 bits of the second byte and the first 3 bits of the third byte
//	crc = SD_CRC7_Table[piece ^ crc];  // crc third piece
//	
//	piece = ((b3 & 0x1F) << 2) | ((b4 & 0xC0) >> 6);  // next piece is the last 5 bits of the third byte and the first 2 bits of the fourth byte
//	crc = SD_CRC7_Table[piece ^ crc];  // crc fourth piece
//	
//	piece = ((b4 & 0x3F) << 1) | ((b5 & 0x80) >> 7);  // next piece is the last 6 bits of the fourth byte and the first 1 bit of the final byte
//	crc = SD_CRC7_Table[piece ^ crc];  // crc fifth piece
//	
//	piece = b5 & 0x7F;  // last piece is the last 7 bits of the final byte
//	crc = SD_CRC7_Table[piece ^ crc];  // crc last piece
//	
//	return crc;
//}
