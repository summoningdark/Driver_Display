/*
 * SensorCov().c
 *
 *  Created on: Oct 30, 2013
 *      Author: Nathan
 */

#include "all.h"
#include "menus.h"

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
	static int State=0;
	StopWatchRestart(conv_watch);

	switch(State)
	{
	case 0:			//default state (race)
		//check if CAN variable 1 has changed, if so update the display
		//check if menu button pressed, if so change state to 3
		break;

	case 1:			//Display 2 with descriptions
		//check if CAN variables 1 or 2 have changes, if so update the display
		//check if menu button pressed, if so change state to 3
		break;

	case 2:			//Display 4 without descriptions
		//check if any CAN variables have changed, if so update display
		//check if menu button is pressed, if so change state to 3
		break;

	case 3:			//Main Menu
		switch(GetMenuSelection(MainMenuText))
		{

		}
		break;

	case 4:			//Select Variable 1
		break;

	case 5:			//Select Variable 2
		break;

	case 6:			//Select Variable 3
		break;

	case 7:			//Select Variable 4
		break;

	default:
		State = 0;
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
