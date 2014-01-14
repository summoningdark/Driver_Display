/*
 * Display_Functions.c
 *
 *  Created on: Jan 12, 2014
 *      Author: jennifer
 */

#include "all.h"

#define uint8_t unsigned int
#define int8_t int
#define uint16_t unsigned int
#define int16_t int

//function prototypes
void SetLEDs(uint16_t LEDword);
unsigned int GetButtonPress();
int GetMenuSelection(const unsigned char List[][20]);
void LEDGpio_init();
void LCDGpio_init();
void ButtonGpioInit();
void WriteLCDDataPort(uint8_t Data);	//this function should set whatever pins are used for the LCD data port to outputs, and put Data on the pins
uint8_t ReadLCDDataPort();				//this function should set whatever pins are used for the LCD data port to inputs, and return the value on the pins
void SetLCDEN(int s);					//this function should set whatever pin is used for LCD EN high or low depending on s !=0 or s=0
void LCDdelay();						//a delay of at least 500ns
void delay_ms(uint16_t ms);				//function to delay ms milliseconds
void SetLCDControlPort(uint8_t Cmd);	//this function should set whatever pins are used for the LCD control pins to outputs and set the values as follows:

void SetLEDs(uint16_t LEDword)
{
	int i;
	unsigned int d;

	d = LEDword;
	//pull LEDclk low
	GpioDataRegs.GPADAT.bit.GPIO18 = 0;
	//pull LEDclear low, wait, then drive LEDclear high
	GpioDataRegs.GPADAT.bit.GPIO17 = 0;
	DELAY_US(1);
	GpioDataRegs.GPADAT.bit.GPIO17 = 1;

	for(i=0;i<16;i++)
	{
		GpioDataRegs.GPADAT.bit.GPIO8 = d & 0x0001;		//place data on pin
		GpioDataRegs.GPADAT.bit.GPIO18 = 1;				//clk high
		DELAY_US(1);									//pause
		GpioDataRegs.GPADAT.bit.GPIO18 = 0;				//clk low
		DELAY_US(1);									//pause
		d = d >> 1;										//shift data
	}
}

int GetMenuSelection(const unsigned char List[][20])
{
	//these defines determine the size of the menu. NLINES is the number of line the menu consists of.
#define NLINES 4

	int highlight,offset,i,max;

	//start at the beginning of the menu
	highlight = 0;
	offset = 0;
	//find the end of the menu, marked by an empty string. max = number of menu entries
	max = 0;
	while(List[max][0]!=0) max++;

	//set the font to small(default font)
	set_font(Font);
	//clear the screen
	clear_screen(0);
	//print NLINES lines of the menu starting at line offset, with highlight inverted
	set_cursor(0,0);
	for(i=0;i<NLINES;i++)
	{
		print_cstr((const uint8_t*)&List[i+offset][0],(i==highlight));		//print line
		clear_to_end();										//clear the rest of the line (entries may not all be the same length)
		print_char(0x0D,0);									//CR
		print_char(0x0A,0);									//LF
	}

	while(1)
		switch(GetButtonPress())
		{
		case BTN_UP:
				if(--highlight == -1)			//decrement highlighted line, if it goes off the top,
					if(--offset == -1)			//decrement the list offset, if it goes off the top
					{
						offset = max-NLINES;	//offset to the end
						highlight = NLINES-1;	//highlight the last row
					}
					else
					{
						highlight = 0;			//keep highlight at the top
					}
				//redraw screen
				set_cursor(0,0);
				for(i=0;i<NLINES;i++)
				{
					print_cstr((const uint8_t*)&List[i+offset][0],(i==highlight));		//print line
					clear_to_end();										//clear the rest of the line (entries may not all be the same length)
					print_char(0x0D,0);									//CR
					print_char(0x0A,0);									//LF
				}
			break;
		case BTN_DOWN:
				if(++highlight == NLINES)			//increment highlighted line, if it goes off the bottom,
					if(++offset == max)				//increment the list offset, if it goes off the bottom,
					{
						offset = 0;					//offset to the beginning
						highlight = 0;				//highlight the first row
					}
					else
					{
						highlight = NLINES-1;		//keep highlight at the bottom
					}
				//redraw screen
				set_cursor(0,0);
				for(i=0;i<NLINES;i++)
				{
					print_cstr((const uint8_t*)&List[i+offset][0],(i==highlight));		//print line
					clear_to_end();										//clear the rest of the line (entries may not all be the same length)
					print_char(0x0D,0);									//CR
					print_char(0x0A,0);									//LF
				}
			break;
		case BTN_SELECT:
				return (highlight+offset);
			break;
		case BTN_BACK:
		case BTN_MENU:
				return -1;
			break;
		default:
			break;
		}
}

