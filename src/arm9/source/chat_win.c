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

#include "chat_win.h"
#include "scrollbar.h"
#include "fb.h"
#include "draws.h"
#include "listbox.h"
#include "all_gfx.h"
#include "contacts.h"

//---------------------------------------------------------------------------------
static int wndproc_chat_tab_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
//---------------------------------------------------------------------------------
int create_chat_tab_win(unsigned char screen,int x,int y,int width,int height)
{
	int win;	
	LPCHATTABDATA p;
	
	win = get_free_window();
	if(win == -1)
		return -1;
	windows[win].data = malloc(sizeof(LPCHATTABDATA)+500);		
	if(windows[win].data == NULL)
		return -1;
	memset(windows[win].data,0,sizeof(LPCHATTABDATA)+500);
	p = (LPCHATTABDATA)windows[win].data;
	p->hdr.vert_sb = p->hdr.horz_sb = -1;
	p->hdr.caption = (char *)(p+1);
	p->select_item = (unsigned int)-1;
	p->timer = -1;
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width;
	windows[win].rc.bottom = y + height;
	windows[win].status = 3|WST_ENABLED;	
	windows[win].screen = screen;
	windows[win].style = WS_BORDER|WS_VISIBLE;
	windows[win].type = WC_TABCONTROL;
	if(screen == 0)
		windows[win].bg_color = 2;
	else
		windows[win].bg_color = PA_RGB(7,11,19);
	append_win(win);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_chat_tab_win;	
	wndproc_chat_tab_win(&windows[win],WM_CREATE,0,0);	
	return win;
}
//---------------------------------------------------------------------------------
static int adjust_chat_tab_cursor(LPWINDOW hWnd)
{
	LPCHATTABDATA p;
	
	if(hWnd == NULL || hWnd->type != WC_TABCONTROL || (p = (LPCHATTABDATA)hWnd->data) == NULL)
		return -1;
	PA_EnableSpecialFx(hWnd->screen,SFX_ALPHA,0,SFX_BG0|SFX_BG1|SFX_BG2|SFX_BG3|SFX_BD);
	PA_SetSFXAlpha(hWnd->screen,5,16);	
	if(p->first_item != 0){
		PA_SetSpriteMode(hWnd->screen,64,0);//sx button
		PA_SetSpriteMode(hWnd->screen,66,0);//sx2 button
	}
	else{
		PA_SetSpriteMode(hWnd->screen,64,1);
		PA_SetSpriteMode(hWnd->screen,66,1);
	}
	if((p->first_item + p->draw_item) >= p->count_item){
		PA_SetSpriteMode(hWnd->screen,65,1);
		PA_SetSpriteMode(hWnd->screen,67,1);
	}
	else{
		PA_SetSpriteMode(hWnd->screen,65,0);
		PA_SetSpriteMode(hWnd->screen,67,0);
	}
	return 0;
}
//---------------------------------------------------------------------------------
static int wndproc_chat_tab_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPCHATTABDATA p;
	
	switch(uMsg){
		case WM_CREATE:
			{
				int y;

				y = hWnd->rc.top + (((hWnd->rc.bottom - hWnd->rc.top) - 12) >> 1);				
				PA_LoadSprite16cPal(hWnd->screen,10,(void*)sx_arrowPal);					
				PA_CreateSprite(hWnd->screen,64,(void*)sx_arrowTiles,OBJ_SIZE_8X16,0,10,0,y);								
				PA_CreateSprite(hWnd->screen,65,(void*)dx_arrowTiles,OBJ_SIZE_8X16,0,10,8,y);
				PA_CreateSprite(hWnd->screen,66,(void*)sx2_arrowTiles,OBJ_SIZE_8X16,0,10,256-16,y);
				PA_CreateSprite(hWnd->screen,67,(void*)dx2_arrowTiles,OBJ_SIZE_8X16,0,10,256-8,y);
				adjust_chat_tab_cursor(hWnd);
			}
		break;
		case WM_LBUTTONDOWN:
			p = (LPCHATTABDATA)hWnd->data;				
			if(PA_GetSpriteMode(hWnd->screen,64) == 0 && PA_SpriteTouched(64)){
				if(p->first_item > 0){
					p->first_item--;
					hWnd->status |= 3;
				}
				return 1;
			}
			else if(PA_GetSpriteMode(hWnd->screen,65) == 0 && PA_SpriteTouched(65)){
				p->first_item++;
				hWnd->status |= 3;
				return 1;
			}
			else if(PA_GetSpriteMode(hWnd->screen,66) == 0 && PA_SpriteTouched(66)){
				p->first_item++;
				hWnd->status |= 3;
				return 1;
			}
			else if(PA_GetSpriteMode(hWnd->screen,67) == 0 && PA_SpriteTouched(67)){
				p->first_item++;
				hWnd->status |= 3;
				return 1;
			}
			{
				char *c;
				unsigned int i,item;
				RECT rc,rc1;
				SIZE sz;
				int x,y,len;
				
				x = LOWORD(lParam);
				y = HIWORD(lParam);
				c = (char *)p->data;
				if(c == NULL)
					return 0;
				for(i = item = 0;item < p->first_item && i < p->size_used;item++){				
					while(*c != 0){
						i++;
						c++;
					}
					i += 8;
					c += 8;
				}				
				get_client_rect(hWnd,&rc);
				rc1 = rc;				
				while(i < p->size_used && rc1.left < rc.right){					
					len = strlen(c);
					get_text_extent(hWnd->screen,c,len,&sz);
					if(rc1.left + sz.cx > rc.right)
						break;					
					rc1.right = rc1.left + sz.cx;
					i += len + 8;
					c += len + 8;
					if(x >= rc1.left && x < rc1.right){
						i = (unsigned int)c;
						i = (i & ~3) - 4;
						i = *((unsigned int *)i);
						create_chat_win((LPFBCONTACT)i);
						break;
					}
					item++;
					rc1.left += sz.cx + 5;
				}			
			}
		break;
		case WM_ERASEBKGND:
			if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
				return 1;
			p = (LPCHATTABDATA)hWnd->data;
			fill_rect(hWnd->screen,&hWnd->rc,hWnd->bg_color);	
			unlock_mutex(&hWnd->critical_section);
		break;
		case TCM_GETCOUNT:
			p = (LPCHATTABDATA)hWnd->data;
			if(p == NULL)
				return -1;
			return (int)p->count_item;
		case TCM_RESETCONTENT:
			p = (LPCHATTABDATA)hWnd->data;
			if(p == NULL)
				return -1;
			if(p->data != NULL){
				free(p->data);
				p->data = NULL;
			}
			p->first_item = 0;
			p->size_data = 0;
			p->size_used = 0;
			p->count_item = 0;
			p->draw_item = 0;
			if(p->timer != -1){
				destroy_timer(p->timer);
				p->timer = -1;
			}
			hWnd->status |= 3;
		break;
		case TCM_ADDSTRING:
			{
				int len;
				unsigned int size;
				
				p = (LPCHATTABDATA)hWnd->data;				
				if(p == NULL || !lParam)
					return -1;
				len = strlen((char *)lParam);
				if(p->data == NULL || (p->size_data - p->size_used) < (len + 8)){					
					void *data;
					
					size = p->size_data + (((len >> 10) + 1) << 10);
					data = realloc(p->data,size);
					if(data == NULL)
						return -2;
					p->data = data;
					p->size_data = size;
				}				
				strcpy(&((char *)p->data)[p->size_used],(char *)lParam);
				p->size_used += len + 1 + 7;
				size = p->count_item;
				p->count_item++;	
				if(p->timer == -1)
					p->timer = set_timer(hWnd,1000,1,NULL);				
				hWnd->status |= 3;				
				return (int)&((char *)p->data)[(p->size_used & ~3) - 4];
			}		
		break;
		case WM_TIMER:
			p = (LPCHATTABDATA)hWnd->data;
			if(p != NULL && p->data != NULL){
				LPFBCONTACT pfbc;
				unsigned int item,i,data;
				char *c;
				int len;
				
				c = (char *)p->data;
				if(c == NULL)
					return 0;
				for(i = item = 0;item < p->first_item && i < p->size_used;item++){				
					while(*c != 0){
						i++;
						c++;
					}
					i += 8;
					c += 8;
				}
				item = 0;
				while(i < p->size_used && item < p->draw_item){
					len = strlen(c);
					i += len + 8;
					c += len + 8;
					data = (unsigned int)c;
					data = (data & ~3) - 4;
					pfbc = (LPFBCONTACT)*((unsigned int *)data);
					if(pfbc != NULL && (pfbc->status & 0xA0000000) == 0x80000000 && pfbc->win == -1)
						pfbc->status |= 0x20000000;
					else
						pfbc->status &= ~0x20000000;
					item++;					
				}
				if(item)
					hWnd->status |= 3;
			}
		break;
		case WM_PAINT:
			p = (LPCHATTABDATA)hWnd->data;
			if(p != NULL && p->data != NULL){
				LPFBCONTACT pfbc;
				char *c;
				int len;
				unsigned int item,i,data;
				RECT rc,rc1;
				u16 col,old_col,select_col;
				SIZE sz;
				
				p->draw_item = 0;
				c = (char *)p->data;
				if(c == NULL)
					return 0;
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;
				for(i = item = 0;item < p->first_item && i < p->size_used;item++){				
					while(*c != 0){
						i++;
						c++;
					}
					i += 8;
					c += 8;
				}				
				get_client_rect(hWnd,&rc);
				rc1 = rc;				
				if(hWnd->screen == 0){
					col = 4;
					select_col = 6;
				}
				else{
					col = PA_RGB(31,31,31);
					select_col = PA_RGB(31,0,0);
				}
				set_text_font(hWnd->screen,hWnd->font);
				old_col = set_text_color(hWnd->screen,col);
				while(i < p->size_used && rc1.left < rc.right){					
					len = strlen(c);
					get_text_extent(hWnd->screen,c,len,&sz);
					if(rc1.left + sz.cx > rc.right)
						break;					
					if(item == p->select_item)
						set_text_color(hWnd->screen,select_col);
					else
						set_text_color(hWnd->screen,col);
					rc1.right = rc1.left + sz.cx;
					data = (unsigned int)(c + len + 8);
					data = (data & ~3) - 4;
					pfbc = (LPFBCONTACT)*((unsigned int *)data);
					if(pfbc == NULL || pfbc->win != -1 || !(pfbc->status & 0x20000000))
						draw_text(hWnd->screen,c,len,&rc1,DT_VCENTER|DT_EDITCONTROL);
					i += len + 8;
					c += len + 8;
					item++;
					rc1.left += sz.cx + 5;
					p->draw_item++;
				}
				col = set_text_color(hWnd->screen,old_col);
				adjust_chat_tab_cursor(hWnd);
				unlock_mutex(&hWnd->critical_section);
			}
		break;
		case WM_DESTROY:
			p = (LPCHATTABDATA)hWnd->data;
			if(p != NULL){
				if(p->timer != -1){
					destroy_timer(p->timer);
					p->timer = -1;
				}
				if(p->data != NULL){
					free(p->data);
					p->size_data = 0;
				}
			}
		break;	
		case WM_NCCALCSIZE:
			if(!wndproc_winex_win(hWnd,uMsg,wParam,lParam)){
				((LPRECT)lParam)->left += 16;
				((LPRECT)lParam)->right -= 16;
				return 0;
			}
			return -1;	
		break;
	}
	return 0;
}