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

#include "draws.h"
#include <gif_lib.h>
#include <png.h>
#include "fb.h"
#include "fonts.h"

extern u8 *temp_buffer;
static u16 text_colors[2] = {4,PA_RGB(31,31,31)};
static LPFBFONT text_fonts[2] = {&fonts[0],&fonts[0]};
extern const short InterlacedOffset[];
extern const short InterlacedJumps[];
static unsigned char *fileBytes;
static u8 line_buffer[256];
//---------------------------------------------------------------------------------
int get_font_height(u8 screen)
{
	return text_fonts[screen]->height;
}
//---------------------------------------------------------------------------------
int get_text_font(u8 screen)
{
	if(screen > 1)
		return -1;
	return (int)(((unsigned int)text_fonts[screen] - (unsigned int)fonts) / sizeof(FBFONT));
}
//---------------------------------------------------------------------------------
int set_text_font(u8 screen,int font)
{
	LPFBFONT new;
	int old;
	
	if(screen > 1)
		return 0;
	if(font < 0 || font > 3)
		font = 0;
	new = &fonts[font];
	if(!new->loaded){
		font = 0;
		new = fonts;
	}		
	old = (int)(((unsigned int)text_fonts[screen] - (unsigned int)fonts) / sizeof(FBFONT));
	text_fonts[screen] = new;
	return old;
}
//---------------------------------------------------------------------------------
u16 set_text_color(u8 screen,u16 color)
{
	u16 old;
	
	if(screen > 1)
		return (u16)-1;
	old = text_colors[screen];
	text_colors[screen] = color;
	return old;
}
//---------------------------------------------------------------------------------
int get_text_extent(u8 screen,char *txt,int len,LPSIZE p)
{
	int i,x,x1,idx_font;
	u8 letter,ly,el;
	
	if(txt == NULL || len < 1 || *txt == 0 || p == NULL)
		return -1;
	idx_font = (int)(((unsigned int)text_fonts[screen] - (unsigned int)fonts) / sizeof(FBFONT));
	ly = text_fonts[screen]->height;
	p->cx = p->cy = 0;
	x = 0;
	el = 0;
	for(i=0;i<len;){
		letter = translate_UTF(&txt[i],&x1);						
		i += x1;
		if(letter == '\r'){
			if(p->cx < x)
				p->cx = x;
			x = 0;
			p->cy += ly;
			i++;
			el = 1;
			continue;
		}
		else if(letter == '\n'){
			if(p->cx < x)
				p->cx = x;
			x = 0;
			p->cy += ly;
			el = 1;
			continue;
		}
		x += text_fonts[screen]->pfn_get_width(idx_font,letter);
		el = 0;
	}	
	if(p->cx < x)
		p->cx = x;
	if(!el)
		p->cy += ly;
	return 0;	
}
//---------------------------------------------------------------------------------
int draw_text(u8 screen,char *txt,int len,LPRECT lprc,unsigned int flags)
{
	unsigned long cr;
	int x,y,n,x_start,x_max,x1,idx_font;
	u8 lx,letter,ly;
	u16 color;
	
	if(txt == NULL || screen > 1 || lprc == NULL)
		return 0;
	EnterCriticalSection(&cr);
	if(len == -1)
		len = strlen_UTF(txt);
	idx_font = (int)(((unsigned int)text_fonts[screen] - (unsigned int)fonts) / sizeof(FBFONT));
	
	ly = text_fonts[screen]->height;
	color = text_colors[screen];
	x_start = lprc->left;
	if(flags & DT_CENTER){
		SIZE sz;
		
		get_text_extent(screen,txt,len,&sz);
		x_start = lprc->left + (((lprc->right - lprc->left) - sz.cx) >> 1);
	}
	else if(flags & DT_RIGHT){
		SIZE sz;
		
		get_text_extent(screen,txt,len,&sz);
		x_start = lprc->right - sz.cx;
	}
	x = x_start;
	x_max = x;
	y = lprc->top;
	if(flags & DT_VCENTER){
		SIZE sz;
		
		get_text_extent(screen,txt,len,&sz);
		y += ((lprc->bottom - lprc->top) - sz.cy) >> 1;
	}
	for(n=0;n<len;){
		if(!(flags & DT_CALCRECT)){
			if(y >= lprc->bottom)
				break;
		}
		letter = translate_UTF(&txt[n],&x1);				
		n += x1;
		if(letter == '\r'){
			if(x > x_max)
				x_max = x;
			x = x_start;
			y += ly;
			n++;
			continue;
		}
		else if(letter == '\n'){
			if(x > x_max)
				x_max = x;
			x = x_start;
			y += ly;
			continue;
		}
		else if(letter == '\t')
			letter = 32;		
		lx = text_fonts[screen]->pfn_get_width(idx_font,letter);
		if((x+lx) > lprc->right){
			if(x > x_max)
				x_max = x;		
			if(flags & DT_SINGLELINE)
				break;				
			x = x_start;
			y += ly;
		}			
		if(!(flags & DT_CALCRECT))
			text_fonts[screen]->pfn_draw_glyph(idx_font,screen,x,y,lprc->bottom,letter,color);
		x += lx;
		if(letter == 32 && (flags & DT_WORDBREAK)){
			int i1;
			
			x1 = x;
			for(i1 = n+1;i1<len;i1++){
				letter = txt[i1];
				if(isalpha(letter)){
					x1 += text_fonts[screen]->pfn_get_width(idx_font,letter);
					if(x1 > lprc->right){
						if(x > x_max)
							x_max = x;						
						x = x_start;
						y += ly;
						break;
					}
				}
				else
					break;
			}
		}
	}	
	if(flags & DT_CALCRECT){
		lprc->right = x_max;
		lprc->bottom = y+ly;
	}
	LeaveCriticalSection(&cr);
	return n;
}
//---------------------------------------------------------------------------------
static void copy_line(u32 *src,u32 *dst,int size,int mode)
{
	if(mode){
		dst[0] = src[0];
		dst[size-1] = src[size-1];
	}
	else{
		int i;
		
		for(i=0;i<size;i++)
			*dst++ = *src++;
	}
}
//---------------------------------------------------------------------------------
void rect(u8 screen,LPRECT lprc,u16 color,u16 border_color)
{
	int x,y;
	unsigned long cr;
	
	EnterCriticalSection(&cr);
	if(screen == 0){
		u8 *buffer,*p;
		int ofs,size;
		
		buffer = &((u8 *)PA_DrawBg[0])[lprc->top << 8];
		ofs = lprc->left & ~3;
		x = lprc->right - lprc->left;
		size = (x + 7) >> 2;		
		p = line_buffer + lprc->left;
		copy_line((u32 *)&buffer[ofs],(u32 *)&line_buffer[ofs],size,1);
		for(;x > 0;x--)
			*p++ = border_color;
		copy_line((u32 *)&line_buffer[ofs],(u32 *)&buffer[ofs],size,0);
		buffer += 256;
		y = (lprc->bottom-1) - (lprc->top+1);		
		for(;y > 0;y--){			
			copy_line((u32 *)&buffer[ofs],(u32 *)&line_buffer[ofs],size,1);
			p = line_buffer + lprc->left;
			*p++ = border_color;
			x = (lprc->right-1) - (lprc->left+1);
			for(;x > 0;x--)
				*p++ = color;
			*p = border_color;
			copy_line((u32 *)&line_buffer[ofs],(u32 *)&buffer[ofs],size,0);
			buffer += 256;
		}
		p = line_buffer + lprc->left;
		x = lprc->right - lprc->left;
		copy_line((u32 *)&buffer[ofs],(u32 *)&line_buffer[ofs],size,1);
		for(;x > 0;x--)
			*p++ = border_color;		
		copy_line((u32 *)&line_buffer[ofs],(u32 *)&buffer[ofs],size,0);
	}
	else{
		u16 *buffer,*p;
		
		buffer = &((u16 *)PA_DrawBg[1])[lprc->top << 8];
		p = buffer + lprc->left;
		x = lprc->right - lprc->left;
		for(;x > 0;x--)
			*p++ = border_color;
		buffer += 256;
		y = (lprc->bottom-1) - (lprc->top+1);
		for(;y > 0;y--){
			p = buffer + lprc->left;
			*p++ = border_color;
			x = (lprc->right-1) - (lprc->left+1);
			for(;x > 0;x--)
				*p++ = color;
			*p = border_color;
			buffer += 256;
		}
		p = buffer + lprc->left;
		x = lprc->right - lprc->left;
		for(;x > 0;x--)
			*p++ = border_color;		
	}
	LeaveCriticalSection(&cr);
}
//---------------------------------------------------------------------------------
void draw_rect(u8 screen,LPRECT lprc,u16 color)
{
	draw_line(screen,lprc->left,lprc->top,lprc->right-1,lprc->top,color);
	draw_line(screen,lprc->right-1,lprc->top,lprc->right-1,lprc->bottom-1,color);
	draw_line(screen,lprc->right-1,lprc->bottom-1,lprc->left,lprc->bottom-1,color);
	draw_line(screen,lprc->left,lprc->bottom-1,lprc->left,lprc->top,color);
}
//---------------------------------------------------------------------------------
void fill_rect(u8 screen,LPRECT lprc,u16 color)
{
	int x,y;
	unsigned long cr;
	
	EnterCriticalSection(&cr);
	if(screen == 0){
		u8 *buffer,*p;
		int ofs,size;
		
		buffer = &((u8 *)PA_DrawBg[0])[lprc->top << 8];
		ofs = lprc->left & ~3;
		x = lprc->right - lprc->left;
		size = (x + 7) >> 2;		
		y = lprc->bottom - lprc->top;
		for(;y > 0;y--){
			p = line_buffer + lprc->left;
			x = lprc->right - lprc->left;
			copy_line((u32 *)&buffer[ofs],(u32 *)&line_buffer[ofs],size,1);			
			for(;x > 0;x--)
				*p++ = color;
			copy_line((u32 *)&line_buffer[ofs],(u32 *)&buffer[ofs],size,0);
			buffer += 256;
		}
	}
	else{
		u16 *buffer,*p;
		
		buffer = &((u16 *)PA_DrawBg[1])[lprc->top << 8];
		y = lprc->bottom - lprc->top;
		for(;y > 0;y--){
			p = buffer + lprc->left;
			x = lprc->right - lprc->left;
			for(;x > 0;x--)
				*p++ = color;
			buffer += 256;
		}
	}
	LeaveCriticalSection(&cr);
}
//---------------------------------------------------------------------------------
void circle(u8 screen,int x,int y,int rad,u16 color,u16 border_color)
{
	int x1,y1,x1_2,y1_2,rad_2;
	unsigned long cr;
	
	EnterCriticalSection(&cr);
	rad_2 = rad * rad;
	if(screen == 0){
		u8 *buffer,*p;
		int ofs,size;
		
		ofs = x & ~3;
		size = rad >> 1;		
		buffer = (u8 *)PA_DrawBg[0];
		buffer += (y - rad) << 8;
		for(y1=-rad;y1<=rad;y1++){
			y1_2 = y1 * y1;			
			p = line_buffer + x - rad;
			copy_line((u32 *)&buffer[ofs],(u32 *)&line_buffer[ofs],size,0);
			for(x1=-rad;x1<=rad;x1++,p++){
				x1_2 = x1 * x1 + y1_2;
				if(x1_2 < rad_2)
					*p = (u8)color;
				else if(x1_2 == rad_2)
					*p = (u8)border_color;
			}
			copy_line((u32 *)&line_buffer[ofs],(u32 *)&buffer[ofs],size,0);
			buffer += 256;			
		}
	}
	else{
		u16 *buffer,*p;
	
		buffer = &((u16 *)PA_DrawBg[1])[(y - rad) << 8];
		for(y1=-rad;y1<=rad;y1++){
			y1_2 = y1 * y1;			
			p = buffer + x - rad;
			for(x1=-rad;x1<=rad;x1++,p++){
				x1_2 = x1 * x1 + y1_2;
				if(x1_2 < rad_2)
					*p = color;
				else if(x1_2 == rad_2)
					*p = border_color;					
			}
			buffer += 256;			
		}
	}
	LeaveCriticalSection(&cr);
}
//---------------------------------------------------------------------------------
void fill_circle(u8 screen,int x,int y,int rad,u16 color)
{
	int x1,y1,x1_2,y1_2,rad_2;
	unsigned long cr;
	
	EnterCriticalSection(&cr);
	rad_2 = rad * rad;
	if(screen == 0){
		u8 *buffer,*p;
		int ofs,size;
		
		ofs = x & ~3;
		size = rad >> 1;		
		buffer = (u8 *)PA_DrawBg[0];
		buffer += (y - rad) << 8;
		for(y1=-rad;y1<=rad;y1++){
			y1_2 = y1 * y1;			
			p = line_buffer + x - rad;
			copy_line((u32 *)&buffer[ofs],(u32 *)&line_buffer[ofs],size,0);
			for(x1=-rad;x1<=rad;x1++,p++){
				x1_2 = x1 * x1 + y1_2;
				if(x1_2 <= rad_2)
					*p = (u8)color;
			}
			copy_line((u32 *)&line_buffer[ofs],(u32 *)&buffer[ofs],size,0);
			buffer += 256;			
		}
	}
	else{
		u16 *buffer,*p;
	
		buffer = &((u16 *)PA_DrawBg[1])[(y - rad) << 8];
		for(y1=-rad;y1<=rad;y1++){
			y1_2 = y1 * y1;			
			p = buffer + x - rad;
			for(x1=-rad;x1<=rad;x1++,p++){
				x1_2 = x1 * x1 + y1_2;
				if(x1_2 <= rad_2)
					*p = color;
			}
			buffer += 256;			
		}
	}
	LeaveCriticalSection(&cr);
}
//---------------------------------------------------------------------------------
void draw_line(u8 screen, int x0, int y0, int x1, int y1, u16 color) 
{ 
	int stepx;
	int stepy;
	int dy = y1 - y0;
	int dx = x1 - x0;
	
	if(dy < 0){
		dy = -dy;
		stepy = -1;
	} 
	else
		stepy = 1;
	if(dx < 0){
		dx = -dx;
		stepx = -1;
	} 
	else
		stepx = 1;
	dy <<= 1;
	dx <<= 1;	
	if(screen == 0)
		((u8 *)PA_DrawBg[0])[x0 + (y0<<8)] = color;
	else
		((u16 *)PA_DrawBg[1])[x0 + (y0<<8)] = color;		
	if(dx > dy){
		int fraction = dy - (dx >> 1);
		while(x0 != x1){
			if(fraction >= 0){
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			if(screen == 0)
				((u8 *)PA_DrawBg[0])[x0 + (y0<<8)] = color;
			else
				((u16 *)PA_DrawBg[1])[x0 + (y0<<8)] = color;
		}
	} 
	else{
		int fraction = dx - (dy >> 1);
		while(y0 != y1){
			if(fraction >= 0){
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			if(screen == 0)
				((u8 *)PA_DrawBg[0])[x0 + (y0<<8)] = color;
			else
				((u16 *)PA_DrawBg[1])[x0 + (y0<<8)] = color;
		}
	}
}
//---------------------------------------------------------------------------------
static int readFunc_gif(GifFileType* GifFile, GifByteType* buf, int count)
{
   char* ptr = (char*)GifFile->UserData;
   memcpy(buf,ptr,count);
   GifFile->UserData = ptr + count;
   return count;
}
//---------------------------------------------------------------------------------
static int stretch_blt_gif(u16 *out_buffer,int x,int y,int width,int width_byte,int height,u8 *image)
{
	int ExtCode,w,h,f_wstep,f_ihstep,i,j,x1,x2,i1;
	u16 *palette;
	GifFileType *GifFile;
	GifRecordType RecordType;
	GifByteType *Extension;
	ColorMapObject *ColorMap;
	GifPixelType *LineBuf;
		
	GifFile = DGifOpen((void *)image,readFunc_gif);
	if(GifFile == NULL)
		return -1;
	palette = NULL;
	out_buffer += (width_byte * y) + x;
	LineBuf = (GifPixelType *)temp_buffer;
	do{
		DGifGetRecordType(GifFile, &RecordType);	
		switch(RecordType){
			case IMAGE_DESC_RECORD_TYPE:
				DGifGetImageDesc(GifFile);
				w = GifFile->Image.Width;
				h = GifFile->Image.Height;
				f_wstep = (w << 12) / width;
				f_ihstep = (height << 12) / h;
				if(palette == NULL){
					palette = (u16 *)(LineBuf + 2048);
					ColorMap = GifFile->Image.ColorMap != NULL ? GifFile->Image.ColorMap : GifFile->SColorMap;
					i = ColorMap->ColorCount;
					while(--i >= 0){
						GifColorType* pColor = &ColorMap->Colors[i];
						palette[i] = PA_RGB(pColor->Red>>3,pColor->Green>>3, pColor->Blue>>3);
					}					
				}				
				if(GifFile->Image.Interlace) {
					for(i = 0;i < 4;i++){
						j = InterlacedOffset[i];
						y = j * f_ihstep;
						for(;j < h;j += InterlacedJumps[i]){
							DGifGetLine(GifFile,LineBuf,w);
							for(x1 = x2 = 0;x2 < width;x2++){
								out_buffer[(y >> 12) * width_byte + x2] = palette[LineBuf[x1 >> 12]];
								x1 += f_wstep;
							}
							x1 = y;
							x2 = y + f_ihstep;
							x2 = (x2 - x1) >> 12;
                            for(i1=1;x2 > 1;x2--,i1++)
                               memcpy(&out_buffer[((x1 >> 12) + i1) * width_byte],&out_buffer[(x1 >> 12) * width_byte],width*sizeof(u16));
							y += InterlacedJumps[i] * f_ihstep;
						}
					}
				}
				else{
					for(y = i = 0; i < h;i++) {
						DGifGetLine(GifFile,LineBuf,w);
						for(x1=x2=0;x2 < width;x2++){
							out_buffer[(y >> 12) * width_byte + x2] = palette[LineBuf[x1 >> 12]];
							x1 += f_wstep;
						}
						x1 = y;
						y += f_ihstep;
						x2 = (y - x1) >> 12;
						for(j=1;x2 > 1;x2--,j++)
							memcpy(&out_buffer[((x1 >> 12) + j) * width_byte],&out_buffer[(x1 >> 12) * width_byte],width*sizeof(u16));
					}
				}
			break;
			case EXTENSION_RECORD_TYPE:
				DGifGetExtension(GifFile, &ExtCode, &Extension);
			break;
			case TERMINATE_RECORD_TYPE:
			break;
			default:
			break;
		}
	}while(RecordType != TERMINATE_RECORD_TYPE);	
	DGifCloseFile(GifFile);
	return 0;
}
//---------------------------------------------------------------------------------
static int jpeg_callback(int mode,int x,int y,int w_block,int h_block,void *user_param)
{
	u32 *data;
	u16 *buffer;
	int i,i1,x1;
	
	data = (u32 *)user_param;
	buffer = (u16 *)data[0];
	switch(mode){
		case 1:			
			h_block <<= 12;
			for(i=0;i < h_block;){
				x1 = 0;
				for(i1=0;i1<data[1];i1++){
					buffer[data[8] * data[6] + i1] = ((u16 *)temp_buffer)[(x1 >> 12) + ((i >> 12) * data[5])];
					x1 += (int)data[3];
				}				
				i += data[9];
				data[8]++;
			}
		break;
	}
	return 1;
}
//---------------------------------------------------------------------------------
int stretch_blt_jpeg(u16 *out_buffer,int x,int y,int width,int width_byte,int height,u8 *image)
{
    int f_ihstep,f_wstep,w,h;
	JPEG_Decoder decoder;
	JPEG_FrameHeader *frame;
	u32 data[10];
	
    if(!JPEG_Decoder_ReadHeaders(&decoder,(const unsigned char **)&image))
        return -1;
	frame = &decoder.frame;
    f_wstep = (frame->width << 12) / width;
    f_ihstep = (height << 12) / frame->height;
	out_buffer += (width_byte * y) + x;
	w = h = 0;
    data[0] = (u32)out_buffer;
	data[1] = width;
	data[2] = height;
	data[3] = f_wstep;
	data[4] = f_ihstep;	
	data[5] = frame->width;
	data[6] = width_byte;
	data[7] = frame->height;
	data[8] = 0;
	data[9] = (data[7] << 12) / data[2];
	if(!JPEG_Decoder_ReadImageEx(&decoder,(const unsigned char **)&image,(volatile short unsigned int *)temp_buffer,frame->width,frame->height,jpeg_callback,data))
        return -1;	
	return 0;
}
//---------------------------------------------------------------------------
static void readPngCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	memcpy(data,fileBytes,length);
	fileBytes += length;
}
//---------------------------------------------------------------------------------
int stretch_blt_png(u16 *out_buffer,int x,int y,int width,int width_byte,int height,u8 *image)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 w, h;
	int bitDepth, colorType,x_1,y_1,y_2;
	int f_wstep,f_hstep,f_ihstep;
	u8 *lines[1];
	int x1,x2,j;

	if(out_buffer == NULL)
		return -1;
	fileBytes = (unsigned char *)image;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,0,0,0);
	info_ptr = png_create_info_struct(png_ptr);
	if(setjmp(png_jmpbuf(png_ptr)))
		return -2;
	png_set_read_fn(png_ptr,0,readPngCallback);
	png_read_info(png_ptr,info_ptr);

	png_get_IHDR(png_ptr,info_ptr,&w,&h,&bitDepth,&colorType, 0, 0, 0);
	if(bitDepth == 16)
		png_set_strip_16(png_ptr);
	if(colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	if(bitDepth < 8)
		png_set_expand(png_ptr);
	if(png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand( png_ptr );
	if(colorType == PNG_COLOR_TYPE_GRAY)
		png_set_gray_to_rgb( png_ptr );
	if(colorType & PNG_COLOR_MASK_ALPHA)
		png_set_strip_alpha( png_ptr );
	png_read_update_info( png_ptr, info_ptr );
	png_get_IHDR( png_ptr, info_ptr, &w, &h, &bitDepth, &colorType, 0, 0, 0);

	f_wstep = (w * 4096) / width;
	f_hstep = (h * 4096) / height;
	f_ihstep = (height * 4096) / h;

	lines[0] = temp_buffer;
	for(y_2 = y_1 = 0;y_1<h;y_1++){
		png_read_rows(png_ptr,(unsigned char **)lines, 0, 1);
		for(x1=x_1=0;x_1<width;x_1++){
			int r,g,b;

			r = temp_buffer[(x1 >> 12)*3];
			g = temp_buffer[(x1 >> 12)*3 + 1];
			b = temp_buffer[(x1 >> 12)*3 + 2];
			out_buffer[(y_2 >> 12) * width_byte + x_1] = PA_RGB(r>>3,g>>3,b>>3);
			x1 += f_wstep;
		}
		x1 = y_2;
		y_2 += f_ihstep;
		x2 = (y_2 - x1) >> 12;
        for(j=1;x2 > 1;x2--,j++)
			memcpy(&out_buffer[((x1 >> 12) + j) * width_byte],&out_buffer[(x1 >> 12) * width_byte],width*sizeof(u16));
	}
	png_destroy_read_struct( &png_ptr, &info_ptr, png_infopp_NULL );	
	return 0;
}
//---------------------------------------------------------------------------------
int stretch_blt(u16 *out_buffer,int x,int y,int width,int width_byte,int height,u8 *image)
{
	int res;
	unsigned long cr;
	
	if(out_buffer == NULL || image == NULL)
		return -1;
	EnterCriticalSection(&cr);
	res = -2;
	if(image[0] == 0x47 && image[1] == 0x49 && image[2] == 0x46 && image[3] == 0x38)
		res = stretch_blt_gif(out_buffer,x,y,width,width_byte,height,image);
	else if(image[0] == 0xFF && image[1] == 0xD8 && image[2] == 0xFF && image[3] == 0xE0)
		res = stretch_blt_jpeg(out_buffer,x,y,width,width_byte,height,image);
	else if(image[0] == 0x89 && image[1] == 0x50 && image[2] == 0x4e && image[3] == 0x47)
		res = stretch_blt_png(out_buffer,x,y,width,width_byte,height,image);
	LeaveCriticalSection(&cr);	
	return res;
}
//---------------------------------------------------------------------------------
int draw_tiles(u8 screen,int x,int y,int width,int height,u8 *tiles,u16 *pal,u8 colors,int alpha)
{
	unsigned long cr;	
	int y1,x1;
	u8 index,i,*p1,tile;
	u16 *buffer,*p,color;
	
	if(tiles == NULL || pal == NULL || screen != 1)
		return -1;
	EnterCriticalSection(&cr);		
	buffer = &((u16 *)PA_DrawBg[1])[y << 8];
	for(y1 = 0;y1<height;y1++){
		p = buffer + x;
		p1 = &tiles[((y1 & 7) << 2) + ((y1 >> 3) << (3 + 5))];
		for(x1=0;x1<width;x1++,p++){
			tile = p1[((x1 >> 3) << 5) + ((x1 & 7) >> 1)];
			index = (tile >> ((x1 & 1) << 2)) & 0xF;
			if(index == 0)
				continue;
			color = pal[index];
			if(alpha != -1){
				int r,g,b;
						
				r = ((color >> 10) & 31) * alpha >> 8;
				g = ((color >> 5) & 31) * alpha >> 8;
				b = (color & 31) * alpha >> 8;
				color = *p;
				r += ((color >> 10) & 31) * (255 - alpha) >> 8;
				g += ((color >> 5) & 31) * (255 - alpha) >> 8;
				b += (color & 31) * (255 - alpha) >> 8;
				if(r > 31) r = 31;
				if(g > 31) g = 31;
				if(b > 31) b = 31;
				color = (b | (g << 5) | (r << 10));
			}						
			*p = color|0x8000;
		}
		buffer += 256;			
	}
	LeaveCriticalSection(&cr);
	return 0;
}
//---------------------------------------------------------------------------------
int tile_to_bitmap(u32 *tiles,u16 *pal,int colors,int width,int height,u16 *dst)
{
	int x,y,mask,shift,res;
	u32 tile;
	
	if(tiles == NULL || dst == NULL || pal == NULL)
		return -1;
	res = 0;
	mask = colors - 1;	
	shift = i_log2(colors);	
	for(y=0;y<height;y++){
		for(x=0;x<width;){
			u8 index,i;
			
			tile = *tiles++;
			for(i=0;i<32 && x < width;i+=shift,x++){
				index = tile & mask;
				*dst++ = pal[index]|0x8000;
				tile >>= shift;
				res++;
			}
		}
	}
	return res;
}
//---------------------------------------------------------------------------------
int draw_image_rotate(u8 screen,int x,int y,int width,int height,u8 *image,int degree,int zoom,int tr_color)
{
	int xc,yc,sn,cs,x2,y2,x1,y1;
	unsigned long cr;
	
	if(image == NULL)
		return -1;
	EnterCriticalSection(&cr);
	{
		int rad;
		
		rad = degreesToAngle(degree);
		cs = cosLerp(rad);
		sn = sinLerp(rad);
		cs = (cs * zoom) >> 12;
		sn = (sn * zoom) >> 12;
	}
	xc = (width > height ? width : height);
	xc = xc >> 1;
	yc = xc;

	if(screen == 0){
	}
	else{
		u16 *buffer,*p;
	
		buffer = &((u16 *)PA_DrawBg[1])[y << 8];
		for(y1 = -yc;y1<yc;y1++){
			p = buffer + x;
			for(x1=-xc;x1<xc;x1++,p++){
				x2 = xc + (((x1 * cs) - (y1 * sn)) >> 12);
				//check wrap around
				if(x2 < 0 || x2 >= width)
					continue;
				y2 = yc + (((x1 * sn) + (y1 * cs)) >> 12);
				//check wrap around
				if(y2 < 0 || y2 >= height)
					continue;
				{
					u16 color;
					
					color = ((u16 *)image)[y2 * width + x2];
					if(tr_color == -1 || (tr_color != -1 && color != tr_color))
						*p = color;				
				}
			}
			buffer += 256;			
		}	
	}
	LeaveCriticalSection(&cr);
	return 0;
}
//---------------------------------------------------------------------------------
int draw_image(u8 screen,int x,int y,int width,int height,u8 *image,int tr_color,int alpha)
{
	int x1,y1;
	unsigned long cr;

	if(image == NULL)
		return -1;
	EnterCriticalSection(&cr);
	if(screen == 0){
		u8 *buffer,*p;
		int ofs,size;
		
		ofs = x & ~3;
		size = width >> 2;		
		buffer = (u8 *)PA_DrawBg[0];
		buffer += y << 8;
		for(y1 = 0;y1<height;y1++){
			p = line_buffer + x;
			copy_line((u32 *)&buffer[ofs],(u32 *)&line_buffer[ofs],size,0);
			for(x1=0;x1<width;x1++,image++,p++){
				if(tr_color == -1 || (tr_color != -1 && *image != (u8)tr_color))
					*p = *image;
			}
			copy_line((u32 *)&line_buffer[ofs],(u32 *)&buffer[ofs],size,0);
			buffer += 256;			
		}
	}
	else{
		u16 *buffer,*p;
	
		buffer = &((u16 *)PA_DrawBg[1])[y << 8];
		for(y1 = 0;y1<height;y1++){
			p = buffer + x;
			for(x1=0;x1<width;x1++,p++,image += 2){
				if(tr_color == -1 || (tr_color != -1 && *((u16 *)image) != tr_color)){
					u16 color;
					
					color = *((u16 *)image);
					if(alpha != -1){
						int r,g,b;
						
						r = ((color >> 10) & 31) * alpha >> 8;
						g = ((color >> 5) & 31) * alpha >> 8;
						b = (color & 31) * alpha >> 8;
						color = *p;
						r += ((color >> 10) & 31) * (255 - alpha) >> 8;
						g += ((color >> 5) & 31) * (255 - alpha) >> 8;
						b += (color & 31) * (255 - alpha) >> 8;
						if(r > 31) r = 31;
						if(g > 31) g = 31;
						if(b > 31) b = 31;
						color = (b | (g << 5) | (r << 10));
					}
					*p = color;
				}
			}
			buffer += 256;			
		}
	}
	LeaveCriticalSection(&cr);
	return 0;
}