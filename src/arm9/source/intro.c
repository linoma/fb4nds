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
 
#include "intro.h"
#include "draws.h"
#include "all_gfx.h"
#include "fonts.h"

extern u8 *temp_buffer;
//---------------------------------------------------------------------------------
void intro(void)
{
	int i,i1,alpha,color,font;
	unsigned long size;	
	RECT rc;
	s8 pan;
	
	PA_HideBg(1,0);
	size = 0;
	SetWindowText(3,"");
	redraw_windows();
	my_sleep(32);
	{
		FILE *fp;
		
		fp = open_file("data/start.raw","rb");
		if(fp != NULL){								
			fseek(fp,0,SEEK_END);
			size = ftell(fp);
			fseek(fp,0,SEEK_SET);
			fread(temp_buffer,size,1,fp);
			fclose(fp);
		}
	}
	font = load_font("data/verdana.bdf",2) == 0 ? 2 : 0;	
	
	_REG16(0x04001050) = 16|128;
	_REG16(0x04001054) = 16;
	PA_LoadSprite16cPal(1,0,(void*)fb4_logo_oamPal);
	PA_CreateSprite(1,0,(void*)fb4_logo_oamTiles,OBJ_SIZE_64X64,0,0,32,36);
	PA_SetSpritePrio(1,0,3);
	PA_CreateSprite(1,1,(void*)&fb4_logo_oamTiles[512],OBJ_SIZE_64X64,0,0,96,36);
	PA_SetSpritePrio(1,1,3);
	PA_CreateSprite(1,2,(void*)&fb4_logo_oamTiles[1024],OBJ_SIZE_64X64,0,0,160,36);			
	PA_SetSpritePrio(1,2,3);
#ifndef _DEBUG	
	PA_SetRotset(1,0,512,256,256);
	PA_SetSpriteRotEnable(1,0,0);
	PA_SetSpriteRotEnable(1,1,0);
	PA_SetSpriteRotEnable(1,2,0);
	alpha = 0;
	rc.left = 0;
	rc.right = 256;
	rc.bottom = 192;
	color = 255;
	pan = 0;
	PA_IPC.Sound[16].Volume = 0;
	if(size > 0)
		PA_PlaySoundEx2(0,temp_buffer,(size - 1),127,22050,2,false,0);
	for(i=360,i1=0;i>=0;i--){//6 seconds * 60
		PA_WaitForVBL();
		if(i == 329){//after ~500 ms
			if(size > 0)
				PA_PlaySoundEx2(1,temp_buffer,(size - 1),64,22050,2,false,0);//echo 
		}
		else if(i > 299){//simple fade in for 1 second
			int i2;
			
			i2 = ((360 - i) * 8669) >> 12;
			PA_IPC.Sound[16].Volume = i2;
		}
		else if(i == 299){
			if(size > 0)
				PA_PlaySoundEx2(2,temp_buffer,(size - 1),32,22050,2,false,0);//echo 
		}
		//set pan for simple 3D sound we need of 4 speakers
		{
			int i2;
			
			PA_IPC.Sound[0].Pan = pan;
			PA_IPC.Sound[1].Pan = pan;
			i2 = i % 120;
			pan = ((i2 * 8669) >> 12);
			pan = abs(pan);
		}
		//finally my name :)
		set_text_font(1,2);
		color = (i * 352) >> 12;
		set_text_color(1,PA_RGB(color,color,color));
		rc.top = 120;
		draw_text(1,"Facebook for Nintendo DS",-1,&rc,DT_CENTER);
		rc.top += (fonts[font].height*160) >> 7;
		draw_text(1,"by Lino Maglione",-1,&rc,DT_CENTER);
		rc.top += (fonts[font].height*160) >> 7;
		draw_text(1,"All right reserved 2010",-1,&rc,DT_CENTER);
		i1 = (i1 + 1) & 0xF;
		if(!i1){
			alpha++;
			if(alpha > 16)
				alpha = 16;
			//PA_SetSFXAlpha(1,alpha,16-alpha);
			_REG16(0x04001054) = 16 - alpha;
		}
		PA_SetRotset(1,0,(i * 5825) >> 12,256+((i * 2912) >> 12),256+((i * 2912) >> 12));		
	}
	PA_SetSpriteRotDisable(1,0);
	PA_SetSpriteRotDisable(1,1);
	PA_SetSpriteRotDisable(1,2);
	PA_SetRotset(1,0,0,256,256);
	PA_DisableSpecialFx(1);
	
	set_text_font(1,0);
	set_text_color(1,PA_RGB(0,0,0));
	if(font != 0)
		free_font(2);
#else
	PA_DisableSpecialFx(1);
#endif		
	PA_ShowBg(1,0);	
	PA_WaitForVBL();
}