/*
 * Display.h
 *
 *  Created on: Jan 13, 2014
 *      Author: jennifer
 */

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

extern int GetMenuSelection(const unsigned char List[][20]);
extern unsigned int GetButtonPress();
extern void LCDGpio_init();
extern void ButtonGpioInit();
extern void WriteLCDDataPort(uint8_t Data);
extern uint8_t ReadLCDDataPort();
extern void SetLCDEN(int s);
extern void LCDdelay();
extern void delay_ms(uint16_t ms);
extern void SetLCDControlPort(uint8_t Cmd);

#endif /* DISPLAY_H_ */
