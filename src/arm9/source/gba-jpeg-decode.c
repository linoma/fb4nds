#include "gba-jpeg-decode.h"


typedef int(*LPJPEGCALLBACK)(int,int,int,int,int,void *);

/* Converts left-to-right coefficient indices into zig-zagged indices. */
extern const unsigned char JPEG_ToZigZag [JPEG_DCTSIZE2];
extern const unsigned char JPEG_ComponentRange [32 * 3];
extern const JPEG_FIXED_TYPE JPEG_AANScaleFactor [JPEG_DCTSIZE2];
extern int jpeg_width;

extern void JPEG_ConvertBlock (
    signed char *YBlock, signed char *CbBlock, signed char *CrBlock,
    int YHorzFactor, int YVertFactor, int CbHorzFactor, int CbVertFactor, int CrHorzFactor, int CrVertFactor, int horzMax, int vertMax,
    char M211, volatile JPEG_OUTPUT_TYPE *out, int outStride, const unsigned char *ComponentRange);
extern void JPEG_IDCT_Columns(JPEG_FIXED_TYPE *zz);
extern void JPEG_IDCT_Rows(const JPEG_FIXED_TYPE *zz, signed char *chunk, int chunkStride);
extern void JPEG_DecodeCoefficients(
    JPEG_FIXED_TYPE *dcLast, JPEG_FIXED_TYPE *zz, JPEG_FIXED_TYPE *quant,
    JPEG_HuffmanTable *dcTable, JPEG_HuffmanTable *acTable,
    const unsigned char **dataBase, unsigned int *bitsLeftBase,
    unsigned long int *bitsDataBase, const unsigned char *toZigZag);

