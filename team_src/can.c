/*
 * can.c
 *
 *  Created on: Nov 12, 2013
 *      Author: Nathan
 */
#include "all.h"

unsigned int mask;
stopwatch_struct* can_watch;
stopwatch_struct* cancorder_watch;	//stopwatch for cancorder heartbeat timeout
stopwatch_struct* tritium_watch;	//stopwatch for tritium messages timeout

struct ECAN_REGS ECanaShadow;

#include "CANdbc.h"

can_variable_struct CANvars[8];

void CANSetup()
{

	//copy information from CANdbc to CANvars for defaults
	CANvars[0].SID = CANdbc[VAR1DEFAULT].SID;
	CANvars[0].TypeCode = CANdbc[VAR1DEFAULT].TypeCode;
	CANvars[0].Offset = CANdbc[VAR1DEFAULT].Offset;
	CANvars[0].New = 0;
	memcpy(&CANvars[0].Name, &CANdbcNames[VAR1DEFAULT],22);

	CANvars[1].SID = CANdbc[VAR2DEFAULT].SID;
	CANvars[1].TypeCode = CANdbc[VAR2DEFAULT].TypeCode;
	CANvars[1].Offset = CANdbc[VAR2DEFAULT].Offset;
	CANvars[1].New = 0;
	memcpy(&CANvars[1].Name, &CANdbcNames[VAR2DEFAULT],22);

	CANvars[2].SID = CANdbc[VAR3DEFAULT].SID;
	CANvars[2].TypeCode = CANdbc[VAR3DEFAULT].TypeCode;
	CANvars[2].Offset = CANdbc[VAR3DEFAULT].Offset;
	CANvars[2].New = 0;
	memcpy(&CANvars[2].Name, &CANdbcNames[VAR3DEFAULT],22);

	CANvars[3].SID = CANdbc[VAR4DEFAULT].SID;
	CANvars[3].TypeCode = CANdbc[VAR4DEFAULT].TypeCode;
	CANvars[3].Offset = CANdbc[VAR4DEFAULT].Offset;
	CANvars[3].New = 0;
	memcpy(&CANvars[3].Name, &CANdbcNames[VAR4DEFAULT],22);
	//CANvars[6] is always CANcorder heartbeat
	CANvars[6].SID = CANCORDERHEART_SID;
	CANvars[6].TypeCode = CANCORDERHEART_TYPE;
	CANvars[6].Offset = CANCORDERHEART_OFFSET;
	CANvars[6].New = 0;
	memcpy(&CANvars[6].Name, "CANcorder Heartbeat",20);
	//CANvars[4] is always motor temperature
	CANvars[4].SID = CANMOTORTEMP_SID;
	CANvars[4].TypeCode = CANMOTORTEMP_TYPE;
	CANvars[4].Offset = CANMOTORTEMP_OFFSET;
	CANvars[4].New = 0;
	memcpy(&CANvars[4].Name, "Motor Temp",11);
	//CANvars[5] is always 12V bus voltage
	CANvars[5].SID = CAN12VBUS_SID;
	CANvars[5].TypeCode = CAN12VBUS_TYPE;
	CANvars[5].Offset = CAN12VBUS_OFFSET;
	CANvars[5].New = 0;
	memcpy(&CANvars[5].Name, "12V Bus Voltage",16);
	//CANvars[7] is always Tritium bus voltage
	CANvars[7].SID = TRITIUMVBUS_SID;
	CANvars[7].TypeCode = TRITIUMVBUS_TYPE;
	CANvars[7].Offset = TRITIUMVBUS_OFFSET;
	CANvars[7].New = 0;
	memcpy(&CANvars[7].Name, "Tritium Bus Voltage",20);

	InitECanaGpio();
	InitECana();

	ClearMailBoxes();

	ECanaShadow.CANMIM.all = 0;
	ECanaShadow.CANMIL.all = 0;
	ECanaShadow.CANGIM.all = 0;
	ECanaShadow.CANGAM.bit.AMI = 0; //must be standard
	ECanaShadow.CANGIM.bit.I1EN = 1;  // enable I1EN
	ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
	ECanaShadow.CANME.all = ECanaRegs.CANME.all;

	//todo USER: Node specifc CAN setup
	EALLOW;

	// create mailbox for all Receive and transmit IDs
	// MBOX0 - MBOX31

	//Command RECEIVE
	ECanaMboxes.MBOX0.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX0.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX0.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX0.MSGID.bit.STDMSGID = COMMAND_ID;
	ECanaShadow.CANMD.bit.MD0 = 1;			//receive
	ECanaShadow.CANME.bit.ME0 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM0  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL0  = 1;  		// Int.-Level MB#0  -> I1EN

	//Heart TRANSMIT
	ECanaMboxes.MBOX1.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX1.MSGID.bit.AME = 0; 	// all bit must match
	ECanaMboxes.MBOX1.MSGID.bit.AAM = 1; 	//RTR AUTO TRANSMIT
	ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX1.MSGID.bit.STDMSGID = HEARTBEAT_ID;
	ECanaShadow.CANMD.bit.MD1 = 0; 			//transmit
	ECanaShadow.CANME.bit.ME1 = 1;			//enable

	//CAN Variable 1 RECEIVE
	ECanaMboxes.MBOX2.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX2.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX2.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX2.MSGID.bit.STDMSGID = CANdbc[VAR1DEFAULT].SID;
	ECanaShadow.CANMD.bit.MD2 = 1;			//receive
	ECanaShadow.CANME.bit.ME2 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM2  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL2  = 1;  		// Int.-Level MB#0  -> I1EN

	//CAN Variable 2 RECEIVE
	ECanaMboxes.MBOX3.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX3.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX3.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX3.MSGID.bit.STDMSGID = CANdbc[VAR2DEFAULT].SID;
	ECanaShadow.CANMD.bit.MD3 = 1;			//receive
	ECanaShadow.CANME.bit.ME3 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM3  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL3  = 1;  		// Int.-Level MB#0  -> I1EN

	//CAN Variable 3 RECEIVE
	ECanaMboxes.MBOX4.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX4.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX4.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX4.MSGID.bit.STDMSGID = CANdbc[VAR3DEFAULT].SID;
	ECanaShadow.CANMD.bit.MD4 = 1;			//receive
	ECanaShadow.CANME.bit.ME4 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM4  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL4  = 1;  		// Int.-Level MB#0  -> I1EN

	//CAN Variable 4 RECEIVE
	ECanaMboxes.MBOX5.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX5.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX5.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX5.MSGID.bit.STDMSGID = CANdbc[VAR4DEFAULT].SID;
	ECanaShadow.CANMD.bit.MD5 = 1;			//receive
	ECanaShadow.CANME.bit.ME5 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM5  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL5  = 1;  		// Int.-Level MB#0  -> I1EN

	//CANcorder heartbeat RECEIVE
	ECanaMboxes.MBOX6.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX6.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX6.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX6.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX6.MSGID.bit.STDMSGID = CANCORDERHEART_SID;
	ECanaShadow.CANMD.bit.MD6 = 1;			//receive
	ECanaShadow.CANME.bit.ME6 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM6  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL6  = 1;  		// Int.-Level MB#0  -> I1EN

	//Motor Temp RECEIVE
	ECanaMboxes.MBOX7.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX7.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX7.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX7.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX7.MSGID.bit.STDMSGID = CANMOTORTEMP_SID;
	ECanaShadow.CANMD.bit.MD7 = 1;			//receive
	ECanaShadow.CANME.bit.ME7 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM7  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL7  = 1;  		// Int.-Level MB#0  -> I1EN

	//12V Bus voltage RECEIVE
	ECanaMboxes.MBOX8.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX8.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX8.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX8.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX8.MSGID.bit.STDMSGID = CAN12VBUS_SID;
	ECanaShadow.CANMD.bit.MD8 = 1;			//receive
	ECanaShadow.CANME.bit.ME8 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM8  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL8  = 1;  		// Int.-Level MB#0  -> I1EN

	//Tritium Bus voltage RECEIVE
	ECanaMboxes.MBOX9.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX9.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX9.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX9.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX9.MSGID.bit.STDMSGID = TRITIUMVBUS_SID;
	ECanaShadow.CANMD.bit.MD9 = 1;			//receive
	ECanaShadow.CANME.bit.ME9 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM9  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL9  = 1;  		// Int.-Level MB#0  -> I1EN

	ECanaRegs.CANGAM.all = ECanaShadow.CANGAM.all;
	ECanaRegs.CANGIM.all = ECanaShadow.CANGIM.all;
	ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
	ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
	ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
	ECanaRegs.CANME.all = ECanaShadow.CANME.all;
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.STM = 0;    // No self-test mode
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    EDIS;

    //ENABLE PIE INTERRUPTS
    IER |= M_INT9;
    PieCtrlRegs.PIEIER9.bit.INTx6= 1;

    can_watch = StartStopWatch(SENDCAN_STOPWATCH);
    cancorder_watch = StartStopWatch(CANCORDER_TIMEOUT);
    tritium_watch = StartStopWatch(TRITIUM_TIMEOUT);
}

