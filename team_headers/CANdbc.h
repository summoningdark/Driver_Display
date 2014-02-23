/*
 * CANdbc.h
 *
 *  Created on: Jan 17, 2014
 *      Author: jennifer
 */

#include "data.h"

#define NUMCANVARS 23

#define VAR1DEFAULT 6
#define VAR2DEFAULT 1
#define VAR3DEFAULT 2
#define VAR4DEFAULT 3


//This holds all the CAN variables on our system. the format is simple, each array element is a struct of {uint16,uint16,uint16}
//with the first member(SID) being the CAN ID the variable is transmitted on, the second member(TypeCode) indicates what type
//the variable is, and the last member(Offset) is bow many bits from the LSB the variable starts.
//typeCodes:
// 0 int8
// 1 uint8
// 2 int16
// 3 uint16
// 4 int32
// 5 uint32
// 6 float32
// 7 int64
// 8 uint64
// 9 float64
// 10 Time
// 11 Latitude/Longitude coordinate
const can_variable_list_struct CANdbc[NUMCANVARS]={	{0x401,5,0},
													{0x402,6,32},
													{0x403,6,0},
													{0x403,6,32},
													{0x404,6,0},
													{0x40E,6,32},
													{0x40E,6,0},
													{0x501,6,32},
													{0x109,2,0},
													{0x107,2,0},
													{0x108,2,0},
													{0x10A,2,0},
													{0x10B,11,0},
													{0x10C,11,0},
													{0x10E,10,0},
													{0x300,3,0},
													{0x300,3,32},
													{0x302,3,0},
													{0x302,3,32},
													{0x304,3,0},
													{0x304,3,32},
													{0x306,3,0},
													{0x306,3,32},
};

//always end the CANdbcNames array with an empty string, the menu function needs this to know how many entries there are
//note only 21 characters are printable on a line
const char CANdbcNames[NUMCANVARS+1][22]={	"Tritium Flags",
				"Tritium Bus I",
				"Motor RPM",
				"Speed",
				"Motor Current",
				"Bus AmpHour",
				"Trip Odo (m)",
				"Command Current",
				"Coolant Low",
				"Coolant Motor",
				"Coolant Controller",
				"Ambient Temp",
				"Latitude",
				"Longitude",
				"Time",
				"BIM1 Max",
				"BIM1 Min",
				"BIM2 Max",
				"BIM2 Min",
				"BIM3 Max",
				"BIM3 Min",
				"BIM4 Max",
				"BIM4 Min",
				""
};
