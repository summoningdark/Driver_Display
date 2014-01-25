/*
Copyright (c) 2012 Jennifer Holt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

//C2000 doesn't have an 8-bit type, just use the unsigned 16-bit int
#define uint8_t unsigned int
#define uint16_t unsigned int
#define int8_t int
#define int16_t int

#define NULL 0

//User must provide the following external functions:

extern void WriteLCDDataPort(uint8_t Data);	//this function should set whatever pins are used for the LCD data port to outputs, and put Data on the pins
extern uint8_t ReadLCDDataPort();		//this function should set whatever pins are used for the LCD data port to inputs, and return the value on the pins
extern void SetLCDEN(int s);			//this function should set whatever pin is used for LCD EN high or low depending on s !=0 or s=0
extern void LCDdelay();				//a delay of at least 500ns
extern void delay_ms(uint16_t ms);		//function to delay ms milliseconds
extern void SetLCDControlPort(uint8_t Cmd);	//this function should set whatever pins are used for the LCD control pins to outputs and set the values as follows:
//CS2	bit 7 of Cmd
//CS1	bit 6 of Cmd
//RESET	bit 5 of Cmd
//RW	bit 4 of Cmd
//DI	bit 3 of Cmd
//note: SetLCDControlPort() should not alter whatever pin is used for EN, that is taken care of with SetLCDEN()

//LCD control port states
// note PORTD is CS2 CS1 RESET RW   DI x x x
#define LCD_COMMAND2 0x60       //cs1,reset
#define LCD_COMMAND1 0xA0       //cs2,reset
#define LCD_COMMAND12 0xE0      //cs1,cs2,reset
#define LCD_DATA2 0x68          //cs1,reset,DI
#define LCD_DATA1 0xA8          //cs2,reset,DI
#define LCD_DATA12 0xE8         //cs1,cs2,reset,DI
#define LCD_READ2 0x78          //cs1,reset,rw,DI
#define LCD_READ1 0xB8          //cs2,reset,rw,DI
#define LCD_STATUS2 0x70        //cs1,reset,rw
#define LCD_STATUS1 0xB0        //cs2,reset,rw
#define LCD_IDLE 0x20           //reset


//macros for common LCD commands
#define CLK_LCD SetLCDEN(1);LCDdelay();SetLCDEN(0);LCDdelay()
#define set_x(x) write_command_LCD((0x40 | x), 0)
#define set_x_side(x,y) write_command_LCD((0x40 | x),y)
#define set_page(y) write_command_LCD((0xB8 | y),0)
#define display_on() write_command_LCD(0x3F,0)

//Define functions
//======================

//LCD interface functions
void LCD_reset(void);
void write_command_LCD(uint8_t byte, uint8_t chip);
void write_data_LCD(uint8_t byte, uint8_t chip);
uint8_t read_data_LCD(uint8_t chip);
uint8_t read_byte(uint8_t byte, uint8_t side);
void read_block(uint8_t x, uint8_t page, uint8_t length, uint8_t* buf);
void write_block(uint8_t x, uint8_t page, uint8_t length, uint8_t* buf);

//graphics functions
void clear_screen(uint8_t option);
void set_font(const uint8_t *pNewFont);
void set_cursor(uint8_t x, uint8_t y);
void print_char(uint8_t txt, int8_t inv, uint8_t reduced);
void print_cstr(const char* str, int8_t inv, uint8_t reduced);
void print_rstr(char* str, int8_t inv, uint8_t reduced);
void clear_to_end();
void del_char(void);
void pixel(uint8_t S_R, uint8_t x, uint8_t y);
void line(uint8_t S_R, int8_t x1, int8_t y1, int8_t x2, int8_t y2);
void circle(uint8_t S_R, int8_t x, int8_t y, int8_t r);
void draw_block(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t data);
void box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void status_bar(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t percent, uint8_t direction);
void bitblt(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t mode, uint8_t* data);
void draw_sprite(uint8_t x, uint8_t y, uint8_t n, uint8_t mode);

//======================

//graphics variables
uint8_t reverse = 0;
uint8_t splash_screen = 1;
uint8_t buffer[256];			//buffer for image data in bitblt routines

//sprite stuff
#include "sprites.h" 			//sprites for building the user display
// sprites are: 
// 0	Battery icon
// 1	motor icon
// 3	disk icon
// 4	serial_disconnect_icon

#include "splash.h"		//splash screen

//font stuff

//font drawing variables
const uint8_t *pFont;		//pointer to font data, useful for switching fonts dynamically
uint8_t x_offset = 0;	//upper left corner of character, x-coord
uint8_t y_offset = 0;	//upper left corner of character, y-coord
uint8_t font_mode=7;	//how font interacts with background. (bitblt option) 7 = overwrite and is default(no need to erase the background first)
uint8_t CR_LF = 1;	//should CR have an automatic LF
uint8_t font_bytes;	//# of bytes in a character (5 for default font)
uint8_t font_w;		//width of a character in pixels (5 for default font)
uint8_t font_h;		//height of a character in pixels (8 for default font)
uint8_t font_sw;	//font short width for punctuation
uint8_t font_space;	//horizontal space to leave between characters
uint8_t aux_font=0;	//which font to use 0=default, 1=aux


//this include file stores all the relevant font data.
#include "font.h"
#include "aux_font.h"
#include "R24x40_font.h"

//===============================================================these functions are updated=========================================================================
void status_bar(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t percent, uint8_t direction)
{
	//This function draws a box with corners (x1,y1) and (x2,y2) and fills percent of it in from the top, right, left, bottom for direction = 0, 1, 2, 3

	//check for reasonable values
	if(x1<128 && x2<128 && y1<64 && y2<64 && percent <101)
	{
		uint8_t f,t;
		uint16_t stop;
		//make sure x1,y1 is upper left corner
		if(x2 < x1)
		{
			t = x1;
			x1 = x2;
			x2 = t;
		}

		if(y2<y2)
		{
			t = y1;
			y1 = y2;
			y2 = t;
		}
		//draw box outline
		box(x1,y1,x2,y2);
		if (reverse) f=0x00; else f=0xFF;		//fill byte
		switch (direction)
		{
		case 0:	//fill from top
			 stop = (y2-y1)*percent;
			 stop/=100;
			 draw_block(x1,y1,x2,y1+(uint8_t)stop,f);
		break;
		case 1: //fill from right
			 stop = (x2-x1)*percent;
			 stop/=100;
			 draw_block(x2-(uint8_t)stop,y1,x2,y2,f);
			 break;
		case 2: //fill from left
			 stop = (x2-x1)*percent;
			 stop/=100;
			 draw_block(x1,y1,x1+(uint8_t)stop,y2,f);
		break;
		default: //fill from bottom
			stop = (y2-y1)*percent;
			stop/=100;
			draw_block(x1,y2-(uint8_t)stop,x2,y2,f);
		}
	}
}
void LCDSplash(uint16_t ms)
{
	uint16_t i,A;
	unsigned int temp_buf[128];

	//display splash screen for ms milliseconds
	
	//splash screen in program memory and is 128x64, since it is page aligned, we can write directly to the LCD
	for(A=0;A<8;A++)	//loop for each page
	{
		//copy program memory into ram buffer
		for(i=0;i<128;i++)
			temp_buf[i] = Splash[A*128+i];
		write_block(0,A,128,temp_buf);	//write data to screen
	}
	
	delay_ms(ms);		//display the splash screen for 2 seconds
	clear_screen(0);	//clear the screen
}


void LCDinit (void)
{	
	//Set up the default font===========================================
	pFont = &Font[0];
	font_w = *pFont;		//get font width from font array	
	font_h = *pFont + 1;		//get font height from font array
	font_space = *pFont + 2;	//get font space from font array
	font_bytes = font_h/8;		//8 pixels/byte
	if (font_h % 8 != 0)
		font_bytes++;		//partial rows count too
	font_bytes *= font_w;		//need font_w stacks of rows	

	//Reset the display=================================================
	LCD_reset();
}

void clear_screen(uint8_t option)
{
	uint8_t x, y;

	if (option==0)	//actual erase of screen
	{		
		
		for (x = 0; x < 8; x++)
		{
			//set x address (page)
			set_page(x);
			
			//Set y address to zero
			set_x(0x00);
			
			//y address increments after every write
			//write data
					
			if (reverse == 1) WriteLCDDataPort(0xFF);	//since data never changes when clearing the screen, set it once and do a bunch of clocks
			else WriteLCDDataPort(0);

			SetLCDControlPort(LCD_DATA12);		//write data to both chips
			for (y = 0; y < 64; y++)		//clock for all 64 horizontal positions
			{
				LCDdelay();
				CLK_LCD;
			}
			SetLCDControlPort(LCD_IDLE);

			x_offset = 0;

			y_offset = 0;
		}//end page loop normal clear
	}//end normal clear
	else	//reverse mode switch, do a logical inversion of the screen
	{
		if (reverse==1) reverse = 0; else reverse = 1;	//toggle reverse
		for(y=0;y<8;y++)	//loop for each page
		{
			read_block(0,y,128,buffer);	//read in display data
			for (x=0;x<128;x++)
				buffer[x]=~buffer[x];	//invert the data
			write_block(0,y,128,buffer);	//write data back to screen
		}
	}//end if(option)
}

void set_cursor(uint8_t x, uint8_t y)
{
	x_offset = x;
	y_offset = y;
	//coerce values to make sure a char fits in the space
	if ((x_offset + font_w + font_space) > 127) x_offset = 127 - (font_w + font_space);
	if (y_offset > (64-font_h)) y_offset = (64-font_h);

}

void set_font(const uint8_t *pNewFont)
{
	if (pNewFont==NULL)			//default font
	{
		pFont = &Font[0];		//get pointer to default font
		font_w = *pFont;		//get font width from font array	
		font_h = *(pFont + 1);		//get font height from font array
		font_sw = *(pFont + 2);		//get font short width ( for ! ' , . : ; ` characters)
		font_space = *(pFont + 3);	//get font space from font array
		font_bytes = font_h/8;		//8 pixels/byte
		if (font_h % 8 != 0)
			font_bytes++;		//partial rows count too
		font_bytes *= font_w;		//need font_w stacks of rows	
	}
	else			//new font
	{
		pFont = pNewFont;		//get pointer to new font
		font_w = *pFont;		//get font width from font array	
		font_h = *(pFont + 1);		//get font height from font array
		font_sw = *(pFont + 2);		//get font short width ( for ! ' , . : ; ` characters)
		font_space = *(pFont + 3);	//get font space from font array
		font_bytes = font_h/8;		//8 pixels/byte
		if (font_h % 8 != 0)
			font_bytes++;		//partial rows count too
		font_bytes *= font_w;		//need font_w stacks of rows	
	}
}

//prints a const string. iv inv is 0 prints normally, if inv is 1, prints inverted
void print_cstr(const char* str, int8_t inv, uint8_t reduced)
{
	uint8_t ch,i;
	i=0;
	while((ch=str[i++])!=0) print_char(ch, inv,reduced);
}

//prints a string. iv inv is 0 prints normally, if inv is 1, prints inverted
void print_rstr(char* str, int8_t inv, uint8_t reduced)
{
	uint8_t ch,i;
	i=0;
	while((ch=str[i++])!=0) print_char(ch, inv,reduced);
}

//prints a character to the screen, if inv !=0 prints char inverted
//at x_offset, y_offset(top/left corner of character). Automatically augments offsets for next write
void print_char(uint8_t txt, int8_t inv, uint8_t reduced)
{

    // x_offset counts pixels from the left side of the screen
    // y_offset counts pixels from the top of the screen

	int16_t text_array_offset, j;
	uint8_t f;

	unsigned int temp_buf[128];

	if(txt == 0x0D)	//check for CR
	{
		x_offset = 0;		//reset to start of line
	}
	else if (txt == 0x0A)	//check for LF
	{
		if (y_offset > (64-font_h-font_h))
			y_offset = y_offset % font_h;		//this makes sure that the line restarted at the top will overlap the old one
		else 
			y_offset += font_h;
	}
	else
	{


		if(reduced)	//see if we are using a reduced font (numerals only)
		{
			//coerce txt to valid printable character
			//must treat '+' '-' '.' separately
			if(txt == 43)	// '+'
			{
				text_array_offset = 10 * font_bytes + 4;
			}
			else if (txt == 45) // '-'
			{
				text_array_offset = 11 * font_bytes + 4;
			}
			else if (txt == 46) // '.'
			{
				text_array_offset = 12 * font_bytes + 4;
			}
			else
			{
				if ((txt<48) || (txt>57)) txt = 48;				// these are the numerals 0-9
				text_array_offset = (txt - 48) * font_bytes+4;	// txt-48 is the ascii offset to '0', font_bytes is the # of bytes/character, and 3 for font width,height,short width,space which are stores at the beginning of the array
			}
		}
		else
		{
			//coerce txt to valid printable character
			if ((txt<32) || (txt>126)) txt = 32;
			text_array_offset = (txt - 32) * font_bytes+4;	// txt-32 is the ascii offset to 'space', font_bytes is the # of bytes/character, and 3 for font width,height,short width,space which are stores at the beginning of the array
		}

		//fetch font data from program memory
		for(j=0;j<font_bytes;j++)
		{
			temp_buf[j] = *(pFont+text_array_offset+j);
			if (inv) temp_buf[j] = ~temp_buf[j] & 0x00FF;
		}

		//bitblt it
		//check for half-width characters ( ! ' , . : ; ` ) and adjust x_offset accordingly
		if ((txt == 33) ||(txt == 39) ||(txt == 44) ||(txt == 46) ||(txt == 58) ||(txt == 59) ||(txt == 96) )
		{
			bitblt(x_offset, y_offset, font_w, font_h, font_mode, temp_buf);
			x_offset+=font_sw;
			f=0x00;
			if (reverse==1)
			{
				f=0xFF;
			}
			if (inv)
			{
				f = ~f;
			}
			draw_block(x_offset, y_offset, x_offset+font_space, y_offset+font_h-1,f);	//erase the block
			x_offset +=	font_space;
		}
		else
		{
			bitblt(x_offset, y_offset, font_w, font_h, font_mode, temp_buf);
			x_offset+=font_w;
				f=0x00;
				if (reverse==1)
				{
					f=0xFF;
				}
				if (inv)
				{
					f = ~f;
				}
				draw_block(x_offset, y_offset, x_offset+font_space, y_offset+font_h-1,f);	//erase the block

			x_offset+=font_space;
		}
		//check x offset and do necessary wrapping

		if ((x_offset + font_w + font_space) > 127)
		{
			x_offset = x_offset % (font_w+font_space);	//this makes sure text on the next line will line up with the previous line
			if (y_offset > (64-font_h-font_h))
				y_offset = y_offset % font_h;		//this makes sure that the line restarted at the top will overlap the old one
			else
				y_offset += font_h;
		}
	}//end CR/LF/printable if
}

//write_command_LCD
void write_command_LCD(uint8_t byte, uint8_t chip)
{
	//byte is the command to write, chip determines which chips get written to 1=1,2=2, anything else=both

	if (chip==1) 
		SetLCDControlPort(LCD_COMMAND1);
	else if (chip == 2)
		SetLCDControlPort(LCD_COMMAND2);
	     else
		SetLCDControlPort(LCD_COMMAND12);
 	
	WriteLCDDataPort(byte);
	LCDdelay();
	CLK_LCD;

	SetLCDControlPort(LCD_IDLE);	
	LCDdelay();
}

//write_data_LCD
void write_data_LCD(uint8_t byte, uint8_t chip)
{
	//byte is the data to write, chip determines which chips get written to 1=1,2=2 anything else=both

	if (chip==1) 
		SetLCDControlPort(LCD_DATA1);
	else if (chip == 2)
		SetLCDControlPort(LCD_DATA2);
	     else
		SetLCDControlPort(LCD_DATA12);

	WriteLCDDataPort(byte);
	LCDdelay();

	CLK_LCD;
	SetLCDControlPort(LCD_IDLE);	
	LCDdelay();
	
}


//this writes a block of contiguous bytes to a single page
//automatically takes care of crossing from one chip to the next
void write_block(uint8_t x, uint8_t page, uint8_t length, uint8_t* buf)
{
	uint8_t side;
	uint8_t i, s, n1, n2, o;

	side=1;
	o=0;			//offset in data
	set_page(page);		//set proper page
	if (x>63)		//account for x being larger than 63 
	{
		x-= 64;
		side=2;
	}

	s = (64-x);		//distance to edge

	if (length>s)		
	{
		n1=s;		//how much to read in the first loop
		n2=length-s;	//how much to read in the second loop
	}
	else
	{
		n1=length;
		n2=0;
	}

	set_x(x);

	//write the data
	if  (side==1) 					//select proper chip
		SetLCDControlPort(LCD_DATA1);	
	else
		SetLCDControlPort(LCD_DATA2);


	for(i=0;i<n1;i++)
	{
		WriteLCDDataPort(buf[o++]);		//put data on bus
		LCDdelay();
		CLK_LCD;				//clock it in
	}

	SetLCDControlPort(LCD_IDLE);	
	LCDdelay();

	if(n2>0)
	{
		set_x(0);

		if  (side==1) 				//select proper chip(opposite of before, since we ran over)
			SetLCDControlPort(LCD_DATA2);	
		else
			SetLCDControlPort(LCD_DATA1);


		for(i=0;i<n2;i++)
		{
			WriteLCDDataPort(buf[o++]);		//put data on bus
			LCDdelay();
			CLK_LCD;				//clock it in
		}
	
		SetLCDControlPort(LCD_IDLE);	
		LCDdelay();

	}//if n2>0
}

//read_data_LCD
//this incorporates a dummy read, which is necessary for reading after changing the LCD command port, but messes up successive reads if set_x is not called.
//for reading a block of continuous data, use read_block
uint8_t read_data_LCD(uint8_t chip)
{
	uint8_t data1;
	//chip determines which chip to read from
		
	if  (chip==1) 				//select proper chip
		SetLCDControlPort(LCD_READ1);	
	else
		SetLCDControlPort(LCD_READ2);
	
	CLK_LCD;				//dummy clk
	

	SetLCDEN(1);
	LCDdelay();

	data1 = ReadLCDDataPort();				//read data
	
	SetLCDEN(0);
	LCDdelay();

	SetLCDControlPort(LCD_IDLE);	
	LCDdelay();

	return data1;
}

//reads [length] display bytes from page [page] starting at horizontal value [x] and puts the values in buf
//automatically accounts for crossing chips 
void read_block(uint8_t x, uint8_t page, uint8_t length, uint8_t* buf)
{
	uint8_t side;
	uint8_t c,s, n1, n2, o;

	side=1;
	o=0;				//offset in data
	set_page(page);			//set proper page
	if (x>63)			//account for x being larger than 63 
	{
		x-= 64;
		side=2;
	}

	s = (64-x);			//distance to edge

	if (length>s)		
	{
		n1=s;			//how much to read in the first loop
		n2=length-s;		//how much to read in the second loop
	}
	else
	{
		n1=length;
		n2=0;
	}

	set_x(x);			//set x	
				
	if  (side==1) 			//select proper chip
		SetLCDControlPort(LCD_READ1);	
	else
		SetLCDControlPort(LCD_READ2);

	CLK_LCD;			//dummy clk
	

	for(c=0;c < n1;c++)		//loop for all the bytes	
		{
			SetLCDEN(1);
			LCDdelay();

			buf[o++] = ReadLCDDataPort(); //read data

			SetLCDEN(0);
			LCDdelay();

		}

	SetLCDControlPort(LCD_IDLE);	
	LCDdelay();	

	if (n2>0)
	{	
		set_x(0);			// we got here because we overran the previous side. always start @ 0
		if  (side==1) 			//select proper chip, note this is opposite of normal because this read catches overrun from previous side
			SetLCDControlPort(LCD_READ2);	
		else
			SetLCDControlPort(LCD_READ1);
		
		CLK_LCD;			//dummy clk
		

		for(c=0;c < n2;c++)		//loop for all the bytes	
			{
				SetLCDEN(1);
				LCDdelay();

				buf[o++] = ReadLCDDataPort();	//read data

				SetLCDEN(0);
				LCDdelay();

			}

		SetLCDControlPort(LCD_IDLE);	
		LCDdelay();	
	}//if n2>0
}


//mapping to Cartesian coordinates, (0,0) is in the lower left corner, (127,63) is in the upper right
void pixel(uint8_t S_R, uint8_t x, uint8_t y)
{
	static uint8_t temp_page, temp_side, temp_x = 0, temp_data1 = 0, temp_data2 = 0;
	
	//don't try to print something outside of our range
	if (x > 127) return;
	if (y > 63) return;
	
	y=63-y;	

	if (reverse == 1) S_R ^= 1;
	
	if (x >= 64) temp_side = 2, temp_x = x - 64;
	else temp_side = 1, temp_x = x;
		
	temp_page = 7 - (y >> 3);
		
	//data = (1 << (y - ((7 - temp_page) * 8)));

	temp_data1 = (1 << (7 - (y - ((7 - temp_page) * 8))));
		
	set_page(temp_page);
				
	//need to read the existing byte here, then or it with the new byte
	temp_data2 = read_byte(temp_x, temp_side);
		
	if (S_R == 0)
	{
		temp_data1 = ~temp_data1;
		temp_data1 &= temp_data2;
	}
	else temp_data1 |= temp_data2;
		
	set_x(temp_x);//reset this...
	write_data_LCD(temp_data1, temp_side);
}


//draws (S_R = 1) or erases (S_R = 0) a line from x0, y0 to x1, y1 using Bresenham's line algorithm.
void line(uint8_t S_R, int8_t x0, int8_t y0, int8_t x1, int8_t y1)
{
	
	uint8_t steep;
	uint8_t swp;

    steep = (abs(y1 - y0) > abs(x1 - x0));
    if (steep) 
	{
		swp=x0;
		x0=y0;
		y0=swp;
		swp=x1;
		x1=y1;
		y1=swp;
	}

    if (x0 > x1) 
	{
       		swp=x0;
		x0=x1;
		x1=swp;
		swp=y0;
		y0=y1;
		y1=swp;
	}

	int deltax = x1 - x0;
	int deltay = abs(y1 - y0);
	int error = deltax / 2;
	int ystep;
	int y = y0;
	int x;

        if (y0 < y1) ystep = 1; else ystep = -1;
        for (x = x0; x <= x1; ++x)
	{
            if (steep) pixel(S_R,y,x); else pixel(S_R,x,y);

            error = error - deltay;
		if (error < 0) 
		{
			y = y + ystep;
			error = error + deltax;
		}
        }
}


uint8_t read_byte(uint8_t byte, uint8_t side)
{
	set_x(byte);
	return read_data_LCD(side);
}


//draws (S_R = 1) or erases (S_R = 0) a circle at x, y with radius r, using midpoint circle algorithm
void circle(uint8_t S_R, int8_t x0, int8_t y0, int8_t r)
{
	
	int8_t f = 1 - r;
	int8_t ddF_x = 1;
	int8_t ddF_y = -2 * r;
	int8_t x = 0;
	int8_t y = r;
 
  pixel(S_R, x0, y0 + r);
  pixel(S_R, x0, y0 - r);
  pixel(S_R, x0 + r, y0);
  pixel(S_R, x0 - r, y0);
 
  while(x < y)
  {
    if(f >= 0) 
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;    
    pixel(S_R, x0 + x, y0 + y);
    pixel(S_R, x0 - x, y0 + y);
    pixel(S_R, x0 + x, y0 - y);
    pixel(S_R, x0 - x, y0 - y);
    pixel(S_R, x0 + y, y0 + x);
    pixel(S_R, x0 - y, y0 + x);
    pixel(S_R, x0 + y, y0 - x);
    pixel(S_R, x0 - y, y0 - x);
  }
}


//Deletes a full character space previous to the current location (backspace)
void del_char()
{
	uint8_t f;

	if (x_offset <= font_w)					//if previous char wouldn't have fit
	{			
		x_offset = (128 - (font_w+1) - ((128-x_offset) % (font_w+1)) );			
				
		if (y_offset < font_h) 	
			y_offset = (64 - font_h - ((64-y_offset) % font_h) );	//if we run off the top of the screen
		else
			y_offset-=font_h;
	}
	
	else x_offset -= (font_w+1);					// back x_offset up by the font width + 1 pixel space btwn characters
	
	f=0;
	if (reverse==1)
		f=0xff;	

	draw_block(x_offset, y_offset, x_offset+font_w, y_offset+font_h-1,f);	//erase the block
}

void clear_to_end()
{
	uint8_t f;
	f=0;
	if (reverse==1)
		f=0xff;
	draw_block(x_offset, y_offset, 127, y_offset+font_h-1,f);	//erase the block
}

//draws a block on the screen. Block is described
//by a diagonal line from x, y1 to x2, y2
//block is filled with byte data (describes a vertical row of 8 pixels, use 0x00 to clear the block, 0xFF to fill it, etc)
void draw_block(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t data)
{
	static int16_t width = 0, height = 0,x,y;
	uint8_t n;
	n=data;

	//coerce values to be in range
	if (x1 > 127) x1 = 127;
	if (x2 > 127) x2 = 127;
	if (y1 > 127) y1 = 127;
	if (y2 > 127) y2 = 127;

	if (x1>x2)
	{
		width=x1-x2;
		x=x2;
	}
	else
	{
		width=x2-x1;
		x=x1;
	}
	if (y1>y2)
	{
		height=y1-y2;
		y=y2;
	}	
	else
	{
		height=y2-y1;
		y=y1;
	}
	
	bitblt(x, y, width, height, 6, &n);	//use erase mode of bitblt

}

//draws a box. The box is described

//by a diagonal line from x, y1 to x2, y2
void box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	
	line(1, x2, y2, x2, y1);
	line(1, x1, y2, x2, y2);
	line(1, x1, y2, x1, y1);
	line(1, x1, y1, x2, y1);
}

void draw_sprite(uint8_t x, uint8_t y, uint8_t n, uint8_t mode)	//draws nth sprite at (x,y) using mode
{
	uint16_t o;
	o=n*SPRITE_SIZE;				//offset to sprite
	bitblt(x, y, sprite[o], sprite[o+1], mode, sprite + 2 + o);
}

void LCD_reset(void)
{
	SetLCDControlPort(0x00);
	delay_ms(60);
	SetLCDControlPort(LCD_IDLE);
	delay_ms(60);

	clear_screen(0);

	display_on();
	
	set_page(0);
			
	set_x(0);
			
	//set display start line to 0, command 0xC0
	write_command_LCD(0xC0,0);
	
	x_offset = 0;

	set_page(0);
}

//does a bit transfer from data to display memory
//bitblt will not return until it gets all the bytes it wants
//x,y is upper left corner of image in pixels
//bitblt counts coordinates in the standard fashion. ie (0,0) is upper left, +x it to the right +y is down
//width is width in pixels
//height is height in pixels
//mode determines how the bits in the image combine with the bits already present on the display 
//mode 0=AND, 1=NAND, 2=OR, 3=NOR, 4=XOR, 5=NXOR, 6=fill(used for block erase, data[0] sets fill byte), 7=copy(overwrites background)
void bitblt(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t mode, uint8_t* data)
{
	uint8_t row, column, shift, shift2, mask1, mask2, n, n2, temp;
	uint16_t offset, offset2;

	shift = y % 8;				//calculate how much to shift the data bytes to line them up with the pages
	shift2 = 8 - shift;
	n = ((height-1+shift)/8)+1;		//number of pages(rows) the image occupies(need to loop through all of these, each gets pixels changed)
	n2 = height/8;				//number of rows in image
	if (height % 8 != 0)
		n2++;
	
	mask1 =	0xFF << shift;			//mask1 needs to have 0's for each pixel in the top row NOT occupied by new image data	
	mask2 = 0xFF >> (8-( (height+y) % 8) );	//mask2 needs to have 0's for each pixel in the bottom row NOT occupied by new image data
	
	if ( ((height+y) % 8) == 0)
		mask2=0xff;			//special case of exact fit in last row
		
		offset=0;			//start at the beginning of data
		offset2=-width;			//offset2 points a the previous row

	for(row = 0;row < n;row++)		//loop through all rows
	{
		//it is necessary to have 2 rows of data, current and previous to do bitblt
		//since 0<width<128 (display is only 128 wide), we can use the second 128 bytes in buffer to hold the previous row

		read_block(x,(row+(y/8)),width,buffer);				//read the row in(background image data)

			for(column=0;column<width;column++)			//loop for columns
			{

				//Get data
				temp = ( (data[offset++] << shift) | (data[offset2++] >> shift2) );	//data from ram
				if(reverse) temp = ~temp;											//invert the data if reverse mode
				
				if (mode==6)		//fill is a special case, just use data[0]
					temp = ( (data[0] << shift) | (data[0] >> shift2) );				

				if (row == 0)		//some special treatment for first and last rows
				{
					//if this is the first row, we need to mask off the blank pixels at the top of the row(these pix have random data)
					temp &= mask1; //mask1 has shift blank pixels starting from LSB (LSB is the top of the stripe) 
				}
				else if (row == (n-1))
				{
					//if this is the last row, we need to maks off the blank pixels at the bottom of the image
					temp &= mask2; //mask2 has blank pixels starting at MSB (MSB is the bottom of the stripe)
				}
								
				//combine image data with background
				switch(mode)	
				{
				 case 0:
				 case 1:
					buffer[column] &= temp;			//AND it with buffer
				 break;
				 case 2:
				 case 3:
					buffer[column] |= temp;			//OR it with buffer
				 break;
				 case 4:
				 case 5:
					buffer[column] ^= temp;			//XOR it with buffer
				 break;
				 case 6:	//for copy and fill, we don't want the background bits to interfere with the image, so we clear them
				 case 7:
				 default:
					
					if (row == 0)
						buffer[column] &= ~mask1;	//first row, clear bottom bits of the background
					else if (row == (n-1))
						buffer[column] &= ~mask2;	//last row, clear top bits of background
					else
						buffer[column] = 0;		//middle row, clear all of the background
					
					buffer[column] |= temp;			//OR it with buffer
				 break;
				}
			
				if ((mode % 2 == 1) && (mode < 6))
					buffer[column] = ~buffer[column];	//if we wanted an inverted operation, do it
			}//end column loop
		write_block(x,(row+(y/8)),width,buffer);	//write new row to display
	}//row loop
}