void ClearMailBoxes()
{
    ECanaMboxes.MBOX0.MDH.all = 0;
    ECanaMboxes.MBOX0.MDL.all = 0;
    ECanaMboxes.MBOX1.MDH.all = 0;
    ECanaMboxes.MBOX1.MDL.all = 0;
    ECanaMboxes.MBOX2.MDH.all = 0;
    ECanaMboxes.MBOX2.MDL.all = 0;
    ECanaMboxes.MBOX3.MDH.all = 0;
    ECanaMboxes.MBOX3.MDL.all = 0;
    ECanaMboxes.MBOX4.MDH.all = 0;
    ECanaMboxes.MBOX4.MDL.all = 0;
    ECanaMboxes.MBOX5.MDH.all = 0;
    ECanaMboxes.MBOX5.MDL.all = 0;
    ECanaMboxes.MBOX6.MDH.all = 0;
    ECanaMboxes.MBOX6.MDL.all = 0;
    ECanaMboxes.MBOX7.MDH.all = 0;
    ECanaMboxes.MBOX7.MDL.all = 0;
    ECanaMboxes.MBOX8.MDH.all = 0;
    ECanaMboxes.MBOX8.MDL.all = 0;
    ECanaMboxes.MBOX9.MDH.all = 0;
    ECanaMboxes.MBOX9.MDL.all = 0;
    ECanaMboxes.MBOX10.MDH.all = 0;
    ECanaMboxes.MBOX10.MDL.all = 0;
    ECanaMboxes.MBOX11.MDH.all = 0;
    ECanaMboxes.MBOX11.MDL.all = 0;
    ECanaMboxes.MBOX12.MDH.all = 0;
    ECanaMboxes.MBOX12.MDL.all = 0;
    ECanaMboxes.MBOX13.MDH.all = 0;
    ECanaMboxes.MBOX13.MDL.all = 0;
    ECanaMboxes.MBOX14.MDH.all = 0;
    ECanaMboxes.MBOX14.MDL.all = 0;
    ECanaMboxes.MBOX15.MDH.all = 0;
    ECanaMboxes.MBOX15.MDL.all = 0;
    ECanaMboxes.MBOX16.MDH.all = 0;
    ECanaMboxes.MBOX16.MDL.all = 0;
    ECanaMboxes.MBOX17.MDH.all = 0;
    ECanaMboxes.MBOX17.MDL.all = 0;
    ECanaMboxes.MBOX18.MDH.all = 0;
    ECanaMboxes.MBOX18.MDL.all = 0;
    ECanaMboxes.MBOX19.MDH.all = 0;
    ECanaMboxes.MBOX19.MDL.all = 0;
    ECanaMboxes.MBOX20.MDH.all = 0;
    ECanaMboxes.MBOX20.MDL.all = 0;
    ECanaMboxes.MBOX21.MDH.all = 0;
    ECanaMboxes.MBOX21.MDL.all = 0;
    ECanaMboxes.MBOX22.MDH.all = 0;
    ECanaMboxes.MBOX22.MDL.all = 0;
    ECanaMboxes.MBOX23.MDH.all = 0;
    ECanaMboxes.MBOX23.MDL.all = 0;
    ECanaMboxes.MBOX24.MDH.all = 0;
    ECanaMboxes.MBOX24.MDL.all = 0;
    ECanaMboxes.MBOX25.MDH.all = 0;
    ECanaMboxes.MBOX25.MDL.all = 0;
    ECanaMboxes.MBOX26.MDH.all = 0;
    ECanaMboxes.MBOX26.MDL.all = 0;
    ECanaMboxes.MBOX27.MDH.all = 0;
    ECanaMboxes.MBOX27.MDL.all = 0;
    ECanaMboxes.MBOX28.MDH.all = 0;
    ECanaMboxes.MBOX28.MDL.all = 0;
    ECanaMboxes.MBOX29.MDH.all = 0;
    ECanaMboxes.MBOX30.MDL.all = 0;
    ECanaMboxes.MBOX30.MDH.all = 0;
    ECanaMboxes.MBOX31.MDL.all = 0;
    ECanaMboxes.MBOX31.MDH.all = 0;
}

