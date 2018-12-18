#include <PA9.h>
#include "utils.h"

#ifndef __WINDOWSH__
#define __WINDOWSH__

#define MAX_WINDOWS		100
#define MAX_TIMERS		10

#define WST_ENABLED		0x80000000
#define WST_HASFOCUS	0x40000000

#define WS_HSCROLL		0x00000001
#define WS_VSCROLL		0x00000002
#define WS_CAPTION		0x00000004
#define WS_BORDER		0x00000008
#define WS_TABSTOP		0x00000010
#define WS_VISIBLE		0x00000020
#define WS_CHILD		0x00000040
#define WS_CLOSE		0x00000080

#define WM_PAINT 			1
#define WM_ERASEBKGND		2
#define WM_SETTEXT			3
#define WM_KEYDOWN			4
#define WM_KEYUP			5
#define WM_CREATE			6
#define WM_DESTROY			7
#define WM_TIMER			8
#define WM_LBUTTONDOWN		9
#define WM_MOUSEMOVE		10
#define WM_LBUTTONDBLCLK	11
#define WM_SETFOCUS			12
#define WM_KILLFOCUS		13
#define WM_CLOSE			14
#define WM_VSCROLL			20
#define WM_HSCROLL			21
#define WM_COMMAND			22
#define WM_GETTEXT			23
#define WM_GETTEXTLENGTH	24
#define WM_NCCALCSIZE		25
#define WM_MEASUREITEM		26
#define WM_NCLBUTTONDOWN	27
#define WM_RBUTTONDOWN		28

#define SB_SETTEXT			1000

#define VK_TAB				0x9
#define VK_RETURN			0xD
#define VK_SHIFT			0x10
#define VK_LEFT      		0x25
#define VK_UP           	0x26
#define VK_RIGHT        	0x27
#define VK_DOWN         	0x28


#define SB_LINEDOWN			1
#define SB_LINEUP			2
#define SB_PAGEDOWN			3
#define SB_PAGEUP			4
#define SB_THUMBPOSITION	5

#define MB_OK				1
#define MB_CANCEL			2
#define MB_YES				5
#define MB_NO				6

#define MB_SYSTEMMODAL		0x80000000

#define WC_SCROLLBAR		10
#define WC_LISTBOX			11
#define WC_EDIT				12
#define WC_BUTTON			13
#define WC_MENU				14
#define WC_DIALOG			15
#define WC_TABCONTROL		16
#define WC_COMBOBOX			17
#define WC_CHECKBOX			18

#define SW_SHOW				1
#define SW_HIDE				0

#define MAKEWORD(a,b) 		(((unsigned char)a) | (((unsigned char)b) << 8))
#define MAKELONG(a,b) 		(((unsigned short)a) | (((unsigned short)b) << 16))
#define MAKELPARAM(a,b) 	MAKELONG(a,b)
#define MAKEWPARAM(a,b) 	MAKELONG(a,b)
#define LOWORD(a)			((unsigned short)a)
#define HIWORD(a)			((unsigned short)(a >> 16))

typedef struct {
	long left;
	long top;
	long right;
	long bottom;
} RECT,*LPRECT;

typedef struct {
	long x;
	long y;
} POINT, *LPPOINT;

typedef struct {
	long cx;
	long cy;
} SIZE, *LPSIZE;

struct WINDOW;
typedef int (*LPWNDPROC)(struct WINDOW *,unsigned int,unsigned int,unsigned int);
typedef int (*LPTIMERPROC)(unsigned int,unsigned int);

typedef struct {
	RECT rc;
	unsigned long style;
	unsigned long status;
	unsigned char screen;
	unsigned char type;
	unsigned long critical_section;
	unsigned short bg_color;
	int font;
	LPWNDPROC pfn_proc;
	struct WINDOW *next,*prev;
	struct{
		struct WINDOW *next,*prev,*parent;
		int id;
	} childs;
	unsigned char *data;
	unsigned char *user_data;
} WINDOW,*LPWINDOW;

typedef struct _wintimer{
	u32 timer;
	u32 elapsed;
	u32 id;
	LPWINDOW hWnd;
	LPTIMERPROC pfn_proc;
} WINTIMER,*LPWINTIMER;

typedef struct {
    LPWINDOW	 hwndItem;
    unsigned int itemID;
	unsigned int itemAction;
    unsigned int itemState;
	char *		itemText;
	unsigned int itemData;
    RECT         rcItem;	
} DRAWITEMSTRUCT,*LPDRAWITEMSTRUCT;

typedef struct {
	LPWINDOW hwndItem;
	unsigned int itemID;
	char * itemText;
	unsigned int itemData;
	RECT rcItem;
} MEASUREITEM,*LPMEASUREITEM;

typedef unsigned int 	LPARAM;
typedef unsigned int 	WPARAM;
typedef LPWINDOW 		HWND;

typedef int (*LPOWNERDRAW)(LPDRAWITEMSTRUCT);

typedef struct{
	char *caption;
	int vert_sb,horz_sb;
	LPOWNERDRAW pfn_ownerdraw;
} WINEXHEADER,*LPWINEXHEADER;

typedef struct {
	WINEXHEADER hdr;
	int style;
	char *msg;
	LPWNDPROC pfn_proc;
} WINMSGBOX,*LPWINMSGBOX;

int init_windows();
void destroy_windows();
int update_window(int win);
void update_windows();
void redraw_windows();
void redraw_window(int win);
void redraw_windows_screen(u8 screen);
int SetWindowText(int win,char *txt);
int get_free_window();
int get_window_index(LPWINDOW hWnd);
int SendMessage(int win,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
int SendMessage_HWND(HWND hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
void adjust_input_text(char *txt);
int set_timer(LPWINDOW hWnd,unsigned int timeout,unsigned int id,LPTIMERPROC proc);
void destroy_timer(int i);
void execute_windows_loop();
int get_window_from_pos(u8 screen,long x,long y);
int get_client_rect(LPWINDOW hWnd,LPRECT lprc);
int get_next_tab_window(u8 screen,LPWINDOW hWnd);
int set_focus_window(HWND hWnd);
int enable_window(LPWINDOW hWnd,int enable);
void destroy_window(int win);
int def_vbl_proc(void);
int create_dialog_win(u8 screen,int x,int y,int width,int height,int style,char *caption);
void insert_win(int win,int pos);
int bring_win_top(int win);
void switch_screen();
void append_win(int win);
int create_message_box(u8 screen,char *caption,char *msg,unsigned int style,LPWNDPROC pfn_proc);
void adjust_input_text(char *txt);
int read_keyboard();
int check_move_win();
void check_touch_screen();
void check_tab_key();
void check_close_key();
int init_winex_header(HWND hWnd,unsigned int size);
HWND get_dlg_item(HWND parent,int id);
HWND set_parent_window(HWND hWnd,HWND parent);
int set_dlg_item(int win,HWND parent,int id);
int SendDlgItemMessage(HWND hWnd,int id,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
int invalidate_window(int win,LPRECT lprc,int be);
int wndproc_winex_win(LPWINDOW hWnd,unsigned int uMsg,unsigned int wParam,unsigned int lParam);
int show_window(int win,int nCmdShow);
void invalidate_windows_screen(u8 screen,int be);
int msg_box_proc(void);

extern WINDOW windows[MAX_WINDOWS];
extern int active_win,active_win_top,menu_bar_bottom_win,chat_tab_win;
extern int active_focus,quit_msg_box;
extern int consolle_win,btn_connect_win;
extern char text[1000];
extern u8 system_images[5000];
extern int system_image_index[20];

#endif
