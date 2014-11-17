/*definitions for controller buttons in hex
these HEX values are the outputs of the encoder
DO NOT USE THIS VALUES AS THE COMBINATION
******************************************************/
#define Select_Button 0x00
#define B_Button 0x01
#define A_Button 0x02
#define Up_button 0x03
#define Right_Button 0x04
#define Down_Button 0x05
#define Left_Button 0x06
#define NO_Press 0x07
//******************************************************

/* these are the HEX values that will be sent to the LCD, most of the values 
correspond to the ASCCII table, but not all do*/
#define UP		0x01
#define DOWN	0x02
#define RIGHT	0x7E
#define LEFT	0X7F
#define A		0x41
#define B		0x42
#define SELECT  0x03




unsigned char Buttons[8]={Select_Button,B_Button,A_Button,Up_button,Right_Button,Down_Button,Left_Button,NO_Press};


unsigned char Output [8]={SELECT,B,A,UP,RIGHT,DOWN,LEFT,0x00};
	



/* this will be the "master" combination. the combination needs to be 10 inputs long. if the user desires 
to change combination simple change pattern bellow*/
unsigned char Combination[10]={UP,UP,DOWN,DOWN,LEFT,RIGHT,LEFT,RIGHT,B,A};	
	
	
//counter to be used to increment eeprom addresing
uint8_t Counter_eeprom[10]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};

//initializing functions
unsigned char Button_Press_Detected(unsigned char Button_Value);
unsigned char PollController();
void Check_For_Match(unsigned char Button_Pressed);
void Check_Combination();
void Reset_Return_Home();
void User_Defined_Passcode();
