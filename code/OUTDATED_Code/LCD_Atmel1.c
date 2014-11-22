/*
 * LCD_Atmel1.c
 *
 * Created: 10/24/2014 3:29:51 PM
 *  Author: LuisDavid
 * still a alpha Version 0.2
 */ 
// CARLOS:
// FROM LCD_Atmel.h you there is a line you can modify at will
// to change how the cursor looks like 
  #define F_CPU 8000000UL // 1MHz internal clock speed of ATmega328
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <util/delay.h>
  #include <util/twi.h>
  #include "TWI_Master.h"
  #include "NH_LCD.h"
  
void LCD_LONG_MEM_WRITE();
void  init_lcd();
void LCD_SINGLE_MEM_WRITE (unsigned char BYTE);


int main(void)
{
	unsigned char single[1];
	unsigned char word[11]={"hello world"};
	

	//all steps to initialize and timings 
	init_lcd();
	
	// store and send one char at the time using Display single
	single[0] = 'C';
	Display_Single(single);
	single[0] = 'A';
	Display_Single(single);
	single[0] = 'R';
	Display_Single(single);
	single[0] = 'L';
	Display_Single(single);
	single[0] = 'O';
	Display_Single(single);
	single[0] = 'S';
	Display_Single(single);

	_delay_ms(500);
	
	// changes the cursor address to the second line
	// no need to fill the line
	Second_Line();
	
	// same sequence as above one char at a time 
	single[0] = 'D';
	Display_Single(single);
	single[0] = 'O';
	Display_Single(single);
	single[0] = 'N';
	Display_Single(single);
	single[0] = 'E';
	Display_Single(single);
	single[0] = '!';
	Display_Single(single);
	
	_delay_ms(1000);
	
	// Clear and bring cursor to 0,0
	Clear_Screen();			
	
	// DONT USE THIS ONE YET NEEDS MORE THINKING BUT IT WORKS 
	Display_Word (word); 
	_delay_ms(3000);
	
	// only to know I was done Ignore
    while(1)
    {
       
       PORTB |= (1<<PORTB1); //turn blue LED on to signify we are in at home screen
       _delay_ms(500);
       PORTB &= ~(1<<PORTB1); //turn red LED off
       _delay_ms(500);   
    }
	return 0;
}