unsigned int GetButtonPress()
{
	unsigned int tmp;
	//this function returns 0 if there are no button presses in the queue, otherwise it returns the code for the button pressed
	tmp = 0;																		//assume no press
	if(ButtonPress.Empty == 0)														//if there are presses to get
	{
		tmp = ButtonPress.Queue[ButtonPress.Current];								//get button press
		if(++ButtonPress.Current == BUTTON_QUEUE_SIZE) ButtonPress.Current = 0;		//increment with wrap
		if(ButtonPress.Next == ButtonPress.Current) ButtonPress.Empty = 1;			//flag empty if necessary
		ButtonPress.Full = 0;														//just removed a press, cannot be full
	}

	return tmp;
}

void LEDGpio_init()
{
	//mapping:
	//LED shift clock = GPIO18
	//LED shift data = GPIO8
	//LED shift clear = GPIO17
	//LED word [0:15] = Btn0[0],Btn4[0],Btn3[1],Btn2[1],Btn1[0],Btn4[1],Btn0[1],Btn1[1],Btn2[0],Yellow2,Green2,Red2,Btn3[0],Red1,Green1,Yellow1

	EALLOW;
		//LEDclk
	    GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO18 = 1;          // output
	    GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO18 = 1;          //disable pull up
	    //LEDdata
	    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO8 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO8 = 1;          //disable pull up
	    //LEDclear
	    GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO17 = 1;          // output
	    GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO17 = 1;          //disable pull up
	EDIS;
}

void ButtonGpioInit()
{
	//mapping:
	//Btn3 (leftmost)	= GPIO33
	//Btn2				= GPIO32
	//Btn1				= GPIO22
	//Btn0				= GPIO24
	//Btn4 (rightmost)	= GPIO21	(also MCN button)
	EALLOW;
	GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 0;         // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO33 = 0;          // input
    GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 0;        //Synch to SYSCLKOUT only
    GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;          //enable pull up

    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;         // GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO32 = 0;          // input
    GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 0;        //Synch to SYSCLKOUT only
    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;          //enable pull up

	GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 0;          // input
    GpioCtrlRegs.GPAQSEL2.bit.GPIO22 = 0;        //Synch to SYSCLKOUT only
    GpioCtrlRegs.GPAPUD.bit.GPIO22 = 0;          //enable pull up

	GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO24 = 0;          // input
    GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = 0;        //Synch to SYSCLKOUT only
    GpioCtrlRegs.GPAPUD.bit.GPIO24 = 0;          //enable pull up

	GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;         // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 0;          // input
    GpioCtrlRegs.GPAQSEL2.bit.GPIO21 = 0;        //Synch to SYSCLKOUT only
    GpioCtrlRegs.GPAPUD.bit.GPIO21 = 0;          //enable pull up
    EDIS;
}

void LCDGpio_init()	//this function does one-time setup on LCD GPIO pins
{
	EALLOW;
		//Data port
		//D0
	    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO20 = 1;          // output
	    GpioCtrlRegs.GPAQSEL2.bit.GPIO20 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO20 = 1;          //disable pull up
	    //D1
	    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO14 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO14 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO14 = 1;          //disable pull up
	    //D2
	    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO1 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;          //disable pull up
	    //D3
	    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO2 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;          //disable pull up
	    //D4
	    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO3 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;          //disable pull up
	    //D5
	    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO10 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO10 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;          //disable pull up
	    //D6
	    GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0;         // GPIO
	    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 1;          // output
	    GpioCtrlRegs.GPBQSEL1.bit.GPIO40 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPBPUD.bit.GPIO40 = 1;          //disable pull up
	    //D7
	    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO4 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO4 = 1;          //disable pull up

	    //command port
	    //CS2
	    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO5 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO5 = 1;          //disable pull up
	    //CS1
	    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO11 = 1;          //disable pull up
	    //Reset
	    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO6 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO6 = 1;          //disable pull up
	    //RW
	    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO7 = 1;          //disable pull up
	    //DI
	    GpioCtrlRegs.GPBMUX1.bit.GPIO41 = 0;         // GPIO
	    GpioCtrlRegs.GPBDIR.bit.GPIO41 = 1;          // output
	    GpioCtrlRegs.GPBQSEL1.bit.GPIO41 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPBPUD.bit.GPIO41 = 1;          //disable pull up
	    //E
	    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;          // output
	    GpioCtrlRegs.GPAQSEL1.bit.GPIO12 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO12 = 1;          //disable pull up

	    //backlight control
	    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 0;         // GPIO
	    GpioCtrlRegs.GPADIR.bit.GPIO16 = 1;          // output
	    GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = 0;        //Synch to SYSCLKOUT only
	    GpioCtrlRegs.GPAPUD.bit.GPIO16 = 1;          //disable pull up
	    EDIS;
}

