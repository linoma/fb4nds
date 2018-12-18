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
#include <PA9.h> 
#include "all_gfx.h"

#ifndef __FONTSH__
#define __FONTSH__

typedef struct sFBGLYPH{
	u16 id;
	u8 width;
	u8 height;
	s8 x;
	s8 y;
	u8 ofs;
	u8 *bitmap;	
} __attribute__ ((__packed__))  FBGLYPH; 

typedef FBGLYPH *LPFBGLYPH;
typedef int (*LPGETFONTWIDTH)(unsigned char,unsigned int);
typedef int (*LPDRAWFONTGLYPH)(unsigned char,unsigned char,int,int,int,unsigned int,unsigned short);

typedef struct {
	u8 width;
	u8 height;
	u32 n_glyph;
	u8 loaded;
	u32 flags;
	u16 start_id;
	LPGETFONTWIDTH pfn_get_width;
	LPDRAWFONTGLYPH pfn_draw_glyph;
	LPFBGLYPH glyphs;
} FBFONT,*LPFBFONT;

extern int load_fonts();
extern int init_fonts();
extern int load_font(const char *name,int slot);
extern void free_font(int slot);
extern LPFBGLYPH bdffont_get_glyph(unsigned char idx_font,unsigned int chr);
extern void destroy_fonts();

extern FBFONT fonts[4];

#endif
 