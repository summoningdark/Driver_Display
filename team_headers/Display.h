/*
 * Display.h
 *
 *  Created on: Jan 13, 2014
 *      Author: jennifer
 */
#include "data.h"

#ifndef DISPLAY_H_
#define DISPLAY_H_

//button flag defines that match the board silkscreen and schematic
#define BTN0 0x0008
#define BTN1 0x0004
#define BTN2 0x0002
#define BTN3 0x0001
#define BTN4 0x0010

//button flag defines that match function
#define BTN_BACK	0x0001
#define BTN_UP		0x0002
#define BTN_DOWN	0x0004
#define BTN_SELECT	0x0008
#define BTN_MENU	0x0010

//LED defines used as:  XXX[color] | XXX[color] ...
//LED word [15:0] = Btn0[0],Btn4[0],Btn3[1],Btn2[1],Btn1[0],Btn4[1],Btn0[1],Btn1[1],Btn2[0],Yellow2,Green2,Red2,Btn3[0],Red1,Green1,Yellow1
//bits for the different colors
#define BTN0RED				0x8000
#define BTN1RED				0x0800
#define BTN2RED				0x0080
#define BTN3RED				0x0008
#define BTN4RED				0x4000
#define BTN0GREEN			0x0200
#define BTN1GREEN			0x0100
#define BTN2GREEN			0x1000
#define BTN3GREEN			0x2000
#define BTN4GREEN			0x0400
#define BTN0MASK			0x7DFF
#define BTN1MASK			0xF6FF
#define BTN2MASK			0xEF7F
#define BTN3MASK			0xDFF7
#define BTN4MASK			0xBBFF
#define BTN_BACK_MASK		BTN3MASK
#define BTN_UP_MASK			BTN2MASK
#define BTN_DOWN_MASK		BTN1MASK
#define BTN_SELECT_MASK		BTN0MASK
#define BTN_MENU_MASK		BTN4MASK
#define BTN_ALL_MASK		(BTN0MASK & BTN1MASK & BTN2MASK & BTN3MASK & BTN4MASK)
#define BTN_BACK_GREEN		BTN3GREEN
#define BTN_UP_GREEN		BTN2GREEN
#define BTN_DOWN_GREEN		BTN1GREEN
#define BTN_SELECT_GREEN	BTN0GREEN
#define BTN_MENU_GREEN		BTN4GREEN
#define BTN_BACK_RED		BTN3RED
#define BTN_UP_RED			BTN2RED
#define BTN_DOWN_RED		BTN1RED
#define BTN_SELECT_RED		BTN0RED
#define BTN_MENU_RED		BTN4RED
#define IND1MASK		0xFFF8
#define IND1OFF			0x0007
#define IND1RED			0x0003
#define IND1YELLOW		0x0006
#define IND1GREEN		0x0005
#define IND2MASK		0xFF8F
#define IND2OFF			0x0070
#define IND2RED			0x0060
#define IND2YELLOW		0x0030
#define IND2GREEN		0x0050

extern void LCD_bl(int i);
extern void Buttons();
extern void SetLEDs(uint16_t LEDword, uint16_t LEDmask);
extern int GetMenuSelection(const unsigned char List[][22]);
extern unsigned int GetButtonPress();
extern void LEDGpio_init();
extern void LCDGpio_init();
extern void ButtonGpioInit();
extern void WriteLCDDataPort(uint8_t Data);
extern uint8_t ReadLCDDataPort();
extern void SetLCDEN(int s);
extern void LCDdelay();
extern void delay_ms(uint16_t ms);
extern void SetLCDControlPort(uint8_t Cmd);
extern void SetCANmonitor(uint8_t N, can_variable_list_struct CANvar);
extern void PrintCANvariable(uint8_t N, uint8_t reduced);

#endif /* DISPLAY_H_ */
