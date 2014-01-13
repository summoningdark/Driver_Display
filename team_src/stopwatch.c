/*
 * stopwatch.c
 *
 *  Created on: Nov 12, 2013
 *      Author: Nathan
 */

#include "all.h"
#include "DSP2803x_CpuTimers.h"

struct CPUTIMER_VARS StopWatch;

void StopWatchSetUp(float time)
{
    // CPU Timer0
	// Initialize address pointers to respective timer registers:
	StopWatch.RegsAddr = &CpuTimer0Regs;
	// Initialize timer period to maximum:
	CpuTimer0Regs.PRD.all  = 0xFFFFFFFF;
	// Initialize pre-scale counter to divide by 1 (SYSCLKOUT):
	CpuTimer0Regs.TPR.all  = 0;
	CpuTimer0Regs.TPRH.all = 0;
	// Make sure timer is stopped:
	CpuTimer0Regs.TCR.bit.TSS = 1;
	// Reload all counter register with period value:
	CpuTimer0Regs.TCR.bit.TRB = 1;
	// Reset interrupt counters:
	StopWatch.InterruptCount = 0;

	ConfigCpuTimer(&StopWatch,CPU_FREQ_MHZ , time);

	//pie interrupt
	IER |= M_INT1;
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

	ReloadCpuTimer0();
	StartCpuTimer0();
}

stopwatch_struct* StartStopWatch(unsigned int time)
{
	stopwatch_struct* watch = (stopwatch_struct*)myMalloc(sizeof(stopwatch_struct));
	watch->Start = StopWatch.InterruptCount;
	watch->Time = time;
	return watch;
}

void StopWatchRestart(stopwatch_struct* watch)
{
	watch->Start = StopWatch.InterruptCount;
}

