#include "Flash2803x_API_Library.h"

void EraseFlashB()
{
	FLASH_ST FlashStatus;
	DINT;
	Flash_Erase(SECTORB,&FlashStatus);
	EINT;
}

unsigned int WriteFloatFlashB(Uint32 Address, float value)
{
	Uint16 tmp;
	FLASH_ST FlashStatus;
	tmp = 10;
	DINT;
	tmp = Flash_Program((Uint16 *)Address,(Uint16 *)(&value),2,&FlashStatus);
	EINT;
	return tmp;
}
