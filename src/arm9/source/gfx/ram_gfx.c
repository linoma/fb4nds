
//{{BLOCK(ram)

//======================================================================
//
//	ram, 16x8@4, 
//	Transparent palette entry: 2.
//	+ palette 16 entries, not compressed
//	+ 2 tiles not compressed
//	Total size: 32 + 64 = 96
//
//	Time-stamp: 2010-01-30, 11:08:47
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.3
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

const unsigned int ramTiles[16] __attribute__((aligned(4)))=
{
	0x22222200,0x22222200,0x22222200,0x22222200,0x22222200,0x22222222,0x22222222,0x22222220,
	0x00222222,0x00222222,0x00222222,0x00222222,0x00222222,0x22222222,0x22222222,0x02222222,
};

const unsigned short ramPal[16] __attribute__((aligned(4)))=
{
	0x7FFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};

//}}BLOCK(ram)
