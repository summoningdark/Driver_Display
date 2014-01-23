/*
 * SensorCov().c
 *
 *  Created on: Oct 30, 2013
 *      Author: Nathan
 */

#include "all.h"
#include "menus.h"

extern const char CANdbcNames[][22];
extern const can_variable_list_struct CANdbc[];

ops_struct ops_temp;
data_struct data_temp;
stopwatch_struct* conv_watch;
stopwatch_struct* Menu_watch;		//stopwatch for menu timeouts

//Defines for States
#define CV1			1
#define CV2			2
#define CV3			3
#define CV4			4
#define MENUSETUP	5
#define MENUCHOICE	6
#define MAINMENU	7
#define VARMENU		8
#define MANUALVAR	9
#define RACEMODE	10
#define TESTMODE	11
#define DISPLAY4	12

//variables for menus
#define NLINES 8
int highlight,offset,i,max,lines,MenuReturn;
const char (*MenuList)[22];		//pointer to menu entries
int MenuStack[10];							//stack for successive menu calls
int MenuStackp;
void Push(int s);
int Pop();

void SensorCov()
{
	SensorCovInit();
	while (ops.State == STATE_SENSOR_COV)
	{
		LatchStruct();
		SensorCovMeasure();
		UpdateStruct();
		FillCANData();
	}
	SensorCovDeInit();
}

void SensorCovInit()
{
	//todo USER: SensorCovInit()

	//config input buttons
	ButtonGpioInit();

	//initialize the button press queue
	ButtonPress.Current = 0;
	ButtonPress.Next = 0;
	ButtonPress.Full = 0;
	ButtonPress.Empty = 1;

	//config LCD GPIO pins
	LCDGpio_init();

	//config LED control pins
	LEDGpio_init();
	SetLEDs((IND1OFF | IND2OFF), 0x0000);	//turn off all LEDs

	//LCD init
	LCDinit();
	LCDSplash(1000);
	SetLEDs(IND1RED, IND1MASK);				//start with red to indicate no CANcorder
	set_font(Font);

	//set up menus
	MenuStackp = 0;

	//testing
	CANvars[4].data.F32 = 37.4;			//testing motor temp
	CANvars[5].data.F32 = 12.6;			//testing 12V bus
	Menu_watch = StartStopWatch(700000);
	conv_watch = StartStopWatch(1000);
}


void LatchStruct()
{
	memcpy(&ops_temp, &ops, sizeof(struct OPERATIONS));
	memcpy(&data_temp, &data, sizeof(struct DATA));
	ops.Change.all = 0;	//clear change states
}

