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

#include "fb.h"
#include "login_win.h"
#include "draws.h"
#include "editbox.h"

static int win = -1;
extern int (*pfn_MainFunc)(void);
extern int (*pfn_VBLFunc)(void);
extern int main_func(void);
extern u32 ticks,to_frient_list,to_notify,to_msg,to_buddy,to_feed,to_fail_msg;
static LPWNDPROC old_dlg_proc = NULL;
//---------------------------------------------------------------------------------
int OnConnectButtonPressed(void)
{
	SetWindowText(3,"Connecting...");
	active_win = 2;
	active_focus = 6;
	adjust_input_text(NULL);
	redraw_windows();
	pfn_VBLFunc = NULL;	
	if(fb_login()){
		pfn_MainFunc = show_login_win;
		pfn_VBLFunc = def_vbl_proc;	
		redraw_windows();
		active_win = -1;
		active_focus = -1;		
	}
	else{
		pfn_MainFunc = main_func;
		pfn_VBLFunc = def_vbl_proc;	
		redraw_windows();
		to_msg = to_frient_list = to_notify = to_buddy = to_feed = to_fail_msg = ticks;
	}
}
//---------------------------------------------------------------------------------
static int login_win_dlgproc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
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
		case WM_ERASEBKGND:
			{
				u16 color;				
				
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;
				if(hWnd->screen == 0)
					color = 5;
				else
					color = PA_RGB(0,0,0);
				color = set_text_color(hWnd->screen,color);
				set_text_font(hWnd->screen,hWnd->font);
				{					
					RECT rc_client,rc;
					
					get_client_rect(hWnd,&rc_client);
					{
						HWND h;					
						
						h = get_dlg_item(hWnd,1000);
						get_client_rect(h,&rc);						
						
						rc.right = rc.left+1;
						rc.left = rc_client.left+1;
					}					
					draw_text(hWnd->screen,"EMail",-1,&rc,DT_VCENTER);
					{
						HWND h;					
						
						h = get_dlg_item(hWnd,1001);
						get_client_rect(h,&rc);						
						
						rc.right = rc.left+1;
						rc.left = rc_client.left + 1;					
					}
					draw_text(hWnd->screen,"Password",-1,&rc,DT_VCENTER);
				}
				set_text_color(hWnd->screen,color);
				unlock_mutex(&hWnd->critical_section);
			}	
		break;
	}
	return res;
}
//---------------------------------------------------------------------------------
int show_login_win(void)
{
	int w,err;	
	RECT rc;
	
	err = -1;
	win = create_dialog_win(1,5,90,245,80,3,"Welcome to Facebook");
	if(win < 0)
		goto ex_show_login_win;
	old_dlg_proc = windows[win].pfn_proc;
	windows[win].pfn_proc = (LPWNDPROC)login_win_dlgproc;			
	w = create_editbox_win(1,52,108,189,14);
	if(w < 0)
		goto ex_show_login_win;
	windows[w].style |= WS_TABSTOP;
	set_dlg_item(w,&windows[win],1000);
	SetWindowText(w,pfb->mail);
	set_focus_window(&windows[w]);
	w = create_editbox_win(1,52,125,189,14);	
	if(w < 0)
		goto ex_show_login_win;
	windows[w].style |= WS_TABSTOP;
	SetWindowText(w,pfb->pass);
	set_dlg_item(w,&windows[win],1001);
	redraw_windows_screen(1);
	quit_msg_box = 0;
	while(!quit_msg_box)
		PA_WaitForVBL();	
	if(quit_msg_box == 2){
		SendDlgItemMessage(&windows[win],1000,WM_GETTEXT,300,(LPARAM)pfb->mail);
		SendDlgItemMessage(&windows[win],1001,WM_GETTEXT,300,(LPARAM)pfb->pass);
		pfn_MainFunc = OnConnectButtonPressed;
		err = 0;		
	}
	else
		err = -2;
ex_show_login_win:
	msg_box_proc();
	win = -1;
	return err;
}
//---------------------------------------------------------------------------------
int get_login_win_mail()
{
	if(win < 0)
		return -1;
	return get_dlg_item(&windows[win],1000);
}
//---------------------------------------------------------------------------------
int get_login_win_pass()
{
	if(win < 0)
		return -1;
	return get_dlg_item(&windows[win],1001);
}
