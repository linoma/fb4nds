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

#ifndef __FBFRIENDSH__ 
#define __FBFRIENDSH__

typedef struct{
	char uid[40];
	char url_avatar[400];
	unsigned short *avatar;
	unsigned long flags;
} FBSEARCH_USERS,*LPFBSEARCH_USERS;

typedef struct {
	char *uid;
	char *url_avatar;
	char *name;
	char *msg;
	unsigned short *avatar;
	unsigned int size;
} FBPSHIP,*LPFBPSHIP;

int fb_friends_parse_req(char *buffer,unsigned int size);
int show_friends_notify_win();
int search_friends();
int init_pships_list(void *mem);

#endif