int JPEG_Decoder_ReadImageEx(JPEG_Decoder *decoder, const unsigned char **dataBase, volatile JPEG_OUTPUT_TYPE *out, int outWidth, int outHeight, LPJPEGCALLBACK pfn_cb,void *user_param)
{
    JPEG_FrameHeader *frame = &decoder->frame; /* Pointer to the image's frame. */
    JPEG_ScanHeader *scan = &decoder->scan; /* Pointer to the image's scan. */
    int YHorzFactor = 0, YVertFactor = 0; /* Scaling factors for the Y component. */
    int CbHorzFactor = 1, CbVertFactor = 1; /* Scaling factors for the Cb component.  The default is important because it is used for greyscale images. */
    int CrHorzFactor = 1, CrVertFactor = 1; /* Scaling factors for the Cr component.  The default is important because it is used for greyscale images. */
    int horzMax = 0, vertMax = 0; /* The maximum horizontal and vertical scaling factors for the components. */
    JPEG_FrameHeader_Component *frameComponents [JPEG_MAXIMUM_COMPONENTS]; /* Pointers translating scan header components to frame header components. */
    JPEG_FrameHeader_Component *item, *itemEnd = frame->componentList + frame->componentCount; /* The frame header's components for loops. */
    JPEG_FIXED_TYPE dcLast [JPEG_MAXIMUM_COMPONENTS]; /* The last DC coefficient computed.  This is initialized to zeroes at the start and after a restart interval. */
    int c, bx, by, cx, cy; /* Various loop parameters. */
    int horzShift = 0; /* The right shift to use after multiplying by nHorzFactor to get the actual sample. */
    int vertShift = 0; /* The right shift to use after multiplying by nVertFactor to get the actual sample. */
    char M211 = 0; /* Whether this scan satisfies the 2:1:1 relationship, which leads to faster code. */
    const unsigned char *data = *dataBase; /* The input data pointer; this must be right at the start of scan data. */
    
    signed char blockBase [JPEG_DCTSIZE2 * JPEG_MAXIMUM_SCAN_COMPONENT_FACTORS]; /* Blocks that have been read and are alloted to YBlock, CbBlock, and CrBlock based on their scaling factors. */
    signed char *YBlock; /* Y component temporary block that holds samples for the MCU currently being decompressed. */
    signed char *CbBlock; /* Cb component temporary block that holds samples for the MCU currently being decompressed. */
    signed char *CrBlock; /* Cr component temporary block that holds samples for the MCU currently being decompressed. */
    
    JPEG_HuffmanTable acTableList [2]; /* The decompressed AC Huffman tables.  JPEG Baseline allows only two AC Huffman tables in a scan. */
    int acTableUse [2] = { -1, -1 }; /* The indices of the decompressed AC Huffman tables, or -1 if this table hasn't been used. */
    JPEG_HuffmanTable dcTableList [2]; /* The decompressed DC Huffman tables.  JPEG Baseline allows only two DC Huffman tables in a scan. */
    int dcTableUse [2] = { -1, -1 }; /* The indices of the decompressed DC Huffman tables, or -1 if this table hasn't been used. */
    int restartInterval = decoder->restartInterval; /* Number of blocks until the next restart. */
    
    /* Pointer to JPEG_ConvertBlock, which might be moved to IWRAM. */
    void (*ConvertBlock) (signed char *, signed char *, signed char *,
        int, int, int, int, int, int, int, int, char,
        volatile JPEG_OUTPUT_TYPE *, int, const unsigned char *)
        = &JPEG_ConvertBlock;

    /* Pointer to JPEG_IDCT_Columns, which might be moved to IWRAM. */
    void (*IDCT_Columns) (JPEG_FIXED_TYPE *) = &JPEG_IDCT_Columns;

    /* Pointer to JPEG_IDCT_Rows, which might be moved to IWRAM. */
    void (*IDCT_Rows) (const JPEG_FIXED_TYPE *, signed char *, int) = &JPEG_IDCT_Rows;
    
    /* Pointer to JPEG_DecodeCoefficients, which might be moved to IWRAM. */
    void (*DecodeCoefficients) (JPEG_FIXED_TYPE *, JPEG_FIXED_TYPE *, JPEG_FIXED_TYPE *, JPEG_HuffmanTable *,
        JPEG_HuffmanTable *, const unsigned char **, unsigned int *,
        unsigned long int *, const unsigned char *) = &JPEG_DecodeCoefficients;
    
    const unsigned char *ToZigZag = JPEG_ToZigZag; /* Pointer to JPEG_ToZigZag, which might be moved to IWRAM. */
    const unsigned char *ComponentRange = JPEG_ComponentRange; /* Pointer to JPEG_ComponentRange, which might be moved to IWRAM. */
   
    /* Start decoding bits. */
    JPEG_BITS_START ();
    
    /* The sum of all factors in the scan; this cannot be greater than 10 in JPEG Baseline. */
    int factorSum = 0;

    /* Find the maximum factors and the factors for each component. */    
    for (item = frame->componentList; item < itemEnd; item ++)
    {
        /* Find the opposing scan header component. */
        for (c = 0; ; c ++)
        {
            JPEG_ScanHeader_Component *sc;

            JPEG_Assert (c < scan->componentCount);
            sc = &scan->componentList [c];
            if (sc->selector != item->selector)
                continue;
            
            /* Decompress the DC table if necessary. */
            if (sc->dcTable != dcTableUse [0] && sc->dcTable != dcTableUse [1])
            {
                const unsigned char *tablePointer = decoder->dcTables [sc->dcTable];
                
                if (dcTableUse [0] == -1)
                    dcTableUse [0] = sc->dcTable, JPEG_HuffmanTable_Read (&dcTableList [0], &tablePointer);
                else if (dcTableUse [1] == -1)
                    dcTableUse [1] = sc->dcTable, JPEG_HuffmanTable_Read (&dcTableList [1], &tablePointer);
                else
                    JPEG_Assert (0);
            }
            
            /* Decompress the AC table if necessary. */
            if (sc->acTable != acTableUse [0] && sc->acTable != acTableUse [1])
            {
                const unsigned char *tablePointer = decoder->acTables [sc->acTable];
                
                if (acTableUse [0] == -1)
                    acTableUse [0] = sc->acTable, JPEG_HuffmanTable_Read (&acTableList [0], &tablePointer);
                else if (acTableUse [1] == -1)
                    acTableUse [1] = sc->acTable, JPEG_HuffmanTable_Read (&acTableList [1], &tablePointer);
                else
                    JPEG_Assert (0);
            }
            
            frameComponents [c] = item;
            break;
        }
        
        /* Add the sum for a later assertion test. */
        factorSum += item->horzFactor * item->vertFactor;
        
        /* Adjust the maximum horizontal and vertical scaling factors as necessary. */
        if (item->horzFactor > horzMax)
            horzMax = item->horzFactor;
        if (item->vertFactor > vertMax)
            vertMax = item->vertFactor;
            
        /* Update the relevant component scaling factors if necessary. */
        if (item->selector == 1)
        {
            YHorzFactor = item->horzFactor;
            YVertFactor = item->vertFactor;
        }
        else if (item->selector == 2)
        {
            CbHorzFactor = item->horzFactor;
            CbVertFactor = item->vertFactor;
        }
        else if (item->selector == 3)
        {
            CrHorzFactor = item->horzFactor;
            CrVertFactor = item->vertFactor;
        }
    }
   
    /* Ensure that we have enough memory for these factors. */
    JPEG_Assert (factorSum < JPEG_MAXIMUM_SCAN_COMPONENT_FACTORS);
     
    /* Split up blockBase according to the components. */
    YBlock = blockBase;
    CbBlock = YBlock + YHorzFactor * YVertFactor * JPEG_DCTSIZE2;
    CrBlock = CbBlock + CbHorzFactor * CbVertFactor * JPEG_DCTSIZE2;
    
    /* Compute the right shift to be done after multiplying against the scaling factor. */
    if (horzMax == 1) horzShift = 8;
    else if (horzMax == 2) horzShift = 7;
    else if (horzMax == 4) horzShift = 6;
    
    /* Compute the right shift to be done after multiplying against the scaling factor. */
    if (vertMax == 1) vertShift = 8;
    else if (vertMax == 2) vertShift = 7;
    else if (vertMax == 4) vertShift = 6;

    /* Adjust the scaling factors for our parameters. */    
    YHorzFactor <<= horzShift;
    YVertFactor <<= vertShift;
    CbHorzFactor <<= horzShift;
    CbVertFactor <<= vertShift;
    CrHorzFactor <<= horzShift;
    CrVertFactor <<= vertShift;
    
    /* Clear the Cb channel for potential grayscale. */
    {
        signed char *e = CbBlock + JPEG_DCTSIZE2;
        
        do *-- e = 0;
        while (e > CbBlock);
    }
    
    /* Clear the Cr channel for potential grayscale. */
    {
        signed char *e = CrBlock + JPEG_DCTSIZE2;
        
        do *-- e = 0;
        while (e > CrBlock);
    }

/* Compute whether this satisfies the sped up 2:1:1 relationship. */
#if JPEG_FASTER_M211
    if (YHorzFactor == 256 && YVertFactor == 256 && CbHorzFactor == 128 && CbVertFactor == 128 && CrHorzFactor == 128 && CrVertFactor == 128)
        M211 = 1;
#endif /* JPEG_FASTER_M211 */
        
    /* Clear the DC parameters. */
    for (c = 0; c < JPEG_MAXIMUM_COMPONENTS; c ++)
        dcLast [c] = 0;
    
    /* Now run over each MCU horizontally, then vertically. */
    for (by = 0; by < frame->height; by += vertMax * JPEG_DCTSIZE)
    {
		for (bx = 0; bx < frame->width; bx += horzMax * JPEG_DCTSIZE)
        {
            /* Read the components for the MCU. */
            for (c = 0; c < scan->componentCount; c ++)
            {
                JPEG_ScanHeader_Component *sc = &scan->componentList [c];
                JPEG_FrameHeader_Component *fc = frameComponents [c];
                JPEG_HuffmanTable *dcTable, *acTable;
                JPEG_FIXED_TYPE *quant = decoder->quantTables [fc->quantTable];
                int stride = fc->horzFactor * JPEG_DCTSIZE;
                signed char *chunk = 0;
                
                dcTable = &dcTableList [sc->dcTable == dcTableUse [1] ? 1 : 0];
                acTable = &acTableList [sc->acTable == acTableUse [1] ? 1 : 0];
                
                /* Compute the output chunk. */
                if (fc->selector == 1)
                    chunk = YBlock;
                else if (fc->selector == 2)
                    chunk = CbBlock;
                else if (fc->selector == 3)
                    chunk = CrBlock;
                    
                for (cy = 0; cy < fc->vertFactor * JPEG_DCTSIZE; cy += JPEG_DCTSIZE)
                {
                    for (cx = 0; cx < fc->horzFactor * JPEG_DCTSIZE; cx += JPEG_DCTSIZE)
                    {
                        int start = cx + cy * stride;
                        JPEG_FIXED_TYPE zz [JPEG_DCTSIZE2];

                        /* Decode coefficients. */
                        DecodeCoefficients (&dcLast [c], zz, quant, dcTable, acTable, &data, &bits_left, &bits_data, ToZigZag);

                        /* Perform an IDCT if this component will contribute to the image. */
                        if (chunk)
                        {
                            IDCT_Columns (zz);
                            IDCT_Rows (zz, chunk + start, stride);
                        }
                    }
                }
            }            
            /* Check that our block will be in-range; this should actually use clamping. */
            if (bx + horzMax * JPEG_DCTSIZE > outWidth || by + vertMax * JPEG_DCTSIZE > outHeight)
                continue;               
            /* Convert our block from YCbCr to the output. */
            ConvertBlock (YBlock, CbBlock, CrBlock,
                YHorzFactor, YVertFactor, CbHorzFactor, CbVertFactor, CrHorzFactor, CrVertFactor,
                horzMax * JPEG_DCTSIZE, vertMax * JPEG_DCTSIZE, M211, out + bx, outWidth, ComponentRange);
            /* Handle the restart interval. */
            if(decoder->restartInterval && --restartInterval == 0){
                restartInterval = decoder->restartInterval;
                JPEG_BITS_REWIND ();
                if (((data [0] << 8) | data [1]) == JPEG_Marker_EOI)
                    goto finish;
                JPEG_Assert (data [0] == 0xFF && (data [1] >= 0xD0 && data [1] <= 0xD7));
                for (c = 0; c < JPEG_MAXIMUM_COMPONENTS; c ++)
                    dcLast [c] = 0;
                data += 2;
            }
        }
		if(pfn_cb != 0)
			pfn_cb(1,0,by,frame->width, vertMax * JPEG_DCTSIZE,user_param);		
    }
   
finish:
    /* Make sure we read an EOI marker. */ 
    JPEG_BITS_REWIND ();
    JPEG_Assert (((data [0] << 8) | data [1]) == JPEG_Marker_EOI);
    data += 2;
    
    /* Clear up and return success. */
    *dataBase = data;
    return 1;
}

//int JPEG_FrameHeader_Read (JPEG_FrameHeader *frame, const unsigned char **dataBase, JPEG_Marker marker)
//int JPEG_ScanHeader_Read (JPEG_ScanHeader *scan, const unsigned char **dataBase)
//int JPEG_Decoder_ReadHeaders (JPEG_Decoder *decoder, const unsigned char **dataBase)
//int JPEG_Match (const unsigned char *data, int length)
