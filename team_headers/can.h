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
#define COMMAND_ID 		0x1
#define COMMAND_BOX 	0
#define VARIABLE1_BOX	2
#define VARIABLE2_BOX	3
#define VARIABLE3_BOX	4
#define VARIABLE4_BOX	5

#define HEARTBEAT_ID 	0x0
#define HEARTBEAT_BOX 	1

extern can_variable_struct CANvars[4];

#endif /* CAN_H_ */
