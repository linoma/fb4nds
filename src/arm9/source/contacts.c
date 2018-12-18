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
 
#include "contacts.h"
#include "listbox.h"
#include "editbox.h"
#include "fb.h"
#include "chat_win.h"
#include "json.h"

extern int (*pfn_MainFunc)(void);
extern int (*pfn_VBLFunc)(void);
extern u8 *temp_buffer;
extern int read_keyboard();
//---------------------------------------------------------------------------------
static int win_list = -1,win_name = -1,win_email = -1,win_dlg = -1;
static int (*old_pfn_MainFunc)(void);
static int (*old_pfn_VBLFunc)(void);
static LPWNDPROC old_proc,old_listbox_proc;
LPFBLIST ls_contacts;
//---------------------------------------------------------------------------------
int init_contact_list(void *mem)
{
	ls_contacts = init_fb_list(mem,15000);
	return 15000;
}
//---------------------------------------------------------------------------------
int get_online_contacts()
{
	LPFBCONTACT p;
	LPFBLIST_ITEM item;
	int i,count;
	
	if(ls_contacts == NULL)
		return -1;	
	p = (LPFBCONTACT)get_fb_list_first(ls_contacts,&item);
	for(count = 0;p != NULL;){
		if(p->status & 1)
			count++;
		p = (LPFBCONTACT)get_fb_list_next(ls_contacts,&item);
	}
	return count;
}
//---------------------------------------------------------------------------------
LPFBCONTACT get_selected_contact()
{
	int index;

	if(win_list == -1)
		return NULL;
	index = SendMessage(win_list,LB_GETCURSEL,0,0);
	if(index == -1)
		return NULL;
	index = SendMessage(win_list,LB_GETITEM,index,0);
	if(index == -1)
		return NULL;
	index = ((index + strlen((char *)index) + 8) & ~3) - 4;	
	index = (int)*((int *)index);						
	return get_contact_item(index);
}
//---------------------------------------------------------------------------------
LPFBCONTACT get_contact_item(int index)
{
	return (LPFBCONTACT)get_fb_list_item(ls_contacts,index);
}
//---------------------------------------------------------------------------------
LPFBCONTACT find_chat_win_uid(LPWINDOW hWnd,char *uid)
{
	LPCHATTABDATA p;
	LPFBCONTACT p1;
	char *c;
	unsigned int i,item;
	
	if(uid == NULL || hWnd == NULL || hWnd->type != WC_TABCONTROL || (p = (LPCHATTABDATA)hWnd->data) == NULL)
		return NULL;
	c = (char *)p->data;
	if(c == NULL)
		return 0;
	for(i = item = 0;i < p->size_used;item++){				
		while(*c != 0){
			i++;
			c++;
		}
		i += 8;
		c += 8;
		{
			unsigned int i1;
			
			i1 = (unsigned int)c;
			i1 = (i1 & ~3) - 4;
			i1 = *((unsigned int *)i1);
			p1 = (LPFBCONTACT)i1;
			if(p1 != NULL && strcmp(uid,p1->uid) == 0)
				return p1;
		}
	}
	return NULL;
}
//---------------------------------------------------------------------------------
static int lib_od(LPDRAWITEMSTRUCT p)
{
	LPFBCONTACT p1;
	RECT rc;
	int y;
	
	p1 = get_contact_item(p->itemData);
	if(p1 == NULL)
		return 1;
	rc = p->rcItem;	
	y = (rc.bottom - rc.top) >> 1;
	fill_circle(p->hwndItem->screen,rc.left + y,rc.top + y,y>>1,((p1->status & 1) ? PA_RGB(0,31,0) : PA_RGB(15,15,15)));
	rc.left += y << 1;		
	if(p->itemState & ODS_SELECTED)
		set_text_color(p->hwndItem->screen,PA_RGB(16,0,0));
	else
		set_text_color(p->hwndItem->screen,PA_RGB(0,0,0));	
	draw_text(p->hwndItem->screen,(char *)p1->name,-1,&rc,DT_VCENTER);
	return 1;
}
//---------------------------------------------------------------------------------
static int new_listbox_proc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	switch(uMsg){
		case WM_DESTROY:
			win_list = -1;
		break;
	}
	return old_listbox_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
}
//---------------------------------------------------------------------------------
int show_contacts_list()
{
	if(win_list > -1)
		return win_list;
	win_list = create_listbox_win(1,16,30,120,160);
	if(win_list < 0)
		return -1;
	old_listbox_proc = windows[win_list].pfn_proc;
	windows[win_list].pfn_proc = (LPWNDPROC)new_listbox_proc;
	windows[win_list].style |= WS_CAPTION|WS_TABSTOP|WS_VISIBLE|WS_CLOSE;
	SetWindowText(win_list,"Contacts");	
	SendMessage(win_list,LB_SETITEMHEIGHT,0,15);
	set_listbox_ownerdraw(win_list,lib_od);
	set_dlg_item(win_list,&windows[1],2000);
	refresh_contacts_list();
	return win_list;
}
//---------------------------------------------------------------------------------
int update_contacts_list()
{
	LPFBCONTACT p;
	LPFBLIST_ITEM item;
	
	p = (LPFBCONTACT)get_fb_list_first(ls_contacts,&item);
	while(p != NULL){
		if(p->status & 2)
			p->status &= ~2;
		else
			p->status &= ~1;//not online
		p = (LPFBCONTACT)get_fb_list_next(ls_contacts,&item);
	}
	refresh_contacts_list();
	redraw_windows();
	return 0;
}
//---------------------------------------------------------------------------------
LPFBCONTACT find_contact_from_id(char *uid)
{
	LPFBCONTACT p;
	LPFBLIST_ITEM item;
	
	p = (LPFBCONTACT)get_fb_list_first(ls_contacts,&item);
	if(uid == NULL || uid[0] == 0 || p == NULL)
		return NULL;	
	while(p != NULL){
		if(p->uid != NULL){
			if(!strcmp(uid,p->uid))
				return p;
		}
		p = (LPFBCONTACT)get_fb_list_next(ls_contacts,&item);
	}
	return NULL;
}
//---------------------------------------------------------------------------------
int add_contact(char *name,char *uid,char *url_avatar)
{
	LPFBCONTACT p;
	int size,len,len_uid,len_avatar;
	
	if(name == NULL || uid == NULL || name[0] == 0 || uid[0] == 0)
		return -1;
	size = sizeof(FBCONTACT) + (len = strlen(name)) + 1 + (len_uid = strlen(uid)) + 1;	
	size += (len_avatar = strlen(url_avatar)) + 1;
	size = (size + 3) & ~3;
	{
		LPFBLIST_ITEM item;
		
		item = add_fb_list_item(ls_contacts,NULL,size);
		if(item == NULL)
			return -2;
		p = (LPFBCONTACT)item->data;
	}			
	p->name = (char *)(p+1);
	strcpy(p->name,name);
	p->uid = p->name + len + 1;
	strcpy(p->uid,uid);
	p->url_avatar = p->uid + len_uid + 1;
	strcpy(p->url_avatar,url_avatar);
	p->avatar = NULL;
	p->win = -1;
	p->mail = NULL;
	p->status = 0;
	return ls_contacts->items - 1;
}
//---------------------------------------------------------------------------------
int refresh_contacts_list()
{
	unsigned int i;
	int data;
	LPFBCONTACT pc;
	LPFBLIST_ITEM item;
			
#ifdef _DEBUG
/*	FILE *fp;
	char *p,*p1,*s,*p2,*p3;
	int item;
	
	clear_contact_list(ls_contacts);
	fp = open_file("data/contacts.lst","rb");
	if(fp == NULL)
		return -2;
	fseek(fp,0,SEEK_END);
	i = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fread(temp_buffer,i,1,fp);
	fclose(fp);
	temp_buffer[i] = 0;
	p = (char *)temp_buffer;
	s = p + 10000;
	while((p = strstr(p,"<contact")) != NULL){		
		p += 8;
		p1 = strstr(p,"</contact>");
		if(p1 == NULL)
			continue;
		//<name></name>
		//<email></email>
		p2 = strstr(p,"<email>");
		if(p2 == NULL || p2 > p1)
			continue;
		p2 += 7;
		p3 = strstr(p2,"</email>");
		if(p3 == NULL || p3 > p1)
			continue;
		i = 0;
		while(p2 < p3)
			s[i++] = *p2++;
		s[i] = 0;
		item = add_contact(s,"000",NULL);
		p = p1 + 10;
	}*/
#endif	
	if(win_list == -1)
		return -1;	
	SendMessage(win_list,LB_RESETCONTENT,0,0);		
	pc = (LPFBCONTACT)get_fb_list_first(ls_contacts,&item);	
	for(i=0;pc != NULL;i++){
		data = SendMessage(win_list,LB_ADDSTRING,0,(LPARAM)pc->name);
		if(data > 0)
			*((u32 *)data) = i;
		pc = (LPFBCONTACT)get_fb_list_next(ls_contacts,&item);
	}	
	return 0;
}
//---------------------------------------------------------------------------------
void hide_contacts_list()
{
	if(win_list == -1)
		return;
	destroy_window(win_list);
	win_list = -1;
}
//---------------------------------------------------------------------------------
static int new_chat_win_proc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	LPFBCONTACT pfbc;
	
	pfbc = (LPFBCONTACT)hWnd->user_data;
	if(pfbc == NULL)
		return 0;
	switch(uMsg){
		case WM_MEASUREITEM:
			{
				LPMEASUREITEM p;
				
				p = (LPMEASUREITEM)lParam;
				if(p != NULL){
					int font;
					RECT rc;
					
					font = set_text_font(hWnd->screen,hWnd->font);
					get_client_rect(hWnd,&rc);
					rc.top = 0;
					draw_text(hWnd->screen,p->itemText,-1,&rc,DT_CALCRECT|DT_LEFT|DT_WORDBREAK);					
					set_text_font(hWnd->screen,font);
					font = rc.bottom - rc.top;
					return font < 20 ? 20 : font;
				}
				return 20;
			}
		case WM_NCCALCSIZE:
			pfbc->old_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
			if(pfbc->avatar != NULL && (hWnd->style & WS_CAPTION))
				((LPRECT)lParam)->top += 22;
			return 0;
		case WM_DESTROY:
			pfbc->win = -1;
		break;
		case WM_ERASEBKGND:
			{
				LPWINEXHEADER p;
				int res;
				
				if(pfbc->avatar != NULL && (hWnd->style & WS_CAPTION)){
					if((p = (LPWINEXHEADER)hWnd->data) != NULL){
						char *caption;
						RECT rc;
						
						caption = p->caption;
						p->caption = NULL;
						res = pfbc->old_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
						p->caption = caption;
						if(res)
							return res;
						rc = hWnd->rc;						
						rc.bottom = rc.top + 14 + 20;
						rc.left += 2;rc.top += 2;
						rc.right -= 2;rc.bottom -= 2;
						caption = ram_disk_lock(pfbc->avatar);
						draw_image(hWnd->screen,rc.left,rc.top,32,32,(u8 *)caption,-1,-1);
						ram_disk_unlock(pfbc->avatar);
						rc.left += 33;
						rc.right -= 10;
						if(p->caption != NULL)														
							draw_text(hWnd->screen,p->caption,-1,&rc,DT_VCENTER);													
						return 0;
					}
				}
			}
		break;
	}
	return pfbc->old_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
}
//---------------------------------------------------------------------------------
static int lib_od_chat_win(LPDRAWITEMSTRUCT p)
{
	RECT rc;
	u16 col;
	
	rc = p->rcItem;	
	if(p->itemState & ODS_SELECTED)
		col = set_text_color(p->hwndItem->screen,PA_RGB(16,0,0));
	else
		col = set_text_color(p->hwndItem->screen,(u16)p->itemData);		
	draw_text(p->hwndItem->screen,(char *)p->itemText,-1,&rc,DT_WORDBREAK);
	set_text_color(p->hwndItem->screen,col);
	return 1;
}
//---------------------------------------------------------------------------------
int create_chat_win(LPFBCONTACT pfbc)
{
	LPLISTBOXDATA p;
	
	if(pfbc == NULL)
		return -1;
	if(pfbc->win == -1){
		pfbc->win = create_listbox_win(1,10,10,150,150);
		if(pfbc->win < 0)
			return -2;		
		windows[pfbc->win].font = 1;
		windows[pfbc->win].user_data = (unsigned char *)pfbc;
		pfbc->old_proc = windows[pfbc->win].pfn_proc;
		windows[pfbc->win].pfn_proc = (LPWNDPROC)new_chat_win_proc;
		windows[pfbc->win].style |= WS_CAPTION|WS_BORDER|WS_CLOSE|WS_VISIBLE|WS_TABSTOP|LBS_OWNERDRAWVARIABLE;
		p = (LPLISTBOXDATA)windows[pfbc->win].data;
		if(p != NULL && p->hdr.vert_sb != -1){
			windows[p->hdr.vert_sb].style |= WS_VISIBLE;
			windows[p->hdr.vert_sb].status |= 3;
		}
		SetWindowText(pfbc->win,pfbc->name);
		//SendMessage(pfbc->win,LB_SETITEMHEIGHT,0,50);
		set_listbox_ownerdraw(pfbc->win,lib_od_chat_win);
		windows[pfbc->win].status |= 3;
		pfbc->status |= 0x80000000;
	}
	return pfbc->win;
}
//---------------------------------------------------------------------------------
static int load_avatar(LPFBCONTACT pfbc)
{
	int res;
	
	if(pfbc->avatar != NULL)
		return 0;
	SetWindowText(3,"Loading avatar...");
	res = -1;
	if(pfbc->url_avatar != NULL && pfbc->url_avatar[0] != 0 && (pfbc->status & 0x40000000) == 0){
		pfbc->avatar = ram_disk_alloc(32*32*4);
		if(pfbc->avatar != NULL){
			unsigned short *sprite;
						
			if((sprite = ram_disk_lock(pfbc->avatar)) != NULL){
				pfbc->status |= 0x40000000;
				if(!fb_load_avatar(pfbc->url_avatar,sprite,32,32)){
					windows[pfbc->win].status |= 3;
					res = 0;
				}
				ram_disk_unlock(pfbc->avatar);				
			}			
		}
	}
	if(res && pfbc->avatar != NULL){
		ram_disk_free(pfbc->avatar);
		pfbc->avatar = NULL;
	}
	return res;
}
//---------------------------------------------------------------------------------
int execute_contacts_loop()
{
	unsigned int i,sz;
	LPFBCONTACT pfbc;
	LPFBLIST_ITEM item;
	int size;
	
	pfbc = (LPFBCONTACT)get_fb_list_first(ls_contacts,&item);	
	while(pfbc != NULL){
		if(pfbc->win != -1){
			if(pfbc->status & 0x80000000){
				load_avatar(pfbc);
				SetWindowText(3,"Parsing history...");
				size = fb_history_fetch(pfbc->uid);
				if(size > 0){
					char *p,*p_end;
					unsigned int now;					
		
					p = json_get_node(pfb->current_page,"\"payload\"",&sz);
					if(p != NULL){
						p_end = pfb->current_page + size;
						p = json_get_node_value(p,"\"history\"",sz);
						SendMessage(pfbc->win,LB_RESETCONTENT,0,0);						
						if(p != NULL){
							int i2;
							char *p2,*s,*p_from,*p_time;
							unsigned long msg_time;
							
							now = time(NULL);
							while(p < p_end){
								p = json_get_node_value(p,"\"msg\"",sz);
								if(p == NULL)
									break;							
								i2 = 1;
								p2 = p;
								while(p2 < p_end){
									if(*p2 == '{')
										i2++;
									else if(*p2 == '}'){
										i2--;
										if(i2 == 0)
											break;
									}
									p2++;
								}
								if(p2 >= p_end)
									break;
								i2 = (int)((unsigned int)p2 - (unsigned int)p);
								if(i2 > sz)
									sz = 0;
								else
									sz -= i2;									
								p_from = json_get_node_value(p,"\"from\"",i2);
								p_time = json_get_node_value(p,"\"time\"",i2);
								p = json_get_node_value(p,"\"text\"",i2);
								if(p != NULL && p < p2 && p_time != NULL && p_time < p2){
									while(*p != '\"')
										p++;
									p++;									
									i2 = 0;
									s = (char *)temp_buffer;
									while(*p != '\"' && p < p_end)
										s[i2++] = *p++;
									s[i2] = 0;									
									if(p_from != NULL){
										i2 = 0;
										while(p_from[i2] != ',')
											i2++;
										p_from[i2] = 0;
									}
									i2 = 0;
									while(p_time[i2] != ',' && i2 < 10)
										i2++;
									p_time[i2] = 0;
									msg_time = atoi(p_time);
#ifdef _DEBUG
									if(1/*msg_time <= now && (now - msg_time) < 86400*/){
#else
									if(msg_time <= now && (now - msg_time) < 86400){									
#endif
										i2 = SendMessage(pfbc->win,LB_ADDSTRING,0,(LPARAM)s);
										if(i2 > 0){
											if(p_from != NULL && !strcmp(pfbc->uid,p_from))
												*((int *)i2) = PA_RGB(0,0,0);
											else
												*((int *)i2) = PA_RGB(22,22,22);
										}
									}
								}
								p = p2;
							}
							size = SendMessage(pfbc->win,LB_GETCOUNT,0,0);
							if(size > -1)
								SendMessage(pfbc->win,LB_SETCURSEL,(WPARAM)size-1,0);
						}
					}
				}				
				pfbc->status &= ~0x80000000;
			}			
		}
		pfbc = (LPFBCONTACT)get_fb_list_next(ls_contacts,&item);
	}
	return 1;
}
//---------------------------------------------------------------------------------
static int add_new_contact_main(void)
{
	read_keyboard();
	return 1;
}
//---------------------------------------------------------------------------------
static int vbl_proc(void)
{
	check_tab_key();
	
	if(Pad.Newpress.A)
		SendMessage(active_win_top,WM_KEYDOWN,65,0);
	else if(Pad.Held.A)
		SendMessage(active_win_top,WM_KEYDOWN,65,1);
	else if(Pad.Released.A)
		SendMessage(active_win_top,WM_KEYUP,65,0);

	if(Pad.Newpress.Start)
		SendMessage(active_win_top,WM_KEYDOWN,VK_RETURN,0);
	else if(Pad.Held.Start)
		SendMessage(active_win_top,WM_KEYDOWN,VK_RETURN,1);
	else if(Pad.Released.Start)
		SendMessage(active_win_top,WM_KEYUP,VK_RETURN,0);
	
	return 1;
}
//---------------------------------------------------------------------------------
static int new_contact_dlgproc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	int res;
	
	res = old_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
	switch(uMsg){
		case WM_PAINT:
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case 1:
				case 2:
					if(win_dlg != -1){
						destroy_window(win_dlg);
						win_dlg++;
						destroy_window(win_dlg);
						win_dlg++;
						destroy_window(win_dlg);
						win_dlg = -1;
					}
					if(win_name != -1){
						destroy_window(win_name);
						win_name = -1;
					}
					if(win_email != -1){
						destroy_window(win_email);
						win_email = -1;
					}
					pfn_MainFunc = old_pfn_MainFunc;
					pfn_VBLFunc = old_pfn_VBLFunc;
					redraw_windows();
				break;
				
			}
		break;
	}
	return res;
}
//---------------------------------------------------------------------------------
int add_new_contact()
{
	win_dlg = create_dialog_win(1,10,70,230,100,3,"Create a new contact");
	if(win_dlg == -1)
		return -1;
	old_proc = windows[win_dlg].pfn_proc;
	windows[win_dlg].pfn_proc = (LPWNDPROC)new_contact_dlgproc;	
	if(win_name == -1){
		win_name = create_editbox_win(1,52,98,159,14);
		if(win_name == -1)
			return -2;
		windows[win_name].style |= WS_TABSTOP|WS_CHILD;
		SetWindowText(win_name,"");
		active_win_top = win_name;
		set_focus_window(&windows[win_name]);
	}
	if(win_email == -1){
		win_email = create_editbox_win(1,52,115,159,14);	
		if(win_email == -1)
			return -3;
		windows[win_email].style |= WS_TABSTOP|WS_CHILD;
		SetWindowText(win_email,"");
	}
	
	old_pfn_MainFunc = pfn_MainFunc;
	old_pfn_VBLFunc = pfn_VBLFunc;
	
	pfn_MainFunc = add_new_contact_main;	
	pfn_VBLFunc = vbl_proc;
	
	redraw_windows();
	return 0;
}
