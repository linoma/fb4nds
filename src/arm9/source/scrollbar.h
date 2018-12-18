#include "windows.h"

#ifndef __SCROLLBARH__
#define __SCROLLBARH__

typedef struct
{
	long pos;
	long min;
	long max;
	long page;
	long size_thumb;
	LPWINDOW hWnd;
} SCROLLBARDATA,*LPSCROLLBARDATA;

int create_scrollbar_win(unsigned char screen,int x,int y,int width,int height);
int create_scrollbar_win_win(HWND hWnd,int style);
int set_scrollbar_info(int sb,unsigned int min,unsigned int max,unsigned int page);
int set_scrollbar_pos(int sb,unsigned int pos);
int get_scrollbar_pos(int sb);

#endif
