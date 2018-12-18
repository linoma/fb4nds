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

#include "listbox.h"
#include "scrollbar.h"
#include "draws.h"

static int wndproc_listbox_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
//---------------------------------------------------------------------------------
int create_listbox_win(unsigned char screen,int x,int y,int width,int height)
{
	int win;	
	LPLISTBOXDATA p;
	
	win = get_free_window();
	if(win == -1)
		return -1;
	memset(&windows[win],0,sizeof(WINDOW));
	windows[win].data = malloc(sizeof(LISTBOXDATA)+500);		
	if(windows[win].data == NULL)
		return -2;
	memset(windows[win].data,0,sizeof(LISTBOXDATA)+500);
	init_winex_header(&windows[win],sizeof(LISTBOXDATA));
	p = (LPLISTBOXDATA)windows[win].data;
	p->select_item = (unsigned int)-1;
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width;
	windows[win].rc.bottom = y + height;
	windows[win].status = 3|WST_ENABLED;	
	windows[win].screen = screen;
	windows[win].style = WS_VSCROLL|WS_BORDER;
	windows[win].type = WC_LISTBOX;
	if(screen == 0)
		windows[win].bg_color = 4;
	else
		windows[win].bg_color = PA_RGB(31,31,31);	
	append_win(win);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_listbox_win;	
	wndproc_listbox_win(&windows[win],WM_CREATE,0,0);	
	return win;
}
//---------------------------------------------------------------------------------
int set_listbox_ownerdraw(int win,LPOWNERDRAW pfn)
{
	LPLISTBOXDATA p;
	LPWINDOW hWnd;
	
	if(win < 0 || win >= MAX_WINDOWS)
		return -1;
	hWnd = &windows[win];
	if(hWnd->type != WC_LISTBOX || hWnd->data == NULL)
		return -2;
	p = (LPLISTBOXDATA)hWnd->data;
	p->hdr.pfn_ownerdraw = pfn;
	hWnd->status |= 3;
	return 0;
}
//---------------------------------------------------------------------------------
static unsigned int get_item_from_pos(LPWINDOW hWnd,int x,int y)
{
	LPLISTBOXDATA p;
	RECT rc,rc1;
	unsigned int i,it;
	int len;
	char *c;
	
	if(hWnd == NULL || (p = (LPLISTBOXDATA)hWnd->data) == NULL || (c = (char *)p->data) == NULL)
		return LB_ERR;
	get_client_rect(hWnd,&rc);
	if(x < rc.left || y < rc.top || x > rc.right || y > rc.bottom)
		return LB_ERR;
	rc1 = rc;
	rc1.bottom = rc1.top;
	for(i = it = 0;it < p->count_item && i < p->size_used;it++){				
		len = strlen(c);
		if(hWnd->style & LBS_OWNERDRAWVARIABLE){
			unsigned int adr,adr1;
						
			adr = (unsigned int)(c + len + 12);
			adr1 = ((adr - 4) & ~3) - 4;
			adr = (adr & ~3) - 4;
			if(*((int *)adr) == -1){
				MEASUREITEM mis;
							
				mis.hwndItem = hWnd;
				mis.itemID = it;
				mis.itemText = (char *)c;
				mis.itemData = adr1;
				mis.rcItem = rc1;
				mis.rcItem.bottom = rc1.top + p->height_item;
				*((int *)adr) = hWnd->pfn_proc((struct WINDOW *)hWnd,WM_MEASUREITEM,it,(LPARAM)&mis);
			}
			rc1.bottom += *((int *)adr);
		}
		else
			rc1.bottom += p->height_item;	
		i += len + 8;
		c += len + 8;
		if(hWnd->style & LBS_OWNERDRAWVARIABLE){
			i += 4;
			c += 4;
		}											
		if(y >= rc1.top && y < rc1.bottom){
			if(x >= rc1.left && x < rc1.right)
				return it;
		}
		rc1.top = rc1.bottom;			
	}	
	return LB_ERR;
}
//---------------------------------------------------------------------------------
static int get_item_rect(LPWINDOW hWnd,unsigned int item,LPRECT lprc,unsigned int flags)
{
	LPLISTBOXDATA p;
	unsigned int i,height,height_item;
	char *c;
	RECT rc;
	
	if(hWnd == NULL || lprc == NULL || (p = (LPLISTBOXDATA)hWnd->data) == NULL)
		return -1;
	if((c = (char *)p->data) == NULL)
		return -2;
	if(item >= p->count_item)
		return -3;
	get_client_rect(hWnd,lprc);
	height = 0;
	if(hWnd->style & LBS_OWNERDRAWVARIABLE){
		unsigned int adr,adr1,it,it_max,h;		
		int len;

		rc.top = rc.bottom = 0;
		h = 0;
		if(flags & 1){
			if(item > p->first_item)
				it_max = item;
			else
				it_max = p->first_item;
		}
		else
			it_max = item;
		for(i = it = 0;it <= it_max && i < p->size_used;it++){				
			len = strlen_UTF(c);
			i += len + 12;
			c += len + 12;					
			adr = (unsigned int)c;
			adr1 = ((adr - 4) & ~3) - 4;
			adr = (adr & ~3) - 4;
			if(*((int *)adr) == -1){
				MEASUREITEM mis;
							
				mis.hwndItem = hWnd;
				mis.itemID = it;
				mis.itemText = (char *)c;
				mis.itemData = adr1;
				mis.rcItem.bottom = height + p->height_item;
				*((int *)adr) = hWnd->pfn_proc((struct WINDOW *)hWnd,WM_MEASUREITEM,it,(LPARAM)&mis);
			}			
			if(it == p->first_item){
				rc.top = h;
				rc.bottom = h + *((int *)adr);			
			}
			h += *((int *)adr);
			if(it < item)
				height = h;			
			else if(it == item)
				height_item = *((int *)adr);
		}
		if(i > p->size_used && it != item)
			return -4;		
	}
	else{
		height_item = p->height_item;
		height = item * height_item;
		rc.top = p->first_item * height_item;
		rc.bottom = rc.top + height_item;		
	}	
	lprc->top = (long)height;
	if(flags & 1)
		lprc->top -= rc.top;	
	lprc->bottom = lprc->top + height_item;	
	return 0;
}
//---------------------------------------------------------------------------------
static int is_item_visible(LPWINDOW hWnd,unsigned int item,int partial)
{
	RECT rc,rc_client;
	int res;
	
	res = get_item_rect(hWnd,item,&rc,1);
	if(res)
		return 0;
	get_client_rect(hWnd,&rc_client);
	if(partial){
		if(rc.top > (rc_client.bottom-rc_client.top) || rc.top < 0)
			return 0;
		else if(rc.bottom > (rc_client.bottom-rc_client.top)){
			if(rc.top < (rc_client.bottom-rc_client.top))
				return 1;
			return 0;
		}
		return 1;
	}
	if(rc.top < 0){
		if(rc.bottom > 0)
			return 1;
		return 0;
	}
	else if(rc.bottom > (rc_client.bottom-rc_client.top)){
//		if(rc.top < (rc_client.bottom-rc_client.top))
//			return 1;		
		return 0;
	}
	return 1;
}
//---------------------------------------------------------------------------------
static int recalc_layout(LPWINDOW hWnd)
{
	LPLISTBOXDATA p;
	
	if(hWnd == NULL)
		return -1;
	p = (LPLISTBOXDATA)hWnd->data;
	if(p == NULL)
		return -2;
	if(p->hdr.vert_sb != -1){
		int height,i,height_item;		
		{
			RECT rc;
			
			get_client_rect(hWnd,&rc);
			i = rc.bottom - rc.top;
		}
		height_item = p->height_item;		
		if(hWnd->style & LBS_OWNERDRAWVARIABLE){
			char *c;
			unsigned int adr,adr1,it,mem_used;
			int len;
			
			c = (char *)p->data;
			if(c == NULL)
				return -3;
			height = 0;
			for(mem_used = it = 0;mem_used < p->size_used && it < p->count_item;it++){									
				len = strlen_UTF(c);
				adr = (unsigned int)(c + len + 12);
				adr1 = ((adr - 4) & ~3) - 4;
				adr = (adr & ~3) - 4;
				if(*((int *)adr) == -1){
					MEASUREITEM mis;
					RECT rc1;
					
					mis.hwndItem = hWnd;
					mis.itemID = it;
					mis.itemText = (char *)c;
					mis.itemData = adr1;
					rc1.top = height;
					mis.rcItem = rc1;
					mis.rcItem.bottom = rc1.top + p->height_item;
					*((int *)adr) = hWnd->pfn_proc((struct WINDOW *)hWnd,WM_MEASUREITEM,it,(LPARAM)&mis);
				}
				height += *((int *)adr);
				mem_used += len + 12;
				c += len + 12;
			}
			height -= i;			
		}
		else
			height = (p->count_item * height_item) - i;
		if(height > 0){		
			i = (height / height_item);
			i *= height_item;
			if(i < height)
				i += height_item;
		}
		else
			i = 0;		
		set_scrollbar_info(p->hdr.vert_sb,0,i,height_item);		
	}
	return 0;
}
//---------------------------------------------------------------------------------
static int wndproc_listbox_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPLISTBOXDATA p;
	
	switch(uMsg){
		case WM_LBUTTONDOWN:
			{
				unsigned int item;
				
				if(wParam == 0){
					int x,y;
					
					p = (LPLISTBOXDATA)hWnd->data;
					if(p == NULL)
						return LB_ERR;
					x = (int)(short)LOWORD(lParam);
					y = (int)(short)HIWORD(lParam);
					if(p->hdr.vert_sb != -1)
						y += get_scrollbar_pos(p->hdr.vert_sb);
					item = get_item_from_pos(hWnd,x,y);
				}
				else
					item = hWnd->pfn_proc((struct WINDOW *)hWnd,LB_GETCURSEL,0,0);
				if(item != LB_ERR){				
					hWnd->pfn_proc((struct WINDOW *)hWnd,LB_SETCURSEL,item,0);
					if(hWnd->childs.parent != NULL)
						SendMessage_HWND((HWND)hWnd->childs.parent,WM_COMMAND,MAKEWPARAM(hWnd->childs.id,LBN_SELCHANGE),(LPARAM)hWnd);												
				}
			}			
		break;
		case WM_LBUTTONDBLCLK:
			{
				unsigned int item;
								
				if(wParam == 0){
					int x,y;
					
					p = (LPLISTBOXDATA)hWnd->data;
					if(p == NULL)
						return LB_ERR;
					x = (int)(short)LOWORD(lParam);
					y = (int)(short)HIWORD(lParam);
					if(p->hdr.vert_sb != -1)
						y += get_scrollbar_pos(p->hdr.vert_sb);
					item = get_item_from_pos(hWnd,x,y);
				}
				else
					item = hWnd->pfn_proc((struct WINDOW *)hWnd,LB_GETCURSEL,0,0);
				if(item != LB_ERR){				
					hWnd->pfn_proc((struct WINDOW *)hWnd,LB_SETCURSEL,item,0);
					if(hWnd->childs.parent != NULL)
						SendMessage_HWND((HWND)hWnd->childs.parent,WM_COMMAND,MAKEWPARAM(hWnd->childs.id,LBN_DBLCLK),(LPARAM)hWnd);												
				}
			}						
		break;
		case WM_RBUTTONDOWN:
		break;
		case LB_GETITEMRECT:
			return get_item_rect(hWnd,wParam,(LPRECT)lParam,1);
		case LB_GETITEM:
			{
				char *c;
				unsigned int i,item;
				int len;
				
				p = (LPLISTBOXDATA)hWnd->data;
				if(p == NULL || wParam >= p->count_item || (c = (char *)p->data) == NULL)
					return -1;		
				if(!wParam)
					return (int)c;
				for(i = 0,item = 0;item < wParam && i < p->size_used;item++){				
					len = strlen_UTF(c);
					i += len + 8;
					c += len + 8;					
					if(hWnd->style & LBS_OWNERDRAWVARIABLE){
						i += 4;
						c += 4;
					}						
				}
				return (int)c;
			}
		break;
		case WM_VSCROLL:
			p = (LPLISTBOXDATA)hWnd->data;
			if(p == NULL)
				return -1;
			switch(LOWORD(wParam)){
				case SB_LINEUP:
					if(p->first_item > 0){
						p->first_item--;					
						hWnd->status |= 3;
					}
				break;
				case SB_LINEDOWN:					
					if(p->first_item < p->count_item-1){
						p->first_item++;					
						hWnd->status |= 3;
					}
				break;
				case SB_THUMBPOSITION:
				break;
			}
			if((hWnd->style & LBS_OWNERDRAWVARIABLE) && p->hdr.vert_sb != -1 && (hWnd->status & 3) == 3){
				RECT rc;
				
				if(!get_item_rect(hWnd,p->first_item,&rc,0))
					set_scrollbar_pos(p->hdr.vert_sb,rc.top);
			}
		break;
		case LB_SETITEMHEIGHT:
			p = (LPLISTBOXDATA)hWnd->data;
			if(p == NULL)
				return -1;
			p->height_item = lParam;
			recalc_layout(hWnd);
			hWnd->status |= 3;
		break;
		case LB_GETITEMHEIGHT:
			p = (LPLISTBOXDATA)hWnd->data;
			if(p == NULL)
				return -1;
			return p->height_item;
		case LB_RESETCONTENT:
			p = (LPLISTBOXDATA)hWnd->data;
			if(p == NULL)
				return -1;
			if(p->data != NULL){
				free(p->data);
				p->data = NULL;
			}			
			p->size_data = 0;
			p->size_used = 0;
			p->count_item = 0;
			p->select_item = -1;
			p->first_item = 0;
			
			if(p->hdr.vert_sb != -1){
				set_scrollbar_info(p->hdr.vert_sb,0,0,0);		
				set_scrollbar_pos(p->hdr.vert_sb,0);
			}
			hWnd->status |= 3;
		break;
		case LB_GETCOUNT:
			p = (LPLISTBOXDATA)hWnd->data;
			if(p == NULL)
				return -1;
			return p->count_item;
		case LB_ADDSTRING:
			{
				int len;
				unsigned int size;
				
				p = (LPLISTBOXDATA)hWnd->data;				
				if(p == NULL || !lParam)
					return -1;
				len = strlen_UTF((char *)lParam);
				size = len + 8;
				if(hWnd->style & LBS_OWNERDRAWVARIABLE)
					size += 4;
				if(p->data == NULL || (p->size_data - p->size_used) < size){					
					void *data;
					
					size = p->size_data + (((size >> 10) + 1) << 10);
					data = realloc(p->data,size);
					if(data == NULL)
						return -2;
					p->data = data;
					p->size_data = size;
				}				
				memcpy(&((char *)p->data)[p->size_used],(void *)lParam,len+1);
				p->size_used += len + 8;
				size = p->count_item;
				p->count_item++;	
				len = (int)&((char *)p->data)[(p->size_used & ~3) - 4];
				if(hWnd->style & LBS_OWNERDRAWVARIABLE){
					((int *)len)[1] = -1;
					p->size_used += 4;					
				}
				recalc_layout(hWnd);
				hWnd->status |= 3;				
				return len;
			}
		break;
		case WM_MEASUREITEM:
			p = (LPLISTBOXDATA)hWnd->data;				
			if(p == NULL)
				return -1;
			return p->height_item;
		case LB_SETCURSEL:
			p = (LPLISTBOXDATA)hWnd->data;				
			if(p == NULL)
				return -1;
			if(wParam < p->count_item){	
				p->select_item = wParam;
				while(!is_item_visible(hWnd,p->select_item,FALSE)){
					SendMessage(p->hdr.vert_sb,WM_KEYDOWN,VK_DOWN,0);
				}
				hWnd->status |= 2;
				return (int)wParam;
			}
			return -1;
		case LB_GETCURSEL:
			p = (LPLISTBOXDATA)hWnd->data;				
			if(p == NULL)
				return -1;
			return p->select_item;
		case WM_DESTROY:
			p = (LPLISTBOXDATA)hWnd->data;
			if(p != NULL && p->data != NULL){
				free(p->data);				
				p->size_data = 0;
			}
			if(p->hdr.vert_sb != -1){
				destroy_window(p->hdr.vert_sb);
				p->hdr.vert_sb = -1;
			}
		break;
		case WM_CREATE:
			{
				SIZE sz;
				
				p = (LPLISTBOXDATA)hWnd->data;
				if(p != NULL){
					set_text_font(hWnd->screen,hWnd->font);
					get_text_extent(hWnd->screen,"ABCDy",5,&sz);
					p->height_item = sz.cy+1;
				}
				return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
			}
		case WM_ERASEBKGND:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case WM_SETTEXT:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case WM_KEYDOWN:
			p = (LPLISTBOXDATA)hWnd->data;
			if(p != NULL && !lParam){
				switch(wParam){
					case VK_UP:
						if(p->select_item > 0){
							p->select_item--;							
							if(!is_item_visible(hWnd,p->select_item,FALSE))
								SendMessage(p->hdr.vert_sb,WM_KEYDOWN,VK_UP,0);
							hWnd->status |= 2;
							if(hWnd->childs.parent != NULL)
								SendMessage(get_window_index((HWND)hWnd->childs.parent),WM_COMMAND,MAKEWPARAM(hWnd->childs.id,LBN_SELCHANGE),(LPARAM)hWnd);							
						}
					break;
					case VK_DOWN:						
						p->select_item++;
						if(p->select_item >= p->count_item)
							p->select_item--;							
						else{
							if(!is_item_visible(hWnd,p->select_item,FALSE)){
								SendMessage(p->hdr.vert_sb,WM_KEYDOWN,VK_DOWN,0);
							}
							hWnd->status |= 2;
							if(hWnd->childs.parent != NULL)
								SendMessage(get_window_index((HWND)hWnd->childs.parent),WM_COMMAND,MAKEWPARAM(hWnd->childs.id,LBN_SELCHANGE),(LPARAM)hWnd);														
						}
					break;
					case VK_RETURN:
					case VK_SHIFT:
						{
							RECT rc,rc_client;
														
							if(!lParam && !get_item_rect(hWnd,p->select_item,&rc,1)){							
								get_client_rect(hWnd,&rc_client);
								hWnd->pfn_proc((struct WINDOW *)hWnd,wParam == VK_RETURN ? WM_LBUTTONDOWN : WM_RBUTTONDOWN,0,MAKELPARAM(rc.left,rc.top+rc_client.top));
							}
						}
					break;
				}
				hWnd->status |= 2;				
			}
		break;
		case WM_PAINT:
			p = (LPLISTBOXDATA)hWnd->data;
			if(p != NULL && p->data != NULL){
				char *c;
				int len;
				unsigned int item,i;
				RECT rc,rc1;
				u16 col,old_col,select_col;
								
				c = (char *)p->data;
				if(c == NULL)
					return 0;
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;
				for(i = item = 0;item < p->first_item && i < p->size_used;item++){				
					len = strlen_UTF(c);
					i += len + 8;
					c += len + 8;
					if(hWnd->style & LBS_OWNERDRAWVARIABLE){
						i += 4;
						c += 4;
					}											
				}				
				get_client_rect(hWnd,&rc);
				rc1 = rc;				
				rc1.bottom = rc1.top + p->height_item;
				if(hWnd->screen == 0){
					col = 5;
					select_col = 6;
				}
				else{
					col = PA_RGB(0,0,0);														
					select_col = PA_RGB(31,0,0);
				}
				set_text_font(hWnd->screen,hWnd->font);
				old_col = set_text_color(hWnd->screen,col);
				while(i < p->size_used && rc1.top < rc.bottom){					
					len = strlen_UTF(c);
					rc1.bottom = rc1.top;
					if(hWnd->style & LBS_OWNERDRAWVARIABLE){
						unsigned int adr,adr1;
						
						adr = (unsigned int)(c + len + 12);
						adr1 = ((adr - 4) & ~3) - 4;
						adr = (adr & ~3) - 4;
						if(*((int *)adr) == -1){
							MEASUREITEM mis;
							
							mis.hwndItem = hWnd;
							mis.itemID = item;
							mis.itemText = (char *)c;
							mis.itemData = adr1;
							mis.rcItem = rc1;
							mis.rcItem.bottom = rc1.top + p->height_item;
							*((int *)adr) = hWnd->pfn_proc((struct WINDOW *)hWnd,WM_MEASUREITEM,item,(LPARAM)&mis);
						}
						rc1.bottom += *((int *)adr);
					}
					else
						rc1.bottom += p->height_item;
					if(rc1.bottom > rc.bottom)
						rc1.bottom = rc.bottom;
					if(p->hdr.pfn_ownerdraw){
						DRAWITEMSTRUCT dis;
						
						dis.hwndItem = hWnd;
						dis.rcItem = rc1;
						dis.itemID = item;
						dis.itemState = item == p->select_item ? ODS_SELECTED : 0;
						dis.itemText = (char *)c;
						dis.itemData = (unsigned int)c;
						dis.itemData = ((dis.itemData + len + 8) & ~3) - 4;	
						dis.itemData = (int)*((u32 *)dis.itemData);						
						p->hdr.pfn_ownerdraw(&dis);
					}
					else{
						if(item == p->select_item)
							set_text_color(hWnd->screen,select_col);
						else
							set_text_color(hWnd->screen,col);						
						draw_text(hWnd->screen,c,len,&rc1,DT_VCENTER|DT_SINGLELINE);
					}
					i += len + 8;
					c += len + 8;
					if(hWnd->style & LBS_OWNERDRAWVARIABLE){
						i += 4;
						c += 4;
					}											
					rc1.top = rc1.bottom;
					item++;
				}
				col = set_text_color(hWnd->screen,old_col);
				unlock_mutex(&hWnd->critical_section);
			}
		break;
		case WM_KILLFOCUS:
			if(hWnd->childs.parent != NULL){
				if(((HWND)hWnd->childs.parent)->type == WC_COMBOBOX){
					hWnd->style &= ~WS_VISIBLE;
					invalidate_windows_screen(hWnd->screen,1);
				}
			}
		default:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}