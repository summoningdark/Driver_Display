/*
 * SensorCov().c
 *
 *  Created on: Oct 30, 2013
 *      Author: Nathan
 */

#include "all.h"
#include "menus.h"
#include <stdio.h>
#include "Flash2803x_API_Library.h"

#define round(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

extern void EraseFlashB(void);
extern int WriteFloatFlashB(Uint32 Address, float Value);

extern const char CANdbcNames[][22];
extern const can_variable_list_struct CANdbc[];
extern unsigned int x_offset,y_offset;
typedef union
{
	float F32;
	Uint32 U32;
} Flash_union;
const Flash_union FlashB[0x1000];			//array used to read from flashb sector
#pragma DATA_SECTION(FlashB,"flb");

ops_struct ops_temp;
data_struct data_temp;
stopwatch_struct* Menu_watch;		//stopwatch for menu timeouts
stopwatch_struct* CellVolt_watch;		//stopwatch for cell voltage timeouts
stopwatch_struct* CellTime_watch;		//stopwatch for cell voltage measurement timing
stopwatch_struct* Refresh_watch;		//stopwatch for display refresh
unsigned int GPSvalid = 0;				//flag for GPS lock
float AmpHoursOffset = 0;
int AmpHours_pointer = 0;

//Defines for States
#define CV1			1
#define CV2			2
#define CV3			3
#define CV4			4
#define MENUSETUP	5
#define MENUCHOICE	6
#define MAINMENU	7
#define VARMENU		8
#define MANUALVAR1	9
#define MANUALVAR2	10
#define MANUALVAR3	11
#define MANUALVAR4	12
#define RACEMODE	13
#define TESTMODE	14
#define DISPLAY4	15
#define BATTMON		16
#define FUELG		17

//variables for menus
#define NLINES 8
int highlight,offset,i,max,lines,MenuReturn;
const char (*MenuList)[22];		//pointer to menu entries
int MenuStack[10];							//stack for successive menu calls
int MenuStackp;
void Push(int s);
int Pop();

//variables for Cell Stats
char BatMonCell = 0;			//battery monitor current cell
char BatGraphFlag = 0;			//default to bar graph
float CellVolt[120] = {0};		//holds cell voltages
char CellGraph[128] = {0};		//holds cell graph
float MaxCell=0, MinCell=5;		//max and min cell voltages
int MaxN=23, MinN=5;			//max and min cell numbers
int MaxB=1, MinB=4;				//max and min BIM numbers
int NCells = 0;					//total number of cells with good data
float CellChargeV = 4.5;		//Cell Max Charge Voltage
float CellDeadV = 2.5;			//Cell Max Discharge Voltage

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
	int tmp;
	FLASH_ST FlashStatus;
	//config input buttons
	ButtonGpioInit();
	ConfigLED0();
	ConfigLED1();
	SETLED0();
	SETLED1();

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
	SetLEDs(IND1RED | IND2RED, IND1MASK | IND2MASK);				//start with red to indicate no CANcorder, no Tritium
	set_font(Font);


	//retrieve the AmpHour offset from flash
	//scan through flash looking for first blank(0xFFFFFFFF) value
	//set pointer to first blank place -1
	//if pointer is -1, set pointer to 0, then set amphours to value at pointer
	for (tmp=0;tmp<0x1000;tmp++)
		if (FlashB[tmp].U32 == 0xFFFFFFFF)
		{
			AmpHours_pointer = tmp - 1;
			break;
		}
		if (AmpHours_pointer == -1) //if this is the very first entry
		{
			AmpHours_pointer = 0;	//set the pointer to 0
			AmpHoursOffset = 0;		//set the offset to 0
			//program the first location to 0
			tmp = WriteFloatFlashB(0x3F4000L, AmpHoursOffset);
		}
		else
		{
			AmpHoursOffset = FlashB[AmpHours_pointer].F32;
		}

	//set up menus
	MenuStackp = 0;

	//battery graphs
	for(tmp=0;tmp<128;tmp++)
		CellGraph[tmp] = 0;
	for(tmp=0;tmp<120;tmp++)
			CellVolt[tmp] = -1;

	//testing
	CANvars[4].data.F32 = 37.4;			//testing motor temp
	CANvars[5].data.F32 = 12.6;			//testing 12V bus
	CANvars[0].data.F32 = 0.123;
	Menu_watch = StartStopWatch(700000L);	//stopwatch for menu timeout
	CellVolt_watch = StartStopWatch(1000);	//stopwatch for Cell voltage timeout
	CellTime_watch = StartStopWatch(1000);	//stopwatch for Cell voltage request timing
	Refresh_watch = StartStopWatch(50000);	//stopwatch for display refresh
}


