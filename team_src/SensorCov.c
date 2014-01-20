/*
 * SensorCov().c
 *
 *  Created on: Oct 30, 2013
 *      Author: Nathan
 */

#include "all.h"
#include "menus.h"

extern const unsigned char CANdbcNames[][22];
extern const can_variable_list_struct CANdbc[];

ops_struct ops_temp;
data_struct data_temp;
stopwatch_struct* conv_watch;

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

	//CONFIG ADC
	//adcinit();

	//CONFIG GP_BUTTON
	//ConfigGPButton();

	//CONFIG LEDS
	//led 0
	//ConfigLED0();
	//led 1
	//ConfigLED1();
	//CONFIG 12V SWITCH
	//Config12V();
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
	static int State=RACE_MODE, LastState=-1,DisplayState=RACE_MODE,d4S=0;
	static int d4N[4]={0,1,2,3};
	StopWatchRestart(conv_watch);
	int tmp;
	can_variable_list_struct tmpCANvar;

	switch(State)
	{
	case -1:				//main menu
		SetLEDs(BTN_BACK_RED | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
		tmp = GetMenuSelection(MainMenuText);	//get menu selection
		if (tmp == -1)							//check if the user canceled
			State = LastState;
		else
			State = tmp;

		LastState = -1;							//update LastState
		clear_screen(0);						//give the next state a clear screen to work with
	break;

	case RACE_MODE:			//default state (race)
		SetLEDs(BTN_MENU_GREEN,BTN_ALL_MASK);
		if(CANvars[0].New == 1 || LastState == -1)	//if new can data or flag for redraw
		{
			set_font(FontLarge);
			set_cursor(0,10);					//center the value
			PrintCANvariable(0, 0);				//update the display
		}

		if(GetButtonPress() == BTN_MENU)
			State = -1;
		else
			State = RACE_MODE;

		DisplayState=RACE_MODE;
		LastState = RACE_MODE;
	break;

	case DISPLAY4:			//Display 4 with descriptions
		SetLEDs(BTN_BACK_GREEN | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
		set_cursor(0,0);	//start at top
		set_font(Font);		//use small font
		for(tmp=0;tmp<4;tmp++)
		{
			if(CANvars[d4N[tmp]].New == 1 || LastState == -1)							//update first displayed variable
			{
				//variable label, print in inverse if d4S == tmp
				print_cstr((const uint8_t*)&CANdbcNames[CANvars[d4N[tmp]].index][0],(d4S==tmp),0);			//print line
				clear_to_end();															//clear the rest of the line (entries may not all be the same length)
				print_char(0x0D,0,0);													//CR
				print_char(0x0A,0,0);													//LF
				//print variable value
				PrintCANvariable(d4N[tmp], 0);
				print_char(0x0D,0,0);													//CR
				print_char(0x0A,0,0);													//LF
			}
			else	// if not update, print 2 line feeds
			{
				print_char(0x0A,0,0);													//LF
				print_char(0x0A,0,0);													//LF
			}
		}

		State = DISPLAY4;			//default to same state
		LastState = DISPLAY4;
		switch(GetButtonPress())
		{
		case BTN_MENU:
			State = -1;
		break;
		case BTN_UP:			//these buttons select which displayed variable is highlighted.
			if (--d4S == -1) d4S = 3;
			LastState = -1;		//this changes the display, flag re-draw
		break;
		case BTN_DOWN:
			if (++d4S == 4) d4S = 0;
			LastState = -1;		//this changes the display, flag re-draw
		break;
		case BTN_SELECT:		//this button increments the index of the highlighted variable
			if (++d4N[d4S] == 4) d4N[d4S] = 0;
			LastState = -1;		//this changes the display, flag re-draw
		break;
		case BTN_BACK:			//this button decrements the index of the highlighted variable
			if (--d4N[d4S] == -1) d4N[d4S] = 3;
			LastState = -1;		//this changes the display, flag re-draw
		break;
		}
		DisplayState=DISPLAY4;
	break;

	case CNGVAR1:		//Select Variable 1
		SetLEDs(BTN_BACK_RED | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
		tmp = GetMenuSelection(CANdbcNames);	//get menu selection

		if(tmp == -1)
		{
			State = -1;
		}
		else
		{
			tmpCANvar = CANdbc[tmp];
			SetCANmonitor(1,  tmpCANvar);
			CANvars[0].index = tmp;
		}
		State = DisplayState;
		LastState = -1;							//after changing variables, flag display redraw
		clear_screen(0);						//give the next state a clear screen to work with
	break;

	case CNGVAR2:			//Select Variable 2
		SetLEDs(BTN_BACK_RED | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
		tmp = GetMenuSelection(CANdbcNames);	//get menu selection

		if(tmp == -1)
		{
			State = -1;
		}
		else
		{
			tmpCANvar = CANdbc[tmp];
			SetCANmonitor(2,  tmpCANvar);
			CANvars[1].index = tmp;
		}
		State = DisplayState;
		LastState = -1;							//after changing variables, flag display redraw
		clear_screen(0);						//give the next state a clear screen to work with
	break;

	case CNGVAR3:			//Select Variable 3
		SetLEDs(BTN_BACK_RED | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
		tmp = GetMenuSelection(CANdbcNames);	//get menu selection

		if(tmp == -1)
		{
			State = -1;
		}
		else
		{
			tmpCANvar = CANdbc[tmp];
			SetCANmonitor(3,  tmpCANvar);
			CANvars[2].index = tmp;
		}
		State = DisplayState;
		LastState = -1;							//after changing variables, flag display redraw
		clear_screen(0);						//give the next state a clear screen to work with
	break;

	case CNGVAR4:			//Select Variable 4
		SetLEDs(BTN_BACK_RED | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
		tmp = GetMenuSelection(CANdbcNames);	//get menu selection

		if(tmp == -1)
		{
			State = -1;
		}
		else
		{
			tmpCANvar = CANdbc[tmp];
			SetCANmonitor(4,  tmpCANvar);
			CANvars[3].index = tmp;
		}
		State = DisplayState;
		LastState = -1;							//after changing variables, flag display redraw
		clear_screen(0);						//give the next state a clear screen to work with
	break;

	default:
		State = RACE_MODE;
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
	CLEARLED0();
	CLEARLED1();
}