char isStopWatchComplete(stopwatch_struct* watch)
{
	if((StopWatch.InterruptCount - watch->Start) > watch->Time)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void StopStopWatch(stopwatch_struct* watch)
{
	myFree(watch);
}

// INT1.7
__interrupt void  TINT0_ISR(void)      // CPU-Timer 0
{
	StopWatch.InterruptCount++;

	//do button debounce and reading
	if(GpioDataRegs.GPBDAT.bit.GPIO33 == 0)		//check leftmost button
		ButtonCounter[0]++;
	else
		ButtonCounter[0] = 0;

	if(GpioDataRegs.GPBDAT.bit.GPIO32 == 0)		//check second
		ButtonCounter[1]++;
	else
		ButtonCounter[1] = 0;

	if(GpioDataRegs.GPADAT.bit.GPIO22 == 0)		//check third
		ButtonCounter[2]++;
	else
		ButtonCounter[2] = 0;

	if(GpioDataRegs.GPADAT.bit.GPIO24 == 0)		//check fourth
		ButtonCounter[3]++;
	else
		ButtonCounter[3] = 0;

	if(GpioDataRegs.GPADAT.bit.GPIO21 == 0)		//check rightmost button
		ButtonCounter[4]++;
	else
		ButtonCounter[4] = 0;

	//update button status
	if (ButtonCounter[0] >= BUTTON_DEBOUNCE_TICKS) ButtonStatus |= 0x0001; else ButtonStatus &=0x001E;
	if (ButtonCounter[1] >= BUTTON_DEBOUNCE_TICKS) ButtonStatus |= 0x0002; else ButtonStatus &=0x001D;
	if (ButtonCounter[2] >= BUTTON_DEBOUNCE_TICKS) ButtonStatus |= 0x0004; else ButtonStatus &=0x001B;
	if (ButtonCounter[3] >= BUTTON_DEBOUNCE_TICKS) ButtonStatus |= 0x0008; else ButtonStatus &=0x0017;
	if (ButtonCounter[4] >= BUTTON_DEBOUNCE_TICKS) ButtonStatus |= 0x0010; else ButtonStatus &=0x000F;

	//update button press queue
	if ((ButtonCounter[0] == BUTTON_DEBOUNCE_TICKS) || (ButtonCounter[0] > BUTTON_HOLD_TICKS))
	{
		if(ButtonPress.Full == 0)																	//don't add press if queue is full
		{
			ButtonPress.Queue[ButtonPress.Next] = 0x0001;											//flag button press
			ButtonPress.Empty = 0;																	//just added cannot be empty
			if(++ButtonPress.Next == BUTTON_QUEUE_SIZE) ButtonPress.Next = 0;						//increment end pointer with wrap
			if(ButtonPress.Next == ButtonPress.Current) ButtonPress.Full = 1;						//flag full if necessary
			if(ButtonCounter[0] > BUTTON_DEBOUNCE_TICKS) ButtonCounter[0] -= BUTTON_REPETE_TICKS;	//reset repete counter
		}
	}

	if ((ButtonCounter[1] == BUTTON_DEBOUNCE_TICKS) || (ButtonCounter[1] > BUTTON_HOLD_TICKS))
	{
		if(ButtonPress.Full == 0)																	//don't add press if queue is full
		{
			ButtonPress.Queue[ButtonPress.Next] = 0x0002;											//flag button press
			ButtonPress.Empty = 0;																	//just added cannot be empty
			if(++ButtonPress.Next == BUTTON_QUEUE_SIZE) ButtonPress.Next = 0;						//increment end pointer with wrap
			if(ButtonPress.Next == ButtonPress.Current) ButtonPress.Full = 1;						//flag full if necessary
			if(ButtonCounter[1] > BUTTON_DEBOUNCE_TICKS) ButtonCounter[1] -= BUTTON_REPETE_TICKS;	//reset repete counter
		}
	}

	if ((ButtonCounter[2] == BUTTON_DEBOUNCE_TICKS) || (ButtonCounter[2] > BUTTON_HOLD_TICKS))
	{
		if(ButtonPress.Full == 0)																	//don't add press if queue is full
		{
			ButtonPress.Queue[ButtonPress.Next] = 0x0004;											//flag button press
			ButtonPress.Empty = 0;																	//just added cannot be empty
			if(++ButtonPress.Next == BUTTON_QUEUE_SIZE) ButtonPress.Next = 0;						//increment end pointer with wrap
			if(ButtonPress.Next == ButtonPress.Current) ButtonPress.Full = 1;						//flag full if necessary
			if(ButtonCounter[2] > BUTTON_DEBOUNCE_TICKS) ButtonCounter[2] -= BUTTON_REPETE_TICKS;	//reset repete counter
		}
	}

	if ((ButtonCounter[3] == BUTTON_DEBOUNCE_TICKS) || (ButtonCounter[3] > BUTTON_HOLD_TICKS))
	{
		if(ButtonPress.Full == 0)																	//don't add press if queue is full
		{
			ButtonPress.Queue[ButtonPress.Next] = 0x0008;											//flag button press
			ButtonPress.Empty = 0;																	//just added cannot be empty
			if(++ButtonPress.Next == BUTTON_QUEUE_SIZE) ButtonPress.Next = 0;						//increment end pointer with wrap
			if(ButtonPress.Next == ButtonPress.Current) ButtonPress.Full = 1;						//flag full if necessary
			if(ButtonCounter[3] > BUTTON_DEBOUNCE_TICKS) ButtonCounter[3] -= BUTTON_REPETE_TICKS;	//reset repete counter
		}
	}

	if ((ButtonCounter[4] == BUTTON_DEBOUNCE_TICKS) || (ButtonCounter[4] > BUTTON_HOLD_TICKS))
	{
		if(ButtonPress.Full == 0)																	//don't add press if queue is full
		{
			ButtonPress.Queue[ButtonPress.Next] = 0x0010;											//flag button press
			ButtonPress.Empty = 0;																	//just added cannot be empty
			if(++ButtonPress.Next == BUTTON_QUEUE_SIZE) ButtonPress.Next = 0;						//increment end pointer with wrap
			if(ButtonPress.Next == ButtonPress.Current) ButtonPress.Full = 1;						//flag full if necessary
			if(ButtonCounter[4] > BUTTON_DEBOUNCE_TICKS) ButtonCounter[4] -= BUTTON_REPETE_TICKS;	//reset repete counter
		}
	}

   // Acknowledge this interrupt to receive more interrupts from group 1
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
