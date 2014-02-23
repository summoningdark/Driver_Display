/*
 * menus.h
 *
 *  Created on: Jan 13, 2014
 *      Author: jennifer
 */

#ifndef MENUS_H_
#define MENUS_H_


#define MM_RACEMODE 0
#define MM_TESTMODE 1
#define MM_DISPLAY4 2
#define MM_BATTMON	3
#define MM_CV1 		4
#define MM_CV2 		5
#define MM_CV3 		6
#define MM_CV4 		7

//note only 21 chars are printable on a line.
const char MainMenuText[][22]={				"Race Mode",
											"Test Mode",
											"Display 4",
											"Battery Monitor",
											"Choose Watch 1",
											"Choose Watch 2",
											"Choose Watch 3",
											"Choose Watch 4",
											""};

#define VM_LIST 	0
#define VM_MANUAL	1

const char VariableMenuText[][22]={
											"Choose from List",
											"Enter Manually",
											""
};

const char VariableTypeText[][22]={	"I8",
									"U8",
									"I16",
									"U16",
									"I32",
									"U32",
									"F32",
									"I64",
									"U64",
									"F64",
									"TIME",
									"COORD",
									""
};

#endif /* MENUS_H_ */
