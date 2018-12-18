#include "windows.h"

#ifndef __LISTBOXH__
#define __LISTBOXH__

#define LBS_OWNERDRAWVARIABLE	0x100000

#define LB_ADDSTRING 			1000
#define LB_INSERTSTRING			1001
#define LB_GETCURSEL			1002
#define LB_GETCOUNT				1003
#define LB_RESETCONTENT			1004
#define LB_SETCURSEL			1005
#define LB_GETITEMHEIGHT		1006
#define LB_SETITEMHEIGHT		1007
#define LB_GETITEM				1008
#define LB_GETITEMRECT			1009

#define ODS_SELECTED		1

#define LBN_SELCHANGE		1
#define LBN_DBLCLK			2

#define LB_ERR				((unsigned int)-1)

typedef struct
{
	WINEXHEADER hdr;
	unsigned int first_item;
	unsigned int count_item;	
	unsigned int select_item;
	int height_item;
	void *data;
	unsigned int size_data;
	unsigned int size_used;
} LISTBOXDATA,*LPLISTBOXDATA;

extern int create_listbox_win(unsigned char screen,int x,int y,int width,int height);
extern int set_listbox_ownerdraw(int win,LPOWNERDRAW pfn);

#endif