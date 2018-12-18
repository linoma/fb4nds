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
#include "windows.h"

#ifndef __DRAWSH__
#define __DRAWSH__

#define DT_CALCRECT		1
#define DT_VCENTER		2
#define DT_LEFT			0
#define DT_CENTER		4
#define DT_RIGHT		8
#define DT_EDITCONTROL	16
#define DT_SINGLELINE	32
#define DT_WORDBREAK	64

int draw_text(u8 screen,char *txt,int len,LPRECT lprc,unsigned int flags);
int get_text_extent(u8 screen,char *txt,int len,LPSIZE p);
u16 set_text_color(u8 screen,u16 color);
int stretch_blt(u16 *out_buffer,int x,int y,int width,int width_byte,int height,u8 *image);
void draw_line(u8 screen, int x0, int y0, int x1, int y1, u16 color);
void fill_rect(u8 screen,LPRECT lprc,u16 color);
void draw_rect(u8 screen,LPRECT lprc,u16 color);
void rect(u8 screen,LPRECT lprc,u16 color,u16 border_color);
void fill_circle(u8 screen,int x,int y,int rad,u16 color);
int draw_image(u8 screen,int x,int y,int width,int height,u8 *image,int tr_color,int alpha);
int set_text_font(u8 screen,int font);
int get_font_height(u8 screen);
int draw_image_rotate(u8 screen,int x,int y,int width,int height,u8 *image,int degree,int zoom,int tr_color);
int tile_to_bitmap(u32 *tiles,u16 *pal,int colors,int width,int height,u16 *dst);
int draw_tiles(u8 screen,int x,int y,int width,int height,u8 *tiles,u16 *pal,u8 colors,int alpha);
int get_text_font(u8 screen);

#endif
