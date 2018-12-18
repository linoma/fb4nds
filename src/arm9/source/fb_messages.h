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
 
#include "windows.h"

#ifndef __FBMSGH__
#define __FBMSGH__

typedef struct
{
	char *txt;
	char *uid;
	unsigned int id;
	time_t time;
	unsigned int retry;
	unsigned int status;
	unsigned int flags;
}FBMSG,*LPFBMSG;

extern int init_messages_list(void *mem);
extern LPFBMSG get_message_item(int index);
extern int execute_messages_loop();
extern int fb_parse_new_messages(char *buffer,unsigned int size);
extern int fb_send_news(char *txt,char *uid);

#endif