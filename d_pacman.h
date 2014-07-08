#ifndef _D_PACMAN_H_
#define _D_PACMAN_H_

#include "burnint.h"
#include "bitswap.h"
//#include "sn76496.h"
//#include "namco_snd.h"
//extern "C" {
//#include "ay8910.h"
//}
#include "saturn/ovl.h"

#define nBurnSoundLen 240

int ovlInit(char *szShortName) __attribute__ ((boot,section(".boot")));

/*static*/ //INT32 mspacmanInit();
/*static*/ //INT32 puckmanInit();
/*static*/// INT32 pengoInit();
static INT32 pengouInit();
/*static*/ //INT32 pengobInit();
static INT32 DrvExit();
static INT32 DrvFrame();
static INT32 DrvDraw();
static void make_lut(void);

/*static*/ UINT8 *AllMem = NULL;
/*static*/ UINT8 *MemEnd = NULL;
/*static*/ UINT8 *AllRam = NULL;
/*static*/ UINT8 *RamEnd = NULL;
/*static*/ UINT8 *PengoStart = NULL;
/*static*/ UINT8 *DrvZ80ROM = NULL;
/*static*/ UINT8 *DrvQROM = NULL;
/*static*/ UINT8 *DrvGfxROM = NULL;
/*static*/ UINT8 *DrvZ80RAM = NULL;
/*static*/ UINT8 *DrvVidRAM = NULL;
/*static*/ UINT8 *DrvColRAM = NULL;
/*static*/ UINT8 *DrvSprRAM = NULL;
/*static*/ UINT8 *DrvSprRAM2 = NULL;
/*static*/ UINT8 *DrvColPROM = NULL;
/*static*/ UINT8 *bg_dirtybuffer = NULL;
/*static*/ UINT16 *map_offset_lut = NULL;
/*static*/ UINT16 *ofst_lut = NULL;

/*static*/ //INT16 pBurnSoundOut[0x8000];
///*static*/ INT16 *pAY8910Buffer[3];
/*static*/ //UINT32 *Palette = NULL;;
/*static*/ //UINT8 DrvRecalc = 0;

/*static*/ UINT8 DrvReset = 0;
/*static*/ UINT8 DrvJoy1[8] = {0,0,0,0,0,0,0,0};
/*static*/ UINT8 DrvJoy2[8] = {0,0,0,0,0,0,0,0};
/*static*/ UINT8 DrvInputs[2] = {0,0};
/*static*/ UINT8 DrvDips[4] = {0,0,0,0};
/*static*/ INT16 DrvAxis[2] = { 0, 0 };
/*static*/ INT16 nAnalogAxis[2] = {0,0};
/*static*/ UINT8 nCharAxis[2] = {0,0};

enum { PACMAN=0, PENGO };

/*static*/ INT32 game_select = 0;
///*static*/ INT32 acitya = 0;

/*static*/ //UINT8 *flipscreen = 0;

/*static*/ INT32 interrupt_mode = 0;
/*static*/ INT32 interrupt_mask = 0;

/*static*/ UINT8 colortablebank = 0;
/*static*/ UINT8 palettebank = 0;
/*static*/ UINT8 spritebank = 0;
/*static*/ UINT8 charbank = 0;
/*static*/ INT32 nPacBank = 0;
/*static*/ UINT32 watchdog = 0;
//------------------------------------------------------------------------------------------------------
/*static*/ struct BurnInputInfo PengoInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
	{"Dip D",		BIT_DIPSWITCH,	DrvDips + 3,	"dip"},
};

STDINPUTINFO(Pengo)

