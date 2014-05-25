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

//CAN IDs mailbox breakdown
//Mailbox 2, DB0, CAN IDs 0x100-0x17F, mask 0x0000007F
//Mailbox 3, DB1, CAN IDs 0x180-0x1FF, mask 0x0000007F
//Mailbox 4, DB2, CAN IDs 0x200-0x27F, mask 0x0000007F
//Mailbox 5, DB3, CAN IDs 0x280-0x2FF, mask 0x0000007F
//Mailbox 6, DB4, CAN IDs 0x300-0x37F, mask 0x0000007F
//Mailbox 7, DB5, CAN IDs 0x380-0x3FF, mask 0x0000007F
//Mailbox 8, DB6, CAN IDs 0x400-0x47F, mask 0x0000007F
//Mailbox 9, DB7, CAN IDs 0x480-0x4FF, mask 0x0000007F

//total number of values read from CAN bus.
// there should be this many defines below
//note one CAN message can hold more than one value.
#define CANDATAENTRIES 110

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

//***************************CAN Variables Database*****************************
/*
 * This database holes the specifics of the CAN variables the parser handles.
 */

//This define is the initializer for the CAN Variables database
//There should be exactly as many lines here as the number CANDATAENTRIES above.
//each line should be {"units",ofsb,type,scale,offset},\  (Note the last line doesn't have the comma)
//make sure the only thing after the \  is a carriage return
//the final } has no "\"
//*** Important***, the order of these entries must match the order of their identifiers above (ie. RPM is first)
//"units" is the unit string, max of 3 chars
//ofsb is the number of bits the variable is offset
//type is the type code for the variable as stored in the CAN message.(see type code at top of file)
//scale is the scale factor for the CAN variable
//offset is the offset for the can variable (numeric offset for scale and offset, not the bit offset)
#define CANDATAINITVALUES {\
            {"",24,U16TYPE,0.125,0},\
            {"V",24,U16TYPE,0.125,0},\
            {"A",8,U8TYPE,1,-125},\
            {"RPM",16,U8TYPE,1,-125},\
            {"mph",16,U8TYPE,1,-125},\
            {"A",16,U8TYPE,1,-125},\
            {"Ohm",0,U8TYPE,1,-40},\
            {"Ah",0,U8TYPE,0.5,0},\
            {"km",24,U16TYPE,.03125,-273},\
            {"%",0,U2TYPE,1,0},\
            {"C",16,U16TYPE,1,0},\
            {"C",0,U4TYPE,1,0},\
            {"C",6,U2TYPE,1,0},\
            {"C",4,U2TYPE,1,0},\
            {"deg",8,U8TYPE,1,0},\
            {"deg",16,U8TYPE,1,0},\
            {"s",0,U8TYPE,1,0},\
            {"V",8,U8TYPE,1,0},\
            {"V",16,U8TYPE,1,0},\
            {"V",24,U8TYPE,1,0},\
            {"V",32,U8TYPE,1,0},\
            {"V",40,U8TYPE,1,0},\
            {"V",48,U8TYPE,1,0},\
            {"V",56,U8TYPE,1,0},\
            {"V",24,U8TYPE,1,0}\
}

//****************************CAN ID database***********************************
/*
 * The CAN ID database is a list of all the CAN IDs that are handled by the
 * automatic parser.
 */

//This define is the initializer for the CAN ID database(s)
//There should be exactly 16 lines for each mailbox database.
//The entries should be sorted by CAN ID so that it is easy to filter them based on mailbox
//(ie. all entries in a single CANDBINITVALUES should be easy to filter (like ascending numerical order))
//each line should be {0xnnn,{x,y,z},m},\  (Note the last line doesn't have the comma)
//make sure the only thing after the \  is a carriage return
//the final } has no "\"
//0xnnn is the CAN ID value in Hex
//{x,y,z} is a list of the CAN variables contained in the CAN ID, currently max of 6. (Use above defines in the list)
//,m is the number of CAN variables in the CAN ID (ie length of the list)

