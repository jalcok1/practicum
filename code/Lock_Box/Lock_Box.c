/*
ECE 411 
Fall 2014
Lock Box project

Remember the Nintendo Entertainment System (NES)? We sure do. 
It simply is a box that locks. But thats not the cool part. 
This box is electronically locked, and must be unlocked by using
the old school NES controller as a user interface.

visit the wiki for more information:
https://github.com/jalcok1/practicum/wiki 
 
 */ 

#define F_CPU 8000000UL // 1MHz internal clock speed of ATmega328
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdbool.h>
#include <avr/eeprom.h>
#include <stdint.h>

#include "TWI_Master.h"
#include "LCD_Driver.h"
#include "Controller_Driver.h"

unsigned char Previous_Read = 0x07; //variable to hold encoder status
unsigned char Current_Read  = 0x00;
int  Confidence_Level = 0;
volatile bool User_Input = false;
volatile int Input_Index = 0;
volatile int Input_Index_2 = 0;
volatile int Combiantion_Match_counter = 0;
volatile bool Continue_Match_Check =true;
volatile bool No_Press_Flag = false;
volatile char Button_Held = 0x00;
volatile bool New_Passcode_Entry = true;
volatile bool Time_Out_Flag=false;


/*interrupt routine to handle START button press. we will call "home screen" when 
the LCD displays "press START to begin" at the beginning of the program the LCD will be set
to home screen also after a determined time out the program will return to home screen.
while in home screen the user must press START to begin the unlocking process. in this interrupt
function the user input will be enable and the timer interrupt will also be enabled.*/
// toggles arbitrary LED on and off for debug
ISR(INT0_vect)
{
	int k=0;
	
	EIMSK &= ~(1<<INT0); //disable interrupt on INT0
	TCNT1 = 0; //reset timer counter to 0
	TIFR1 |= (1<<OCF1A); //clear timer interrupt flag
	
	User_Input=true; // enable User input from NES controller
	
	//counter for debounce
	for (k=0;k<1000;k++);
	TIMSK1 |= 1<<OCIE1A; //enable interrupt for counter	
}

/*interrupt routine for internal counter.this will be used as our 
time out counter. after a determined time out the program will return to home screen were 
user input is disabled. to enable user input the user must press START which in turn causes
an interrupt described above*/
ISR(TIMER1_COMPA_vect)
{
	EIMSK |= (1<<INT0); //enable interrupt on INT0
	TIMSK1 &= ~(1<<OCIE1A); // disable interrupt for counter
	
	//disable user input from NES controller and return to home screen 
	User_Input=false;	
	
	//reseting all variables
	User_Input=false;
	Combiantion_Match_counter=0;
	Input_Index=0;
	Continue_Match_Check =true;
	New_Passcode_Entry=false;
	Time_Out_Flag=true;
	
	LEDs(1,1,1); //white back light
}


int main(void)
{
    //setting PBC0-PC3 as inputs
	DDRC &= ~((1<<DDC0)|(1<<DDC1)|(1<<DDC2)|(1<<DDC3));//pins 23-26 or PC0-PC3  as inputs
	PORTC |= ((1<<PORTC0)|(1<<PORTC1)|(1<<PORTC2)|(1<<PORTC3));//enable pull-ups
	
	DDRD |= ((1<<DDD5)|(1<<DDD4)); // setting PD5(Reset_LCD) and PD4(Solenoid) as output for solenoid and reset for LCD
	PORTD |= (1<<PORTD5); //set high for LCD reset (reset is active low)
	
	DDRB |= ((1<<DDB0)|(1<<DDB1)|(1<<DDB2)); //PB0-PB2 as outputs for LCD LEDs control
	
	LEDs(1,1,1); //all on -white

	
	
	//configuring interrupt INT0
	DDRD &= ~(1<<DDD2); //PD2 as input
	PORTD |= (1<<PORTD2); //enable pull-up on PD2
	EICRA |= (1<<ISC01); //set interrupt on INT0 falling edge
	EIMSK |= (1<<INT0); //enable interrupt on INT0
	
	//timer setup
	//set prescale-divide clock by 1024 and enable clear after match
	//so counter runs at ~1kHz.
	TCCR1B |= (1<<CS12 | 1<<CS10 | 1<<WGM12); 
	OCR1A = 30000; // set value for counter every 1000=~ 1second
	
	sei(); //enable global interrupts
	
	
	 
	Initialize_LCD();
	
	
	//main program loop
	while(1)
	{
		
		Home_Message();
		_delay_ms(25);
	
		while (User_Input)
		{
			LEDs(0,0,1); //blue LED on
			Enter_Code_Message();
			Move_Cursor_to(2,1);

			/*this double while loop was an easy way to just print to LCD once
			without having to worry about more variables*/
			while (User_Input)
			{
				
			
			unsigned char Button_Pressed = 0x00;
			/*
				in this block of code we will have to display the user input into 
				the LCD and also match the input to the predetermined combination.if 
				a match is reached we can then move out of this while statement and disable 
				user input, and open lock*/
			Button_Pressed = PollController();
			
			/*if encoder output is 0x00 means no buttons have been pressed
			so this this only be tru if a button is pressed and the user is not holding down a button*/
			if ((Button_Pressed != 0x00) & (Button_Held != Button_Pressed))
			{
				Button_Held = Button_Pressed;
				No_Press_Flag = false;
				
				Print_User_Input(&Button_Pressed);

				/*this if statement will only be true if the user pressed a button for the
				firsts time or if a partial match to the combination was made. this will allow us 
				to skip a matching inputs to the combination if the a wrong value was entered previously.*/
				if ((Input_Index==0) | Continue_Match_Check)
				{
					Check_For_Match(Button_Pressed);
					Input_Index++;
				}
				/* if we get into this else we already now that the combination entered is wrong,
				so we only need to increment a counter so we can keep track of when to alert
				 the user the combination is wrong i.e. wait till all user inputs have been entered*/
				else
				{
					Input_Index++;
				}
				
				
				//if the user has pressed a button 6 times lets check if 
				//the lock needs to open or not and return to home screen
				if (Input_Index>=10)
				{
					Check_Combination();
				}
				
			}
			
			/*this statement is used to filter out a no press. meaning 
			that no button has been pressed. this flag is only false when 
			the user has pressed a button. when no buttons are pressed the No_Press_Flag is 
			set. after the flag is set we can rule out a button being held down */
			if (No_Press_Flag)
			{
				Button_Held = 0x00;
			}
			
		}
		}
			
	}
	return 0;
}

