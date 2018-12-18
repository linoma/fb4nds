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
#include "digital_clock.h"
#include "all_gfx.h"
#include "utils.h"

static u16 digit_tile[11];
//---------------------------------------------------------------------------------
int init_digital_clock()
{
	int i;
	u8 screen;
	FILE *fp;
	unsigned int size,value,count,*timer_digitTiles,*timer_digitPal;
	
	fp = open_file("data/bin/digitclock.bin","rb");
	if(fp == NULL){
		SetWindowText(3,"I can not find clock's file");
		return -1;
	}
	
	fseek(fp,4,SEEK_SET);
	fread(&count,sizeof(count),1,fp);
	for(size=0,i=count;i>0;i--){
		fread(&value,sizeof(value),1,fp);
		size += value;
		fseek(fp,sizeof(unsigned int)*2,SEEK_CUR);
	}

	timer_digitTiles = (unsigned int *)malloc(size+10);
	fseek(fp,8,SEEK_SET);
	fread(&size,sizeof(size),1,fp);
	fread(&value,sizeof(value),1,fp);
	fseek(fp,value,SEEK_SET);
	fread(timer_digitTiles,size,1,fp);
		
	timer_digitPal = (unsigned short *)((char *)timer_digitTiles + size);
	fseek(fp,8+12,SEEK_SET);
	fread(&size,sizeof(size),1,fp);
	fread(&value,sizeof(value),1,fp);
	fseek(fp,value,SEEK_SET);
	fread(timer_digitPal,size,1,fp);	
	
	screen = windows[0].screen;
	PA_LoadSprite16cPal(screen,3,(void*)timer_digitPal);							
	for(i=0;i<12;i++)
		digit_tile[i] = PA_CreateGfx(screen,(void *)&timer_digitTiles[i*256],2,3,0);	

	PA_CreateSpriteFromGfx(1,127,digit_tile[10],OBJ_SIZE_32X64,0,3,180,192);
	PA_SetSpritePrio(1,127,3);

	PA_CreateSpriteFromGfx(screen,126,digit_tile[0],OBJ_SIZE_32X64,0,3,130,192);
	PA_SetSpritePrio(screen,126,3);
	
	PA_CreateSpriteFromGfx(screen,125,digit_tile[0],OBJ_SIZE_32X64,0,3,164,192);
	PA_SetSpritePrio(1,125,3);
	
	PA_CreateSpriteFromGfx(1,124,digit_tile[0],OBJ_SIZE_32X64,0,3,196,192);
	PA_SetSpritePrio(1,124,3);

	PA_CreateSpriteFromGfx(1,123,digit_tile[0],OBJ_SIZE_32X64,0,3,222,192);
	PA_SetSpritePrio(1,123,3);

	PA_SetRotset(1,1,0,384,384);
	PA_SetSpriteRotEnable(1,127,1);
	PA_SetSpriteRotEnable(1,126,1);
	PA_SetSpriteRotEnable(1,125,1);
	PA_SetSpriteRotEnable(1,124,1);
	PA_SetSpriteRotEnable(1,123,1);
	free(timer_digitTiles);
	
	set_timer(&windows[0],950,3333,NULL);	
	return 0;
}
//---------------------------------------------------------------------------------
int update_digital_clock()
{
	int value,res;
	
	divf32_async(PA_RTC.Hour,40960);
	PA_SetSpriteY(1,127,194 - ((PA_RTC.Seconds & 1) << 6) - ((PA_RTC.Seconds & 1) << 4));
	while(REG_DIVCNT & DIV_BUSY);
	value = REG_DIV_RESULT_L;
	res = REG_DIVREM_RESULT_L;

	divf32_async(PA_RTC.Minutes,40960);
	if(value == 0)
		PA_SetSpriteY(1,126,192);
	else{		
		PA_SetSpriteGfx(1,126,digit_tile[value]);
		PA_SetSpriteY(1,126,118);
	}
	PA_SetSpriteGfx(1,125,digit_tile[res >> 12]);
	PA_SetSpriteY(1,125,118);

	while(REG_DIVCNT & DIV_BUSY);
	value = REG_DIV_RESULT_L;
	res = REG_DIVREM_RESULT_L;

	PA_SetSpriteGfx(1,124,digit_tile[value]);
	PA_SetSpriteY(1,124,118);

	PA_SetSpriteGfx(1,123,digit_tile[res >> 12]);
	PA_SetSpriteY(1,123,118);	
}