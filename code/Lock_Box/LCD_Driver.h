


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
#define DISP_ON_CMD				0x0E	// Display ON command control cursor (0x0C-0x0F)


//this matrix is used to map the location of the cursor of the LCD. the LCD is 2x20 
unsigned char Cursor_Position[2][20]={
{0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93},
{0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3},
};


/*pre determined messages. may edit, but messages may not eccess more than 20 bytes.
this also includes spaces*/
unsigned char Message_Home[]={"Press START to Begin"};
unsigned char Message_Unlock[]={"Open Sesame!!!"};
unsigned char Message_Try_Again[]={"Try Again :("};
unsigned char Message_Enter_Code[]={"Enter Code To Unlock"};
unsigned char Message_Enter_New_Passcode[]={"Enter New Passcode"};
unsigned char Message_Passcode_Saved[]={"New Passcode Saved"};
unsigned char Message_Time_Out_1[]={"Timed Out"};
unsigned char Message_Time_Out_2[]={"Try Again"};	
	

/*these array are used to sen dot the LCD character Generating RAM*/
unsigned char Character_Bytes[4][8]={
{0x04,0x0E,0x015,0x04,0x04,0x04,0x04,0x04},
{0x04,0x04,0x04,0x04,0x04,0x15,0x0E,0x04},
{0x00,0x00,0x00,0x1F,0x11,0x1F,0x00,0x00},	
};
	

// function prototypes
void LCD_LONG_MEM_WRITE();
void Initialize_LCD();
void LCD_SINGLE_MEM_WRITE (unsigned char BYTE);
void Second_Line();
void Move_Cursor_to(int row, int column);
void Display_Single(unsigned char  *chr_msg);
void LEDs (int green,int red,int blue);
void Print_User_Input();
void Print_User_Input_2();
void Create_UP_DOWN_Chararters();
void Time_Out_Message();



/*******************************************************************************
*                        Display to screen  Functions 						   *
*******************************************************************************/
// this function is called to print out single characters to LCD
void Display_Single(unsigned char  *chr_msg)
{
	unsigned char DATA_out[3];
	DATA_out[0] = i2c_addr ;		//LCD Address
	DATA_out[1] = WRITE_CMD;		//Control word instruction will follow
	DATA_out[2] = *chr_msg;			//instruction byte to be sent
	
	
	TWI_Start_Transceiver_With_Data(DATA_out, 3);
}

// move cursor to the second line at left side
/*
void Second_Line()

{
unsigned char DATA_out[3];
DATA_out[0] = i2c_addr ;		//LCD Address
DATA_out[1] = DISP_CMD;			//Control word instruction will follow
DATA_out[2] = LINE_2;			//instruction byte to be sent

TWI_Start_Transceiver_With_Data(DATA_out, 3);
}
*/	

/*******************************************************************************
*                         Commands instructions								   *
*******************************************************************************/

//this function is called when an LCD command is sent
void LCD_SINGLE_MEM_WRITE (unsigned char BYTE)
{
	unsigned char DATA_out[3];
	DATA_out[0] = i2c_addr ;	//LCD Address
	DATA_out[1] = DISP_CMD;	//Control word instruction will follow
	DATA_out[2] = BYTE;			//instruction byte to be sent
	
	
	TWI_Start_Transceiver_With_Data(DATA_out, 3);
}

//this function sends neccesary instruction to properly initialize LCD
void  Initialize_LCD()
{
	TWI_Master_Initialise();
	_delay_ms(CMD_DELAY);
	
	unsigned char  LCD_COMMANDS[11];
	
	LCD_COMMANDS[0]= i2c_addr;			// first Store the LCD address
	LCD_COMMANDS[1]= DISP_CMD;			// Control word instructions will follow
	LCD_COMMANDS[2]= FUNC_SET_TBL0;
	LCD_COMMANDS[3]= FUNC_SET_TBL1;
	LCD_COMMANDS[4]=0x14;				// cursor Shift
	LCD_COMMANDS[5]=CONTRAST_CMD;		// Contrast
	LCD_COMMANDS[6]=0x5E;				// Power/icon/control
	LCD_COMMANDS[7]=0x6D;				// Set Follower
	LCD_COMMANDS[8]=0x0C;				// Display on command no cursor
	LCD_COMMANDS[9]=CLEAR_DISP_CMD;		// Clear display command
	LCD_COMMANDS[10]=0x07;				// Display on or OFF
	
	TWI_Start_Transceiver_With_Data(LCD_COMMANDS, 9);
	
	Create_UP_DOWN_Chararters();
}

//set LCD to double height
void Double_Height()
{
	LCD_SINGLE_MEM_WRITE (0x34); 
	_delay_ms(CMD_DELAY);
	LCD_SINGLE_MEM_WRITE(0x35);
	_delay_ms(CMD_DELAY);
}

//set LCD to single height
void Single_Height()
{
	LCD_SINGLE_MEM_WRITE (0x38);
	_delay_ms(CMD_DELAY);
	LCD_SINGLE_MEM_WRITE(0x39);
	_delay_ms(CMD_DELAY);
}

