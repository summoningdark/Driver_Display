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

#define COMMAND_BOX 	0
#define VARIABLE1_BOX	2
#define VARIABLE2_BOX	3
#define VARIABLE3_BOX	4
#define VARIABLE4_BOX	5
#define CANCORDERHEART_BOX	6
#define CANMOTORTEMP_BOX	7
#define CAN12VBUS_BOX	8
#define TRITIUMVBUS_BOX 9

#define COMMAND_ID 		0x1
#define HEARTBEAT_ID 	0xff

#define HEARTBEAT_BOX 	1

#define CANCORDERHEART_SID		0x96
#define CANCORDERHEART_TYPE		0x00
#define CANCORDERHEART_OFFSET	0x00

#define CANMOTORTEMP_SID		0x400 + 0x0B
#define CANMOTORTEMP_TYPE		0x06
#define CANMOTORTEMP_OFFSET		0x00

#define CAN12VBUS_SID			0x100
#define CAN12VBUS_TYPE			0x06
#define CAN12VBUS_OFFSET		0x00

#define TRITIUMVBUS_SID			0x400 + 0x02
#define TRITIUMVBUS_TYPE		0x06
#define TRITIUMVBUS_OFFSET		0x00

#define CANCORDER_TIMEOUT		150000L
#define TRITIUM_TIMEOUT			100000L

extern can_variable_struct CANvars[8];

#endif /* CAN_H_ */
