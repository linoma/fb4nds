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

#include "utils.h"
#include <malloc.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>

#define IS_ENTITY(a,b) (!strncmp(a,b,strlen(b)))

extern u8 __end__[];        // end of static code and data
extern u8 __eheap_end[];    // farthest point to which the heap will grow
extern u32 ticks; 
extern u8 *temp_buffer;
static char exe_path[256];
//---------------------------------------------------------------------------------
char *strnstri(char *str0,char *str1,int size)
{
	int i,len;
	char *s;

	if(str1 == NULL || str0 == NULL || str0[0] == 0 || size < 0)
		return NULL;
	for(len = 0,s = str1;*s != 0;len++,s++)
		*s = (char)tolower(*s);
	if(len < 1)
		return NULL;
	while(size > 0){
		s = str0;
		for(i=0;i<len && *s != 0 && size > i;i++){
			if(tolower(*s++) != str1[i])
               break;
		}
		if(i == len)
			return str0;
		str0 = s;
		size -= ++i;
	}
	return NULL;
}
//---------------------------------------------------------------------------------
void my_sleep(int timeout)
{
	u32 oldticks;
	
	timeout >>= 4; 
	oldticks = ticks;	
	while(1){
		swiWaitForVBlank();
		if(ticks > (oldticks + timeout))
			break;
	}		
}
//---------------------------------------------------------------------------------
void dmaFillWordsAsync(u32 value, void* dest, uint32 size) 
{
	while(dmaBusy(3));
#ifdef ARM7	
	(*(vu32*)0x027FFE04) = (vu32) value;
	DMA_SRC(3) = 0x027FFE04;
#else	
	DMA_FILL(3) = (vuint32)value;
	DMA_SRC(3) = (uint32)&DMA_FILL(3);
#endif	
    DMA_DEST(3) = (uint32)dest;
	DMA_CR(3) = DMA_SRC_FIX | DMA_COPY_WORDS | (size>>2);	
}
//---------------------------------------------------------------------------------
void divf32_async(int num,int den)
{
	int64 value;
	
	REG_DIVCNT = DIV_64_32;
	while(REG_DIVCNT & DIV_BUSY);
	value = num;
	value <<= 12;	
	REG_DIV_NUMER = value;
	REG_DIV_DENOM_L = den;
}
//---------------------------------------------------------------------------------
unsigned long i_log2(unsigned long value)
{
	unsigned long val;
	
#ifdef ARM9
	asm volatile("tst\t%0,%0\nclzne\t%0, %1\nrsbne %0,%0,#31\nmoveq %0,#0\n"
		:"=r" (val)
		:"r" (value)
		:"memory");
#else
	for(val=0;value > 1;value >>= 1)
		val++;
#endif		
	return val;
}
//---------------------------------------------------------------------------------
void unlock_mutex(unsigned long *cr)
{
	long val;
	
	asm volatile ("swp\t%0, %1, [%2]"
        :"=r" (val)
        :"r" (0),"r" (cr)
        :"memory");
}
//---------------------------------------------------------------------------------
int trylock_mutex(unsigned long *cr)
{
	long val;
	
	asm volatile ("swp\t%0, %1, [%2]"
        :"=&r,&r" (val)
        :"r,0" (1),"r,r" (cr)
        :"memory");
	return val != 1;	
}
//---------------------------------------------------------------------------------
void EnterCriticalSection(unsigned long *cr)
{
	*cr = enterCriticalSection();
}
//---------------------------------------------------------------------------------
void LeaveCriticalSection(unsigned long *cr)
{
	leaveCriticalSection(*cr);
}
//---------------------------------------------------------------------------------
int translate_UTF(const char *text, int *i) 
{
	int code;
	
	*i = 0;
	code = text[*i];
	if(code > 127){
		/*if(text[i] == 0xFF){
			code = -1;
			i++;
		} 
		else */
		if(~text[*i] & 32){
			code = ((text[*i] & 31) << 6) | (text[*i+1] & 63);
			*i = *i + 2;
		} 
		else if (~text[*i] & 16){
			code = ((text[*i] & 31) << 12) | ((text[*i+1] & 63) << 6) | (text[*i+2] & 63);
			*i = *i + 3;
		} 
		else if (~text[*i] & 8){
			code = ((text[*i] & 31) << 18) | 
				((text[*i+1] & 63) << 12) | 
				((text[*i+2] & 63) << 6) | 
				(text[*i+3] & 63);
			*i = *i + 4;
		} 
		else{
			code = ((text[*i] & 31) << 24) | 
				((text[*i+1] & 63) << 18) | 
				((text[*i+2] & 63) << 12) | 
				((text[*i+3] & 63) << 6) | 
				(text[*i+4] & 63);
			*i = *i + 5;
		}
	} 
	else{		
		if(code == '&'){
			if(IS_ENTITY(&text[*i],"&apos;")){
				code = '\'';				
				*i = *i + 5;
			}
			else if(IS_ENTITY(&text[*i],"&amp;")){
				code = '&';
				*i = *i + 4;
			}
			else if(IS_ENTITY(&text[*i],"&lt;")){
				code = '<';
				*i = *i + 3;
			}
			else if(IS_ENTITY(&text[*i],"&gt;")){
				code = '>';
				*i = *i + 3;
			}
			else if(IS_ENTITY(&text[*i],"&nbsp;")){
				code = ' ';
				*i = *i + 5;
			}
			else if(IS_ENTITY(&text[*i],"&quot;")){
				code = '\"';
				*i = *i + 5;
			}
			else if(text[*i+1] == '#'){
				sscanf(&text[*i+2],"%3d",&code);
				*i = *i + 2 + 3;
			}
		}
		else if(code == '\\' && text[*i + 1] == 'u'){
			code = 32;
			*i = *i + 5;
		}
		*i = *i + 1;
	}
	return code;
}
//---------------------------------------------------------------------------------
int strlen_UTF(char *text)
{
	int x1,pos;
	
	if(text == NULL)
		return 0;		
	for(pos = 0;;){
		if(translate_UTF(&text[pos],&x1) == 0)
			break;
		pos += x1;
	}
	return pos;
}
//---------------------------------------------------------------------------------
int encode_string(char *src,char *dst)
{
	int i;
	char c1[5];
	
	dst[0] = 0;
	for(i=0;src[i] != 0;i++){
		if(src[i] < 128 && (isalnum(src[i]) || src[i] == '-' || src[i] == '.' || src[i] == '_' || src[i] == '~')){
			c1[0] = src[i];
			c1[1] = 0;
		}
		else
			sprintf(c1,"%%%02X",(int)(unsigned char)src[i]);
		strcat(dst,c1);
	}	
	return i;
}
//---------------------------------------------------------------------------------
unsigned char* getHeapStart() 
{
	return __end__;
}
//---------------------------------------------------------------------------------
unsigned char* getHeapEnd() 
{
    return (unsigned char*)sbrk(0);
}
//---------------------------------------------------------------------------------
unsigned char* getHeapLimit() 
{
    return __eheap_end;
}
//---------------------------------------------------------------------------------
int getMemUsed() 
{
    struct mallinfo mi = mallinfo();
    return mi.uordblks;
}
//---------------------------------------------------------------------------------
int getMemFree() 
{
    struct mallinfo mi = mallinfo();
    return mi.fordblks + (getHeapLimit() - getHeapEnd());
}
//---------------------------------------------------------------------------------
const char *get_module_filename()
{
	return exe_path;
}
//---------------------------------------------------------------------------------
FILE *open_file(char *file_name,char *flags)
{
	strcpy((char *)temp_buffer,exe_path);
	strcat((char *)temp_buffer,file_name);
	return fopen((char *)temp_buffer,flags);
}
//---------------------------------------------------------------------------------
static int enum_directories(char *path)
{	
	DIR *dir;
	int i;

	i = strlen((char *)temp_buffer);
	strcat((char *)temp_buffer,path);
	if(temp_buffer[strlen((char *)temp_buffer) - 1] != '/')
		strcat((char *)temp_buffer,"/");
	dir = opendir((char *)temp_buffer);
	if(dir != NULL){
		struct dirent *ent;

		while((ent = readdir(dir)) != NULL) {
			if(strcmpi(ent->d_name,"fb4nds.nds") == 0){
				strcpy(exe_path,(char *)temp_buffer);
				closedir(dir);
				return 1;
			}
			if(ent->d_name[0] != '.'){
				if(enum_directories(ent->d_name) == 1){
					closedir(dir);
					return 1;
				}
			}
		}
		closedir(dir);
		temp_buffer[i] = 0;
	}
	temp_buffer[i] = 0;
	return 0;
}
//---------------------------------------------------------------------------------
int init_system()
{			
	chdir("/");		
	memset(temp_buffer,0,1000);
	enum_directories("/");
	return 0;
}

