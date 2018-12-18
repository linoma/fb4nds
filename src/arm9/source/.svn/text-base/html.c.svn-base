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
 
#include "html.h"
#include "utils.h"

//------------------------------------------------------------------------------------------
char *html_get_tag(char *buffer,int size,char *to,char *tc,int *psize,int *psize1)
{	
   int i,i1,i2,i3,i4,i5;

	if(psize)
		*psize = 0;
	if(psize1)
		*psize1 = 0;
	if(buffer == NULL || buffer[0] == NULL || size == 0 || to == NULL || to[0] == 0)
		return NULL;
	if(size == -1)
		size = (int)0x7FFFFFFF;
	size--;
	i2 = -1;
	for(i=0;i < size && buffer[i] != 0;i++){
		if(buffer[i] == '<'){
           char *p;

           for(;!isalpha(buffer[i]);i++);
           if((p = strnstri(&buffer[i],to,(size - i))) != NULL){
               i2 = (int)((unsigned int)p - (unsigned int)buffer);
               i = i2--;
               break;
           }
		}
	}
	if(i2 == -1)
		return NULL;
	i1 = -1;
	i5 = 1;
	for(;i < size && buffer[i] != 0;i++){
		if(buffer[i] == '>'){
			i1 = i++;//end tag
			break;
		}
	}
	if(i1 == -1)
		return NULL;
	if(psize)
		*psize = i1 - i2 + 1;
	if(tc == NULL)
		return &buffer[i2];
	i4 = -1;
	i5 = 0;
	for(;i < size && buffer[i] != 0;i++){
		if(buffer[i] == '<'){
			i4 = i;
			i5++;
			for(;!isalpha(buffer[i]);i++);
			if(strnstri(&buffer[i],tc,(size - i)) != NULL){
				i++;
				break;
			}
			i4 = -1;
		}
		else if(buffer[i] == '>')
			i5--;
	}
	if(i4 == -1)
		return &buffer[i2];
	i3 = -1;
	i5 = 1;
	for(;i < size && buffer[i] != 0;i++){
		if(buffer[i] == '>'){
			i3 = i;//end tag
			break;
		}
	}
	if(i3 != -1 && psize1)
		*psize1 = i3 - i1;
	return &buffer[i2];
}
//------------------------------------------------------------------------------------------
char *html_find_tag(char *buffer,int size,char *str)
{
	char *p;
	int sz;

	if(buffer == NULL || str == NULL || buffer[0] == 0)
		return NULL;
	if(size == -1)
		size = (int)0x7FFFFFFF;
	p = strstr(buffer,str);
	if(p == NULL)
		return NULL;
	sz = (int)((unsigned int)p - (unsigned int)buffer);
	if(sz > size)
		return NULL;
	while(*p != '<' && p >= buffer)
		p--;
	return p;
}
//------------------------------------------------------------------------------------------
char *html_get_anchor_ref(char *buffer,int size,char *str,int *psize)
{
	char *p;
	int sz;

	p = html_find_tag(buffer,size,str);
	if(p == NULL)
		return NULL;
	sz = size != -1 ? (size - ((unsigned int)p - (unsigned int)buffer)) : -1;
	p = html_get_tag(p,sz,"a","a",&sz,NULL);
	if(p == NULL)
		return NULL;
	if(psize)
		*psize = sz;
	return strnstri(p,"href",sz);
}
//------------------------------------------------------------------------------------------
char *html_get_anchor(char *buffer,int size,char *str,int *psize)
{
	char *p;
	int sz;

	p = html_find_tag(buffer,size,str);
	if(p == NULL)
		return NULL;
	sz = size != -1 ? (size - ((unsigned int)p - (unsigned int)buffer)) : -1;
	return html_get_tag(p,sz,"a","a",psize,NULL);
}
//------------------------------------------------------------------------------------------
char *html_get_image_source(char *buffer,int size,int *psize)
{
	return NULL;
}
 //------------------------------------------------------------------------------------------
char *html_get_image(char *buffer,int size,int *psize)
{
	return html_get_tag(buffer,size,"img",NULL,psize,NULL);		
}
