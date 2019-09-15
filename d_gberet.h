#ifndef _D_GBERET_H_
#define _D_GBERET_H_

//#include "sn76496.h"
#include "PSG.h"
#include "burnint.h"
#include "czet.h"
#include "saturn/ovl.h"

#define SOUND_LEN 256
int DrvFrame();
int DrvDraw();
INT32 DrvExit();
int gberetInit();
int mrgoemonInit();
void dummy();
void SN76496Update(int Num, short* pSoundBuf, int Length);
void SN76496Write(int Num, int Data);
void SN76489Init(int Num, int Clock, int SignalAdd);
void renderSound(unsigned int *nSoundBufferPos);
void cleanSprites();
void init_32_colors(unsigned int *t_pal,unsigned char *color_prom);
#ifdef CACHE
unsigned char 	bg_dirtybuffer[2048];
#endif

unsigned short *DrvPalette = NULL;
unsigned int _30_HZ=0;

UINT8 *AllMem = NULL;
UINT8 *MemEnd = NULL;
UINT8 *AllRam = NULL;
UINT8 *RamEnd = NULL;
UINT8 *DrvZ80ROM = NULL;
UINT8 *DrvGfxROM0 = NULL;
UINT8 *DrvGfxROM1 = NULL;
UINT8 *DrvColPROM = NULL;
UINT8 *DrvZ80RAM = NULL;
UINT8 *DrvVidRAM = NULL;
UINT8 *DrvColRAM = NULL;
UINT8 *DrvSprRAM0 = NULL;
UINT8 *DrvSprRAM1 = NULL;
UINT8 *DrvSprRAM2 = NULL;
UINT8 *DrvScrollRAM = NULL;

UINT8 DrvJoy1[8] = {0,0,0,0,0,0,0,0};
UINT8 DrvJoy2[8] = {0,0,0,0,0,0,0,0};
UINT8 DrvJoy3[8] = {0,0,0,0,0,0,0,0};
UINT8 DrvDips[3] = {0,0,0};
UINT8 DrvInputs[3] = {0,0,0};
UINT8 nmi_enable = 0, irq_enable = 0;
UINT8 sprite_bank = 0;
INT32 z80_bank = 0;
INT32 irq_mask = 0;
INT32 irq_timer = 0;
INT32 soundlatch = 0;
INT32 mrgoemon_bank = 0;

INT32 game_type = 0; // 0 gberet / rushatck, 1 gberetb, 2 mrgoemon

struct BurnInputInfo GberetInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	NULL,			"reset"		},
	{"Service",			BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",			BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Gberet)

struct BurnDIPInfo GberetDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL					},
	{0x13, 0xff, 0xff, 0x4a, NULL					},
	{0x14, 0xff, 0xff, 0x0f, NULL					},

	{0   , 0xfe, 0   ,    16, "Coin A"				},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"			},

	{0   , 0xfe, 0   ,    16, "Coin B"				},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0xf0, 0x00, "No Coin B"			},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x13, 0x01, 0x03, 0x03, "2"					},
	{0x13, 0x01, 0x03, 0x02, "3"					},
	{0x13, 0x01, 0x03, 0x01, "5"					},
	{0x13, 0x01, 0x03, 0x00, "7"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x13, 0x01, 0x04, 0x00, "Upright"				},
	{0x13, 0x01, 0x04, 0x04, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x13, 0x01, 0x18, 0x18, "30K, 70K, Every 70K"	},
	{0x13, 0x01, 0x18, 0x10, "40K, 80K, Every 80K"	},
	{0x13, 0x01, 0x18, 0x08, "50K, 100K, Every 100K"},
	{0x13, 0x01, 0x18, 0x00, "50K, 200K, Every 200K"},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x13, 0x01, 0x60, 0x60, "Easy"					},
	{0x13, 0x01, 0x60, 0x40, "Normal"				},
	{0x13, 0x01, 0x60, 0x20, "Difficult"			},
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x13, 0x01, 0x80, 0x80, "Off"					},
	{0x13, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x14, 0x01, 0x01, 0x01, "Off"					},
	{0x14, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Upright Controls"		},
	{0x14, 0x01, 0x02, 0x02, "Single"				},
	{0x14, 0x01, 0x02, 0x00, "Dual"					},
};

