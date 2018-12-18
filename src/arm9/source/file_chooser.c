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
 
#include "file_chooser.h"
#include "combobox.h"
#include "editbox.h"
#include "button.h"
#include "fb_list.h"
#include "draws.h"
#include <dirent.h>

extern u8 *temp_buffer;
static LPWNDPROC old_dlg_proc = NULL;
extern int (*pfn_MainFunc)(void);
extern int (*pfn_timer2Func)(void);
extern int modal_win;
//---------------------------------------------------------------------------------
static int enum_files(char *path)
{	
	DIR *dir;
	char *s,*s1,*filter;
	LPFBLIST file_list;
	
	SendDlgItemMessage(&windows[modal_win],1000,LB_RESETCONTENT,0,0);
	file_list = init_fb_list(&temp_buffer[1000],15000);
	s = (char *)&temp_buffer[16000];
	s1 = s + 2500;
	filter = NULL;
	{
		int index;
		
		index = SendDlgItemMessage(&windows[modal_win],1004,CB_GETCURSEL,0,0);
		if(index != CB_ERR){
			index = SendDlgItemMessage(&windows[modal_win],1004,CB_GETLBTEXT,(WPARAM)index,0);
			if(index != LB_ERR){
				index += strlen_UTF((char *)index) + 8;
				index = (index & ~3) - 4;				
				filter = (char *)*((u32 *)index);
				if(filter != NULL){
					filter = strchr(filter,'.');					
					if(filter != NULL && filter[1] == '*')
						filter = NULL;
				}
			}
		}
	}
	dir = opendir(path);
	if(dir != NULL){
		struct dirent *ent;
		int i;
		
		strcpy(s1,path);
		if(s1[(i = strlen(s1)) - 1] != '/'){
			strcat(s1,"/");
			i++;
		}
		while((ent = readdir(dir)) != NULL) {							
			if(ent->d_name[0] != '.'){
				DIR *d;
				
				strcat(s1,ent->d_name);
				d = opendir(s1);
				s[0] = 0;
				if(d != NULL){
					strcpy(s,"[DIR] ");
					closedir(d);
				}
				else{
					if(filter != NULL && strnstri(ent->d_name,filter,strlen(ent->d_name)) == NULL){
						s1[i] = 0;
						continue;
					}
				}
				strcat(s,ent->d_name);
				add_fb_list_item(file_list,s,strlen(s) + 1);				
			}
			s1[i] = 0;
		}
		closedir(dir);		
	}
	{
		LPFBLIST_ITEM item;
		char *data;
		
		sort_fb_list(file_list,NULL);
		data = get_fb_list_first(file_list,&item);
		while(data != NULL){
			SendDlgItemMessage(&windows[modal_win],1000,LB_ADDSTRING,0,(LPARAM)data);
			data = get_fb_list_next(file_list,&item);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------------
static int update_button_status(LPWINDOW hWnd)
{
	int flag,len;
	
	if(hWnd == NULL)
		return -1;
	flag = 0;
	memset(temp_buffer,0,1000);
	if((len = SendDlgItemMessage(hWnd,1002,WM_GETTEXT,1000,(LPARAM)temp_buffer)) > 0){
		if(len > 1){
			char *p;
			
			p = strrchr((char *)temp_buffer,'/');
			if(p != NULL){
				flag = 1;
			}
		}
	}
	enable_window(get_dlg_item(hWnd,1005),flag);
	return 0;
}
//---------------------------------------------------------------------------------
static int file_chooser_dlgproc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	int res;
	
	res = old_dlg_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
	switch(uMsg){
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
				{
					HWND h;					
					RECT rc;
					
					h = get_dlg_item(hWnd,1002);
					get_client_rect(h,&rc);
					{
						RECT rc1;
						
						get_client_rect(hWnd,&rc1);
						rc.right = rc.left+1;
						rc.left = rc1.left;
					}
					set_text_font(hWnd->screen,hWnd->font);
					draw_text(hWnd->screen,"Search in",-1,&rc,DT_VCENTER);										
				}
				set_text_color(hWnd->screen,color);
				unlock_mutex(&hWnd->critical_section);
			}	
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case 1005://up button
					{
						int len;
						
						len = SendDlgItemMessage(&windows[modal_win],1002,WM_GETTEXT,1000,(LPARAM)temp_buffer);
						if(len > 0){
							char *p;
							
							if(temp_buffer[len-1] == '/')
								temp_buffer[--len] = 0;							
							p = strrchr((char *)temp_buffer,'/');
							if(p != NULL)
								p[0] = 0;
							if(temp_buffer[0] == 0){
								temp_buffer[0] = '/';
								temp_buffer[1] = 0;
							}
							SendDlgItemMessage(&windows[modal_win],1002,WM_SETTEXT,0,(LPARAM)temp_buffer);
							SendDlgItemMessage(hWnd,1003,WM_SETTEXT,0,(LPARAM)"");
							enum_files((char *)temp_buffer);					
							update_button_status(&windows[modal_win]);
						}
					}
				break;				
				case 1004://combobox
					switch(HIWORD(wParam)){
						case CBN_SELENDOK:
							SendDlgItemMessage(hWnd,1003,WM_SETTEXT,0,(LPARAM)"");
							memset(temp_buffer,0,1000);
							SendDlgItemMessage(&windows[modal_win],1002,WM_GETTEXT,1000,(LPARAM)temp_buffer);
							enum_files((char *)temp_buffer);
							update_button_status(&windows[modal_win]);
						break;
					}
				break;
				case 1000://listbox
					switch(HIWORD(wParam)){
						case LBN_DBLCLK:
							{
								int index;
								
								index = SendMessage_HWND((HWND)lParam,LB_GETCURSEL,0,0);
								if(index != LB_ERR){								
									index = SendMessage_HWND((HWND)lParam,LB_GETITEM,(WPARAM)index,0);
									if(index != LB_ERR){			
										if(((char *)index)[0] == '['){
											char *p;
											
											p = (char *)index;
											memset(temp_buffer,0,1000);
											index = SendDlgItemMessage(&windows[modal_win],1002,WM_GETTEXT,1000,(LPARAM)temp_buffer);
											if(temp_buffer[index-1] != '/')
												strcat((char *)temp_buffer,"/");
											strcat((char *)temp_buffer,&p[6]);
											SendDlgItemMessage(&windows[modal_win],1002,WM_SETTEXT,0,(LPARAM)temp_buffer);
											SendDlgItemMessage(hWnd,1003,WM_SETTEXT,0,(LPARAM)"");
											enum_files((char *)temp_buffer);
											update_button_status(&windows[modal_win]);
										}
										else
											quit_msg_box = 2;
									}
								}
							}
						break;
						case LBN_SELCHANGE:
							{
								int index;
								
								SendDlgItemMessage(hWnd,1003,WM_SETTEXT,0,(LPARAM)"");
								index = SendMessage_HWND((HWND)lParam,LB_GETCURSEL,0,0);
								if(index != LB_ERR){								
									index = SendMessage_HWND((HWND)lParam,LB_GETITEM,(WPARAM)index,0);
									if(index != LB_ERR){
										if(((char *)index)[0] != '[')
											SendDlgItemMessage(hWnd,1003,WM_SETTEXT,0,(LPARAM)index);
									}
								}
							}
						break;
					}
				break;
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
int create_file_chooser(int screen,char *title,char *filter,char *dir,char *filename)
{
	int win,w;
	
	if(screen > 1 || filename == NULL)
		return -1;
	*filename = 0;
	if(title == NULL || title[0] == 0){
		title = (char *)temp_buffer;
		strcpy(title,"Open...");
	}
	win = create_dialog_win(screen,10,10,230,160,3,title);
	if(win < 0)
		return -1;
	windows[win].style |= WS_CLOSE;
	old_dlg_proc = windows[win].pfn_proc;
	windows[win].pfn_proc = (LPWNDPROC)file_chooser_dlgproc;	

	w = create_listbox_win(screen,15,45,210,75);
	if(w < 0)
		return -1;
	windows[w].style |= WS_TABSTOP|WS_VISIBLE|WS_CHILD;
	set_dlg_item(w,&windows[win],1000);
	{
		LPWINEXHEADER p;
		
		p = (LPWINEXHEADER)windows[w].data;
		if(p != NULL && p->vert_sb != -1)
			windows[p->vert_sb].style |= WS_VISIBLE;		
	}
		
	w = create_editbox_win(screen,15,122,210,12);
	if(w < 0)
		return -1;	
	windows[w].style |= WS_TABSTOP|WS_CHILD|WS_VISIBLE;
	set_dlg_item(w,&windows[win],1003);	
	set_focus_window(&windows[w]);
		
	w = create_combobox_win(screen,15,136,135,40,0);
	if(w < 0)
		return -1;
	windows[w].style |= WS_TABSTOP|WS_VISIBLE|WS_CHILD;
	set_dlg_item(w,&windows[win],1004);
	if(filter != NULL){
		int r;
		
		while(*filter != 0){
			r = SendMessage_HWND(&windows[w],CB_ADDSTRING,0,(LPARAM)filter);
			filter += strlen(filter) + 1;
			if(*filter){//Extension
				*((u32 *)r) = (u32)filter;
				filter += strlen(filter) + 1;
			}
			else
				*((u32 *)r) = 0;
		}
		SendMessage_HWND(&windows[w],CB_SETCURSEL,0,0);
	}

	w = create_editbox_win(screen,55,29,170,12);
	if(w == -1)
		return -1;
	windows[w].style |= WS_TABSTOP|WS_CHILD|WS_VISIBLE;
	set_dlg_item(w,&windows[win],1002);	
	set_focus_window(&windows[w]);
	if(dir == NULL || dir[0] == 0){
		dir = (char *)temp_buffer;
		dir[0] = 0;
		strcpy(dir,(char *)get_module_filename());
	}
	else
		strcpy((char *)temp_buffer,dir);
	SendMessage_HWND(&windows[w],WM_SETTEXT,0,(LPARAM)dir);	
	{
		RECT rc;
		
		get_client_rect(get_dlg_item(&windows[win],1),&rc);		
		w = create_button_win(screen,15,rc.top-2,rc.right - rc.left,rc.bottom - rc.top + 4);
		if(w == -1)
			return -1;		
		windows[w].style |= WS_TABSTOP|WS_VISIBLE|WS_CHILD;
		SetWindowText(w,"Up");
		set_dlg_item(w,&windows[win],1005);		
	}
	
	enum_files(dir);
	update_button_status(&windows[modal_win]);		
	
	redraw_windows_screen(screen);
	quit_msg_box = 0;
	while(!quit_msg_box)
		PA_WaitForVBL();
	if(quit_msg_box == 2){		
		memset(temp_buffer,0,5000);
		//path
		w = SendDlgItemMessage(&windows[modal_win],1002,WM_GETTEXT,1000,(LPARAM)temp_buffer);
		//filename
		win = SendDlgItemMessage(&windows[modal_win],1003,WM_GETTEXT,1000,(LPARAM)&temp_buffer[2000]);		
		if(w > 0 && win > 0){
			if(temp_buffer[w-1] != '/')
				strcat((char *)temp_buffer,"/");
			strcat((char *)temp_buffer,(char *)&temp_buffer[2000]);
			strcpy(filename,(char *)temp_buffer);
		}
		else
			*filename = 0;
	}
	msg_box_proc();
	return quit_msg_box;		
}