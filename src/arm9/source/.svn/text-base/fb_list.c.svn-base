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
 
#include "fb_list.h"
//---------------------------------------------------------------------------------
void clear_fb_list(LPFBLIST p,int (*pfn)(void *))
{
	LPFBLIST_ITEM pc;	
	int i;
	
	pc = (LPFBLIST_ITEM)p->first;
	if(pc != NULL && pfn != NULL){
		for(i=0;i<p->items;i++){		
			pfn(pc);
			pc = (LPFBLIST_ITEM)pc->next;	
		}
	}
	p->used = 0;
	p->items = 0;
	p->deleted = NULL;
	p->first = NULL;
	p->last = NULL;
	memset(p->data,0,p->size);	
}
//---------------------------------------------------------------------------------
LPFBLIST init_fb_list(void *mem,unsigned int size)
{
	unsigned int i;
	LPFBLIST p;
	
	i = (unsigned int)mem;
	p = (LPFBLIST)((i + 3) & ~3);	
	i = (unsigned int)(p+1);
	i = ((i + 3) & ~3);
	p->data = (unsigned char *)i;
	p->size = size - (i - (unsigned int)mem);
	p->first = NULL;
	clear_fb_list(p,NULL);
	return p;
}
//---------------------------------------------------------------------------------
int del_fb_list_item(LPFBLIST p,LPFBLIST_ITEM item)
{
	LPFBLIST_ITEM itm;
	
	if(p == NULL || item == NULL)
		return -1;
	{
		LPFBLIST_ITEM itm1;
		
		itm = itm1 = p->deleted;
		while(itm != NULL){
			itm1 = itm;
			itm = (LPFBLIST_ITEM)itm->next;
		}
		itm = itm1;
	}
	if(itm == NULL)
		p->deleted = itm;
	if(itm->prev != NULL)
		((LPFBLIST_ITEM)itm->prev)->next = itm->next;
	if(itm->next != NULL)
		((LPFBLIST_ITEM)itm->next)->prev = itm->prev;
	if(p->first == itm)
		p->first = (LPFBLIST_ITEM)itm->next;
	if(p->last == itm)
		p->last = (LPFBLIST_ITEM)itm->prev;
	p->items--;
	return 0;
}
//---------------------------------------------------------------------------------
LPFBLIST_ITEM add_fb_list_item(LPFBLIST p,void *data,unsigned int size)
{
	unsigned int i;
	LPFBLIST_ITEM item;

	if(p == NULL)
		return NULL;
	{
		LPFBLIST_ITEM itm;

		item = p->deleted;
		itm = NULL;
		while(item != NULL){
			if(item->size >= size){
				if(itm == NULL || item->size < itm->size)
					itm = item;
			}
			item = item->next;
		}
		if(itm == NULL){
			i = size + sizeof(FBLIST_ITEM);
			i = (i + 3) & ~3;
			if((p->size - p->used) < i)
				return NULL;
			item = (LPFBLIST_ITEM)&p->data[p->used];
			p->used += i;
		}
		else{
			i = itm->size + sizeof(FBLIST_ITEM);
			i = (i + 3) & ~3;			
			item = itm;
			if(itm == p->deleted)
				p->deleted = itm->next;
			if(itm->prev != NULL)
				((LPFBLIST_ITEM)itm->prev)->next = itm->next;
			if(itm->next != NULL)
				((LPFBLIST_ITEM)itm->next)->prev = itm->prev;
		}
	}
	memset(item,0,i);
	i = (unsigned int)item;
	i += sizeof(FBLIST_ITEM);	
	item->data = (void *)i;
	if(data != NULL)
		memcpy(item->data,data,size);
	item->size = size;
	item->prev = p->last;
	item->next = NULL;
	if(p->last != NULL)
		p->last->next = item;
	p->last = item;
	if(p->first == NULL)
		p->first = item;
	p->items++;
	return item;
}
//---------------------------------------------------------------------------------
void *get_fb_list_item(LPFBLIST p,unsigned int index)
{
	LPFBLIST_ITEM item;
	unsigned int i,len;
	
	if(p == NULL || index >= p->items)
		return NULL;
	item = (LPFBLIST_ITEM)p->data;
	for(i=0;i<index;i++){
		len = item->size + sizeof(FBLIST_ITEM);
		len = (len + 3) & ~3;
		item = (LPFBLIST_ITEM)((char *)item + len);
	}
	if(item == NULL)
		return NULL;
	return item->data;
}
//---------------------------------------------------------------------------------
void *get_fb_list_next(LPFBLIST p,LPFBLIST_ITEM *item)
{
	LPFBLIST_ITEM itm;
	
	if(p == NULL || item == NULL || *item == NULL)
		return NULL;
	itm = *item;
	itm = itm->next;
	*item = itm;
	if(itm == NULL)
		return NULL;
	return itm->data;
}
//---------------------------------------------------------------------------------
void *get_fb_list_prev(LPFBLIST p,LPFBLIST_ITEM *item)
{
	LPFBLIST_ITEM itm;
	
	if(p == NULL || item == NULL || *item == NULL)
		return NULL;
	itm = *item;
	itm = itm->prev;
	*item = itm;
	if(itm == NULL)
		return NULL;
	return itm->data;
}
//---------------------------------------------------------------------------------
void *get_fb_list_first(LPFBLIST p,LPFBLIST_ITEM *item)
{
	LPFBLIST_ITEM itm;
	
	if(p == NULL)
		return NULL;
	itm = p->first;
	if(item != NULL)
		*item = itm;
	if(itm != NULL)
		return itm->data;
	return NULL;
}
//---------------------------------------------------------------------------------
void *get_fb_list_last(LPFBLIST p,LPFBLIST_ITEM *item)
{
	LPFBLIST_ITEM itm;
	
	if(p == NULL)
		return NULL;
	itm = p->last;
	if(item != NULL)
		*item = itm;
	if(itm != NULL)
		return itm->data;
	return NULL;
}
//---------------------------------------------------------------------------------
static int sort_fnc_fb_list(void *a,void *b)
{
   LPFBLIST_ITEM p,p1;

   p = (LPFBLIST_ITEM)a;
   p1 = (LPFBLIST_ITEM)b;
   return strcmp((char *)p->data,(char *)p1->data);
}
//---------------------------------------------------------------------------------
int sort_fb_list(LPFBLIST p,int (*pfn)(void *,void *))
{
	LPFBLIST_ITEM p_i,p_i1,p_i2;
	unsigned int i,i1;

	if(p == NULL)
		return -1;
	if(pfn == NULL)
		pfn = sort_fnc_fb_list;
	p_i = (LPFBLIST_ITEM)p->first;
	if(p_i == NULL)
		return -2;
	for(i=0;i<p->items;i++){
		p_i1 = (LPFBLIST_ITEM)p_i->next;
		p_i2 = p_i;
		for(i1=i+1;i1<p->items;i1++){
            if(pfn(p_i1,p_i2) < 0)
               p_i2 = p_i1;
            p_i1 = (LPFBLIST_ITEM)p_i1->next;
		}
		if(p_i2 != p_i){
			LPFBLIST_ITEM ins_p_i2;

			if(p_i2->prev != NULL)
				((LPFBLIST_ITEM)p_i2->prev)->next = p_i2->next;
			if(p_i2->next != NULL)
				((LPFBLIST_ITEM)p_i2->next)->prev = p_i2->prev;
			else
				p->last = (LPFBLIST_ITEM)p_i2->prev;       

			if(p_i->prev != NULL)
				((LPFBLIST_ITEM)p_i->prev)->next = p_i->next;
			if(p_i->next != NULL)
				((LPFBLIST_ITEM)p_i->next)->prev = p_i->prev;
			ins_p_i2 = (LPFBLIST_ITEM)p_i->prev;
			if(ins_p_i2 == NULL){
				ins_p_i2 = (LPFBLIST_ITEM)p->first->next;

				p_i2->prev = NULL;
				p_i2->next = (struct FBLIST_ITEM *)ins_p_i2;
				ins_p_i2->prev = (struct FBLIST_ITEM *)p_i2;
				p->first = p_i2;
			}
			else{
				p_i2->prev = (struct FBLIST_ITEM *)ins_p_i2;
				p_i2->next = (struct FBLIST_ITEM *)ins_p_i2->next;
				if(p_i2->next != NULL)
					((LPFBLIST_ITEM)p_i2->next)->prev = (struct FBLIST_ITEM *)p_i2;
				ins_p_i2->next = (struct FBLIST_ITEM *)p_i2;
			}
			if(p->last != p_i){
				((LPFBLIST_ITEM)p->last)->next = (struct FBLIST_ITEM *)p_i;
				p_i->prev = (struct FBLIST_ITEM *)p->last;
				p_i->next = NULL;
				p->last = p_i;
			}
			else if(p_i2->next == NULL){
				p_i->prev = (struct FBLIST_ITEM *)p_i2;
				p_i2->next = (struct FBLIST_ITEM *)p->last;
			}
			p_i = p_i2;
		}
		p_i = (LPFBLIST_ITEM)p_i->next;
	}
	return 0;
}