#include "d_segae.h"
#define SELECTED_CHIP 1
// based on MESS/MAME driver by David Haywood
Fixed32	ss_scl1[SCL_MAXLINE];
volatile SysPort	*ss_port;

int ovlInit(char *szShortName)
{
	struct BurnDriver nBurnDrvHangonjr = {
		"hangonjr", "segae", 
		"Hang-On Jr.", 
		hangonjrRomInfo, hangonjrRomName, HangonjrInputInfo, HangonjrDIPInfo,
		DrvHangonJrInit, DrvExit, DrvFrame, NULL
	};

	struct BurnDriver nBurnDrvTetrisse = {
		"tetrisse", "segae",
		"Tetris (Japan, System E)",
		TetrisseRomInfo, TetrisseRomName, TetrisseInputInfo, TetrisseDIPInfo,
		DrvTetrisInit, DrvExit, DrvFrame, NULL
	};

	struct BurnDriver nBurnDrvTransfrm = {
		"transfrm", "segae",
		"Transformer",
		TransfrmRomInfo, TransfrmRomName, TransfrmInputInfo, TransfrmDIPInfo,
		DrvTransfrmInit, DrvExit, DrvFrame, NULL
	};

	struct BurnDriver nBurnDrvFantzn2 = {
		"fantzn2", "segae",
		"Fantasy Zone 2 - The Tears of Opa-Opa",
		fantzn2RomInfo, fantzn2RomName, TransfrmInputInfo, Fantzn2DIPInfo,
		DrvFantzn2Init, DrvExit, DrvFrame, NULL
	};

	if (strcmp(nBurnDrvHangonjr.szShortName, szShortName) == 0) 
	memcpy(shared,&nBurnDrvHangonjr,sizeof(struct BurnDriver));
	if (strcmp(nBurnDrvTetrisse.szShortName, szShortName) == 0) 
	memcpy(shared,&nBurnDrvTetrisse,sizeof(struct BurnDriver));
	if (strcmp(nBurnDrvTransfrm.szShortName, szShortName) == 0) 
	memcpy(shared,&nBurnDrvTransfrm,sizeof(struct BurnDriver));
	if (strcmp(nBurnDrvFantzn2.szShortName, szShortName) == 0) 
	memcpy(shared,&nBurnDrvFantzn2,sizeof(struct BurnDriver));

	ss_reg    = (SclNorscl *)SS_REG;
	ss_regs  = (SclSysreg *)SS_REGS;
	ss_port  = (SysPort *)SS_PORT;
}

static UINT8 __fastcall systeme_main_read(UINT16 address)
{
//	bprintf(0, _T("systeme_main_read adr %X.\n"), address);
	return 0;
}

static void __fastcall systeme_main_write(UINT16 address, UINT8 data)
{
	if(address >= 0x8000 && address <= 0xbfff)
	{
		segae_vdp_vram [1-segae_8000bank][(address - 0x8000) + (0x4000-(segae_vdp_vrambank[1-segae_8000bank] * 0x4000))] = data;

		return;
	}
}

