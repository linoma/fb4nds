//////////////////////////////////////////////////////////////////////
// Simple ARM7 stub (sends RTC, TSC, and X/Y data to the ARM 9)
// -- joat
// -- modified by Darkain and others
//////////////////////////////////////////////////////////////////////

#include <PA7.h>
#include <dswifi7.h>
#include <nds.h>
#include <PA_Shared.h>
#include <stdlib.h>

int vcount;
touchPosition first, tempPos;
int current_length = 0;
u8* microphone_buffer = 0;
int microphone_buffer_length = 0;
//---------------------------------------------------------------------------------
void PA_IPCManage(void)
{
	if(PA_SoundBusyInit) {  // Sound ready to use...
		PA_Mic(); // Manage Mic
		PA_SoundProcess(); // Play/stop comands
		PA_SoundUpdates();  // Get busy sound channels, change volume...
	}
	else if(PA_IPC_compat->mailData != 0) {
		PA_IPC = (PA_IPCType*)(PA_IPC_compat->mailData); // Inits PA Sound busy commands
		PA_IPC_compat->mailData = 0;
		PA_SoundBusyInit = 1;
	}
}
//---------------------------------------------------------------------------------
void VcountHandler() 
{
	static int lastbut = -1;	
	uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0;

	but = REG_KEYXY;
	if (!( (but ^ lastbut) & (1<<6))) {
 		touchReadXY(&tempPos);
		if ( tempPos.rawx == 0 || tempPos.rawy == 0 ) {
			but |= (1 <<6);
			lastbut = but;
		} 
		else {
			x = tempPos.rawx;
			y = tempPos.rawx;
			xpx = tempPos.px;
			ypx = tempPos.py;
			z1 = tempPos.z1;
			z2 = tempPos.z2;
		}		
	} 
	else {
		lastbut = but;
		but |= (1 <<6);
	}
	if ( vcount == 80 )
		first = tempPos;
	else {
		if (abs( xpx - first.px) > 10 || abs( ypx - first.py) > 10 || (but & ( 1<<6)) ) {
			but |= (1 <<6);
			lastbut = but;
		} 
		else { 	
			PA_IPC_compat->mailBusy 	= 1;
			PA_IPC_compat->touchX		= x;
			PA_IPC_compat->touchY		= y;
			PA_IPC_compat->touchXpx		= xpx;
			PA_IPC_compat->touchYpx		= ypx;
			PA_IPC_compat->touchZ1		= z1;
			PA_IPC_compat->touchZ2		= z2;
			PA_IPC_compat->mailBusy 	= 0;
		}
	}
	PA_IPC_compat->buttons = but;
	vcount ^= (80 ^ 130);
	SetYtrigger(vcount);
}
//---------------------------------------------------------------------------------
void PA_VBL() 
{
	s32 batt=0;
    int t1=0, t2=0;
    uint32 temp=0;

    batt = touchRead(TSC_MEASURE_BATTERY);    
    // Read Time
    resyncClock(); 
    // Read the temperature
    temp = touchReadTemperature(&t1, &t2);
    PA_IPC_compat->battery   	= batt;
    PA_IPC_compat->temperature 	= temp;
    PA_IPC_compat->tdiode1 		= t1;
    PA_IPC_compat->tdiode2 		= t2;	
	if (PA_NewSPI != (PA_IPC_compat->aux)){
		PA_NewSPI = PA_IPC_compat->aux;
		PA_ScreenLight(); // Update the screen lights...
	}	
	PA_IPCManage();
}
//---------------------------------------------------------------------------------
void timer2(void)
{
	if(microphone_buffer && microphone_buffer_length > 0) {
		*microphone_buffer++ = micReadData8() ^ 0x80;
		--microphone_buffer_length;
		current_length++;
	}
}
//---------------------------------------------------------------------------------
static void fifoUser01Value32Handler(u32 value, void *data) 
{
	u8 cmd;
	
	cmd = (value >> 28);
	switch(cmd){
		case 1:
			
		break;
	}
}
//---------------------------------------------------------------------------------
int main()
{
	PA_IPC_compat->mailData=0;
	PA_IPC_compat->mailSize=0; 
	PA_SoundBusyInit = 0;

	PA_Init(false);
	
	irqSet(IRQ_VBLANK, PA_VBL);
	irqSet(IRQ_TIMER2, timer2);
	SetYtrigger(80);
	vcount = 80;
	installWifiFIFO();

	fifoSetValue32Handler(FIFO_USER_01, fifoUser01Value32Handler, 0);
		
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqEnable(IRQ_VBLANK | IRQ_TIMER2 | IRQ_VCOUNT | IRQ_NETWORK);
	REG_IME = 1;	
	while(1){
		Wifi_Update();
		swiWaitForVBlank();
	}
}
