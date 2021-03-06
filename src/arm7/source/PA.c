#include "PA7.h"
#include <nds.h>
#include <PA_Shared.h>


#define TOUCH_CNTRL_X1   (*(vu8*)0x027FFCDC)
#define TOUCH_CNTRL_Y1   (*(vu8*)0x027FFCDD)
#define TOUCH_CNTRL_X2   (*(vu8*)0x027FFCE2)
#define TOUCH_CNTRL_Y2   (*(vu8*)0x027FFCE3)

#define DISP_SR			(*(vuint16*)0x04000004)

// DS Lite screen brightness registers
#define PM_DSLITE_REG   (4)
#define PM_IS_LITE      BIT(6)
#define PM_BACKLIGHTS   (PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP)

int16 CNTRL_WIDTH;
int16 CNTRL_HEIGHT;

s32 TOUCH_WIDTH;
s32 TOUCH_HEIGHT;

u16 PA_ReadSPI(void);

u16 PA_NewSPI;

volatile PA_IPCType *PA_IPC;
u8 PA_SoundBusyInit;
s32 oldx; s32 oldy; // Stylus positions...
touchPosition tempPos;
//---------------------------------------------------------------------------------
void PA_Init(bool maxmod)
{
	irqInit();
	fifoInit();
	readUserSettings();

	// Enable sound if maxmod support was not requested
	if(!maxmod){
		// powerOn(POWER_SOUND);
		REG_POWERCNT |= POWER_SOUND; // We have to do this thanks to a dumb idea by the libnds team
		REG_SOUNDCNT = SOUND_ENABLE | SOUND_VOL(0x7F);
	}

	PA_IPC_compat->soundData = 0;

	PA_NewSPI = PA_ReadSPI();
	PA_IPC_compat->aux = PA_NewSPI;
}
//---------------------------------------------------------------------------------
u16 PA_ReadSPI(void)
{
	u8 pmData;
 
	SerialWaitBusy();
	
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz | SPI_CONTINUOUS; 
	REG_SPIDATA = (1 << 7); 
 
	SerialWaitBusy();
 
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz; // On revient en mode single byte ... 
	REG_SPIDATA = 0; // On indique � nouveau qu'on veut lire ... 
	
	SerialWaitBusy();
	
	pmData = REG_SPIDATA; // Et on r�cup�re la valeur du registre ! 
	
	REG_SPICNT = 0; // Pour finit on arr�te le SPI ... 
 
	return pmData;
}
//---------------------------------------------------------------------------------
void PA_WriteSPI(u8 pmReg, u8 pmData)
{
	SerialWaitBusy();
	
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz | SPI_CONTINUOUS;
	REG_SPIDATA = pmReg;
	SerialWaitBusy();
 
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz;
	REG_SPIDATA = pmData; 
	SerialWaitBusy();
}
//---------------------------------------------------------------------------------
void PA_ScreenLight(void)
{
	SerialWaitBusy();
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz | SPI_CONTINUOUS;
	REG_SPIDATA = 0;
	SerialWaitBusy();
	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER | SPI_BAUD_1MHz;
	
	REG_SPIDATA = PA_NewSPI; // On met en fonction de ce qu'on a dans l'IPC
}
//---------------------------------------------------------------------------------
void PA_SetDSLiteBrightness(u8 level)
{
	if (level > 3) level = 3;
    writePowerManagement(PM_DSLITE_REG, level);
}