/*This function polls the NES controller for user input. the user input is 
processed by the Button_Press_Detected function which outputs a hex value 
that will be sent to the LCD and to compare for 
a match of correct combination.
Input:  none
Output: Hex value that will be sent to LCD and for combination match*/
unsigned char PollController()
{
		unsigned char Button_Pressed=0x00;
		Current_Read=PINC; //reading pins
		
		/********************************
		*********************************
		masking pins that don't matter
		will need to unmask bit4 if we want to detect START being pressed*/
		Current_Read= Current_Read & 0b00000111; 
		
		//this check will check the previous status of the pins for debouncing 
		if (Current_Read==Previous_Read)
		{
			//if the previous state equals current increase confidence level else so nothing
			Confidence_Level++;
			if(Confidence_Level>500) //arbitrary confidence level
			{
				Button_Pressed=Button_Press_Detected(Current_Read);
				Confidence_Level=0; //reset counter
				
				/*this statement is to set a flag variable when a no press has been detected.
				this is useful so we can track when the user has depressed a button.*/
				if (Button_Pressed==0x00)
				{
					No_Press_Flag = true;
				}
				
			}
		}
		Previous_Read=Current_Read; //store current state	
	
	return Button_Pressed;
}




/*This function takes the current state of the encoder and outputs 
the corresponding hex value to print out the user input to the LCD
Input: current state of encoder
Output: Hex value that will be sent to LCD*/
unsigned char Button_Press_Detected(unsigned char Button_Value)
{
	unsigned char Button_Pressed=0x00;
	int i;
	/*step through all possible values of the encoder output 
	and match it to a button being pressed or not*/
	for (i=7;i>=0;i--)
	{
		if (Button_Value==Buttons[i])
		{
			/*print out the output. this output array conatains the value that
			will be printed to the LCD and be used to compare to the correct
			combination*/

			Button_Pressed = Output[i];
		}
	}
	return Button_Pressed;
}

void Check_For_Match(unsigned char Button_Pressed)
{
	uint8_t eeprom_Memory;
	eeprom_Memory = eeprom_read_byte(&Counter_eeprom[Input_Index]);
	/*if the button that was pressed matches to the correct combination
	given the index the we will increase the "correctness counter*/
	if ((Button_Pressed == Combination[Input_Index]) | (Button_Pressed ==eeprom_Memory))
	{
		Combiantion_Match_counter++; //increase correctness counter
	} 
	/* if the input does not match the correct combination then set the 
	continue checking variable to false. this will allow us to ignore future inputs. 
	i.e. makes no sense to check checking if one is already incorrect */
	else
	{
		Continue_Match_Check=false;
	}
}


/*this function is called when the user has inputed a combination
if the combination is correct then the lock will be disengaged for~2sec
after execution of this function the program will return to home screen
Input:void
Output:void*/
void Check_Combination()
{
	
	
	if (Combiantion_Match_counter>=10)
	{
		LEDs(1,0,0); //green on
		Unlock_Message();
		PORTD |= 1<<PORTD4; // open lock
		_delay_ms(300);
		PORTD &= ~(1<<PORTD4); //lock off
		_delay_ms(300);
				
		Current_Read=PINC; //reading pins
		Current_Read= Current_Read & 0b00001111; //masking
		
		/*this if statement checks if START and SELECT and pressed. if true then 
		it will make a function call that will handle the user input of a new passcode*/
		if(Current_Read==0x00)
		{
			User_Defined_Passcode(); //function call
		}
		
		Reset_Return_Home(); //function call
			
	}
	else
	{
		LEDs(0,1,0); //red on
		Try_Again_Message(); //print to LCD
		_delay_ms(300);
		Reset_Return_Home();//function call
	}
}




