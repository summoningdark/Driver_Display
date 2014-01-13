/*
 * data.h
 *
 *  Created on: Oct 30, 2013
 *      Author: Nathan
 */

#ifndef DATA_H_
#define DATA_H_


typedef struct DATA
{
  unsigned long adc;
  char gp_button;
} data_struct;

//Size of the button press queue
#define BUTTON_QUEUE_SIZE 10
//number of ticks (CPU-Timer0 interrupts, so 100 uS/tick) before a switch press is considered valid 100 = .01s
#define BUTTON_DEBOUNCE_TICKS 100
//number of ticks before a button is considered held (another press is registered) 5000 = .5s
#define BUTTON_HOLD_TICKS 5000
//number of ticks before a held button repetes a press. 3000 = .3s
#define BUTTON_REPETE_TICKS 3000

typedef struct BUTTONQUEUE
{
	unsigned int Full;
	unsigned int Empty;
	unsigned int Current;
	unsigned int Next;
	unsigned int Queue[BUTTON_QUEUE_SIZE];
} button_queue_struct;

#endif /* DATA_H_ */
