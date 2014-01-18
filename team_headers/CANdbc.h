/*
 * CANdbc.h
 *
 *  Created on: Jan 17, 2014
 *      Author: jennifer
 */

#include "data.h"

#define NUMCANVARS 4

#define VAR1DEFAULT 0
#define VAR2DEFAULT 1
#define VAR3DEFAULT 2
#define VAR4DEFAULT 3


//This holds all the CAN variables on our system. the format is simple, each array element is a struct of {uint16,uint16,uint16}
//with the first member(SID) being the CAN ID the variable is transmitted on, the second member(TypeCode) indicates what type
//the variable is, and the last member(Offset) is bow many bits from the LSB the variable starts.

const can_variable_list_struct CANdbc[NUMCANVARS]={	{0,0,0},
													{1,0,0},
													{2,0,0},
													{3,0,0}
};

//always end the CANdbcNames array with an empty string, the menu function needs this to know how many entries there are
const unsigned char CANdbcNames[][20]={	"Variable 1",
											"Variable 2",
											"Variable 3",
											"Variable 4",
											"",
};
