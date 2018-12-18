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
 
#include "fb.h"
#include "listbox.h"
#include "button.h"
#include "editbox.h"
#include "draws.h"
#include "fb_friends.h"

static int friends_win = -1,timer = -1,search_friends_win = -1,search_quit = 0,search_start = 0;
static LPWNDPROC old_listbox_proc = NULL;
static LPWNDPROC old_dlg_proc = NULL;
extern int (*pfn_MainFunc)(void);
extern int (*pfn_timer2Func)(void);
extern unsigned long main_mutex;
static unsigned long timer2_command;
extern int modal_win;
extern unsigned char fb_char_test[];
static LPFBSEARCH_USERS psearch_users = NULL;
LPFBLIST ls_pships = NULL;
//---------------------------------------------------------------------------------
static int clear_pships_list(void *p)
{
	LPFBPSHIP pc;	
	int i;
	
	pc = (LPFBPSHIP)p;
	if(pc && pc->avatar != NULL)
		ram_disk_free(pc->avatar);
	return 1;
}
//---------------------------------------------------------------------------------
int init_pships_list(void *mem)
{
	ls_pships = init_fb_list(mem,10000);
	return 10000;
}
//---------------------------------------------------------------------------------
static int add_pships_new(char *uid,char *name,char *msg,char *url_avatar)
{
	LPFBPSHIP p;	
	int size,len,len_name,len_url,len_msg;
	
	if(uid == NULL || uid[0] == 0)
		return -1;	
	len_name = len_url = len_msg = 0;
	size = sizeof(LPFBPSHIP) + (len = strlen(uid)) + 1;
	if(name != NULL && name[0] != 0)
		size += (len_name = strlen(name)) + 1;
	if(url_avatar != NULL && url_avatar[0] != 0)
		size += (len_url = strlen(url_avatar)) + 1;	
	if(msg != NULL && msg[0] != 0)
		size += (len_msg = strlen(msg)) + 1;	
	size = (size + 3) & ~3;
	{
		LPFBLIST_ITEM item;
		
		item = add_fb_list_item(ls_pships,NULL,size);
		if(item == NULL)
			return -2;
		p = (LPFBPSHIP)item->data;
	}
	p->uid = (char *)(p+1);
	strcpy(p->uid,uid);
	p->name = p->uid + len + 1;
	if(len_name > 0)
		strcpy(p->name,name);
	p->url_avatar = p->name + len_name + 1;
	if(len_url > 0)
		strcpy(p->url_avatar,url_avatar);
	p->msg = p->url_avatar + len_url + 1;
	if(len_msg > 0)
		strcpy(p->msg,msg);		
	return ls_pships->items - 1;
}
//---------------------------------------------------------------------------------
LPFBPSHIP get_pships_item(int index)
{
	return (LPFBPSHIP)get_fb_list_item(ls_pships,index);
}
//---------------------------------------------------------------------------------
static int new_listbox_proc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{	
	switch(uMsg){
		case WM_DESTROY:
			{
				LPWINEXHEADER p;
			
				windows[friends_win].style &= ~WS_VISIBLE;
				p = (LPWINEXHEADER)windows[friends_win].data;
				if(p != NULL && p->vert_sb != -1)
					windows[p->vert_sb].style &= ~WS_VISIBLE;				
				redraw_windows_screen(windows[friends_win].screen);				
				return 1;
			}
		break;
	}	
	return old_listbox_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
}
//---------------------------------------------------------------------------------
static int lbod_pships(LPDRAWITEMSTRUCT p)
{
	RECT rc;
	
	rc = p->rcItem;	
	if(ls_pships != NULL && is_ext_ram()){		
		char *sprite;
		LPFBPSHIP ps;
		
		ps = get_pships_item(p->itemData);
		if(ps != NULL && ps->avatar != NULL){
			if((sprite = ram_disk_lock(ps->avatar)) != NULL){
				int y;
								
				y = rc.bottom - rc.top;
				if(y > 24)
					y = 24;				
				draw_image(p->hwndItem->screen,rc.left,rc.top,24,y,(u8 *)sprite,-1,-1);
				ram_disk_unlock(ps->avatar);								
			}
		}
		rc.left += 26;
	}
	if(p->itemState & ODS_SELECTED)
		set_text_color(p->hwndItem->screen,PA_RGB(31,0,0));
	else
		set_text_color(p->hwndItem->screen,PA_RGB(0,0,0));
	draw_text(p->hwndItem->screen,p->itemText,-1,&rc,DT_WORDBREAK|DT_VCENTER);
	return 1;
}
//---------------------------------------------------------------------------------
int show_friends_notify_win()
{
	LPWINEXHEADER p;
	
	if(friends_win == -1)
		return -1;		
	active_win_top = friends_win;
	windows[friends_win].style |= WS_CAPTION|WS_TABSTOP|WS_VISIBLE|WS_CLOSE;
	windows[friends_win].status |= 3;
	p = (LPWINEXHEADER)windows[friends_win].data;
	if(p != NULL && p->vert_sb != -1)
		windows[p->vert_sb].style |= WS_VISIBLE;
	SetWindowText(friends_win,"Friends Notify");
	return friends_win;
}
//---------------------------------------------------------------------------------
int fb_friends_accept_req(char *uid)
{
	char *s;
	int cli_sock;
	
	s = (char *)temp_buffer;	
	sprintf(s,"type=friend_connect&id=%s&actions[accept]=Confirm&post_form_id=%s&fb_dtsg=%s&confirm=%s&post_form_id_source=AsyncRequest&__a=1",
			uid,pfb->post_form_id, pfb->dtsg, pfb->c_user);
	if(!fb_send_req(1,"/ajax/reqs.php",s,NULL,&cli_sock))
		return -1;
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
	return 0;
}
//---------------------------------------------------------------------------------
int fb_friends_reject_req(char *uid)
{
	char *s;
	int cli_sock;
	
	s = (char *)temp_buffer;	
	sprintf(s,"type=friend_connect&id=%s&action=reject&post_form_id=%s&fb_dtsg=%s&post_form_id_source=AsyncRequest&__a=1",
			uid, pfb->post_form_id, pfb->dtsg);
	if(!fb_send_req(1,"/ajax/reqs.php",s,NULL,&cli_sock))
		return -1;
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
	return 0;
}
//---------------------------------------------------------------------------------
int fb_friends_parse_req(char *buffer,unsigned int size)
{
	const char *uid_pre_text = "class=\"confirm\" id=\"friend_connect_";
	const char *name_pre_text = "<td class=\"info\"><a ";
	const char *msg_pre_text = "<div class=\"personal_msg\"><span>";
	char *p1,*p,*uid,*msg,*name,*p_end,*url_avatar;
	int i;
	
#ifdef _DEBUG
	FILE *fp;
	
	static int parsed = 0;
	
	if(parsed == 0){
		fp = fopen("data/fb_friends.txt","rb");
		if(fp != NULL){		
			fseek(fp,0,SEEK_END);
			size = ftell(fp);
			fseek(fp,0,SEEK_SET);
			fread((void *)pfb->current_page,1,size,fp);							
			buffer = pfb->current_page;
			fclose(fp);		
			parsed = 1;
		}
	}
#endif
	if(buffer == NULL || size == 0)
		return -1;
	clear_fb_list(ls_pships,clear_pships_list);
	if(friends_win != -1)
		SendMessage(friends_win,LB_RESETCONTENT,0,0);
	p = buffer;
	p_end = p + size;
	while((p = strstr(p, uid_pre_text))){
		url_avatar = NULL;
		p += strlen(uid_pre_text);
		p1 = strchr(p,'\"');
		if(p1 == NULL)
			continue;
		uid = p;
		p = p1 + 1;
		*p1 = 0;
		if(timer == -1)
			timer = set_timer(&windows[menu_bar_bottom_win],1000,2,NULL);
		if(friends_win == -1){
			friends_win = create_listbox_win(1,16,70,224,100);
			if(friends_win > -1){
				old_listbox_proc = windows[friends_win].pfn_proc;
				windows[friends_win].pfn_proc = (LPWNDPROC)new_listbox_proc;
				set_listbox_ownerdraw(friends_win,lbod_pships);
				SendMessage(friends_win,LB_SETITEMHEIGHT,0,26);				
			}						
		}
		p1 = html_get_image(p,size - ((unsigned int)p - (unsigned int)buffer),NULL);
		if(p1 != NULL){
			p1 = strstr(p1,"src");
			if(p1 != NULL){
				int i1;
				
				p1 += 3;
				while(*p1 != 0 && !isalpha(*p1))
					p1++;
				url_avatar = (char *)(temp_buffer + 5000);
				for(i1=0;*p1 != '\"';p1++){
					if(*p1 != '\\')
						url_avatar[i1++] = *p1;
				}
				url_avatar[i1] = 0;
			}
		}
		name = strstr(p,name_pre_text);
		if(name != NULL){
			int i1;
			
			name += strlen(name_pre_text);
			for(i=0,i1 = 1;name < p_end && *name != 0 && i1 > 0;i++,name++){
				if(*name == '<')
					i1++;
				else if(*name == '>')
					i1--;
			}
			for(i=0;&name[i] < p_end;i++){
				if(name[i] == '<')
					break;
			}
			name[i] = 0;
			p = &name[i+1];
		}
		msg = strstr(p,msg_pre_text);
		if(msg != NULL){
		}
		if(friends_win != -1){
			int item;
			
			if((item = add_pships_new(uid,name,NULL,url_avatar)) >= 0){
				i = SendMessage(friends_win,LB_ADDSTRING,0,(LPARAM)name);
				if(i > 0)
					*((unsigned long *)i) = item;
			}
		}
	}
	if(is_ext_ram()){
		LPFBPSHIP pc;
		LPFBLIST_ITEM item;
		
		pc = (LPFBPSHIP)get_fb_list_first(ls_pships,&item);
		for(i=0;i<pc != NULL;i++){
			if(pc->url_avatar != NULL && pc->url_avatar[0] != 0){
				pc->avatar = ram_disk_alloc(24*24*4);
				if(pc->avatar != NULL){
					unsigned short *sprite;
											
					if((sprite = ram_disk_lock(pc->avatar)) != NULL){
						fb_load_avatar(pc->url_avatar,sprite,24,24);							
						ram_disk_unlock(pc->avatar);				
					}			
				}				
			}
			pc = (LPFBPSHIP)get_fb_list_next(ls_pships,&item);	
		}
	}	
	return 0;
}
//---------------------------------------------------------------------------------
static int parse_search_result(char *data,unsigned int size)
{
	char *p,*p1,*p_name,*p_img,*p_last;
	int res,i,data_item;
	unsigned int items;
	
	SetWindowText(3,"Parsing search result...");
#ifdef _DEBUG
	{
		FILE *fp;
		
		fp = open_file("data/fb_search.txt","rb");
		if(fp != NULL){
			fseek(fp,0,SEEK_END);
			size = ftell(fp);
			if(size > 85000)
				size = 84999;
			fseek(fp,0,SEEK_SET);
			fread((void *)pfb->current_page,1,size,fp);			
			data = pfb->current_page;
			fclose(fp);	
		}
	}
#endif	
	if(data == NULL || size == 0 || psearch_users == NULL)
		return -2;
	for(i=0;i<20;i++){
		if(psearch_users[i].avatar != NULL){
			ram_disk_free(psearch_users[i].avatar);		
			psearch_users[i].avatar = NULL;
		}
		psearch_users[i].uid[0] = 0;
		psearch_users[i].url_avatar[0] = 0;
		psearch_users[i].flags = 0;
	}
	items = 0;
	res = -1;
	p = data;	
	p_last = p;
	while(items < 20 && (p = strstr(p,"<div class=\\\"UIFullListing")) != NULL){
		p_name = p_img = NULL;
		res++;
		p += 26;
		p1 = strstr(p,"<a href");
		if(p1 != NULL){//image
			char *p2;
			
			p2 = strstr(p1,"/a>");
			if(p2 != NULL){
				p = p2;
				p2 = strstr(p1,"<img");
				if(p2 != NULL){
					p2 = strstr(p2+4,"src");
					if(p2 != NULL && p2 < p){
						p2 += 3;
						while(*p2 != '\"' && p2 < p) 
							p2++;
						if(*p2 == '\"'){
							int len;
							
							p2++;
							p_img = p2;
							len = 0;
							while(*p2 != '\"' && p2 < p){ 
								p2++;
								if(*p2 != '\\')
									len++;
							}
							if(*p2 == '\"'){															
								p2 = p_img;
								p_img = (char *)temp_buffer;
								for(i=0;i<len;p2++){
									if(*p2 == '\\')
										continue;
									p_img[i++] = *p2;
								}
								p_img[i] = 0;
							}
							else
								p_img = NULL;
						}
					}
				}
			}
		}
		p1 = strstr(p,"<a href");
		if(p1 != NULL){//name
			char *p2;
			
			p2 = strstr(p1,"/a>");
			if(p2 != NULL){								
				p = p2;
				while(*p2 != '<') p2--;
				*p2=0;
				while(*p2 != '>' && p2 > p1) p2--;
				if(*p2 == '>')
					p_name = p2 + 1;
			}
		}
		p1 = html_find_tag(p,-1,"li class=\\\"actionspro_li\\\"");
		if(p1 != NULL){
			char *p2;
			
			p1 += 28;
			p1 = html_get_tag(p1,-1,"a",NULL,&i,NULL);
			p1 = strnstri(p1,"addfriend",i);
			if(p1 != NULL)
				psearch_users[items].flags |= 1;
		}
		p1 = strstr(p,"?compose&amp;id=");
		if(p1 == NULL)
			continue;
		p1 += 16;
		for(i=0;;i++){
			if(!isdigit(p1[i])){
				p1[i] = 0;
				p = p1 + i + 1;
				p_last = p;
				strcpy(psearch_users[items].uid,p1);
				if(p_img != NULL)
					strcpy(psearch_users[items].url_avatar,p_img);				
				data_item = SendMessage_HWND(get_dlg_item(&windows[search_friends_win],1000),LB_ADDSTRING,0,p_name);
				if(data_item > 0)
					*((u32 *)data_item) = items;
				items++;
				break;
			}
		}		
	}
	if(items == 0)
		return res;
	i = html_get_anchor_ref(p_last,-1,"UIPager_Button UIPager_ButtonForward",NULL) != NULL ? 1 : 0;
	enable_window(get_dlg_item(&windows[search_friends_win],1003),i);
	
	i = html_get_anchor_ref(p_last,-1,"UIPager_Button UIPager_ButtonBack",NULL) != NULL ? 1 : 0;
	
	enable_window(get_dlg_item(&windows[search_friends_win],1002),i);
	if(is_ext_ram()){
		SetWindowText(3,"Loading avatars...");
		for(i=0;i<items;i++){
			if(psearch_users[i].url_avatar[0] == 0)
				continue;
			psearch_users[i].avatar = ram_disk_alloc(32*32*4);
			if(psearch_users[i].avatar == NULL)
				continue;
			{
				unsigned short *sprite;
												
				sprite = ram_disk_lock(psearch_users[i].avatar);
				if(sprite == NULL){
					ram_disk_free(psearch_users[i].avatar);
					psearch_users[i].avatar = NULL;
					continue;
				}
				fb_load_avatar(psearch_users[i].url_avatar,sprite,32,32);
				ram_disk_unlock(psearch_users[i].avatar);				
			}
		}
	}
	return res;
}
//---------------------------------------------------------------------------------
static int timer2_search_friend_proc(void)
{	
	if(!trylock_mutex(&main_mutex)){
		PA_SetBrightness(windows[search_friends_win].screen,-8);
		return 0;
	}
	_REG16(REG_BRIGHT + (0x1000 * windows[search_friends_win].screen)) = 0;
	start_wait_anim();
	REG_IME = 1;
	switch(timer2_command){
		case 1://Start Search
			{
				char *s,*s1;
				int i;
				
				timer2_command = 0;
				s = (char *)&temp_buffer[5000];
				s1 = s + 1000;
				memset(s,0,2000);
				
				enable_window(get_dlg_item(&windows[search_friends_win],1001),0);				
				SendMessage_HWND(get_dlg_item(&windows[search_friends_win],1004),WM_GETTEXT,300,(LPARAM)s);
				encode_string(s,s1);
				SendMessage_HWND(get_dlg_item(&windows[search_friends_win],1000),LB_RESETCONTENT,0,0);
				s = fb_search_friend(s1,search_start,&i);
				if(s != NULL){
					parse_search_result(s,i);
					redraw_window(get_window_index(get_dlg_item(&windows[search_friends_win],1000)));
					set_focus_window(get_dlg_item(&windows[search_friends_win],1000));
					SetWindowText(3,"");
				}
			}
		break;
		case 2:
			{
				int item;				
				
				timer2_command = 0;
				item = SendDlgItemMessage(&windows[search_friends_win],1000,LB_GETCURSEL,0,0);
				if(item != LB_ERR){
					char *s,*c,*req;
					int i,cli_sock;
										
					s = (char *)(temp_buffer+5000);
					c = s + 1000;
					req = c + 1000;
					memset(s,0,3000);
										
					encode_string((char *)fb_char_test,c);
					sprintf(s,"fb_dtsg=%s&post_form_id=%s&charset_test=%s&confirmed=1",pfb->dtsg,pfb->post_form_id,c);
					sprintf(req,"/addfriend.php?id=%s",psearch_users[item].uid);
					i = fb_send_req(FB4_RQ_POST,req,s,NULL,&cli_sock);
					if(i > 0){
						SetWindowText(3,"Sending a request...");
						i = recv_socket(cli_sock,NULL,0);
						shutdown(cli_sock,SHUT_RDWR);
						closesocket(cli_sock);
					}
				}
			}
		break;
	}	
	destroy_wait_anim();
	unlock_mutex(&main_mutex);
	pfn_timer2Func = NULL;
	return 1;
}
//---------------------------------------------------------------------------------
static int new_search_friends_dlgproc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	int res;
	
	res = old_dlg_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
	switch(uMsg){
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case 2:
					modal_win = -1;
					{
						HWND h;
							
						h = (HWND)windows[search_friends_win].childs.next;
						while(h != NULL){
							HWND h1;
								
							h1 = (HWND)h->childs.next;
							destroy_window(get_window_index(h));
							h = h1;
						}
					}
					destroy_window(search_friends_win);				
					search_friends_win = -1;
					if(psearch_users != NULL){
						free(psearch_users);
						psearch_users = NULL;
					}
					redraw_windows_screen(1);
				break;
				case 1002:
					switch(HIWORD(wParam)){
						case BN_CLICKED:
							if(pfn_timer2Func != timer2_search_friend_proc){
								search_quit = 0;
								timer2_command = 1;								
								search_start -= SendDlgItemMessage(hWnd,1000,LB_GETCOUNT,0,0);
								pfn_timer2Func =  timer2_search_friend_proc;	
								enable_window((HWND)lParam,0);
							}						
						break;
					}
				break;
				case 1003: //Next Button
					switch(HIWORD(wParam)){
						case BN_CLICKED:
							if(pfn_timer2Func != timer2_search_friend_proc){
								search_quit = 0;
								timer2_command = 1;								
								search_start += SendDlgItemMessage(hWnd,1000,LB_GETCOUNT,0,0);
								pfn_timer2Func =  timer2_search_friend_proc;	
								enable_window((HWND)lParam,0);
							}						
						break;
					}
				break;
				case 1004:
					switch(HIWORD(wParam)){
						case EN_UPDATE:
							enable_window(get_dlg_item(hWnd,1005),SendMessage_HWND((HWND)lParam,WM_GETTEXTLENGTH,0,0) > 0 ? 1 : 0); 
						break;
					}
				break;
				case 1005://Search Button
					switch(HIWORD(wParam)){
						case BN_CLICKED:
							if(pfn_timer2Func != timer2_search_friend_proc){
								search_quit = 0;
								timer2_command = 1;
								search_start = 0;
								pfn_timer2Func =  timer2_search_friend_proc;									
								enable_window((HWND)lParam,0);
							}
						break;
					}
				break;
				case 1000:
					switch(HIWORD(wParam)){
						case LBN_SELCHANGE:
							{
								int enable,item;
								
								enable = 0;
								item = SendDlgItemMessage(&windows[search_friends_win],1000,LB_GETCURSEL,0,0);
								if(item != LB_ERR){
									if(psearch_users[item].flags & 1)
										enable = 1;
								}
								enable_window(get_dlg_item(&windows[search_friends_win],1001),enable);
							}
						break;
					}
				break;
				case 1001://Add button
					switch(HIWORD(wParam)){
						case BN_CLICKED:
							if(pfn_timer2Func != timer2_search_friend_proc){
								search_quit = 0;
								timer2_command = 2;
								search_start = 0;
								pfn_timer2Func =  timer2_search_friend_proc;									
								enable_window((HWND)lParam,0);
							}						
						break;
					}
				break;
			}
		break;
	}
	return res;
}
//---------------------------------------------------------------------------------
static int lib_od(LPDRAWITEMSTRUCT p)
{
	RECT rc;
	
	rc = p->rcItem;	
	if(psearch_users != NULL && is_ext_ram()){		
		char *sprite;
		
		if(psearch_users[p->itemData].avatar != NULL){
			if((sprite = ram_disk_lock(psearch_users[p->itemData].avatar)) != NULL){
				int y;
								
				y = rc.bottom - rc.top;
				if(y > 32)
					y = 32;				
				draw_image(p->hwndItem->screen,rc.left,rc.top,32,y,(u8 *)sprite,-1,-1);
				ram_disk_unlock(psearch_users[p->itemData].avatar);								
			}
		}
		rc.left += 34;
	}
	if(p->itemState & ODS_SELECTED)
		set_text_color(p->hwndItem->screen,PA_RGB(31,0,0));
	else
		set_text_color(p->hwndItem->screen,PA_RGB(0,0,0));
	draw_text(p->hwndItem->screen,p->itemText,-1,&rc,DT_WORDBREAK|DT_VCENTER);
	return 1;
}
//---------------------------------------------------------------------------------
int search_friends()
{
	int w;
	
	if(search_friends_win != -1)
		return -2;
	search_start = 0;
	search_friends_win = create_dialog_win(1,10,30,230,140,1,"Search Friends");
	if(search_friends_win == -1)
		return -1;
	windows[search_friends_win].style |= WS_CLOSE;
	old_dlg_proc = windows[search_friends_win].pfn_proc;
	windows[search_friends_win].pfn_proc = (LPWNDPROC)new_search_friends_dlgproc;	

	w = create_listbox_win(1,15,69,210,80);
	if(w < 0)
		return -1;
	windows[w].style |= WS_TABSTOP|WS_VISIBLE|WS_CHILD;
	set_dlg_item(w,&windows[search_friends_win],1000);
	{
		LPWINEXHEADER p;
		
		p = (LPWINEXHEADER)windows[w].data;
		if(p != NULL && p->vert_sb != -1)
			windows[p->vert_sb].style |= WS_VISIBLE;		
		set_listbox_ownerdraw(w,lib_od);
		SendMessage(w,LB_SETITEMHEIGHT,0,34);										
	}
	w = create_button_win(1,15,152,50,13);
	if(w == -1)
		return -1;
	windows[w].style |= WS_TABSTOP|WS_VISIBLE|WS_CHILD;
	SetWindowText(w,"Add");
	enable_window(&windows[w],0);
	set_dlg_item(w,&windows[search_friends_win],1001);
	
	w = create_button_win(1,70,152,40,13);
	if(w == -1)
		return -1;
	windows[w].style |= WS_TABSTOP|WS_VISIBLE|WS_CHILD;
	SetWindowText(w,"Prev");
	enable_window(&windows[w],0);
	set_dlg_item(w,&windows[search_friends_win],1002);
	
	w = create_button_win(1,130,152,40,13);
	if(w == -1)
		return -1;
	windows[w].style |= WS_TABSTOP|WS_VISIBLE|WS_CHILD;
	SetWindowText(w,"Next");
	enable_window(&windows[w],0);
	set_dlg_item(w,&windows[search_friends_win],1003);

	w = create_editbox_win(1,55,49,135,14);
	if(w == -1)
		return -1;
	windows[w].style |= WS_TABSTOP|WS_CHILD|WS_VISIBLE;
	set_dlg_item(w,&windows[search_friends_win],1004);	
	set_focus_window(&windows[w]);
	
	w = create_button_win(1,195,49,40,13);
	if(w == -1)
		return -1;
	windows[w].style |= WS_TABSTOP|WS_VISIBLE|WS_CHILD;
	SetWindowText(w,"Search");
	enable_window(&windows[w],0);
	set_dlg_item(w,&windows[search_friends_win],1005);

	redraw_windows_screen(1);

	psearch_users = (LPFBSEARCH_USERS)calloc(20,sizeof(FBSEARCH_USERS));
	if(psearch_users == NULL)
		return -3;		
#ifdef _DEBUG
	parse_search_result(NULL,0);
#endif		
	return search_friends_win;
}