void WriteLCDDataPort(uint8_t Data)
{
	//mapping:
	//D0 = GPIO20
	//D1 = GPIO14
	//D2 = GPIO1
	//D3 = GPIO2
	//D4 = GPIO3
	//D5 = GPIO10
	//D6 = GPIO40
	//D7 = GPIO4

	//first set all the GPIO used for LCD data to outputs
    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO20 = 1;          // output
    GpioCtrlRegs.GPADIR.bit.GPIO14 = 1;          // output
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;          // output
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;          // output
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;          // output
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 1;          // output
    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 1;          // output
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;          // output
    EDIS;

    //now write the data to the pins
    GpioDataRegs.GPADAT.bit.GPIO20 = Data & 0x0001;
    GpioDataRegs.GPADAT.bit.GPIO14 = Data & 0x0002;
    GpioDataRegs.GPADAT.bit.GPIO1 = Data & 0x0004;
    GpioDataRegs.GPADAT.bit.GPIO2 = Data & 0x0008;
    GpioDataRegs.GPADAT.bit.GPIO3 = Data & 0x00010;
    GpioDataRegs.GPADAT.bit.GPIO10 = Data & 0x0020;
    GpioDataRegs.GPBDAT.bit.GPIO40 = Data & 0x0040;
    GpioDataRegs.GPADAT.bit.GPIO4 = Data & 0x0080;
}

uint8_t ReadLCDDataPort()
{
	int temp;

	//first set all the GPIO used for LCD data to inputs
    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO20 = 0;          //input
    GpioCtrlRegs.GPADIR.bit.GPIO14 = 0;          //input
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 0;          //input
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 0;          //input
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 0;          //input
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 0;          //input
    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 0;          //input
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 0;          //input
    EDIS;

    temp = 0;
    temp |= ((GpioDataRegs.GPADAT.bit.GPIO4 << 7) | (GpioDataRegs.GPBDAT.bit.GPIO40 << 6) | (GpioDataRegs.GPADAT.bit.GPIO10 << 5) | (GpioDataRegs.GPADAT.bit.GPIO3 << 4));
    temp |= ((GpioDataRegs.GPADAT.bit.GPIO2 << 3) | (GpioDataRegs.GPADAT.bit.GPIO1 << 2) | (GpioDataRegs.GPADAT.bit.GPIO14 << 1) | (GpioDataRegs.GPADAT.bit.GPIO20));
	return temp;
}

void SetLCDControlPort(uint8_t Cmd)
{
	//CS2	bit 7 of Cmd
	//CS1	bit 6 of Cmd
	//RESET	bit 5 of Cmd
	//RW	bit 4 of Cmd
	//DI	bit 3 of Cmd

	//mapping:
	//CS2 = GPIO5
	//CS1 = GPIO11
	//RESET = GPIO6
	//RW = GPIO7
	//DI = GPIO41

    GpioDataRegs.GPADAT.bit.GPIO5 = Cmd & 0x0080;
    GpioDataRegs.GPADAT.bit.GPIO11 = Cmd & 0x0040;
    GpioDataRegs.GPADAT.bit.GPIO6 = Cmd & 0x0020;
    GpioDataRegs.GPADAT.bit.GPIO7 = Cmd & 0x0010;
    GpioDataRegs.GPBDAT.bit.GPIO41 = Cmd & 0x00008;

}

void SetLCDEN(int s)
{
	//mapping:
	//EN = GPIO12
	if (s) GpioDataRegs.GPADAT.bit.GPIO12 = 1; else GpioDataRegs.GPADAT.bit.GPIO12 = 0;
}

void LCDdelay()
{
	DELAY_US(1);
}

void delay_ms(uint16_t ms)
{
	int i;
	for(i=0;i<ms;i++)
		DELAY_US(1000);
}
