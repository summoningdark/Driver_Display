/* 
 * File:   CANdb.h
 * Author: jennifer
 *
 * Created on May 3, 2014, 2:58 PM
 * Header with the CAN database information
 * This is the place to add/remove monitored CAN messages and CAN vairables
 */

#ifndef CANDB_H
#define	CANDB_H

//defines for data type definitions, used so the parser can correctly interpret 
//the data in the CAN message. simply an identifier, the actual numbers are
//meaningless.
#define I16TYPE 0
#define U16TYPE 1
#define I32TYPE 2
#define U32TYPE 3
#define F32TYPE 4
#define I64TYPE 5
#define U64TYPE 6
#define F64TYPE 7
#define U8TYPE 8
#define I8TYPE 9
#define U7TYPE 10
#define U6TYPE 11
#define U5TYPE 12
#define U4TYPE 13
#define U3TYPE 14
#define U2TYPE 15
#define U1TYPE 16


//total number of values read from CAN bus.
// there should be this many defines below
//note one CAN message can hold more than one value.
#define CANDATAENTRIES 25

//defines for the database entries, Human-readable form.
//(ie. Names of all the CAN variables)
//These are indices to the current CAN data array, so they should be contiguous
//(ie no gaps in the numbers).

#define TRIFLAGS 0
#define BUSVOLT 1
#define BUSCUR 2
#define MOTRPM 3
#define SPEED 4
#define MOTCUR 5
#define FRAMER 6
#define AMPHOUR 7
#define TRIPODO 8
#define COMMANDCUR 9
#define COOLANTLOW 10
#define COLLANTMOT 11
#define COOLANTCTRL 12
#define AMBIENTT 13
#define LATITUDE 14
#define LONGITUDE 15
#define TIME 16
#define BIM1MAX 17
#define BIM1MIN 18
#define BIM2MAX 19
#define BIM2MIN 20
#define BIM3MAX 21
#define BIM3MIN 22
#define BIM4MAX 23
#define BIM4MIN 24

//always end the CANdbcNames array with an empty string, the menu function needs this to know how many entries there are
//note only 21 characters are printable on a line
//this array should match the order and number of the defines above
const char CANdbcNames[CANDATAENTRIES+1][22]={	"Tritium Flags",
				"Bus Voltage",
				"Tritium Bus I",
				"Motor RPM",
				"Speed",
				"Motor Current",
				"Frame Resistance",
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

//****************************CAN ID database***********************************
/*
 * The CAN ID database is a list of all the CAN IDs that are handled by the
 * automatic parser.
 */

//This define is the initializer for the CAN ID database(s)
//There should be exactly 10 lines for each mailbox database.
//The entries should be sorted by CAN ID so that it is easy to filter them based on mailbox
//(ie. all entries in a single CANDBINITVALUES should be easy to filter (like ascending numerical order))
//each line should be {0xnnnnnnnn,{x,y,z},m},\  (Note the last line doesn't have the comma)
//make sure the only thing after the \  is a carriage return
//the final } has no "\"
//0xnnnn is the CAN ID value in Hex
//{x,y,z} is a list of the CAN variables contained in the CAN ID. (Use above defines in the list)
//,m is the number of CAN variables in the CAN ID (ie length of the list)
#define CANDBINITVALUES {\
	{\
    	{0x0CF00400, {RPM,DDTQ,ACTTQ},3},\
    	{0x18FEEE00, {ECT},1},\
    	{0x18FEF500, {BARO,AMBT},2},\
    	{0x18F11E05, {NGSTAT,NGASPW},2},\
    	{0x18F11C05, {DMAST_OVR_STAT,DFRP_OVR_STAT,DPW_OVR_STAT,DFRP_OVR,DPW_OVR},5},\
    	{0x18F11F05, {NSHIFT_0,NSHIFT_26,NSHIFT_51,NSHIFT_76,NSHIFT_101,NSHIFT_126,NSHIFT_151,NSHIFT_176},8}\
    },\
	{\
    	{0x0CF00400, {RPM,DDTQ,ACTTQ},3},\
    	{0x18FEEE00, {ECT},1},\
    	{0x18FEF500, {BARO,AMBT},2},\
    	{0x18F11E05, {NGSTAT,NGASPW},2},\
    	{0x18F11C05, {DMAST_OVR_STAT,DFRP_OVR_STAT,DPW_OVR_STAT,DFRP_OVR,DPW_OVR},5},\
    	{0x18F11F05, {NSHIFT_0,NSHIFT_26,NSHIFT_51,NSHIFT_76,NSHIFT_101,NSHIFT_126,NSHIFT_151,NSHIFT_176},8}\
    },\
    {\
    	{0x0CF00400, {RPM,DDTQ,ACTTQ},3},\
    	{0x18FEEE00, {ECT},1},\
    	{0x18FEF500, {BARO,AMBT},2},\
    	{0x18F11E05, {NGSTAT,NGASPW},2},\
    	{0x18F11C05, {DMAST_OVR_STAT,DFRP_OVR_STAT,DPW_OVR_STAT,DFRP_OVR,DPW_OVR},5},\
    	{0x18F11F05, {NSHIFT_0,NSHIFT_26,NSHIFT_51,NSHIFT_76,NSHIFT_101,NSHIFT_126,NSHIFT_151,NSHIFT_176},8}\
    },\
    {\
    	{0x0CF00400, {RPM,DDTQ,ACTTQ},3},\
    	{0x18FEEE00, {ECT},1},\
    	{0x18FEF500, {BARO,AMBT},2},\
    	{0x18F11E05, {NGSTAT,NGASPW},2},\
    	{0x18F11C05, {DMAST_OVR_STAT,DFRP_OVR_STAT,DPW_OVR_STAT,DFRP_OVR,DPW_OVR},5},\
    	{0x18F11F05, {NSHIFT_0,NSHIFT_26,NSHIFT_51,NSHIFT_76,NSHIFT_101,NSHIFT_126,NSHIFT_151,NSHIFT_176},8}\
    },\
    {\
    	{0x0CF00400, {RPM,DDTQ,ACTTQ},3},\
    	{0x18FEEE00, {ECT},1},\
    	{0x18FEF500, {BARO,AMBT},2},\
    	{0x18F11E05, {NGSTAT,NGASPW},2},\
    	{0x18F11C05, {DMAST_OVR_STAT,DFRP_OVR_STAT,DPW_OVR_STAT,DFRP_OVR,DPW_OVR},5},\
    	{0x18F11F05, {NSHIFT_0,NSHIFT_26,NSHIFT_51,NSHIFT_76,NSHIFT_101,NSHIFT_126,NSHIFT_151,NSHIFT_176},8}\
    },\
}