//CAN IDs mailbox breakdown
//Mailbox 2, DB0, CAN IDs 0x100-0x10F
//Mailbox 3, DB1, CAN IDs 0x110,0x111, 0x1E0-0x1E2
//Mailbox 4, DB2, CAN IDs 0x200-0x20F
//Mailbox 5, DB3, CAN IDs 0x300-0x30F
//Mailbox 6, DB4, CAN IDs 0x335-0x344
#define CANDBINITVALUES {\
	{\
    	{0x100, {CANDATAENTRIES},1},\
    	{0x101, {CANDATAENTRIES},1},\
    	{0x102, {CANDATAENTRIES},1},\
    	{0x103, {CANDATAENTRIES},1},\
    	{0x104, {CANDATAENTRIES},1},\
    	{0x105, {CANDATAENTRIES},1},\
    	{0x106, {CANDATAENTRIES},1},\
    	{0x107, {CANDATAENTRIES},1},\
    	{0x108, {CANDATAENTRIES},1},\
    	{0x109, {CANDATAENTRIES},1},\
    	{0x10A, {CANDATAENTRIES},1},\
    	{0x10B, {CANDATAENTRIES},1},\
    	{0x10C, {CANDATAENTRIES},1},\
    	{0x10D, {CANDATAENTRIES},1},\
    	{0x10E, {CANDATAENTRIES},1},\
    	{0x10F, {CANDATAENTRIES},1}\
    },\
	{\
    	{0x110, {CANDATAENTRIES},1},\
    	{0x111, {CANDATAENTRIES},1},\
    	{0x1E0, {CANDATAENTRIES},1},\
    	{0x1E2, {CANDATAENTRIES},1},\
    	{0x104, {CANDATAENTRIES},1},\
    	{0x105, {CANDATAENTRIES},1},\
    	{0x106, {CANDATAENTRIES},1},\
    	{0x107, {CANDATAENTRIES},1},\
    	{0x108, {CANDATAENTRIES},1},\
    	{0x109, {CANDATAENTRIES},1},\
    	{0x10A, {CANDATAENTRIES},1},\
    	{0x10B, {CANDATAENTRIES},1},\
    	{0x10C, {CANDATAENTRIES},1},\
    	{0x10D, {CANDATAENTRIES},1},\
    	{0x10E, {CANDATAENTRIES},1},\
    	{0x10F, {CANDATAENTRIES},1}\
    },\
    {\
    	{0x200, {CANDATAENTRIES},1},\
    	{0x201, {CANDATAENTRIES},1},\
    	{0x202, {CANDATAENTRIES},1},\
    	{0x203, {CANDATAENTRIES},1},\
    	{0x204, {CANDATAENTRIES},1},\
    	{0x205, {CANDATAENTRIES},1},\
    	{0x206, {CANDATAENTRIES},1},\
    	{0x207, {CANDATAENTRIES},1},\
    	{0x208, {CANDATAENTRIES},1},\
    	{0x209, {CANDATAENTRIES},1},\
    	{0x20A, {CANDATAENTRIES},1},\
    	{0x20B, {CANDATAENTRIES},1},\
    	{0x20C, {CANDATAENTRIES},1},\
    	{0x20D, {CANDATAENTRIES},1},\
    	{0x20E, {CANDATAENTRIES},1},\
    	{0x20F, {CANDATAENTRIES},1}\
    },\
    {\
    	{0x300, {CANDATAENTRIES},1},\
    	{0x301, {CANDATAENTRIES},1},\
    	{0x302, {CANDATAENTRIES},1},\
    	{0x303, {CANDATAENTRIES},1},\
    	{0x304, {CANDATAENTRIES},1},\
    	{0x305, {CANDATAENTRIES},1},\
    	{0x306, {CANDATAENTRIES},1},\
    	{0x307, {CANDATAENTRIES},1},\
    	{0x308, {CANDATAENTRIES},1},\
    	{0x309, {CANDATAENTRIES},1},\
    	{0x30A, {CANDATAENTRIES},1},\
    	{0x30B, {CANDATAENTRIES},1},\
    	{0x30C, {CANDATAENTRIES},1},\
    	{0x30D, {CANDATAENTRIES},1},\
    	{0x30E, {CANDATAENTRIES},1},\
    	{0x30F, {CANDATAENTRIES},1}\
    },\
    {\
    	{0x335, {CANDATAENTRIES},1},\
    	{0x336, {CANDATAENTRIES},1},\
    	{0x337, {CANDATAENTRIES},1},\
    	{0x338, {CANDATAENTRIES},1},\
    	{0x339, {CANDATAENTRIES},1},\
    	{0x33A, {CANDATAENTRIES},1},\
    	{0x33B, {CANDATAENTRIES},1},\
    	{0x33C, {CANDATAENTRIES},1},\
    	{0x33D, {CANDATAENTRIES},1},\
    	{0x33E, {CANDATAENTRIES},1},\
    	{0x33F, {CANDATAENTRIES},1},\
    	{0x340, {CANDATAENTRIES},1},\
    	{0x341, {CANDATAENTRIES},1},\
    	{0x342, {CANDATAENTRIES},1},\
    	{0x343, {CANDATAENTRIES},1},\
    	{0x344, {CANDATAENTRIES},1}\
    },\
}
#endif
