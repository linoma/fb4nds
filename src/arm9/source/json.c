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

#include "json.h"

//---------------------------------------------------------------------------------
char *json_get_node_value(char *buffer,char *key,unsigned int size)
{
	unsigned int sz;
	char *p;
	
	p = strstr(buffer,key);
	if(p == NULL)
		return NULL;
	sz = (unsigned int)p - (unsigned int)buffer;
	if(sz >= size)
		return NULL;
	while(*p != ':' && *p != 0 && sz < size){
		p++;
		sz++;
	}
	if(*p == 0)
		return NULL;
	p++;
	return p;
}
//---------------------------------------------------------------------------------
char *json_get_node(char *buffer,char *key,unsigned int *size)
{
	char *p,*p1;
	int i;
	
	if(buffer == NULL || key == NULL || buffer[0] == 0 || key[0] == 0)
		return NULL;
	p = strstr(buffer,key);
	if(p == NULL)
		return NULL;
	p += strlen(key);
	while(*p != ':' && *p != 0)
		p++;
	if(*p == 0)
		return NULL;
	while(*p != '{' && *p != 0)
		p++;
	if(*p == 0)
		return NULL;
	p++;
	p1 = p;
	i = 1;
	while(*p != 0){
		if(*p == '}'){
			i--;
			if(i == 0)
				break;
		}
		else if(*p == '{')
			i++;
		p++;
	}
	if(i)
		return NULL;
	if(size != NULL)
		*size = (unsigned int)((unsigned int)p - (unsigned int)p1);
	return p1;
}