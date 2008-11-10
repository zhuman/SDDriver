#ifndef _SD_H
#define _SD_H

/*** Configuration ***/
// peripheral selection
#define SD_SPI_MODULE		2	// 1 or 2
//  other pin definitions are selected with the SPI module
#define SD_PIN_CD			PORTBbits.RB5		// this is required because without it, you absolutely need a pull-up on SDI, and that's silly
#define SD_PIN_CD_INT		CNEN1bits.CN7IE		// this may be commented out if you use a non-CN CD pin
#define SD_PIN_CD_PU		CNPU1bits.CN7PUE	// this may be commented out if you use a non-CN CD pin
												// NOTE: if you don't use a CN pin for CD, you should use one of the external interrupts
												//		 instead, to allow the SD library to automatically reset an ejected card properly.
												//		 if you do use another CD source, use SD_Invalidate_State() when the card is ejected.
#define SD_PIN_WP			PORTBbits.RB4		// this will be ignored if SD_HONOR_WP is not defined
#define SD_PIN_WP_PU		CNPU1bits.CN6PUE	// use WP on a CN pin if you don't have an external pull-up, ignored if commented
// software configuration
#define SD_CD_INTERRUPT_HANDLER		// comment this if you have your own CN interrupt handler, otherwise one will be generated in the SD library
									// if you have your own handler, be sure to add the SD_CD_Interrupt_Hook() call below to your routine
#define SD_HONOR_WP					// if this is uncommented, the state of the write-protect switch as determined by SD_PIN_WP will be honored
//#define SD_DMA_WRITES				// TODO: implement this
//#define SD_STATS					// if this is uncommented, the SD_Stats structure will be defined and updated accordingly
//#define SD_STRICT_RATES				// TODO: implement this -- strictly honor the spec's transfer rate definitions even though it says all current cards must support at least 25MHz

// semi-configuration
//	the SPI transfer rates are based off the clock frequency of the chip and a pair of divisors. in order to get the SPI clock within required
//	ranges, the divisor values must be set below according to your clock frequency. see the chip datasheet for more information.
//	these 5-bit values are ORed into SPICON1, setting the two divisor values to meet the given clock speed requirements:
#define SD_SPIDIV_INITIALIZE	0x8			// the clock speed during initialization should be between 100-400 kHz
											// 0x8: 40 MHz / (64 * 6) divisor = 104.17 kHz SPI clock
#define SD_SPIDIV_NORMAL		0x1E		// the clock speed during normal use, our max (10 MHz) is well below the minimum required by the spec
											// 0x1E: 40 MHz / (4 * 1) divisor = 10 MHz SPI clock
//	TODO: add more values here for users who want SD_STRICT_RATES
/*** End of configuration ***/


// SPI register defines
#if SD_SPI_MODULE == 1
  #define SD_SPICON1		SPI1CON1
  #define SD_SPICON1bits	SPI1CON1bits
  #define SD_SPICON2		SPI1CON2
  #define SD_SPICON2bits	SPI1CON2bits
  #define SD_SPISTAT		SPI1STAT
  #define SD_SPISTATbits	SPI1STATbits
  #define SD_SPIBUF			SPI1BUF
  
  #define SD_PIN_CS 		PORTBbits.RB2		// SS1
  #define SD_PIN_CS_T		TRISBbits.TRISB2
#elif SD_SPI_MODULE == 2
  #define SD_SPICON1		SPI2CON1
  #define SD_SPICON1bits	SPI2CON1bits
  #define SD_SPICON2		SPI2CON2
  #define SD_SPICON2bits	SPI2CON2bits
  #define SD_SPISTAT		SPI2STAT
  #define SD_SPISTATbits	SPI2STATbits
  #define SD_SPIBUF			SPI2BUF
  
  #define SD_PIN_CS 		PORTGbits.RG9		// SS2
  #define SD_PIN_CS_T		TRISGbits.TRISG9
#else
  #error Invalid SD_SPI_MODULE - Please select an SPI module in sd.h
#endif

// generic constants
#define CARD_INSERTED	0
#define CARD_REMOVED	1
#define WP_PROTECTED	1
#define WP_UNPROTECTED	0

// error constants
#define ERR_SUCCESS			0		// command completed successfully
#define ERR_NO_CARD			-1		// no card is in the SD slot
#define ERR_RESET_FAILED	-2		// the reset sequence was not completed successfully
#define ERR_NO_COMM			-3		// the communications channel was lost
#define ERR_BAD_CRC			-4		// the CRC check of a data block read failed
#define ERR_DATA_FAILED		-5		// a data transaction failed for a reason other than communications error
#define ERR_WRITE_PROTECTED	-6		// a data write failed because the card's write protect switch is in the locked position

