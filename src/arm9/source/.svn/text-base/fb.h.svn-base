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
 
#include <PA9.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include "fb_list.h"
#include "fb_notify.h"

#ifndef __FBH__
#define __FBH__

#define SZ_LOCAL_BUFFER			400000

//For HTTP Requests
#define FB4_RQ_GET				0
#define FB4_RQ_POST				1
#define FB4_RQ_KEEP_ALIVE		2
#define FB4_RQ_GZIP_ENCODING	4
#define FB4_RQ_MULTI_DATA		8
#define FB4_RQ_USE_HTTP11		16

//For Options
#define FBO_USELRAM_AVATAR			1
#define FBO_LOADNEWS_AVATAR			2
#define FBO_LOADFRIENDSHIP_AVATAR	4

typedef struct{
	char *name;
	char *value;	
} COOKIE,*LPCOOKIE;

typedef struct{
	char *mail;
	char *pass;
	char *name;
	char *post_form_id;
	char *c_user;
	char *channel;	
	char *dtsg;
	char *feed_url;
	char *avatar;
	char *font_name;
	int status;
	int cli_sock;
	u32 message_fetch_sequence;
	time_t last_message_time;
	time_t last_messages_download_time;
	time_t last_fetch_time;//rss feed
	COOKIE cookies[30];
	int nCookies;
	char *current_page;
	u32 flags;
} FACEBOOK,*LPFACEBOOK;

extern LPFACEBOOK pfb;
extern u8 *temp_buffer;
extern u32 temp_buffer_index;

extern int init_fb();
extern void destroy_fb();
extern int fb_login();
extern int fb_get_new_messages();
extern int fb_check_friend_requests();
extern int fb_get_buddy_list();
extern int fb_load_config();
extern int fb_get_notifications_feed();
extern int fb_send_req(int mode,char *req,char *postdata,char *host,int *sock);
extern int fb_reconnect();
extern int fb_history_fetch(char *uid);
extern int parse_response(char *str);
extern int recv_socket(int socket,char *buffer,int size);
extern int fb_get_messages_failsafe();
extern int fb_write_news(char *uid,char *msg);
extern int fb_load_avatar(char *url,unsigned short *mem,int width,int height);
extern char *fb_search_friend(char *str,int start,int *size);
extern int start_wait_anim();
extern void destroy_wait_anim();
extern int fb_upload_profile_image(char *filename);
extern int init_wait_anim();
extern int fb_get_profile_image_url(char *buffer,int flags);
extern int fb_get_profile_image(int flags);
extern int fb_get_post_form(char **p);
extern int fb_save_config();

#endif