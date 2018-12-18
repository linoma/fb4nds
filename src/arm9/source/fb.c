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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "fb.h"
#include "contacts.h"
#include "windows.h"
#include "fblist.h"
#include "fb_messages.h"
#include "fb_friends.h"
#include "gzip_ex.h"
#include "json.h"
#include "all_gfx.h"

extern u32 ticks;
extern int (*pfn_VBLFunc)(void);
//------------------------------------------------------------------------------------------
static u8 local_buffer[SZ_LOCAL_BUFFER+5000];
u8 *temp_buffer;
u32 temp_buffer_index;
FACEBOOK fb;
LPFACEBOOK pfb;
static u32 mem_alloc,mem_used;
static u16 wait_anim_gfx[8];
static u16 wait_anim_frame;
static int wait_anim_timer_id = -1;

static const char SSL_Certificate[] = {
	"MIIDFDCCAr6gAwIBAgIJAP5V9E5XT/LpMA0GCSqGSIb3DQEBBQUAMIGQMQswCQYD\r\n"
	"VQQGEwJJVDEPMA0GA1UECBMGSVRBTElBMRMwEQYDVQQHEwpDQU1QT0JBU1NPMRAw\r\n"
	"DgYDVQQKEwdub3RoaW5nMRAwDgYDVQQLEwdub3RoaW5nMRYwFAYDVQQDEw1MaW5v\r\n"
	"IE1hZ2xpb25lMR8wHQYJKoZIhvcNAQkBFhBsaW5vbWFAZ21haWwuY29tMB4XDTEw\r\n"
	"MDEwNTE1MTk1MloXDTEwMDIwNDE1MTk1MlowgZAxCzAJBgNVBAYTAklUMQ8wDQYD\r\n"
	"VQQIEwZJVEFMSUExEzARBgNVBAcTCkNBTVBPQkFTU08xEDAOBgNVBAoTB25vdGhp\r\n"
	"bmcxEDAOBgNVBAsTB25vdGhpbmcxFjAUBgNVBAMTDUxpbm8gTWFnbGlvbmUxHzAd\r\n"
	"BgkqhkiG9w0BCQEWEGxpbm9tYUBnbWFpbC5jb20wXDANBgkqhkiG9w0BAQEFAANL\r\n"
	"ADBIAkEAso+LbGxJDaH2VrfaXS6iOAEn1ojALxWAJaxyQJZ6nqQQsZ7x4GJFg8Ab\r\n"
	"JqgNk6L9F0uFcD4in32EklUn9jr/twIDAQABo4H4MIH1MB0GA1UdDgQWBBSG3Hat\r\n"
	"yBGYWFBSOfhTGCNZVvkGgDCBxQYDVR0jBIG9MIG6gBSG3HatyBGYWFBSOfhTGCNZ\r\n"
	"VvkGgKGBlqSBkzCBkDELMAkGA1UEBhMCSVQxDzANBgNVBAgTBklUQUxJQTETMBEG\r\n"
	"A1UEBxMKQ0FNUE9CQVNTTzEQMA4GA1UEChMHbm90aGluZzEQMA4GA1UECxMHbm90\r\n"
	"aGluZzEWMBQGA1UEAxMNTGlubyBNYWdsaW9uZTEfMB0GCSqGSIb3DQEJARYQbGlu\r\n"
	"b21hQGdtYWlsLmNvbYIJAP5V9E5XT/LpMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcN\r\n"
	"AQEFBQADQQAscwICDaThNGgebUt2ihoCY2lB9/47gmczM174VDmuiCanEZW18+P4\r\n"
	"xkepLYwOkFPhu7k162GPmbPA64QCyjNZ\r\n"
};
unsigned char fb_char_test[] = {"€,´,€,´,水,Д,Є"};
//------------------------------------------------------------------------------------------
int parse_response(char *str)
{
   char *p;

//   if(str[0] != 'H' || str[1] != 'T' || str[2] != 'T' || str[3] != 'P')
//       return -1;
   p = strchr(str,' ');   
   if(p == NULL)
       return -2;
   return atoi(p);
}
//------------------------------------------------------------------------------------------
static int move_logo_right_top(unsigned int id,unsigned int timer)
{
	s16 x,y;
	int i;
	
	i = 0;
	x = PA_GetSpriteX(1,0);
	if(x < 256 - 192){
		x++;
		i |= 1;
	}
	y = PA_GetSpriteY(1,0);
	if(y > 16){
		y--;
		i |= 2;
	}
	PA_SetSpriteXY(1,0,x,y);
	
	x = PA_GetSpriteX(1,1);
	if(x < 256 - 128){
		x++;
		i |= 4;
	}
	y = PA_GetSpriteY(1,1);
	if(y > 16){
		y--;
		i |= 8;
	}
	PA_SetSpriteXY(1,1,x,y);
	
	x = PA_GetSpriteX(1,2);
	if(x < 256-64){
		x++;
		i |= 16;
	}
	y = PA_GetSpriteY(1,2);
	if(y > 16){
		y--;
		i |= 32;
	}
	PA_SetSpriteXY(1,2,x,y);
	if(i == 0)
		destroy_timer(timer);
	return 1;
}
//------------------------------------------------------------------------------------------
int fb_load_config()
{
	FILE *fp;
	char *s,*p,*p1;
	unsigned int i,len_line;
	
	s = (char *)temp_buffer;
	fp = open_file("data/config.ini","rb");
	if(fp == NULL)
		return -1;
	fseek(fp,0,SEEK_END);
	i = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fread(s,i,1,fp);
	fclose(fp);
	s[i] = 0;
	s[i+1] = 0;
	while(*s != 0){
		p = s;
		for(len_line = 0;*p != 0;len_line++,p++){
			if(*p == '\n'){
				*p = 0;
				break;
			}
			else if(*p == 0){
				len_line--;
				break;
			}
		}
		if(len_line == 0)
			break;
		p = strtok(s,"=");
		if(p == NULL){
			s += len_line + 1;
			continue;
		}
		p1 = p + strlen(p) + 1;
		//remove the space at start
		while(*p1 == 32) p1++;
		while(*p == 32)	p++;
		//remove the space at end
		for(i=0;p[i] != 0;i++){
			if(p[i] == 32){
				p[i] = 0;
				break;
			}
		}
		for(i=0;p1[i] != 0;i++){
			if(p1[i] == 32){
				p1[i] = 0;
				break;
			}
		}

		if(!strcmpi(p,"email")){				
			for(i = 0;*p1 != 0 && *p1 != '\r';)
				pfb->mail[i++] = *p1++;			
			pfb->mail[i] = 0;		
		}
		else if(!strcmpi(p,"password")){
			for(i = 0;*p1 != 0 && *p1 != '\r';)
				pfb->pass[i++] = *p1++;			
			pfb->pass[i] = 0;
		}
		else if(!strcmpi(p,"font_name")){
			for(i = 0;*p1 != 0 && *p1 != '\r';)
				pfb->font_name[i++] = *p1++;			
			pfb->font_name[i] = 0;
		}
		else if(!strcmpi(p,"lft")){
			p = s + 8000;
			for(i = 0;*p1 != 0 && *p1 != '\r';)
				p[i++] = *p1++;			
			p[i] = 0;		
			pfb->last_fetch_time = atoi(p);
		}
		else if(!strcmpi(p,"flags")){
			p = s + 8000;
			for(i = 0;*p1 != 0 && *p1 != '\r';p1++){
				if((*p1 > 47 && *p1 < 58) || (toupper(*p1) >='A' && toupper(*p1) <= 'F'))
					p[i++] = *p1;			
			}
			p[i] = 0;				
			sscanf(p,"%X",&pfb->flags);
		}
		s += len_line + 1;
	}		
	return 0;
}
//------------------------------------------------------------------------------------------
int fb_save_config()
{
	FILE *fp;

	fp = open_file("data/config.ini","wb");
	if(fp == NULL)
		return -1;
	fprintf(fp,"email=%s\r\n",pfb->mail);
	fprintf(fp,"password=%s\r\n",pfb->pass);
	fprintf(fp,"lft=%u\r\n",(unsigned int)pfb->last_fetch_time);
	fprintf(fp,"font_name=%s\r\n",pfb->font_name);
	fprintf(fp,"flags=%X\r\n",pfb->flags);
	fclose(fp);	
	return 0;
}
//------------------------------------------------------------------------------------------
static int recalc_mem_used()
{
	int i,i1;
	
	i = 0;
	if(pfb != NULL){
		for(i1=0;i1<pfb->nCookies && i1 < 20;i1++){
			if(pfb->cookies[i1].name != NULL)
				i += strlen((char *)pfb->cookies[i1].name);
			if(pfb->cookies[i1].value != NULL)
				i += strlen((char *)pfb->cookies[i1].value);
		}
	}
	return i;	
}
//------------------------------------------------------------------------------------------
int init_wait_anim()
{
	FILE *fp;
	unsigned int size,value,count,*waiting_oamTiles,*waiting_oamPal,i;
	
	fp = open_file("data/bin/waiting_oam.bin","rb");
	if(fp == NULL){
		SetWindowText(3,"I Can not find  Wait Image files!!!");
		return -1;
	}	
	fseek(fp,4,SEEK_SET);
	fread(&count,sizeof(count),1,fp);
	for(size=0,i=count;i>0;i--){
		fread(&value,sizeof(value),1,fp);
		size += value;
		fseek(fp,sizeof(unsigned int)*2,SEEK_CUR);
	}

	waiting_oamTiles = (unsigned int *)malloc(size+10);
	if(waiting_oamTiles == NULL){
		SetWindowText(3,"I Can not find free memory!!!");
		return -2;
	}
	fseek(fp,8,SEEK_SET);
	fread(&size,sizeof(size),1,fp);
	fread(&value,sizeof(value),1,fp);
	fseek(fp,value,SEEK_SET);
	fread(waiting_oamTiles,size,1,fp);
		
	waiting_oamPal = (unsigned int *)((char *)waiting_oamTiles + size);
	fseek(fp,8+12,SEEK_SET);
	fread(&size,sizeof(size),1,fp);
	fread(&value,sizeof(value),1,fp);
	fseek(fp,value,SEEK_SET);
	fread(waiting_oamPal,size,1,fp);		

	PA_LoadSprite16cPal(1,1,(void*)waiting_oamPal);	
		
	for(i=0;i<8;i++)
		wait_anim_gfx[i] = PA_CreateGfx(1,&waiting_oamTiles[i<<7],OBJ_SIZE_32X32,0);
	PA_CreateSpriteFromGfx(1,4,wait_anim_gfx[0],OBJ_SIZE_32X32,0,1,260,110);	
	free(waiting_oamTiles);
	return 0;
}
//------------------------------------------------------------------------------------------
static int wait_anim_timer(unsigned int id,unsigned int timer)
{
	PA_SetSpriteGfx(1,4,wait_anim_gfx[wait_anim_frame++]);
	if(wait_anim_frame > 7)
		wait_anim_frame = 0;
	return 1;
}
//------------------------------------------------------------------------------------------
void destroy_wait_anim()
{
	destroy_timer(wait_anim_timer_id);
	wait_anim_timer_id = -1;
	PA_SetSpriteX (1,4,260);	
}
//------------------------------------------------------------------------------------------
int start_wait_anim()
{
	wait_anim_frame = 0;
	PA_SetSpriteX (1,4,112);	
	wait_anim_timer_id = set_timer(NULL,150,1,wait_anim_timer);
	return wait_anim_timer_id;
}
//------------------------------------------------------------------------------------------
int init_fb()
{	
	u32 i;

	memset(local_buffer,0,sizeof(local_buffer));
	pfb = (LPFACEBOOK)&fb;	
	mem_alloc = 5000;
	fb.post_form_id = (char *)&local_buffer[mem_alloc];//5000+7*300+3*1000+100000+10000+15000+10000 ((260000) - 130100)
	fb.c_user = fb.post_form_id + 300;
	fb.channel = fb.c_user + 300;	
	fb.dtsg = fb.channel + 300;
	fb.mail = fb.dtsg + 300;
	fb.pass = fb.mail + 300;
	fb.name = fb.pass + 300;
	fb.feed_url = fb.name + 300;
	fb.avatar = fb.feed_url + 1000;
	fb.font_name = fb.avatar + 1000;
	fb.flags = 0;
	i = (u32)(fb.font_name + 1000);
	i = (i + 3) & ~3;
	fb.current_page = (char *)i;
	i += 100000;
	i = (i + 3) & ~3;
	i += init_contact_list((void *)i);//15000
	i = (i + 3) & ~3;
	i += init_messages_list((void *)i);//10000
	i = (i + 3) & ~3;
	i += init_news_list((void *)i);//10000
	i = (i + 3) & ~3;
	i += init_pships_list((void *)i);//10000
	i = (i + 3) & ~3;
	temp_buffer = (u8 *)i;//start 130100
	temp_buffer_index = ((u32)temp_buffer) - ((u32)local_buffer);
	fb.message_fetch_sequence = 0;
	fb.last_fetch_time = 0;
	mem_used = (u32)recalc_mem_used();		

	return 0;
}
//------------------------------------------------------------------------------------------
void destroy_fb()
{
	if(pfb != NULL){
		if(pfb->cli_sock != 0){
			shutdown(pfb->cli_sock,SHUT_RDWR);
			closesocket(pfb->cli_sock);
		}
		pfb->cli_sock = 0;
		pfb = NULL;
	}	
}
//------------------------------------------------------------------------------------------
int recv_socket(int socket,char *buffer,int size)
{
	int i,i1,i2;
	fd_set rfds;
	struct timeval timeout;
		
	SendMessage(3,SB_SETTEXT,1,(unsigned int)"");	
	if(buffer == NULL){
		buffer = pfb->current_page;
		if(size < 1 || size > 99500)
			size = 99500;
	}	
	dmaFillWords(0,(void*)buffer,size);
	FD_ZERO(&rfds);
	FD_SET(socket, &rfds);
	timeout.tv_sec = 6;
	timeout.tv_usec = 0;
	i2 = 0;
	for(i1=0;i1<2 && i2 < size;i1++){
		i = select(socket+1, &rfds, NULL, NULL,&timeout);	
		if(i > 0){
			i = size - i2;			
			i = recv(socket,buffer,i,0);
			if(i > 0){
				i2 += i;
				buffer += i;
				i1 = -1;
				{
					char c[10];
					
					sprintf(c,"%d",i2);
					SendMessage(3,SB_SETTEXT,1,(unsigned int)c);				
				}
			}
			else if(i == 0)
				break;
		}
		else if(i == -1){
			i2 = -1;
			break;
		}
		else
			my_sleep(32);
	}			
	SendMessage(3,SB_SETTEXT,1,(unsigned int)"");
	return i2;
}
//------------------------------------------------------------------------------------------
int wait_socket(int socket,int timeout)
{
/*	int oldticks,rx;
			
	timeout >>= 4; 
	rx = 0;
	oldticks = ticks;	
	while(rx == 0){
		ioctl(socket,FIONREAD,&rx);
		swiWaitForVBlank();
		if(ticks > oldticks + timeout)
			return rx;
	}		
	return rx;*/
	return 1;
}
//------------------------------------------------------------------------------------------
int fb_send_req(int mode,char *req,char *postdata,char *host,int *sock)
{	
	struct hostent *h;
	int cli_sock;
	char *buffer,*c,*c1;
	int res,i;
	
	if(pfb == NULL)
		return 0;
	buffer = (char *)(temp_buffer + 10000);
	c = buffer + 90000;
	c1 = c + 1000;	
	res = 0;
	if(host != NULL)
		strcpy(c,host);
	else
		strcpy(c,"www.facebook.com");	
	if((h = gethostbyname(c)) == NULL)
		return 0;
	cli_sock = socket(PF_INET,SOCK_STREAM,0);
	if(cli_sock == 0)
		return 0;		
	if((mode & FB4_RQ_KEEP_ALIVE)){
		i = 1;
		if(setsockopt(cli_sock,0xFFF,8,(char *)&i,sizeof(int)))
			goto ex_fb_send_req;
	}
	{
		struct sockaddr_in srv_addr;
	
		memset(&srv_addr,0,sizeof(struct sockaddr_in));
		srv_addr.sin_family         = AF_INET;
		srv_addr.sin_addr.s_addr    = *((unsigned long *)h->h_addr_list[0]);
		srv_addr.sin_family         = AF_INET;
		srv_addr.sin_port           = htons(80);
		res = connect(cli_sock,(const struct sockaddr *)&srv_addr,sizeof(srv_addr));
	}
	if(res)
		goto ex_fb_send_req;
	if((mode & FB4_RQ_POST))
		strcpy(buffer,"POST ");
	else
		strcpy(buffer,"GET ");
	strcat(buffer,req);
	if((mode & FB4_RQ_USE_HTTP11))
		strcat(buffer," HTTP/1.1");
	else
		strcat(buffer," HTTP/1.0");
	sprintf(c1,"\r\nHost: %s\r\n",c);	
	strcat(buffer,c1);
	if((mode & FB4_RQ_KEEP_ALIVE)){
		strcat(buffer,"Keep-Alive: 300\r\n");
		strcat(buffer,"Connection: keep-alive\r\n");	
	}
	else
		strcat(buffer,"Connection: close\r\n");	
	strcat(buffer,"User-Agent: Opera/9.50 (Windows NT 5.1; U; it)\r\n");
	strcat(buffer,"Accept: */*\r\n");	
	if(mode & FB4_RQ_GZIP_ENCODING)//GZIP ENCODING
		strcat(buffer,"Accept-Encoding: gzip,deflate\r\n");	
	strcat(buffer,"Cookie: isfbe=false;");
	for(i=0;i<pfb->nCookies;i++){
		if(pfb->cookies[i].value == NULL || !strcmpi(pfb->cookies[i].value,"deleted"))
			continue;
		if(i)
			strcat(buffer,";");
		strcat(buffer," ");
		strcat(buffer,pfb->cookies[i].name);
		strcat(buffer,"=");
		strcat(buffer,pfb->cookies[i].value);
	}
	if((mode & FB4_RQ_POST)){
		if((mode & FB4_RQ_MULTI_DATA)){
			strcat(buffer,"\r\nContent-Type: multipart/form-data; boundary=");
			strcat(buffer,(char *)&postdata[4]);
			sprintf(c1,"\r\nContent-Length: %u", *((int *)postdata));			
		}
		else{
			strcat(buffer,"\r\nContent-Type: application/x-www-form-urlencoded\r\n");
			sprintf(c1,"Content-Length: %u", strlen(postdata));			
		}
		strcat(buffer,c1);
	}
	strcat(buffer,"\r\n\r\n");
	i = strlen(buffer);
	if((mode & FB4_RQ_POST) && postdata){
		if((mode & FB4_RQ_MULTI_DATA)){
			int size_data;

			size_data = *((int *)postdata);
			memcpy(&buffer[i],&postdata[strlen((char *)&postdata[4])+5],size_data);
			i += size_data;
		}
		else{
			strcat(buffer,postdata);
			i += strlen(postdata);
		}
	}
	{
		int data_send,i1,i2;
		fd_set wfds;
		struct timeval timeout;
		
		FD_ZERO(&wfds);
		FD_SET(cli_sock, &wfds);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		for(data_send = i1 = 0,res = i;data_send < i && i1 < 10;){			
			i2 = select(cli_sock+1, NULL, &wfds, NULL,&timeout);	
			if(i2 > 0){
				res = send(cli_sock,buffer,res,0);
				if(res > 0){
					data_send += res;
					buffer += res;
					res = i - data_send;
					i1 = -1;	
					sprintf(c,"%d",data_send);
					SendMessage(3,SB_SETTEXT,1,(unsigned int)c);	
				}
				else if(res == 0)
					break;
			}
			else if(i2 == -1){
				i1 = -1;
				break;
			}
			else
				my_sleep(64);		
		}
		SendMessage(3,SB_SETTEXT,1,(unsigned int)"");	
		res = data_send;
	}
	if(res == i){
		*sock = cli_sock;
		return res;
	}
ex_fb_send_req:
	if(cli_sock != 0){
		res = 0;
		shutdown(cli_sock,SHUT_RDWR);
		closesocket(cli_sock);
	}
	return res;
}
//---------------------------------------------------------------------------
int fb_history_fetch(char *uid)
{
	int i,cli_sock;
	char *s;
	
#ifdef _DEBUG
	FILE *fp;
	
	fp = open_file("data/fb_history.txt","rb");
	if(fp == NULL)
		return -1;
	fseek(fp,0,SEEK_END);
	i = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fread((void *)pfb->current_page,1,i,fp);					
	fclose(fp);	
#else	
	SetWindowText(3,"Fetching history...");
	s = (char *)temp_buffer;	
	sprintf(s,"/ajax/chat/history.php?id=%s&__a=1",uid);      //100000524697414
	i = fb_send_req(0,s,NULL,NULL,&cli_sock);
	if(!i)
		return -1;
	i = recv_socket(cli_sock,NULL,0);
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);	
#endif	
	return i;
}
//---------------------------------------------------------------------------
int fb_get_new_messages()
{
	char *s,*buffer;
	int cli_sock,i,b_reconnect;
	u32 now;
	
	//SetWindowText(3,"Getting new messages...");
	now = time(NULL);
	s = (char *)temp_buffer;
	buffer = s + 1000;
	sprintf(s,"%d.%s.facebook.com", 0, pfb->channel);
	sprintf(buffer,"/x/%lu/%s/p_%s=%d",(long unsigned int)now,"false", pfb->c_user, pfb->message_fetch_sequence);
	i = fb_send_req(2,buffer,NULL,s,&cli_sock);	
	if(!i)
		return -1;	
	pfb->last_messages_download_time = now;
	i = recv_socket(cli_sock,NULL,0);
	b_reconnect = 0;
	if(i > 0){
		char *p;
			
		p = json_get_node_value(pfb->current_page,"\"t\"",i);
		if(p != NULL){
			char *p1;

			p1 = p;
			while(*p1 != '\"' && *p1 != 0)
			    p1++;
			p1++;
			if(strncmp(p1,"msg\"",4) == 0){
				unsigned int sz;
				int res;
				
				p1 += 4;
				while(*p1 != '\"')
					p1++;
				sz = (unsigned int)(pfb->current_page + i) - (unsigned int)p1;
				res = fb_parse_new_messages(p1,sz);				
			}
			else if(strncmp(p1,"refresh\"",8) == 0){
				char *p2;

				p2 = json_get_node_value(pfb->current_page,"\"seq\"",i);
				if(p2 != 0){
					while(*p2 != '\"' && *p2 != 0)
						p2++;
					p2++;
					if(*p2 != 0)
						pfb->message_fetch_sequence = atoi(p2);
				}
				b_reconnect = 1;				
			}
		}
	}
	if(b_reconnect)
		fb_reconnect();
/*	else{
		if(cli_sock){
			shutdown(cli_sock,SHUT_RDWR);
			closesocket(cli_sock);
			cli_sock = 0;
		}
		fb_reconnect();
	}*/
	if(pfb->cli_sock){
		shutdown(pfb->cli_sock,SHUT_RDWR);
		closesocket(pfb->cli_sock);	
		pfb->cli_sock = 0;
	}
	pfb->cli_sock = cli_sock;
	return 0;
}
//------------------------------------------------------------------------------------------
int fb_reconnect()
{
	int i,cli_sock;
	char *s,*p,*p1,*p_end;
	unsigned int sz;
	
	SetWindowText(3,"Reconnecting...");
	s = (char *)temp_buffer;
	sprintf(s,"/ajax/presence/reconnect.php?reason=7&post_form_id=%s&__a=1", pfb->post_form_id);
	i = fb_send_req(0,s,NULL,NULL,&cli_sock);
	if(!i)
		return -1;
	i = recv_socket(cli_sock,NULL,0);
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);			
	if(i < 1)
		return -2;
	p = json_get_node(pfb->current_page,"\"payload\"",&sz);
	if(p == NULL)
		return -3;
	p_end = p + sz;
	p1 = json_get_node_value(p,"\"host\"",sz);
	if(p1 == NULL)
		return -4;
	while(*p1 != '\"' && p1 < p_end)
		p1++;
	if(p1 >= p_end)
		return -6;
	p1++;
	i = 0;
	while(*p1 != '\"' && p1 < p_end)
		pfb->channel[i++] = *p1++;
	pfb->channel[i] = 0;
	p1 = json_get_node_value(p,"seq",sz);
	if(p1 == NULL)
		return -7;
	pfb->message_fetch_sequence = atoi(p1);
	fb_get_new_messages();
	return 0;
}
//------------------------------------------------------------------------------------------
int fb_get_profile_image_url(char *buffer,int flags)
{
	const char *chat_user_info = "ChatUserInfos";
	char *p;
	int res;
	
	p = strstr(buffer,chat_user_info);
	res = 0;
	if(p != NULL){
		int i2;
		char *p1;
		unsigned int size;
			
		res |= 0x100;
        while(*p != '{')
            p++;
        p1 = p;
        size = 0;
        if(*p == '{'){
			i2 = 1;
            while(*p != 0){
                size++;
                if(*p == '{')
                    i2++;
                else if(*p == '}'){
                    i2--;
                    if(!i2)
                        break;
                }
                p++;
            }
		}
        p = strstr(p1,pfb->c_user);
        if(p != NULL && ((unsigned long)p - (unsigned long)p1) < size){
			p++;
			p1 = strstr(p,"\"name\"");
			if(p1 != NULL){
				res |= 0x400;
				p1 += 6;
				while(*p1 != '\"')
					p1++;
				p1++;
				i2 = 0;
				while(*p1 != '\"'){
					if(flags & 1)
						pfb->name[i2++] = *p1;
					p1++;
				}
				if(flags & 1){
					pfb->name[i2] = 0;
					res |= 16;
				}
                p1++;
                while(*p1 != 0){
                    if(*p1 == '\\'){
                        p1++;
                        if(*p1 == '/'){
                            p1 -= 6;
                            i2 = 0;
                            while(*p1 != '\"'){
                                if(*p1 != '\\')
                                    pfb->avatar[i2++] = *p1;
                                p1++;
                            }
                            pfb->avatar[i2] = 0;
							res |= 32;								
                        }
                        break;
                    }
                    p1++;
                }					
			}
		}
	}	
	return res;
}
//------------------------------------------------------------------------------------------
int fb_get_profile_image(int flags)
{
	char *prot,*host,*u,*s;
	int cli_sock,i;
	
	prot = strtok(pfb->avatar,"/");
	host = prot + strlen(prot) + 2;
	host = strtok(host,"/");
	u = host + strlen(host)+1;
	s = (char *)temp_buffer;
	s[0] = '/';
	s[1] = 0;
	strcat(s,u);
	i = fb_send_req(0,s,NULL,host,&cli_sock);
	if(i > 0){
		PA_DeleteSprite(1,3);	
		i = recv_socket(cli_sock,NULL,0);
		if(parse_response(pfb->current_page) == 200){
			host = NULL;
			u = pfb->current_page;
			while(*u != 0){
				if(*u == '\r'){
					if(u[1] == '\n' && u[2] == '\r' && u[3] == '\n'){
						host = u + 4;
						break;
					}
				}
				u++;
			}
			if(host != NULL){
				u8 *sprite;
				
				if(flags & 1)
					set_timer(NULL,100,1,move_logo_right_top);
				sprite = malloc(64*64*4);
				if(sprite != NULL){					
					if(!stretch_blt((u16 *)sprite,0,0,64,64,64,(u8*)host)){						
						PA_Create16bitSprite(1,3,(void *)sprite,OBJ_SIZE_64X64,0,16);												
						PA_SetSpritePrio(1,3,3);
					}
					free(sprite);
				}
				invalidate_windows_screen(1,1);
			}					
		}
		shutdown(cli_sock,SHUT_RDWR);
		closesocket(cli_sock);
		return 0;
	}
	return -1;
}
//------------------------------------------------------------------------------------------
int fb_get_post_form(char **p)
{
	char *s,*buffer;
	int i,cli_sock;
	
	s = (char *)temp_buffer;
	i = fb_send_req(FB4_RQ_GZIP_ENCODING,"/presence/popout.php",NULL,NULL,&cli_sock);
	if(i <= 0)
		return -1;
	i = recv_socket(cli_sock,NULL,0);	
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
    if(i <= 0)
		return -1;
	buffer = s + 25000;
	{
		int i1;

		i1 = gz_expand_response((char *)pfb->current_page,buffer,i);		
		if(i1 < 0)
			buffer = (char *)pfb->current_page;	
		else
			i = i1;					
	}
	if(p != NULL)
		*p = buffer;
	return i;
}
//------------------------------------------------------------------------------------------
int fb_get_post_form_id()
{
	const char *start_text = "id=\"post_form_id\" name=\"post_form_id\" value=\"";
	const char *dtsg_start = "fb_dtsg:\"";
	const char *channel_start = "js\", \"channel";
	const char *channel_start2 = "js\",\"channel";	
	int i,res;
	char *p,*s,*buffer;

	SetWindowText(3,"Getting Profile Info...");
	i = fb_get_post_form(&buffer);
	if(i <= 0)
		return -1;
	s = (char *)temp_buffer;
	res = 0;

	p = strstr(buffer,start_text);
    if(p != NULL){
		int i2;

        p += strlen(start_text);
        for(i2=0;*p != '\"';i2++)
            pfb->post_form_id[i2] = *p++;
        pfb->post_form_id[i2] = 0;
		res |= 1;
	}
	p = strstr(buffer,dtsg_start);
	if(p != NULL){
        int i2;

        p += strlen(dtsg_start);
        for(i2=0;*p != '\"';i2++)
			pfb->dtsg[i2] = *p++;
        pfb->dtsg[i2] = 0;
        res |= 2;
    }
    p = strstr(buffer,channel_start);
    if(p != NULL){
        int i2;

        p += 6;
        for(i2=0;*p != '\"';i2++)
			pfb->channel[i2] = *p++;
        pfb->channel[i2] = 0;
        res |= 4;
    }
    if(!(res & 4)){
        p = strstr(buffer,channel_start2);
        if(p != NULL){
			int i2;
                           
            p += 5;
            for(i2=0;*p !='\"';i2++)
				pfb->channel[i2] = *p++;
            pfb->channel[i2] = 0;
            res |= 4;
        }
    }
	if((res & 1) == 0)
		return -2;
	s[0] = 0;
	if(res)		
		res |= fb_get_profile_image_url(buffer,1);
	SetWindowText(3,"Getting Profile Info 2...");
	if(res & 16)
		invalidate_windows_screen(1,1);
	if(res & 32)
		fb_get_profile_image(1);
	
	SetWindowText(3,"Getting Profile Info 3...");
	sprintf(s,"visibility=true&post_form_id=%s",pfb->post_form_id);
	{
		int cli_sock;
		
		i = fb_send_req(1,"/ajax/chat/settings.php",s,"apps.facebook.com",&cli_sock);
		if(i != 0)
			recv_socket(cli_sock,NULL,0);
		shutdown(cli_sock,SHUT_RDWR);
		closesocket(cli_sock);	
	}
	if(!(res & 4))
		fb_reconnect();
	else
		fb_get_new_messages();
	return 0;
}
//---------------------------------------------------------------------------
int fb_get_messages_failsafe()
{
	if((pfb->last_messages_download_time + 300) < time(NULL))
		return fb_get_post_form_id();
	return 0;
}
//---------------------------------------------------------------------------
int fb_get_buddy_list()
{
	char *s,*buffer;
	int cli_sock,i;

	SetWindowText(3,"Getting Buddy List Info...");
	s = (char *)temp_buffer;
	buffer = pfb->current_page;
	sprintf(s,"user=%s&popped_out=true&force_render=true&buddy_list=1&__a=1&post_form_id_source=AsyncRequest&post_form_id=%s&fb_dtsg=%s&notifications=1",
		pfb->c_user,pfb->post_form_id,pfb->dtsg);
	i = fb_send_req(FB4_RQ_POST|FB4_RQ_GZIP_ENCODING,"/ajax/presence/update.php",s,NULL,&cli_sock);
	if(i == 0)
		return -1;
	
	i = recv_socket(cli_sock,NULL,0);
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
	if(i){
		int i1;		
		
		s = (char *)(temp_buffer + 25000);		
		i1 = gz_expand_response((char *)pfb->current_page,s,i);		
		fb_buddy_list_parse(s,(unsigned int)i);
		update_contacts_list();
	}
	sprintf(s,"/ajax/intent.php?filter=app_2915120374&request_type=1&__a=1&newest=%d&ignore_self=true",0);
	i = fb_send_req(FB4_RQ_GZIP_ENCODING,s,NULL,NULL,&cli_sock);
	if(i == 0)
		return -2;
	recv_socket(cli_sock,NULL,0);
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
	return 0;
}
//------------------------------------------------------------------------------------------
static char *alloc_buffer(int len)
{
	u8 *p;
	
	if(local_buffer == NULL || mem_used > mem_alloc || (mem_alloc - mem_used) < len)
		return NULL;
	p = &local_buffer[mem_used];
	memset(p,0,len);
	mem_used += len;
	return (char *)p;
}
//------------------------------------------------------------------------------------------
static int search_cookie(char *str)
{
	int i;
	
	if(str == NULL || pfb == NULL)
		return 0;
	for(i=0;i<pfb->nCookies;i++){
		if(pfb->cookies[i].name != NULL){
			if(!strcmp(str,(char *)pfb->cookies[i].name))
				return i;
		}
	}
	return -1;
}
//------------------------------------------------------------------------------------------
static int add_cookie(char *str)
{
	int i1,len,ck;
	char *p1,*p2;
	
	if(str == NULL || local_buffer == NULL)
		return 0;
	p1 = str;
    for(i1=len=0;*p1 != '=';i1++,p1++)
		len++;
	p1 = str;
	p2 = (char *)&local_buffer[mem_alloc - 1 - len - 1];
	memset(p2,0,len+1);
	for(i1=0;i1<len;i1++)
		p2[i1] = *p1++;
	ck = search_cookie(p2);
	if(ck < 0){
		ck = pfb->nCookies;
		pfb->cookies[ck].name = (char *)alloc_buffer((len+1)*2);				
		strcpy((char *)pfb->cookies[ck].name,p2);
	}
    p1++;
	str = p1;
    for(i1=len=0;*p1 != ';';i1++,p1++)
        len++;
	p2 = (char *)&local_buffer[mem_alloc - 1 - len - 1];
	memset(p2,0,len+1);
	p1 = str;
	for(i1=0;i1<len;i1++)
		p2[i1] = *p1++;	
	if(ck == pfb->nCookies)
		pfb->cookies[pfb->nCookies++].value = (char *)alloc_buffer((len+1) * 2);		
	strcpy((char *)pfb->cookies[ck].value,p2);
	return 1;
}
//------------------------------------------------------------------------------------------
int parse_cookie(char *str)
{
	char *p,*p1;
	int res;

	res = 0;
	p1 = str;
	while((p = strstr(p1,"\r\n")) != NULL){
		*p = 0;
		if(strstr(p1,"Set-Cookie") != NULL){
			p1 = strchr(p1+10,':');
			if(p1 != NULL){
				p1++;
				while(!isalnum((int)*p1) && *p1)
					p1++;
				if(!*p1)
					break;
				add_cookie(p1);
				res = 1;
			}
		}
		*p = '\r';
		p1 = p + 2;
	}
	return res;
}
//---------------------------------------------------------------------------
int fb_check_friend_requests()
{
	int cli_sock,i;

	SetWindowText(3,"Checking friend request...");
	i = fb_send_req(FB4_RQ_GZIP_ENCODING,"/reqs.php",NULL,NULL,&cli_sock);
	if(!i)
		return -1;
	i = recv_socket(cli_sock,NULL,0);
	if(i > 0){
		char *s;
		int i1;
		
		s = (char *)(temp_buffer + 25000);
		i1 = gz_expand_response((char *)pfb->current_page,s,i);
		if(i1 < 0)
			s = (char *)pfb->current_page;
		else
			i = i1;
		fb_friends_parse_req(s,(unsigned int)i);
		i = 0;
	}
	else
		i = -2;
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
	return i;
}
//---------------------------------------------------------------------------
int fb_get_notifications_feed()
{
	const char search_string[] = {"/feeds/notifications.php"};
    char *p;	
    int cli_sock,i;
           
	if(pfb->feed_url[0] == 0){
		SetWindowText(3,"Getting Notifications Feed URL...");
		i = fb_send_req(FB4_RQ_GZIP_ENCODING,"/notifications.php",NULL,NULL,&cli_sock);
        if(i == 0)
			return -1;
		i = recv_socket(cli_sock,NULL,0);
		shutdown(cli_sock,SHUT_RDWR);
		closesocket(cli_sock);
		cli_sock = 0;
        if(i == 0)
			return -2;
		p = (char *)(temp_buffer + 25000);
		cli_sock = gz_expand_response((char *)pfb->current_page,p,i);
		if(cli_sock < 0)
			p = (char *)pfb->current_page;
		p = strstr(p,search_string);
        if(p == NULL)
			return -3;
        i = 0;
        while(*p != 0 && *p != '\"'){
            if(*p == '&'){
                if(strnicmp(p,"&amp;",5) == 0){
                    pfb->feed_url[i++] = '&';
                    p += 5;
                }
            }
            else
                pfb->feed_url[i++] = *p++;
        }
        pfb->feed_url[i]=0;
	}
	if(pfb->feed_url[0] == 0)
		return -4;
	SetWindowText(3,"Getting Notifications Feed...");
	i = fb_send_req(FB4_RQ_GZIP_ENCODING,pfb->feed_url,NULL,NULL,&cli_sock);
	if(i == 0)
		return -5;
	i = recv_socket(cli_sock,NULL,0);
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
	if(i < 1)
		return -6;	
	SetWindowText(3,"Parsing Notifications Feed...");
	{
		int i1;
				
		p = (char *)(temp_buffer + 25000);
		i1 = gz_expand_response((char *)pfb->current_page,p,i);
		if(i1 < 0)
			p = (char *)pfb->current_page;	
		else
			i = i1;
	}
	fb_parse_notify(p,i);
	return 0;
}
//---------------------------------------------------------------------------
int fb_write_news(char *uid,char *msg)
{
	int i,cli_sock;
	char *postdata,*s,*c1;

	postdata = (char *)temp_buffer;
	s = postdata + 4000;
	c1 = s + 2000;
	
	encode_string((char *)fb_char_test,s);
	strcpy(postdata,"charset_test=");
	strcat(postdata,s);
	strcat(postdata,"&fb_dtsg=");
	strcat(postdata,pfb->dtsg);
	strcat(postdata,"&profile_id=");
	strcat(postdata,pfb->c_user);
	strcat(postdata,"&target_id=");
	strcat(postdata,uid);
	strcat(postdata,"&post_form_id=");
	strcat(postdata,pfb->post_form_id);
	strcat(postdata,"&status=");
	encode_string(msg,s);
	strcat(postdata,s);
	i = fb_send_req(1,"/ajax/updatestatus.php",postdata,NULL,&cli_sock);
	if(i == 0)
        return -1;
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
	return 0;
}
//------------------------------------------------------------------------------------------
int fb_login()
{	
	struct sockaddr_in srv_addr;
	struct hostent *h;
	char *req,*post_data,*s;
	int i,res,cli_sock;
	SSL_METHOD*  method;
	SSL_CTX*     ctx;
	SSL*         ssl;
	
	if(pfb == NULL)
		return -1;	
	start_wait_anim();
	
	pfb->status = 0;
	i = fb_send_req(0,"/index.php",NULL,NULL,&cli_sock);
	ctx     = 0;
	ssl     = 0;
	method 	= 0;
	res = -1;
	if(i == 0)
		goto ex_login_fb;
	req = (char *)temp_buffer;	
	res = -2;	
	i = recv_socket(cli_sock,req,5000);
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
	cli_sock = 0;
	if(i < 1000)
		goto ex_login_fb;
	res = -3;
	i = parse_response(req);
	if(i < 200)
		goto ex_login_fb;
	SetWindowText(3,"Authenticating...");
	res = -4;
	parse_cookie(req);
	cli_sock = socket(AF_INET,SOCK_STREAM,0);
	if(cli_sock == 0)
		goto ex_login_fb;
	res = -5;
	method  = TLSv1_client_method();
	if(method == NULL)
		goto ex_login_fb;
	res = -6;
	ctx = SSL_CTX_new(method);
	if(ctx == NULL)
		goto ex_login_fb;
	res = -7;
	if(!SSL_CTX_load_verify_locations(ctx, SSL_Certificate, 0))
		goto ex_login_fb;
	res = -8;
	ssl = SSL_new(ctx);
	if(ssl == NULL)
		goto ex_login_fb;
	res = -9;
	h = gethostbyname("login.facebook.com");
	if(h == NULL)
		goto ex_login_fb;
	memset(&srv_addr,0,sizeof(srv_addr));
	srv_addr.sin_addr.s_addr    = *((unsigned long *)h->h_addr_list[0]);
	srv_addr.sin_family         = AF_INET;
	srv_addr.sin_port           = htons(443);
	res = -10;
	if(connect(cli_sock,(struct sockaddr *)&srv_addr,sizeof(srv_addr)))
		goto ex_login_fb;	
	SSL_set_fd(ssl, cli_sock);
	SSL_connect(ssl);
	memset(req,0,4000);
	post_data = req + 2000;	
	s = post_data + 1500;
	encode_string((char *)fb_char_test,s);
    strcpy(post_data,"charset_test=");
    strcat(post_data,s);
    strcat(post_data,"&locale=it_IT");
	if(pfb->mail[0] != 0){
		encode_string(pfb->mail,req);
		sprintf(req+1000,"&email=%s",req);
		strcat(post_data,req+1000);
	}
	if(pfb->pass[0] != 0){
		encode_string(pfb->pass,req);
		sprintf(req+1000,"&pass=%s",req);
		strcat(post_data,req+1000);
	}		
    strcat(post_data,"&pass_placeHolder=Password");
    strcat(post_data,"&version=1.0");
    strcat(post_data,"&persistent=1");
    strcat(post_data,"&login=Login");
    strcat(post_data,"&charset_test=");
    strcat(post_data,s);
    strcat(post_data,"&lsd=");
	for(i=0;i<pfb->nCookies;i++){
		if(!strcmp((char *)pfb->cookies[i].name,"lsd")){
           strcat(post_data,(char *)pfb->cookies[i].value);
           break;
		}
	}
	strcpy(req,"POST /login.php?login_attempt=1&_fb_noscript=1 HTTP/1.0\r\n");
	strcat(req,"Host: www.facebook.com\r\n");
	strcat(req,"Connection: close\r\n");
	strcat(req,"User-Agent: Opera/9.50 (Windows NT 5.1; U; it)\r\n");
	strcat(req,"Content-Type: application/x-www-form-urlencoded\r\n");
	sprintf(s,"Content-Length: %u\r\n", strlen(post_data));
	strcat(req,s);
	strcat(req,"Accept: */*\r\n");
	strcat(req,"Cookie: isfbe=false;");
	for(i=0;i<pfb->nCookies;i++){
		if(i)
			strcat(req,";");
		strcat(req," ");
		strcat(req,(char *)pfb->cookies[i].name);
		strcat(req,"=");
		strcat(req,(char *)pfb->cookies[i].value);
	}
	strcat(req,"\r\n");
	strcat(req,"\r\n");
	strcat(req,post_data);
	i = SSL_write(ssl,req,strlen(req));
	res = -11;
	if(i != strlen(req))
		goto ex_login_fb;
	i = SSL_read(ssl,req,5000);
	if(!i)
		i = SSL_read(ssl,req,5000);		
	if(ssl){
		SSL_shutdown(ssl);
		SSL_shutdown(ssl);
		SSL_free(ssl);
		ssl = NULL;
	}
	if(ctx){
		SSL_CTX_free(ctx);		
		ctx = NULL;
	}
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);
	cli_sock = 0;
	res = -12;
	if(strstr(req,"c_user") == NULL)
		goto ex_login_fb;	
	for(i=0;i<pfb->nCookies;i++)
		pfb->cookies[i].name = pfb->cookies[i].value = NULL;
	pfb->nCookies = 0;
	mem_used = (u32)recalc_mem_used();
	parse_cookie(req);
	for(i=0;i<pfb->nCookies;i++){
		if(!strcmp(pfb->cookies[i].name,"c_user")){
			strcpy(pfb->c_user,pfb->cookies[i].value);
			break;
		}
	}	
