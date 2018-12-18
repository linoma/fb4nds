#include "windows.h"

#ifndef __MENUH__
#define __MENUH__

#define PMT_NORMAL	0
#define PMT_BAR		1

#define MIT_STRING	1
#define MF_ENABLED	0
#define MF_DISABLED 0x40000000
#define MF_SELECTED 0x80000000

typedef struct
{
	unsigned int flags;
	unsigned int id;
	void *data;
	unsigned int size_data;
} MENUITEM,*LPMENUITEM;

typedef struct
{
	int h_parent;
	unsigned int flags;
	void *data;
	unsigned int selected_item;
	unsigned int size_data;
	unsigned int size_used;	
	unsigned int count_item;
} POPUPMENU,*LPPOPUPMENU;

extern int insert_menu_item(HWND hWnd,unsigned int id,void *data,unsigned int flags);
extern int create_menubar_win(unsigned char screen,int x,int y,int width,int height,int parent);
extern LPMENUITEM get_menuitem_from_pos(LPWINDOW hWnd,int x, int y);
extern int show_popupmenu_win(int win,int x,int y,int parent);
extern int create_popupmenu_win(unsigned char screen,int parent);
extern void destroy_menu(int win);
extern int select_menu_item(HWND hWnd,unsigned int index);
extern int get_menuitem_rect(LPWINDOW hWnd,unsigned int id,LPRECT lprc);
extern int enable_menu_item(HWND hWnd,unsigned int index,unsigned int flags);
extern LPMENUITEM get_first_menuitem_enabled(LPWINDOW hWnd,LPRECT lprc);

#endif