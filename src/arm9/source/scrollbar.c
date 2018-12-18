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

#include "scrollbar.h"
#include "draws.h"

extern u32 ticks;
static int wndproc_scrollbar_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
//---------------------------------------------------------------------------------
int create_scrollbar_win(unsigned char screen,int x,int y,int width,int height)
{	
	int win;	
	
	win = get_free_window();
	if(win == -1)
		return -1;
	memset(&windows[win],0,sizeof(WINDOW));
	windows[win].data = malloc(sizeof(SCROLLBARDATA));		
	if(windows[win].data == NULL)
		return -1;
	memset(windows[win].data,0,sizeof(SCROLLBARDATA));
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width;
	windows[win].rc.bottom = y + height;
	windows[win].status |= 3;	
	windows[win].style = WS_BORDER;
	windows[win].screen = screen;
	if(screen == 0)
		windows[win].bg_color = 4;
	else
		windows[win].bg_color = PA_RGB(31,31,31);
	windows[win].type = WC_SCROLLBAR;
	append_win(win);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_scrollbar_win;	
	wndproc_scrollbar_win(&windows[win],WM_CREATE,0,0);	
	return win;
}
//---------------------------------------------------------------------------------
int create_scrollbar_win_win(HWND hWnd,int style)
{
	LPSCROLLBARDATA p;
	RECT rc;
	int win;
	
	rc.left = hWnd->rc.left;
	rc.top = hWnd->rc.top;
	rc.right = hWnd->rc.right - rc.left;
	rc.bottom = hWnd->rc.bottom - hWnd->rc.top;	
	rc.left = hWnd->rc.right;
	rc.right = 7;
	win = create_scrollbar_win(hWnd->screen,rc.left,rc.top,rc.right,rc.bottom);
	if(win == -1)
		return -1;
	if(hWnd->style & WS_VISIBLE)
		windows[win].style |= WS_VISIBLE;
	if(hWnd->status & WST_ENABLED)
		windows[win].status |= WST_ENABLED;
	p = (LPSCROLLBARDATA)windows[win].data;
	p->hWnd = hWnd;
	return win;
}
//---------------------------------------------------------------------------------
int set_scrollbar_info(int sb,unsigned int min,unsigned int max,unsigned int page)
{
	LPSCROLLBARDATA p;
	long size,l;
	HWND hWnd;
	
	if(windows[sb].pfn_proc == NULL || windows[sb].type != WC_SCROLLBAR)
		return -1;
	p = (LPSCROLLBARDATA)windows[sb].data;
	if(p == NULL)
		return -2;
	hWnd = &windows[sb];
	if(max == 0){
		p->min = 0;
		p->max = 0;
		p->page = 0;
		p->size_thumb = 0;		
	}
	else{
		if(min > max || page > (max - min))
			return -3;	
		p->min = min;
		p->max = max;
		p->page = page;
		size = (hWnd->rc.bottom - hWnd->rc.top) - 2 - 2;
		l = ((max - min + page) << 8) / page;	
		p->size_thumb = ((size << 8) / l);	
		if(p->size_thumb < 1)
			p->size_thumb = size;
	}
	hWnd->status |= 3;
	return 0;
}
//---------------------------------------------------------------------------------
int get_scrollbar_pos(int sb)
{
	LPSCROLLBARDATA p;
	
	if(windows[sb].pfn_proc == NULL || windows[sb].type != WC_SCROLLBAR)
		return 0;
	p = (LPSCROLLBARDATA)windows[sb].data;
	if(p == NULL)
		return 0;
	return p->pos;
}
//---------------------------------------------------------------------------------
int set_scrollbar_pos(int sb,unsigned int pos)
{
	LPSCROLLBARDATA p;
	
	if(windows[sb].pfn_proc == NULL || windows[sb].type != WC_SCROLLBAR)
		return -1;
	p = (LPSCROLLBARDATA)windows[sb].data;
	if(p == NULL)
		return -2;
	if(pos < p->min)
		pos = p->min;
	if(pos > p->max)
		pos = p->max;	
	p->pos = pos;
	windows[sb].status |= 3;
	return 0;
}
//---------------------------------------------------------------------------------
static int wndproc_scrollbar_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPSCROLLBARDATA p;
	
	switch(uMsg){
		case WM_NCCALCSIZE:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case WM_ERASEBKGND:
			{
				RECT rc;
				u16 border_color;
				
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;
				rc = hWnd->rc;
				if(hWnd->screen == 0)
					border_color = 5;
				else
					border_color = PA_RGB(0,0,0);
				rect(hWnd->screen,&rc,hWnd->bg_color,border_color);
				p = (LPSCROLLBARDATA)hWnd->data;
				if(p != NULL){
					long l,size;
					
					rc.left += 2;rc.right -= 2;
					size = (rc.bottom - rc.top) - 4;
					l = (p->max - p->min);
					if(l != 0){
						l = (size - p->size_thumb) * p->pos / l;
						rc.top += 2 + l;
						rc.bottom = rc.top + p->size_thumb;
					}
					fill_rect(hWnd->screen,&rc,border_color);
				}
				unlock_mutex(&hWnd->critical_section);
			}
		break;
		case WM_PAINT:
		break;
		case WM_MOUSEMOVE:
			p = (LPSCROLLBARDATA)hWnd->data;
			if(p != NULL){			
				int x,y,size;
				RECT rc;
				
				x = (int)LOWORD(lParam);
				y = (int)HIWORD(lParam);				
				get_client_rect(hWnd,&rc);				
				size = (rc.bottom - rc.top) - 4;				
				if(y > size)
					y = size;
				p->pos = y * p->max / size;
				SendMessage(get_window_index(p->hWnd),WM_VSCROLL,MAKEWPARAM(SB_THUMBPOSITION,p->pos),0);
				hWnd->status |= 3;
			}
		break;
		case WM_KEYDOWN:
			p = (LPSCROLLBARDATA)hWnd->data;
			if(p != NULL && !lParam && p->hWnd != NULL){
				switch(wParam){
					case VK_UP:
						if(p->pos >= p->page){
							p->pos -= p->page;
							SendMessage(get_window_index(p->hWnd),WM_VSCROLL,SB_LINEUP,0);
							hWnd->status |= 3;
						}
					break;
					case VK_DOWN:
						if(p->pos < p->max){
							p->pos += p->page;
							SendMessage(get_window_index(p->hWnd),WM_VSCROLL,SB_LINEDOWN,0);
							hWnd->status |= 3;
						}
					break;
				}
			}
		break;
	}
	return 0;
}