void SensorCovMeasure()
{
	static int State=RACEMODE, d4S=0, DisplayRefresh = 1;
	static int d4N[4]={0,1,2,3};
	StopWatchRestart(conv_watch);
	int tmp;
	can_variable_list_struct tmpCANvar;

	switch(State)
	{
	case MENUSETUP:				//menu setup
		//set LEDs for a menu
		SetLEDs(BTN_BACK_RED | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
		//start at the beginning of the menu
		lines = NLINES;
		highlight = 0;
		offset = 0;
		//find the end of the menu, marked by an empty string. max = number of menu entries
		max = 0;
		while(MenuList[max][0]!=0) max++;
		if (max < lines) lines = max;

		//set the font to small(default font)
		set_font(Font);
		//clear the screen
		clear_screen(0);
		//print lines lines of the menu starting at line offset, with highlight inverted
		set_cursor(0,0);
		for(i=0;i<lines;i++)
		{
			print_cstr(&MenuList[i+offset][0],(i==highlight),0);		//print line
			clear_to_end();										//clear the rest of the line (entries may not all be the same length)
			print_char(0x0D,0,0);									//CR
			print_char(0x0A,0,0);									//LF
		}
		StopWatchRestart(Menu_watch);
		State = MENUCHOICE;		//next state is the choosing
	break;
	case MENUCHOICE:				//menu choice
		if(isStopWatchComplete(Menu_watch)==0)		//check for timeout
		{
			switch(GetButtonPress())
			{
			case BTN_UP:
				StopWatchRestart(Menu_watch);
					if(--highlight == -1)			//decrement highlighted line, if it goes off the top,
						if(--offset == -1)			//decrement the list offset, if it goes off the top
						{
							if (max < lines)
							{
								offset = 0;				//short menu offset stays at 0
								highlight = max-1;		//short menu highlight the last entry
							}
							else
							{
								offset = max-lines;	//offset to the end
								highlight = lines-1;	//highlight the last row
							}
						}
						else
						{
							highlight = 0;			//keep highlight at the top
						}
					//redraw screen
					set_cursor(0,0);
					for(i=0;i<lines;i++)
					{
						print_cstr(&MenuList[i+offset][0],(i==highlight),0);		//print line
						clear_to_end();										//clear the rest of the line (entries may not all be the same length)
						print_char(0x0D,0,0);									//CR
						print_char(0x0A,0,0);									//LF
					}
				break;
			case BTN_DOWN:
				StopWatchRestart(Menu_watch);
					++highlight;						//increment highlighted line
					if ((highlight == max) || (highlight == lines))			//test if highlighted line overruns either the screen or the menu
					{
						if(max < lines)
						{
							highlight = 0;					//wrap highlight back to top
						}
						else
						{
							if(++offset == (max-lines+1))	//increment the list offset, if it goes off the bottom,
							{
								offset = 0;					//offset to the beginning
								highlight = 0;				//highlight the first row
							}
							else
							{
								highlight = lines-1;		//keep highlight at the bottom
							}
						}
					}
					//redraw screen
					set_cursor(0,0);
					for(i=0;i<lines;i++)
					{
						print_cstr(&MenuList[i+offset][0],(i==highlight),0);		//print line
						clear_to_end();										//clear the rest of the line (entries may not all be the same length)
						print_char(0x0D,0,0);									//CR
						print_char(0x0A,0,0);									//LF
					}
				break;
			case BTN_SELECT:
					MenuReturn = (highlight+offset);		//return value is choice
					State = Pop();							//return state from stack
					clear_screen(0);						//give the next state a clear screen to work with
			break;
			case BTN_BACK:
			case BTN_MENU:
					MenuReturn = -1;						//return value is cancel
					State = Pop();							//return state from stack
					clear_screen(0);						//give the next state a clear screen to work with
			break;
			default:
				break;
			}
		}
		else
		{
			State = Pop();
			MenuReturn = -1;						// return Cancel for a timeout
			clear_screen(0);						//give the next state a clear screen to work with
		}
	break;

	case MAINMENU:				//this state decides what to do with the choice from the main menu
		switch (MenuReturn)
		{
		case -1:				//Cancel, pop stack
			State = Pop();
		break;
		case MM_RACEMODE:
			//clear stack
			MenuStackp = 0;
			State = RACEMODE;
			clear_screen(0);
		break;
		case MM_TESTMODE:
		//clear stack
		MenuStackp = 0;
		State = TESTMODE;
		clear_screen(0);
		break;
		case MM_DISPLAY4:
			//clear stack
			MenuStackp = 0;
			State = DISPLAY4;
			clear_screen(0);
		break;
		case MM_CV1:
			Push(CV1);						//ultimately need to return to set variable 1
			Push(VARMENU);					//need to select from list or manual
			MenuList = VariableMenuText;	//point to the right text for the menu
			State = MENUSETUP;				//go to menu setup
		break;
		case MM_CV2:
			Push(CV2);						//ultimately need to return to set variable 2
			Push(VARMENU);					//need to select from list or manual
			MenuList = VariableMenuText;	//point to the right text for the menu
			State = MENUSETUP;				//go to menu setup
		break;
		case MM_CV3:
			Push(CV3);						//ultimately need to return to set variable 3
			Push(VARMENU);					//need to select from list or manual
			MenuList = VariableMenuText;	//point to the right text for the menu
			State = MENUSETUP;				//go to menu setup
		break;
		case MM_CV4:
			Push(CV4);						//ultimately need to return to set variable 4
			Push(VARMENU);					//need to select from list or manual
			MenuList = VariableMenuText;	//point to the right text for the menu
			State = MENUSETUP;				//go to menu setup
		break;
		}
	break;

	case VARMENU:				//this picks action based on variable menu selection
		switch(MenuReturn)
		{
		case -1:
			State = Pop();
		break;
		case VM_LIST:
			MenuList = CANdbcNames;		//point to the list of variables
			State = MENUSETUP;			//setup menu
		break;
		case VM_MANUAL:
			State = MANUALVAR;
		break;
		}
	break;

	case MANUALVAR:			//this does up a manual entry of a watch variable
		print_cstr("Not Yet Implemented",0,0);
		delay_ms(1500);
		Pop();				//pop the CVn
		State = Pop();
		clear_screen(0);
	break;

	case RACEMODE:			//default state (race)
			if (DisplayRefresh)		//do initial screen drawing
			{
				SetLEDs(BTN_MENU_GREEN,BTN_ALL_MASK);
			}
			if(CANvars[0].New == 1 || DisplayRefresh)	//if new can data or flag for redraw
			{
				DisplayRefresh=0;					//just redrew the display
				set_font(RFontHuge);				//big font for race mode
				set_cursor(0,10);					//center the value
				PrintCANvariable(0, 1);				//update the display
			}

			if(GetButtonPress() == BTN_MENU)
			{
				DisplayRefresh = 1;					//Flag Display for update
				MenuList = MainMenuText;			//point to main menu text
				MenuStackp = 0;						//clear stack
				Push(RACEMODE);						//make sure we come back here
				Push(MAINMENU);						//push main menu state
				State = MENUSETUP;					//go to menu setup
			}
			else
			{
				State = RACEMODE;					//stay on this state
			}
	break;

	case TESTMODE:									// test mode displays Variable 0 in medium font and 2 bar graphs
		if (DisplayRefresh)		//do initial screen drawing
		{
			SetLEDs(BTN_MENU_GREEN,BTN_ALL_MASK);
			draw_sprite(0,0,1,7);	//draw motor icon 1
			draw_sprite(0,54,0,7);	//draw 12v battery icon 0
		}

		if(CANvars[0].New == 1 || DisplayRefresh)	//if new can data or flag for redraw
		{
			set_font(FontLarge);				//Medium font for test mode
			set_cursor(0,23);					//center the value
			PrintCANvariable(0, 0);				//update the display
		}

		if(CANvars[4].New == 1 || DisplayRefresh)	//if new can data or flag for redraw
		{
				//do percent bar for motor temp
				//max motor temp is 100 so this is easy
				status_bar(15,0,115,10,(int)CANvars[4].data.F32,2);

		}

		if(CANvars[5].New == 1 || DisplayRefresh)	//if new can data or flag for redraw
		{
			DisplayRefresh=0;					//just redrew the display
			//do status bar for 12V (get from wavesculptor)
			status_bar(15,54,115,63,(int)(CANvars[5].data.F32/.14),2);

		}

		if(GetButtonPress() == BTN_MENU)
		{
			DisplayRefresh = 1;					//Flag Display for update
			MenuList = MainMenuText;			//point to main menu text
			MenuStackp = 0;						//clear stack
			Push(TESTMODE);						//make sure we come back here
			Push(MAINMENU);						//push main menu state
			State = MENUSETUP;					//go to menu setup
		}
		else
		{
			State = TESTMODE;					//stay on this state
		}
	break;

	case DISPLAY4:			//Display 4 with descriptions
			SetLEDs(BTN_BACK_GREEN | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
			set_cursor(0,0);	//start at top
			set_font(Font);		//use small font
			for(tmp=0;tmp<4;tmp++)
			{
				if(CANvars[d4N[tmp]].New == 1 || DisplayRefresh)							//update first displayed variable
				{
					//variable label, print in inverse if d4S == tmp
					print_char(tmp+49,(d4S==tmp),0);										//print Watch label
					print_char(' ',(d4S==tmp),0);;
					print_rstr(&CANvars[d4N[tmp]].Name[0],(d4S==tmp),0);					//print Variable Name
					clear_to_end();															//clear the rest of the line (entries may not all be the same length)
					print_char(0x0D,0,0);													//CR
					print_char(0x0A,0,0);													//LF
					//print variable value
					PrintCANvariable(d4N[tmp], 0);
					clear_to_end();															//clear the rest of the line (entries may not all be the same length)
					print_char(0x0D,0,0);													//CR
					print_char(0x0A,0,0);													//LF
				}
				else	// if not update, print 2 line feeds
				{
					print_char(0x0A,0,0);													//LF
					print_char(0x0A,0,0);													//LF
				}
			}
			DisplayRefresh = 0;						//by the time we get here, the display is redrawn
			State = DISPLAY4;						//default come back to this state
			switch(GetButtonPress())
			{
			case BTN_MENU:
				DisplayRefresh = 1;					//Flag Display for update
				MenuList = MainMenuText;			//point to main menu text
				MenuStackp = 0;						//clear stack
				Push(DISPLAY4);						//make sure we come back here
				Push(MAINMENU);						//push main menu state
				State = MENUSETUP;					//go to menu setup
			break;
			case BTN_UP:			//these buttons select which displayed variable is highlighted.
				if (--d4S == -1) d4S = 3;
				DisplayRefresh = 1;					//Flag Display for update
				break;
			case BTN_DOWN:
				if (++d4S == 4) d4S = 0;
				DisplayRefresh = 1;					//Flag Display for update
				break;
			case BTN_SELECT:		//this button increments the index of the highlighted variable
				if (++d4N[d4S] == 6) d4N[d4S] = 0;
				DisplayRefresh = 1;					//Flag Display for update
				break;
			case BTN_BACK:			//this button decrements the index of the highlighted variable
				if (--d4N[d4S] == -1) d4N[d4S] = 5;
				DisplayRefresh = 1;					//Flag Display for update
				break;
			}
	break;

	case CV1:		//Change Variable 1,2,3,4
	case CV2:
	case CV3:
	case CV4:
		if (MenuReturn == -1)
		{
			State = Pop();
		}
		else
		{
			tmpCANvar = CANdbc[MenuReturn];
			SetCANmonitor(State,  tmpCANvar);		//for code efficiency the State variable CVn = n
			memcpy(&CANvars[State-1].Name, &CANdbcNames[MenuReturn],22);
			State = Pop();
			clear_screen(0);						//give the next state a clear screen to work with
		}
	break;
	default:
		MenuStackp=0;
		State = RACEMODE;
	}
}

void UpdateStruct()
{
	memcpy(&data, &data_temp, sizeof(struct DATA));

	//todo USER: UpdateStruct
	//update with node specific op changes

	//if ops is not changed outside of sensor conversion copy temp over, otherwise don't change

	//Change bit is only set by ops changes outside of SensorCov.
	if (ops.Change.bit.State == 0)
	{
		ops.State = ops_temp.State;
	}

	if (ops.Change.bit.Flags == 0)
	{
		//only cov error happens inside of conversion so all other changes are considered correct.
		//update accordingly to correct cov_errors
		ops.Flags.bit.cov_error = ops_temp.Flags.bit.cov_error;
	}
	ops.Change.all = 0;	//clear change states
}

void SensorCovDeInit()
{
	//todo USER: SensorCovDeInit()
	StopStopWatch(conv_watch);
	StopStopWatch(Menu_watch);
	CLEARLED0();
	CLEARLED1();
}

void Push(int s)
{
	MenuStack[MenuStackp++] = s;
	if (MenuStackp == 10) MenuStackp = 0;
}

int Pop()
{
	int s;
	s = MenuStack[--MenuStackp];
	if (MenuStackp == -1) MenuStackp = 0;
	return s;
}