// SD constants
#define SD_CMD_TIMEOUT 2500

// command response types
// R1m is not a response type defined in the SD spec, but it is used when more data is
// to be transferred (CS is not deasserted and the eight cleanup clocks are not issued.)
// All such commands respond with an R1 response anyway, followed by their additional data.
enum { R1, R1b, R1m, R2, R3, R7 };

// SD structures
// Microchip's compiler packs bit fields right-to-left in each byte.
// I.E. if I want a two-byte struct with two-bit fields A, B, C, D, E, F, G, H to appear in that order,
// the struct should be declared with the fields in the following order: D C B A H G F E (backwards inside each byte.)
typedef union {
	struct {
		unsigned char Command 	 : 6 __attribute__ ((packed));
		unsigned char StartField : 2 __attribute__ ((packed));
		union {
			struct {
				unsigned char c3;
				unsigned char c2;
				unsigned char c1;
				unsigned char c0;
			};
			unsigned long l;
		} Arg						 __attribute__ ((packed));
		unsigned char EndField	 : 1 __attribute__ ((packed));
		unsigned char CRC		 : 7 __attribute__ ((packed));
	} Cmd;
	unsigned char Byte[6];
} SD_Cmd;

typedef union {
	struct {
		unsigned char IdleState		: 1 __attribute__ ((packed));
		unsigned char EraseReset	: 1 __attribute__ ((packed));
		unsigned char IllegalCmd	: 1 __attribute__ ((packed));
		unsigned char CRCError		: 1 __attribute__ ((packed));
		unsigned char EraseError	: 1 __attribute__ ((packed));
		unsigned char AddrError		: 1 __attribute__ ((packed));
		unsigned char ParamError	: 1 __attribute__ ((packed));
		unsigned char StartField	: 1 __attribute__ ((packed));
	} Response;
	unsigned char Byte[1];
} SD_R1;

typedef union {
	struct {
		// R1
		unsigned char IdleState		: 1 __attribute__ ((packed));
		unsigned char EraseReset	: 1 __attribute__ ((packed));
		unsigned char IllegalCmd	: 1 __attribute__ ((packed));
		unsigned char CRCError		: 1 __attribute__ ((packed));
		unsigned char EraseError	: 1 __attribute__ ((packed));
		unsigned char AddrError		: 1 __attribute__ ((packed));
		unsigned char ParamError	: 1 __attribute__ ((packed));
		unsigned char StartField	: 1 __attribute__ ((packed));
		
		unsigned char CardLocked	: 1 __attribute__ ((packed));
		unsigned char WPEraseSkip	: 1 __attribute__ ((packed));
		unsigned char GenericError	: 1 __attribute__ ((packed));
		unsigned char InternalError	: 1 __attribute__ ((packed));
		unsigned char ECCFailed		: 1 __attribute__ ((packed));
		unsigned char WPViolation	: 1 __attribute__ ((packed));
		unsigned char EraseParam	: 1 __attribute__ ((packed));
		unsigned char OutOfRange	: 1 __attribute__ ((packed));
	} Response;
	unsigned char Byte[2];
} SD_R2;

typedef union {
	struct {
		// R1
		unsigned char IdleState		: 1 __attribute__ ((packed));
		unsigned char EraseReset	: 1 __attribute__ ((packed));
		unsigned char IllegalCmd	: 1 __attribute__ ((packed));
		unsigned char CRCError		: 1 __attribute__ ((packed));
		unsigned char EraseError	: 1 __attribute__ ((packed));
		unsigned char AddrError		: 1 __attribute__ ((packed));
		unsigned char ParamError	: 1 __attribute__ ((packed));
		unsigned char StartField	: 1 __attribute__ ((packed));
		
		union {
			unsigned char c[4];
		} OCR							 __attribute__ ((packed));
	} Response;
	unsigned char Byte[5];
} SD_R3;

typedef union {
	struct {
		// R1
		unsigned char IdleState		: 1 __attribute__ ((packed));
		unsigned char EraseReset	: 1 __attribute__ ((packed));
		unsigned char IllegalCmd	: 1 __attribute__ ((packed));
		unsigned char CRCError		: 1 __attribute__ ((packed));
		unsigned char EraseError	: 1 __attribute__ ((packed));
		unsigned char AddrError		: 1 __attribute__ ((packed));
		unsigned char ParamError	: 1 __attribute__ ((packed));
		unsigned char StartField	: 1 __attribute__ ((packed));
		
		unsigned char Reserved1		: 4 __attribute__ ((packed));
		unsigned char CmdVersion	: 4 __attribute__ ((packed));
		
		unsigned char Reserved2			__attribute__ ((packed));
		
		unsigned char Voltage		: 4 __attribute__ ((packed));
		unsigned char Reserved3		: 4 __attribute__ ((packed));
		
		unsigned char CheckPattern		__attribute__ ((packed));
	} Response;
	unsigned char Byte[5];
} SD_R7;

