/*
 * can.c
 *
 *  Created on: Nov 12, 2013
 *      Author: Nathan
 */
#include "all.h"

extern unsigned int GPSvalid;
unsigned int mask;

stopwatch_struct* can_watch;

struct ECAN_REGS ECanaShadow;

#include "CANparser.h"

//this array holds all the live CAN variable data
volatile LiveCANvariable LiveCANdata[CANDATAENTRIES+1];		//add 1 on the end for the cancorder heartbeat
//this static array holds the information to parse a variable out of a can message
const can_variable_struct CANVariabledb[CANDATAENTRIES] = CANDATAINITVALUES;
//this static array holds the mapping from the CAN ID to variables
const canDB CANdatabase0[5][10] = CANDBINITVALUES;

cell_can_union CanCell;
char CellVoltFlag = 0;
CellBlock CurrCellBlock;

void CANSetup()
{
	int i;

	//clear all LiveCanData data fields
	//init all LiveCanData timeouts
	for(i=0;i<CANDATAENTRIES+1;i++){
		LiveCanData[i].data = 0;
		LiveCanData[i].Timeout = StartStopWatch(500000L);	//CAN variables timeout after 5 seconds
	}

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
	ECanaMboxes.MBOX1.MSGID.bit.IDE = 0;	 //standard id
	ECanaMboxes.MBOX1.MSGID.bit.AME = 0; 	// all bit must match
	ECanaMboxes.MBOX1.MSGID.bit.AAM = 1; 	//RTR AUTO TRANSMIT
	ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX1.MSGID.bit.STDMSGID = HEARTBEAT_ID;
	ECanaShadow.CANMD.bit.MD1 = 0; 			//transmit
	ECanaShadow.CANME.bit.ME1 = 1;			//enable

	//todo set up mailboxes 2-6 for blocks of 16 CAN IDs
	//CAN mailboxes 2 to 6 are general receive mailboxes

	//mailbox 2 handles can IDs 0x100-0x10F
	ECanaMboxes.MBOX2.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX2.MSGID.bit.AME = 1;	//must match mask
	ECanaLAMRegs.LAM2.all = 0x00000FF0;		//mask for last 4 bits
	ECanaMboxes.MBOX2.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX2.MSGID.bit.STDMSGID = 0x100;
	ECanaShadow.CANMD.bit.MD2 = 1;			//receive
	ECanaShadow.CANME.bit.ME2 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM2  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL2  = 1;  		//Int.-Level MB#0  -> I1EN

	ECanaMboxes.MBOX3.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX3.MSGID.bit.AME = 1;	// all bit must match
	ECanaLAMRegs.LAM3.all = 0x00000FF0;		//mask for last 4 bits
	ECanaMboxes.MBOX3.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX3.MSGID.bit.STDMSGID = CANdbc[VAR2DEFAULT].SID;
	ECanaShadow.CANMD.bit.MD3 = 1;			//receive
	ECanaShadow.CANME.bit.ME3 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM3  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL3  = 1;  		// Int.-Level MB#0  -> I1EN

	ECanaMboxes.MBOX4.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX4.MSGID.bit.AME = 1;	// all bit must match
	ECanaLAMRegs.LAM4.all = 0x00000FF0;		//mask for last 4 bits
	ECanaMboxes.MBOX4.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX4.MSGID.bit.STDMSGID = CANdbc[VAR3DEFAULT].SID;
	ECanaShadow.CANMD.bit.MD4 = 1;			//receive
	ECanaShadow.CANME.bit.ME4 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM4  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL4  = 1;  		// Int.-Level MB#0  -> I1EN

	ECanaMboxes.MBOX5.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX5.MSGID.bit.AME = 1;	// all bit must match
	ECanaLAMRegs.LAM5.all = 0x00000FF0;		//mask for last 4 bits
	ECanaMboxes.MBOX5.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX5.MSGID.bit.STDMSGID = CANdbc[VAR4DEFAULT].SID;
	ECanaShadow.CANMD.bit.MD5 = 1;			//receive
	ECanaShadow.CANME.bit.ME5 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM5  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL5  = 1;  		// Int.-Level MB#0  -> I1EN

	ECanaMboxes.MBOX6.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX6.MSGID.bit.AME = 1;	// all bit must match
	ECanaLAMRegs.LAM6.all = 0x00000FF0;		//mask for last 4 bits
	ECanaMboxes.MBOX6.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX6.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX6.MSGID.bit.STDMSGID = CANMOTORTEMP_SID;
	ECanaShadow.CANMD.bit.MD6 = 1;			//receive
	ECanaShadow.CANME.bit.ME6 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM6  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL6  = 1;  		// Int.-Level MB#0  -> I1EN

	//CANcorder heartbeat RECEIVE
	ECanaMboxes.MBOX30.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX30.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX30.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX30.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX30.MSGID.bit.STDMSGID = CANCORDERHEART_SID;
	ECanaShadow.CANMD.bit.MD30 = 1;			//receive
	ECanaShadow.CANME.bit.ME30 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM30  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL30  = 1; 		// Int.-Level MB#0  -> I1EN

	//Cell voltage RECEIVE
	ECanaMboxes.MBOX31.MSGID.bit.IDE = 0; 	//standard id
	ECanaMboxes.MBOX31.MSGID.bit.AME = 0;	// all bit must match
	ECanaMboxes.MBOX31.MSGID.bit.AAM = 0; 	// no RTR AUTO TRANSMIT
	ECanaMboxes.MBOX31.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX31.MSGID.bit.STDMSGID = 0x310;
	ECanaShadow.CANMD.bit.MD31 = 1;			//receive
	ECanaShadow.CANME.bit.ME31 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM31  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL31  = 1;  	// Int.-Level MB#0  -> I1EN

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
	struct ECAN_REGS ECanaShadow;
	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
	switch (Mbox)								//choose mailbox
	{
	case HEARTBEAT_BOX:
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
  	switch(mailbox_nr)
  	{
  	case COMMAND_BOX:
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

	case 2:
		//process the can message
		processCANmessage(0, ECanaMboxes.MBOX2.MSGID, ECanaMboxes.MBOX2.MDH.all,ECanaMboxes.MBOX2.MDL.all);
		ECanaRegs.CANRMP.bit.RMP2 = 1;
	break;

	case 3:
		//process the can message
		processCANmessage(1, ECanaMboxes.MBOX3.MSGID, ECanaMboxes.MBOX3.MDH.all,ECanaMboxes.MBOX3.MDL.all);
		ECanaRegs.CANRMP.bit.RMP3 = 1;
	break;

	case 4:
		//process the can message
		processCANmessage(2, ECanaMboxes.MBOX4.MSGID, ECanaMboxes.MBOX4.MDH.all,ECanaMboxes.MBOX4.MDL.all);
		ECanaRegs.CANRMP.bit.RMP4 = 1;
	break;

	case 5:
		//process the can message
		processCANmessage(3, ECanaMboxes.MBOX5.MSGID, ECanaMboxes.MBOX5.MDH.all,ECanaMboxes.MBOX5.MDL.all);
		ECanaRegs.CANRMP.bit.RMP5 = 1;
	break;

	case 6:
		//process the can message
		processCANmessage(4, ECanaMboxes.MBOX6.MSGID, ECanaMboxes.MBOX6.MDH.all,ECanaMboxes.MBOX6.MDL.all);
		ECanaRegs.CANRMP.bit.RMP6 = 1;
	break;

	case CANCORDERHEART_BOX:
		StopWatchRestart(LiveCANdata[CANDATAENTRIES].Timeout);
		ECanaRegs.CANRMP.bit.RMP30 = 1;
	break;

	case CELLVOLT_BOX:
		CanCell.U32 = ECanaMboxes.MBOX31.MDL.all;		//copy can data into union for decoding
		CurrCellBlock.Volt[0] = CanCell.data.C1mv/1000.0;
		CurrCellBlock.Balance[0] = CanCell.data.C1b;
		CurrCellBlock.Volt[1] = CanCell.data.C2mv/1000.0;
		CurrCellBlock.Balance[1] = CanCell.data.C2b;
		CanCell.U32 = ECanaMboxes.MBOX31.MDH.all;		//copy can data into union for decoding
		CurrCellBlock.Volt[2] = CanCell.data.C1mv/1000.0;
		CurrCellBlock.Balance[2] = CanCell.data.C1b;
		CurrCellBlock.Volt[3] = CanCell.data.C2mv/1000.0;
		CurrCellBlock.Balance[3] = CanCell.data.C2b;
		CellVoltFlag = 2;								//flag reception
		ECanaRegs.CANRMP.bit.RMP31 = 1;
	break;
  }


  	//To receive more interrupts from this PIE group, acknowledge this interrupt
  	PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}


//This function take a CAN message and extracts
//all the variables and writes them to the global data structure
void processCANmessage (int DB, unsigned int ID, Uint32 DataH, Uint32 DataL)
{
    //Pair message ID in buffer to one in canDB that we created
    int i;
    int indexer;
    static canUnion temp;
    indexer = -1;

    //this is currently a really slow lookup. if the CAN database gets large,
    //this should be switched to a binary tree search or a hash table

    //See if the received message is in our database
    for (i = 0; i<10; i++) {
        if (ID == CANdatabase[DB][i].canID) {
            indexer = i;
            break;
        }
    }

    //if this message is for us, process it
    if (indexer >= 0){

        //fill variables in CAN Variable array with actual data
        for (i = 0; i<CANdatabase[DB][indexer].length; i++) {                                   //loop for each variable stored in the message

        	temp.U32 = DataH;
        	temp.U64 = temp.U64 << 32L;
        	temp.U32 = DataL;
            temp.U64 = (temp.U64 >> LiveCANdata[CANdatabase[DB][indexer].index[i]].datapos);    //Shift the data by the bit offset for this CAN valriable

            //reset the stopwatch for this variable
            StopWatchRestart(LiveCANdata[CANdatabase[DB][indexer].index[i]].Timeout);

            //now interpret and scale the data, store in LiveCANdata
            switch (LiveCANdata[CANdatabase[DB][indexer].index[i]].type) {                      //switch for datatype
                case I16TYPE :
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.I16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U16TYPE :
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case I32TYPE :
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.I32 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U32TYPE :
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U32 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case F32TYPE :
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.F32 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case I64TYPE :
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.I64 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U64TYPE :
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U64 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case F64TYPE :
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.F64 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U8TYPE :
                    temp.U16 = temp.U16 & 0x00FF;
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U7TYPE :
                    temp.U16 = temp.U16 & 0x007F;
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U6TYPE :
                    temp.U16 = temp.U16 & 0x003F;
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U5TYPE :
                    temp.U16 = temp.U16 & 0x001F;
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U4TYPE :
                    temp.U16 = temp.U16 & 0x000F;
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U3TYPE :
                    temp.U16 = temp.U16 & 0x0007;
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U2TYPE :
                    temp.U16 = temp.U16 & 0x0003;
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case U1TYPE :
                    temp.U16 = temp.U16 & 0x0001;
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.U16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;
                case I8TYPE :
                    temp.U16 = temp.U16 & 0x00FF; // lops off everything but the 8 bits
                    if (temp.U16 & 0x0080) {        //sign extend out to 16 bits
                        temp.U16 = temp.U16|0xFF00;
                    }
                    LiveCANdata[CANdatabase[DB][indexer].index[i]].data = temp.I16 * LiveCANdata[CANdatabase[DB][indexer].index[i]].scale + LiveCANdata[CANdatabase[DB][indexer].index[i]].offset;
                    break;

            }
        }

    }

}