static UINT32 scalerange(UINT32 x, UINT32 in_min, UINT32 in_max, UINT32 out_min, UINT32 out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


static UINT8 scale_wheel(UINT32 PaddlePortnum) {
	UINT8 Temp;

	Temp = 0x7f + (PaddlePortnum >> 4);
	if (Temp < 0x01) Temp = 0x01;
	if (Temp > 0xfe) Temp = 0xfe;
	Temp = scalerange(Temp, 0x3f, 0xbe, 0x20, 0xe0);
	return Temp;
}

static UINT8 scale_accel(UINT32 PaddlePortnum) {
	UINT8 Temp;

	Temp = PaddlePortnum >> 4;
	if (Temp < 0x08) Temp = 0x00; // sometimes default for digital button -> analog is "1"
	if (Temp > 0x30) Temp = 0xff;

	return Temp;
}


static UINT8 __fastcall hangonjr_port_f8_read (UINT8 port)
{
	UINT8 temp = 0;
	//bprintf(0, _T("Wheel %.04X  Accel %.04X\n"), scale_wheel(DrvWheel), scale_accel(DrvAccel));
// � modifier pour gerer la manette
// if (!wheel_pressed) { if (wheel < 0) wheel++; if (wheel > 0) wheel--; // autocenter }
//if (left_pressed) wheel--; else if (right_pressed) wheel++;
	SysDevice	*device;

	if(( device = PER_GetDeviceR( &ss_port[0], 0 )) != NULL )
	{
		FNT_Print256_2bpp((volatile Uint8 *)ss_font,(Uint8 *)"button pushed ",0,180);	

//		pltriggerE[0] = pltrigger[0];
		pltrigger[0] = PER_GetTrigger( device );
//		pltriggerE[0] = (pltrigger[0]) ^ (pltriggerE[0]);
//		pltriggerE[0] = (pltrigger[0]) & (pltriggerE[0]);
	}
	else
	{
		pltrigger[0] = 0;
	}

	if (port_fa_last == 0x08)  /* 0000 1000 */ /* Angle */
	{
		if(pltrigger[0] & PER_DGT_L )
		{
			if(DrvWheel>-64)
				DrvWheel-=4;
		}
		else if(pltrigger[0] & PER_DGT_R )
		{
			if(DrvWheel<63)
				DrvWheel+=4;
		}
		else
		{
			if(DrvWheel>1)
				DrvWheel-=4;
			else if(DrvWheel<63)
				DrvWheel+=4;
		}

		temp = 0x7f + DrvWheel;
		temp = scalerange(temp, 0x3f, 0xbe, 0x20, 0xe0);
	}

	if (port_fa_last == 0x09)  /* 0000 1001 */ /* Accel */
	{
		if(pltrigger[0] & PER_DGT_B )
		{
			if(DrvAccel<63)
				DrvAccel+=4;
		}
		else
		{
			if(DrvAccel>-64)
				DrvAccel-=4;
		}

		temp=DrvAccel;

		if(DrvAccel<8)
			temp=0x00;
		else if(DrvAccel>0x30)
			temp=0xff;
		else
			temp=0xff;
	}
	return temp;
}

static inline void __fastcall hangonjr_port_fa_write (UINT8 data)
{
	/* Seems to write the same pattern again and again bits ---- xx-x used */
	port_fa_last = data;
}

static void segae_bankswitch (void)
{
	UINT32 bankloc = 0x10000 + rombank * 0x4000;

	CZetMapArea(0x8000, 0xbfff, 0, DrvMainROM + bankloc);
	CZetMapArea(0x8000, 0xbfff, 2, DrvMainROM + bankloc);

	if (mc8123_banked) {
		CZetMapArea2(0x8000, 0xbfff, 2, DrvMainROMFetch + bankloc, DrvMainROM + bankloc); // fetch ops(encrypted), opargs(unencrypted)
	}
}

static void __fastcall bank_write(UINT8 data)
{
	segae_vdp_vrambank[0]	= (data & 0x80) >> 7; /* Back  Layer VDP (0) VRAM Bank */
	segae_vdp_vrambank[1]	= (data & 0x40) >> 6; /* Front Layer VDP (1) VRAM Bank */
	segae_8000bank			= (data & 0x20) >> 5; /* 0x8000 Write Select */
	rombank					=  data & 0x0f;		  /* Rom Banking */
	segae_bankswitch();
}

static void segae_vdp_setregister ( UINT8 chip, UINT16 cmd )
{
	UINT8 regnumber;
	UINT8 regdata;

	regnumber = (cmd & 0x0f00) >> 8;
	regdata   = (cmd & 0x00ff);
	if (regnumber < 11) {
		segae_vdp_regs[chip][regnumber] = regdata;

//		if(chip==SELECTED_CHIP)
		{
			switch(regnumber)
			{
				case 0x02: /* Name Table A Base Address */
					ntab[chip] = (segae_vdp_regs[chip][regnumber] << 10) & 0x3800;
					break;

				case 0x05: /* Sprite Attribute Table Base Address */
//					satb[chip] = ((segae_vdp_regs[chip][regnumber] << 7) + (segae_vdp_vrambank[chip] * 0x4000)) & 0x3F00;
//					if (segae_vdp_vrambank[chip])
						satb[chip] = ((segae_vdp_regs[chip][regnumber] << 7) + (segae_vdp_vrambank[chip] * 0x4000)) & 0x3F00;
//					else
//						satb[chip] = (segae_vdp_regs[chip][regnumber] << 7) & 0x3F00;
					break;

				case 0x08 :
						scroll_x[chip] =  ((segae_vdp_regs[chip][regnumber]) ^ 0xff) ;
					break;
				case 0x09 :
						scroll_y[chip] = (segae_vdp_regs[chip][regnumber]&0xff);
					break;
			}
		}


	} else {
		/* Illegal, there aren't this many registers! */
	}
}

static void segae_vdp_processcmd ( UINT8 chip, UINT16 cmd )
{
	if ( (cmd & 0xf000) == 0x8000 ) { /*  1 0 0 0 - - - - - - - - - - - -  VDP Register Set */
		segae_vdp_setregister (chip, cmd);
	} else { /* Anything Else */
		segae_vdp_accessmode[chip] = (cmd & 0xc000) >> 14;
		segae_vdp_accessaddr[chip] = (cmd & 0x3fff);

		if ((segae_vdp_accessmode[chip]==0x03) && (segae_vdp_accessaddr[chip] > 0x1f) ) { /* Check Address is valid for CRAM */
			/* Illegal, CRAM isn't this large! */
			segae_vdp_accessaddr[chip] &= 0x1f;
		}

		if (segae_vdp_accessmode[chip] == 0x00) { /*  0 0 - - - - - - - - - - - - - -  VRAM Acess Mode (Special Read) */
			segae_vdp_readbuffer[chip] = segae_vdp_vram[chip][ segae_vdp_vrambank[chip]*0x4000 + segae_vdp_accessaddr[chip] ];
			segae_vdp_accessaddr[chip] += 1;
			segae_vdp_accessaddr[chip] &= 0x3fff;
		}
	}
}

static UINT8 segae_vdp_counter_r (UINT8 chip, UINT8 offset)
{
	UINT8 temp = 0;
	UINT16 sline;

	switch (offset)
	{
		case 0: /* port 0x7e, Vert Position (in scanlines) */
			sline = currentLine;
//			if (sline > 0xDA) sline -= 6;
//			temp = sline-1 ;
			if (sline > 0xDA) sline -= 5;
			temp = sline ;
			break;
		case 1: /* port 0x7f, Horz Position (in pixel clock cycles)  */
			/* unhandled for now */
			break;
	}
	return temp;
}

static UINT8 segae_vdp_data_r(UINT8 chip)
{
	UINT8 temp;

	segae_vdp_cmdpart[chip] = 0;

	temp = segae_vdp_readbuffer[chip];

	if (segae_vdp_accessmode[chip]==0x03) { /* CRAM Access */
		/* error CRAM can't be read!! */
	} else { /* VRAM */
		segae_vdp_readbuffer[chip] = segae_vdp_vram[chip][ segae_vdp_vrambank[chip]*0x4000 + segae_vdp_accessaddr[chip] ];
		segae_vdp_accessaddr[chip] += 1;
		segae_vdp_accessaddr[chip] &= 0x3fff;
	}
	return temp;
}

static UINT8 segae_vdp_reg_r ( UINT8 chip )
{
	UINT8 temp;

	temp = 0;

	temp |= (vintpending << 7);
	temp |= (hintpending << 6);

	hintpending = vintpending = 0;

	return temp;
}

static inline UINT8 pal2bit(UINT8 bits)
{
	bits &= 3;
	return (bits << 6) | (bits << 4) | (bits << 2) | bits;
}

static void segae_vdp_data_w ( UINT8 chip, UINT8 data )
{
	segae_vdp_cmdpart[chip] = 0;

	if (segae_vdp_accessmode[chip]==0x03) 
	{ /* CRAM Access */
		UINT8 temp;

		temp = segae_vdp_cram[chip][segae_vdp_accessaddr[chip]];

		if (temp != data) 
		{
			segae_vdp_cram[chip][segae_vdp_accessaddr[chip]] = data;

			UINT8 index = segae_vdp_accessaddr[chip] & 0x1F;

//			if(chip==SELECTED_CHIP)
			{
				colBgAddr[index+(chip<<8)] = cram_lut[data & 0x3F];
				if(index >0x0f)
					colAddr[(index&0x0f)+(chip*16)] =  cram_lut[data & 0x3F];
			}
		}

		segae_vdp_accessaddr[chip] += 1;
		segae_vdp_accessaddr[chip] &= 0x1f;
	} 
	else 
	{ /* VRAM Accesses */

		UINT32 index = segae_vdp_vrambank[chip]*0x4000 + (segae_vdp_accessaddr[chip] & 0x3fff);
//		segae_vdp_vram[chip][ segae_vdp_vrambank[chip]*0x4000 + segae_vdp_accessaddr[chip] ] = data;

		segae_vdp_accessaddr[chip] += 1;
		segae_vdp_accessaddr[chip] &= 0x3fff;
//		 index = (segae_vdp_accessaddr[chip] & 0x3fff);

//		if(chip==SELECTED_CHIP)
		if (segae_vdp_vram[chip][index] != data) 
		{
			segae_vdp_vram[chip][index] = data;
			update_bg(chip, index);		
			update_sprites(chip, index);		
		}
	}
}

static void segae_vdp_reg_w ( UINT8 chip, UINT8 data )
{
	if (!segae_vdp_cmdpart[chip]) {
		segae_vdp_cmdpart[chip] = 1;
		segae_vdp_command[chip] = data;
	} else {
		segae_vdp_cmdpart[chip] = 0;
		segae_vdp_command[chip] |= (data << 8);
		segae_vdp_processcmd (chip, segae_vdp_command[chip]);
	}
}

/*static UINT8 input_r(INT32 offset)
{
	//bprintf(0, _T("input_r chip %X.\n"), offset);
	return 0xff;
}*/

static UINT8 __fastcall systeme_main_in(UINT16 port)
{
	port &= 0xff;

	switch (port) {
		case 0x7e: return segae_vdp_counter_r(0, 0);
		case 0x7f: return segae_vdp_counter_r(0, 1);
		case 0xba: return segae_vdp_data_r(0);
		case 0xbb: return segae_vdp_reg_r(0);

		case 0xbe: return segae_vdp_data_r(1);
		case 0xbf: return segae_vdp_reg_r(1);

		case 0xe0: return 0xff - DrvInput[0];
		case 0xe1: return 0xff - DrvInput[1];
//		case 0xe2: return input_r(port); // AM_RANGE(0xe2, 0xe2) AM_READ_PORT( "e2" )
		case 0xf2: return DrvDip[0];//input_r(port); // AM_RANGE(0xf2, 0xf2) AM_READ_PORT( "f2" ) /* DSW0 */
		case 0xf3: return DrvDip[1];//input_r(port); // AM_RANGE(0xf3, 0xf3) AM_READ_PORT( "f3" ) /* DSW1 */
		case 0xf8: return hangonjr_port_f8_read(port); // m_maincpu->space(AS_IO).install_read_handler(0xf8, 0xf8, read8_delegate(FUNC(systeme_state::hangonjr_port_f8_read), this));
	}	
	////bprintf(PRINT_NORMAL, _T("IO Read %x\n"), a);
	return 0;
}

static void __fastcall systeme_main_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x7b: SN76496Write(0, data);
		break;

		case 0x7f: SN76496Write(1, data);
		break;

		case 0xba: segae_vdp_data_w(0, data);
		break;

		case 0xbb: segae_vdp_reg_w(0, data);
		break;

		case 0xbe: segae_vdp_data_w(1, data);
		break;

		case 0xbf:	segae_vdp_reg_w(1, data);
		break;

		case 0xf7:	bank_write(data);
		break;

		case 0xfa:	hangonjr_port_fa_write(data);
		break;
	}
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvMainROM	 	    = (UINT8 *) 0x00200000;
	DrvMainROMFetch= (UINT8 *) 0x00280000;
	mc8123key           = Next; Next += 0x02000;
	
	AllRam				= Next;
	DrvRAM			    = Next; Next += 0x10000;

	segae_vdp_vram[0]	= Next; Next += 0x8000; /* 32kb (2 banks) */
	segae_vdp_vram[1]	= Next; Next += 0x8000; /* 32kb (2 banks) */

	segae_vdp_cram[0]	= Next; Next += 0x20;
	segae_vdp_regs[0]	= Next; Next += 0x20;

	segae_vdp_cram[1]	= Next; Next += 0x20;
	segae_vdp_regs[1]	= Next; Next += 0x20;

//	cache_bitmap		= Next; Next += ( (16+256+16) * 192+17 ) + 0x0f; /* 16 pixels either side to simplify drawing + 0xf for alignment */

//	DrvPalette			= (UINT32*)Next; Next += 0x040 * sizeof(UINT32);
//	Palette			    = (UINT32*)Next; Next += 0x040 * sizeof(UINT32);

	RamEnd			= Next;
	
	name_lut		= Next; Next += 0x10000*sizeof(UINT16);
	bp_lut			= Next; Next += 0x10000*sizeof(UINT32);
	cram_lut		= Next; Next += 0x40*sizeof(UINT16);
	map_lut	 		= Next; Next += 0x800*sizeof(UINT16);	
	
	MemEnd			= Next;

	return 0;
}

//-----------------------
static INT32 DrvExit()
{
	nBurnFunction = NULL;
	CZetExit();

	DrvMainROM	 = AllRam = DrvRAM = MemEnd = segae_vdp_vram[0]	= segae_vdp_vram[1]	= NULL;
	segae_vdp_cram[0] = segae_vdp_regs[0] = segae_vdp_cram[1] = segae_vdp_regs[1] = NULL;
	name_lut = cram_lut = map_lut = NULL;
	bp_lut = NULL;
	free(AllMem);
	AllMem = NULL;

	leftcolumnblank = 0;
	mc8123 = 0;
	mc8123_banked = 0;

	return 0;
}

