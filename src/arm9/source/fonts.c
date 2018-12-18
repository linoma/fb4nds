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
 
#include "fonts.h"
#include "fb.h"

FBFONT fonts[4];
//---------------------------------------------------------------------------------
static int bs_cmpfnc(const LPFBGLYPH a,const LPFBGLYPH b)
{
	return (int)a->id - (int)b->id;
}
//---------------------------------------------------------------------------------
LPFBGLYPH bdffont_get_glyph(unsigned char idx_font,unsigned int chr)
{
	return (LPFBGLYPH)bsearch(&chr,fonts[idx_font].glyphs,fonts[idx_font].n_glyph,sizeof(FBGLYPH),(int (*)(const void *,const void *))bs_cmpfnc);
}
//---------------------------------------------------------------------------------
static int bdffont_get_width(unsigned char idx_font,unsigned int chr)
{
	LPFBGLYPH p;
	
	if(fonts[idx_font].flags & 1)
		p = bsearch(&chr,fonts[idx_font].glyphs,fonts[idx_font].n_glyph,sizeof(FBGLYPH),(int (*)(const void *,const void *))bs_cmpfnc);
	else{
		unsigned int ch;
		
		if(chr >= fonts[idx_font].start_id && (ch = chr - fonts[idx_font].start_id) < fonts[idx_font].n_glyph)
			p = &fonts[idx_font].glyphs[ch];
		else
			p = NULL;
	}
	if(p != NULL){
		if(p->ofs == 0)
			return fonts[idx_font].width >> 1;
		return p->width;
	}
	return 0;
}
//---------------------------------------------------------------------------------
static int bdffont_draw_glyph(unsigned char idx_font,unsigned char screen,int x,int y,int max_y,unsigned int chr,unsigned short color)
{
	LPFBGLYPH p;
	u8 ly,lx,*data,i1,ly1;
	u16 i,j,j1,width_bytes;
		
	if(fonts[idx_font].flags & 1)
		p = bsearch(&chr,fonts[idx_font].glyphs,fonts[idx_font].n_glyph,sizeof(FBGLYPH),(int (*)(const void *,const void *))bs_cmpfnc);
	else{
		unsigned int ch;
		
		if(chr >= fonts[idx_font].start_id && (ch = chr - fonts[idx_font].start_id) < fonts[idx_font].n_glyph)
			p = &fonts[idx_font].glyphs[ch];
		else
			p = NULL;
	}
	if(p == NULL)
		return -1;
	ly = fonts[idx_font].height;
	ly1 = ly - p->height - p->y;
	ly += ly1;
	lx = p->width;
	width_bytes = lx >> 3;
	if((width_bytes << 3) < lx)
		width_bytes++;	
	data = p->bitmap;
	if(screen == 0){
		for(j = j1 = 0; j < ly; j++){
			if((j+y) > max_y)
				return -1;
			if(j < ly1)
				continue;			
			for(i1=128,i = 0;i < lx;i++){
				if(data[i >> 3] & i1){
					u8 decal = (((x + i) & 1) << 3);
					
					PA_DrawBg[0][((y + j) << 7) + ((x+i)>>1)] &= 255 << (8 - decal);
					PA_DrawBg[0][((y + j) << 7) + ((x+i)>>1)] |= color << decal;
				}
				if(!(i1 >>= 1))
					i1 = 128;								
			}
			data += width_bytes;
			j1++;
			if(j1 >= p->height)
				break;			
		}
	}
	else{
		for(j = j1 = 0; j < ly; j++){
			if((j+y) > max_y)
				return -1;
			if(j < ly1)
				continue;
			for(i1=128,i = 0;i < lx;i++){
				if(data[i >> 3] & i1)
					PA_Put16bitPixel(screen,x+i,y+j,color);
				if(!(i1 >>= 1))
					i1 = 128;				
			}
			data += width_bytes;
			j1++;
			if(j1 >= p->height)
				break;
		}
	}
	return 0;
}
//---------------------------------------------------------------------------------
static int sysfont_draw_glyph(unsigned char idx_font,unsigned char screen,int x,int y,int max_y,unsigned int chr,unsigned short color)
{
	u8 *data,ly,lx;
	u16 i,j;
	
	data = (u8*)(((u8 *)fonts[0].glyphs) + (def_font_Map[chr]<<6));
	lx = def_font_Sizes[chr];
	ly = fonts[0].height;
	if(screen == 0){						
		for(j = 0; j < ly; j++){
			if((j+y) > max_y)
				return -1;
			for(i = 0; i < lx; i++){
				u8 decal = (((x+i) & 1) << 3);
				u16 pixel = (data[i+(j<<3)]*color) << decal;
				if(pixel){
					PA_DrawBg[0][((y + j) << 7) + ((x+i)>>1)] &= 255<<(8-decal);
					PA_DrawBg[0][((y + j) << 7) + ((x+i)>>1)] |= pixel;
				}
			}
		}			
	}
	else{
		for(j = 0;j < ly;j++){
			if((j+y) > max_y)
				return -1;
			for(i = 0;i < lx;i++){
				if(data[i+(j<<3)]) 
					PA_Put16bitPixel(screen, x+i, y+j, data[i+(j<<3)]*color);
			}
		}		
	}
	return 0;
}
//---------------------------------------------------------------------------------
static int sysfont_get_width(unsigned char idx_font,unsigned int chr)
{
	return def_font_Sizes[chr];
}
//---------------------------------------------------------------------------------
int init_fonts()
{
	u32 tilesize;
	int i;
	
	memset(fonts,0,sizeof(fonts));
	fonts[0].height = 8;
	fonts[0].pfn_get_width = (LPGETFONTWIDTH)sysfont_get_width;
	fonts[0].pfn_draw_glyph = (LPDRAWFONTGLYPH)sysfont_draw_glyph;
	fonts[0].loaded = 1;
	tilesize = sizeof(def_font_Tiles) << 3;
	fonts[0].glyphs = (LPFBGLYPH)malloc(tilesize);
	if(fonts[0].glyphs  == NULL)
		return -1;
	for(i = 0;i < tilesize;i++)
		((u8 *)fonts[0].glyphs)[i] = (def_font_Tiles[i>>3] >> (i & 7) ) & 1;	
	return 0;
}
//---------------------------------------------------------------------------------
void destroy_fonts()
{
	int i;
	
	for(i=0;i<4;i++)
		free_font(i);
}
//---------------------------------------------------------------------------------
void free_font(int slot)
{
	if(fonts[slot].glyphs != NULL)
		free(fonts[slot].glyphs);
	memset(&fonts[slot],0,sizeof(FBFONT));
}
//---------------------------------------------------------------------------------
int load_font(const char *name,int slot)
{
	FILE *fp;
	LPFBGLYPH p_f;
	u8 *bitmaps,*fonts_buffer;
	int res,mode,n_f,size,bytes_width,size_using,prev_id;
	char *s,*p;
		
	if(name == NULL || name[0] == 0)
		return -1;	
	fp = open_file(name,"rb");
	if(fp == NULL)
		return -2;	
	bitmaps = NULL;
	p_f = NULL;
	res = -2;      
	mode = size_using = size = 0;
	n_f = -1;
	prev_id = -1;
	s = (char *)temp_buffer;
	while(!feof(fp) && mode >= 0){
		if(fgets(s,1000,fp) == NULL)
			break;
		switch(mode){
			case 0:
				p = strstr(s,"FONTBOUNDINGBOX ");
				if(p != NULL){
					p += 15;
					while(!isdigit(*p) && *p != 0)	p++;					
					fonts[slot].width = atoi(p);
					while(isdigit(*p) && *p != 0) p++;
					while(!isdigit(*p) && *p != 0) p++;
					fonts[slot].height = atoi(p);
					fonts[slot].pfn_get_width = (LPGETFONTWIDTH)bdffont_get_width;
					res = -3;
				}
				p = strstr(s,"CHARS ");
                if(p != NULL){
					p += 5;
                    while(!isdigit(*p) && *p != 0)
						p++;
					if(*p != 0){
						unsigned int i;

						fonts[slot].n_glyph = atoi(p);						
						size = fonts[slot].width >> 3;
						if((size << 3) < fonts[slot].width)
							size++;
						size *= fonts[slot].height * fonts[slot].n_glyph;
						
						fonts_buffer = malloc(fonts[slot].n_glyph * sizeof(FBGLYPH) + size);
						memset(fonts_buffer,0,fonts[slot].n_glyph * sizeof(FBGLYPH) + size);
						
						fonts[slot].glyphs = (LPFBGLYPH)fonts_buffer;
						fonts[slot].pfn_draw_glyph = (LPDRAWFONTGLYPH)bdffont_draw_glyph;
						
						i = (unsigned int)fonts_buffer;
						i += fonts[slot].n_glyph * sizeof(FBGLYPH);
						i = (i + 3) & ~3;
						
						bitmaps = (unsigned char *)i;
						mode = 1;
						res = -4;
					}
				}
			break;
			case 1:
				p = strstr(s,"ENCODING ");
				if(p != NULL){
					p += 8;
					while(!isdigit(*p) && *p != 0)
						p++;
					if(*p != 0){
						p_f = &fonts[slot].glyphs[++n_f];
						p_f->id = atoi(p);
						if(prev_id != -1){
							if((p_f->id - prev_id) != 1)
								fonts[slot].flags |= 1;//need binary search
						}
						else
							fonts[slot].start_id = p_f->id;
						prev_id = p_f->id;
						p_f->bitmap = bitmaps;
						p_f->ofs = 0;
						mode = 2;
						res = -5;
					}
				}
				else if(strstr(s,"ENDFONT")){				
					mode = -2;
					res = 0;
					fonts[slot].loaded = 1;
				}
			break;
			case 2:
				p = strstr(s,"BBX ");
				if(p != NULL){
					p += 3;
					while(*p == 32 && *p != 0)
						p++;
					if(*p != 0){
						p_f->width = atoi(p);
						while((isdigit(*p) || *p == '-') && *p != 0)
							p++;
						while(!isdigit(*p) && *p != '-' && *p != 0)
							p++;
						p_f->height = atoi(p);
						while((isdigit(*p) || *p == '-') && *p != 0)
							p++;
						while(!isdigit(*p) && *p != '-' && *p != 0)
							p++;
						p_f->x = atoi(p);
						while((isdigit(*p) || *p == '-') && *p != 0)
							p++;
						while(!isdigit(*p) && *p != '-' && *p != 0)
							p++;
						p_f->y = atoi(p);

						bytes_width = p_f->width >> 3;
						if((bytes_width << 3) < p_f->width)
							bytes_width++;
						mode = bytes_width * p_f->height;
						size_using += mode;
						bitmaps += mode;
						if(size_using > size){
							mode = -1;
							res = -6;
							break;
						}
						mode = 3;
					}
				}
			break;
			case 3:
				p = strstr(s,"BITMAP");
				if(p != NULL){
					p += 6;
					mode = 4;
				}
			break;
			default:
				mode++;
				if(mode > p_f->height + 4 || strstr(s,"ENDCHAR"))
					mode = 1;
				else{				
					p = s;
					while(*p != 0 && *p != '\r' && *p != '\n'){
						unsigned int value;

						sscanf(p,"%02X",&value);
						p_f->bitmap[p_f->ofs++] = value;
						p += 2;
					}					
				}
			break;
		}
	}
	fclose(fp);
	return res;
}
//---------------------------------------------------------------------------------
int load_fonts()
{	
	char *s;
	
	if(pfb->font_name[0] == 0)
		return 0;	
	SetWindowText(3,"Loading font...");
	s = (char *)temp_buffer;	
	strcpy(s,"data/");
	strcat(s,pfb->font_name);
	return load_font(s,1);
}
