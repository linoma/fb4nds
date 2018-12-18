#include "windows.h"

#ifndef __FBNOTIFYH__
#define __FBNOTIFYH__

typedef struct
{
	char *txt;
	char *link;
	char *url_profile;
	char *time;
	unsigned int u_time;
	unsigned short *avatar;
	unsigned int status;
}FBNEWS,*LPFBNEWS;

extern int fb_parse_notify(char *data,int size);
extern int show_notify_win();
extern int init_news_list(void *mem);
extern LPFBNEWS get_news_item(int index);

#endif
