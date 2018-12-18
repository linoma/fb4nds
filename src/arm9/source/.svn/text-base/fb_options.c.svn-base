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
 
#include "fb_options.h"
#include "fb.h"
#include "checkbox.h"

extern u8 *temp_buffer;
static LPWNDPROC old_dlg_proc = NULL;
extern int (*pfn_MainFunc)(void);
extern int (*pfn_timer2Func)(void);
extern int modal_win;
//---------------------------------------------------------------------------------
static int options_win_dlgproc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	int res;
	
	res = old_dlg_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
	switch(uMsg){
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case 1:
				case 2:
					quit_msg_box = LOWORD(wParam);
				break;
			}
		break;
	}
	return res;
}
//---------------------------------------------------------------------------------
int create_options_win(int screen)
{
	int win,w;
	
	if(screen > 1)
		return -1;
	win = create_dialog_win(screen,10,10,230,160,3,"fb4nds - Options");
	if(win < 0)
		return -1;
	windows[win].style |= WS_CLOSE;
	old_dlg_proc = windows[win].pfn_proc;
	windows[win].pfn_proc = (LPWNDPROC)options_win_dlgproc;	
	
	w = create_checkbox_win(screen,15,30,150,100);
	if(w < 0)
		return -1;
	set_dlg_item(w,&windows[win],1000);
	SetWindowText(w,"Use Local Ram for Chat Avatars");
	SendDlgItemMessage(&windows[win],1000,BM_SETCHECK,(WPARAM)((pfb->flags & FBO_USELRAM_AVATAR) ? BST_CHECKED : BST_UNCHECKED),0);
	
	w = create_checkbox_win(screen,15,45,150,100);
	if(w < 0)
		return -1;
	set_dlg_item(w,&windows[win],1001);
	
	SetWindowText(w,"Load the Avatars in the News");
	if(!is_ext_ram())
		enable_window(&windows[w],0);
	w = create_checkbox_win(screen,15,60,180,100);
	if(w < 0)
		return -1;
	set_dlg_item(w,&windows[win],1002);
	SetWindowText(w,"Load the Avatars in the FriendShips");
	if(!is_ext_ram())
		enable_window(&windows[w],0);		
	redraw_windows_screen(screen);
	quit_msg_box = 0;
	while(!quit_msg_box)
		PA_WaitForVBL();
	if(quit_msg_box == 2){		
		
		w = SendDlgItemMessage(&windows[win],1000,BM_GETCHECK,0,0);
		if(w != BST_CHECKED)
			pfb->flags &= ~FBO_USELRAM_AVATAR;
		else
			pfb->flags |= FBO_USELRAM_AVATAR;
		fb_save_config();
	}
	msg_box_proc();
	return quit_msg_box;		
}