char FillCAN(unsigned int Mbox)
{
	//todo USER: setup for all transmit MBOXs
	struct ECAN_REGS ECanaShadow;
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	switch (Mbox)								//choose mailbox
	{
	case HEARTBEAT_BOX:
		//todo Nathan define heartbeat
		EALLOW;
		ECanaShadow.CANMC.bit.MBNR = Mbox;
		ECanaShadow.CANMC.bit.CDR = 1;
		ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
		ECanaMboxes.MBOX1.MDH.all = 0;
		ECanaMboxes.MBOX1.MDL.all = 0;
		ECanaMboxes.MBOX1.MDL.word.LOW_WORD = ops.Flags.all;
		ECanaShadow.CANMC.bit.MBNR = 0;
		ECanaShadow.CANMC.bit.CDR = 0;
		ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
		EDIS;
		return 1;
	}
	return 0;
}

void FillSendCAN(unsigned Mbox)
{
	if (FillCAN(Mbox) == 1)
	{
		SendCAN(Mbox);
	}
}

void SendCAN(unsigned int Mbox)
{
	mask = 1 << Mbox;
	ECanaRegs.CANTRS.all = mask;

	//todo Nathan: calibrate sendcan stopwatch
	StopWatchRestart(can_watch);

	do{ECanaShadow.CANTA.all = ECanaRegs.CANTA.all;}
	while(((ECanaShadow.CANTA.all & mask) != mask) && (isStopWatchComplete(can_watch) == 0)); //wait to send or hit stop watch

	ECanaRegs.CANTA.all = mask;						//clear flag
	if (isStopWatchComplete(can_watch) == 1)					//if stopwatch flag
	{
		ops.Flags.bit.can_error = 1;
	}
	else if (ops.Flags.bit.can_error == 1)		//if no stopwatch and flagged reset
	{
		ops.Flags.bit.can_error = 0;
	}
}


