#include "windows.h"

#ifndef __EDITBOXH__
#define __EDITBOXH__

#define ES_MULTILINE	0x10000
#define EN_UPDATE		0x1

#define EM_REPLACESEL	1000
#define EM_GETLINECOUNT	1001
#define EM_LINELENGTH	1002
#define EM_GETLINE		1003
#define EM_POSFROMCHAR	1004

typedef struct
{
	WINEXHEADER hdr;
	char *txt;	
	unsigned int size_buffer;
	unsigned int first_visible_line;
	unsigned int cr_char;	
} EDITBOXDATA,*LPEDITBOXDATA;

typedef struct
{
	int x,y;
	HWND hWnd;
	int timer_id;
	int oam_index[2];
} CARETDATA,*LPCARETDATA;

extern int create_editbox_win(unsigned char screen,int x,int y,int width,int height);

#endif