/*	if(parse_response(req) == 302){
		char *p;
				
		p = strstr(req,"Location:");
		if(p != NULL){
			p += 9;
			while(*p != ' ')
				p++;
			i = 0;
			while(*p != 0 && *p != '\r')
				s[i++] = *p++;
			s[i] = 0;
			SetWindowText(3,"Redirecting....");
			i = fb_send_req(0,s,NULL,NULL,&cli_sock);
			if(i){
				wait_socket(cli_sock,20000);
				recv_socket(cli_sock,req,5000);
				if(parse_response(req) == 200)
					parse_cookie(req);
				shutdown(cli_sock,SHUT_RDWR);
				closesocket(cli_sock);
				cli_sock = 0;
			}
		}
	}*/
	res = -14;
	if(fb_get_post_form_id())
		goto ex_login_fb;
	fb_check_friend_requests();
	res = -15;		
	fb_get_buddy_list();   	
	SetWindowText(3,"Connected");
	pfb->status |= 1;
	fb_save_config();
	res = 0;
ex_login_fb:
	if(ssl){
		SSL_shutdown(ssl);
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}
	if(ctx)
		SSL_CTX_free(ctx);		
	if(cli_sock != 0){
		shutdown(cli_sock,SHUT_RDWR);
		closesocket(cli_sock);
	}
	destroy_wait_anim();
	if(res){
		sprintf((char *)temp_buffer,"Login failed!!!! Err. : %d",res);
		SetWindowText(3,(char *)temp_buffer);
	}
	return res;	
}
//------------------------------------------------------------------------------------------
char *fb_search_friend(char *str,int start,int *size)
{
	char *s;
	int i,cli_sock,i1;
	
	if(str == NULL)
		return NULL;							
	SetWindowText(3,"Searching friends...");
	s = (char *)temp_buffer;
	sprintf(s,"/ajax/search/results.php?__a=1&flt=1&o=2048&q=%s&s=%d&post_form_id=%s&user=%s",str,start,pfb->post_form_id,pfb->c_user);				
	i = fb_send_req(FB4_RQ_GZIP_ENCODING,s,NULL,NULL,&cli_sock);		
	if(i <= 0)
		return NULL;
	i = recv_socket(cli_sock,NULL,0);
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);				
	if(i <= 0)
		return NULL;
	s += 5000;
	i1 = gz_expand_response((char *)pfb->current_page,s,i);
	if(i1 < 0){
		s = (char *)pfb->current_page;	
		if(parse_response(s) != 200)
			return NULL;
	}
	else
		i = i1;
	if(size != NULL)
		*size = i;
	return s;				
}
//------------------------------------------------------------------------------------------
int fb_load_avatar(char *url,unsigned short *mem,int width,int height)
{	
	char *prot,*host,*u,*s;
	int cli_sock,i,res;	
	
	if(url == NULL || url[0] == 0 || mem == NULL)
		return -1;	
	s = (char *)(temp_buffer+5000);
	u = s + 5000;
	strcpy(u,url);
	prot = strtok(u,"/");
	host = prot + strlen(prot) + 2;
	host = strtok(host,"/");
	u = host + strlen(host)+1;
	s[0] = '/';
	s[1] = 0;
	strcat(s,u);
	i = fb_send_req(0,s,NULL,host,&cli_sock);		
	if(i <= 0)
		return -2;
	res = -3;
	i = recv_socket(cli_sock,NULL,0);
	shutdown(cli_sock,SHUT_RDWR);
	closesocket(cli_sock);				
	if(i > 0 && parse_response(pfb->current_page) == 200){
		host = NULL;
		u = pfb->current_page;
		while(*u != 0){
			if(*u == '\r'){
				if(u[1] == '\n' && u[2] == '\r' && u[3] == '\n'){
					host = u + 4;
					break;
				}
			}
			u++;
		}
		if(host != NULL){
			if(!stretch_blt((u16 *)mem,0,0,width,height,width,(u8*)host))
				res = 0;
		}
	}
	return res;
}
//------------------------------------------------------------------------------------------
int fb_upload_profile_image(char *filename)
{
	unsigned int size;
	int cli_sock,i;
	char *postdata,*boundary,*s,*file_name,*mime_type;
	FILE *fp;
	
	if(filename == NULL || filename[0] == 0)
		return -1;
	file_name = strrchr(filename,'/');
	if(file_name == NULL)
		return -2;
	file_name++;
	mime_type = strrchr(file_name,'.');
	if(mime_type == NULL)
		return -3;		
	mime_type++;
	s = (char *)temp_buffer;
	if(stricmp(mime_type,"jpg") == 0 || stricmp(mime_type,"jpeg") == 0)
		strcpy(s,"image/jpeg");
	else if(stricmp(mime_type,"gif") == 0)
		strcpy(s,"image/gif");
	else if(stricmp(mime_type,"png") == 0)
		strcpy(s,"image/png");
	else if(stricmp(mime_type,"bmp") == 0)
		strcpy(s,"image/bmp");
	else
		return -4;
	fp = fopen(filename,"rb");
	if(fp == NULL)
		return -5;
	start_wait_anim();
	boundary = (char *)&pfb->current_page[4];
	sprintf(boundary,"---------------------------%04d%03d%04d",PA_RandMax(32768),PA_RandMax(32768),PA_RandMax(32768));
	postdata = boundary + strlen(boundary) + 1;
	
	strcpy(postdata,"--");
	strcat(postdata,boundary);
	strcat(postdata,"\r\n");
	strcat(postdata,"Content-Disposition: form-data; name=\"post_form_id\"\r\n\r\n");
	strcat(postdata,pfb->post_form_id);
	strcat(postdata,"\r\n");
	
	strcat(postdata,"--");
	strcat(postdata,boundary);
	strcat(postdata,"\r\n");
	strcat(postdata,"Content-Disposition: form-data; name=\"fb_dtsg\"\r\n\r\n");
	strcat(postdata,pfb->dtsg);
	strcat(postdata,"\r\n");
	
	strcat(postdata,"--");
	strcat(postdata,boundary);
	strcat(postdata,"\r\n");
	strcat(postdata,"Content-Disposition: form-data; name=\"id\"\r\n\r\n");
	strcat(postdata,pfb->c_user);
	strcat(postdata,"\r\n");

	strcat(postdata,"--");
	strcat(postdata,boundary);
	strcat(postdata,"\r\n");
	strcat(postdata,"Content-Disposition: form-data; name=\"type\"\r\n\r\n");
	strcat(postdata,"profile");
	strcat(postdata,"\r\n");
	
	strcat(postdata,"--");
	strcat(postdata,boundary);
	strcat(postdata,"\r\n");
	strcat(postdata,"Content-Disposition: form-data; name=\"pic\"; filename=\"");
	strcat(postdata,file_name);
	strcat(postdata,"\"\r\n");
	strcat(postdata,"Content-Type: ");
	strcat(postdata,s);
	strcat(postdata,"\r\n\r\n");	
	size = strlen(postdata);	
	{
		unsigned long fp_size;

		fseek(fp,0,SEEK_END);
		fp_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		if((fp_size + size) > 88000)
			fp_size = 88000 - size;
		fread(&postdata[size],fp_size,1,fp);
		fclose(fp);
		size += fp_size;
	}
    
	strcpy(s,"\r\n--");
    strcat(s,boundary);
    strcat(s,"\r\n");
    strcat(s,"Content-Disposition: form-data; name=\"pic\"\r\n\r\n");
    strcat(s,"Sfoglia");
    strcat(s,"\r\n");
    strcat(s,"--");
    strcat(s,boundary);
    strcat(s,"--\r\n");
    strcat(&postdata[size],s);
    size += strlen(s);
	*((unsigned int *)(boundary - 4)) = size;

	i = fb_send_req(FB4_RQ_POST|FB4_RQ_MULTI_DATA|FB4_RQ_USE_HTTP11,"/pic_upload.php",pfb->current_page,"upload.facebook.com",&cli_sock);
	if(i != 0){
		i = recv_socket(cli_sock,NULL,0);
		if(i > 0){
			char *p;
			
			i = 1;		
			p = strstr(pfb->current_page,"success=");
			if(p != NULL)
				i = atoi(p+8);
		}
		else
			i = 1;
		shutdown(cli_sock,SHUT_RDWR);
		closesocket(cli_sock);
	}
	destroy_wait_anim();
	return i;
}
