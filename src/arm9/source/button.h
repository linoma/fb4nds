#include "windows.h"

#ifndef __BUTTONH__
#define __BUTTONH__

#define BN_CLICKED 		1

#define BM_GETCHECK		1000
#define BM_SETCHECK		1001
#define BM_GETSTATE		1002

#define BST_CHECKED			1
#define BST_UNCHECKED		0
#define BST_INDETERMINATE	2
extern int create_button_win(unsigned char screen,int x,int y,int width,int height);

#endif