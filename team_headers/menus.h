/*
 * menus.h
 *
 *  Created on: Jan 13, 2014
 *      Author: jennifer
 */

#ifndef MENUS_H_
#define MENUS_H_

//menu lists
//make sure to update defines if the order is changed
#define RACE_MODE	0
#define DISPLAY4	1
#define CNGVAR1		2
#define CNGVAR2		3
#define CNGVAR3		4
#define CNGVAR4		5

//note only 21 chars are printable on a line.
const unsigned char MainMenuText[][22]={	"Race Mode",
											"Display 4",
											"Choose Watch 1",
											"Choose Watch 2",
											"Choose Watch 3",
											"Choose Watch 4",
											""};



#endif /* MENUS_H_ */
