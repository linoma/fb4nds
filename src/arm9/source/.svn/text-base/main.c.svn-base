/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <PA9.h>
#include <fat.h>
#include "fb.h"
#include "windows.h"
#include "login_win.h"
#include "fonts.h"
#include "all_gfx.h"
#include "menu.h"

//---------------------------------------------------------------------------------
int bQuit = 0;
u32 ticks = 0,to_frient_list=0,to_notify=0,to_msg=0,to_buddy = 0,to_feed = 0,to_fail_msg = 0;
unsigned long main_mutex;
//---------------------------------------------------------------------------------
int (*pfn_MainFunc)(void);
int (*pfn_VBLFunc)(void);
int (*pfn_timer2Func)(void);
static void fifoUser01Value32Handler(u32 value, void* data);
//---------------------------------------------------------------------------------
int main_func(void)
{
#ifndef _DEBUG	
	if((ticks - to_msg) > 3*59 || ticks < to_msg){	
		if(trylock_mutex(&main_mutex)){
			fb_get_new_messages();
			to_msg = ticks;
			unlock_mutex(&main_mutex);
		}
	}
	if((ticks - to_frient_list) > 59*300 || ticks < to_frient_list){
		if(trylock_mutex(&main_mutex)){
			fb_check_friend_requests();
			to_frient_list = ticks;
			unlock_mutex(&main_mutex);
		}
	}
	if((ticks - to_notify) > 59*60 || ticks < to_notify){
		if(trylock_mutex(&main_mutex)){		
			if(!fb_get_notifications_feed())
				to_notify = ticks;
			unlock_mutex(&main_mutex);
		}
	}
	if((ticks - to_buddy) > 59*60 || ticks < to_buddy){
		if(trylock_mutex(&main_mutex)){
			fb_get_buddy_list();
			to_buddy = ticks;
			unlock_mutex(&main_mutex);
		}
	}
	if((ticks - to_feed) > 59 * 60 || ticks < to_feed){
		to_feed = ticks;		
	}
	if((ticks - to_fail_msg) > 15 * 59 || ticks < to_fail_msg){
		if(trylock_mutex(&main_mutex)){
			fb_get_messages_failsafe();
			to_fail_msg = ticks;
			unlock_mutex(&main_mutex);
		}
	}
	if(trylock_mutex(&main_mutex)){
		execute_messages_loop();
		execute_contacts_loop();
		unlock_mutex(&main_mutex);
	}
	SetWindowText(3,"");
#else
/*	if((ticks - to_notify) > 10*60 || ticks < to_notify){
		fb_parse_notify(NULL,0);
		to_notify = ticks;
	}	*/
//	fb_parse_new_messages(NULL,0);
//	execute_contacts_loop();
#endif
	PA_WaitForVBL();
	return 1;
}
//---------------------------------------------------------------------------------
void OnVBL(void)
{
	ticks++;
	execute_windows_loop();
	if(pfn_VBLFunc != NULL)
		pfn_VBLFunc();		
}
//---------------------------------------------------------------------------------
static void on_timer2_irq(void)
{	
	if(pfn_timer2Func != NULL){
		timerStop(2);
		pfn_timer2Func();
		TIMER_CR(2) |= TIMER_ENABLE;
	}
}
//---------------------------------------------------------------------------------
int main(int argc, char ** argv)
{			
	PA_Init();
	PA_InitVBL();
#ifndef _DEBUG		
	{	
		int i;
			
		PA_SetBgColor(0,0x7FFF);	
		PA_EasyBgLoad(0,3,neo_flash);
		for(i=0;i<5000/16;i++)
			PA_WaitForVBL();	
	}
#endif	
	PA_Init16bitBg(1, 2);			
	PA_Init8bitBg(0, 3);

	PA_LoadBgPal(1, 0, (void*)bg1_Pal);		
	PA_DeleteBg(1,0);
	PA_LoadBgTilesEx(1, 0, (void*)bg1_Tiles, 192);
	PA_LoadBgMap(1, 0, (void*)bg1_Map, BG_256X256); 
	PA_InitBg(1, 0, BG_256X256, 0, 1);
	PA_SetBgPrio(1,0,3);
		
	PA_SetBgColor(1,0x7FFF);	
	PA_SetBgPalCol(1,2,PA_RGB(7,11,19));
	
	PA_SetBgColor(0,0x7FFF);	
	PA_SetBgPalCol(0,0,PA_RGB(31,31,31));
	PA_SetBgPalCol(0,2,PA_RGB(7,11,19));
	PA_SetBgPalCol(0,3,PA_RGB(24,24,24));//light gray
	PA_SetBgPalCol(0,4,PA_RGB(31,31,31));
	PA_SetBgPalCol(0,5,PA_RGB(0,0,0));
	PA_SetBgPalCol(0,6,PA_RGB(31,0,0));
	PA_SetBgPalCol(0,7,PA_RGB(15,15,15));//dark gray	
	PA_VBLFunctionInit(OnVBL);	
	init_fonts();	
	init_windows();		
	SetWindowText(3,"Initializing Fat...");
	if(!fatInitDefault()){
		SetWindowText(3, "Initializing Fat Error!!!");
		goto main_0;		
	}
	
	fifoSetValue32Handler(FIFO_USER_01, fifoUser01Value32Handler, 0);
	
	init_fb();
	
	SetWindowText(3,"Initializing system...");
	init_system();	
	init_wait_anim();
	intro();
	
	timerStart(2,ClockDivider_64,timerFreqToTicks_64(40),on_timer2_irq);
		
	{		
		FILE *fp;
		unsigned char *keyboardcustom_Tiles;//,*sprite;
		int *keyboardcustom_Info,i;
		unsigned short *keyboardcustom_Pal,*keyboardcustom_Map;		
		unsigned int size,value,count;
				
		fp = open_file("data/bin/keyboardcustom.bin","rb");
		if(fp == NULL){
			SetWindowText(3,"I can not find keyboard's file");
			goto main_0;
		}
		fseek(fp,4,SEEK_SET);
		fread(&count,sizeof(count),1,fp);
		for(size=0,i=count;i>0;i--){
			fread(&value,sizeof(value),1,fp);
			size += value;
			fseek(fp,sizeof(unsigned int)*2,SEEK_CUR);
		}

		keyboardcustom_Info = (int *)malloc(size+10);
		fseek(fp,8,SEEK_SET);
		fread(&size,sizeof(size),1,fp);
		fread(&value,sizeof(value),1,fp);
		fseek(fp,value,SEEK_SET);
		fread(keyboardcustom_Info,size,1,fp);
		
		keyboardcustom_Map = (unsigned short *)((char *)keyboardcustom_Info + size);
		fseek(fp,8+12,SEEK_SET);
		fread(&size,sizeof(size),1,fp);
		fread(&value,sizeof(value),1,fp);
		fseek(fp,value,SEEK_SET);
		fread(keyboardcustom_Map,size,1,fp);

		keyboardcustom_Tiles = (unsigned char *)((char *)keyboardcustom_Map + size);
		fseek(fp,8+12*2,SEEK_SET);
		fread(&size,sizeof(size),1,fp);
		fread(&value,sizeof(value),1,fp);
		fseek(fp,value,SEEK_SET);
		fread(keyboardcustom_Tiles,size,1,fp);

		keyboardcustom_Pal = (unsigned short *)((char *)keyboardcustom_Tiles + size);
		fseek(fp,8+12*3,SEEK_SET);
		fread(&size,sizeof(size),1,fp);
		fread(&value,sizeof(value),1,fp);
		fseek(fp,value,SEEK_SET);
		fread(keyboardcustom_Pal,size,1,fp);

		fclose(fp);	
		
		PA_LoadBgPal(0, 1, (void*)keyboardcustom_Pal);		
		PA_DeleteBg(0,1);
		PA_LoadBgTilesEx(0, 1, (void*)keyboardcustom_Tiles, 30016);
		PA_LoadBgMap(0, 1, (void*)keyboardcustom_Map, BG_256X512); 

		{
			s16 charset;
			u8 blocksize;
			
			size = bg_sizes[BG_256X512];
			blocksize = PA_BgInfo[0][1].mapsize;
			charset = PA_BgInfo[0][1].mapchar + blocksize;
			
			DMA_Copy(&keyboardcustom_Map[2048], (void*)ScreenBaseBlock(0, charset), size, DMA_16NOW);
			
			for (i = 0; i < blocksize; i++) 
				charblocks[0][charset + i] = 1;
		}
		
		PA_InitBg(0, 1, BG_256X512, 0, 1);
		PA_BGScrollXY(0, 1, 0, 0);
		PA_Keyboard_Struct.Bg = 1;	
		PA_Keyboard_Struct.Type = 0;	
		PA_Keyboard_Struct.Repeat = 0;
		PA_Keyboard_Struct.Custom = 1;
		PA_BgInfo[0][PA_Keyboard_Struct.Bg].Map = (u32)keyboardcustom_Map;
				
		free(keyboardcustom_Info);
		PA_KeyboardIn(25,81);
	}		
	
	init_digital_clock();
	redraw_windows();	
#ifndef _DEBUG	
	InitCyaSSL();
	SetWindowText(3, "Initializing WiFi..."); 
	PA_InitWifi();
	if(!PA_ConnectWifiWFC()){
		SetWindowText(3, "Initializing WiFi Error!!!"); 
		goto main_0;
	}
#endif	
	SetWindowText(3,"");	
	fb_load_config();

	ram_init(DETECT_RAM);

	load_fonts();
	redraw_windows();	
	PA_WaitForVBL();	
	pfn_MainFunc = show_login_win;	
	pfn_VBLFunc = def_vbl_proc;
	if(!is_ext_ram())
		create_message_box(1,"System Message","Some features are disabled!!!\r\nBecause you have not the expansion ram!!!",1|0x80000000,NULL);			
#ifdef _DEBUG	
	active_win = 1;
	active_focus = consolle_win;
	adjust_input_text(NULL);
	redraw_windows();	
	to_msg = to_frient_list = to_buddy = to_feed = to_fail_msg = to_notify = ticks;
	//fb_buddy_list_parse(NULL,0);
	//fb_friends_parse_req(NULL,0);
	pfn_MainFunc = main_func;		
#endif	
main_0:
	while(!bQuit){	
		if(pfn_MainFunc != NULL)
			pfn_MainFunc();
		PA_WaitForVBL();
	}	
	timerStop(3);
	destroy_fb();
	destroy_windows();
	destroy_fonts();
	ram_lock();
	return 0;
}
//---------------------------------------------------------------------------------
static void fifoUser01Value32Handler(u32 value, void* data) 
{
}