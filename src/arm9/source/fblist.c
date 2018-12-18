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

#include "fblist.h"
#include "json.h"
#include "contacts.h"
#include "utils.h"
#include "fb.h"

//---------------------------------------------------------------------------------
int fb_buddy_list_parse(char *buffer,unsigned int size)
{
	char *p,*p1,*p2,*p3,*p4,*p_end,*uid,*name,*c,*avatar_url;
	unsigned int sz,sz_1,sz_2,sz_3,sz_4;
	int i,i_b,i1;
	
#ifdef _DEBUG
	FILE *fp;
	int count;
	
	fp = open_file("data/fb_update.txt","rb");
	if(fp != NULL){		
		fseek(fp,0,SEEK_END);
		size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		fread((void *)pfb->current_page,1,size,fp);			
		buffer = (char *)pfb->current_page;
		fclose(fp);	
	}
	count = 0;
#endif
	p = json_get_node(buffer,"\"payload\"",&sz);
	if(p == NULL)
		return -1;	
	p1 = json_get_node(p,"\"buddy_list\"",&sz_1);
	if(p1 == NULL)
		return -2;
	p2 = json_get_node(p1,"\"userInfos\"",&sz_2);
	if(p2 == NULL)
		return -3;
/*	p3 = json_get_node_value(p1,"\"availableCount\"",sz_1);
	if(p3 == NULL)
		return -3;*/
	p3 = json_get_node(p1,"\"nowAvailableList\"",&sz_3);
	if(p3 == NULL)
		return -4;	
	p4 = json_get_node(p1,"\"userInfos\"",&sz_4);
	if(p4 == NULL)
		return -5;	
	uid = (char *)temp_buffer;
	name = (char *)uid + 300;
	avatar_url = (char *)name + 500;
	p_end = p4 + sz_4;
	p3[sz_3] = 0;
	i_b = 0;
	while(p4 < p_end){
		if(i_b == 0){
			memset(uid,0,2000);
			//get uid
			while(*p4 != '\"' && p4 < p_end)
				p4++;
			p4++;
			i = 0;
			while(*p4 != '\"' && p4 < p_end){
				uid[i++] = *p4;
				p4++;
			}
			uid[i] = 0;//uid			
			while(*p4 != '{' && p4 < p_end)
				p4++;
			if(*p4 != '{')
				break;
			i_b = 1;
			c = strstr(p4,"\"name\"");
			if(c != NULL){
				LPFBCONTACT pfbc;
				
				p4 = c + 6;
				while(*p4 != '\"' && p4 < p_end)
					p4++;
				p4++;
				i = 0;
				while(*p4 != '\"' && p4 < p_end){
					name[i++] = translate_UTF(p4,&i1);
					p4 += i1;
				}
				name[i] = 0;
				c = strstr(p4,"\"thumbSrc\"");
				if(c != NULL){
					i = 0;
					p4 = c + 10;
					while(*p4 != '\"' && p4 < p_end)
						p4++;
					p4++;
					i = 0;
					while(*p4 != '\"' && p4 < p_end){
						if(*p4 != '\\')
							avatar_url[i++] = *p4;
						p4++;
					}
					avatar_url[i] = 0;					
				}
				
				pfbc = find_contact_from_id(uid);
				if(pfbc == NULL){
					i = add_contact(name,uid,avatar_url);
					pfbc = get_contact_item(i);
				}
				if(pfbc != NULL){
					pfbc->status |= 2;//find in buddy list
					if(strstr(p3,uid) != NULL)
						pfbc->status |= 1;//Online
					else
						pfbc->status &= ~1;//Not online
				}
			}
		}
		else if(*p4 == '}')
			i_b--;
		else if(*p4 == '{')
			i_b++;
		p4++;
	}
	pfb->status |= 1;	
	return 0;
}
//---------------------------------------------------------------------------------