#include "windows.h"

#ifndef __CHATWINH__
#define __CHATWINH__

#define TCM_ADDSTRING		1000
#define TCM_RESETCONTENT	1001
#define TCM_GETCOUNT		1002
#define TCM_GETITEM			1003

typedef struct
{
	WINEXHEADER hdr;
	unsigned int first_item;
	unsigned int count_item;	
	unsigned int select_item;
	unsigned int draw_item;
	int timer;
	void *data;
	unsigned int size_data;
	unsigned int size_used;
} CHATTABDATA,*LPCHATTABDATA;

extern int create_chat_tab_win(unsigned char screen,int x,int y,int width,int height);

#endif