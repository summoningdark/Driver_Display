/*
 * data.h
 *
 *  Created on: Oct 30, 2013
 *      Author: Nathan
 */

#ifndef DATA_H_
#define DATA_H_

typedef long long int64;

typedef struct CELLBLOCK
{
	float Volt[4];
	char Balance[4];
} CellBlock;

typedef struct TIME
{
	unsigned char years;
	unsigned char month :8;
	unsigned char day:8;
	unsigned char hours:8;
	unsigned char minutes:8;
	unsigned char seconds:8;
} TIME_STRUCT;

typedef struct COORDINATES
{
	unsigned int Degrees:8;
	float Minutes;
} LAT_LONG;

//this union is to make extracting arbitrary variable data out of a CAN message easy
//first the data bytes from the CAN message are placed in a variable with this union type
//then the union is shifted using its uint64 representation to bring the correct data bits to the right justified position
//you can then access the data as whatever type is appropriate
typedef union
{
	int16	I16;
	Uint16	U16;
	int32	I32;
	Uint32	U32;
	float32	F32;
	int64	I64;
	Uint64	U64;
	float64	F64;
	TIME_STRUCT	TIME;
	LAT_LONG COORD;
} CAN_DATA_u;

typedef struct
{
	unsigned char C1mv : 15;
	unsigned char C1b  : 1;
	unsigned char C2mv : 15;
	unsigned char C2b  : 1;
} cell_can_struct;

typedef union
{
	cell_can_struct data;
	Uint32 U32;
} cell_can_union;

//this structure holds a CAN variable. it includes the CAN ID the variable lives on, a type code to tell how
//to interpret the bits, and an offset to the first bit(offset is measured in bits from the LSB of CAN data byte 0).
//ie. CAN data bytes data[0] .. data[8] are assumed LSB first.
typedef struct CAN_VAR
{
	Uint16 SID;
	Uint16 TypeCode;
	Uint16 Offset;
	Uint16 New;
	char Name[21];
	CAN_DATA_u data;
} can_variable_struct;

//this struct is fort the const array in memory that holds our CAN variable data, same as a CAN_VAR but without the data
typedef struct CAN_VAR_LIST
{
	Uint16 SID;
	Uint16 TypeCode;
	Uint16 Offset;
} can_variable_list_struct;

typedef struct DATA
{
  unsigned long adc;
  char gp_button;
} data_struct;

//Size of the button press queue
#define BUTTON_QUEUE_SIZE 10
//number of ticks before a button is considered held (another press is registered) 5000 = .5s
#define BUTTON_HOLD_TICKS 50
//number of ticks before a held button repetes a press. 3000 = .3s
#define BUTTON_REPETE_TICKS 30

typedef struct BUTTONQUEUE
{
	unsigned int Full;
	unsigned int Empty;
	unsigned int Current;
	unsigned int Next;
	unsigned int Queue[BUTTON_QUEUE_SIZE];
} button_queue_struct;

#endif /* DATA_H_ */