void LatchStruct()
{
	memcpy(&ops_temp, &ops, sizeof(struct OPERATIONS));
	memcpy(&data_temp, &data, sizeof(struct DATA));
	ops.Change.all = 0;	//clear change states
}

void SensorCovMeasure()
{
	static int State=RACEMODE, d4S=0, DisplayRefresh = 1,ManID=0,ManDigit=2,ManOffset=0;
	static int RaceVar = 8;		//start Racemode displaying AmpHours
	static int d4N[4]={0,1,2,3};
	int tmp,tmp2;
	char text[80];
	float tmpFloat;
	struct ECAN_REGS ECanaShadow;	//shadow structure for modifying CAN registers
	static can_variable_list_struct tmpCANvar;

	//always check for cancorder, GPS and tritium status
	tmp = 3;
	if (isStopWatchComplete(CANvars[NUM_CANVARS - 1].Timeout))
		tmp &= 0xFE;	//if Cancorder timed out, clear bit 0 in tmp
	if (GPSvalid == 0)
		tmp &= 0xFD;	//if GPS not valid, clear bit 1 in tmp
	switch (tmp)
	{
	case 0:		//no Cancorder or GPS
		SetLEDs(IND1OFF,IND1MASK);
	break;
	case 1:		//Cancorder but no GPS
		SetLEDs(IND1RED,IND1MASK);
	break;
	case 2:		//GPS but no Cancorder
		SetLEDs(IND1OFF,IND1MASK);
	break;
	case 3:		//Both Cancorder and GPS
		SetLEDs(IND1GREEN,IND1MASK);
	break;
	}

	if (isStopWatchComplete(CANvars[6].Timeout))
	{
		SetLEDs(IND2OFF,IND2MASK);
	}
	else if (CANvars[6].data.F32 < 50)
	{
		SetLEDs(IND2YELLOW,IND2MASK);
	}
	else if (CANvars[6].data.F32 < 300)
	{
		SetLEDs(IND2RED,IND2MASK);
	}
	else
	{
		SetLEDs(IND2GREEN,IND2MASK);
	}

//check for super secret two button press to reset LCD
	if ((ButtonStatus & BTN_UP) && (ButtonStatus & BTN_DOWN))
	{
		LCDinit();
		DisplayRefresh=1;
	}

//check for extra super secret button press to reset AmpHours counter
	if ((ButtonStatus & BTN_UP) && (ButtonStatus & BTN_SELECT))
	{
		EraseFlashB();
		AmpHoursOffset = 0;
		AmpHours_pointer = 0;
	}

	//check if current AmpHours differ from stored offset by .1 or more, if so update
	tmpFloat = FlashB[AmpHours_pointer].F32;

	//compare old value with new value
	if ( ((CANvars[8].data.F32 - tmpFloat) >= 0.1) || ((CANvars[8].data.F32 - tmpFloat) <= -0.1) )
	{
		if (++AmpHours_pointer == 0x1000)
		{
			AmpHours_pointer = 0;
			EraseFlashB();
		}
		tmp = WriteFloatFlashB(0x3F4000L+(AmpHours_pointer*2), CANvars[8].data.F32);
	}

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
		case MM_BATTMON:
			//clear stack
			MenuStackp = 0;
			State = BATTMON;
			clear_screen(0);
		break;
		case MM_FUELG:
			//clear stack
			MenuStackp = 0;
			State = FUELG;
			clear_screen(0);
			tmpCANvar.SID = 0x40E;			//set CAN variable 1 to Tritium bus Amp Hours
			tmpCANvar.Offset = 32;
			tmpCANvar.TypeCode = 6;
			SetCANmonitor(1,tmpCANvar);
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
			ManID=0;					//set up manual entry variables
			ManOffset=0;
			ManDigit=2;
			State = MANUALVAR1;			//go directly to manual variable set
		break;
		}
	break;

	case MANUALVAR1:			//this does up a manual entry of a watch variable, CAN ID part
		if(MenuReturn == -1)
		{
			State = Pop();
		}
		else
		{
			SetLEDs(BTN_BACK_GREEN | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
			//first pick the CAN ID, this is done using up/down to change the highlighted hex digit, and back/select to select the digit
			//if you back off the most significant digit, the process is considered canceled, if you select off the least significant digit the process is considered complete
			set_font(Font);
			set_cursor(0,0);
			print_cstr("Choose CAN ID",0,0);
			set_font(FontLarge);				//Medium font for test mode
			set_cursor(0,23);					//center the value
			print_cstr("0x",0,0);				//print hex prefix
			//print the current values, highlight the selected one
			tmp = ((ManID & 0x0F00)>>8)+48;	//get first digit
			if (tmp>57) tmp += 7;				//need this to go from 9 to A
			print_char(tmp,(ManDigit==2),0);
			tmp = ((ManID & 0x00F0)>>4)+48;		//get second digit
			if (tmp>57) tmp += 7;				//need this to go from 9 to A
			print_char(tmp,(ManDigit==1),0);
			tmp = (ManID & 0x000F) + 48;		//get last digit
			if (tmp>57) tmp += 7;				//need this to go from 9 to A
			print_char(tmp,(ManDigit==0),0);
			//read the buttons and decide what to do
			switch(GetButtonPress())
			{
			case BTN_BACK:	//move highlighted digit 1 to the left. if they go off the end, cancel
				if(++ManDigit == 3)
				{
					MenuReturn = -1;
					State = Pop();
					clear_screen(0);
				}
			break;
			case BTN_UP:	//increment the value of the current digit, if it goes above F, set to 0
				tmp = (ManID >> (ManDigit*4)) & 0x000F;		//get current digit
				tmp = (++tmp) & 0x0F;						//increment
				ManID &= ~(0xF << (ManDigit*4));			//clear the current digit
				ManID |= (tmp << (ManDigit*4));				//replace with new value
			break;
			case BTN_DOWN:	//decrement the value of the current digit, if it goes below 0, set to F
				tmp = (ManID >> (ManDigit*4)) & 0x000F;		//get current digit
				tmp = (--tmp) & 0x0F;						//decrement
				ManID &= ~(0xF << (ManDigit*4));			//clear the current digit
				ManID |= (tmp << (ManDigit*4));				//replace with new value
			break;
			case BTN_SELECT://move highlighted digit 1 to the right. if they go off the end, go to the next phase
				if(--ManDigit == -1)
				{
					tmpCANvar.SID = ManID & 0x07FF;		//mask all but 11-bits
					Push(MANUALVAR2);					//set up menu to return to MANUALVAR2
					MenuList = VariableTypeText;		//point to the list of variable types
					State = MENUSETUP;					//setup menu
				}
			break;
			case BTN_MENU:	//this cancels
				MenuReturn = -1;
				State = Pop();
				clear_screen(0);
			}
		}
	break;

	case MANUALVAR2:			//this does up a manual entry of a watch variable, CAN variable type part
		if(MenuReturn == -1)
		{
			State = Pop();
		}
		else
		{
			tmpCANvar.TypeCode = MenuReturn;
			ManDigit = 1;						//CAN offset is only 2 digits, make sure we start on a valid one
			State = MANUALVAR3;
		}
	break;

	case MANUALVAR3:			//this does up a manual entry of a watch variable, CAN variable offset part
		if(MenuReturn == -1)
		{
			State = Pop();
		}
		else
		{
			SetLEDs(BTN_BACK_GREEN | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
			//last pick the CAN offset, this is done using up/down to change the highlighted hex digit, and back/select to select the digit
			//if you back off the most significant digit, the process is considered canceled, if you select off the least significant digit the process is considered complete
			set_font(Font);
			set_cursor(0,0);
			print_cstr("Choose CAN Offset in bits",0,0);
			set_font(FontLarge);				//Medium font for test mode
			set_cursor(0,23);					//center the value
			print_cstr("0x",0,0);				//print hex prefix
			//print the current values, highlight the selected one
			tmp = ((ManOffset & 0x00F0)>>4)+48;	//get first digit
			if (tmp>57) tmp += 7;				//need this to go from 9 to A
			print_char(tmp,(ManDigit==1),0);
			tmp = (ManOffset & 0x000F) + 48;		//get last digit
			if (tmp>57) tmp += 7;				//need this to go from 9 to A
			print_char(tmp,(ManDigit==0),0);
			//read the buttons and decide what to do
			switch(GetButtonPress())
			{
			case BTN_BACK:	//move highlighted digit 1 to the left. if they go off the end, cancel
				if(++ManDigit == 2)
				{
					MenuReturn = -1;
					State = Pop();
					clear_screen(0);
				}
			break;
			case BTN_UP:	//increment the value of the current digit, if it goes above F, set to 0
				tmp = (ManOffset >> (ManDigit*4)) & 0x000F;		//get current digit
				tmp = (++tmp) & 0x0F;						//increment
				ManOffset &= ~(0xF << (ManDigit*4));			//clear the current digit
				ManOffset |= (tmp << (ManDigit*4));				//replace with new value
			break;
			case BTN_DOWN:	//decrement the value of the current digit, if it goes below 0, set to F
				tmp = (ManOffset >> (ManDigit*4)) & 0x000F;		//get current digit
				tmp = (--tmp) & 0x0F;						//decrement
				ManOffset &= ~(0xF << (ManDigit*4));			//clear the current digit
				ManOffset |= (tmp << (ManDigit*4));				//replace with new value
			break;
			case BTN_SELECT:	//move highlighted digit 1 to the right. if they go off the end, go to the next phase
				if(--ManDigit == -1)
				{
					tmpCANvar.Offset = ManOffset & 0x003F;		//max offset is 63 bits
					clear_screen(0);
					State = MANUALVAR4;							//go to confirmation screen
				}
			break;
			case BTN_MENU:	//this cancels
				MenuReturn = -1;
				State = Pop();
				clear_screen(0);
			}
		}
	break;

	case MANUALVAR4:
		if(MenuReturn == -1)
		{
			State = Pop();
		}
		else
		{
			SetLEDs(BTN_BACK_RED | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
			set_font(Font);			//small font
			set_cursor(0,0);
			print_cstr("Manual Set Variable",0,0);
			print_char(0x0D,0,0);													//CR
			print_char(0x0A,0,0);													//LF
			print_cstr("CAN ID: 0x",0,0);
			tmp = ((tmpCANvar.SID >> 8) & 0x000F) + 48;
			if (tmp>57) tmp += 7;
			print_char(tmp,0,0);
			tmp = ((tmpCANvar.SID >> 4) & 0x000F) + 48;
			if (tmp>57) tmp += 7;
			print_char(tmp,0,0);
			tmp = (tmpCANvar.SID & 0x000F) + 48;
			if (tmp>57) tmp += 7;
			print_char(tmp,0,0);
			print_char(0x0D,0,0);													//CR
			print_char(0x0A,0,0);													//LF
			print_cstr("Variable Type: ",0,0);
			print_cstr(&VariableTypeText[tmpCANvar.TypeCode][0],0,0);
			print_char(0x0D,0,0);													//CR
			print_char(0x0A,0,0);													//LF
			print_cstr("Variable Offset: 0x",0,0);
			tmp = ((tmpCANvar.Offset >> 4) & 0x000F) + 48;
			if (tmp>57) tmp += 7;
			print_char(tmp,0,0);
			tmp = (tmpCANvar.Offset & 0x000F) + 48;
			if (tmp>57) tmp += 7;
			print_char(tmp,0,0);
			print_char(0x0D,0,0);												//CR
			print_char(0x0A,0,0);													//LF
			print_cstr("Back = Cancel",0,0);
			print_char(0x0D,0,0);												//CR
			print_char(0x0A,0,0);												//LF
			print_cstr("Select = OK",0,0);

			switch(GetButtonPress())
			{
			case BTN_BACK:	//cancel
				MenuReturn = -1;
				State = Pop();
				clear_screen(0);
			break;
			case BTN_SELECT:	//accept
				tmp = Pop();						//get which CVn the user wanted
				SetCANmonitor(tmp,  tmpCANvar);		//for code efficiency the State variable CVn = n
				sprintf(CANvars[tmp-1].Name,"%s@%#03x OF:%#02x",VariableTypeText[tmpCANvar.TypeCode],tmpCANvar.SID,tmpCANvar.Offset);
				State = Pop();						//pop the next state
				clear_screen(0);					//give the next state a clear screen to work with
			break;
			case BTN_MENU:	//cancels
				MenuReturn = -1;
				State = Pop();
				clear_screen(0);
			}
		}
	break;

	case RACEMODE:			//default state (race)
			if (DisplayRefresh)		//do initial screen drawing
			{
				SetLEDs(BTN_BACK_GREEN | BTN_UP_GREEN | BTN_SELECT_GREEN | BTN_MENU_GREEN,BTN_ALL_MASK);
			}
			if(isStopWatchComplete(Refresh_watch) || DisplayRefresh || isStopWatchComplete(CANvars[RaceVar].Timeout))	//if new can data, data timeout, or flag for redraw
			{
				StopWatchRestart(Refresh_watch);
				DisplayRefresh=0;					//just redrew the display
				set_cursor(0,10);					//center the value
				PrintCANvariable(RaceVar, 2);				//update the display
				if (y_offset == 10) clear_to_end();
				CANvars[RaceVar].New = 0;					//variable is no longer new
				set_font(Font);
				set_cursor(0,55);
				print_rstr(&CANvars[RaceVar].Name[0],0,0);//print Variable Name
				if (x_offset > 0) clear_to_end();

			}

			switch(GetButtonPress())
			{
			case BTN_MENU:
				DisplayRefresh = 1;					//Flag Display for update
				MenuList = MainMenuText;			//point to main menu text
				MenuStackp = 0;						//clear stack
				Push(RACEMODE);						//make sure we come back here
				Push(MAINMENU);						//push main menu state
				State = MENUSETUP;					//go to menu setup
				break;
			case BTN_BACK:
				GpioDataRegs.GPATOGGLE.bit.GPIO16 = 1;	//toggle backlight
				State = RACEMODE;					//stay on this state
				break;
			case BTN_UP:
				if (--RaceVar == -1) RaceVar = (NUM_CANVARS - 2);
				DisplayRefresh = 1;					//Flag Display for update
			break;
			case BTN_SELECT:
				if (++RaceVar == (NUM_CANVARS - 1)) RaceVar = 0;
				DisplayRefresh = 1;					//Flag Display for update
			break;
			default:
				State = RACEMODE;					//stay on this state
			}

	break;

	case TESTMODE:									// test mode displays Variable 0 in medium font and 2 bar graphs
		if (DisplayRefresh)		//do initial screen drawing
		{
			SetLEDs(BTN_BACK_GREEN | BTN_MENU_GREEN,BTN_ALL_MASK);
			draw_sprite(0,0,1,7);	//draw motor icon 1
			draw_sprite(0,54,0,7);	//draw 12v battery icon 0
		}

		if(isStopWatchComplete(Refresh_watch) || DisplayRefresh || isStopWatchComplete(CANvars[RaceVar].Timeout))	//if new can data or flag for redraw
		{
			StopWatchRestart(Refresh_watch);
			set_cursor(0,23);					//center the value
			PrintCANvariable(RaceVar, 1);				//update the display
			if (y_offset == 23) clear_to_end();
			CANvars[RaceVar].New = 0;					//variable is no longer new
		}

		if(CANvars[4].New == 1 || DisplayRefresh  || isStopWatchComplete(CANvars[4].Timeout))	//if new can data or flag for redraw
		{
				//do percent bar for motor temp
				//max motor temp is 100 so this is easy
			if(isStopWatchComplete(CANvars[4].Timeout))
			{
				status_bar(15,0,115,10,0,2);	//for no data, draw an empty bar
				set_cursor(54,2);				//center the value
				set_font(Font);
				print_cstr("XXX",0,0);			//print some Xs
			}
			else
			{
				status_bar(15,0,115,10,(int)CANvars[4].data.F32,2);
			}
				CANvars[4].New = 0;
		}

		if(CANvars[5].New == 1 || DisplayRefresh  || isStopWatchComplete(CANvars[5].Timeout))	//if new can data or flag for redraw
		{
			DisplayRefresh=0;					//just redrew the display
			//do status bar for 12V
			if(isStopWatchComplete(CANvars[5].Timeout))
			{
				status_bar(15,52,115,63,0,2);	//for no data, draw an empty bar

				set_font(Font);
				set_cursor(54,54);				//center the value
				print_cstr("XXX",0,0);			//print some Xs
			}
			else
			{
				status_bar(15,52,115,63,(int)(CANvars[5].data.F32/.14),2);
				CANvars[5].New = 0;
			}
		}

		switch(GetButtonPress())
		{
		case BTN_MENU:
			DisplayRefresh = 1;					//Flag Display for update
			MenuList = MainMenuText;			//point to main menu text
			MenuStackp = 0;						//clear stack
			Push(TESTMODE);						//make sure we come back here
			Push(MAINMENU);						//push main menu state
			State = MENUSETUP;					//go to menu setup
			break;
		case BTN_BACK:
			GpioDataRegs.GPATOGGLE.bit.GPIO16 = 1;	//toggle backlight
			State = TESTMODE;					//stay on this state
			break;
		default:
			State = TESTMODE;					//stay on this state
		}
	break;

	case DISPLAY4:			//Display 4 with descriptions
		if (DisplayRefresh)		//do initial screen drawing
		{
			SetLEDs(BTN_BACK_GREEN | BTN_UP_GREEN | BTN_DOWN_GREEN | BTN_SELECT_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
		}

			set_cursor(0,0);	//start at top
			set_font(Font);		//use small font
			for(tmp=0;tmp<4;tmp++)
			{
				if(isStopWatchComplete(Refresh_watch) || DisplayRefresh || isStopWatchComplete(CANvars[d4N[tmp]].Timeout))							//update first displayed variable
				{
					StopWatchRestart(Refresh_watch);
					//variable label, print in inverse if d4S == tmp
					print_char(tmp+49,(d4S==tmp),0);										//print Watch label
					print_char(' ',(d4S==tmp),0);
					print_rstr(&CANvars[d4N[tmp]].Name[0],(d4S==tmp),0);					//print Variable Name
					if (x_offset > 0)
					{
						clear_to_end();															//clear the rest of the line (entries may not all be the same length)
						print_char(0x0D,0,0);													//CR
						print_char(0x0A,0,0);													//LF
					}
					//print variable value
					PrintCANvariable(d4N[tmp], 0);
					CANvars[d4N[tmp]].New = 0 ;												//variable is no longer new
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
				if (++d4N[d4S] == (NUM_CANVARS - 1)) d4N[d4S] = 0;
				DisplayRefresh = 1;					//Flag Display for update
				break;
			case BTN_BACK:			//this button decrements the index of the highlighted variable
				if (--d4N[d4S] == -1) d4N[d4S] = (NUM_CANVARS - 2);
				DisplayRefresh = 1;					//Flag Display for update
				break;
			}
	break;

	case BATTMON:				//Battery Monitor
		if (DisplayRefresh)		//screen drawing
		{
			if(BatGraphFlag)
				SetLEDs(BTN_BACK_GREEN | BTN_DOWN_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
			else
				SetLEDs(BTN_BACK_GREEN | BTN_UP_GREEN | BTN_MENU_RED,BTN_ALL_MASK);
			set_font(Font);		//use small font
			if(BatGraphFlag)
			{
				//print total cells
				set_cursor(0,0);
				sprintf(text,"%d",NCells);
				print_rstr(text,0,0);
			}
			//print cell max/min
			set_cursor(0,56);	//bottom row
			clear_to_end();		//clear old text
			sprintf(text,"%#.3f %d:%d %#.3f %d:%d",MaxCell,MaxB,MaxN,MinCell,MinB,MinN);
			print_rstr(text,0,0);
			for(tmp=0;tmp<128;tmp++)	//draw graph
			{
				tmp2 = CellGraph[tmp];
				if(tmp2 >= 0)	//positive values are not balancing
				{
					draw_block(tmp,54,tmp+1,0,0x00);					//clear line on graph
					if(tmp2 != 0) draw_block(tmp,50,tmp+1,tmp2,0xFF);	//a value of 0 indicates no data
				}
				else
				{
					draw_block(tmp,54,tmp+1,0,0x00);			//clear line on graph
					draw_block(tmp, 54,tmp+1,52,0xFF);		//draw balancing indicator
					draw_block(tmp,50,tmp+1,-tmp2,0xFF);		//draw graph
				}
			}
			MaxCell = 0;			//reset max/min cells
			MinCell = 5;
		}

		DisplayRefresh = 0;						//by the time we get here, the display is redrawn

		if(BatMonCell == 30)	//if we have all the cell voltages, redraw the graph
		{
			if(BatGraphFlag)
			{	//calculate Histogram
				//Histogram is 1 bar for each voltage bin [(CellChargV - CellDeadV)/128] volts/bin
				//with height scaled so 0 pixels is 0 cells and 50 pixels is all the cells
				//All the cells defined to be all the cells we have data for
				for(tmp=0;tmp<128;tmp++)	//zero out all elements
					CellGraph[tmp]=0;
				NCells = 0;

				tmpFloat = (CellChargeV-CellDeadV)/128.0;	//calculate bin size

				for(tmp=0;tmp<NCELLS;tmp++)	//put each cell in a bin
				{
					if (CellVolt[tmp]>0)	//count number of cells with data
						NCells++;

					if (CellVolt[tmp] >= CellDeadV)	//only histogram those cells with voltages higher than dead
						CellGraph[(int)((CellVolt[tmp]-CellDeadV)/tmpFloat)] +=1;	//increment bin corresponding to cell
				}

				//scale bins so 1 = 50, NCells = 0
				if (NCells == 0) NCells = 1;			//prevent divide by 0
				for(tmp=0;tmp<128;tmp++)
				{
					if(CellGraph[tmp] > 0)
						CellGraph[tmp] = 50-(int)(CellGraph[tmp]*50.0/NCells);
				}
			}
			else
			{	//calculate Bar Graph
				//Bar Graph is one vertical line/cell, scaled so that max discharge has a height of 1 pixel(50) and max charge has a height of 49 pixels(1)
				//if there is no data for a cell, the graph value is set to 0 and no line is drawn
				tmpFloat = -50.0/(CellChargeV-CellDeadV);	//slope for conversion
				for(tmp=0;tmp<NCELLS;tmp++)	//loop for all cells
				{
					if(CellVolt[tmp] > 0)
					{
						if(CellGraph[tmp] == -1)	//previously used graph array to indicate balancing state
						{
							CellGraph[tmp] = -round(tmpFloat*(CellVolt[tmp]-CellChargeV));
						}
						else
						{
							CellGraph[tmp] = round(tmpFloat*(CellVolt[tmp]-CellChargeV));
						}
					}
					else
					{
						CellGraph[tmp] = 0;	//no good cell voltage, zero graph
					}
				}
				for(tmp=tmp;tmp<128;tmp++)	//zero out remaining screen area
					CellGraph[tmp]=0;
			}

			DisplayRefresh = 1;		//flag redraw
			BatMonCell = 0;			//start over
		}
		else if(CellVoltFlag == 0)					//request cell voltage
		{
			if(isStopWatchComplete(CellTime_watch))	//this stopwatch limits spamming the bus with Cell voltage requests
			{
				EALLOW;
				//set up mailbox(11)
				ECanaShadow.CANME.all = ECanaRegs.CANME.all;	//get current CAN registers
				ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;	//get current CAN registers

				ECanaShadow.CANME.bit.ME11 = 0;
				ECanaRegs.CANME.all = ECanaShadow.CANME.all;	//disable mailboxes so we can change the ID

				//mailbox IDs, Cell voltages start at 0x310
				ECanaMboxes.MBOX11.MSGID.bit.STDMSGID = 0x310+BatMonCell;

				ECanaMboxes.MBOX11.MSGCTRL.bit.RTR = 1;	//send RTR

				ECanaShadow.CANME.bit.ME11 = 1;			//enable mailbox 11
				ECanaRegs.CANME.all = ECanaShadow.CANME.all;
				EDIS;

				//request that the mailbox be sent
				ECanaShadow.CANTRS.bit.TRS11 = 1;				//mark mailbox 11 for RTR
				ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;	//set in real registers

				StopWatchRestart(CellVolt_watch);		//restart timeout
				StopWatchRestart(CellTime_watch);		//restart Cell voltage request timer
				CellVoltFlag = 1;						//flag waiting for response
			}
		}
		else if(CellVoltFlag == 1)					//waiting for response
		{
			if(isStopWatchComplete(CellVolt_watch))	//check for timeout
			{
				CellVoltFlag = 0;					//flag request next cell block
				CellVolt[BatMonCell*4] = -1;		//flag cell as no data
				CellGraph[BatMonCell*4] = 0;
				CellVolt[BatMonCell*4+1] = -1;		//flag cell as no data
				CellGraph[BatMonCell*4+1] = 0;
				CellVolt[BatMonCell*4+2] = -1;		//flag cell as no data
				CellGraph[BatMonCell*4+2] = 0;
				CellVolt[BatMonCell*4+3] = -1;		//flag cell as no data
				CellGraph[BatMonCell*4+3] = 0;
				BatMonCell++;						//increment cell block
			}
		}
		else if (CellVoltFlag == 2)					//got Cell voltage
		{
			for(tmp=0;tmp<4;tmp++)
			{
				if((CurrCellBlock.Volt[tmp] > 0) && (CurrCellBlock.Volt[tmp] < 5))
				{
					if(CurrCellBlock.Volt[tmp] > MaxCell) MaxCell = CurrCellBlock.Volt[tmp];	//keep track of max and min
					if(CurrCellBlock.Volt[tmp] < MinCell) MinCell = CurrCellBlock.Volt[tmp];
					CellVolt[BatMonCell*4+tmp] = CurrCellBlock.Volt[tmp];	//save Cell voltage
					if (CurrCellBlock.Balance[tmp] == 1)					//use graph array to save balancing status
						CellGraph[BatMonCell*4+tmp] = -1;
					else
						CellGraph[BatMonCell*4+tmp] = 0;
				}
				else
				{
					CellVolt[BatMonCell*4+tmp] = -2;		//flag impossible voltage
					CellGraph[BatMonCell*4+tmp] = 0;
				}
			}
			CellVoltFlag = 0;						//flag request next cell block
			BatMonCell++;							//increment cell block
		}

		State = BATTMON;						//default come back to this state
		switch(GetButtonPress())
		{
		case BTN_MENU:
			DisplayRefresh = 1;					//Flag Display for update
			MenuList = MainMenuText;			//point to main menu text
			MenuStackp = 0;						//clear stack
			Push(BATTMON);						//make sure we come back here
			Push(MAINMENU);						//push main menu state
			State = MENUSETUP;					//go to menu setup
		break;
		case BTN_UP:						//this button switches to Histogram mode
			if(BatGraphFlag == 0)
			{
				SetLEDs(BTN_UP_RED, BTN_UP_MASK);
				BatGraphFlag = 1;
			}
		break;
		case BTN_DOWN:							//this button switches to Bar Graph mode
			if(BatGraphFlag == 1)
			{
				SetLEDs(BTN_DOWN_RED, BTN_DOWN_MASK);
				BatGraphFlag = 0;
			}
		break;
		case BTN_BACK:
			GpioDataRegs.GPATOGGLE.bit.GPIO16 = 1;	//toggle backlight
		break;
		}
	break;			//end case BATTMON

	case FUELG:
		//note Fuel Gauge automatically sets CAN variable 1 to be Tritium Bus Amp Hours
		if (DisplayRefresh)		//do initial screen drawing
			{
				SetLEDs(BTN_BACK_GREEN | BTN_MENU_GREEN,BTN_ALL_MASK);
				box(0,38,127,60);
				status_bar(2,40,125,58,0,2);		//draw empty bar
				draw_block(0, 38, 3, 28, 0xFF);		//draw empty tick
				draw_block(124, 38, 127, 28, 0xFF);	//draw full tick
				draw_block(63, 38, 65, 28, 0xFF);	//draw half tick
				draw_block(32, 38, 34, 33, 0xFF);	//draw quarter tick
				draw_block(94, 38, 96, 33, 0xFF);	//draw 3quarter tick
				set_font(FontLarge);					//medium Font
				set_cursor(0,0);
				print_char('E',0,0);
				set_cursor(112,0);
				print_char('F',0,0);
				DisplayRefresh = 0;
			}

		switch(GetButtonPress())
		{
		case BTN_MENU:
			DisplayRefresh = 1;					//Flag Display for update
			MenuList = MainMenuText;			//point to main menu text
			MenuStackp = 0;						//clear stack
			Push(FUELG);						//make sure we come back here
			Push(MAINMENU);						//push main menu state
			State = MENUSETUP;					//go to menu setup
			break;
		case BTN_BACK:
			GpioDataRegs.GPATOGGLE.bit.GPIO16 = 1;	//toggle backlight
			State = FUELG;							//stay on this state
			break;
		default:
			State = FUELG;						//stay on this state
		}
	break;			//end case FUELG
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
	StopStopWatch(CellVolt_watch);
	StopStopWatch(Menu_watch);
	SETLED0();
	SETLED1();
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
