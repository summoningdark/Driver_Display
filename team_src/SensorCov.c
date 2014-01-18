/*
 * SensorCov().c
 *
 *  Created on: Oct 30, 2013
 *      Author: Nathan
 */

#include "all.h"
#include "menus.h"
extern const unsigned char CANdbcNames[][20];
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

	//LCD init
	LCDinit();

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
	static int State=RACE_MODE, LastState=RACE_MODE,DisplayState=RACE_MODE,d2N1=0,d2N2=1,d2S=0;
	StopWatchRestart(conv_watch);
	int tmp;
	can_variable_list_struct tmpCANvar;

	switch(State)
	{
	case -1:				//main menu
		tmp = GetMenuSelection(MainMenuText);	//get menu selection
		if (tmp == -1)							//check if the user canceled
			State = LastState;
		else
			State = tmp;

		LastState = -1;							//update LastState
		clear_screen(0);						//give the next state a clear screen to work with
	break;

	case RACE_MODE:			//default state (race)
		if(CANvars[0].New == 1 || LastState == -1)	//if new can data or we just came off of the main menu
			PrintCANvariable(1,0,0);				//update the display

		if(GetButtonPress() == BTN_MENU)
			State = -1;
		else
			State = RACE_MODE;

		DisplayState=RACE_MODE;
		LastState = RACE_MODE;
	break;

	case DISPLAY2:			//Display 2 with descriptions

		if(CANvars[d2N1].New == 1 || LastState == -1)		//update first displayed variable
		{
			//print first variable label, print in inverse if d2S == 0
			//print first variable value
		}

		if(CANvars[d2N2].New == 1 || LastState == -1)		//update second displayed variable
		{
			//print second variable label, print in inverse if d2S == 1
			//print second variable value
		}


		State = DISPLAY2;			//default to same state
		switch(GetButtonPress())
		{
		case BTN_MENU:
			State = -1;
		break;
		case BTN_UP:			//these buttons select which displayed variable is highlighted. in a list of two up and down do the same thing
		case BTN_DOWN:
			if (d2S == 0) d2S = 1; else d2S = 0;
		break;
		case BTN_SELECT:		//this button increments the index of the highlighted variable
			if(d2S == 0)
			{
				if (++d2N1 == 4) d2N1 = 0;
			}
			else
			{
				if (++d2N2 == 4) d2N2 = 0;
			}
		break;
		case BTN_BACK:			//this button decrements the index of the highlighted variable
			if(d2S == 0)
			{
				if (--d2N1 == -1) d2N1 = 3;
			}
			else
			{
				if (--d2N2 == -1) d2N2 = 3;
			}
		break;
		}
		DisplayState=DISPLAY2;
		LastState = DISPLAY2;
	break;

	case DISPLAY4:			//Display 4 without descriptions


		if(GetButtonPress() == BTN_MENU)
			State = -1;
		else
			State = DISPLAY4;

		DisplayState=DISPLAY4;
		LastState = DISPLAY4;
	break;

	case CNGVAR1:		//Select Variable 1
		tmp = GetMenuSelection(CANdbcNames);	//get menu selection

		if(tmp == -1)
		{
			State = -1;
		}
		else
		{
			tmpCANvar = CANdbc[tmp];
			SetCANmonitor(1,  tmpCANvar);
		}
		State = DisplayState;
		LastState = CNGVAR1;
	break;

	case CNGVAR2:			//Select Variable 2
		tmp = GetMenuSelection(CANdbcNames);	//get menu selection

		if(tmp == -1)
		{
			State = -1;
		}
		else
		{
			tmpCANvar = CANdbc[tmp];
			SetCANmonitor(2,  tmpCANvar);
		}
		State = DisplayState;
		LastState = CNGVAR2;	break;

	case CNGVAR3:			//Select Variable 3
		tmp = GetMenuSelection(CANdbcNames);	//get menu selection

		if(tmp == -1)
		{
			State = -1;
		}
		else
		{
			tmpCANvar = CANdbc[tmp];
			SetCANmonitor(3,  tmpCANvar);
		}
		State = DisplayState;
		LastState = CNGVAR3;	break;

	case CNGVAR4:			//Select Variable 4
		tmp = GetMenuSelection(CANdbcNames);	//get menu selection

		if(tmp == -1)
		{
			State = -1;
		}
		else
		{
			tmpCANvar = CANdbc[tmp];
			SetCANmonitor(4,  tmpCANvar);
		}
		State = DisplayState;
		LastState = CNGVAR4;	break;

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