/*return the cursor to "home" which is row 1 column 1
alternatively the Move_Cursor_to(1,1) function can be used*/
void Cursor_Home()
{
	LCD_SINGLE_MEM_WRITE (0x02);
}

//clear contents of LCD. this also return cursor to "home"
void Clear_LCD()
{
	LCD_SINGLE_MEM_WRITE (0x01);
}

/*this function tell the LCD where to place the Cursor
Input: Y,X cordinates from row 1 to 2 and column 1-20
Ouput:void */
void Move_Cursor_to(int row, int column)
{
	LCD_SINGLE_MEM_WRITE (Cursor_Position[row-1][column-1]);
}

/*******************************************************************************
*                         Pre-defined message Functions						   *
*******************************************************************************/
//print home message to LCD
void Home_Message()
{
	int j=0;
	Clear_LCD();
	
	for (j=0; j<sizeof(Message_Home); j++)
	{
		Display_Single(&Message_Home[j]);
	}
}

//print unlock message to LCD
void Unlock_Message()
{
	int j=0;
	Clear_LCD();
	Move_Cursor_to(1,5);
	
	for (j=0; j<sizeof(Message_Unlock); j++)
	{
		Display_Single(&Message_Unlock[j]);
	}
}

//print try again message to LCD
void Try_Again_Message()
{
	int j=0;
	Clear_LCD();
	Move_Cursor_to(1,5);
	
	for (j=0; j<sizeof(Message_Try_Again); j++)
	{
		Display_Single(&Message_Try_Again[j]);
	}
}

//print Enter code message to LCD
void Enter_Code_Message()
{
	int j=0;
	Clear_LCD();
	//Move_Cursor_to(1,5);
	
	for (j=0; j<sizeof(Message_Enter_Code); j++)
	{
		Display_Single(&Message_Enter_Code[j]);
	}
}

//print Enter new Passcode  to LCD
void Enter_New_Passcode_Message()
{
	int j=0;
	Clear_LCD();
	Move_Cursor_to(1,2);
	
	
	for (j=0; j<sizeof(Message_Enter_New_Passcode); j++)
	{
		Display_Single(&Message_Enter_New_Passcode[j]);
	}
}

//print Passcode saved  to LCD
void Passcode_Saved_Message()
{
	int j=0;
	//Clear_LCD();
	Move_Cursor_to(1,2);
	
	for (j=0; j<sizeof(Message_Passcode_Saved); j++)
	{
		Display_Single(&Message_Passcode_Saved[j]);
	}
	LEDs(0,0,1);
	_delay_ms(200);
	LEDs(0,1,0);
	_delay_ms(200);
	LEDs(1,0,0);
	_delay_ms(200);
	
}

//print Time out  to LCD
void Time_Out_Message()
{
	int j=0;
	Clear_LCD();
	Move_Cursor_to(1,6);
	
	for (j=0; j<sizeof(Message_Time_Out_1); j++)
	{
		Display_Single(&Message_Time_Out_1[j]);
	}
	
	Move_Cursor_to(2,6);
	for (j=0; j<sizeof(Message_Time_Out_2); j++)
	{
		Display_Single(&Message_Time_Out_2[j]);
	}
	
}


/*******************************************************************************
*                         Miscellaneous Functions							   *
*******************************************************************************/

/*this function handles the LCD LEDs 
Inputs : integer 1 for on 0 for off
Output: void */
void LEDs (int green,int red,int blue)
{
	int LED_State[3];
	LED_State[0]=red;
	LED_State[1]=green;
	LED_State[2]=blue;
	int k=0;
	for (k=0;k<=2;k++)
	{
		if (LED_State[k])
		{
			PORTB |= (1<<k);
		}
		else
		{
			PORTB &= ~(1<<k);
		}
	}
}


/*this function is	ONLY CALLED ONCE. it is called automatically in the 
Initialize_LCD() function. it creates the up and down arrows. 
Inputs:void
Outputs:void
*/
void Create_UP_DOWN_Chararters()
{
	///this array holds the DDRAM address for the new characters
	unsigned char Character_Posisiton[3]={0x01,0x02,0x03};
	//array that holds CGRAM address for characters	
	unsigned char CGRAM_Posisiton[3]={0x48,0x50,0x58};
	int i; //counting variables
	int j;
	
	/*to write a new character the DDRAm address must be set firsts. 
	the LCD supports 8 user defined characters. ( I tried to enter 4 but
	it will give fatal errors so I only created a character for up down and select
	Carlos M)  after sending the DDRAM address a CGRAm address is necessary (look at 
	LCD driver LCD for more details)*/
	
	/*this loop steps through all bytes that need to be sent to the LCD module*/
	for (j=0;j<3;j++)
	{
		LCD_SINGLE_MEM_WRITE(Character_Posisiton[j]);
		LCD_SINGLE_MEM_WRITE(FUNC_SET_TBL0);
		_delay_ms(CMD_DELAY);
		LCD_SINGLE_MEM_WRITE(CGRAM_Posisiton[j]);	
		
		for (i=0;i<8;i++)
		{
			Display_Single(&Character_Bytes[j][i]);
		}
		LCD_SINGLE_MEM_WRITE(FUNC_SET_TBL1);
		_delay_ms(CMD_DELAY);
	}
}
