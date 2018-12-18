/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ram.h"
#include "fb.h"
#include "all_gfx.h"

static vu16* (*_unlock)() = 0;
static void  (*_lock)() = 0;
static u32  _size = 0;
static LPRAMBLOCKLIST ramblocks_list = NULL;
static RAM_TYPE _ram_type = 0;
//---------------------------------------------------------------------------------
vu16 *_sc_unlock()
{
	sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);
    *(vu16*)0x9FFFFFE = 0xA55A;
    *(vu16*)0x9FFFFFE = 0xA55A;
    *(vu16*)0x9FFFFFE = 0x5; // RAM_RW
    *(vu16*)0x9FFFFFE = 0x5;
    return (vu16*)0x8000000;
}
//---------------------------------------------------------------------------------
void _sc_lock()
{
    *(vu16*)0x9FFFFFE = 0xA55A;
    *(vu16*)0x9FFFFFE = 0xA55A;
    *(vu16*)0x9FFFFFE = 0x3; // MEDIA
    *(vu16*)0x9FFFFFE = 0x3;
}
//---------------------------------------------------------------------------------
vu16 *_m3_unlock()
{
    u32 mode = 0x00400006; // RAM_RW
    vu16 tmp;

    sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);
    tmp = *(vu16*)0x08E00002;
    tmp = *(vu16*)0x0800000E;
    tmp = *(vu16*)0x08801FFC;
    tmp = *(vu16*)0x0800104A;
    tmp = *(vu16*)0x08800612;
    tmp = *(vu16*)0x08000000;
    tmp = *(vu16*)0x08801B66;
    tmp = *(vu16*)(0x08000000 + (mode << 1));
    tmp = *(vu16*)0x0800080E;
    tmp = *(vu16*)0x08000000;
    tmp = *(vu16*)0x080001E4;
    tmp = *(vu16*)0x080001E4;
    tmp = *(vu16*)0x08000188;
    tmp = *(vu16*)0x08000188;
    return (vu16*)0x8000000;
}
//---------------------------------------------------------------------------------
void _m3_lock()
{
    u32 mode = 0x00400003; // MEDIA
    vu16 tmp;
    tmp = *(vu16*)0x08E00002;
    tmp = *(vu16*)0x0800000E;
    tmp = *(vu16*)0x08801FFC;
    tmp = *(vu16*)0x0800104A;
    tmp = *(vu16*)0x08800612;
    tmp = *(vu16*)0x08000000;
    tmp = *(vu16*)0x08801B66;
    tmp = *(vu16*)(0x08000000 + (mode << 1));
    tmp = *(vu16*)0x0800080E;
    tmp = *(vu16*)0x08000000;
    tmp = *(vu16*)0x080001E4;
    tmp = *(vu16*)0x080001E4;
    tmp = *(vu16*)0x08000188;
    tmp = *(vu16*)0x08000188;
}
//---------------------------------------------------------------------------------
vu16 *_opera_unlock()
{
    sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);       
	*(vu16*)0x8240000 = 1;
    return (vu16*)0x9000000;
}
//---------------------------------------------------------------------------------
void _opera_lock()
{
	*(vu16*)0x8240000 = 0;
}
//---------------------------------------------------------------------------------
vu16 *_g6_unlock()
{
    u32 mode; // RAM_RW
    vu16 tmp;

    sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);
	mode = 6;
	tmp = *(vu16*)0x09000000;
	tmp = *(vu16*)0x09FFFFE0;
	tmp = *(vu16*)0x09FFFFEC;
	tmp = *(vu16*)0x09FFFFEC;
	tmp = *(vu16*)0x09FFFFEC;
	tmp = *(vu16*)0x09FFFFFC;
	tmp = *(vu16*)0x09FFFFFC;
	tmp = *(vu16*)0x09FFFFFC;
	tmp = *(vu16*)0x09FFFF4A;
	tmp = *(vu16*)0x09FFFF4A;
	tmp = *(vu16*)0x09FFFF4A;
	tmp = *(vu16*)(0x09200000 + (mode << 1));
	tmp = *(vu16*)0x09FFFFF0;
	tmp = *(vu16*)0x09FFFFE8;
    return (vu16*)0x8000000;
}
//---------------------------------------------------------------------------------
void _g6_lock()
{
    u32 mode; // MEDIA
    vu16 tmp;
	
	mode = 3;
	tmp = *(vu16*)0x09000000;
	tmp = *(vu16*)0x09FFFFE0;
	tmp = *(vu16*)0x09FFFFEC;
	tmp = *(vu16*)0x09FFFFEC;
	tmp = *(vu16*)0x09FFFFEC;
	tmp = *(vu16*)0x09FFFFFC;
	tmp = *(vu16*)0x09FFFFFC;
	tmp = *(vu16*)0x09FFFFFC;
	tmp = *(vu16*)0x09FFFF4A;
	tmp = *(vu16*)0x09FFFF4A;
	tmp = *(vu16*)0x09FFFF4A;
	tmp = *(vu16*)(0x09200000 + (mode << 1));
	tmp = *(vu16*)0x09FFFFF0;
	tmp = *(vu16*)0x09FFFFE8;
}
//---------------------------------------------------------------------------------
vu16 *_ez_ex_unlock()
{
	sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);	
    *(vu16*)0x9FE0000 = 0xD200;
    *(vu16*)0x8000000 = 0x1500;
    *(vu16*)0x8020000 = 0xD200;
    *(vu16*)0x8040000 = 0x1500;
    *(vu16*)0x9880000 = 0x8000;
    *(vu16*)0x9FC0000 = 0x1500;

    *(vu16*)0x9FE0000 = 0xD200;
    *(vu16*)0x8000000 = 0x1500;
    *(vu16*)0x8020000 = 0xD200;
    *(vu16*)0x8040000 = 0x1500;
    *(vu16*)0x9C40000 = 0x1500;
    *(vu16*)0x9FC0000 = 0x1500;	
	return (vu16*)0x8400000;
}
//---------------------------------------------------------------------------------
vu16 *_ez_unlock()
{
    sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);
    *(vu16*)0x9FE0000 = 0xD200;
    *(vu16*)0x8000000 = 0x1500;
    *(vu16*)0x8020000 = 0xD200;
    *(vu16*)0x8040000 = 0x1500;
    *(vu16*)0x9880000 = 0x300;
    *(vu16*)0x9FC0000 = 0x1500;

    *(vu16*)0x9FE0000 = 0xD200;
    *(vu16*)0x8000000 = 0x1500;
    *(vu16*)0x8020000 = 0xD200;
    *(vu16*)0x8040000 = 0x1500;
    *(vu16*)0x9C40000 = 0x1500;
    *(vu16*)0x9FC0000 = 0x1500;
    return (vu16*)0x9000000;
}
//---------------------------------------------------------------------------------
void _ez_lock()
{
	*(vu16*)0x9FE0000 = 0xD200;
	*(vu16*)0x8000000 = 0x1500;
	*(vu16*)0x8020000 = 0xD200;
	*(vu16*)0x8040000 = 0x1500;
	*(vu16*)0x9C40000 = 0xD200;
	*(vu16*)0x9FC0000 = 0x1500;	
}
//---------------------------------------------------------------------------------
vu16 *_lr_unlock()
{
	return (vu16 *)ramblocks_list;
}
//---------------------------------------------------------------------------------
void _lr_lock()
{
}
//---------------------------------------------------------------------------------
RAM_TYPE _get_ram_type()
{
	return _ram_type;
}
//---------------------------------------------------------------------------------
int is_ext_ram()
{
	if(_get_ram_type() == 0 || _get_ram_type() == LOCAL_RAM)
		return 0;
	return 1;
}
//---------------------------------------------------------------------------------
bool ram_init(RAM_TYPE type)
{
    vu16 *ram;
	
	sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);    
    switch(type){
        case SC_RAM:
            _unlock = _sc_unlock;
            _lock   = _sc_lock;
        break;
        case M3_RAM:
            _unlock = _m3_unlock;
            _lock   = _m3_lock;
        break;
        case OPERA_RAM:
            _unlock = _opera_unlock;
            _lock   = _opera_lock;
        break;
        case G6_RAM:
            _unlock = _g6_unlock;
            _lock   = _g6_lock;
        break;
        case EZ_RAM:
            _unlock = _ez_unlock;
            _lock   = _ez_lock;
        break;
        case DETECT_RAM:
        default:
			_ram_type = SC_RAM;
			// try supercard
            ram = _sc_unlock();
            ram[0] = 0x1234;
            if(ram[0] == 0x1234){
                _sc_lock();
                _unlock = _sc_unlock;
                _lock   = _sc_lock;
                break;
            }
			_ram_type = M3_RAM;
            // try m3
            ram = _m3_unlock();
            ram[0] = 0x1234;
            if(ram[0] == 0x1234){
                _m3_lock();
                _unlock = _m3_unlock;
                _lock   = _m3_lock;
                break;
            }
			_ram_type = EZ_RAM;
			// try ez plus
            ram = _ez_ex_unlock();
            ram[0] = 0x1234;
            if(ram[0] == 0x1234){				
				ram = (vu16*)0x08000000;
				ram[0] = 0x4321;
				if(ram[0] != 0x4321)    // test non-writability
				{
					_ez_lock();
					_unlock = _ez_ex_unlock;
					_lock   = _ez_lock;
					break;
				}				
				_ez_lock();
            }
			
			// try ez
            ram = _ez_unlock();
            ram[0] = 0x1234;
            if(ram[0] == 0x1234){
                _ez_lock();
                _unlock = _ez_unlock;
                _lock   = _ez_lock;
                break;
            }
			_ram_type = OPERA_RAM;
            // try opera
            ram = _opera_unlock();
            ram[0] = 0x1234;
            if(ram[0] == 0x1234){
                _opera_lock();
                _unlock = _opera_unlock;
                _lock   = _opera_lock;
                break;
            }
			_ram_type = G6_RAM;
            // try g6
            ram = _g6_unlock();
            ram[0] = 0x1234;
            if(ram[0] == 0x1234){
                _g6_lock();
                _unlock = _g6_unlock;
                _lock   = _g6_lock;
                break;
            }
			_ram_type = 0;
			//use local ram
			if(!(pfb->flags & FBO_USELRAM_AVATAR))
				return false;
			_unlock = _lr_unlock;
			_lock = _lr_lock;
    }
	// detect RAM size !   
	if(_ram_type == 0){
		unsigned long sz;
		
		sz = 600 * 1024;
		ram = calloc(sz,1);
		if(ram == NULL)
			return FALSE;
		_ram_type = LOCAL_RAM;
		_size = sz;
	}	
    else{
		ram = _unlock();    
		do {
			ram[_size] = 0;
			_size += 512;
			ram[_size] = 0x1234;
		} while(ram[_size] == 0x1234);
		_size <<= 1;	
	}
    if(_size){
		unsigned int i;
		
		ramblocks_list = (LPRAMBLOCKLIST)ram;
		memset((void *)ram,0,sizeof(RAMBLOCKLIST));
		i = ((_size * 75) >> 10) & ~3;
		ramblocks_list->size = i - sizeof(RAMBLOCKLIST);
		ramblocks_list->size_ram = _size - i;
		ramblocks_list->adr_ram = i;
		PA_LoadSprite16cPal(0,15,(void*)ramPal);					
		PA_CreateSprite(0,127,(void*)ramTiles,OBJ_SIZE_16X8,0,15,256-16,192-16-8);					
	}	
	//_lock();
    return true;
}
//---------------------------------------------------------------------------------
u32 ram_size()
{
	return _size;
}
//---------------------------------------------------------------------------------
vu16* ram_unlock()
{
	if(_unlock)
		return _unlock();
	return 0;
}
//---------------------------------------------------------------------------------
void ram_lock()
{
	if(_lock)
		_lock();
}
//---------------------------------------------------------------------------------
static void sort_block()
{
	LPRAMBLOCK p,p1;

	if(ramblocks_list == NULL)
		return;
	p = ramblocks_list->first;
	for(;p != NULL;){
		if(!(p->status & 0x80000000)){
			p1 = (LPRAMBLOCK)p->next;
			for(;p1 != NULL;){
				if(!(p1->status & 0x80000000) && p1->adr < p->adr){
					LPRAMBLOCK prev,next,prev_1,next_1;

					//swap
					prev_1 = (LPRAMBLOCK)p1->prev;
					next_1 = (LPRAMBLOCK)p1->next;
					
					prev = (LPRAMBLOCK)p->prev;
					next = (LPRAMBLOCK)p->next;
					
					if(next_1 != NULL)
						next_1->prev = prev;
					if(prev_1 != NULL)
						prev_1->next = next;

					if(prev != NULL)
						prev->next = next_1;
					if(next != NULL)
						next->prev = prev_1;

					if(ramblocks_list->first == p)
						ramblocks_list->first = p1;
					if(ramblocks_list->last == p1)
						ramblocks_list->last = p;
					
					prev = p1;				
					p1 = p;
					p = prev;
				}
				p1 = (LPRAMBLOCK)p1->next;
			}
		}
		p = (LPRAMBLOCK)p->next;
	}
}
//---------------------------------------------------------------------------------
void *ram_disk_alloc(unsigned int size)
{
	LPRAMBLOCK p,p1;
	unsigned long adr;
	
	if(ramblocks_list == NULL)
		return NULL;
	//_unlock();
	p = NULL;
	if((ramblocks_list->size - ramblocks_list->used) < sizeof(RAMBLOCK))
		goto ex_local_alloc;
	p1 = ramblocks_list->first;
	adr = ramblocks_list->adr_ram;
	while(p1 != NULL){
		adr += ((p1->size + 1) & ~1);
		p1 = (LPRAMBLOCK)p1->next;
	}
	if((_size - adr) < size)
		return NULL;
	p = (LPRAMBLOCK)(ramblocks_list + 1);
	{
		unsigned long index;
		
		for(index=0;index < ramblocks_list->items;index++){
			if(p[index].status & 0x80000000){
				p += index;
				break;
			}
		}
		if(index == ramblocks_list->items){
			p = &p[ramblocks_list->items++];
			ramblocks_list->used += sizeof(RAMBLOCK);
		}		
	}	
	p->adr = adr;
	p->size = size;
	p->status = 0;
	p->next = NULL;
	p->prev = ramblocks_list->last;
	if(ramblocks_list->first == NULL)
		ramblocks_list->first = p;
	if(ramblocks_list->last != NULL)
		ramblocks_list->last->next = p;
	ramblocks_list->last = p;
	sort_block();
ex_local_alloc:	
	//_lock();	
	return p;
}
//---------------------------------------------------------------------------------
unsigned short *ram_disk_lock(void *mem)
{
	LPRAMBLOCK p;
	char *c;
	
	if(mem == NULL || ramblocks_list == NULL)
		return NULL;
	//_unlock();
	p = (LPRAMBLOCK)mem;
	p->status |= 3;//locked | used
	c = (char *)ramblocks_list;
	c += p->adr;
	return (unsigned short *)c;
}
//---------------------------------------------------------------------------------
int ram_disk_unlock(void *mem)
{
	LPRAMBLOCK p;
	int res;
	
	if(ramblocks_list == NULL)
		return -1;
	res = -2;
	if(mem != NULL){
		p = (LPRAMBLOCK)mem;
		p->status &= ~1;
		res = 0;
	}
	//_lock();
	return res;
}
//---------------------------------------------------------------------------------
void ram_disk_memcpy(void *dst,void *src,unsigned int size)
{
	register u16 *p,*p1;
	
	size = ((size + 1) & ~3) >> 1;
	p = (u16 *)dst;
	p1 = (u16 *)src;
	for(;size > 0;size--)
		*p++ = *p1++;
}
//---------------------------------------------------------------------------------
int ram_disk_free(void *mem)
{
	LPRAMBLOCK p;
	unsigned long adr,start;
	char *c;
	
	if(ramblocks_list == NULL)
		return -1;
	if(mem == NULL)
		return -2;
	//_unlock();	
	p = (LPRAMBLOCK)mem;
	p->status |= 0x80000000;//deleted
			
	if(ramblocks_list->first == p)
		ramblocks_list->first = p->next;
	if(ramblocks_list->last == p){
		if(p->next != NULL)
			ramblocks_list->last = p->next;
		else
			ramblocks_list->last = p->prev;
	}
	
	if(p->next != NULL)
		((LPRAMBLOCK)p->next)->prev = p->prev;
	if(p->prev != NULL)
		((LPRAMBLOCK)p->prev)->next = p->next;	
		
	p = ramblocks_list->first;
	adr = ramblocks_list->adr_ram;
	start = 0;
	c = (char *)ramblocks_list;
	while(p != NULL){		
		if((p->status & 1) == 0){						
			start = 1;
			if((p->status & 0x80000000) == 0 && adr != p->adr){														
				if((p->status & 2))
					ram_disk_memcpy(&c[adr],&c[p->adr],p->size);//fix me
				p->adr = adr;
			}
		}
		else if(!start)
			adr = p->adr;
		if((p->status & 0x80000000) == 0)
			adr += ((p->size + 1) & ~1);
		p = p->next;
	}
	sort_block();	
	
	//_lock();
	return 0;
}
