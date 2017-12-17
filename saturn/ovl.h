#ifndef _OVL_H_
#define _OVL_H_

//extern Uint32   _bstart, _bend;
extern Uint32 *shared;
//extern struct BurnDriver* pDriver[];
//static int nBurnSoundLen;
//static Sint8 *nSoundBuffer = (Sint8 *)0x25a20000;
extern unsigned int  nSoundBufferPos;
/*static*/ SclNorscl *ss_reg = NULL;
/*static*/ SclSysreg	*ss_regs = NULL;
/*static*/ SclDataset	*ss_regd = NULL;
/*static*/ SclSpPriNumRegister  *ss_SpPriNum = NULL;
/*static*/ SclBgPriNumRegister  *ss_BgPriNum = NULL;
/*static*/ SclOtherPriRegister    *ss_OtherPri = NULL;
/*static*/ SclBgColMixRegister  *ss_BgColMix = NULL;
/*static*/ SprSpCmd				*ss_sprite = NULL;
//static struct BurnDriver *fba_drv;

/*static*/ UINT16  *colAddr = NULL; 
/*static*/ UINT16  *colBgAddr = NULL; // color
/*static*/ UINT16  *colBgAddr2 = NULL;
/*static*/ Uint16  *ss_font = NULL;
/*static*/ Uint16  *ss_map = NULL;
/*static*/ Uint16  *ss_map2 = NULL;
/*static*/ Fixed32 *ss_scl = NULL;
/*static*/ //Fixed32 *ss_scl1 = NULL;

static unsigned char *cache = NULL;

//#define	REGADDR	    0x25F80000
#define TVMD        (*(Uint16 *)0x25F80000)
#define TVSTAT      (*(volatile Uint16 *)0x25F80004)
#define TVOFF   TVMD &= 0x7fff
#define TVON     TVMD |= 0x8000

#define FBA_DRV	 shared
#define SS_MAP	 *(&shared + 1)
#define SS_MAP2	 *(&shared + 2)
#define SS_FONT	 *(&shared + 3)
#define SS_CACHE *(&shared + 4)
#define SS_SPRAM *(&shared + 5)
#define SS_REG *(&shared + 6)
#define SS_REGD *(&shared + 7)
#define SS_REGS *(&shared + 8)
#define SS_N0PRI *(&shared + 9)
#define SS_SPPRI *(&shared + 10)
#define SS_SPRIT *(&shared + 11)
#define SS_OTHR *(&shared + 12)
#define SS_BGMIX *(&shared + 13)
#define SS_SCL *(&shared + 14)
//#define SS_SCL1 *(&shared + 15)
#define SS_Z80CY *(&shared + 15)
#define SS_PORT *(&shared + 16)
//#define SS_Z80CY (unsigned int*)0x00200000
#endif

#define SS_SET_N0PRIN(/* 3 bits */ n0prin) \
  (ss_BgPriNum->PriorityNumberNBG01 \
   = (ss_BgPriNum->PriorityNumberNBG01 & 0xFF00) | ((n0prin)))
#define SS_SET_N1PRIN(/* 3 bits */ n1prin) \
  (ss_BgPriNum->PriorityNumberNBG01 \
   = (ss_BgPriNum->PriorityNumberNBG01 & 0x00FF) | ((n1prin) << 8))
#define SS_SET_N2PRIN(/* 3 bits */ n2prin) \
  (ss_BgPriNum->PriorityNumberNBG23 \
   = (ss_BgPriNum->PriorityNumberNBG23 & 0xFF00) | ((n2prin)))
#define SS_SET_S0PRIN(/* 3 bits */ s0prin) \
  (ss_SpPriNum->PriorityNumberSP01 \
   = (ss_SpPriNum->PriorityNumberSP01 & 0xFFF0) | (s0prin))
#define SS_SET_SPCLMD(/* 1 bit */ spclmd) \
   (ss_OtherPri->SpriteControl \
   = (ss_OtherPri->SpriteControl & 0xFFDF) | ((spclmd) << 5))
#define SS_SET_N0SPRM(/* 2bits */ n0sprm) \
  (ss_OtherPri->SpecialPriorityMode \
   = (ss_OtherPri->SpecialPriorityMode & 0xFFFC) | ((n0sprm)))
#define SS_SET_N0SCCM(/* 2 bits */ n0sccm) \
  ( ss_OtherPri->SpecialColorMixMode \
   = (ss_OtherPri->SpecialColorMixMode & 0xFFFC) | ((n0sccm)))
#define SS_SET_N0CCRT(/* 5 bits */ n0ccrt) \
  ( ss_BgColMix->ColMixRateNBG01 \
   = (ss_BgColMix->ColMixRateNBG01 & 0xFF00) | ((n0ccrt)))
#define SS_SET_N0LCEN(/* 1bit */ n0lcen) \
  ( ss_OtherPri->LineColorEnable \
   = (ss_OtherPri->LineColorEnable & 0xFFFE) | ((n0lcen)))
#define SS_SET_CCMD(/* 1 bit */ ccmd) \
  (ss_OtherPri->ColorMixControl \
   = (ss_OtherPri->ColorMixControl & 0xFEFF) | ((Uint32)(ccmd) << 8))  
#define SS_SET_N0CCEN(/* 1 bit */ n0ccen) \
  (ss_OtherPri->ColorMixControl \
   = (ss_OtherPri->ColorMixControl & 0xFFFE) | ((n0ccen)))
#define SS_SET_CCRTMD(/* 1 bit */ ccrtmd) \
  (ss_OtherPri->ColorMixControl \
   = (ss_OtherPri->ColorMixControl & 0xFDFF) | ((Uint32)(ccrtmd) << 9))
#define SS_SET_N2SPRM(/* 2bits */ n2sprm) \
  (ss_OtherPri->SpecialPriorityMode \
   = (ss_OtherPri->SpecialPriorityMode & 0xFFCF) | ((n2sprm) << 4))
#define SS_SET_N2CCEN(/* 1 bit */ n2ccen) \
  (ss_OtherPri->ColorMixControl \
   = (ss_OtherPri->ColorMixControl & 0xFFFB) | ((n2ccen) << 2))
#define SS_SET_N2SCCM(/* 2 bits */ n2sccm) \
  (ss_OtherPri->SpecialColorMixMode \
   = (ss_OtherPri->SpecialColorMixMode & 0xFFCF) | ((n2sccm) << 4))
#define SS_SET_N2CCRT(/* 5 bits */ n2ccrt) \
  (ss_BgColMix->ColMixRateNBG23 \
   = (ss_BgColMix->ColMixRateNBG23 & 0xFF00) | ((n2ccrt)))
#define SS_SET_N1SPRM(/* 2bits */ n1sprm) \
  (ss_OtherPri->SpecialPriorityMode \
   = (ss_OtherPri->SpecialPriorityMode & 0xFFF3) | ((n1sprm) << 2))
