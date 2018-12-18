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

#include "checkbox.h"
#include "draws.h"

static int wndproc_checkbox_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
//---------------------------------------------------------------------------------
int create_checkbox_win(unsigned char screen,int x,int y,int width,int height)
{
	int win;
	
	win = get_free_window();
	if(win < 0)
		return -1;
	memset(&windows[win],0,sizeof(WINDOW));
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width;
	{
		SIZE sz;
		
		get_text_extent(screen,"Ay",2,&sz);
		height = sz.cy + 2;
	}
	windows[win].rc.bottom = y + height;
	windows[win].status = 3|WST_ENABLED;	
	windows[win].data = malloc(sizeof(WINEXHEADER)+500);
	windows[win].screen = screen;
	windows[win].type = WC_CHECKBOX;
	windows[win].style = WS_VISIBLE|WS_CHILD|WS_TABSTOP;
	if(screen == 0)
		windows[win].bg_color = 1;
	else
		windows[win].bg_color = PA_RGB(31,31,31);
	if(windows[win].data == NULL)
		return -1;
	memset(windows[win].data,0,sizeof(WINEXHEADER) + 500);
	init_winex_header(&windows[win],0);
	{
		LPWINEXHEADER p;
		
		p = (LPWINEXHEADER)windows[win].data;
		p->vert_sb = 0;
	}
	append_win(win);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_checkbox_win;	
	wndproc_checkbox_win(&windows[win],WM_CREATE,0,0);
	return win;
}
//---------------------------------------------------------------------------------
static int wndproc_checkbox_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPWINEXHEADER p;
	
	switch(uMsg){
		default:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case BM_SETCHECK:
			if((p = (LPWINEXHEADER)hWnd->data) != NULL){
				p->vert_sb = (int)(wParam & 3);				
				hWnd->status |= 2;			
			}
		break;
		case BM_GETCHECK:
			if((p = (LPWINEXHEADER)hWnd->data) != NULL)
				return (int)(p->vert_sb & 3);
		case WM_KEYDOWN:
			if(wParam != VK_RETURN || lParam)
				return 0;
		case WM_LBUTTONDOWN:
			if((p = (LPWINEXHEADER)hWnd->data) != NULL){
				p->vert_sb ^= 1;				
				hWnd->status |= 2;			
			}			
			SendMessage(get_window_index(hWnd->childs.parent),WM_COMMAND,MAKEWPARAM(hWnd->childs.id,BN_CLICKED),(LPARAM)hWnd);
		break;
		case WM_ERASEBKGND:
			{		
				RECT rc;
				
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;
				fill_rect(hWnd->screen,&hWnd->rc,hWnd->bg_color);
				rc = hWnd->rc;
				rc.left++;
				rc.right--;
				rc.top++;
				rc.bottom--;
				rc.right = rc.left + (rc.bottom - rc.top);								
				if((p = (LPWINEXHEADER)hWnd->data) != NULL){					
					RECT rc_client;
					u16 col;
							
					rc.left = rc.right + 2;
					rc.right = hWnd->rc.right - 1;
					if(hWnd->screen == 0){
						if(hWnd->status & WST_ENABLED)
							col = 5;
						else
							col = 3;
					}
					else{
						if(hWnd->status & WST_ENABLED)
							col = PA_RGB(0,0,0);
						else
							col = PA_RGB(24,24,24);
					}
					col = set_text_color(hWnd->screen,col);
					if(p->caption != NULL)														
						draw_text(hWnd->screen,p->caption,-1,&rc,DT_VCENTER);							
					set_text_color(hWnd->screen,col);
				}
				unlock_mutex(&hWnd->critical_section);
			}				
		break;
		case WM_PAINT:
			{
				RECT rc;
				u16 col,border_color;
				
				if(hWnd->screen == 0){
					if(!(hWnd->status & WST_ENABLED))
						border_color = 3;
					else{
						border_color = 5;
						if(hWnd->status & WST_HASFOCUS)
							border_color = 6;
					}
				}
				else{
					if(!(hWnd->status & WST_ENABLED))
						border_color = PA_RGB(24,24,24);
					else{
						border_color = PA_RGB(0,0,0);
						if(hWnd->status & WST_HASFOCUS)
							border_color = PA_RGB(31,0,0);
					}
				}

				rc = hWnd->rc;
				rc.left++;
				rc.right--;
				rc.top++;
				rc.bottom--;
				rc.right = rc.left + (rc.bottom - rc.top);
				
				rect(hWnd->screen,&rc,hWnd->bg_color,border_color);
				
				rc.left+=2;
				rc.right-=2;
				rc.top+=2;
				rc.bottom-=2;
				
				if(hWnd->screen == 0){
					if(hWnd->status & WST_ENABLED)
						col = 5;
					else
						col = 3;
				}
				else{
					if(hWnd->status & WST_ENABLED)
						col = PA_RGB(0,0,0);
					else
						col = PA_RGB(24,24,24);
				}
				if((p = (LPWINEXHEADER)hWnd->data) != NULL){					
					if(p->vert_sb)
						fill_rect(hWnd->screen,&rc,col);
				}
			}
		break;
	}
	return 0;
}