static INT32 DrvDoReset()
{
	memset (DrvRAM, 0, RamEnd - DrvRAM);

	hintcount = 0;
	vintpending = 0;
	hintpending = 0;
//	SN76496Reset();
	CZetOpen(0);
	segae_bankswitch();
	CZetReset();
	CZetClose();
	
	return 0;
}
//-------------------------------------------------------------------------------------------------------------------------------------
static inline void DrvClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------
static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = DrvInput[1] = DrvInput[2] = DrvInput[3] = DrvInput[4] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy0[i] & 1) << i;
		DrvInput[1] |= (DrvJoy1[i] & 1) << i;
		DrvInput[2] |= (DrvJoy2[i] & 1) << i;
		DrvInput[3] |= (DrvJoy3[i] & 1) << i;
		DrvInput[4] |= (DrvJoy4[i] & 1) << i;
	}

	// Clear Opposites
	DrvClearOpposites(&DrvInput[0]);
	DrvClearOpposites(&DrvInput[1]);
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void segae_interrupt ()
{
	if (currentLine == 0) {
		hintcount = segae_vdp_regs[1][10];
	}

	if (currentLine <= 192) {

//		if (currentLine != 192) segae_drawscanline(currentLine);
/*
		if (segae_vdp_regs[0][1] & 0x40) 
		{
			render_obj(0, currentLine);
		}

		if (segae_vdp_regs[1][1] & 0x40) 
		{
			render_obj(1, currentLine);
		}
*/
		if (currentLine == 192)
			vintpending = 1;

		if (hintcount == 0) {
			hintcount = segae_vdp_regs[1][10];
			hintpending = 1;

			if  ((segae_vdp_regs[1][0] & 0x10)) {
				CZetSetIRQLine(0, CZET_IRQSTATUS_AUTO);
				return;
			}

		} else {
			hintcount--;
		}
	}

	if (currentLine > 192) {
		hintcount = segae_vdp_regs[1][10];

		if ( (currentLine<0xe0) && (vintpending) ) {
			CZetSetIRQLine(0, CZET_IRQSTATUS_AUTO);
		}
	}
}


static INT32 DrvFrame()
{
	INT32 nInterleave = 262;
	*(Uint16 *)0x25E00000 = colBgAddr[0]; // set bg_color

//	if (DrvReset) DrvDoReset();
	DrvMakeInputs();

	nCyclesTotal = 10738635 / 2 / 60;
	nCyclesDone = 0;
	currentLine = 0;
	CZetOpen(0);

	CZetNewFrame();

	for (UINT32 i = 0; i < nInterleave; i++) {
		UINT32 nNext;

		// Run Z80 #1
		nNext = (i + 1) * nCyclesTotal / nInterleave;
		nCyclesSegment = nNext - nCyclesDone;
		nCyclesDone += CZetRun(nCyclesSegment);
		currentLine = (i - 4) & 0xff;

		if(currentLine>7)
		{
			ss_scl[currentLine] = scroll_x[0] << 16;
			ss_scl1[currentLine] = scroll_x[1] << 16;
		}
		segae_interrupt();
	}
	CZetClose();

	signed short *nSoundBuffer = (signed short *)(0x25a20000+nSoundBufferPos*(sizeof(signed short)));
//	if (pBurnSoundOut)
	{
		SN76496Update(0, nSoundBuffer, nBurnSoundLen);
		SN76496Update(1, nSoundBuffer, nBurnSoundLen);
	}	

//	SCL_SetLineParamNBG1();
	nSoundBufferPos+=nBurnSoundLen; // DOIT etre deux fois la taille copiee

	if(nSoundBufferPos>=0x3C00)//RING_BUF_SIZE)
//	if(nSoundBufferPos>=(2048L*10))
	{
		PCM_NotifyWriteSize(pcm, nSoundBufferPos);
		nSoundBufferPos=0;
	}
	PCM_Task(pcm); // bon emplacement

	ss_reg->n0_move_y = scroll_y[0]<<16;
	ss_reg->n1_move_y = scroll_y[1]<<16;

	return 0;
}

static INT32 DrvInit(UINT8 game)
{
	DrvInitSaturn(game);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) 
	{
		return 0;
	}

	memset(AllMem, 0, nLen);
	MemIndex();

	switch (game) {
		case 0:
			if (BurnLoadRom(DrvMainROM + 0x00000,  0, 1)) return 1;	// ( "rom5.ic7",   0x00000, 0x08000, CRC(d63925a7) SHA1(699f222d9712fa42651c753fe75d7b60e016d3ad) ) /* Fixed Code */
			if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;	// ( "rom4.ic5",   0x10000, 0x08000, CRC(ee3caab3) SHA1(f583cf92c579d1ca235e8b300e256ba58a04dc90) )
			if (BurnLoadRom(DrvMainROM + 0x18000,  2, 1)) return 1;	// ( "rom3.ic4",   0x18000, 0x08000, CRC(d2ba9bc9) SHA1(85cf2a801883bf69f78134fc4d5075134f47dc03) )
			break;
		case 1:
		case 2:
			if (BurnLoadRom(DrvMainROM + 0x00000,  0, 1)) return 1;	// ( "rom5.ic7",   0x00000, 0x08000, CRC(d63925a7) SHA1(699f222d9712fa42651c753fe75d7b60e016d3ad) ) /* Fixed Code */
			if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;	// ( "rom4.ic5",   0x10000, 0x08000, CRC(ee3caab3) SHA1(f583cf92c579d1ca235e8b300e256ba58a04dc90) )
			if (BurnLoadRom(DrvMainROM + 0x18000,  2, 1)) return 1;	// ( "rom3.ic4",   0x18000, 0x08000, CRC(d2ba9bc9) SHA1(85cf2a801883bf69f78134fc4d5075134f47dc03) )
			if (BurnLoadRom(DrvMainROM + 0x20000,  3, 1)) return 1;	// ( "rom2.ic3",   0x20000, 0x08000, CRC(e14da070) SHA1(f8781f65be5246a23c1f492905409775bbf82ea8) )
			if (BurnLoadRom(DrvMainROM + 0x28000,  4, 1)) return 1; // ( "rom1.ic2",   0x28000, 0x08000, CRC(3810cbf5) SHA1(c8d5032522c0c903ab3d138f62406a66e14a5c69) )
			break;
		case 3: // fantzn2
			if (BurnLoadRom(DrvMainROM + 0x00000,  0, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x20000,  2, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x30000,  3, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x40000,  4, 1)) return 1;
			if (BurnLoadRom(mc8123key  + 0x00000,  5, 1)) return 1;
			mc8123_decrypt_rom(0, 0, DrvMainROM, DrvMainROMFetch, mc8123key);
			mc8123 = 1;

			break;
		case 4: // opaopa
			if (BurnLoadRom(DrvMainROM + 0x00000,  0, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x18000,  2, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x20000,  3, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x28000,  4, 1)) return 1;
			if (BurnLoadRom(mc8123key  + 0x00000,  5, 1)) return 1;
			mc8123_decrypt_rom(1, 16, DrvMainROM, DrvMainROMFetch, mc8123key);
			mc8123 = 1;
			mc8123_banked = 1;
			break;
	}

	CZetInit(1);
	CZetOpen(0);
//	CZetMapMemory(DrvMainROM, 0x0000, 0x7fff, MAP_ROM);
	CZetMapArea(0x0000, 0x7fff, 0, DrvMainROM + 0x0000);
	CZetMapArea(0x0000, 0x7fff, 2, DrvMainROM + 0x0000);

//	CZetMapMemory(DrvRAM, 0xc000, 0xffff, MAP_RAM);
	CZetMapArea(0xc000, 0xffff, 0, DrvRAM);
	CZetMapArea(0xc000, 0xffff, 1, DrvRAM);
	CZetMapArea(0xc000, 0xffff, 2, DrvRAM);

	CZetSetWriteHandler(systeme_main_write);
	CZetSetReadHandler(systeme_main_read);
	CZetSetInHandler(systeme_main_in);
	CZetSetOutHandler(systeme_main_out);

	CZetClose();

	SN76489Init(0, 10738635 / 3, 0);
	SN76489Init(1, 10738635 / 3, 1);

//	SN76496SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);
//	SN76496SetRoute(1, 0.50, BURN_SND_ROUTE_BOTH);

//	GenericTilesInit();

	for (INT32 i = 0; i < 0x40; i++) 
	{
		colAddr[i] = colBgAddr[i] = colBgAddr[i+256] = cram_lut[i];
	}

	DrvDoReset();
	make_lut();
//	drawWindow(0,192,192,2,66);
	return 0;
}

static INT32 DrvFantzn2Init()
{
	leftcolumnblank = 1;
//	leftcolumnblank_special = 1;
//	sprite_bug = 1;

	return DrvInit(3);
}

static INT32 DrvTransfrmInit()
{
	leftcolumnblank = 1;

	return DrvInit(2);
}

static INT32 DrvHangonJrInit()
{
	leftcolumnblank = 1;

	return DrvInit(1);
}

