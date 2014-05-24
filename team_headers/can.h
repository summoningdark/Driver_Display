/*
 * can.h
 *
 *  Created on: Nov 13, 2013
 *      Author: Nathan
 */

#ifndef CAN_H_
#define CAN_H_




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

#define CANCORDERHEART_BOX	30
#define CELLVOLT_BOX		31

#define COMMAND_ID 		0x35
#define HEARTBEAT_ID 	0x34

#define HEARTBEAT_BOX 	1

#define CANCORDERHEART_SID		0xAA

extern char CellVoltFlag;
extern CellBlock CurrCellBlock;

#endif /* CAN_H_ */
