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

#include "windows.h"
#include "fb.h"
#include "draws.h"
#include "chat_win.h"
#include "all_gfx.h"
#include "scrollbar.h"
#include "listbox.h"
#include "button.h"
#include "menu.h"
#include "contacts.h"
#include "fb_messages.h"
#include "editbox.h"
#include "keys_sound.h"
#include "digital_clock.h"
#include "keyboard.h"
#include "fb_options.h"

WINDOW windows[MAX_WINDOWS],*first_win = NULL,*last_win = NULL;
WINTIMER timers[MAX_TIMERS];
u8 system_images[5000];
int system_image_index[20];
//---------------------------------------------------------------------------------
char text[1000]={0};
int active_win = -1,active_focus = -1,consolle_win = -1,chat_win = -1,active_win_top = -1,menu_bar_bottom_win = -1,news_win = -1;
int nletter,chat_tab_win = -1,btn_connect_win = -1,modal_win = -1,quit_msg_box = 0,my_wall = 1;
//---------------------------------------------------------------------------------
static LPWNDPROC old_wnd_proc = NULL;
//---------------------------------------------------------------------------------
extern int (*pfn_MainFunc)(void);
extern int (*pfn_timer2Func)(void);
extern u32 ticks;
extern LPFBLIST ls_contacts;
static int wndproc_topwin(WINDOW *,unsigned int,unsigned int,unsigned int);
static int wndproc_bottomwin(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
static int wndproc_menubar_topwin(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
static int wndproc_statusbar_topwin(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
static int wndproc_menubar_bottomwin(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
//---------------------------------------------------------------------------------
void append_win(int win)
{
	WINDOW *p;
	
	if(win < 0 || win >= MAX_WINDOWS)
		return;
	p = &windows[win];
	p->prev = (struct WINDOW *)last_win;
	p->next = NULL;
	if(last_win != NULL)
		last_win->next = (struct WINDOW *)p;
	if(first_win == NULL)
		first_win = p;
	last_win = p;
}
//---------------------------------------------------------------------------------
int bring_win_top(int win)
{
	LPWINDOW p;
	
	//return win;
	if(win < 0 || win >= MAX_WINDOWS)
		return -1;
	p = &windows[win];
	if(last_win == p)
		return win;
	if(p->prev != NULL)
		((LPWINDOW)p->prev)->next = p->next;
	if(p->next != NULL)
		((LPWINDOW)p->next)->prev = p->prev;	
	if(last_win != NULL)
		last_win->next = (struct WINDOW *)p;
	if(first_win == p)
		first_win = (LPWINDOW)p->next;
	p->prev = (struct WINDOW *)last_win;	
	p->next = NULL;
	last_win = p;
	return win;
}
//---------------------------------------------------------------------------------
int show_window(int win,int nCmdShow)
{
	unsigned long style;
	LPWINEXHEADER p;
	
	if(win < 0 || win >= MAX_WINDOWS)
		return -1;
	if(windows[win].type != WC_COMBOBOX)
		return -2;
	switch(nCmdShow){
		case SW_HIDE:
			style = 0;
		break;
		case SW_SHOW:
			style = WS_VISIBLE;
		break;
		default:
			return -3;
	}
	p = (LPWINEXHEADER)windows[win].data;
	windows[win].style &= ~WS_VISIBLE;	
	windows[win].style |= style;
	if(p != NULL){
		if(p->horz_sb > -1){
			windows[p->horz_sb].style &= ~WS_VISIBLE;
			windows[p->horz_sb].style |= style;
			invalidate_window(p->horz_sb,NULL,1);
		}
		if(p->vert_sb > -1 && !(style & WS_VISIBLE)){
			windows[p->vert_sb].style &= ~WS_VISIBLE;
			invalidate_window(p->vert_sb,NULL,1);
		}
	}
	invalidate_window(win,NULL,1);
	return 0;
}
//---------------------------------------------------------------------------------
int init_windows()
{
	int i;
	
	for(i=0;i<sizeof(system_image_index) / sizeof(int);i++)
		system_image_index[i] = -1;

	i = tile_to_bitmap(downTiles,downPal,16,7,4,system_images);
	if(i != -1){
		system_image_index[0] = 0;
		system_image_index[1] = i;
	}
	
	memset(windows,0,sizeof(windows));
	memset(timers,0,sizeof(timers));
	windows[0].pfn_proc = (LPWNDPROC)wndproc_topwin;	
	windows[1].pfn_proc = (LPWNDPROC)wndproc_bottomwin;
	windows[2].pfn_proc = (LPWNDPROC)wndproc_menubar_topwin;
	windows[3].pfn_proc = (LPWNDPROC)wndproc_statusbar_topwin;
	for(i=0;i<4;i++){
		append_win(i);
		windows[i].style |= WS_VISIBLE;
		windows[i].status |= WST_ENABLED;
		SendMessage(i,WM_CREATE,0,0);
	}		
	return 1;
}
//---------------------------------------------------------------------------------
void destroy_window(int win)
{
	LPWINDOW p;
	
	if(win < 0 || win >= MAX_WINDOWS)
		return;
	p = &windows[win];
	if(p->pfn_proc != NULL){
		if(p->pfn_proc((struct WINDOW *)p,WM_DESTROY,0,0))
			return;
		p->pfn_proc = NULL;
	}	
	if(p->prev != NULL)
		((LPWINDOW)p->prev)->next = p->next;
	if(p->next != NULL)
		((LPWINDOW)p->next)->prev = p->prev;		
	if(last_win == p){
		if(p->next != NULL)
			last_win = (LPWINDOW)p->next;
		else
			last_win = (LPWINDOW)p->prev;
	}
	if(first_win == p){
		if(p->prev != NULL)
			first_win = (LPWINDOW)p->prev;
		else
			first_win = (LPWINDOW)p->next;
	}
	if(p->data != NULL){
		free(p->data);
		p->data = NULL;
	}
	p->user_data = NULL;
	p->style = 0;
}
//---------------------------------------------------------------------------------
void destroy_windows()
{
	int i;
	
	for(i=0;i<sizeof(windows)/sizeof(WINDOW);i++)
		destroy_window(i);
}
//---------------------------------------------------------------------------------
void redraw_windows_screen(u8 screen)
{
	int i;
	
	for(i=0;i<sizeof(windows)/sizeof(WINDOW);i++){
		if(windows[i].pfn_proc == NULL || windows[i].screen != screen)
			continue;
		windows[i].status |= 3;
	}
	update_windows();
}
//---------------------------------------------------------------------------------
void redraw_windows()
{
	int i;
	
	for(i=0;i<sizeof(windows)/sizeof(WINDOW);i++){
		if(windows[i].pfn_proc == NULL)
			continue;
		windows[i].status |= 3;
	}
	update_windows();
}
//---------------------------------------------------------------------------------
void execute_windows_loop()
{
	LPWINTIMER p;
	int i;

	for(p = timers,i=0;i<sizeof(timers)/sizeof(WINTIMER);i++,p++){
		if(p->hWnd == NULL && p->pfn_proc == NULL)
			continue;		
		if((ticks - p->elapsed) > (p->timer >> 4)){
			if(p->hWnd != NULL && p->hWnd->pfn_proc != NULL)
				p->hWnd->pfn_proc((struct WINDOW *)p->hWnd,WM_TIMER,p->id,(LPARAM)p);
			else if(p->pfn_proc != NULL)
				p->pfn_proc(p->id,i);			
			p->elapsed = ticks;			
		}		
	}
	update_windows();	
}
//---------------------------------------------------------------------------------
void update_windows()
{
	LPWINDOW p;
	
	p = first_win;
	while(p != NULL){
		if(update_window(get_window_index(p)) < 0)
			return;
		p = (LPWINDOW)p->next;
	}
}
//---------------------------------------------------------------------------------
void destroy_timer(int i)
{
	if(i < 0 || i >= MAX_TIMERS)
		return;
	timers[i].hWnd = NULL;
	timers[i].pfn_proc = NULL;
}
//---------------------------------------------------------------------------------
int set_timer(LPWINDOW hWnd,unsigned int timeout,unsigned int id,LPTIMERPROC proc)
{
	int i;
	
	if(hWnd == NULL && proc == NULL)
		return -1;
	for(i=0;i<sizeof(timers)/sizeof(WINTIMER);i++){
		if(timers[i].hWnd == NULL && timers[i].pfn_proc == NULL){
			timers[i].timer = timeout;
			timers[i].hWnd = hWnd;
			timers[i].elapsed = ticks;
			timers[i].pfn_proc = proc;
			timers[i].id = id;
			return i;
		}
	}
	return -1;
}
//---------------------------------------------------------------------------------
int update_window(int win)
{
	if(win < 0 || win >= MAX_WINDOWS)
		return 0;
	if(windows[win].pfn_proc == NULL || !(windows[win].style & WS_VISIBLE))
		return 0;
	if(windows[win].status & 1){
		if(windows[win].pfn_proc((struct WINDOW *)&windows[win],WM_ERASEBKGND,0,0))
			return -1;
		windows[win].status &= ~1;	
	}
	if(windows[win].status & 2){
		if(windows[win].pfn_proc((struct WINDOW *)&windows[win],WM_PAINT,0,0))
			return -2;
		windows[win].status &= ~2;	
	}
	return 0;
}
//---------------------------------------------------------------------------------
void invalidate_windows_screen(u8 screen,int be)
{
	int i;
	
	for(i=0;i<sizeof(windows)/sizeof(WINDOW);i++){
		if(windows[i].pfn_proc == NULL || windows[i].screen != screen)
			continue;
		invalidate_window(i,NULL,be);
	}
}
//---------------------------------------------------------------------------------
int invalidate_window(int win,LPRECT lprc,int be)
{
	if(win < 0 || win >= MAX_WINDOWS)
		return -1;
	if(windows[win].pfn_proc == NULL || !(windows[win].style & WS_VISIBLE))
		return -2;
	windows[win].status |= 2;
	if(be)
		windows[win].status |= 1;
	return 0;
}
//---------------------------------------------------------------------------------
void redraw_window(int win)
{
	if(win < 0 || win >= MAX_WINDOWS)
		return;
	windows[win].status |= 3;
	update_window(win);
}
//---------------------------------------------------------------------------------
int SendMessage_HWND(HWND hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	if(hWnd == NULL)
		return -1;
	if(hWnd->pfn_proc == NULL)
		return -2;
	switch(uMsg){
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:
		case WM_KEYDOWN:
		case WM_KEYUP:
			if(!(hWnd->status & WST_ENABLED))
				return -1;
		break;
	}
	return hWnd->pfn_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);	
}
//---------------------------------------------------------------------------------
int SendMessage(int win,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	if(win < 0 || win >= MAX_WINDOWS)
		return -1;
	return SendMessage_HWND(&windows[win],uMsg,wParam,lParam);
}
//---------------------------------------------------------------------------------
int SendDlgItemMessage(HWND hWnd,int id,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	return SendMessage_HWND(get_dlg_item(hWnd,id),uMsg,wParam,lParam);
}
//---------------------------------------------------------------------------------
int SetWindowText(int win,char *txt)
{
	return SendMessage(win,WM_SETTEXT,0,(unsigned int)txt);
}
//---------------------------------------------------------------------------------
int get_client_rect(LPWINDOW hWnd,LPRECT lprc)
{
	if(hWnd == NULL || lprc == NULL || hWnd->pfn_proc == NULL)
		return -1;
	return hWnd->pfn_proc((struct WINDOW *)hWnd,WM_NCCALCSIZE,0,(LPARAM)lprc);
}
//---------------------------------------------------------------------------------
int get_window_from_pos(u8 screen,long x,long y)
{
	LPWINDOW p;
	
	p = last_win;
	while(p != NULL){
		if(p->screen == screen){
			if(x >= p->rc.left && x <= p->rc.right){
				if(y >= p->rc.top && y <= p->rc.bottom){
					int win;
				
					win = get_window_index(p);
					return win;
				}
			}
		}
		p = (LPWINDOW)p->prev;
	}
	return -1;
}
//---------------------------------------------------------------------------------
int enable_window(LPWINDOW hWnd,int enable)
{
	if(hWnd == NULL)
		return -1;
	if(!enable){
		hWnd->status &= ~WST_ENABLED;
		if(get_window_index(hWnd) == active_focus)
			active_focus = -1;
		hWnd->status |= 3;
		return 0;
	}
	hWnd->status |= WST_ENABLED;
	hWnd->status |= 3;
	return 0;
}
//---------------------------------------------------------------------------------
int get_next_tab_window(u8 screen,LPWINDOW hWnd)
{
	int win,i;
	
	if(hWnd != NULL){
		win = get_window_index(hWnd);
		if(win == -1)
			return -1;
		screen = hWnd->screen;
	}
	else
		win = -1;	
	if(modal_win != -1){
		if(windows[modal_win].screen == screen){
			HWND h;
					
			h = (HWND)windows[modal_win].childs.next;
			while(h != NULL){
				HWND h1;
				
				h1 = (HWND)h->childs.next;
				if(get_window_index(h) == win){
					while(h1 != NULL){
						if(h1->status & WST_ENABLED)
							return get_window_index(h1);
						h1 = (HWND)h1->childs.next;
					}
				}
				h = h1;
			}
			h = (HWND)windows[modal_win].childs.next;
			while(h != NULL){
				HWND h1;
				
				h1 = (HWND)h->childs.next;
				if(h->status & WST_ENABLED)
					return get_window_index(h);			
				h = h1;
			}			
		}
	}	
	win++;
	for(i=0;i<2;i++){
		for(;win < sizeof(windows)/sizeof(WINDOW);win++){
			if(windows[win].pfn_proc == NULL || screen != windows[win].screen 
				|| !(windows[win].style & WS_TABSTOP) || !(windows[win].status & WST_ENABLED))
				continue;
			return win;
		}
		win = 0;
	}
	return -1;
}
//---------------------------------------------------------------------------------
int get_window_index(LPWINDOW hWnd)
{
	int i;
	
	for(i=0;i<sizeof(windows)/sizeof(WINDOW);i++){
		if(&windows[i] == hWnd)
			return i;
	}
	return -1;
}
//---------------------------------------------------------------------------------
int get_free_window()
{
	int i;
	
	for(i=0;i<sizeof(windows)/sizeof(WINDOW);i++){
		if(windows[i].pfn_proc == NULL)
			return i;
	}
	return -1;
}
//---------------------------------------------------------------------------------
HWND get_dlg_item(HWND parent,int id)
{
	int i;
	
	for(i=0;i<sizeof(windows)/sizeof(WINDOW);i++){
		if(windows[i].pfn_proc == NULL || windows[i].data == NULL)
			continue;
		if(windows[i].childs.parent != (struct WINDOW*)parent)
			continue;
		if(windows[i].childs.id == id)
			return &windows[i];
	}
	return NULL;
}
//---------------------------------------------------------------------------------
int set_dlg_item(int win,HWND parent,int id)
{
	if(win < 0 || win >= MAX_WINDOWS || parent == NULL)
		return -1;
	set_parent_window(&windows[win],parent);
	windows[win].childs.id = id;
	return 0;
}
//---------------------------------------------------------------------------------
int set_focus_window(HWND hWnd)
{
	int old;
	
	if(hWnd == NULL || 
		!(hWnd->type == WC_MENU || hWnd->type == WC_COMBOBOX || hWnd->type == WC_EDIT || 
			hWnd->type == WC_LISTBOX || hWnd->type == WC_BUTTON || hWnd->type == WC_CHECKBOX) || 
				!(hWnd->status & WST_ENABLED))
		return active_focus;	
	old = active_focus;
	SendMessage(old,WM_KILLFOCUS,0,0);	
	active_focus = SendMessage_HWND(hWnd,WM_SETFOCUS,(WPARAM)old,0);
	if(hWnd->screen == (PA_Screen ^ 1))
		active_win_top = active_focus;
	return old;
}
//---------------------------------------------------------------------------------
HWND set_parent_window(HWND hWnd,HWND parent)
{
	HWND h,h1,h2;
	
	if(hWnd == NULL)
		return NULL;
	h = (HWND)hWnd->childs.parent;
	if(h == parent)
		return h;

	if(hWnd->childs.prev != NULL)
		((HWND)hWnd->childs.prev)->childs.next = (struct WINDOW *)hWnd->childs.next;
	if(hWnd->childs.next != NULL)
		((HWND)hWnd->childs.next)->childs.prev = (struct WINDOW *)hWnd->childs.prev;
		
	h1 = (HWND)parent->childs.next;
	h2 = h1;
	while(h1 != NULL){
		h2 = h1;
		h1 = (HWND)h1->childs.next;		
	}
	
	if(h2 != NULL){
		h2->childs.next = (struct WINDOW *)hWnd;		
		hWnd->childs.prev = (struct WINDOW *)h2;
	}
	else
		parent->childs.next = (struct WINDOW *)hWnd;
	hWnd->childs.next = NULL;
	hWnd->childs.parent = (struct WINDOW *)parent;		
	return h;
}
//---------------------------------------------------------------------------------
int wndproc_topwin(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	switch(uMsg){
		case WM_CREATE:
			hWnd->rc.left = hWnd->rc.top = 0;
			hWnd->rc.right = 256;
			hWnd->rc.bottom = 192;
			hWnd->screen = 1;
			//chat_win = create_editbox_win(1,16,70,224,100);			
		break;
		case WM_TIMER:
			update_digital_clock();
		break;
		case WM_ERASEBKGND:
			dmaFillWordsAsync(0, (void*)PA_DrawBg[1], 256*192*2);
		break;
		case WM_NCCALCSIZE:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		break;
	}
	return 0;
}
//---------------------------------------------------------------------------------
int wndproc_bottomwin(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	int i;
	
	switch(uMsg){
		case WM_NCCALCSIZE:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		break;
		case WM_COMMAND:			
			if(HIWORD(wParam) == 0){
				switch(LOWORD(wParam)){
					case 1:						
						i = create_popupmenu_win(0,get_window_index(hWnd));
						if(i > -1){
							RECT rc;
							
							insert_menu_item(&windows[i],10,"List...",0);
							insert_menu_item(&windows[i],11,"My Wall",0);						
							insert_menu_item(&windows[i],12,"Contact Wall",0);						
							insert_menu_item(&windows[i],13,"Change Profile Image",/*((pfb->status & 1) == 0 || modal_win != -1) ? MF_DISABLED :*/ 0);							
							insert_menu_item(&windows[i],15,"Logout",((pfb->status & 1) == 0 || modal_win != -1) ? MF_DISABLED : 0);
							get_menuitem_rect((LPWINDOW)lParam,1,&rc);
							show_popupmenu_win(i,rc.left,rc.bottom,get_window_index(hWnd));
						}
					break;
					case 2:
						i = create_popupmenu_win(0,get_window_index(hWnd));
						if(i > -1){
							RECT rc;
							
							insert_menu_item(&windows[i],16,"Preferences",(modal_win != -1) ? MF_DISABLED : 0);					
							insert_menu_item(&windows[i],14,"Search Friends...",/*((pfb->status & 1) == 0 || modal_win != -1) ? MF_DISABLED :*/ 0);												
							get_menuitem_rect((LPWINDOW)lParam,2,&rc);
							show_popupmenu_win(i,rc.left,rc.bottom,get_window_index(hWnd));
						}						
					break;
					case 3:
						i = create_popupmenu_win(0,get_window_index(hWnd));
						if(i > -1){
							RECT rc;
							
							insert_menu_item(&windows[i],20,"Help",modal_win != -1 ? MF_DISABLED : 0);
							insert_menu_item(&windows[i],21,"About...",modal_win != -1 ? MF_DISABLED : 0);
							get_menuitem_rect((LPWINDOW)lParam,3,&rc);
							show_popupmenu_win(i,rc.left,rc.bottom,get_window_index(hWnd));
						}
					break;
					case 10:
						show_contacts_list();
					break;
					case 11:
						my_wall = 1;
						SendMessage(news_win,WM_SETTEXT,1,(LPARAM)"My Wall");
					break;
					case 12:
						{
							LPFBCONTACT p;
							
							my_wall = 0;								
							p = get_selected_contact();							
							if(p != NULL)						
								SendMessage(news_win,WM_SETTEXT,1,(LPARAM)p->name);
							else
								SendMessage(news_win,WM_SETTEXT,1,(LPARAM)"");
						}
					break;
					case 13:
						change_profile_image(1);
					break;
					case 14:
						search_friends();
					break;
					case 15://Logout
					break;
					case 16:
						create_options_win(1);
					break;
					case 20://Help
						create_message_box(1,"Help","L - like TAB on Windows\r\nR + (directional keys) - move the active window\r\nSelect - click on the current window\r\nX - Close the active window or Clear Text Box",1,NULL);
					break;
					case 21://About
						{
							char *s;
							
							divf32_async(getMemFree()>>2,1024*1024*1024);//Mb
							s = (char *)&temp_buffer[SZ_LOCAL_BUFFER - temp_buffer_index - 1000];
							strcpy(s,"fb4nds - Facebook for Nintendo DS\r\nVersion 1.5\r\nby Lino\r\nMemory: ");
							{
								int value,res;
								char c[20];
																
								while(REG_DIVCNT & DIV_BUSY);
								value = REG_DIV_RESULT_L;
								res = REG_DIVREM_RESULT_L;
								sprintf(c,"%d",value);
								strcat(s,c);
								sprintf(c,".%d",res);
								divf32_async((SZ_LOCAL_BUFFER - temp_buffer_index),4096*1024);
								c[3] = 0;
								strcat(s,c);
								strcat(s,"Mb");
								strcat(s,"\r\nBuffer: ");
								while(REG_DIVCNT & DIV_BUSY);
								value = REG_DIV_RESULT_L;
								res = REG_DIVREM_RESULT_L;
								sprintf(c,"%d",value);
								strcat(s,c);
								sprintf(c,".%d",res);
								c[3] = 0;
								strcat(s,c);
								strcat(s,"Kb");								
							}
							create_message_box(1,"About...",s,1,NULL);
						}
					break;
				}
			}
			else{
				switch(LOWORD(wParam)){
					case 1002:
#ifndef _DEBUG
						if(pfb->status & 1)
#endif								
						{
							char *c,*uid,c1[3] = "0";
							int len;
									
							uid = NULL;							
							if(my_wall == 0){
								LPFBCONTACT p;
									
								p = get_selected_contact();							
								if(p != NULL)						
									uid = p->uid;
							}
							else
								uid = c1;									
							len = SendMessage(news_win,WM_GETTEXTLENGTH,0,0);
							if(uid != NULL && len > 0){
								c = malloc(len + 10);
								if(c != NULL){
									memset(c,0,len+10);
									SendMessage(news_win,WM_GETTEXT,len+10,(LPARAM)c);									
									active_focus = news_win;
									fb_send_news(c,uid);
									free(c);									
								}
							}
							SendMessage(news_win,WM_SETTEXT,0,(unsigned int)"");
						}
					break;
					case 1001:
						{
							LPFBCONTACT p;
							
							p = get_selected_contact();							
							if(p != NULL){								
#ifndef _DEBUG
								if(pfb->status & 1)
#endif								
								{
									char *c;
									int len;
									
									len = SendMessage(consolle_win,WM_GETTEXTLENGTH,0,0);
									if(len > 0){
										c = malloc(len + 10);
										if(c != NULL){
											memset(c,0,len+10);
											SendMessage(consolle_win,WM_GETTEXT,len+10,(LPARAM)c);
											fb_send_message(c,p->uid);
											free(c);
											active_focus = consolle_win;
											SendMessage(consolle_win,WM_SETTEXT,0,(unsigned int)"");																		
										}
									}
								}
							}
						}
					break;
					case 1003:
						switch(HIWORD(wParam)){
							case EN_UPDATE:
								{
									int enable;
									
									enable = SendMessage(get_window_index((HWND)lParam),WM_GETTEXTLENGTH,0,0) > 0 ? 1 : 0;
									enable_window(get_dlg_item(hWnd,1001),enable);
								}
							break;
						}
					break;
					case 1004:
						switch(HIWORD(wParam)){
							case EN_UPDATE:
								{
									int enable;
									
									enable = SendMessage(get_window_index((HWND)lParam),WM_GETTEXTLENGTH,0,0) > 0 ? 1 : 0;
									enable_window(get_dlg_item(hWnd,1002),enable);
								}
							break;
						}
					break;
					case 2000:
						switch(HIWORD(wParam)){
							case LBN_SELCHANGE:
								{
									LPFBCONTACT p;
																		
									p = get_selected_contact();							
									if(p != NULL)						
										SendMessage(news_win,WM_SETTEXT,1,(LPARAM)p->name);
									else
										SendMessage(news_win,WM_SETTEXT,1,(LPARAM)"");
								}								
							break;
						}
					break;
				}
			}
		break;
		case WM_CREATE:
			hWnd->bg_color = 0;		
			hWnd->rc.left = hWnd->rc.top = 0;
			hWnd->rc.right = 256;
			hWnd->rc.bottom = 192;

			news_win = create_editbox_win(hWnd->screen,32,16,172,32);
			if(news_win != -1){
				windows[news_win].font = 1;
				windows[news_win].style |= WS_VSCROLL|ES_MULTILINE|WS_CAPTION|WS_TABSTOP;
				set_dlg_item(news_win,hWnd,1004);
				SendMessage(news_win,WM_CREATE,0,0);
				SendMessage(news_win,WM_SETTEXT,1,(LPARAM)"My Wall");
			}
			
			consolle_win = create_editbox_win(hWnd->screen,32,49,172,32);
			if(consolle_win != -1){
				windows[consolle_win].font = 1;
				windows[consolle_win].style |= WS_VSCROLL|ES_MULTILINE|WS_TABSTOP;
				set_dlg_item(consolle_win,hWnd,1003);
				SendMessage(consolle_win,WM_CREATE,0,0);
			}

			i = create_button_win(0,215,16,40,13);
			SetWindowText(i,"Send");
			windows[i].style |= WS_TABSTOP;
			enable_window(&windows[i],0);
			set_dlg_item(i,hWnd,1002);

			i = create_button_win(0,215,49,40,13);
			SetWindowText(i,"Send");
			windows[i].style |= WS_TABSTOP;
			enable_window(&windows[i],0);
			set_dlg_item(i,hWnd,1001);

			chat_tab_win = create_chat_tab_win(0,0,192-15,256,15);

			menu_bar_bottom_win = create_menubar_win(0,0,0,256,15,get_window_index(hWnd));
			if(menu_bar_bottom_win != -1){
				old_wnd_proc = windows[menu_bar_bottom_win].pfn_proc;
				windows[menu_bar_bottom_win].pfn_proc = (LPWNDPROC)wndproc_menubar_bottomwin;
				wndproc_menubar_bottomwin(&windows[menu_bar_bottom_win],WM_CREATE,0,0);
				insert_menu_item(&windows[menu_bar_bottom_win],1,"Profile",1);
				insert_menu_item(&windows[menu_bar_bottom_win],2,"Options",1);
				insert_menu_item(&windows[menu_bar_bottom_win],3,"Help",1);
			}
		break;
		case WM_LBUTTONDOWN:
			if(ram_size()){
				if(PA_SpriteTouched(127)){
					char c[48];

					divf32_async(ram_size() >> 2,1024*1024*1024);
					strcpy(c,"Ram Disk Size : ");
					{
						char c1[8];
						u32 res,value;
						
						while(REG_DIVCNT & DIV_BUSY);
						value = REG_DIV_RESULT_L;
						res = REG_DIVREM_RESULT_L;
						sprintf(c1,"%d.",value);
						strcat(c,c1);
						sprintf(c1,"%d",res);
						c1[2] = 0;
						strcat(c,c1);
						strcat(c,"Mb");
					}
					create_message_box(1,"fb4nds",c,1,NULL);
				}
			}
		break;
		case WM_ERASEBKGND:
			{
				u32 value;
				
				value = hWnd->bg_color;
				value |= (value << 8) | (value << 16) | (value << 24);
				dmaFillWordsAsync(value, (void*)PA_DrawBg[0], 256*96*2);
			}
		break;
	}
	return 0;
}
//---------------------------------------------------------------------------------
int wndproc_menubar_topwin(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	switch(uMsg){
		case WM_CREATE:
			hWnd->bg_color = PA_RGB(7,11,19);
			hWnd->rc.left = hWnd->rc.top = 0;
			hWnd->rc.right = 256;
			hWnd->rc.bottom = 16;
			hWnd->screen = 1;						
		break;
		case WM_ERASEBKGND:
			{			
				u32 value;
				
				value = MAKELONG(hWnd->bg_color,hWnd->bg_color);
				dmaFillWordsAsync(0, (void*)&PA_DrawBg[1][0], hWnd->rc.right * hWnd->rc.bottom * 2);
			}
		break;
		case WM_PAINT:					
			{
				RECT rc={1,4,256,12};
				char s[200];
				
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;
				set_text_color(hWnd->screen,PA_RGB(31,31,31));
				set_text_font(hWnd->screen,hWnd->font);
#ifndef _DEBUG
				if(pfb->name[0] != 0){									
					sprintf(s,"Welcome %s",pfb->name);				
					draw_text(hWnd->screen,s,-1,&rc,0);
				}
#else
				draw_text(hWnd->screen,"Welcome Lino Maglione",-1,&rc,0);
#endif					
				if(ls_contacts != NULL && (pfb->status & 1))
					sprintf(s,"Chat (%d) ",get_online_contacts());
				else
					sprintf(s,"Chat disabled ");
				draw_text(hWnd->screen,s,-1,&rc,DT_VCENTER|DT_RIGHT);				
				unlock_mutex(&hWnd->critical_section);
			}
		break;		
		case WM_NCCALCSIZE:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		break;
	}
	return 0;
}
//---------------------------------------------------------------------------------
int wndproc_statusbar_topwin(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	char *p;
	
	switch(uMsg){
		case WM_NCCALCSIZE:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case WM_CREATE:
			hWnd->rc.left = 0;
			hWnd->rc.top = 176;
			hWnd->rc.right = 256;
			hWnd->rc.bottom = 192;
			hWnd->screen = 1;
			hWnd->bg_color = PA_RGB(7,11,19);
		break;
		case WM_ERASEBKGND:			
			{
				u32 value;
				
				value = MAKELONG(hWnd->bg_color,hWnd->bg_color);
				dmaFillWordsAsync(0, (void*)&PA_DrawBg[1][176*256], 256*16*2);
			}
		break;					
		case WM_PAINT:
			if(hWnd->data != NULL){				
				RECT rc;
					
				rc = hWnd->rc;
				rc.left++;rc.right -= 2;
				set_text_color(hWnd->screen,PA_RGB(31,31,31));
				set_text_font(hWnd->screen,hWnd->font);
				draw_text(hWnd->screen,(char *)hWnd->data,-1,&rc,DT_VCENTER);
				p = (char *)hWnd->data;
				p += 100;
				if(*p != 0){
					rc.left = 200;
					draw_text(1,(char *)p,-1,&rc,DT_VCENTER|DT_RIGHT);
				}
			}
		break;
		case SB_SETTEXT:
			{				
				int i;
				
				if(hWnd->data == NULL){
					hWnd->data = malloc(500);					
					memset(hWnd->data,0,500);
				}
				if(hWnd->data != NULL && lParam){					
					p = (char *)hWnd->data;
					i = (int)(wParam & 0xFFFF);
					p += i * 100;
					memset(p,0,100);
					strcpy(p,(char *)lParam);
					hWnd->status |= 3;
				}
			}
		break;
		case WM_SETTEXT:			
			if(lParam){
				char *txt;
				
				txt	= (char *)lParam;
				if(hWnd->data == NULL){
					hWnd->data = malloc(500);
					memset(hWnd->data,0,500);
				}
				if(hWnd->data != NULL){					
					//sprintf((char *)hWnd->data,"%s %d",txt,getMemFree());
					strcpy((char *)hWnd->data,txt);
					//redraw_window(get_window_index(hWnd));					
					hWnd->status |= 3;
				}						
			}
		break;		
	}
	return 0;
}
//---------------------------------------------------------------------------------
static int wndproc_menubar_bottomwin(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	switch(uMsg){
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case 1:
					switch(HIWORD(wParam)){
						case LBN_DBLCLK:
							{
								int win;
								
								win = create_message_box(1,"Friendship Request","Do You want accept this friendship?",MB_YES|MB_NO,NULL);
							}
						break;
					}
				break;
			}
		break;
		case WM_NCCALCSIZE:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case WM_CREATE:
			hWnd->screen = 0;
			PA_LoadSprite16cPal(hWnd->screen,0,(void*)notifyPal);					
			PA_CreateSprite(hWnd->screen,0,(void*)notifyTiles,OBJ_SIZE_16X16,0,0,256,0);						
			
			PA_LoadSprite16cPal(hWnd->screen,1,(void*)notify_friendPal);					
			PA_CreateSprite(hWnd->screen,1,(void*)notify_friendTiles,OBJ_SIZE_16X16,0,1,256,0);									
		break;
		case WM_TIMER:
			switch(wParam){
				case 1:
					if(PA_GetSpriteX(0,0) == 256)
						PA_SetSpriteX(0,0,240);
					else
						PA_SetSpriteX(0,0,256);
				break;
				case 2:
					if(PA_GetSpriteX(0,1) == 256)
						PA_SetSpriteX(0,1,216);
					else
						PA_SetSpriteX(0,1,256);
				break;
			}
		break;
		case WM_LBUTTONDOWN:
			if(pfb->status & 2){
				int win;
				
				if(PA_SpriteTouched(0)){									
					win = show_notify_win();
					if(win != -1)
						windows[win].style |= WS_VISIBLE;					
					return 0;
				}
			}
			if(pfb->status & 1){
				if(PA_SpriteTouched(1)){
					int win;
					
					win = show_friends_notify_win();
					if(win != -1){
						windows[win].style |= WS_VISIBLE;					
						set_dlg_item(win,hWnd,1);
					}
					return 0;					
				}
			}
		break;		
	}
	return old_wnd_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
}
//---------------------------------------------------------------------------------
static int msg_box_wnd_proc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPWINMSGBOX p;
	
	p = (LPWINMSGBOX)hWnd->data;
	if(p != NULL && p->pfn_proc != NULL)
		p->pfn_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
	switch(uMsg){
		case WM_COMMAND:
			quit_msg_box = LOWORD(wParam);
		break;
		case WM_PAINT:
			{												
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;				
				if(p != NULL && p->hdr.caption != NULL){
					u16 col;
					RECT rc;
					
					set_text_font(hWnd->screen,hWnd->font);
					if(hWnd->screen == 0)
						col = 5;
					else
						col = PA_RGB(0,0,0);																			
					col = set_text_color(hWnd->screen,col);
					get_client_rect(hWnd,&rc);
					if((p->style & 3))
						rc.bottom -= 15;
					draw_text(hWnd->screen,(char *)p->msg,-1,&rc,DT_WORDBREAK|DT_VCENTER);
					set_text_color(hWnd->screen,col);
				}
				unlock_mutex(&hWnd->critical_section);
				return 0;
			}
		break;
	}
	return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
}
//---------------------------------------------------------------------------------
int msg_box_proc(void)
{
	u8 screen;
	
	REG_IME = 1;
	screen = windows[modal_win].screen;
	while(!quit_msg_box)
		PA_WaitForVBL();
	{
		HWND hWnd;
		
		hWnd = (HWND)windows[modal_win].childs.next;
		while(hWnd != NULL){
			HWND h;
			
			h = (HWND)hWnd->childs.next;
			destroy_window(get_window_index(hWnd));
			hWnd = h;
		}
	}
	destroy_window(modal_win);
	pfn_timer2Func = NULL;
	modal_win = -1;
	redraw_windows_screen(screen);
	PA_WaitForVBL();
	return 1;
}
//---------------------------------------------------------------------------------
int create_message_box(u8 screen,char *caption,char *msg,unsigned int style,LPWNDPROC pfn_proc)
{
	LPWINMSGBOX p;
	int win,x,size,len,i;
	RECT rc;
	SIZE sz;
	
	if(msg == NULL || msg[0] == 0 || modal_win != -1)
		return -1;
	win = get_free_window();
	if(win == -1)
		return -2;		
	size = (len = strlen(msg)) + 1;
	x = 0;
	if(caption != NULL && caption[0] != 0)
		size += (x = strlen(caption)) + 1;
	windows[win].data = malloc(sizeof(WINMSGBOX)+size);		
	if(windows[win].data == NULL)
		return -3;
	get_text_extent(screen,msg,len,&sz);
	
	memset(windows[win].data,0,sizeof(WINMSGBOX)+size);
	p = (LPWINMSGBOX)windows[win].data;
	p->pfn_proc = pfn_proc;
	p->hdr.caption = (char *)(p + 1);
	p->hdr.vert_sb = p->hdr.horz_sb = -1;
	p->msg = (p->hdr.caption + x + 1);
	strcpy((char *)p->msg,msg);
	p->style = style;	
	
	sz.cx += 4;sz.cy += 4;
	if(style & 3){
		i = 0;
		if(style & 1)
			i += 55;
		if(style & 2)
			i += 55;
		if(caption != NULL)
			i += 8;
		sz.cy += 23;
		if(sz.cy < 23)
			sz.cy += 23;
		if(sz.cx < i)
			sz.cx = i;
	}
	
	windows[win].rc.left = (256 - sz.cx) >> 1;
	windows[win].rc.top = (192 - sz.cy) >> 1;
	windows[win].rc.right = windows[win].rc.left + sz.cx;
	windows[win].rc.bottom = windows[win].rc.top + sz.cy;
	windows[win].status = 3|WST_ENABLED;	
	windows[win].screen = screen;
	windows[win].style = WS_VISIBLE|WS_BORDER;
	windows[win].type = WC_DIALOG;
	if(screen == 0)
		windows[win].bg_color = 4;
	else
		windows[win].bg_color = PA_RGB(31,31,31);	
	
	if(caption != NULL && caption[0] != 0){
		windows[win].style |= WS_CAPTION|WS_CLOSE;
		windows[win].rc.bottom += 12;
		strcpy(p->hdr.caption,caption);
	}
	append_win(win);
	rc = windows[win].rc;		
	rc.right -= 3;
	rc.top = rc.bottom - 15;
	windows[win].pfn_proc = (LPWNDPROC)msg_box_wnd_proc;	
	if(style & 2){
		int w;
		
		rc.left = rc.right - 50;
		w = create_button_win(screen,rc.left,rc.top,50,13);
		if(w != -1){			
			windows[w].style |= WS_TABSTOP;
			set_dlg_item(w,&windows[win],1);
			if(style & 4)
				SetWindowText(w,"No");
			else
				SetWindowText(w,"Cancel");
			rc.right = rc.left - 5;
			set_focus_window(&windows[w]);
		}	
	}
	if(style & 1){			
		int w;
		
		rc.left = rc.right - 50;
		w = create_button_win(screen,rc.left,rc.top,50,13);
		if(w != -1){
			windows[w].style |= WS_TABSTOP|WS_CHILD;
			set_dlg_item(w,&windows[win],2);
			if((style & 4))
				SetWindowText(w,"Yes");
			else
				SetWindowText(w,"Ok");
			rc.right = rc.left - 5;
			set_focus_window(&windows[w]);		
		}
	}	
	msg_box_wnd_proc(&windows[win],WM_CREATE,0,0);	
	modal_win = win;
	quit_msg_box = 0;	
	if(!(style & MB_SYSTEMMODAL))
		pfn_timer2Func = msg_box_proc;	
	redraw_windows_screen(screen);
	if(style & MB_SYSTEMMODAL){
		while(!quit_msg_box)
			PA_WaitForVBL();
		msg_box_proc();
		return quit_msg_box;
	}	
	return win;
}
//---------------------------------------------------------------------------------
int create_dialog_win(u8 screen,int x,int y,int width,int height,int style,char *caption)
{
	LPWINEXHEADER p;
	int win;
	RECT rc;
	
	win = get_free_window();
	if(win == -1)
		return -1;
	memset(&windows[win],0,sizeof(WINDOW));
	windows[win].data = malloc(sizeof(WINEXHEADER)+500);		
	if(windows[win].data == NULL)
		return -1;
	memset(windows[win].data,0,sizeof(WINEXHEADER)+500);
	p = (LPWINEXHEADER)windows[win].data;
	p->vert_sb = p->horz_sb = -1;
	p->caption = (char *)(p+1);
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width;
	windows[win].rc.bottom = y + height;
	windows[win].status = 3|WST_ENABLED;	
	windows[win].screen = screen;
	windows[win].style = WS_VISIBLE|WS_BORDER;
	windows[win].type = WC_DIALOG;
	if(screen == 0)
		windows[win].bg_color = 4;
	else
		windows[win].bg_color = PA_RGB(31,31,31);	
	if(caption != NULL && caption[0] != 0){
		windows[win].style |= WS_CAPTION;
		strcpy(p->caption,caption);
	}
	append_win(win);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_winex_win;	
	rc = windows[win].rc;		
	rc.right -= 5;
	rc.bottom -= 5;
	rc.top = rc.bottom - 13;
	if(style & 2){
		int i;
		
		rc.left = rc.right - 50;
		i = create_button_win(screen,rc.left,rc.top,50,13);
		if(i != -1){
			windows[i].style |= WS_CHILD|WS_TABSTOP;
			set_dlg_item(i,&windows[win],1);
			SetWindowText(i,"Cancel");
			rc.right = rc.left - 5;						
		}	
	}
	if(style & 1){		
		int i;
		
		rc.left = rc.right - 40;
		i = create_button_win(screen,rc.left,rc.top,40,13);
		if(i != -1){
			windows[i].style |= WS_CHILD|WS_TABSTOP;
			set_dlg_item(i,&windows[win],2);
			SetWindowText(i,"Ok");
			rc.right = rc.left - 5;
		}
	}	
	wndproc_winex_win(&windows[win],WM_CREATE,0,0);	
	modal_win = win;
	return win;	
}
//---------------------------------------------------------------------------------
int wndproc_winex_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPWINEXHEADER p;
	
	switch(uMsg){
		case WM_NCCALCSIZE:
			{
				RECT rc;
				
				if(!lParam)
					return -1;
				rc = hWnd->rc;
				if(hWnd->style & WS_BORDER){
					rc.left += 2;
					rc.top += 2;
					rc.right -= 2;
					rc.bottom -= 2;
				}
				if(hWnd->style & WS_CAPTION)
					rc.top += 12;
				memcpy((void *)lParam,&rc,sizeof(RECT));				
			}
		break;
		case WM_SETFOCUS:
			{
				int win;
				
				win = get_window_index(hWnd);
				if(!(hWnd->style & WS_CHILD)){
					win = bring_win_top(win);
					if(win != -1){
						if((hWnd->style & (WS_VSCROLL|WS_HSCROLL)) && (hWnd->type == WC_LISTBOX || hWnd->type == WC_EDIT)){
							LPWINEXHEADER p1;
							
							p1 = (LPWINEXHEADER)hWnd->data;
							if(p1 != NULL){
								if(p1->vert_sb != -1){
									bring_win_top(p1->vert_sb);
									windows[p1->vert_sb].status |= 3;
								}
							}
						}
					}
				}
				active_focus = win;			
				if(win != -1)
					hWnd = &windows[win];					
				hWnd->status |= WST_HASFOCUS|3;
				return active_focus;			
			}
		case WM_KILLFOCUS:
			hWnd->status &= ~WST_HASFOCUS;
			hWnd->status |= 3;
			active_focus = -1;
		break;	
		case WM_CREATE:
			p = (LPWINEXHEADER)hWnd->data;
			if(p != NULL){
				if(hWnd->style & WS_VSCROLL){
					p->vert_sb = create_scrollbar_win_win(hWnd,0);
				}
//				set_scrollbar_info(p->vert_sb,0,100,10);
			}
		break;
		case WM_NCLBUTTONDOWN:
			if((hWnd->style & (WS_CAPTION|WS_CLOSE)) == (WS_CAPTION|WS_CLOSE)){
				RECT rc,rc_client;
				int x,y;
				
				get_client_rect(hWnd,&rc_client);
				rc = hWnd->rc;						
				rc.bottom = rc_client.top;				
				x = LOWORD(lParam);
				y = HIWORD(lParam);
				if(y > rc.top && y < rc.bottom && x < rc.right && x > (rc.right - 8)){					
					destroy_window(get_window_index(hWnd));
					redraw_windows();
					return 1;
				}
			}
		break;
		case WM_GETTEXTLENGTH:
			p = (LPWINEXHEADER)hWnd->data;
			if(p == NULL || p->caption == NULL)
				return -1;
			return strlen(p->caption);
		case WM_GETTEXT:
			{
				int len;
				
				p = (LPWINEXHEADER)hWnd->data;
				if(p == NULL || p->caption == NULL || !lParam)
					return -1;
				len = strlen(p->caption);				
				strncpy((char *)lParam,p->caption,wParam);
				return len > wParam ? wParam : len;
			}
		break;
		case WM_SETTEXT:
			p = (LPWINEXHEADER)hWnd->data;
			if(p == NULL || p->caption == NULL || !lParam)
				return 1;
			strcpy(p->caption,(char *)lParam);		
			hWnd->status |= 3;
		break;
		case WM_ERASEBKGND:
			{		
				u16 border_color;
				RECT rc;
				
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;
				if(hWnd->screen == 0){
					border_color = 5;
					if(hWnd->type == WC_EDIT || hWnd->type == WC_LISTBOX || hWnd->type == WC_BUTTON || hWnd->type == WC_COMBOBOX){
						if(hWnd->status & WST_HASFOCUS)
							border_color = 6;
					}
				}
				else{
					border_color = PA_RGB(0,0,0);
					if(hWnd->type == WC_EDIT || hWnd->type == WC_LISTBOX || hWnd->type == WC_BUTTON || hWnd->type == WC_COMBOBOX){
						if(hWnd->status & WST_HASFOCUS)
							border_color = PA_RGB(31,0,0);
					}
				}				
				if(hWnd->style & WS_BORDER)
					rect(hWnd->screen,&hWnd->rc,hWnd->bg_color,border_color);			
				else
					fill_rect(hWnd->screen,&hWnd->rc,hWnd->bg_color);
				if((p = (LPWINEXHEADER)hWnd->data) != NULL){
					if(hWnd->style & WS_CAPTION){
						RECT rc_client;
						u16 col;
							
						get_client_rect(hWnd,&rc_client);
						rc = hWnd->rc;						
						rc.bottom = rc_client.top;
						fill_rect(hWnd->screen,&rc,border_color);												
						if(hWnd->screen == 0)
							col = 4;
						else
							col = PA_RGB(31,31,31);										
						col = set_text_color(hWnd->screen,col);
						rc.left += 2;rc.top += 2;
						rc.right -= 2;rc.bottom -= 2;
						if(p->caption != NULL)														
							draw_text(hWnd->screen,p->caption,-1,&rc,DT_VCENTER);							
						if(hWnd->style & WS_CLOSE){						
							set_text_font(hWnd->screen,0);								
							draw_text(hWnd->screen,"X",-1,&rc,DT_VCENTER|DT_RIGHT);			
							set_text_font(hWnd->screen,hWnd->font);
						}
						set_text_color(hWnd->screen,col);
					}
				}
				unlock_mutex(&hWnd->critical_section);
			}				
		break;
	}
	return 0;
}
//---------------------------------------------------------------------------------
int read_keyboard()
{
	s32 i;
	char letter;
	
	if(PA_Screen != 0)
		return 0;
	letter = CheckKeyboard();
	i = nletter;
	if (letter > 31) {
		text[nletter] = letter;
		nletter++;
	}
	else if(letter == PA_TAB){
		SendMessage(active_win,WM_KEYDOWN,(unsigned int)VK_TAB,0);
		return 1;
	}
	else if((letter == PA_BACKSPACE) && nletter){
		nletter--;
		text[nletter] = 0;
	}
	else if (letter == '\n'){
		text[nletter] = letter;
		nletter++;
	}
	if(i != nletter){
		PA_PlaySoundEx2(1,keys_sound,keys_sound_size,127,22050,2,false,0);
		SendMessage(active_focus,WM_KEYDOWN,(unsigned int)letter,0);
		return 1;
	}
	return 0;
}
//---------------------------------------------------------------------------------
void switch_screen()
{
	u8 screen;
	int i;
	
	SendMessage(active_win_top,WM_KILLFOCUS,0,0);	
	active_win_top = active_focus = -1;
	screen = PA_Screen;
	PA_SwitchScreens();
	for(i=0;i<sizeof(windows)/sizeof(WINDOW);i++){
		if(windows[i].pfn_proc == NULL)
			continue;
		windows[i].status |= 3;
	}
	update_windows();
}
//---------------------------------------------------------------------------------
void check_tab_key()
{
	if(Pad.Newpress.L){
		int i;
		
		i = active_win_top;
		active_win_top = get_next_tab_window((PA_Screen ^ 1),i != -1 ? &windows[i] : NULL);
		if(active_win_top != -1)
			set_focus_window(&windows[active_win_top]);
	}
	else if(Pad.Held.L && Pad.Newpress.R)
		switch_screen();
}
//---------------------------------------------------------------------------------
void check_close_key()
{
	static u32 lbutton_click = 0,rbutton_click = 0;
	
	if(Pad.Newpress.X){		
		if(active_win_top != -1 && (windows[active_win_top].style & WS_CLOSE)){
			RECT rc,rc_client;
			int x,y;
				
			get_client_rect(&windows[active_win_top],&rc_client);
			rc = windows[active_win_top].rc;
			rc.bottom = rc_client.top;
			y = rc.top + ((rc.bottom - rc.top) >> 1);
			x = rc.right - 3;
			SendMessage(active_win_top,WM_NCLBUTTONDOWN,1,MAKELPARAM(x,y));
		}
		else if(active_focus != -1 && windows[active_focus].type == WC_EDIT)
			SendMessage(active_focus,WM_SETTEXT,0,(LPARAM)"");		
	}
	if(Pad.Newpress.Select){	
		if(lbutton_click != 0){//Double Press on Select Button lile Double Click
			int diff;
			
			diff = ticks - lbutton_click;
			lbutton_click = ticks;
			if(diff > 8 && diff < 20){				
				lbutton_click = 0;
				if(active_win_top != -1){
					RECT rc;
					
					get_client_rect(&windows[active_win_top],&rc);
					SendMessage(active_win_top,WM_LBUTTONDBLCLK,1,MAKELPARAM((rc.right - rc.left) / 2,(rc.bottom - rc.top) / 2));
					return;
				}
			}
		}
		else
			lbutton_click = ticks;
		SendMessage(active_win_top,WM_KEYDOWN,VK_RETURN,0);
	}
	else if(Pad.Held.Select)
		SendMessage(active_win_top,WM_KEYDOWN,VK_RETURN,1);
	else if(Pad.Released.Select)		
		SendMessage(active_win_top,WM_KEYUP,VK_RETURN,0);		
	
	if(Pad.Newpress.Start){
		if(rbutton_click != 0){//Double Press on Start Button Open the MenuBar
			int diff;
			
			diff = ticks - rbutton_click;
			rbutton_click = ticks;
			if(diff > 8 && diff < 20){				
				LPPOPUPMENU p;
				int i;

				rbutton_click = 0;
				for(i=0;i<sizeof(windows)/sizeof(WINDOW);i++){
					if(windows[i].pfn_proc == NULL || windows[i].type != WC_MENU || 
						windows[i].screen != (PA_Screen ^ 1))
							continue;
						if((p = (LPPOPUPMENU)windows[i].data) != NULL){
							if(p->flags & PMT_BAR){							
								
								SendMessage(i,WM_KEYDOWN,VK_RIGHT,0);
								set_focus_window(&windows[i]);
								return;
							}
						}
				}							
			}			
		}
		else
			rbutton_click = ticks;	
		SendMessage(active_win_top,WM_KEYDOWN,VK_SHIFT,0);
	}
	else if(Pad.Held.Start)
		SendMessage(active_win_top,WM_KEYDOWN,VK_SHIFT,1);
	else if(Pad.Released.Start)
		SendMessage(active_win_top,WM_KEYUP,VK_SHIFT,0);			
		
}
//---------------------------------------------------------------------------------
void check_touch_screen()
{
	int win,x,y;
	
	if(Stylus.Newpress){	
		unsigned int msg;
		RECT rc;
		
		win = get_window_from_pos(PA_Screen,Stylus.X,Stylus.Y);
		if(win != -1){
			if(win != active_focus)
				set_focus_window(&windows[win]);
			get_client_rect(&windows[win],&rc);
			if(Stylus.X <= rc.left || Stylus.X >= rc.right || Stylus.Y <= rc.top || Stylus.Y >= rc.bottom)
				msg = WM_NCLBUTTONDOWN;
			else
				msg = WM_LBUTTONDOWN;
			SendMessage(win,msg,0,MAKELPARAM(Stylus.X,Stylus.Y));
		}
	}		
	else if(Stylus.Held){	
		win = get_window_from_pos(PA_Screen,Stylus.X,Stylus.Y);
		if(win != -1){
			x = Stylus.X;
			y = Stylus.Y;
			x -= windows[win].rc.left;
			y -= windows[win].rc.top;
			SendMessage(win,WM_MOUSEMOVE,0,MAKELPARAM(x,y));
		}
	}
	if(Stylus.DblClick){
		win = get_window_from_pos(PA_Screen,Stylus.X,Stylus.Y);
		if(win != -1){			
			x = Stylus.X;
			y = Stylus.Y;
			SendMessage(win,WM_LBUTTONDBLCLK,0,MAKELPARAM(x,y));			
		}
	}
}
//---------------------------------------------------------------------------------
int move_window(LPWINDOW hWnd,LPRECT lprc)
{
	unsigned long cr;
	LPWINEXHEADER p;
	
	if(hWnd == NULL || lprc == NULL)
		return -1;
	EnterCriticalSection(&cr);
	memcpy(&hWnd->rc,lprc,sizeof(RECT));
	if((hWnd->style & (WS_VSCROLL|WS_HSCROLL)) && (hWnd->type == WC_LISTBOX || hWnd->type == WC_EDIT)){
		p = (LPWINEXHEADER)hWnd->data;
		if(p != NULL){
			if(p->vert_sb != -1){
				hWnd = &windows[p->vert_sb];
				if(hWnd->type == WC_SCROLLBAR){
					hWnd->rc.left = lprc->right;
					hWnd->rc.right = lprc->right + 7;
					hWnd->rc.top = lprc->top;
					hWnd->rc.bottom = lprc->bottom;
				}
			}
			if(p->horz_sb != -1){
			}
		}
	}
	LeaveCriticalSection(&cr);
	return 0;
}
//---------------------------------------------------------------------------------
int check_move_win()
{
	if(active_win_top != -1 && Pad.Held.R && (windows[active_win_top].style & WS_CAPTION)){
		RECT rc;
		int i;
		
		rc = windows[active_win_top].rc;
		i = 0;
		if(Pad.Held.Up){
			if(rc.top > 0){
				rc.top--;
				rc.bottom--;
				i |= 1;
			}
		}
		else if(Pad.Held.Left){
			if(rc.left > 0){
				rc.left--;
				rc.right--;
				i |= 2;
			}
		}
		else if(Pad.Held.Right){
			if(rc.right < 256){
				rc.left++;
				rc.right++;
				i |= 4;
			}
		}
		else if(Pad.Held.Down){
			if(rc.bottom < 192){
				rc.bottom++;
				rc.top++;
				i |= 8;
			}
		}				
		if(i){
			move_window(&windows[active_win_top],&rc);
			redraw_windows_screen(1);
		}
		return 1;
	}
	return 0;
}
//---------------------------------------------------------------------------------
int def_vbl_proc(void)
{
	read_keyboard();
	if(!check_move_win()){
		if(Pad.Newpress.Up)
			SendMessage(active_win_top,WM_KEYDOWN,VK_UP,0);
		else if(Pad.Held.Up)
			SendMessage(active_win_top,WM_KEYDOWN,VK_UP,1);
		else if(Pad.Released.Up)
			SendMessage(active_win_top,WM_KEYUP,VK_UP,0);

		if(Pad.Newpress.Left)
			SendMessage(active_win_top,WM_KEYDOWN,VK_LEFT,0);
		else if(Pad.Held.Left)
			SendMessage(active_win_top,WM_KEYDOWN,VK_LEFT,1);
		else if(Pad.Released.Left)
			SendMessage(active_win_top,WM_KEYUP,VK_LEFT,0);
			
		if(Pad.Newpress.Right)
			SendMessage(active_win_top,WM_KEYDOWN,VK_RIGHT,0);
		else if(Pad.Held.Right)
			SendMessage(active_win_top,WM_KEYDOWN,VK_RIGHT,1);
		else if(Pad.Released.Right)
			SendMessage(active_win_top,WM_KEYUP,VK_RIGHT,0);
		
		if(Pad.Newpress.Down)
			SendMessage(active_win_top,WM_KEYDOWN,VK_DOWN,0);
		else if(Pad.Held.Down)
			SendMessage(active_win_top,WM_KEYDOWN,VK_DOWN,1);
		else if(Pad.Released.Down)
			SendMessage(active_win_top,WM_KEYUP,VK_DOWN,0);		
	}
	check_tab_key();
	check_close_key();
	check_touch_screen();	
	return 1;
}
//---------------------------------------------------------------------------------
void adjust_input_text(char *txt)
{	
	memset(text,0,sizeof(text));
	if(txt == NULL){		
		nletter = 0;	
		return;
	}
	nletter = strlen_UTF(txt);			
	memcpy(text,txt,nletter);	
}
//---------------------------------------------------------------------------------
int init_winex_header(HWND hWnd,unsigned int size)
{
	LPWINEXHEADER p;
	
	if(hWnd == NULL || hWnd->data == NULL)
		return -1;
	if(size == 0)
		size = sizeof(WINEXHEADER);
	p = (LPWINEXHEADER)hWnd->data;
	p->caption = (char *)p;
	p->caption += size;
	p->pfn_ownerdraw = NULL;
	p->vert_sb = p->horz_sb = -1;
	return 0;
}