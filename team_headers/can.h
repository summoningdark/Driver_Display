/*
 * can.h
 *
 *  Created on: Nov 13, 2013
 *      Author: Nathan
 */

#ifndef CAN_H_
#define CAN_H_


void BUS_OFF();

struct CANmsg {
   char MBox;
   union CANMSGCTRL_REG   MSGCTRL;
   union CANMDL_REG       MDL;
   union CANMDH_REG       MDH;
};

struct TRS_REG {
	union CANTRS_REG	TRS;
};

void CANSetup();
char FillCAN(unsigned int Mbox);
void SendCAN(unsigned int Mbox);
void FillCANData();
void FillSendCAN(unsigned int Mbox);
void ClearMailBoxes();

//todo USER: DEFINE IDs and mailboxes for output

#define COMMAND_BOX 		0
#define VARIABLE1_BOX		2
#define VARIABLE2_BOX		3
#define VARIABLE3_BOX		4
#define VARIABLE4_BOX		5
#define CANMOTORTEMP_BOX	6
#define CAN12VBUS_BOX		7
#define TRITIUMVBUS_BOX 	8
#define GPSLAT_BOX			9
#define TRITIUMAH_BOX		10
#define CANCORDERHEART_BOX	30
#define CELLVOLT_BOX		11

#define COMMAND_ID 		0x35
#define HEARTBEAT_ID 	0x34

#define HEARTBEAT_BOX 	1

#define CANCORDERHEART_SID		0xAA
#define CANCORDERHEART_TYPE		0x00
#define CANCORDERHEART_OFFSET	0x00

#define CANMOTORTEMP_SID		0x400 + 0x0B
#define CANMOTORTEMP_TYPE		0x06
#define CANMOTORTEMP_OFFSET		0x00

#define CAN12VBUS_SID			0x106
#define CAN12VBUS_TYPE			0x06
#define CAN12VBUS_OFFSET		0x00

#define GPSLAT_SID				0x10b
#define GPSLAT_TYPE				0x06
#define GPSLAT_OFFSET			0x00

#define TRITIUMVBUS_SID			0x400 + 0x02
#define TRITIUMVBUS_TYPE		0x06
#define TRITIUMVBUS_OFFSET		0x00

#define TRITIUMAH_SID			0x400 + 0x0E
#define TRITIUMAH_TYPE			0x06
#define TRITIUMAH_OFFSET		32

#define NUM_CANVARS 10
extern can_variable_struct CANvars[NUM_CANVARS];

extern char CellVoltFlag;
extern CellBlock CurrCellBlock;

#endif /* CAN_H_ */
