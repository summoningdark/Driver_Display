/* 
 * File:   CANdbVars.h
 * Author: jennifer
 *
 * Created on May 3, 2014, 3:20 PM
 * header which declares all the variables/arrays used in the automatic CAN parser
 */

#include "CANdb.h"      //include type definitions

#ifndef CANDBVARS_H
#define	CANDBVARS_H

//Union data type to make extracting real values out of raw CAN data easy
typedef union{
    unsigned int U16;
    int I16;
    unsigned long U32;
    long I32;
    float F32;
    unsigned long long U64;
    long long I64;
    double F64;
    unsigned char data[8];

}canUnion;

//Can variable data structure
//holds everything necessary to interpret raw CAN bytes as a real-world value
//including scaling and offset
//an array of these holds all the information we use to parse data off of the CAN bus
//these are static and stored in flash
typedef struct{
    /*units of value*/
    char units[4];
    /*Offset in bits to the CAN Variable location*/
    unsigned int datapos;
    /*data type as stored in the CAN message*/
    unsigned int type;
    /*Scaling - sometimes also called resolution.  if res = .125, multiply by 8 to get real number */
    float scale;
    /*Offset - using a float just in case the offset is not an integer*/
    float offset;
} can_variable_struct;

//Live CAN data structure, holds the most current data parsed from the CAN bus
typedef struct{
    /*Data value after we scale and offset it.*/
    float data;
    /*stopwatch to tell is the value is stale*/
    stopwatch_struct* Timeout;
} LiveCANvariable;

//CAN database structure
//and array of these holds the mapping from CAN ID to variables
typedef struct {
    unsigned int canID;     //The CAN ID of the message
    unsigned int index[6];  //An array of indices in the canVar array for which this message holds data
    unsigned int length;    //the number of variables contained in this message
}canDB;
//------------------------------------------------------------------------------

// Data from CAN Bus Structure
// volatile gets changed by interrupt
extern volatile LiveCANvariable LiveCanData[CANDATAENTRIES+1];
//Can variable info database
extern const can_variable_struct CANVariabledb[CANDATAENTRIES];

//These arrays holds the mapping from CAN IDs to CAN variables.
//they are divided into 10 mailboxes to shorten the search time
extern const canDB CANdatabase[5][16];

#endif