STDDIPINFO(Gberet)

struct BurnDIPInfo MrgoemonDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL					},
	{0x13, 0xff, 0xff, 0x4a, NULL					},
	{0x14, 0xff, 0xff, 0x0f, NULL					},

	{0   , 0xfe, 0   ,    16, "Coin A"				},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"			},

	{0   , 0xfe, 0   ,    16, "Coin B"				},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0xf0, 0x00, "No Coin B"			},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x13, 0x01, 0x03, 0x03, "2"					},
	{0x13, 0x01, 0x03, 0x02, "3"					},
	{0x13, 0x01, 0x03, 0x01, "5"					},
	{0x13, 0x01, 0x03, 0x00, "7"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x13, 0x01, 0x04, 0x00, "Upright"				},
	{0x13, 0x01, 0x04, 0x04, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x13, 0x01, 0x18, 0x18, "20K, Every 60K"		},
	{0x13, 0x01, 0x18, 0x10, "30K, Every 70K"		},
	{0x13, 0x01, 0x18, 0x08, "40K, Every 80K"		},
	{0x13, 0x01, 0x18, 0x00, "50K, Every 90K"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x13, 0x01, 0x60, 0x60, "Easy"					},
	{0x13, 0x01, 0x60, 0x40, "Normal"				},
	{0x13, 0x01, 0x60, 0x20, "Difficult"			},
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x13, 0x01, 0x80, 0x80, "Off"					},
	{0x13, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x14, 0x01, 0x01, 0x01, "Off"					},
	{0x14, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Upright Controls"		},
	{0x14, 0x01, 0x02, 0x02, "Single"				},
	{0x14, 0x01, 0x02, 0x00, "Dual"					},
};

STDDIPINFO(Mrgoemon)

// Green Beret

struct BurnRomInfo gberetRomDesc[] = {
	{ "577l03.10c",   0x4000, 0xae29e4ff, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "577l02.8c",    0x4000, 0x240836a5, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "577l01.7c",    0x4000, 0x41fa3e1f, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "577l07.3f",    0x4000, 0x4da7bd1b, 2 | BRF_GRA },	       //  3 Characters

	{ "577l06.5e",    0x4000, 0x0f1cb0ca, 3 | BRF_GRA },	       //  4 Sprites
	{ "577l05.4e",    0x4000, 0x523a8b66, 3 | BRF_GRA },	       //  5
	{ "577l08.4f",    0x4000, 0x883933a4, 3 | BRF_GRA },	       //  6
	{ "577l04.3e",    0x4000, 0xccecda4c, 3 | BRF_GRA },	       //  7

	{ "577h09.2f",    0x0020, 0xc15e7c80, 4 | BRF_GRA },	       //  8 Color Proms
	{ "577h11.6f",    0x0100, 0x2a1a992b, 4 | BRF_GRA },	       //  9
	{ "577h10.5f",    0x0100, 0xe9de1e53, 4 | BRF_GRA },	       // 10
};

STD_ROM_PICK(gberet)
STD_ROM_FN(gberet)


// Mr. Goemon (Japan)

struct BurnRomInfo mrgoemonRomDesc[] = {
	{ "621d01.10c",   0x8000, 0xb2219c56, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "621d02.12c",   0x8000, 0xc3337a97, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "621a05.6d",    0x4000, 0xf0a6dfc5, 2 | BRF_GRA },	       //  2 Characters

	{ "621d03.4d",    0x8000, 0x66f2b973, 3 | BRF_GRA },	       //  3 Sprites
	{ "621d04.5d",    0x8000, 0x47df6301, 3 | BRF_GRA },	       //  4

	{ "621a06.5f",    0x0020, 0x7c90de5f, 4 | BRF_GRA },	       //  5 Color Proms
	{ "621a08.7f",    0x0100, 0x2fb244dd, 4 | BRF_GRA },	       //  6
	{ "621a07.6f",    0x0100, 0x3980acdc, 4 | BRF_GRA },	       //  7
};

STD_ROM_PICK(mrgoemon)
STD_ROM_FN(mrgoemon)


#endif

