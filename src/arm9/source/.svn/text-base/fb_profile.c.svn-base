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
 
#include "fb_profile.h"
#include "file_chooser.h"

extern int (*pfn_timer2Func)(void);
extern unsigned long main_mutex;
static char *file_name;
static u8 screen;
//---------------------------------------------------------------------------------
static int timer2_upload_image_profile_proc(void)
{
	int i;
	
	if(!trylock_mutex(&main_mutex)){
		PA_SetBrightness(screen,-8);
		return 0;
	}
	_REG16(REG_BRIGHT + (0x1000 * screen)) = 0;
	REG_IME = 1;	
	i = fb_upload_profile_image(file_name);
	pfn_timer2Func = NULL;
	if(i < 1)
		create_message_box(screen,"fb4nds","An error during the upload!!!",1|MB_SYSTEMMODAL,NULL);	
	else{
		char *p;
		
		start_wait_anim();
		if(fb_get_post_form(&p) >  0){
			if(fb_get_profile_image_url(p,0) & 32)
				fb_get_profile_image(0);
		}
		destroy_wait_anim();
	}
	unlock_mutex(&main_mutex);	
	return 1;	
}
//---------------------------------------------------------------------------------
int change_profile_image(u8 scr)
{	
	file_name = (char *)&temp_buffer[SZ_LOCAL_BUFFER - temp_buffer_index-1000];
	if(create_file_chooser(scr,NULL,"Files JPEG (*.jpg)\0*.jpg\0Files PNG (*.png)\0*.png\0Files GIF (*.gif)\0*.gif\0Files bitmap (*.bmp)\0*.bmp\0All files (*.*)\0*.*\0\0",NULL,file_name) != 2)
		return -1;
	if(file_name[0] == 0)
		return -1;
	if(pfn_timer2Func != NULL)
		return -2;	
	screen = scr;
	pfn_timer2Func = timer2_upload_image_profile_proc;	
	return 0;
}