/*static*/ struct BurnDIPInfo PengoDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xff, NULL				},
	{0x10, 0xff, 0xff, 0xff, NULL				},
	{0x11, 0xff, 0xff, 0xb0, NULL				},
	{0x12, 0xff, 0xff, 0xcc, NULL				},

	{0   , 0xfe, 0   ,    1, "Service Mode"			},
	{0x10, 0x01, 0x10, 0x10, "Off"				},

	{0   , 0xfe, 0   ,    2, "Bonus Life"			},
	{0x11, 0x01, 0x01, 0x00, "30000"			},
	{0x11, 0x01, 0x01, 0x01, "50000"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x02, 0x02, "Off"				},
	{0x11, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x11, 0x01, 0x04, 0x00, "Upright"			},
	{0x11, 0x01, 0x04, 0x04, "Cocktail"			},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x11, 0x01, 0x18, 0x18, "2"				},
	{0x11, 0x01, 0x18, 0x10, "3"				},
	{0x11, 0x01, 0x18, 0x08, "4"				},
	{0x11, 0x01, 0x18, 0x00, "5"				},

	{0   , 0xfe, 0   ,    2, "Rack Test (Cheat)"		},
	{0x11, 0x01, 0x20, 0x20, "Off"				},
	{0x11, 0x01, 0x20, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x11, 0x01, 0xc0, 0xc0, "Easy"				},
	{0x11, 0x01, 0xc0, 0x80, "Medium"			},
	{0x11, 0x01, 0xc0, 0x40, "Hard"				},
	{0x11, 0x01, 0xc0, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    16, "Coin A"			},
	{0x12, 0x01, 0x0f, 0x00, "4 Coins 1 Credits"		},
	{0x12, 0x01, 0x0f, 0x08, "3 Coins 1 Credits"		},
	{0x12, 0x01, 0x0f, 0x04, "2 Coins 1 Credits"		},
	{0x12, 0x01, 0x0f, 0x09, "2 Coins/1 Credit 5/3"		},
	{0x12, 0x01, 0x0f, 0x05, "2 Coins/1 Credit 4/3"		},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  1 Credits"		},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin/1 Credit 5/6"		},
	{0x12, 0x01, 0x0f, 0x03, "1 Coin/1 Credit 4/5"		},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin/1 Credit 2/3"		},
	{0x12, 0x01, 0x0f, 0x02, "1 Coin  2 Credits"		},
	{0x12, 0x01, 0x0f, 0x07, "1 Coin/2 Credits 5/11"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin/2 Credits 4/9"		},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  3 Credits"		},
	{0x12, 0x01, 0x0f, 0x06, "1 Coin  4 Credits"		},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  5 Credits"		},
	{0x12, 0x01, 0x0f, 0x01, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   ,    16, "Coin B"			},
	{0x12, 0x01, 0xf0, 0x00, "4 Coins 1 Credits"		},
	{0x12, 0x01, 0xf0, 0x80, "3 Coins 1 Credits"		},
	{0x12, 0x01, 0xf0, 0x40, "2 Coins 1 Credits"		},
	{0x12, 0x01, 0xf0, 0x90, "2 Coins/1 Credit 5/3"		},
	{0x12, 0x01, 0xf0, 0x50, "2 Coins/1 Credit 4/3"		},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  1 Credits"		},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin/1 Credit 5/6"		},
	{0x12, 0x01, 0xf0, 0x30, "1 Coin/1 Credit 4/5"		},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin/1 Credit 2/3"		},
	{0x12, 0x01, 0xf0, 0x20, "1 Coin  2 Credits"		},
	{0x12, 0x01, 0xf0, 0x70, "1 Coin/2 Credits 5/11"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin/2 Credits 4/9"		},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  3 Credits"		},
	{0x12, 0x01, 0xf0, 0x60, "1 Coin  4 Credits"		},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  5 Credits"		},
	{0x12, 0x01, 0xf0, 0x10, "1 Coin  6 Credits"		},
};

STDDIPINFO(Pengo)

// Pengo (set 2 not encrypted)

/*static*/ struct BurnRomInfo pengo2uRomDesc[] = {
	{ "epr5128.u8",   0x1000, 0x3dfeb20e, 1 | BRF_ESS | BRF_PRG },	//  0 Z80 Code
	{ "epr5129.u7",   0x1000, 0x1db341bd, 1 | BRF_ESS | BRF_PRG },	//  1
	{ "epr5130.u15",  0x1000, 0x7c2842d5, 1 | BRF_ESS | BRF_PRG },	//  2
	{ "epr5131.u14",  0x1000, 0x6e3c1f2f, 1 | BRF_ESS | BRF_PRG },	//  3
	{ "ep5124.21",    0x1000, 0x95f354ff, 1 | BRF_ESS | BRF_PRG },	//  4
	{ "epr5133.u20",  0x1000, 0x0fdb04b8, 1 | BRF_ESS | BRF_PRG },	//  5
	{ "ep5126.32",    0x1000, 0xe5920728, 1 | BRF_ESS | BRF_PRG },	//  6
	{ "epr5135.u31",  0x1000, 0x13de47ed, 1 | BRF_ESS | BRF_PRG },	//  7

	{ "ep1640.92",    0x2000, 0xd7eec6cd, 2 | BRF_GRA },			//  8 Graphics
	{ "ep1695.105",   0x2000, 0x5bfd26e9, 2 | BRF_GRA },			//  9

	{ "pr1633.78",    0x0020, 0x3a5844ec, 3 | BRF_GRA },			// 10 Color Proms
	{ "pr1634.88",    0x0400, 0x766b139b, 3 | BRF_GRA },			// 11

	{ "pr1635.51",    0x0100, 0xc29dea27, 4 | BRF_SND },			// 12 Sound Prom
	{ "pr1636.70",    0x0100, 0x77245b66, 0 | BRF_SND | BRF_OPT },	// 13 Timing Prom
};

STD_ROM_PICK(pengo2u)
STD_ROM_FN(pengo2u)

#endif