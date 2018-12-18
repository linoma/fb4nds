#include <PA9.h>
#include "ram.h"

#ifndef __UTILSH__
#define __UTILSH__

void dmaFillWordsAsync( u32 value, void* dest, uint32 size);
int trylock_mutex(unsigned long *cr);
void unlock_mutex(unsigned long *cr);
void EnterCriticalSection(unsigned long *cr);
void LeaveCriticalSection(unsigned long *cr);
int translate_UTF(const char *text, int *i);
int strlen_UTF(char *text);
int encode_string(char *src,char *dst);
void my_sleep(int timeout);
int getMemFree();
int getMemUsed();
FILE *open_file(char *file_name,char *flags);
int init_system();
char *strnstri(char *str0,char *str1,int size);
unsigned long i_log2(unsigned long value);
const char *get_module_filename();
void divf32_async(int num,int den);

#endif
