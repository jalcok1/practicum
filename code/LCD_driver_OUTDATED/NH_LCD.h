#define		i2c_addr           0x78
#define    CMD_DELAY           2		// Command delay in milliseconds
#define    CHAR_DELAY          0		// Delay between characters in milliseconds
#define    PIXEL_ROWS_PER_CHAR 8		// Number of pixel rows in the LCD character
#define    MAX_USER_CHARS      16		// Maximum number of user defined characters

// LCD Command set
#define DISP_CMD				0x00	// Command for the display
#define WRITE_CMD				0x40	// Write to the display RAM
#define CLEAR_DISP_CMD			0x01	// Clear display command
#define HOME_CMD				0x02	// Set cursor at home (0,0)
#define DISP_OFF_CMD			0x08	// Display off Command
#define SET_DDRAM_CMD			0x80	// Set DDRAM address command
#define CONTRAST_CMD			0x78	// Set contrast LCD command
#define FUNC_SET_TBL0			0x38	// Function set - 8 bit, 2 line display 5x8, inst table 0
#define FUNC_SET_TBL1			0x39	// Function set - 8 bit, 2 line display 5x8, inst table 
#define LINE_2					0xC0	// address to 2 write in 2nd line

/************************************************************
*        Carlos:     Modify as you like                     *
************************************************************/

#define DISP_ON_CMD			0x0E	// Display ON command control cursor (0x0C-0x0F)

// Alpha Version 0.2
// function prototypes
void LCD_LONG_MEM_WRITE();
void init_lcd();
void LCD_SINGLE_MEM_WRITE (unsigned char BYTE);
void Clear_Screen();
void Second_Line();
void Display_Word(unsigned char * word);   // do not use yet


/*******************************************************************************
*                        Display to screen  Functions 						   *
*******************************************************************************/

void Display_Single(unsigned char * chr_msg)
{
	unsigned char DATA_out[3];
	DATA_out[0] = i2c_addr ;		//LCD Address
	DATA_out[1] = WRITE_CMD;		//Control word instruction will follow
	DATA_out[2] = *chr_msg;			//instruction byte to be sent
	
	
	TWI_Start_Transceiver_With_Data(DATA_out, 3);
	
//	return 0;
}

// Send an array of characters but number of characters must be given
// can be used for error or success!
void Display_Word(unsigned char * word)
{
	int i,j;
	unsigned char  my_word[12];
	j=2;
	// The first 2 elements stored directly
	my_word[0]= i2c_addr;			// first Store the LCD address
	my_word[1]= WRITE_CMD;			// Control word instructions will follow
	
	for(i=0;i<12;++i)
	{
		my_word[j] = word[i];
		++j;
	}

	TWI_Start_Transceiver_With_Data(my_word, 14);
}
	
void Clear_Screen(void)
{
	unsigned char DATA_out[3];
	DATA_out[0] = i2c_addr ;		//LCD Address
	DATA_out[1] = DISP_CMD;			//Control word instruction will follow
	DATA_out[2] = CLEAR_DISP_CMD;	//instruction byte to be sent
	
	TWI_Start_Transceiver_With_Data(DATA_out, 3);
	_delay_ms(CMD_DELAY);
}

void Second_Line()
{
unsigned char DATA_out[3];
DATA_out[0] = i2c_addr ;		//LCD Address
DATA_out[1] = DISP_CMD;			//Control word instruction will follow
DATA_out[2] = LINE_2;			//instruction byte to be sent

TWI_Start_Transceiver_With_Data(DATA_out, 3);
}	








/*******************************************************************************
*                         Commands instructions								   *
*******************************************************************************/
void LCD_SINGLE_MEM_WRITE (unsigned char BYTE)
{
	unsigned char DATA_out[3];
	DATA_out[0] = i2c_addr ;	//LCD Address
	DATA_out[1] = DISP_CMD;	//Control word instruction will follow
	DATA_out[2] = BYTE;			//instruction byte to be sent
	
	
	TWI_Start_Transceiver_With_Data(DATA_out, 3);
}

// predefined only used by init finction SHOULD NOT BE CALLED by any other
void LCD_LONG_MEM_WRITE()
{
	unsigned char  LCD_COMMANDS[9];
	// The first 2 elements stored directly
	LCD_COMMANDS[0]= i2c_addr;			// first Store the LCD address
	LCD_COMMANDS[1]= DISP_CMD;			// Control word instructions will follow
	LCD_COMMANDS[2]=0x14;				// cursor Shift
	LCD_COMMANDS[3]=CONTRAST_CMD;		// Contrast
	LCD_COMMANDS[4]=0x5E;				// Set CGRAM Address
	LCD_COMMANDS[5]=0x6D;				// Set Follower
	LCD_COMMANDS[6]=DISP_ON_CMD;		// Display on command
	LCD_COMMANDS[7]=CLEAR_DISP_CMD;		// Clear display command
	LCD_COMMANDS[8]=0x07;				// Display on or OFF
	
	TWI_Start_Transceiver_With_Data(LCD_COMMANDS, 9);
}

void  init_lcd()
{

		sei(); //enable interrupts
	
	TWI_Master_Initialise();
	
	_delay_ms(CMD_DELAY);
	
	
	LCD_SINGLE_MEM_WRITE (FUNC_SET_TBL0);
	_delay_ms(CMD_DELAY);
	
	LCD_SINGLE_MEM_WRITE(FUNC_SET_TBL1);
	_delay_ms(CMD_DELAY);
	
	LCD_LONG_MEM_WRITE();
//	_delay_ms(CMD_DELAY);
}