// card specific data (CSD) register structure
typedef struct
{
	unsigned int CSD_STRUCTURE		: 2		__attribute__ ((packed));
	unsigned int Reserved1			: 6		__attribute__ ((packed));
	unsigned int TAAC				: 8		__attribute__ ((packed));
	unsigned int NSAC				: 8		__attribute__ ((packed));
	unsigned int TRAN_SPEED			: 8		__attribute__ ((packed));
	unsigned int CCC				: 12	__attribute__ ((packed));
	unsigned int READ_BL_LEN		: 4		__attribute__ ((packed));
	unsigned int READ_BL_PARTIAL	: 1		__attribute__ ((packed));
	unsigned int WRITE_BLK_MISALIGN	: 1		__attribute__ ((packed));
	unsigned int READ_BLK_MISALIGN	: 1		__attribute__ ((packed));
	unsigned int DSR_IMP			: 1		__attribute__ ((packed));
	unsigned int Reserved2			: 2		__attribute__ ((packed));
	unsigned int C_SIZE				: 12	__attribute__ ((packed));
	unsigned int VDD_R_CURR_MIN		: 3		__attribute__ ((packed));
	unsigned int VDD_R_CURR_MAX		: 3		__attribute__ ((packed));
	unsigned int VDD_W_CURR_MIN		: 3		__attribute__ ((packed));
	unsigned int VDD_W_CURR_MAX		: 3		__attribute__ ((packed));
	unsigned int C_SIZE_MULT		: 3		__attribute__ ((packed));
	unsigned int ERASE_BLK_LEN		: 1		__attribute__ ((packed));
	unsigned int SECTOR_SIZE		: 7		__attribute__ ((packed));
	unsigned int WP_GRP_SIZE		: 7		__attribute__ ((packed));
	unsigned int WP_GRP_ENABLE		: 1		__attribute__ ((packed));
	unsigned int Reserved3			: 2		__attribute__ ((packed));
	unsigned int R2W_FACTOR			: 3		__attribute__ ((packed));
	unsigned int WRITE_BL_LEN		: 4		__attribute__ ((packed));
	unsigned int WRITE_BL_PARTIAL	: 1		__attribute__ ((packed));
	unsigned int Reserved4			: 5		__attribute__ ((packed));
	unsigned int FILE_FORMAT_GRP	: 1		__attribute__ ((packed));
	unsigned int COPY				: 1		__attribute__ ((packed));
	unsigned int PERM_WRITE_PROTECT	: 1		__attribute__ ((packed));
	unsigned int TMP_WRITE_PROTECT	: 1		__attribute__ ((packed));
	unsigned int FILE_FORMAT		: 2		__attribute__ ((packed));
	unsigned int Reserved5			: 2		__attribute__ ((packed));
	unsigned int CRC				: 7		__attribute__ ((packed));
	unsigned int NotUsed			: 1		__attribute__ ((packed));
} SD_CSD_V1;

