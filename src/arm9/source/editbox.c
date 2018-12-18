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

#include "editbox.h"
#include "scrollbar.h"
#include "fb.h"
#include "draws.h"
#include "fonts.h"

extern u32 ticks;
static CARETDATA caret_data={0,0,NULL,-1,{-1,-1}};
static int wndproc_editbox_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
//---------------------------------------------------------------------------------
int create_editbox_win(unsigned char screen,int x,int y,int width,int height)
{
	int win;
	LPEDITBOXDATA p;
	
	win = get_free_window();
	if(win == -1)
		return -1;
	memset(&windows[win],0,sizeof(WINDOW));
	windows[win].rc.left = x;
	windows[win].rc.top = y;
	windows[win].rc.right = x + width;
	windows[win].rc.bottom = y + height;
	windows[win].status |= 3|WST_ENABLED;	
	windows[win].data = malloc(sizeof(EDITBOXDATA)+2500);
	windows[win].screen = screen;
	windows[win].type = WC_EDIT;
	windows[win].style = WS_VISIBLE|WS_BORDER;
	if(screen == 0)
		windows[win].bg_color = 4;
	else
		windows[win].bg_color = PA_RGB(31,31,31);
	if(windows[win].data == NULL)
		return -1;
	memset(windows[win].data,0,sizeof(EDITBOXDATA) + 2500);
	p = (LPEDITBOXDATA)windows[win].data;
	init_winex_header(&windows[win],sizeof(EDITBOXDATA));
	p->txt = p->hdr.caption + 500;
	p->size_buffer = 2000;	
	append_win(win);
	windows[win].pfn_proc = (LPWNDPROC)wndproc_editbox_win;	
	wndproc_editbox_win(&windows[win],WM_CREATE,0,0);
	return win;
}
//---------------------------------------------------------------------------------
static int get_pos_from_char(HWND hWnd,int index,LPPOINT out)
{
	LPEDITBOXDATA p;
	RECT rc;
	int i,idx_font,ly,y,x;
	char *c;
	
	if(hWnd == NULL || hWnd->type != WC_EDIT || (p = (LPEDITBOXDATA)hWnd->data) == NULL || (c = p->txt) == NULL || out == NULL)
		return -1;
	get_client_rect(hWnd,&rc);
	idx_font = get_text_font(hWnd->screen);
	ly = get_font_height(hWnd->screen);
	if(index == -1)
		index = 0x7FFFFFFF;
	x = rc.left;
	for(i=y=0;i<index && c[i] != 0;){
		int letter,x1;
		
		letter = translate_UTF(&c[i],&x1);						
		i += x1;	
		switch(letter){
			case '\r':
				i++;
			case '\n':
				y += ly;
				x = rc.left;
			break;
		}
		x1 = fonts[idx_font].pfn_get_width(idx_font,letter);
		x += x1;
		if((hWnd->style & ES_MULTILINE) != 0){
			if(x > rc.right){
				y += ly;
				x = rc.left + x1;			
			}
			if(letter == 32){
				int i1;
				
				x1 = x;
				for(i1 = i+1;c[i1] != 0;i1++){
					letter = c[i1];
					if(isalpha(letter)){
						x1 += fonts[idx_font].pfn_get_width(idx_font,letter);
						if(x1 > rc.right){
							x = rc.left;
							y += ly;
							break;
						}
					}
					else
						break;
				}
			}
		}		
	}	
	if(i != index && index != 0x7FFFFFFF)
		return -2;
	out->x = x-rc.left;
	out->y = y;
	return 0;
}
//---------------------------------------------------------------------------------
static char *get_line(HWND hWnd,int line,int *index)
{
	LPEDITBOXDATA p;
	char *c,*c1;
	int i,count;
	
	if(hWnd == NULL || hWnd->type != WC_EDIT || (p = (LPEDITBOXDATA)hWnd->data) == NULL || (c = p->txt) == NULL)
		return NULL;
	if(index != NULL)
		*index = 0;
	if(line == 0)
		return c;
	c1 = c;
	for(i = count = 0;c[i] != 0;){
		int letter,x1;
		
		letter = translate_UTF(&c[i],&x1);						
		i += x1;
		switch(letter){
			case '\r':
				i++;
			case '\n':
				count++;
				c1 = &c[i+1];
				if(count == line){
					if(index != NULL)
						*index = i+1;
					return c1;		
				}
			break;
		}
	}
	if(line == -1){
		if(index != NULL)
			*index = (int)(((unsigned int)c1) - ((unsigned int)c));
		return c1;
	}
	return NULL;
}
//---------------------------------------------------------------------------------
static int get_line_count(HWND hWnd)
{
	LPEDITBOXDATA p;
	int i,count;
	char *c;
	
	if(hWnd == NULL || hWnd->type != WC_EDIT || (p = (LPEDITBOXDATA)hWnd->data) == NULL)
		return -1;
	if((c = p->txt) == NULL)
		return 0;
	for(i = count = 0;c[i] != 0;){
		int letter,x1;
		
		letter = translate_UTF(&c[i],&x1);						
		i += x1;
		switch(letter){
			case '\r':
				i++;
			case '\n':
				count++;
			break;
		}
	}
	return count;
}
//---------------------------------------------------------------------------------
int show_caret(HWND hWnd,int show)
{
#ifdef _DEBUG
	if(show){
		if(hWnd == NULL)
			return -1;
		if(caret_data.hWnd != hWnd){
			if(caret_data.timer_id != -1){
				destroy_timer(caret_data.timer_id);
				caret_data.timer_id = -1;
			}
		}
		if(caret_data.oam_index[hWnd->screen] == -1){
			unsigned short *sprite;
			int i;
			
			if((sprite = (unsigned short *)calloc(sizeof(unsigned short)*8*8,1)) != NULL){
				for(i=0;i<4;i++)
					sprite[56+i] = PA_RGB(0,0,0);
				PA_Create16bitSprite(hWnd->screen,64,(void *)sprite,OBJ_SIZE_8X8,256,192);
				free(sprite);
				caret_data.oam_index[hWnd->screen] = 64;
			}
		}
		if(caret_data.timer_id == -1)
			caret_data.timer_id = set_timer(hWnd,350,4444,NULL);
		caret_data.hWnd = hWnd;
	}
	else{
		if(caret_data.hWnd == hWnd){
			if(caret_data.timer_id != -1){
				destroy_timer(caret_data.timer_id);
				caret_data.timer_id = -1;
			}
			caret_data.hWnd = NULL;
		}
	}	
#endif
	return 0;
}
//---------------------------------------------------------------------------------
int draw_caret(HWND hWnd)
{
#ifdef _DEBUG
	if(caret_data.hWnd != hWnd)
		return -1;
	if(caret_data.oam_index[hWnd->screen] == -1)
		return -2;
	if(PA_GetSpriteX(hWnd->screen,caret_data.oam_index[hWnd->screen]) > 255){
		PA_SetSpriteX(hWnd->screen,caret_data.oam_index[hWnd->screen],caret_data.x);
		PA_SetSpriteY(hWnd->screen,caret_data.oam_index[hWnd->screen],caret_data.y);
	}
	else{
		PA_SetSpriteX(hWnd->screen,caret_data.oam_index[hWnd->screen],256);
	}
#endif
	return 0;
}
//---------------------------------------------------------------------------------
int update_caret_pos(HWND hWnd)
{	
#ifdef _DEBUG
	{
		POINT pt;
		
		if(get_pos_from_char(hWnd,-1,&pt))
			return -1;
		caret_data.x = pt.x;
		caret_data.y = pt.y + (get_font_height(hWnd->screen) - 8);
	}
	{
		RECT rc;
	
		get_client_rect(hWnd,&rc);
		
		caret_data.x += rc.left;
		caret_data.y += rc.top;
	}
#endif	
	return 0;
}
//---------------------------------------------------------------------------------
static int recalc_layout(LPWINDOW hWnd)
{
	LPEDITBOXDATA p;	
	int height,i;
	
	if(hWnd == NULL || !(hWnd->style & WS_VSCROLL) || (p = (LPEDITBOXDATA)hWnd->data) == NULL)
		return -1;
	{
		int font;
		RECT rc,rc_client;
		
		font = set_text_font(hWnd->screen,hWnd->font);
		get_client_rect(hWnd,&rc_client);
		rc = rc_client;
		draw_text(hWnd->screen,p->txt,-1,&rc,DT_WORDBREAK|DT_CALCRECT);		
		height = get_font_height(hWnd->screen);
		set_text_font(hWnd->screen,font);
		i = rc.bottom - rc_client.bottom;
		if(i >= 0 || abs(i) < height){
			i = abs(i);
			font = (i / height);
			if(font * height <= i)
				font++;
			i = font * height;
		}
		else 
			i = 0;
	}
	set_scrollbar_info(p->hdr.vert_sb,0,i,i > 0 ? height : 0);		
	return 0;
}
//---------------------------------------------------------------------------------
static int wndproc_editbox_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPEDITBOXDATA p;
	
	switch(uMsg){
		case EM_GETLINECOUNT:
			return get_line_count(hWnd);
		case WM_VSCROLL:
			switch(LOWORD(wParam)){
				case SB_THUMBPOSITION:
				break;
			}
		break;
		case WM_SETFOCUS:
			p = (LPEDITBOXDATA)hWnd->data;
			if(p != NULL){
				adjust_input_text(p->txt);
				update_caret_pos(hWnd);
			}
#ifdef _DEBUG
			show_caret(hWnd,1);
#endif				
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case WM_TIMER:
			switch(wParam){
				case 4444:
					draw_caret(hWnd);
				break;
			}
		break;
		case WM_KILLFOCUS:
			p = (LPEDITBOXDATA)hWnd->data;
			if(p != NULL){
				strcpy(p->txt,(char *)text);
				show_caret(hWnd,0);
			}
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case WM_KEYDOWN:
			if(active_focus == get_window_index(hWnd)){
				p = (LPEDITBOXDATA)hWnd->data;
				if(p != NULL){
					if(strlen(text) < strlen(p->txt))
						hWnd->status |= 1;
					strcpy(p->txt,text);
					update_caret_pos(hWnd);
					recalc_layout(hWnd);
					SendMessage(get_window_index((HWND)hWnd->childs.parent),WM_COMMAND,MAKEWPARAM(hWnd->childs.id,EN_UPDATE),(LPARAM)hWnd);
					hWnd->status |= 2;
				}
			}
		break;
		case WM_LBUTTONDOWN:
		break;
		case WM_CREATE:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
		case WM_GETTEXTLENGTH:
			p = (LPEDITBOXDATA)hWnd->data;
			if(p == NULL || p->txt == NULL)
				return -1;
			return strlen(p->txt);
		case WM_GETTEXT:
			{
				int len;
				
				p = (LPEDITBOXDATA)hWnd->data;
				if(p == NULL || p->txt == NULL || !lParam)
					return -1;
				len = strlen(p->txt);				
				strncpy((char *)lParam,p->txt,wParam);
				return len > wParam ? wParam : len;
			}
		case WM_SETTEXT:
			p = (LPEDITBOXDATA)hWnd->data;
			if(p != NULL && lParam){							
				if(!wParam){
					strcpy(p->txt,(char *)lParam);
					recalc_layout(hWnd);
					SendMessage(get_window_index((HWND)hWnd->childs.parent),WM_COMMAND,MAKEWPARAM(hWnd->childs.id,EN_UPDATE),(LPARAM)hWnd);
					hWnd->status |= 3;
					if(active_focus == get_window_index(hWnd))
						adjust_input_text(p->txt);
				}
				else
					return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
			}
		break;
		case WM_PAINT:
			{
				RECT rc;
				u16 color;
				
				if(wndproc_winex_win(hWnd,uMsg,wParam,lParam))
					return 1;
				if(dmaBusy(3) || !trylock_mutex(&hWnd->critical_section))
					return 1;				
				if((p = (LPEDITBOXDATA)hWnd->data) != NULL){
					get_client_rect(hWnd,&rc);
					if(hWnd->screen == 0)
						color = 5;
					else
						color = PA_RGB(0,0,0);					
					color = set_text_color(hWnd->screen,color);
					set_text_font(hWnd->screen,hWnd->font);
					draw_text(hWnd->screen,p->txt,-1,&rc,(hWnd->style & ES_MULTILINE) == 0 ? DT_VCENTER : 0);	
					set_text_color(hWnd->screen,color);
				}
				unlock_mutex(&hWnd->critical_section);
			}
		break;
		case WM_ERASEBKGND:
		case WM_NCCALCSIZE:
			return wndproc_winex_win(hWnd,uMsg,wParam,lParam);
	}
	return 0;
}
