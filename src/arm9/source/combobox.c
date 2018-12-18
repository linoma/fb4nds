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

#include "combobox.h"
#include "draws.h"
#include "listbox.h"
#include "editbox.h"

static int wndproc_combobox_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
//---------------------------------------------------------------------------------
int create_combobox_win(unsigned char screen,int x,int y,int width,int height,int style)
{
	int win;
	LPWINEXHEADER p;
	
	win = get_free_window();
	if(win == -1)
		return -1;	
	memset(&windows[win],0,sizeof(WINDOW));
	windows[win].data = malloc(sizeof(WINEXHEADER) + 500);
	if(windows[win].data == NULL)
		return -1;
	memset(windows[win].data,0,sizeof(WINEXHEADER) + 500);	
	init_winex_header(&windows[win],450);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_combobox_win;	
	p = (LPWINEXHEADER)windows[win].data;
	if(style & CBS_DROPDOWN){
		p->horz_sb = create_editbox_win(screen,x,y,width-10,10);
		if(p->horz_sb < 0){
			destroy_window(win);
			return -1;		
		}
		windows[p->horz_sb].style &= ~WS_BORDER;
		set_dlg_item(p->horz_sb,&windows[win],1);
	}
	
	p->vert_sb = create_listbox_win(screen,x,y+9,width,height);
	if(p->vert_sb < 0){
		destroy_window(win);
		return -1;	
	}
	
	windows[p->vert_sb].style &= ~WS_VISIBLE;
	windows[p->vert_sb].style |= WS_CHILD|WS_TABSTOP;
	set_dlg_item(p->vert_sb,&windows[win],2);
	
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width;
	windows[win].rc.bottom = y + 10;
	windows[win].status = 3|WST_ENABLED;		
	windows[win].screen = screen;
	windows[win].type = WC_COMBOBOX;
	windows[win].style = WS_VISIBLE|WS_CHILD|WS_BORDER|(style & 0xFFF0000)|WS_TABSTOP;
	if(screen == 0)
		windows[win].bg_color = 4;
	else
		windows[win].bg_color = PA_RGB(31,31,31);
	append_win(win);
	
	wndproc_combobox_win(&windows[win],WM_CREATE,0,0);
	return win;
}
//---------------------------------------------------------------------------------
static int wndproc_combobox_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPWINEXHEADER p;
	
	switch(uMsg){
		case WM_LBUTTONDOWN:
			{
				int x,y;
				RECT rc;
				
				p = (LPWINEXHEADER)hWnd->data;				
				x = LOWORD(lParam);
				y = HIWORD(lParam);
				get_client_rect(hWnd,&rc);
				rc.left = rc.right - 7;
				if(x >= rc.left && x < rc.right){
					if(y >= rc.top && y < rc.bottom){
						windows[p->vert_sb].style |= WS_VISIBLE;
						invalidate_window(p->vert_sb,NULL,1);
						set_focus_window(&windows[p->vert_sb]);
					}
				}
			}
		break;
		case WM_KEYDOWN:
			switch(wParam){
				case VK_RETURN:
					{
						RECT rc_client;
													
						if(!lParam){							
							get_client_rect(hWnd,&rc_client);
							hWnd->pfn_proc((struct WINDOW *)hWnd,WM_LBUTTONDOWN,0,
								MAKELPARAM(rc_client.right-7,(rc_client.top + ((rc_client.bottom - rc_client.top) >> 1))));
						}
					}
				break;
			}
		break;
		case WM_DESTROY:
			p = (LPWINEXHEADER)hWnd->data;
			if(p != NULL){
				if(p->horz_sb != -1)
					destroy_window(p->horz_sb);
				if(p->vert_sb != -1)
					destroy_window(p->vert_sb);
				free(p);
			}
		break;
		case WM_PAINT:
			{				
				draw_image(hWnd->screen,hWnd->rc.right-8,
					hWnd->rc.top+(((hWnd->rc.bottom - hWnd->rc.top)-4) >> 1),
					7,4,system_images,0xFFFF,-1);				
				p = (LPWINEXHEADER)hWnd->data;				
				if(p != NULL && p->caption != NULL && p->caption[0]){
					RECT rc;
					u16 col,old_col;
					
					get_client_rect(hWnd,&rc);
					rc.right -= 8;
					if(hWnd->screen == 0)
						col = 5;
					else
						col = PA_RGB(0,0,0);														
					set_text_font(hWnd->screen,hWnd->font);
					old_col = set_text_color(hWnd->screen,col);					
					draw_text(hWnd->screen,p->caption,-1,&rc,DT_VCENTER|DT_LEFT);
					set_text_color(hWnd->screen,old_col);
				}
			}
		break;
		case CB_ADDSTRING:		
		case CB_GETCOUNT:
		case CB_GETCURSEL:
		case CB_GETLBTEXT:
			p = (LPWINEXHEADER)hWnd->data;
			if(p != NULL){			
				if(p->vert_sb > -1)
					return SendMessage(p->vert_sb,uMsg,wParam,lParam);
			}
			return CB_ERR;
		case CB_SETCURSEL:
			p = (LPWINEXHEADER)hWnd->data;
			if(p != NULL){			
				if(p->vert_sb > -1){
					int index;
					
					index = SendMessage(p->vert_sb,uMsg,wParam,lParam);
					if(index != LB_ERR){
						index = SendMessage(p->vert_sb,LB_GETITEM,(WPARAM)index,0);
						if(index != LB_ERR){
							strcpy(p->caption,(char *)index);						
							invalidate_window(get_window_index(hWnd),NULL,1);
							return wParam;
						}
					}
				}
			}		
			return CB_ERR;					
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case 2:					
					switch(HIWORD(wParam)){
						case LBN_DBLCLK:
							{
								int index;
																						
								((HWND)lParam)->style &= ~WS_VISIBLE;								
								set_focus_window(hWnd);
								if((p = (LPWINEXHEADER)hWnd->data) != NULL && p->caption != NULL){
									index = SendMessage_HWND((HWND)lParam,LB_GETCURSEL,0,0);
									if(index != LB_ERR){									
										index = SendMessage_HWND((HWND)lParam,LB_GETITEM,(WPARAM)index,0);
										if(index != LB_ERR){
											strcpy(p->caption,(char *)index);
											if(hWnd->childs.parent != NULL)
												SendMessage(get_window_index(hWnd->childs.parent),WM_COMMAND,MAKEWPARAM(hWnd->childs.id,CBN_SELENDOK),(LPARAM)hWnd);																		
											redraw_windows_screen(hWnd->screen);
										}
									}
								}
							}
						break;
					}
				break;
			}
			return 0;
		default:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}