typedef struct
{
	unsigned int NotUsed			: 1		__attribute__ ((packed));
	unsigned int CRC				: 7		__attribute__ ((packed));
	unsigned int Reserved6			: 2		__attribute__ ((packed));
	unsigned int FILE_FORMAT		: 2		__attribute__ ((packed));
	unsigned int TMP_WRITE_PROTECT	: 1		__attribute__ ((packed));
	unsigned int PERM_WRITE_PROTECT	: 1		__attribute__ ((packed));
	unsigned int COPY				: 1		__attribute__ ((packed));
	unsigned int FILE_FORMAT_GRP	: 1		__attribute__ ((packed));
	unsigned int Reserved5			: 5		__attribute__ ((packed));
	unsigned int WRITE_BL_PARTIAL	: 1		__attribute__ ((packed));
	unsigned int WRITE_BL_LEN		: 4		__attribute__ ((packed));
	unsigned int R2W_FACTOR			: 3		__attribute__ ((packed));
	unsigned int Reserved4			: 2		__attribute__ ((packed));
	unsigned int WP_GRP_ENABLE		: 1		__attribute__ ((packed));
	unsigned int WP_GRP_SIZE		: 7		__attribute__ ((packed));
	unsigned int SECTOR_SIZE		: 7		__attribute__ ((packed));
	unsigned int ERASE_NLK_LEN		: 1		__attribute__ ((packed));
	unsigned int Reserved3			: 1		__attribute__ ((packed));
	unsigned long int C_SIZE		: 22	__attribute__ ((packed));
	unsigned int Reserved2			: 6		__attribute__ ((packed));
	unsigned int DSR_IMP			: 1		__attribute__ ((packed));
	unsigned int READ_BLK_MISALIGN	: 1		__attribute__ ((packed));
	unsigned int WRITE_BLK_MISALIGN : 1		__attribute__ ((packed));
	unsigned int READ_BL_PARTIAL	: 1		__attribute__ ((packed));
	unsigned int READ_BL_LEN		: 4		__attribute__ ((packed));
	unsigned int CCC				: 12	__attribute__ ((packed));
	unsigned int TRAN_SPEED			: 8		__attribute__ ((packed));
	unsigned int NSAC				: 8		__attribute__ ((packed));
	unsigned int TAAC				: 8		__attribute__ ((packed));
	unsigned int Reserved1			: 6		__attribute__ ((packed));
	unsigned int CSD_STRUCTURE		: 2		__attribute__ ((packed));
} SD_CSD_V2;

typedef union
{
	char Bytes[16];
	SD_CSD_V1 V1;
	SD_CSD_V2 V2;
} SD_CSD;

// extern card state structure
extern struct SD_Card_State_t
{
	char high_capacity;
	char valid;
	char first;
} SD_Card_State;

// stats structure
#ifdef SD_STATS
  extern struct SD_Stats_t
  {
	  unsigned long com_crc;		// command CRC (CRC7) errors
	  unsigned long dr_crc;			// data read CRC (CRC16) errors
	  unsigned long dw_crc;			// data write CRC (CRC16) errors
	  // TODO: maybe comm_lost, function call counts?
  } SD_Stats;
#endif

// CS assert/de-assert macros
#define SD_CS_Assert() do { SD_PIN_CS = 0; } while(0)
#define SD_CS_Deassert() do { SD_PIN_CS = 1; } while(0)

// misc. macros
#define IsValidResponse(r) (r.Response.StartField == 0)
#define SD_Invalidate_State() do { SD_Card_State.valid = 0; } while(0)

// sanity check macro
#define SD_SanityCheck()											\
	do {															\
		if(!SD_Card_State.first) { 	 /* need the hardware to be */	\
			SD_Bus_Init();			 /* initialized in order to */	\
			SD_Card_State.first = 1; /* do some of the checks   */	\
		}															\
																	\
		if(SD_PIN_CD == CARD_REMOVED) /* check for inserted card */	\
		{															\
			SD_Invalidate_State();									\
			return ERR_NO_CARD;										\
		}															\
																	\
		/* check for a busy signal */								\
		SD_CS_Assert();												\
		while(SD_Bus_ReadByte() != 0xFF);							\
		SD_CS_Deassert();											\
																	\
		/* if the card's not been verified, perform a reset */		\
		if(!SD_Card_State.valid) {									\
			int ret = SD_Reset();									\
			if(ret != ERR_SUCCESS) return ret;						\
		}															\
	} while(0)
	
// CN interrupt hook macro
#define SD_CD_Interrupt_Hook()										\
	do {															\
		if(SD_PIN_CD == CARD_REMOVED)								\
			SD_Invalidate_State();									\
	} while(0)
	
// function declarations
  // bus.c
  void SD_Bus_WriteByte(unsigned char c);
  unsigned char SD_Bus_ReadByte(void);
  void SD_Bus_DummyClock(unsigned int n);
  void SD_Bus_Init(void);
  // cmd.c
  void SD_SendCommand(SD_Cmd cmd, void *response, int responsetype);
  // crc.c
  unsigned int SD_CRC16_Calculate(unsigned char *buf, unsigned len);  // unused
  unsigned char SD_CRC7_Calculate(SD_Cmd *cmd);
  // data.c
  int SD_Data_Read(unsigned char *buf, unsigned len);
  int SD_Data_Write(unsigned char *buf, unsigned len);
  // reset.c
  int SD_Reset(void);
  // SectorRead.c
  int SD_Sector_Read(unsigned char *buf, unsigned long sector_num);
  // SectorWrite.c
  int SD_Sector_Write(unsigned char *buf, unsigned long sector_num);

  // Info.c
  int SD_GetTotalSize(unsigned long long int* size);
	
#endif
