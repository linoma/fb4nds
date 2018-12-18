#include "windows.h"
#include "draws.h"
#include "fb_list.h"

#ifndef __CONTACTSLISTH__
#define __CONTACTSLISTH__

typedef struct
{
	char *mail;
	char *name;	
	char *uid;
	char *url_avatar;
	unsigned short *avatar;
	int win;
	LPWNDPROC old_proc;
	unsigned int status;
} FBCONTACT,*LPFBCONTACT;

int show_contacts_list();
void hide_contacts_list();
int add_new_contact();
int refresh_contacts_list();
int init_contact_list(void *mem);
LPFBCONTACT find_contact_from_id(char *uid);
int add_contact(char *name,char *uid,char *url_avatar);
LPFBCONTACT get_contact_item(int index);
LPFBCONTACT get_selected_contact();
LPFBCONTACT find_chat_win_uid(LPWINDOW hWnd,char *uid);
int create_chat_win(LPFBCONTACT pfbc);
int execute_contacts_loop();
int get_online_contacts();
int update_contacts_list();

extern LPFBLIST ls_contacts;

#endif