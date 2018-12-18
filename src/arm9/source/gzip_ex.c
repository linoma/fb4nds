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
 
#include "gzip_ex.h"
#include "windows.h"
#include "fb.h"

static char *local_gzip_buffer = NULL;
static int gz_magic[2] = {0x1f, 0x8b};
//---------------------------------------------------------------------------
static int gz_destroy(gz_stream *s)
{
	int err = Z_OK;

	if (!s)
		return Z_STREAM_ERROR;
	if (s->stream.state != NULL)
		err = inflateEnd(&(s->stream));
	if (s->z_err < 0)
		err = s->z_err;
	return err;
}
//---------------------------------------------------------------------------
static int gz_get_byte(gz_stream *s)
{
	if (s->z_eof)
		return EOF;
	if (s->stream.avail_in == 0) {
		{
			int i;

			i = s->size - s->cr_pos;
			if(i < 0)
				i = 0;
			else if(i > 4095)
				i = 4095;
			memcpy(s->inbuf,&s->buf[s->cr_pos],i);
			s->cr_pos += i;
			s->stream.avail_in = i;
		}
	    if (s->stream.avail_in == 0) {
	        s->z_eof = 1;
	        if(s->cr_pos >= s->size)
               s->z_err = Z_ERRNO;
	        return EOF;
	    }
	    s->stream.next_in = s->inbuf;
    }
    s->stream.avail_in--;
    return *(s->stream.next_in)++;
}
//---------------------------------------------------------------------------
static uLong gz_getLong(gz_stream *s)
{
	int c;
	uLong x;

	x = (uLong)gz_get_byte(s);	
	x += ((uLong)gz_get_byte(s))<<8;
	x += ((uLong)gz_get_byte(s))<<16;
	c = gz_get_byte(s);
	if (c == EOF)
		s->z_err = Z_DATA_ERROR;
	x += ((uLong)c)<<24;
	return x;
}
//---------------------------------------------------------------------------
static int gz_check_header(gz_stream *s)
{
	int method,flags,c,res;
	uInt len;
	
	res = 0;
	for (len = 0; len < 2; len++) {
	    c = gz_get_byte(s);
		res++;
	    if (c != gz_magic[len]) {
	        if (len != 0){
				s->stream.avail_in++;
				s->stream.next_in--;
			}
	        if (c != EOF) {
		        s->stream.avail_in++;
				s->stream.next_in--;
		        s->transparent = 1;
	        }
	        s->z_err = s->stream.avail_in != 0 ? Z_OK : Z_STREAM_END;
	        return res;
	    }
	}
	method = gz_get_byte(s);
	res++;
	flags = gz_get_byte(s);
	res++;
	if(method != Z_DEFLATED || (flags & RESERVED) != 0){
	    s->z_err = Z_DATA_ERROR;
	    return res;
	}
	for(len = 0; len < 6; len++){
		gz_get_byte(s);
		res++;
	}
	if((flags & EXTRA_FIELD) != 0) {
		len  =  (uInt)gz_get_byte(s);
		res++;
	    len += ((uInt)gz_get_byte(s))<<8;
		res++;
	    while(len-- != 0 && gz_get_byte(s) != EOF)
			res++;
	}
	if((flags & ORIG_NAME) != 0){
		while ((c = gz_get_byte(s)) != 0 && c != EOF)
			res++;
	}
	if((flags & COMMENT) != 0){
		while ((c = gz_get_byte(s)) != 0 && c != EOF)
			res++;
	}
	if((flags & HEAD_CRC) != 0){
		for(len = 0; len < 2; len++){		
			gz_get_byte(s);
			res++;
		}
	}
	s->z_err = s->z_eof ? Z_DATA_ERROR : Z_OK;
	return res;
}
//---------------------------------------------------------------------------
static int gz_read(gz_stream *s,void *buf,unsigned long len)
{
	Bytef *start = (Bytef*)buf;
	unsigned char *next_out;

	if(s->z_err == Z_DATA_ERROR || s->z_err == Z_ERRNO)
		return -1;
	if(s->z_err == Z_STREAM_END)
		return 0;  /* EOF */
	next_out = (unsigned char*)buf;
	s->stream.next_out = (Bytef*)buf;
	s->stream.avail_out = len;
	while(s->stream.avail_out != 0){
		if (s->transparent) {
			uInt n = s->stream.avail_in;
	        if (n > s->stream.avail_out)
				n = s->stream.avail_out;
	        if (n > 0) {
		        memcpy(s->stream.next_out, s->stream.next_in, n);
		        next_out += n;
		        s->stream.next_out = next_out;
		        s->stream.next_in   += n;
		        s->stream.avail_out -= n;
		        s->stream.avail_in  -= n;
	        }
	        if(s->stream.avail_out > 0) {
				int l;

				l = s->size - s->cr_pos;
				if(l < 0)
					l = 0;
				else if(l > 4095)
					l = 4095;
				if(l > s->stream.avail_out)
					l = s->stream.avail_out;
				memcpy(next_out,&s->buf[s->cr_pos],l);
				s->cr_pos += l;
		        s->stream.avail_out -= l;
	        }
	        len -= s->stream.avail_out;
	        s->stream.total_in  += (uLong)len;
	        s->stream.total_out += (uLong)len;
			if(len == 0)
				s->z_eof = 1;
	        return (int)len;
	    }
		if (s->stream.avail_in == 0 && !s->z_eof) {
			unsigned long l;

			if(s->cr_pos < s->size){
				l = s->size - s->cr_pos;
				if(l > 4095)
					l = 4095;
			}
			else
				l = 0;
			memcpy(s->inbuf,&s->buf[s->cr_pos],l);
			s->cr_pos += l;
			s->stream.avail_in = l;
			if (s->stream.avail_in == 0) {
				s->z_eof = 1;
		        if(s->cr_pos >= s->size){
					s->z_err = Z_ERRNO;
		            break;
		        }
			}
			s->stream.next_in = s->inbuf;
		}
		s->z_err = inflate(&(s->stream), Z_NO_FLUSH);
	    if (s->z_err == Z_STREAM_END) {
			s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));
	        start = s->stream.next_out;
	        if(gz_getLong(s) != s->crc)
		        s->z_err = Z_DATA_ERROR;
			else {
				gz_getLong(s);
		        gz_check_header(s);
		        if(s->z_err == Z_OK){
					uLong total_in = s->stream.total_in;
		            uLong total_out = s->stream.total_out;
		            inflateReset(&(s->stream));
		            s->stream.total_in = total_in;
		            s->stream.total_out = total_out;
		            s->crc = crc32(0L, Z_NULL, 0);
		        }
	        }
		}
		if(s->z_err != Z_OK || s->z_eof)
			break;
	}
	s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));
	return (int)(len - s->stream.avail_out);
}
//---------------------------------------------------------------------------
int gz_expand(char *src,char *dst,unsigned int size)
{
	gz_stream *s;
	int err,res;

	if(src == NULL || dst == NULL || size == 0)
		return -1;
	local_gzip_buffer = (char *)&temp_buffer[SZ_LOCAL_BUFFER - temp_buffer_index];
	memset(local_gzip_buffer,0,5000);
	s = (gz_stream *)local_gzip_buffer;
	s->stream.zalloc = (alloc_func)0;
	s->stream.zfree = (free_func)0;
	s->stream.opaque = (voidpf)0;
	s->stream.next_in = s->inbuf = Z_NULL;
	s->stream.next_out = s->outbuf = Z_NULL;
	s->stream.avail_in = s->stream.avail_out = 0;
	s->buf = (unsigned char *)src;
	s->cr_pos = 0;
	s->size = size;
	s->z_err = Z_OK;
	s->z_eof = 0;
	s->crc = crc32(0L,Z_NULL,0);
	s->transparent = 0;
	s->mode = 'r';
	s->stream.next_in = s->inbuf = (unsigned char *)(s+1);
	err = inflateInit2(&(s->stream), -MAX_WBITS);
	if(err != Z_OK || s->inbuf == Z_NULL) {
		gz_destroy(s);
		return -4;
	}
	s->stream.avail_out = 4096;
	gz_check_header(s);
	s->startpos = 0;
//	s->startpos = (size - s->stream.avail_in);
	res = 0;
	while(res < 195000){
		int len;

		len = res + 1000 < 195000 ? 1000 : 195000 - res;
		len = gz_read(s,dst,len);
		if(len <= 0)
			break;
		res += len;
		dst += len;
	}
	gz_destroy(s);
	if(res == 0)
		return -1;
	return res;
}
//---------------------------------------------------------------------------
int gz_expand_response(char *src,char *dst,unsigned int size)
{
	int res;
	
	if(src == NULL || dst == NULL || size == 0)
		return -1;
	{
		char *p;
		
		p = strstr(src,"Content-Encoding");
		if(p == NULL)
			return -2;
		p += 16;
		while(*p != 0 && !isalpha(*p))
			p++;
		if(strstr(p,"gzip") != p)
			return -3;
		p += 4;
		if((p = strstr(src,"\r\n\r\n")) == NULL)
			return -4;		
		p += 4;
		{
			char *p1;
			
			p1 = strstr(src,"Transfer-Encoding:");//HTTP1.1 use this.
			if(p1 != NULL){
				char *p2,*p3;
				
				p2 = p1;
				while(*p2 != '\n')
					p2++;
				p3 = strstr(p1,"chunked");
				if(p3 != NULL && ((unsigned int)p3) < ((unsigned int)p2)){
					if((p1 = strstr(p,"\r\n")) != NULL){
						if(((unsigned long)p1 - (unsigned long)p) < 10)
							p = p1 + 2;
					}
				}
			}
		}
		size -= p - src;
		src = p;		
	}
	
	res = gz_expand(src,dst,size);
	if(res > 0)
		dst[res] = 0;
	return res;
}
