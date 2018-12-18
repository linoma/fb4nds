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

#include "fb_messages.h"
#include "fb.h"
#include "utils.h"
#include "contacts.h"
#include "json.h"
#include "chat_win.h"

LPFBLIST ls_messages;
static char *local_buffer;
//---------------------------------------------------------------------------------
int init_messages_list(void *mem)
{
	ls_messages = init_fb_list(mem,5000);
	local_buffer = (char *)mem;
	local_buffer += 5000;
	return 10000;
}
//---------------------------------------------------------------------------------
static LPFBMSG add_message(LPFBLIST list,char *txt,char *uid)
{
	LPFBMSG p;
	int size,len,len_uid;
	
	if(list == NULL || txt == NULL || uid == NULL || txt[0] == 0 || uid[0] == 0)
		return NULL;
	size = sizeof(FBMSG) + (len = strlen(txt)) + 1 + (len_uid = strlen(uid)) + 1;	
	size = (size + 3) & ~3;
	{
		LPFBLIST_ITEM item;
		
		item = add_fb_list_item(list,NULL,size);
		if(item == NULL)
			return NULL;
		p = (LPFBMSG)item->data;
	}
	p->txt = (char *)(p+1);
	strcpy(p->txt,txt);
	p->uid = p->txt + len + 1;
	strcpy(p->uid,uid);
	p->status = 0;
	p->retry = 0;
	p->id = rand();
	p->time = time(NULL);
	p->flags = 0;
	return p;		
}
//---------------------------------------------------------------------------------
static void rebuild_messages_list()
{
	LPFBLIST list;
	LPFBMSG p;
	LPFBLIST_ITEM item;
	unsigned long cr;
	
	EnterCriticalSection(&cr);
	list = init_fb_list(local_buffer,5000);
	p = (LPFBMSG)get_fb_list_first(ls_messages,&item);
	while(p != NULL){
		if(p->status != 3)
			add_message(list,p->txt,p->uid);
		p = (LPFBMSG)get_fb_list_next(ls_messages,&item);
	}	
	clear_fb_list(ls_messages,NULL);
	p = (LPFBMSG)get_fb_list_first(list,&item);
	while(p != NULL){
		add_message(ls_messages,p->txt,p->uid);
		p = (LPFBMSG)get_fb_list_next(list,&item);
	}		
	LeaveCriticalSection(&cr);
}
//---------------------------------------------------------------------------------
LPFBMSG get_message_item(int index)
{
	return get_fb_list_item(ls_messages,index);
}
//---------------------------------------------------------------------------------
static int send_message(LPFBMSG p)
{
	if(p == NULL)
		return -1;
	p->status = 2;
	if(p->flags == 0){
		char *jstime,*s;
		int cli_sock;

		jstime = local_buffer;
		s = jstime + 500;
		sprintf(jstime,"%ld%ld",(long int)p->time,(long int)0);
		sprintf(s,"msg_text=%s&msg_id=%d&to=%s&client_time=%s&post_form_id=%s",
			p->txt,p->id,p->uid,jstime,pfb->post_form_id);
		if(fb_send_req(1,"/ajax/chat/send.php",s,NULL,&cli_sock)){
			p->status = 3;
			shutdown(cli_sock,SHUT_RDWR);
			closesocket(cli_sock);			
		}
		else	
			p->status = 1;
	}
	else{
		if(!fb_write_news(p->uid,p->txt))
			p->status = 3;
		else
			p->status = 1;
	}
	return 0;
}
//---------------------------------------------------------------------------------
int fb_send_news(char *txt,char *uid)
{
	LPFBMSG p;
	
	p = add_message(ls_messages,txt,uid);
	if(p == NULL)
		return -1;		
	p->status = 1;
	p->flags = 1;
	return 0;
}
//---------------------------------------------------------------------------------
int fb_send_message(char *txt,char *uid)
{
	LPFBMSG p;
	
	p = add_message(ls_messages,txt,uid);
	if(p == NULL)
		return -1;		
	p->status = 1;
	return 0;
}
//---------------------------------------------------------------------------------
int execute_messages_loop()
{
	LPFBLIST_ITEM item;
	LPFBMSG p;
	
	SetWindowText(3,"Sending messages....");
	p = (LPFBMSG)get_fb_list_first(ls_messages,&item);
	while(p != NULL){
		if(p->status == 1)
			send_message(p);
		p = (LPFBMSG)get_fb_list_next(ls_messages,&item);
	}
	SetWindowText(3,"Rebuilding messages....");
	rebuild_messages_list();	
	return 1;
}
//---------------------------------------------------------------------------------
int fb_parse_new_messages(char *buffer,unsigned int size)
{
	char *p,*p_end,*p1,*p2,*p_to,*p_from;
	int i2;
	unsigned int sz;

#ifdef _DEBUG
	FILE *fp;
	
	static int parsed = 0;
	
	if(parsed)
		return 0;
	fp = open_file("data/fb_msg.txt","rb");
	if(fp != NULL){		
		fseek(fp,0,SEEK_END);
		size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		fread((void *)pfb->current_page,1,size,fp);					
		p = json_get_node_value(pfb->current_page,"\"t\"",size);
		if(p != NULL){
			p1 = p;
			while(*p1 != '\"' && *p1 != 0)
			    p1++;
			p1++;
			if(strncmp(p1,"msg\"",4) == 0){				
				p1 += 4;
				while(*p1 != '\"')
					p1++;
				size = (unsigned int)(pfb->current_page + size) - (unsigned int)p1;
				buffer = p1;
				parsed = 1;
			}		
		}
		fclose(fp);	
	}
#endif	
	if(buffer == NULL || size == 0)
		return -1;
	
	p_end = buffer + size;
	p = buffer;
	do{
		p1 = json_get_node_value(p,"\"type\"",size);
		if(p1 == NULL)
			break;
		while(p1 < p_end && *p1 != '\"')
			p1++;
		if(p1 >= p_end){
			p = p1;
			break;
		}
		p1++;
		i2 = 1;
		p2 = p1;
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
		if(p2 > p_end){
			p = p_end;
			break;
		}
		sz = (unsigned int)p2 - (unsigned int)p1;
		if(strncmp(p1,"msg\"",4) == 0){
			p = json_get_node_value(p1,"\"text\"",sz);
			p_to = json_get_node_value(p1,"\"to\"",sz);
			p_from = json_get_node_value(p1,"\"from\"",sz);
			if(p != NULL && p_to != NULL && p_from != NULL){				
				LPFBCONTACT pfbc;
								
				i2 = 0;
				while(p_from[i2] != ',')
					i2++;
				p_from[i2] = 0;
				pfbc = find_contact_from_id(p_from);
				if(pfbc != NULL){
/*					while(*p != '\"')
						p++;
					p++;
					i2 = 0;
					while(p[i2] != '\"')
						i2++;
					p[i2] = 0;*/
					if(find_chat_win_uid(&windows[chat_tab_win],p_from) == NULL){
						int item;
						
						item = SendMessage(chat_tab_win,TCM_ADDSTRING,0,(LPARAM)pfbc->name);//pfbc->name
						if(item > 0)
							*((unsigned int *)item) = (unsigned int)pfbc;
					}
					pfbc->status |= 0x80000000;
				}
			}			
		}
		else if(strncmp(p1,"typ\"",4) == 0){

		}
		pfb->message_fetch_sequence++;
		p = p2;
	}while(p < p_end);
	return ((unsigned int)p - (unsigned int)buffer);
}