void Reset_Return_Home()
{
	LEDs(1,1,1); //white on
	//reset all variables
	User_Input=false;
	Combiantion_Match_counter=0;
	Input_Index=0;
	Continue_Match_Check =true;	
	EIFR |= (1<<INTF0); //clear interrupts on INT0 if they exists
	EIMSK |= (1<<INT0); //enable interrupt on INT0
	TCNT1 = 0; //reset timer counter to 0
	TIFR1 |= (1<<OCF1A); //clear timer interrupt flag
	TIMSK1 &= ~(1<<OCIE1A); // disable interrupt for counter
	
}

/* this function prints the input from the controller to the LCD after
A successive input the previous input will be "covered" with a asterisk
Input :pointer to the user input*
Output: void*/
void Print_User_Input(unsigned char *Button_Pressed)
{
	/*lets check if this is the first input.
	this will determine where to place the asterisks*/
	if (Input_Index==0)
	{
		Display_Single(Button_Pressed); //print to LCD
	}
	/*if not the first input. now we start to place asterisk on previous inputs*/
	else
	{
		Move_Cursor_to(2,(Input_Index*2-1)); // go back to previous input
		Display_Single("*");//print *
		Move_Cursor_to(2,(2*Input_Index+1)); // move forward to next space for current input
		Display_Single(Button_Pressed); //print current input to LCD
	}
}


/*this function is used when the user wants to program a passcode. the function will
poll the controller and use software debounce on the user input. after a valid input is 
registered the input will be saved into non volatile memory (eeprom)
Input:void
Output:void
*/
void User_Defined_Passcode()
{
	Enter_New_Passcode_Message(); //print to LCD
	Move_Cursor_to(2,1); //move cursor to second line
	New_Passcode_Entry=true; // set flag to true to enter while statement
	Input_Index_2=0; //index to keep track of user inputs
	Time_Out_Flag=false; //timeout flag
	unsigned char Temp_Buffer[10]; //initialize temporary buffer
	int i=0; //counting variable
	
	_delay_ms(100); //delay so user can release START and SELECT buttons

	//lets save current eeprom passcode to a temp buffer
	for (i=0;i<10;i++)
	{
		Temp_Buffer[i]=eeprom_read_byte(&Counter_eeprom[i]);
	}
	
	/*entering infinite while loop. this loop can only be broken if a timeout occurs or when
	the user enters 10 characters for a passcode*/
	while (New_Passcode_Entry)
	{
		unsigned char Button_Pressed = 0x00; //initialize variable 0x00 means no button pressed	
		
		Button_Pressed = PollController(); // function call

		/*if encoder output is 0x00 means no buttons have been pressed
		so this this only be true if a button is pressed and the user is not holding down a button*/
		if ((Button_Pressed != 0x00) & (Button_Held != Button_Pressed))
		{
			Button_Held = Button_Pressed; //store the button that was registered to compare next iteration
			No_Press_Flag = false; // set flag for no button press. this will help to detect a button being held down
			
			//functon call to print user input to LCD. 
			Print_User_Input_2(&Button_Pressed);
			
			eeprom_write_byte(&Counter_eeprom[Input_Index_2],Button_Pressed); //write to eeprom		
			Input_Index_2++; 
			
			//if user has entered 10 characters exit loop 
			if(Input_Index_2>=10)
			{
				New_Passcode_Entry=false; //set flag to exit loop 
			} 
		}
		
		/*this statement is used to filter out a no press. meaning 
		that no button has been pressed. this flag is only false when 
		the user has pressed a button. when no buttons are pressed the No_Press_Flag is 
		set. after the flag is set we can rule out a button being held down */
		if (No_Press_Flag)
		{
			Button_Held = 0x00;
		}		
	}
	
	
// lets check if timeout occurred if so restore eeprom to old from buffer
	if (Time_Out_Flag)
	{
		//restore eeprom memory from temporary buffer
		for (i=0;i<10;i++)
		{
			eeprom_write_byte(&Counter_eeprom[i],Temp_Buffer[i]);
		}	
		
		Time_Out_Message(); //display message
		_delay_ms(600);
	 }
	 else
	 {
		Passcode_Saved_Message(); //display message		 
	 }
	
	
	
	
}

/* this function prints the input from the controller to the LCD. this function is similar to 
Print_User_Input, the difference is that this function does not print asterisks on previous 
inputs.
Input :pointer to the user input*
Output: void*/
void Print_User_Input_2(unsigned char *Button_Pressed)
{
	/*lets check if this is the first input.
	this will determine where to place the asterisks*/
	if (Input_Index_2==0)
	{
		Display_Single(Button_Pressed); //print to LCD
	}
	/*if not the first input. now we start to place asterisk on previous inputs*/
	else
	{
		Move_Cursor_to(2,(2*Input_Index_2+1)); // move forward to next space for current input
		Display_Single(Button_Pressed); //print current input to LCD
	}
}