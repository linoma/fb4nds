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

#include "button.h"
#include "draws.h"

static int wndproc_button_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
//---------------------------------------------------------------------------------
int create_button_win(unsigned char screen,int x,int y,int width,int height)
{
	int win;
	
	win = get_free_window();
	if(win == -1)
		return -1;
	memset(&windows[win],0,sizeof(WINDOW));
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width;
	windows[win].rc.bottom = y + height;
	windows[win].status = 3|WST_ENABLED;	
	windows[win].data = malloc(sizeof(WINEXHEADER)+500);
	windows[win].screen = screen;
	windows[win].type = WC_BUTTON;
	windows[win].style = WS_VISIBLE|WS_CHILD|WS_BORDER;
	if(screen == 0)
		windows[win].bg_color = 2;
	else
		windows[win].bg_color = PA_RGB(7,11,19);
	if(windows[win].data == NULL)
		return -1;
	memset(windows[win].data,0,sizeof(WINEXHEADER) + 500);
	init_winex_header(&windows[win],0);
	append_win(win);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_button_win;	
	wndproc_button_win(&windows[win],WM_CREATE,0,0);
	return win;
}
//---------------------------------------------------------------------------------
static int wndproc_button_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPWINEXHEADER p;
	
	switch(uMsg){
		default:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case WM_KEYDOWN:
			if(wParam != VK_RETURN || lParam)
				return 0;
		case WM_LBUTTONDOWN:
			hWnd->status &= ~WST_HASFOCUS;
			hWnd->status |= 3;			
			SendMessage(get_window_index(hWnd->childs.parent),WM_COMMAND,MAKEWPARAM(hWnd->childs.id,BN_CLICKED),(LPARAM)hWnd);
		break;
		case WM_PAINT:
			{
				u16 col;
				
				p = (LPWINEXHEADER)hWnd->data;
				if(p != NULL){
					if(hWnd->screen == 0){						
						if(hWnd->status & WST_ENABLED)
							col = 4;
						else
							col = 3;
					}
					else{
						if(hWnd->status & WST_ENABLED)
							col = PA_RGB(31,31,31);
						else
							col = PA_RGB(24,24,24);
					}
					col = set_text_color(hWnd->screen,col);
					draw_text(hWnd->screen,p->caption,-1,&hWnd->rc,DT_VCENTER|DT_CENTER);
					set_text_color(hWnd->screen,col);
				}
			}
		break;
	}
	return 0;
}


