//============================================================================//
//                                                                            //
//  Copyright 2007 Rick "Lick" Wong                                           //
//                                                                            //
//  Credits to: Chishm, Lazy1, Cory1492, Viruseb, Amadeus, Pepsiman           //
//                                                                            //
//  Feel free to use and redistribute my library freely, as long as you       //
//   a) credit me for this library and                                        //
//   b) publish all your modifications to this library (if any are made) and  //
//   c) do not create any damaging software with this library.                //
//                                                                            //
//                                                                            //
//============================================================================//

#ifndef __RAMH__
#define __RAMH__

#include <nds.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum { DETECT_RAM=0, SC_RAM, M3_RAM, OPERA_RAM, G6_RAM, EZ_RAM, LOCAL_RAM } RAM_TYPE;

typedef struct
{
	unsigned long adr;
	unsigned long size;
	unsigned long status;
	void *prev,*next;
} RAMBLOCK,*LPRAMBLOCK;

typedef struct
{
	unsigned long items;
	unsigned long size;//
	unsigned long used;
	unsigned long size_ram;//
	unsigned long adr_ram;
	unsigned long adr;
	LPRAMBLOCK first,last;
} RAMBLOCKLIST,*LPRAMBLOCKLIST;

bool ram_init(RAM_TYPE);
u32 ram_size();
vu16* ram_unlock();
void ram_lock();
void *ram_disk_alloc(unsigned int size);
unsigned short *ram_disk_lock(void *mem);
int ram_disk_unlock(void *mem);
int ram_disk_free(void *mem);
void ram_disk_memcpy(void *dst,void *src,unsigned int size);
RAM_TYPE _get_ram_type();
int is_ext_ram();

#ifdef __cplusplus
}
#endif
#endif
