/* 
 * File:   CANparser.h
 * Author: jennifer
 *
 * Created on May 3, 2014, 3:31 PM
 * This is the header file to include in any code which will call the CANparser functions
 * it includes all the necessary definitions, etc
 *
 */

#ifndef CANPARSER_H
#define	CANPARSER_H

#include "CANdbVars.h"

//function prototypes
void processCANmessage (mID *message);      //called in CAN recieve interrupt to update the CAN variable array

#endif	/* CANPARSER_H */

