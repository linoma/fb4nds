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
#include <string.h>

#ifndef __FB_LISTH__
#define __FB_LISTH__

struct FBLIST_ITEM;

typedef struct {
	void *data;
	unsigned int size;
	struct FBLIST_ITEM *prev,*next;
} FBLIST_ITEM,*LPFBLIST_ITEM;

typedef struct{
	unsigned char *data;
	unsigned int size;
	unsigned int used;
	unsigned int items;	
	LPFBLIST_ITEM first,last,deleted;
} FBLIST,*LPFBLIST;

extern LPFBLIST init_fb_list(void *mem,unsigned int size);
extern void clear_fb_list(LPFBLIST p,int (*pfn)(void *));
extern void *get_fb_list_first(LPFBLIST p,LPFBLIST_ITEM *item);
extern void *get_fb_list_last(LPFBLIST p,LPFBLIST_ITEM *item);
extern void *get_fb_list_next(LPFBLIST p,LPFBLIST_ITEM *item);
extern void *get_fb_list_prev(LPFBLIST p,LPFBLIST_ITEM *item);
extern void *get_fb_list_item(LPFBLIST p,unsigned int index);

extern int sort_fb_list(LPFBLIST p,int (*pfn)(void *,void *));
extern LPFBLIST_ITEM add_fb_list_item(LPFBLIST p,void *data,unsigned int size);
extern int del_fb_list_item(LPFBLIST p,LPFBLIST_ITEM item);

#endif
