/*
 * menus.h
 *
 *  Created on: Jan 13, 2014
 *      Author: jennifer
 */

#ifndef MENUS_H_
#define MENUS_H_


#define MM_RACEMODE 0
#define MM_DISPLAY4 1
#define MM_CV1 		2
#define MM_CV2 		3
#define MM_CV3 		4
#define MM_CV4 		5

//note only 21 chars are printable on a line.
const char MainMenuText[][22]={	"Race Mode",
											"Display 4",
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

#endif /* MENUS_H_ */
