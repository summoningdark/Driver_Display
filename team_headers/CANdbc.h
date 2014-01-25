/*
 * CANdbc.h
 *
 *  Created on: Jan 17, 2014
 *      Author: jennifer
 */

#include "data.h"

#define NUMCANVARS 10

#define VAR1DEFAULT 0
#define VAR2DEFAULT 1
#define VAR3DEFAULT 2
#define VAR4DEFAULT 3


//This holds all the CAN variables on our system. the format is simple, each array element is a struct of {uint16,uint16,uint16}
//with the first member(SID) being the CAN ID the variable is transmitted on, the second member(TypeCode) indicates what type
//the variable is, and the last member(Offset) is bow many bits from the LSB the variable starts.

const can_variable_list_struct CANdbc[NUMCANVARS]={	{1,0,0},
							{2,1,5},
							{3,2,2},
							{4,3,0},
							{5,4,32},
							{6,5,8},
							{7,6,23},
							{8,7,0},
							{9,8,0},
							{10,9,0}
};

//always end the CANdbcNames array with an empty string, the menu function needs this to know how many entries there are
//note only 21 characters are printable on a line
const char CANdbcNames[NUMCANVARS+1][22]={	"int8 offset 0",
				"uint8 offset 5",
				"int16 offset 2",
				"uint16 offset 0",
				"int32 offset 32",
				"uint32 offset 8",
				"float32 offset 23",
				"int64 offset 0",
				"uint64 offset 0",
				"float64 offset 0",
				"",
};