void FillCANData()
{

}

// INT9.6
__interrupt void ECAN1INTA_ISR(void)  // eCAN-A
{
	Uint32 ops_id;
	Uint32 dummy;
  	unsigned int mailbox_nr;
  	ECanaShadow.CANGIF1.bit.MIV1 =  ECanaRegs.CANGIF1.bit.MIV1;
  	mailbox_nr = ECanaShadow.CANGIF1.bit.MIV1;
  	//todo USER: Setup ops command
  	switch(mailbox_nr)
  	{
  	case COMMAND_BOX:
  			//todo Nathan: Define Command frame
  			//proposed:
  			//HIGH 4 BYTES = Uint32 ID
  			//LOW 4 BYTES = Uint32 change to
  			ops_id = ECanaMboxes.MBOX0.MDH.all;
  			dummy = ECanaMboxes.MBOX0.MDL.all;
			switch (ops_id)
			{
			case OPS_ID_STATE:
				memcpy(&ops.State,&dummy,sizeof ops.State);
				ops.Change.bit.State = 1;
				break;
			case OPS_ID_STOPWATCHERROR:
				memcpy(&ops.Flags.all,&dummy,sizeof ops.Flags.all);
				ops.Change.bit.Flags = 1;
				break;
			}
			ECanaRegs.CANRMP.bit.RMP0 = 1;
	break;

		//todo USER: Setup other reads
	case VARIABLE1_BOX:
		CANvars[0].data.U32 = ECanaMboxes.MBOX2.MDH.all;
		CANvars[0].data.U64 = CANvars[0].data.U64 << 32L;
		CANvars[0].data.U32 = ECanaMboxes.MBOX2.MDL.all;
		CANvars[0].data.U64 = CANvars[0].data.U64 >> CANvars[0].Offset;
		CANvars[0].New = 1;
	break;

	case VARIABLE2_BOX:
		CANvars[1].data.U32 = ECanaMboxes.MBOX3.MDH.all;
		CANvars[1].data.U64 = CANvars[1].data.U64 << 32L;
		CANvars[1].data.U32 = ECanaMboxes.MBOX3.MDL.all;
		CANvars[1].data.U64 = CANvars[1].data.U64 >> CANvars[1].Offset;
		CANvars[2].New = 1;
	break;

	case VARIABLE3_BOX:
		CANvars[2].data.U32 = ECanaMboxes.MBOX4.MDH.all;
		CANvars[2].data.U64 = CANvars[2].data.U64 << 32L;
		CANvars[2].data.U32 = ECanaMboxes.MBOX4.MDL.all;
		CANvars[2].data.U64 = CANvars[2].data.U64 >> CANvars[2].Offset;
		CANvars[2].New = 1;
	break;

	case VARIABLE4_BOX:
		CANvars[3].data.U32 = ECanaMboxes.MBOX5.MDH.all;
		CANvars[3].data.U64 = CANvars[3].data.U64 << 32L;
		CANvars[3].data.U32 = ECanaMboxes.MBOX5.MDL.all;
		CANvars[3].data.U64 = CANvars[3].data.U64 >> CANvars[3].Offset;
		CANvars[3].New = 1;
	break;

	case CANCORDERHEART_BOX:
		StopWatchRestart(cancorder_watch);
		CANvars[6].data.U32 = ECanaMboxes.MBOX6.MDH.all;
		CANvars[6].data.U64 = CANvars[6].data.U64 << 32L;
		CANvars[6].data.U32 = ECanaMboxes.MBOX6.MDL.all;
		CANvars[6].data.U64 = CANvars[6].data.U64 >> CANvars[6].Offset;
		CANvars[6].New = 1;
	break;

	case CANMOTORTEMP_BOX:
		StopWatchRestart(tritium_watch);
		CANvars[4].data.U32 = ECanaMboxes.MBOX7.MDH.all;
		CANvars[4].data.U64 = CANvars[4].data.U64 << 32L;
		CANvars[4].data.U32 = ECanaMboxes.MBOX7.MDL.all;
		CANvars[4].data.U64 = CANvars[4].data.U64 >> CANvars[4].Offset;
		CANvars[4].New = 1;
	break;

	case CAN12VBUS_BOX:
		CANvars[5].data.U32 = ECanaMboxes.MBOX8.MDH.all;
		CANvars[5].data.U64 = CANvars[5].data.U64 << 32L;
		CANvars[5].data.U32 = ECanaMboxes.MBOX8.MDL.all;
		CANvars[5].data.U64 = CANvars[5].data.U64 >> CANvars[5].Offset;
		CANvars[5].New = 1;
	break;

	case TRITIUMVBUS_BOX:
			StopWatchRestart(tritium_watch);
			CANvars[7].data.U32 = ECanaMboxes.MBOX8.MDH.all;
			CANvars[7].data.U64 = CANvars[5].data.U64 << 32L;
			CANvars[7].data.U32 = ECanaMboxes.MBOX8.MDL.all;
			CANvars[7].data.U64 = CANvars[5].data.U64 >> CANvars[5].Offset;
			CANvars[7].New = 1;
		break;
  }


  	//To receive more interrupts from this PIE group, acknowledge this interrupt
  	PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