static INT32 DrvTetrisInit()
{
	return DrvInit(0);
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void initColors()
{
	memset(SclColRamAlloc256,0,sizeof(SclColRamAlloc256));
	colBgAddr		= (Uint16*)SCL_AllocColRam(SCL_NBG0,ON);
	SCL_AllocColRam(SCL_NBG1,OFF);
	colAddr			= (Uint16*)SCL_AllocColRam(SCL_SPR,OFF);
	SCL_AllocColRam(SCL_NBG3,OFF);
	(Uint16*)SCL_AllocColRam(SCL_NBG2,OFF);
//	SCL_SetColRam(SCL_NBG1,0,8,palette);
//	SCL_SetColRam(SCL_NBG1,8,8,palette);
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void initLayers(void)
{
//    SclConfig	config;
// **29/01/2007 : VBT sauvegarde cycle patter qui fonctionne jusqu'� maintenant

	Uint16	CycleTb[]={
		  // VBT 04/02/2007 : cycle pattern qui fonctionne just test avec des ee
		0xff15, 0xefff, //A1
		0xffff, 0xffff,	//A0
		0x04ee, 0xffff,   //B1
		0xffff, 0xffff  //B0
	};
 	SclConfig	scfg;

//	SCL_InitConfigTb(&scfg);
	scfg.dispenbl		= ON;
	scfg.charsize		= SCL_CHAR_SIZE_1X1;//OK du 1*1 surtout pas toucher
#ifdef TWO_WORDS
	scfg.pnamesize	= SCL_PN2WORD;
#else
	scfg.pnamesize	= SCL_PN1WORD;
	scfg.flip				= SCL_PN_10BIT;
#endif
	scfg.platesize	= SCL_PL_SIZE_1X1;
//#ifdef GG
#if 0
	scfg.coltype		= SCL_COL_TYPE_256;
#else
	scfg.coltype		= SCL_COL_TYPE_16;
#endif
	scfg.datatype	= SCL_CELL;
// vbt : active le prioritybit en mode 1word
	scfg.patnamecontrl =  0x0000;// VRAM A0
	scfg.plate_addr[0] = ss_map;
	SCL_SetConfig(SCL_NBG0, &scfg);


	scfg.patnamecontrl =  0x0002;// VRAM A0 + 0x10000
	scfg.plate_addr[0] = ss_map2;
	SCL_SetConfig(SCL_NBG1, &scfg);
/********************************************/	

//	SCL_InitConfigTb(&scfg);
	scfg.dispenbl 	 = OFF;
	scfg.bmpsize 		 = SCL_BMP_SIZE_512X256;
	scfg.datatype 	 = SCL_BITMAP;
	scfg.mapover       = SCL_OVER_0;
	scfg.plate_addr[0] = ss_font;
	SCL_SetConfig(SCL_NBG2, &scfg);
	SCL_SetCycleTable(CycleTb);
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void initPosition(void)
{
	SCL_Open();
	ss_reg->n1_move_x = 0;
	ss_reg->n1_move_y = 0;
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void DrvInitSaturn(UINT8 game)
{
	SPR_InitSlaveSH();

	nBurnSprites  = 131;//27;
	nBurnLinescrollSize = 0x340;
	nSoundBufferPos = 0;//sound position � renommer

 	SS_MAP  = ss_map		=(Uint16 *)SCL_VDP2_VRAM_B1+0xC000;		   //c
	SS_MAP2 = ss_map2	=(Uint16 *)SCL_VDP2_VRAM_B1+0x8000;			//8000
	SS_FONT = ss_font		=(Uint16 *)SCL_VDP2_VRAM_B1+0x0000;
	SS_CACHE= cache		=(Uint8  *)SCL_VDP2_VRAM_A0;

	ss_BgPriNum     = (SclBgPriNumRegister *)SS_N0PRI;
	ss_SpPriNum     = (SclSpPriNumRegister *)SS_SPPRI;
#ifdef TWO_WORDS
	ss_OtherPri       = (SclOtherPriRegister *)SS_OTHR;
#endif
	ss_sprite		= (SprSpCmd *)SS_SPRIT;
	ss_scl			= (Fixed32 *)SS_SCL;

#ifdef TWO_WORDS
    SS_SET_N0PRIN(4);
    SS_SET_S0PRIN(4);	
	SS_SET_N0SPRM(1);  // 1 for special priority	  // mode 2 pixel, mode 1 pattern, mode 0 by screen
	ss_regs->specialcode=0x000e; // sfcode, upper 8bits, function b, lower 8bits function a
	ss_regs->specialcode_sel=0; // sfsel, bit 0 for nbg0 // 1 sfcs, bit 0 = 1 for funcion code b, 0 for function code a
	SclProcess = 2; 
#else
    SS_SET_N0PRIN(4);
    SS_SET_S0PRIN(7);
#endif
	SS_SET_N1PRIN(5);
	SS_SET_N2PRIN(6);

	initLayers();
	initColors();
	initPosition();
		FNT_Print256_2bpp((volatile Uint8 *)ss_font,(Uint8 *)" ",0,180);	

	if(game==1)
	{
			initSprites(240-1,192-1,16,0,0,0);
		SCL_SetWindow(SCL_W0,SCL_NBG0,SCL_NBG1,SCL_NBG1,17,0,240,191);
		SCL_SetWindow(SCL_W1,SCL_NBG1,SCL_NBG0,SCL_NBG0,17,0,240,191);
	}
	else
	{
		initSprites(256-1,192-1,0,0,0,0);
		SCL_SetWindow(SCL_W0,SCL_NBG0,SCL_NBG1,SCL_NBG1,0,0,256,191);
		SCL_SetWindow(SCL_W1,SCL_NBG1,SCL_NBG0,SCL_NBG0,0,0,256,191);
	}

	initScrolling(ON,SCL_VDP2_VRAM_B0);
	initScrollingNBG1(ON,SCL_VDP2_VRAM_B0+0x8000);

	FNT_Print256_2bpp((volatile Uint8 *)ss_font,(Uint8 *)" ",0,180);	
	nBurnFunction = SCL_SetLineParamNBG1;

	UINT8 *ss_vram = (UINT8 *)SS_SPRAM;
	memset(&ss_vram[0x1100],0,0x12000);
}
//-------------------------------------------------------------------------------------------------------------------------------------
void SCL_SetLineParamNBG1()
{
	UINT32 *address		=(UINT32 *)(SCL_VDP2_VRAM_B0+0x8000);	
	memcpyl((void *)address,(void *)&ss_scl1[0], nBurnLinescrollSize);
}
//-------------------------------------------------------------------------------------------------------------------------------------
void initScrollingNBG1(UINT8 enabled,UINT32 address)
{
	UINT16 temp = ss_reg->linecontrl;
	UINT32	*addr;

	ss_reg->linecontrl = (temp <<8) & 0xff00 | temp;
	addr = &ss_reg->lineaddr[1];
	*addr = (address / 2) & 0x0007ffff;
	SclProcess = 2;
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void update_sprites(UINT8 chip, UINT32 index)
{
	if(index>=satb[chip])
		if( index < satb[chip]+0x40)
	{
		SprSpCmd *ss_spritePtr;
		ss_spritePtr = &ss_sprite[3+(chip<<6)];

		// Sprite dimensions 
		unsigned int height = (segae_vdp_regs[chip][1] & 0x02) ? 16 : 8;
		unsigned int width  = 8;
		unsigned int delta=(index-satb[chip]);

	// Pointer to sprite attribute table 
		UINT8 *st = (UINT8 *)&segae_vdp_vram[chip][satb[chip]];
		// Sprite Y position 
		int yp = st[delta];

		if(yp == 208) 
		{

//			ss_spritePtr[delta].control = CTRL_END;
			for(int x=delta;x<64;x++)
			{
//				ss_spritePtr[x].drawMode = 0;
//				ss_spritePtr[x].charAddr	= 0;
//				ss_spritePtr[x].charSize		= 0;
//				ss_spritePtr[x].ax	= 0;
				ss_spritePtr[63-x].ay	= yp;
			}

			return;
		}
		unsigned int inv_delta=63-delta;
		//Actual Y position is +1 
		yp ++;
		//Wrap Y coordinate for sprites > 240 
		if(yp > 240) yp -= 256;
		ss_spritePtr[inv_delta].ay = yp;

		/* Adjust dimensions for double size sprites */
		if(segae_vdp_regs[chip][1] & 0x01)
		{
			width  = 16;
			height *= 2;
		}

		// Clip sprites on left edge 
		ss_spritePtr[inv_delta].control = ( JUMP_NEXT | FUNC_NORMALSP);
		ss_spritePtr[inv_delta].drawMode   = ( COLOR_0 | ECD_DISABLE | COMPO_REP);		
		ss_spritePtr[inv_delta].charSize    = (width<<5) + height;  //0x100
		ss_spritePtr[inv_delta].color    = chip<<4;
	}

	if(index>=satb[chip]+0x80)
		if(index < satb[chip]+0x100)
	{
		SprSpCmd *ss_spritePtr;
		ss_spritePtr = &ss_sprite[3+(chip<<6)];

		UINT8 *st = (UINT8 *)&segae_vdp_vram[chip][satb[chip]];
		unsigned int delta=(((index-(satb[chip]+0x80)))>>1);
		unsigned int inv_delta=63-delta;

//		delta &= 0x1f;

		if((index-satb[chip]) &1)
		{
			int n = st[0x81 + (delta << 1)];
			//Add MSB of pattern name 
			if(segae_vdp_regs[chip][6] & 0x04) n |= 0x0100;
			//Mask LSB for 8x16 sprites 
			if(segae_vdp_regs[chip][1] & 0x02) n &= 0x01FE;

//					ss_sprite[delta+3].charAddr   =  0x220; //+(n<<2);
			ss_spritePtr[inv_delta].charAddr   =  0x220+(0x2000*chip) +(n<<2); // +((64*chip)<<2);
		}
		else
		{
				//Sprite X position 
			int xp = st[0x80 + (delta << 1)];
			//X position shift 
			if(segae_vdp_regs[chip][0] & 0x08) xp -= 8;
			ss_spritePtr[inv_delta].ax = xp;
		}

	}
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void update_bg(UINT8 chip, UINT32 index)
{
/// 		segae_vdp_vram[chip][ segae_vdp_vrambank[chip]*0x4000 + segae_vdp_accessaddr[chip] ] = data;

	if(index>=ntab[chip])
	{
		if( index<ntab+0x700)
		{
			UINT16 temp = *(UINT16 *)&segae_vdp_vram[chip][index&~1];
			unsigned int delta = map_lut[index - ntab[chip]];
			UINT16 *map = (chip==0?&ss_map[delta]:&ss_map2[delta]); 

#ifdef TWO_WORDS
			map[0] =map[64] =map[0xE00] =map[0xE40] = name_lut[temp];//color + flip + prio
			map[1] =map[65] =map[0xE01] =map[0xE41] = ((temp >> 8) & 0xFF) | ((temp  & 0x01) <<8) + 0x3000; //tilenum c00 1800
#else
			map[0] =map[32] =map[0x700] =map[0x720] =name_lut[temp];
#endif
		}
	}
	/* Mark patterns as dirty */
	UINT8 *ss_vram = (UINT8 *)SS_SPRAM;
	UINT32 bp = *(UINT32 *)&segae_vdp_vram[chip][index & ~3];
	UINT32 *pg = (UINT32 *) &cache[(0x00000|(chip<<16)) | (index & ~3)];
	UINT32 *sg = (UINT32 *)&ss_vram[0x1100+ (chip<<16) + (index & ~3)];
	UINT32 temp1 = bp_lut[bp & 0xFFFF];
	UINT32 temp2 = bp_lut[(bp>>16) & 0xFFFF];
	*sg= *pg = (temp1<<2 | temp2 );
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void make_map_lut()
{
	unsigned int row,column;

	for (int i = 0; i < 0x800;i++) 
	{
		row = i & 0x7C0;
		column = (i>>1) & 0x1F;
#ifdef TWO_WORDS
		map_lut[i] = (row+column)<<1;
#else
		map_lut[i] = row+column;
#endif
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void make_name_lut()
{
	unsigned int i, j;
	for(j = 0; j < 0x10000; j++)
	{
		i = ((j >> 8) & 0xFF) | ((j  & 0xFF) <<8);
		unsigned int flip = (i >> 9) & 3;
		unsigned int pal = (i >> 11) & 1;
#ifdef TWO_WORDS
/*
Bit 15 - 13: Unused
Bit 12: Priority flag
Bit 11: Which palette to use
Bit 10: Vertical Flip Flag
Bit 09: Horizontal Flip Flag
Bit 08 - 00 : Pattern Index 
*/
		unsigned int priority = (i >> 12) & 1;
		name_lut[j] = (flip << 14 | priority << 13 | pal);
#else
		unsigned int name = (i & 0x1FF);
		name_lut[j] = (pal << 12 | flip << 10 | name);
#endif
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void make_bp_lut(void)
{
//	bp_lut = (UINT32 *)malloc(0x10000*sizeof(UINT32));
    unsigned int i, j;
    for(j = 0; j < 0x10000; j++)
    {
        UINT32 row = 0;
        i = ((j >> 8) & 0xFF) | ((j  & 0xFF) <<8);

        if(i & 0x8000) row |= 0x20000000;
        if(i & 0x4000) row |= 0x02000000;
        if(i & 0x2000) row |= 0x00200000;
        if(i & 0x1000) row |= 0x00020000;
        if(i & 0x0800) row |= 0x00002000;
        if(i & 0x0400) row |= 0x00000200;
        if(i & 0x0200) row |= 0x00000020;
        if(i & 0x0100) row |= 0x00000002;
        if(i & 0x0080) row |= 0x10000000;
        if(i & 0x0040) row |= 0x01000000;
        if(i & 0x0020) row |= 0x00100000;
        if(i & 0x0010) row |= 0x00010000;
        if(i & 0x0008) row |= 0x00001000;
        if(i & 0x0004) row |= 0x00000100;
        if(i & 0x0002) row |= 0x00000010;
        if(i & 0x0001) row |= 0x00000001;
        bp_lut[j] = row;
    }
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void make_cram_lut(void)
{
    for(unsigned int j = 0; j < 0x40; j++)
    {
        int r = (j >> 0) & 3;
        int g = (j >> 2) & 3;
        int b = (j >> 4) & 3;
        r  = (r << 3) | (r << 1) | (r >> 1);
        g = (g << 3) | (g << 1) | (g >> 1);
        b = (b << 3) | (b << 1) | (b >> 1);
        cram_lut[j] =RGB(r,g,b);
    }
}
//-------------------------------------------------------------------------------------------------------------------------------------
static void make_lut()
{
	make_name_lut();
	make_bp_lut();
	make_cram_lut();
	make_map_lut();
}
//-------------------------------------------------------------------------------------------------------------------------------------

/*
 ----------------------------------------------------------------------------
 Sega System E Hardware Notes
 (C) 2010 Charles MacDonald
 ----------------------------------------------------------------------------

 Unpublished work Copyright 2010 Charles MacDonald

 What's new

 [05/27/2010]
 - Added description of bank latch timing quirks.
 - Added DIP switches.
 - Corrected VRAM banking behavior.
 - Added note about floating Z80 data bus.
 - Added display order list.
 - Added I/O port diagram.
 - Added main board components list.
 - Added miscellaneous notes.

 [05/26/2010]
 - Initial release.

 ----------------------------------------------------------------------------
 Introduction
 ----------------------------------------------------------------------------

 The "System E" board is derived from the Sega Master System game console.
 It has twice as much work RAM, two VDPs, and twice the video RAM for each
 one amongst other changes.

 The board is not JAMMA compatible and uses headers instead of edge
 connectors for interfacing.

 The board has no name designation so I am not sure what the origin of
 the "System E" name comes from.

 Some games are encrypted and use a custom Z80 IC or come in an epoxy
 module with a battery (e.g. MC-8123).

 ----------------------------------------------------------------------------
 System overview
 ----------------------------------------------------------------------------

 Timing

 XTAL is a 10.7386 MHz crystal used as follows:

 VDP #1 XTAL1 input = 10.7368 MHz (XTAL)
 VDP #2 XTAL1 input = 10.7368 MHz (XTAL)
 Z80 /CLK input     = 5.36 MHz    (XTAL/2)

 Z80 environment

 The Z80 has 16K of work RAM at C000-FFFF. VDP VRAM is write-only and
 mapped to 8000-BFFF. The rest of the memory map is defined by the ROM board
 which is described later, and the ROM board can map read-only ROM over
 the 8000-BFFF area without conflicts.

 The Z80 interrupt sources are:

 INT - VDP #1 /INT output
 NMI - VDP #2 /INT output

 Because NMI is edge sensitive and not level sensitive, a pending VDP #2
 interrupt will not continue to cause interrupts until it acknowledged.

 The Z80 NMI pin is not debounced by the debouncing circuit in either VDP
 so there is no delay between the time a VDP #2 interrupt occurs and Z80
 /NMI is asserted.

 There are no wait states used by this system, the Z80 runs at full speed
 all the time.

 Audio

 The analog PSG output of both VDPs are mixed together with equal weighting,
 attenuated by a single volume potentiometer, then amplified and output to
 an external 4 or 8-ohm speaker.

 I/O

 The Z80 is connected to three read-only input ports, two read-only
 DIP switches, an 8255 PPI which is used to control two programmable I/O
 ports on an expansion connector, four ULN2003 outputs suitable for driving
 coin meters and coin chute lockout coils, and four more TTL outputs.

 ----------------------------------------------------------------------------
 Reset behavior
 ----------------------------------------------------------------------------

 A reset pulse is generated at power-up that goes to the following devices

 - 8255 PPI
 - Z80
 - Bank latch (IC54)

 The two VDPs have a longer reset period which lasts exactly eight Z80
 cycles from the time the Z80 starts running. Because VDP #2 also generates
 the chip select for the 8255 PPI, it is advisable to not access any
 VDP ports (PSG, HV counters, control/data) or PPI registers during this
 time.

 When the bank latch is reset the following settings are made:

 - VRAM bank 1 of 2 is selected for VDP #2.
 - VRAM bank 1 of 2 is selected for VDP #1.
 - VDP #1 VRAM bank 1 is mapped to $8000-$BFFF for write access.
 - The speaker amplifier output is enabled.
 - The ROM bank bits are all set to zero.

 A different reset pulse is generated to forcibly mute the speaker
 amplifier at start-up to prevent popping noises from occuring.
 During this time the state of bit 4 of the bank latch is ignored, despite
 the reset state of the bank latch enabling it.

 ----------------------------------------------------------------------------
 Video overview
 ----------------------------------------------------------------------------

 Each VDP is connected to 32K of VRAM which is split into two 16K banks.
 The banks are set up so the VDP can generate the display from one bank while
 the Z80 has full speed write-only access to the other one.

 VDP #1 (IC35) is the main Z80. It has a transparent pixel indicator pin (Y1)
 which is used select the VDP #2 (IC33) RGB output whenever a VDP #1 pixel
 is blank.

 The display layer priority is:

 (Frontmost layer)

 1. VDP #1 tilemap (high priority)
 2. VDP #1 sprites
 3. VDP #1 tilemap (low priority)
 4. VDP #2 tilemap (high priority)
 5. VDP #2 sprites
 6. VDP #2 tilemap (low priority)
 7. VDP #2 backdrop color (register $07)

 (Backmost layer)

 Note that the VDP #2 sprites are always behind the VDP #1 tilemap layer
 which limits their usefulness.

 ----------------------------------------------------------------------------
 ROM boards
 ----------------------------------------------------------------------------

 Type: 171-5234

 Part list

 IC1 - Z80-B CPU
 IC2 - 27256 or 27512 EPROM
 IC3 - 27256 or 27512 EPROM
 IC4 - 27256 or 27512 EPROM
 IC5 - 27256 or 27512 EPROM
 IC6 - 74LS139
 IC7 - 27256 EPROM only

 Z80 memory map

 0000-7FFF : IC7
 8000-BFFF : Bank area, see jumper description for details.
 C000-FFFF : Work RAM (16K)

 Notes

 Z80 /WAIT is accessible at a test point near R1.

 Jumpers

 Only one in each pair of J1,J2 and J3,J4 can be shorted.

 EPROM type setting:

 - 27512 (J1 open, J2 closed)
 - 27256 (J1 closed, J2 open)

 EPROM capacity setting:

 - Two EPROMs  (J3 open, J4 closed)
 - Four EPROMs (J3 closed, J4 open)

 Here is a complete description of all bank latch settings for each
 jumper setting. This board does not use bank latch bit 3 for anything
 so it is omitted.

 27256, two EPROM mode

 D2 D1 D0   
  0  0  0 : IC5, offset 0000-3FFF
  0  0  1 : IC5, offset 4000-7FFF
  0  1  0 : IC5, offset 0000-3FFF (mirror)
  0  1  1 : IC5, offset 4000-7FFF (mirror)
  1  0  0 : IC3, offset 0000-3FFF
  1  0  1 : IC3, offset 4000-7FFF
  1  1  0 : IC3, offset 0000-3FFF (mirror)
  1  1  1 : IC3, offset 4000-7FFF (mirror)

 27512, two EPROM mode

 D2 D1 D0 
  0  0  0 : IC5, offset 0000-3FFF
  0  0  1 : IC5, offset 4000-7FFF
  0  1  0 : IC5, offset 8000-BFFF
  0  1  1 : IC5, offset C000-FFFF
  1  0  0 : IC3, offset 0000-3FFF
  1  0  1 : IC3, offset 4000-7FFF
  1  1  0 : IC3, offset 8000-BFFF
  1  1  1 : IC3, offset C000-FFFF

 27256, four ROM mode

 D2 D1 D0
  0  0  0 : IC5, offset 0000-3FFF
  0  0  1 : IC5, offset 4000-7FFF
  0  1  0 : IC4, offset 0000-3FFF
  0  1  1 : IC4, offset 4000-7FFF
  1  0  0 : IC3, offset 0000-3FFF
  1  0  1 : IC3, offset 4000-7FFF
  1  1  0 : IC2, offset 0000-3FFF
  1  1  1 : IC2, offset 4000-7FFF

 27512, four ROM mode

 D2 D1 D0
  0  0  0 : IC5, offset 0000-3FFF
  0  0  1 : IC5, offset 4000-7FFF
  0  1  0 : IC4, offset 8000-BFFF
  0  1  1 : IC4, offset C000-FFFF
  1  0  0 : IC3, offset 0000-3FFF
  1  0  1 : IC3, offset 4000-7FFF
  1  1  0 : IC2, offset 8000-BFFF
  1  1  1 : IC2, offset C000-FFFF

 Note that in this setting not all the ROM data is accessible. While
 I have not seen a 171-5234 board modified this way, Guru's System E
 notes describe a modification where a wire link is added to connect
 CN2 B22 (bank latch, bit 3) to what is described as the upper part
 of J3, but I think it should be the lower. This would give the
 following mapping:

 D3 D2 D1 D0
  0  0  0  0 : IC5, offset 0000-3FFF
  0  0  0  1 : IC5, offset 4000-7FFF
  0  0  1  0 : IC5, offset 8000-BFFF
  0  0  1  1 : IC5, offset C000-FFFF
  0  1  0  0 : IC3, offset 0000-3FFF
  0  1  0  1 : IC3, offset 4000-7FFF
  0  1  1  0 : IC3, offset 8000-BFFF
  0  1  1  1 : IC3, offset C000-FFFF
  1  0  0  0 : IC4, offset 0000-3FFF
  1  0  0  1 : IC4, offset 4000-7FFF
  1  0  1  0 : IC4, offset 8000-BFFF
  1  0  1  1 : IC4, offset C000-FFFF
  1  1  0  0 : IC2, offset 0000-3FFF
  1  1  0  1 : IC2, offset 4000-7FFF
  1  1  1  0 : IC2, offset 8000-BFFF
  1  1  1  1 : IC2, offset C000-FFFF

 In this configuration all ROM data can be accessed with bits 3,2
 selecting the ROM and bits 1,0 selecting the offset. It seems reasonable.

 Fantasy Zone 2 is the only game with four 27C512 EPROMs and may use
 this arrangement, but it has not been confirmed. Maybe it uses a different
 ROM board that properly supports this EPROM capacity.

 ----------------------------------------------------------------------------
 I/O ports
 ----------------------------------------------------------------------------

 There is a lot of mirroring of the I/O ports so the port address will be
 shown bit by bit. A dash means that bit can be set to any value.

 A list of the most commonly used port address for a certain device will
 be listed where applicable, but that is only one of many other possible
 mirrored locations.

 A7     A0   Addr Device          Read            Write          
 =========   ==== =============== =============== ===============
 ---0 0000 : E0   Input ports     Input port #0   Contention     
 ---0 0001 : E1      "  "         Input port #1   Contention     
 ---0 0010 : E2      "  "         Input port #2   Contention     
 ---0 11-- :      Expansion area  ( Dependent on the ROM board )
 ---1 00-0 : F2   DIP switches    DSW #1          Contention     
 ---1 00-1 : F3      "  "         DSW #2          Contention     
 ---1 01-- : F7   Bank latch      Load latch      Load latch     
 01-1 10-0 : 7A   VDP #2 (IC33)   V-counter       PSG            
 01-1 10-1 : 7B      "  "         H-counter       PSG            
 10-1 10-0 : BA      "  "         Data port       Data port      
 10-1 10-1 : BB      "  "         Control port    Control port   
 11-1 1000 : F8   8255 PPI        Port A          Port A         
 11-1 1001 : F9      "  "         Port B          Port B         
 11-1 1010 : FA      "  "         Port C          Port C         
 11-1 1011 : FB      "  "         Ctrl. reg.      Ctrl. reg.     
 01-1 11-0 : 7E   VDP #1 (IC35)   V-Counter       PSG            
 01-1 11-1 : 7F      "  "         H-Counter       PSG            
 10-1 11-0 : BE      "  "         Data port       Data port      
 10-1 11-1 : BF      "  "         Control port    Control port

 Contention means there will be a data bus conflict when you write to
 a read-only location. Well behaved software will not do this.

 The PPI control register may not be readable, this depends on the
 particular type of 8255 being used.

 For all port addresses not listed there is either no device to respond
 to a read or write, or the device mapped there will not respond.

 There is a 4.7K ohm pull-up resistor network on D0-D7 so all undriven I/O
 ports should return $FF when read, though due to bus capacitance and
 other conditions it may not always be $FF exactly.

 If you read one of these locations the data returned is indeterminate.
 For the same reason reading the bank latch (which is write only) will
 load the latch with this semi-random data. So only write to the latch.

 The expansion area is to map additional hardware on the ROM board to
 an I/O port range. I don't think it is ever used.

 I/O port diagram

 To visualize how the I/O port space is split up, consider this diagram:

 00: III-xxxxxxxxxxxx       Key:
 10: ddddllll--------
 20: III-xxxxxxxxxxxx       I = Input ports 0,1,2
 30: ddddllll--------       x = Expansion area /CS asserted
 40: III-xxxxxxxxxxxx       d = DIP switches 0,1
 50: ddddllllmmmmMMMM       l = Bank latch
 60: III-xxxxxxxxxxxx       m = VDP #2 PSG and H/V counters
 70: ddddllllmmmmMMMM       M = VDP #1 PSG and H/V counters
 80: III-xxxxxxxxxxxx       v = VDP #2 control/data port
 90: ddddllllvvvvVVVV       V = VDP #1 control/data port
 A0: III-xxxxxxxxxxxx       k = 8255 PPI
 B0: ddddllllvvvvVVVV       - = Unused (free for use)
 C0: III-xxxxxxxxxxxx
 D0: ddddllllkkkk----
 E0: III-xxxxxxxxxxxx
 F0: ddddllllkkkk----

 ----------------------------------------------------------------------------
 Input ports
 ----------------------------------------------------------------------------

 Each port inputs the state of eight optocoupled switches which are
 active-low.

 Input port #0

 D7 : CN2 A05
 D6 : CN2 B04
 D5 : CN2 A04
 D4 : CN2 B03
 D3 : CN2 A03
 D2 : CN2 B02
 D1 : CN2 A02
 D0 : CN2 B01
            
 Input port #1

 D7 : CN2 A09
 D6 : CN2 B08
 D5 : CN2 A08
 D4 : CN2 B07
 D3 : CN2 A07
 D2 : CN2 B06
 D1 : CN2 A06
 D0 : CN2 B05
            
 Input port #2

 D7 : CN2 A13
 D6 : CN2 B12
 D5 : CN2 A12
 D4 : CN2 B11
 D3 : CN2 A11
 D2 : CN2 B10
 D1 : CN2 A10
 D0 : CN2 B09
            
 ----------------------------------------------------------------------------
 DIP switches
 ----------------------------------------------------------------------------

 Switch state is 1=off, 0=on

 DIP switch #1

 D7 : SW1.8
 D6 : SW1.7
 D5 : SW1.6
 D4 : SW1.5
 D3 : SW1.4
 D2 : SW1.3
 D1 : SW1.2
 D0 : SW1.1

 DIP switch #2

 D7 : SW2.8
 D6 : SW2.7
 D5 : SW2.6
 D4 : SW2.5
 D3 : SW2.4
 D2 : SW2.3
 D1 : SW2.2
 D0 : SW2.1

 ----------------------------------------------------------------------------
 Bank latch
 ----------------------------------------------------------------------------

 D7 : Select VDP #2 VRAM bank.

      1= VDP #2 uses VRAM bank #1, CPU can access bank #0 at 8000-BFFF.
      0= VDP #2 uses VRAM bank #0, CPU can access bank #1 at 8000-BFFF.

 D6 : Select VDP #1 VRAM bank.

      1= VDP #1 uses VRAM bank #1, CPU can access bank #0 at 8000-BFFF.
      0= VDP #1 uses VRAM bank #0, CPU can access bank #1 at 8000-BFFF.

 D5 : Select VDP VRAM for CPU access.

      1= Writes to 8000-BFFF go to currently selected bank of VDP #2.
      0= Writes to 8000-BFFF go to currently selected bank of VDP #1.

 D4 : Speaker amplifier control (0= output enabled, 1= output muted)
 D3 : ROM bank control (to CN2 B22)
 D2 : ROM bank control (to CN2 A22)
 D1 : ROM bank control (to CN2 B21)
 D0 : ROM bank control (to CN2 A21)

 VRAM access quirks

 The state of bits 7 and 6 are sampled by the video hardware after each
 VRAM access the VDP makes:

 - VDP #2 VRAM access causes bit 7 to be sampled.
 - VDP #1 VRAM access causes bit 6 to be sampled.

 This prevents glitches that would happen if banks were changed in the
 middle of an VRAM access. If these bits are changed in parallel with a VDP
 VRAM access (either due to display refresh or port based I/O) the old state
 remains in use regardless of the new latch state until the current VRAM
 access finishes.

 It is not known how long a VDP VRAM access can last, but as the Z80 and
 VDP are clocked at the same speed it would make sense to wait a few cycles
 (such as a NOP) after writing to this latch and expecting the new VRAM
 banks to be accessible.

 Because bits 7,6 are only sampled after a VDP VRAM access, there is a
 significant problem when these bits are changed when the VDP is idle such
 as during V-Blank. In this case the state of bits 7,6 will be ignored.
 The programmer can force the new state to be recognized by doing any kind
 of VRAM I/O such as:

 - Writing a byte to the data port if the command set is VRAM write.

 - Read a byte from the data port if the command set is VRAM read.

 - Issue a VRAM read command which forces a pre-load of the read buffer
   for subsequent reads.

 You have to do this for the VDP corresponding to the bit you changed, or
 BOTH VDPs if both bits changed. And note that the bank accessed in any
 of these cases is the previously selected bank, NOT the current bank
 you just set in bits 7,6.

 Simply changing bits 7,6 during a period of idle operation will do nothing,
 and the state of bits 7,6 as latched after the last VRAM access will
 continue to persist.

 Idle time may also include any scanlines that are forcibly blanked by
 turning the display off mid-frame.

 Bit 5 can be changed at any time, but the currently selected bank is
 subject to the processing delay of bits 7 and 6 as described above.

 ----------------------------------------------------------------------------
 8255 PPI
 ----------------------------------------------------------------------------

 PA7 : CN3 B01
 PA6 : CN3 B02
 PA5 : CN3 B03
 PA4 : CN3 B04
 PA3 : CN3 B05
 PA2 : CN3 B06
 PA1 : CN3 B07
 PA0 : CN3 B08

 PB7 : CN1 B17
 PB6 : CN1 B18
 PB5 : CN1 B19
 PB4 : CN1 B20
 PB3 : CN1 B22
 PB2 : CN1 B23
 PB1 : CN1 B24
 PB0 : CN1 B25

 PC7 : CN3 B09
 PC6 : CN3 B10
 PC5 : CN3 B11
 PC4 : CN3 B12
 PC3 : CN3 B13
 PC2 : CN3 B14
 PC1 : CN3 B15
 PC0 : CN3 B16

 Port B is output-only.

 Ports A and C can be freely defined as inputs or outputs, there is no
 hardware limiting their function other than 10K ohm pull-up resistors
 to +5V on the pins.

 I have not seen the type of board that plugs into this connector, but
 apparently it is for analog inputs.

 Connector pin assignments

 ----------------------------------------------------------------------------
 CN1 - 2x25 pin connector with latch
 ----------------------------------------------------------------------------

 This connector has audio, video, and I/O.

                        +---------+
   Common pull-up point |A01   B01| Input port #0, bit 0
   Input port #0, bit 1 |A02   B02| Input port #0, bit 2
   Input port #0, bit 3 |A03   B03| Input port #0, bit 4
   Input port #0, bit 5 |A04   B04| Input port #0, bit 6
   Input port #0, bit 7 |A05   B05| Input port #1, bit 0
   Input port #1, bit 1 |A06   B06| Input port #1, bit 2
   Input port #1, bit 3 |A07   B07| Input port #1, bit 4
   Input port #1, bit 5 |A08   B08| Input port #1, bit 6
   Input port #1, bit 7 |A09   B09| Input port #2, bit 0
   Input port #2, bit 1 |A10   B10| Input port #2, bit 2
   Input port #2, bit 3 |A11   B11| Input port #2, bit 4
   Input port #2, bit 5 |A12   B12| Input port #2, bit 6
   Input port #2, bit 7 |A13   B13| Analog red video output
                 (N.C.) |A14   B14| Analog green video output
                 (N.C.) |A15   B15| Analog blue video output
                 (N.C.) |A16   B16| Composite sync output (TTL)
                 (N.C.) |A17   B17| 8255 PB7 (output)
                 (N.C.) |A18   B18| 8255 PB6 (output)
            Speaker (+) |A19   B19| 8255 PB5 (output)
            Speaker (-) |A20   B20| 8255 PB4 (output)
            ULN2003 GND |A21   B21| ULN2003 GND
                    GND |A22   B22| 8255 PB0 (output)
                    GND |A23   B23| 8255 PB1 (output)
                    GND |A24   B24| 8255 PB2 (output)
                    GND |A25   B25| 8255 PB3 (output)
                        +---------+

 A01 = This is common to pin 1 of all three 1K resistor arrays that pull up
       the input port bits. It should be connected to a +12V source.

 Pins B17-B20 are TTL outputs driven directly by the 8255.

 Pins B22-B25 are open collector outputs driven by a ULN2003. Pins B22 and
 B23 each use two darlington transistors paired together.

 ----------------------------------------------------------------------------
 CN2 - 2x25 pin connector
 ----------------------------------------------------------------------------

 This connector is for the ROM board.
    
                            +---------+
                        +5V |A01   B01| +5V
                        +5V |A02   B02| +5V
                   Z80 /CLK |A03   B03| Z80 /M1
                   Z80 /RST |A04   B04| Z80 /NMI (VDP #2 IRQ)
     (VDP #1 IRQ)  Z80 /INT |A05   B05| Z80 /RFSH
                    Z80 A15 |A06   B06| Z80 A14
                    Z80 A13 |A07   B07| Z80 A12
                    Z80 A11 |A08   B08| Z80 A10
                     Z80 A9 |A09   B09| Z80 A8
                     Z80 A7 |A10   B10| Z80 A6
                     Z80 A5 |A11   B11| Z80 A4
                     Z80 A3 |A12   B12| Z80 A2
                     Z80 A1 |A13   B13| Z80 A0
                     Z80 D7 |A14   B14| Z80 D6
                     Z80 D5 |A15   B15| Z80 D4
                     Z80 D3 |A16   B16| Z80 D2
                     Z80 D1 |A17   B17| Z80 D0
                    Z80 /RD |A18   B18| Z80 /WR
                  Z80 /MREQ |A19   B19| Z80 /IORQ
   (8000-BFFF) VDP #1 EXM1# |A20   B20| VDP #1 EXM2# (0000-7FFF)
                   LATCH D0 |A21   B21| LATCH D1
                   LATCH D2 |A22   B22| LATCH D3
          I/O EXPANSION CS# |A23   B23| (N.C.)
                        GND |A24   B24| GND
                        GND |A25   B25| GND
                            +---------+

 The design of the system is Z80-centric so other processors cannot be
 used on the ROM board without some difficulty.

 ----------------------------------------------------------------------------
 CN3 - 2x17 pin connector
 ----------------------------------------------------------------------------

 This connector is for additional I/O.

                        +---------+
                    GND |A01   B01| PA7
                    GND |A02   B02| PA6
                    GND |A03   B03| PA5
                    GND |A04   B04| PA4
                    GND |A05   B05| PA3
                    GND |A06   B06| PA2
                    GND |A07   B07| PA1
                    GND |A08   B08| PA0 
                    GND |A09   B09| PC7 
                    GND |A10   B10| PC6 
                    GND |A11   B11| PC5 
                    GND |A12   B12| PC4 
                    GND |A13   B13| PC3 
                    GND |A14   B14| PC2 
                    GND |A15   B15| PC1 
                    GND |A16   B16| PC0 
                    +5V |A17   B17| +5V 
                        +---------+

 Ports A and C have 10K ohm pull resistors to +5V.

 ----------------------------------------------------------------------------
 CN4 - 2x8 pin connector with latch
 ----------------------------------------------------------------------------

 This connector is for the power supply.

                        +---------+
                    +5V |A01   B01| +5V
                    +5V |A02   B02| +5V
                    GND |A03   B03| GND
                    GND |A04   B04| GND 
                    GND |A05   B05| GND
                        |A06   B06| 
                   +12V |A07   B07| +12V
                        |A08   B08| 
                        +---------+

 Pins A06 and B06 are connected together but don't go anywhere on the board.
 This is also true of A08 and B08.

 ----------------------------------------------------------------------------
 Miscellaneous
 ----------------------------------------------------------------------------

 VDP #1 inputs Z80 MREQ#, A15, A14 and generates three chip selects:

 EXM1#  = !MREQ &  A15 & !A14 (8000-BFFF)
 EXM2#  = !MREQ & !A15        (0000-7FFF)
 CSRAM# = !MREQ &  A15 &  A14 (C000-FFFF)

 The work RAM is connected as follows:

 RFSH# = !MREQ & !RFSH (Asserted during Z80 refresh cycles)
   CE# = CSRAM#
   OE# = Buffered Z80 RD#
   WE# = Buffered Z80 WR#

 Memory mapped VRAM is disabled during Z80 read and refresh cycles to
 8000-BFFF so there is no conflict with the banked ROM which the ROM board
 can map to that area.

 The I/O port decoding is enabled when M1# is high such that I/O ports
 do not respond to Z80 interrupt acknowledge cycles.

 VDP #2 KBSEL# is used to generate the chip select for the 8255 PPI.

 The T1 pin of both VDP is grounded. Because they use an external clock
 signal at XTAL1 instead of an oscillator, the XTAL2 pin is left floating.

 ----------------------------------------------------------------------------
 Component list
 ----------------------------------------------------------------------------

 Main board is 171-5323

  IC1 - NEC uPD4168C-15             8Kx8 NMOS XRAM
  IC2 - NEC uPD4168C-15         
  IC4 - NEC uPD4168C-15
  IC5 - NEC uPD4168C-15
  IC6 - NEC uPD4168C-15
  IC7 - NEC uPD4168C-15
  IC8 - NEC uPD4168C-15
  IC9 - 74LS257
 IC10 - 74LS244
 IC11 - 74LS244
 IC12 - 74LS244
 IC13 - 74LS244
 IC14 - 74LS257
 IC15 - 74LS244
 IC16 - 74LS244
 IC17 - 74LS244
 IC18 - 74LS244
 IC19 - 74LS257
 IC20 - 74LS245
 IC21 - 74LS245
 IC22 - 74LS245
 IC23 - 74LS245
 IC24 - 74LS257
 IC25 - 74LS245
 IC26 - 74LS245
 IC27 - 74LS245
 IC28 - 74LS245
 IC29 - NEC uPA2003C                Open collector Darlington transistor array
 IC30 - Texas Instruments 74145     Open collector 1-of-10 decoder
 IC31 - 74LS257
 IC32 - 74LS257
 IC33 - SEGA 315-5124               Sega Master System VDP (64-pin SDIP)
 IC34 - 74LS257
 IC35 - SEGA 315-5124
 IC36 - No markings (8255 PPI)
 IC37 - NEC 74HC4066C               Quad bilateral switch
 IC38 - NEC 74HC4066C
 IC39 - 74LS257
 IC40 - 74LS74
 IC41 - 74LS00 
 IC42 - 74LS123
 IC43 - 74HC74
 IC44 - 74LS257
 IC45 - 74LS257
 IC46 - 74LS244
 IC47 - 74LS32
 IC48 - 74LS244
 IC49 - NEC uPD4168C-15
 IC50 - 74HC04
 IC51 - 74LS257
 IC52 - 74LS257
 IC53 - 74LS245
 IC54 - 74LS273
 IC55 - NEC uPD4168C-15
 IC56 - 74LS163A
 IC57 - 74LS139
 IC58 - 74LS138
 IC59 - AN608                       Video amplifier (TO-220, 4 pins)
 IC60 - AN608
 IC61 - AN608
 IC62 - LA4460                      Speaker amplifier (SIP)

  PC1 - TLP521-4                    Four channel optocoupler
  PC2 - TLP521-4
  PC3 - TLP521-4
  PC4 - TLP521-4
  PC5 - TLP521-4
  PC6 - TLP521-4

  TR1 - C945                        NPN transistor
  TR2 - C945
  TR3 - C945

 XTAL - 10.7386 MHz                 Oscillator, 2 pins

 ----------------------------------------------------------------------------
 Programming notes
 ----------------------------------------------------------------------------

 I/O ports

 E0 : Input port #0     (r/o)
 E1 : Input port #1     (r/o)
 E2 : Input port #2     (r/o)
 F2 : DIP switch #1     (r/o)
 F3 : DIP switch #2     (r/o)
 F7 : Bank latch        (w/o)
 F8 : PPI port A        (r/w)
 F9 : PPI port B        (r/w)
 FA : PPI port C        (r/w)
 FB : PPI control reg.  (r/w)

 7A : VDP #2 V-counter  ( /r)
 7A : VDP #2 PSG        (w/ )
 7B : VDP #2 H-counter  ( /r)
 7B : VDP #2 PSG        (w/ )
 BA : VDP #2 data port  (r/w)
 BB : VDP #2 ctrl port  (r/w)

 7E : VDP #1 V-counter  ( /r)
 7E : VDP #1 PSG        (w/ )
 7F : VDP #1 H-counter  ( /r)
 7F : VDP #1 PSG        (w/ )
 BE : VDP #1 data port  (r/w)
 BF : VDP #1 ctrl port  (r/w)

 Memory map

 0000-7FFF : ROM (IC6)  (r/o)
 8000-BFFF : Banked ROM (r/ )
 8000-BFFF : VRAM bank  ( /w)
 C000-FFFF : Work RAM   (r/w)

 Interrupts

 INT - VDP #1 frame and line interrupts. Read port $BF to acknowledge
       otherwise an INT will be generated after every instruction.

 NMI - VDP #2 frame and line interrupts. Read port $BB to acknowledge
       otherwise no further NMIs will be generated.

 Timing

 - If you change the bank latch during V-Blank, do some dummy VRAM I/O
   from port VDP #1 if you changed bit 6, VDP #2 if you changed bit 7,
   or both if you changed bits 7 and 6. Note that the I/O in question
   will go the previously selected bank so do something non-destructive
   such as issue a read command and discard the data.

 - Don't touch any VDP 1/2 or PPI registers for the eight cycles after
   reset.

 ----------------------------------------------------------------------------
 Credits and Acknowledgements
 ----------------------------------------------------------------------------

 - Omar Cornut for the System E board.
 - Guru for his System E readme.

 ----------------------------------------------------------------------------
 End
 ----------------------------------------------------------------------------
*/