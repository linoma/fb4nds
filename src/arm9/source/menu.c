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

#include "menu.h"
#include "draws.h"

extern int (*pfn_MainFunc)(void);
extern int (*pfn_VBLFunc)(void);
static int wndproc_menubar_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
static int win_menu = -1;
static int (*old_pfn_MainFunc)(void);
static int (*old_pfn_VBLFunc)(void);
static int def_menu_loop(void);
static int def_menu_vbl_loop(void);
//---------------------------------------------------------------------------------
int create_menubar_win(unsigned char screen,int x,int y,int width,int height,int parent)
{
	int win;
	LPPOPUPMENU p;
	
	win = get_free_window();
	if(win == -1)
		return -1;
	memset(&windows[win],0,sizeof(WINDOW));
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width;
	windows[win].rc.bottom = y + height;
	windows[win].status = 3|WST_ENABLED;	
	windows[win].data = malloc(sizeof(POPUPMENU)+10*sizeof(MENUITEM) + 10 * 100);
	windows[win].screen = screen;
	windows[win].type = WC_MENU;
	windows[win].style = WS_VISIBLE;
	if(screen == 0)
		windows[win].bg_color = 2;
	else
		windows[win].bg_color = PA_RGB(7,11,19);
	if(windows[win].data == NULL)
		return -1;
	memset(windows[win].data,0,sizeof(POPUPMENU)+10*sizeof(MENUITEM) + 10 * 100);
	p = (LPPOPUPMENU)windows[win].data;
	p->data = (void *)(p+1);
	p->h_parent = parent;
	p->size_data = 10 * sizeof(MENUITEM) + 10 * 100;
	p->flags = PMT_BAR;
	p->selected_item = -1;
	append_win(win);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_menubar_win;	
	wndproc_menubar_win(&windows[win],WM_CREATE,0,0);
	return win;
}
//---------------------------------------------------------------------------------
int show_popupmenu_win(int win,int x,int y,int parent)
{
	LPPOPUPMENU p;
	LPMENUITEM item;
	int width,height,i,font;
	unsigned int c;
	SIZE sz;
	
	if(win < 0 || win >= MAX_WINDOWS || windows[win].data == NULL || windows[win].type != WC_MENU)
		return -1;
	p = (LPPOPUPMENU)windows[win].data;
	if(p->count_item < 1)
		return -2;
	select_menu_item(&windows[win],0);
	item = (LPMENUITEM)p->data;
	width = height = 0;
	font = set_text_font(windows[win].screen,windows[win].font);
	for(i=0;i<p->count_item;i++){						
		get_text_extent(windows[win].screen,(char *)item->data,strlen((char *)item->data),&sz);
		if(sz.cx > width)
			width = sz.cx;
		height += sz.cy + 5;
		c = (unsigned int)(item + 1);
		c += item->size_data + 3;
		item = (LPMENUITEM)(c & ~3);
	}
	set_text_font(windows[win].screen,font);
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width + 6;
	windows[win].rc.bottom = y + height + 4;
	windows[win].style |= WS_VISIBLE|WS_BORDER;
	append_win(win);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_menubar_win;
	wndproc_menubar_win(&windows[win],WM_CREATE,0,0);
	windows[win].status |= 3|WST_ENABLED;	
	
	old_pfn_MainFunc = pfn_MainFunc;
	old_pfn_VBLFunc = pfn_VBLFunc;
	win_menu = win;
	pfn_MainFunc = def_menu_loop;
	pfn_VBLFunc = def_menu_vbl_loop;
	
	return 0;
}
//---------------------------------------------------------------------------------
static void exit_menu_loop(void)
{
	u8 screen;
	
	screen = windows[win_menu].screen;
	windows[win_menu].pfn_proc = NULL;
	pfn_MainFunc = old_pfn_MainFunc;
	pfn_VBLFunc = old_pfn_VBLFunc;			
	destroy_menu(win_menu);
	win_menu = -1;			
	redraw_windows_screen(screen);
}
//---------------------------------------------------------------------------------
void destroy_menu(int win)
{
	if(win < 0 || win >= MAX_WINDOWS)
		return;
	if(windows[win].type != WC_MENU)
		return;
	destroy_window(win);	
}
//---------------------------------------------------------------------------------
int create_popupmenu_win(unsigned char screen,int parent)
{
	LPPOPUPMENU p;
	int win;
	
	for(win = MAX_WINDOWS - 1;win >= 0;win--){
		if(windows[win].data == NULL)
			break;
	}
	if(win == -1)
		return -1;
	memset(&windows[win],0,sizeof(WINDOW));
	windows[win].data = malloc(sizeof(POPUPMENU)+10*sizeof(MENUITEM) + 10 * 100);
	if(windows[win].data == NULL)
		return -2;
	windows[win].screen = screen;
	windows[win].type = WC_MENU;
	if(screen == 0)
		windows[win].bg_color = 2;
	else
		windows[win].bg_color = PA_RGB(7,11,19);
	memset(windows[win].data,0,sizeof(POPUPMENU)+10*sizeof(MENUITEM) + 10 * 100);
	p = (LPPOPUPMENU)windows[win].data;
	p->data = (void *)(p+1);
	p->h_parent = parent;
	p->size_data = 10 * sizeof(MENUITEM) + 10 * 100;
	p->flags = PMT_NORMAL;
	p->selected_item = -1;	
	return win;
}
//---------------------------------------------------------------------------------
int enable_menu_item(HWND hWnd,unsigned int index,unsigned int flags)
{
	LPPOPUPMENU p;
	LPMENUITEM item;
	unsigned int c;
	int i;
	
	if(hWnd == NULL || hWnd->type != WC_MENU || hWnd->data == NULL)
		return -1;
	p = (LPPOPUPMENU)hWnd->data;
	item = (LPMENUITEM)p->data;
	for(i=0;i<p->count_item;i++){						
		if(item != NULL){
			if(i == index){
				if(flags & MF_DISABLED)
					item->flags |= MF_DISABLED;
				else
					item->flags &= ~MF_DISABLED;
				break;
			}
		}
		c = (unsigned int)(item + 1);
		c += item->size_data;
		item = (LPMENUITEM)((c + 3) & ~3);		
	}
	hWnd->status |= 3;
	return 0;
}
//---------------------------------------------------------------------------------
int select_menu_item(HWND hWnd,unsigned int index)
{
	LPPOPUPMENU p;
	LPMENUITEM item;
	unsigned int c;
	int i;
	
	if(hWnd == NULL || hWnd->type != WC_MENU || hWnd->data == NULL)
		return -1;
	p = (LPPOPUPMENU)hWnd->data;
	item = (LPMENUITEM)p->data;
	for(i=0;i<p->count_item;i++){						
		if(item != NULL){
			if(i == index)
				item->flags |= MF_SELECTED;
			else
				item->flags &= ~MF_SELECTED;
		}
		c = (unsigned int)(item + 1);
		c += item->size_data;
		item = (LPMENUITEM)((c + 3) & ~3);		
	}
	p->selected_item = index;
	hWnd->status |= 3;
	return 0;
}
//---------------------------------------------------------------------------------
int insert_menu_item(HWND hWnd,unsigned int id,void *data,unsigned int flags)
{
	LPPOPUPMENU p;
	LPMENUITEM item;
	int len;
	unsigned int c;
	
	if(hWnd == NULL || hWnd->type != WC_MENU)
		return -1;
	if((p = (LPPOPUPMENU)hWnd->data) == NULL)
		return -2;
	item = NULL;
	switch(flags & 0xFF){
		default:
			if(data == NULL || ((char *)data)[0] == 0)
				return -2;
			len = strlen(data);
			if((p->size_data - p->size_used) < (sizeof(MENUITEM) + len + 1)){
				int size;
				
				size = p->size_data + 10 * sizeof(MENUITEM) + 10 * 100;
				p->data = realloc(p->data,size);
				if(p->data == NULL)
					return -4;
				p->size_data = size;
			}				
			c = (unsigned int)&(((char *)p->data)[p->size_used]);
			item = (LPMENUITEM)((c + 3) & ~3);			
		break;
	}
	if(item == NULL)
		return -3;
	item->size_data = len + 1;
	p->size_used = ((unsigned int)(item + 1) - ((unsigned int)p->data)) + item->size_data;
	p->count_item++;
	item->data = (void *)(item + 1);
	item->flags = flags & ~0xFF;
	item->id = id;
	strcpy((char *)item->data,(char *)data);
	return 0;
}
//---------------------------------------------------------------------------------
static int draw_item(u8 screen,LPMENUITEM item,LPRECT lprc,unsigned int flags)
{	
	if(item == NULL || lprc == NULL)
		return -1;
	switch(item->flags & 0xFF){		
		default:
			{
				SIZE sz;
				u16 old_col,col;
				
				get_text_extent(screen,(char *)item->data,strlen((char *)item->data),&sz);
				if(!(item->flags & MF_DISABLED)){
					if(item->flags & MF_SELECTED){				
						if(screen == 0){
							old_col = 4;
							col = 2;
						}
						else{
							old_col = PA_RGB(31,31,31);
							col = PA_RGB(7,11,19);
						}
						if(flags & PMT_BAR){
							lprc->right = lprc->left + sz.cx;
							lprc->bottom = lprc->top + sz.cy + 5;										
						}
						fill_rect(screen,lprc,old_col);					
					}
					else{
						if(screen == 0)
							col = 4;
						else
							col = PA_RGB(31,31,31);
					}
				}
				else{
					if(screen == 0)
						col = 3;
					else
						col = PA_RGB(24,24,24);
				}
				old_col = set_text_color(screen,col);
				draw_text(screen,(char *)item->data,-1,lprc,DT_VCENTER);
				lprc->right = sz.cx;
				lprc->bottom = sz.cy;				
				set_text_color(screen,old_col);
			}
		break;
	}
	return 0;
}
//---------------------------------------------------------------------------------
LPMENUITEM get_menuitem_from_pos(LPWINDOW hWnd,int x, int y)
{
	LPPOPUPMENU p;
	LPMENUITEM item;
	int i,font;
	unsigned int c;
	RECT rc;
	SIZE sz;
	
	if(hWnd == NULL || (p = (LPPOPUPMENU)hWnd->data) == NULL)
		return NULL;	
	rc = hWnd->rc;
	rc.left +=2;
	rc.right -= 2;
	rc.top +=2;
	rc.bottom -= 2;
	if(x < rc.left || x > rc.right || y < rc.top || y > rc.bottom)
		return NULL;
	item = (LPMENUITEM)p->data;	
	for(i=0;i<p->count_item;i++){		
		font = set_text_font(hWnd->screen,hWnd->font);
		get_text_extent(hWnd->screen,(char *)item->data,strlen((char *)item->data),&sz);
		set_text_font(hWnd->screen,font);
		if(p->flags & PMT_BAR)
			rc.right = rc.left + sz.cx + 5;
		else
			rc.bottom = rc.top + sz.cy + 5;
			
		if(x >= rc.left && x < rc.right && y >= rc.top && y < rc.bottom)
			return item;
			
		if(p->flags & PMT_BAR)
			rc.left = rc.right;
		else
			rc.top = rc.bottom;
			
		c = (unsigned int)(item + 1);
		c += item->size_data + 3;
		item = (LPMENUITEM)(c & ~3);
	}
	return NULL;
}
//---------------------------------------------------------------------------------
LPMENUITEM get_menuitem_from_item(LPWINDOW hWnd,int pos,LPRECT lprc)
{
	LPPOPUPMENU p;
	LPMENUITEM item;
	RECT rc;
	SIZE sz;
	int i,font;
	unsigned int c;
	
	if(hWnd == NULL || lprc == NULL  || (p = (LPPOPUPMENU)hWnd->data) == NULL)
		return NULL;
	rc = hWnd->rc;
	rc.left +=2;
	rc.right -= 2;
	rc.top +=2;
	rc.bottom -= 2;	
	item = (LPMENUITEM)p->data;
	for(i=0;i<p->count_item && i <= pos;i++){
		font = set_text_font(hWnd->screen,hWnd->font);
		get_text_extent(hWnd->screen,(char *)item->data,strlen((char *)item->data),&sz);
		set_text_font(hWnd->screen,font);
		if(p->flags & PMT_BAR)
			rc.right = rc.left + sz.cx + 5;
		else
			rc.bottom = rc.top + sz.cy + 5;
		if(i == pos){
			if(lprc != NULL)
				memcpy(lprc,&rc,sizeof(RECT));
			return item;
		}
		if(p->flags & PMT_BAR)
			rc.left = rc.right;
		else
			rc.top = rc.bottom;
		c = (unsigned int)(item + 1);
		c += item->size_data + 3;
		item = (LPMENUITEM)(c & ~3);	
	}
	return NULL;
}
//---------------------------------------------------------------------------------
int get_menuitem_rect(LPWINDOW hWnd,unsigned int id,LPRECT lprc)
{
	LPPOPUPMENU p;
	LPMENUITEM item;
	RECT rc;
	SIZE sz;
	int i,font;
	unsigned int c;
	
	if(hWnd == NULL || lprc == NULL  || (p = (LPPOPUPMENU)hWnd->data) == NULL)
		return -1;
	rc = hWnd->rc;
	rc.left +=2;
	rc.right -= 2;
	rc.top +=2;
	rc.bottom -= 2;	
	item = (LPMENUITEM)p->data;
	for(i=0;i<p->count_item;i++){
		font = set_text_font(hWnd->screen,hWnd->font);
		get_text_extent(hWnd->screen,(char *)item->data,strlen((char *)item->data),&sz);
		set_text_font(hWnd->screen,font);
		if(p->flags & PMT_BAR)
			rc.right = rc.left + sz.cx + 5;
		else
			rc.bottom = rc.top + sz.cy + 5;
		if(item->id == id){
			lprc->left = rc.left;
			lprc->top = rc.top;
			lprc->right = rc.right;
			lprc->bottom = rc.bottom;
			return 0;
		}						
		if(p->flags & PMT_BAR)
			rc.left = rc.right;
		else
			rc.top = rc.bottom;
		c = (unsigned int)(item + 1);
		c += item->size_data + 3;
		item = (LPMENUITEM)(c & ~3);	
	}
	return -2;
}
//---------------------------------------------------------------------------------
static int wndproc_menubar_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPPOPUPMENU p;
	LPMENUITEM item;
	
	switch(uMsg){
		case WM_KEYDOWN:
			if(!lParam){
				int index;
				
				p = (LPPOPUPMENU)hWnd->data;
				index = p->selected_item;
				switch(wParam){
					case VK_DOWN:
						if((p->flags & PMT_BAR) == 0)
							index++;
					break;
					case VK_UP:
						if((p->flags & PMT_BAR) == 0)
							index--;
					break;
					case VK_RIGHT:
						if((p->flags & PMT_BAR) != 0)
							index++;
					break;
					case VK_LEFT:
						if((p->flags & PMT_BAR) != 0)
							index--;
					break;
					case VK_RETURN:
						if((p->flags & PMT_BAR) != 0){
							RECT rc;
							
							item = get_menuitem_from_item(hWnd,index,&rc);
							if(item != NULL)
								SendMessage_HWND(hWnd,WM_LBUTTONDOWN,0,MAKELPARAM(rc.left+5,rc.top+5));
						}
						return 0;
					default:
						return 0;
				}
				if(index < 0)
					index = p->count_item - 1;
				else if(index >= p->count_item)
					index = 0;
				select_menu_item(hWnd,index);
			}
		break;
		case WM_LBUTTONDOWN:
			item = get_menuitem_from_pos(hWnd,LOWORD(lParam),HIWORD(lParam));			
			if(item != NULL && !(item->flags & MF_DISABLED)){			
				p = (LPPOPUPMENU)hWnd->data;
				SendMessage(p->h_parent,WM_COMMAND,(WPARAM)item->id,(LPARAM)hWnd);
			}
		break;
		case WM_PAINT:
			{
				int i,font;
				RECT rc;
				unsigned int c;
				
				p = (LPPOPUPMENU)hWnd->data;
				item = (LPMENUITEM)p->data;
				rc = hWnd->rc;
				rc.left+=2;rc.right -= 2;rc.top+=2;rc.bottom -= 2;
				font = set_text_font(hWnd->screen,hWnd->font);
				switch(p->flags & 0xFF){
					case PMT_BAR:
						for(i=0;i<p->count_item;i++){						
							draw_item(hWnd->screen,item,&rc,PMT_BAR);
							rc.left += rc.right + 5;
							rc.right = hWnd->rc.right - 2;
							rc.bottom = hWnd->rc.bottom;
							c = (unsigned int)(item + 1);
							c += item->size_data;
							item = (LPMENUITEM)((c + 3) & ~3);
						}
					break;
					default:
						{
							SIZE sz;
							
							get_text_extent(hWnd->screen,"Ay",2,&sz);
							rc.bottom = rc.top + sz.cy + 5;
							for(i=0;i<p->count_item;i++){						
								draw_item(hWnd->screen,item,&rc,PMT_NORMAL);
								rc.top += rc.bottom + 5;
								rc.bottom = rc.top + rc.bottom + 5;
								rc.right = hWnd->rc.right - 2;
								c = (unsigned int)(item + 1);
								c += item->size_data;
								item = (LPMENUITEM)((c + 3) & ~3);
							}
						}
					break;
				}
				set_text_font(hWnd->screen,font);
			}
		break;
		case WM_ERASEBKGND:
			{		
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;
				p = (LPPOPUPMENU)hWnd->data;
				if(p->flags & PMT_BAR)
					fill_rect(hWnd->screen,&hWnd->rc,hWnd->bg_color);
				else{
					u16 border_color;
					
					if(hWnd->screen == 0)
						border_color = 5;
					else
						border_color = PA_RGB(0,0,0);
					rect(hWnd->screen,&hWnd->rc,hWnd->bg_color,border_color);																
				}
				unlock_mutex(&hWnd->critical_section);
			}
		break;
		case WM_SETFOCUS:
			{
				int win;
				
				win = get_window_index(hWnd);				
				active_focus = win;			
				if(win != -1)
					hWnd = &windows[win];					
				hWnd->status |= WST_HASFOCUS|3;
				return active_focus;						
			}
	}
	return 0;
}
//---------------------------------------------------------------------------------
static int def_menu_vbl_loop(void)
{
	if(Stylus.Newpress){
		int win,id;
		
		win = get_window_from_pos(windows[win_menu].screen,Stylus.X,Stylus.Y);		
		
		id = -1;
		if(win == win_menu){
			LPMENUITEM item;
			LPPOPUPMENU p;

			item = get_menuitem_from_pos(&windows[win_menu],Stylus.X,Stylus.Y);
			if(item != NULL && !(item->flags & MF_DISABLED)){
				p = (LPPOPUPMENU)windows[win_menu].data;
				win = p->h_parent;
				id = item->id;
			}
		}
		exit_menu_loop();		
		if(id != -1)
			SendMessage(win,WM_COMMAND,(WPARAM)id,0);				
	}			
	return 1;
}
//---------------------------------------------------------------------------------
static int def_menu_loop(void)
{
	if(Pad.Newpress.Up)
		SendMessage(win_menu,WM_KEYDOWN,VK_UP,0);
	else if(Pad.Held.Up)
		SendMessage(win_menu,WM_KEYDOWN,VK_UP,1);
	else if(Pad.Released.Up)
		SendMessage(win_menu,WM_KEYUP,VK_UP,0);

	if(Pad.Newpress.Left)
		SendMessage(win_menu,WM_KEYDOWN,VK_LEFT,0);
	else if(Pad.Held.Left)
		SendMessage(win_menu,WM_KEYDOWN,VK_LEFT,1);
	else if(Pad.Released.Left)
		SendMessage(win_menu,WM_KEYUP,VK_LEFT,0);
		
	if(Pad.Newpress.Right)
		SendMessage(win_menu,WM_KEYDOWN,VK_RIGHT,0);
	else if(Pad.Held.Right)
		SendMessage(win_menu,WM_KEYDOWN,VK_RIGHT,1);
	else if(Pad.Released.Right)
		SendMessage(win_menu,WM_KEYUP,VK_RIGHT,0);
	
	if(Pad.Newpress.Down)
		SendMessage(win_menu,WM_KEYDOWN,VK_DOWN,0);
	else if(Pad.Held.Down)
		SendMessage(win_menu,WM_KEYDOWN,VK_DOWN,1);
	else if(Pad.Released.Down)
		SendMessage(win_menu,WM_KEYUP,VK_DOWN,0);
	
	if(Pad.Newpress.L)
		exit_menu_loop();
	return 1;	
}