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
#include "draws.h"
#include "gzip_ex.h"

LPFBLIST ls_news;
static int win = -1,timer=-1;
static LPWNDPROC old_listbox_proc = NULL;
//---------------------------------------------------------------------------------
static int clear_news_item(void *a)
{
	LPFBNEWS pc;
	
	pc = (LPFBNEWS)a;
	if(pc->avatar != NULL)
		ram_disk_free(pc->avatar);
	return 1;
}
//---------------------------------------------------------------------------------
static void clear_news_list()
{
	clear_fb_list(ls_news,clear_news_item);
}
//---------------------------------------------------------------------------------
int init_news_list(void *mem)
{
	ls_news = init_fb_list(mem,10000);	
	return 10000;
}
//---------------------------------------------------------------------------------
static void load_all_news_avatars()
{
	int i1,cli_sock,i;
	char *prot,*host,*u,*s;
	LPFBNEWS pc;
	LPFBLIST_ITEM item;
	
	if(!is_ext_ram())
		return;
	SetWindowText(3,"Loading News avatars...");
	pc = (LPFBNEWS)get_fb_list_first(ls_news,&item);
	s = (char *)temp_buffer;
	for(i=0;pc != NULL;i++){	
		if(pc->url_profile == NULL || pc->url_profile[0] == 0 || (pc->status & 0x40000000))
			goto load_all_news_avatars_0;		
		u = NULL;			
		//Check for the same avatar
		{
			LPFBNEWS pc1;
			LPFBLIST_ITEM item1;
			
			pc1 = (LPFBNEWS)get_fb_list_first(ls_news,&item1);
			for(i1=0;i1 < i;i1++){	
				if(strcmp(pc1->url_profile,pc->url_profile) == 0){
					if(pc1->status & 0x40000000){
						pc->status |= 0x40000000;
						u = (char *)1;
						if(pc1->status & 0x80000000){
							pc->avatar = ram_disk_alloc(24*24*4);
							if(pc->avatar != NULL){
								unsigned short *sprite,*sprite1;
											
								if((sprite = ram_disk_lock(pc->avatar)) != NULL){
									if((sprite1 = ram_disk_lock(pc1->avatar)) != NULL){
										ram_disk_memcpy(sprite,sprite1,24*24*4);
										pc->status |= 0x80000000;
										ram_disk_unlock(pc1->avatar);				
									}
									ram_disk_unlock(pc->avatar);				
								}			
							}							
						}
					}
					i1 = i;
					break;
				}				
				pc1 = (LPFBNEWS)get_fb_list_next(ls_news,&item1);
			}			
		}
		if(u)
			goto load_all_news_avatars_0;		
		pc->status |= 0x40000000;
		u = s + 5000;
		strcpy(u,pc->url_profile);
		prot = strtok(u,"/");
		host = prot + strlen(prot) + 2;
		host = strtok(host,"/");
		u = host + strlen(host) + 1;		
		s[0] = '/';
		s[1] = 0;
		strcat(s,u);
		i1 = fb_send_req(FB4_RQ_GZIP_ENCODING,s,NULL,host,&cli_sock);				
		if(i1 < 1)
			goto load_all_news_avatars_0;
		i1 = recv_socket(cli_sock,NULL,15000);
		shutdown(cli_sock,SHUT_RDWR);
		closesocket(cli_sock);		
		if(i1 <= 0)
			goto load_all_news_avatars_0;	
		u = (char *)(temp_buffer + 10000);
		{
			int i2;
			
			i2 = gz_expand_response((char *)pfb->current_page,u,i1);
			if(i2 < 0)
				u = (char *)pfb->current_page;
			else
				i1 = i2;
		}
		while((u = strstr(u,"{\"pagelet_profile_photo\"")) != NULL){					
			prot = strstr(u,"<img");
			if(prot != NULL){
				u = prot;
				break;
			}
			u += 10; 
		}
		if(u == NULL)
			goto load_all_news_avatars_0;
		u = strstr(u+4,"src");
		if(u == NULL)
			goto load_all_news_avatars_0;				
		u += 3;
		while(*u != 0 && *u != '\"')
			u++;
		if(*u == 0)
			goto load_all_news_avatars_0;		
		u++;
		for(i1=0;*u != '\"';u++){
			if(*u != '\\')
				s[i1++] = *u;
		}
		s[i1] = 0;
		pc->avatar = ram_disk_alloc(24*24*4);
		if(pc->avatar != NULL){
			unsigned short *sprite;
											
			if((sprite = ram_disk_lock(pc->avatar)) != NULL){
				if(!fb_load_avatar(s,sprite,24,24))
					pc->status |= 0x80000000;
				ram_disk_unlock(pc->avatar);				
			}			
		}
load_all_news_avatars_0:		
		pc = (LPFBNEWS)get_fb_list_next(ls_news,&item);
	}		
	if(win != -1)
		redraw_window(win);	
}
//---------------------------------------------------------------------------------
int add_new_new(char *title,char *link,char *url_profile,char *time,unsigned int u_time)
{
	LPFBNEWS p;	
	int size,len,len_link,len_url,len_time;
	
	if(title == NULL || title[0] == 0)
		return -1;
	len_link = len_url = len_time = 0;
	size = sizeof(FBNEWS) + (len = strlen(title)) + 1;
	if(link != NULL && link[0] != 0)
		size += (len_link = strlen(link)) + 1;
	if(url_profile != NULL && url_profile[0] != 0)
		size += (len_url = strlen(url_profile)) + 1;	
	if(time != NULL && time[0] != 0)
		size += (len_time = strlen(time)) + 1;
	size = (size + 3) & ~3;
	{
		LPFBLIST_ITEM item;
		
		item = add_fb_list_item(ls_news,NULL,size);
		if(item == NULL)
			return -2;
		p = (LPFBNEWS)item->data;
	}	
	p->txt = (char *)(p+1);
	strcpy(p->txt,title);
	p->link = p->txt + len + 1;
	if(len_link > 0)
		strcpy(p->link,link);
	p->url_profile = p->link + len_link + 1;
	if(len_url > 0)
		strcpy(p->url_profile,url_profile);
	p->time = p->url_profile + len_url + 1;
	if(len_time > 0)
		strcpy(p->time,time);
	p->u_time = u_time;
	return ls_news->items - 1;
}
//---------------------------------------------------------------------------------
LPFBNEWS get_news_item(int index)
{
	return (LPFBNEWS)get_fb_list_item(ls_news,index);
}
//---------------------------------------------------------------------------------
static int lib_od(LPDRAWITEMSTRUCT p)
{
	LPFBNEWS pc;	
	RECT rc;
	char *txt;
	
	rc = p->rcItem;	
	txt = (char *)p->itemText;
	if(is_ext_ram()){		
		pc = get_news_item(p->itemData);
		if(pc != NULL){
			if(pc->status & 0x80000000){
				char *sprite;
				int y;
				
				y = rc.bottom - rc.top;
				if(y > 24)
					y = 24;
				if((sprite = (char *)ram_disk_lock(pc->avatar)) != NULL){
					draw_image(p->hwndItem->screen,rc.left,rc.top,24,y,(u8 *)sprite,-1,-1);
					ram_disk_unlock(pc->avatar);								
				}				
			}
		}
		rc.left += 26;				
	}
	if(p->itemState & ODS_SELECTED)
		set_text_color(p->hwndItem->screen,PA_RGB(16,0,0));
	else
		set_text_color(p->hwndItem->screen,PA_RGB(0,0,0));
	draw_text(p->hwndItem->screen,txt,-1,&rc,DT_WORDBREAK|DT_VCENTER);
	return 1;
}
//---------------------------------------------------------------------------------
static int new_listbox_proc(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam)
{
	switch(uMsg){
		case WM_DESTROY:
			win = -1;
		break;
	}
	return old_listbox_proc((struct WINDOW *)hWnd,uMsg,wParam,lParam);
}
//---------------------------------------------------------------------------
int show_notify_win()
{
	LPWINEXHEADER p;
	
	if(win == -1){
		win = create_listbox_win(1,16,70,224,100);
		if(win < 0)
			return -1;
		old_listbox_proc = windows[win].pfn_proc;
		windows[win].pfn_proc = (LPWNDPROC)new_listbox_proc;
		set_listbox_ownerdraw(win,lib_od);
		SendMessage(win,LB_SETITEMHEIGHT,0,27);							
	}		
	active_win_top = win;
	windows[win].style |= WS_CAPTION|WS_TABSTOP|WS_VISIBLE|WS_CLOSE;
	p = (LPWINEXHEADER)windows[win].data;
	if(p != NULL && p->vert_sb != -1)
		windows[p->vert_sb].style |= WS_VISIBLE;
	SetWindowText(win,"News");
	{
		LPFBNEWS pc;
		LPFBLIST_ITEM item;
		int i,data;
		
		SendMessage(win,LB_RESETCONTENT,0,0);	
		pc = (LPFBNEWS)get_fb_list_first(ls_news,&item);
		for(i=0;pc != NULL &&  i < 15;i++){
			data = SendMessage(win,LB_ADDSTRING,0,(LPARAM)pc->txt);
			if(data > 0)
				*((u32 *)data) = i;
			pc = (LPFBNEWS)get_fb_list_next(ls_news,&item);
		}		
	}
	return win;
}
//---------------------------------------------------------------------------
int fb_parse_notify(char *data,int size)
{
	char *p,*p1,*p2,*s,*p_end,*url_profile,*url_link,*p_item,*p_time;
	int i,i1;
	unsigned int u_time,last_fetch_time;
	
#ifdef _DEBUG
	{
		FILE *fp;
		
		fp = open_file("data/fb_notify.txt","rb");
		if(fp != NULL){
			fseek(fp,0,SEEK_END);
			size = ftell(fp);
			fseek(fp,0,SEEK_SET);
			fread((void *)pfb->current_page,1,size,fp);			
			data = pfb->current_page;
			fclose(fp);	
		}
	}	
#endif	
	if(data == NULL || size < 0)
		return -1;	
	p = strstr(data,"<rss version=\"");
	if(p == NULL)
		return -2;
	last_fetch_time = pfb->last_fetch_time;
	s = (char *)temp_buffer;
	while((p = strstr(p,"<item>")) != NULL){
		p += 6;
		p1 = strstr(p,"</item>");
		if(p1 != NULL){
			p_end = p1;
			p_item = p;
			p1 = strstr(p,"<title>");
			p = p_end + 7;
			url_profile = url_link = NULL;
			u_time = (unsigned int)-1;
			p_time = NULL;
			if(p1 != NULL){
				p1 += 7;
				p2 = strstr(p1,"</title>");
				if(p2 != NULL){
					for(i=0;p1 < p2;i++){
						s[i] = translate_UTF(p1,&i1);
						p1 += i1;
					}
					s[i] = 0;
					p2 = strstr(p_item,"<pubDate>");
					if(p2 < p_end){
						p2 += 9;
						p1 = strstr(p2,"</pubDate>");
						if(p1 != NULL && p1 < p_end){
							char *weekday,*month_string;
							unsigned int day,year,hour,minute,second,timezone,month;
							struct tm time_check;
														
							p_time = p2;
							for(i=0;&p_time[i] < p1;i++){
								if(p_time[i] == '<')									
									break;
							}
							p_time[i] = 0;							
							sscanf(p2, "%3s, %2u %3s %4u %2u:%2u:%2u %5d",(char*)&weekday,&day,(char*)&month_string,&year,&hour,&minute,&second,&timezone);															
							if (!strcmpi(month_string, "Jan")) 
								month = 1;
							else if (!strcmpi(month_string, "Feb")) 
								month = 2;
							else if (!strcmpi(month_string, "Mar")) 
								month = 3;
							else if (!strcmpi(month_string, "Apr")) 
								month = 4;
							else if (!strcmpi(month_string, "May")) 
								month = 5;
							else if (!strcmpi(month_string, "Jun")) 
								month = 6;
							else if (!strcmpi(month_string, "Jul")) 
								month = 7;
							else if (!strcmpi(month_string, "Aug")) 
								month = 8;
							else if (!strcmpi(month_string, "Sep")) 
								month = 9;
							else if (!strcmpi(month_string, "Oct")) 
								month = 10;
							else if (!strcmpi(month_string, "Nov")) 
								month = 11;
							else if (!strcmpi(month_string, "Dec")) 
								month = 12;
							time_check.tm_year = year - 1900;
							time_check.tm_mon  = month - 1;
							time_check.tm_mday = day;
							time_check.tm_hour = hour;
							time_check.tm_min  = minute;
							time_check.tm_sec  = second;
							time_check.tm_isdst = -1;							
							u_time = mktime(&time_check);
							if(u_time != (unsigned int)-1){
								if(u_time < pfb->last_fetch_time)
									continue;
								if(u_time > last_fetch_time){
									if(ls_news->items > 50)
										clear_news_list();
									last_fetch_time = u_time;
								}
							}
						}
					}
					p2 = strstr(p_item,"<description>");
					if(p2 < p_end){
						p2 += 13;
						p1 = strstr(p2,"</description>");
						if(p1 != NULL && p1 < p_end){
							char *c;
							
							c = s + 5000;
							for(i=0;p2 < p1;i++){
								c[i] = translate_UTF(p2,&i1);
								p2 += i1;							
							}
							c[i] = 0;
							if((p2 = strstr(c,"http://")) != NULL){							
								for(i=0;p2[i] != 0;i++){
									if(p2[i] == '\"'){										
										p2[i] = 0;
										url_profile = p2;
										p2 = NULL;
									}
								}
							}
						}
					}	
					i = add_new_new(s,url_link,url_profile,p_time,time);
					if(i < 0)
						break;
					pfb->status |= 2;
				}
			}
		}	
	}
	if(ls_news->items){
		load_all_news_avatars();
		if(timer == -1)
			timer = set_timer(&windows[menu_bar_bottom_win],1000,1,NULL);		
	}
	if(last_fetch_time > pfb->last_fetch_time)
		pfb->last_fetch_time = last_fetch_time;
	return 0;
}


