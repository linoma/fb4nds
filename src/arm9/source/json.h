#include "windows.h"

#ifndef __JSONH__
#define __JSONH__

extern char *json_get_node(char *buffer,char *key,unsigned int *size);
extern char *json_get_node_value(char *buffer,char *key,unsigned int size);


#endif