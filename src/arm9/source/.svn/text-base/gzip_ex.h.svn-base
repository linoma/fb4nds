#include <PA9.h> 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <zlib.h>

#ifndef __GZIP_EXH__
#define __GZIP_EXH__

#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

typedef struct gz_stream {
	z_stream stream;
    unsigned long size,cr_pos;
    int z_err,z_eof;
    unsigned char *inbuf,*outbuf,*buf;
    uLong crc;
    int transparent;
    char mode;
	long startpos;
} gz_stream;

extern int gz_expand_response(char *src,char *dst,unsigned int size);

#endif