//***************************CAN Variables Database*****************************
/*
 * This database holes the specifics of the CAN variables the parser handles.
 */

//This define is the initializer for the CAN Variables database
//There should be exactly as many lines here as the number CANDATAENTRIES above.
//each line should be {"Name","units",ofsb,type,scale,offset},\  (Note the last line doesn't have the comma)
//make sure the only thing after the \  is a carriage return
//the final } has no "\"
//*** Important***, the order of these entries must match the order of their identifiers above (ie. RPM is first)
//"Name" is the name of the variable, in quotes ", max of 20 characters
//"units" is the unit string, max of 5 chars
//ofsb is the number of bits the variable is offset
//type is the type code for the variable as stored in the CAN message.(see type code at top of file)
//scale is the scale factor for the CAN variable
//offset is the offset for the can variable (numeric offset for scale and offset, not the bit offset)
#define CANDATAINITVALUES {\
            {"Tritium Flags","",24,U16TYPE,0.125,0},\
            {"Bus Voltage","V",24,U16TYPE,0.125,0},\
            {"Tritium Bus I","A",8,U8TYPE,1,-125},\
            {"Motor RPM","RPM",16,U8TYPE,1,-125},\
            {"Speed","mph",16,U8TYPE,1,-125},\
            {"Motor Current","A",16,U8TYPE,1,-125},\
            {"Frame Resistance","Ohm",0,U8TYPE,1,-40},\
            {"Bus AmpHour","Ah",0,U8TYPE,0.5,0},\
            {"Trip Odo","km",24,U16TYPE,.03125,-273},\
            {"Command Current","%",0,U2TYPE,1,0},\
            {"Coolant Low","C",16,U16TYPE,1,0},\
            {"Coolant Motor","C",0,U4TYPE,1,0},\
            {"Coolant Controller","C",6,U2TYPE,1,0},\
            {"Ambient Temp","C",4,U2TYPE,1,0},\
            {"Latitude","deg",8,U8TYPE,1,0},\
            {"Longitude","deg",16,U8TYPE,1,0},\
            {"Time","s",0,U8TYPE,1,0},\
            {"BIM1 Max","V",8,U8TYPE,1,0},\
            {"BIM1 Min","V",16,U8TYPE,1,0},\
            {"BIM2 Max","V",24,U8TYPE,1,0},\
            {"BIM2 Min","V",32,U8TYPE,1,0},\
            {"BIM3 Max","V",40,U8TYPE,1,0},\
            {"BIM3 Min","V",48,U8TYPE,1,0},\
            {"BIM4 Max","V",56,U8TYPE,1,0},\
            {"BIM4 Min","V",24,U8TYPE,1,0}\
}

#endif
