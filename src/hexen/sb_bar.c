//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//


// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "i_cdmus.h"
#include "i_video.h"
#include "m_bbox.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"
#include "i_swap.h"
#include "am_map.h"
#include "a11y.h" // [crispy] A11Y
#include "v_trans.h" // [crispy] dp_translation


// TYPES -------------------------------------------------------------------

typedef struct Cheat_s
{
    void (*func) (player_t * player, struct Cheat_s * cheat);
    cheatseq_t *seq;
} Cheat_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void DrawSoundInfo(void);
static void DrINumber(signed int val, int x, int y);
static void DrRedINumber(signed int val, int x, int y);
static void DrBNumber(signed int val, int x, int y);
static void DrawCommonBar(void);
static void DrawMainBar(void);
static void DrawInventoryBar(void);
static void DrawKeyBar(void);
static void DrawWeaponPieces(void);
static void DrawFullScreenStuff(void);
static void DrawAnimatedIcons(void);
static boolean HandleCheats(byte key);
static boolean CheatAddKey(Cheat_t * cheat, byte key, boolean * eat);
static void CheatGodFunc(player_t * player, Cheat_t * cheat);
static void CheatNoClipFunc(player_t * player, Cheat_t * cheat);
static void CheatWeaponsFunc(player_t * player, Cheat_t * cheat);
static void CheatHealthFunc(player_t * player, Cheat_t * cheat);
static void CheatKeysFunc(player_t * player, Cheat_t * cheat);
static void CheatSoundFunc(player_t * player, Cheat_t * cheat);
static void CheatTickerFunc(player_t * player, Cheat_t * cheat);
static void CheatArtifactAllFunc(player_t * player, Cheat_t * cheat);
static void CheatPuzzleFunc(player_t * player, Cheat_t * cheat);
static void CheatWarpFunc(player_t * player, Cheat_t * cheat);
static void CheatPigFunc(player_t * player, Cheat_t * cheat);
static void CheatMassacreFunc(player_t * player, Cheat_t * cheat);
static void CheatIDKFAFunc(player_t * player, Cheat_t * cheat);
static void CheatQuickenFunc1(player_t * player, Cheat_t * cheat);
static void CheatQuickenFunc2(player_t * player, Cheat_t * cheat);
static void CheatQuickenFunc3(player_t * player, Cheat_t * cheat);
static void CheatClassFunc1(player_t * player, Cheat_t * cheat);
static void CheatClassFunc2(player_t * player, Cheat_t * cheat);
static void CheatInitFunc(player_t * player, Cheat_t * cheat);
static void CheatVersionFunc(player_t * player, Cheat_t * cheat);
static void CheatDebugFunc(player_t * player, Cheat_t * cheat);
static void CheatScriptFunc1(player_t * player, Cheat_t * cheat);
static void CheatScriptFunc2(player_t * player, Cheat_t * cheat);
static void CheatScriptFunc3(player_t * player, Cheat_t * cheat);
static void CheatRevealFunc(player_t * player, Cheat_t * cheat);
static void CheatTrackFunc1(player_t * player, Cheat_t * cheat);
static void CheatTrackFunc2(player_t * player, Cheat_t * cheat);

// [crispy] new cheat functions
static void CheatShowFpsFunc(player_t * player, Cheat_t * cheat);

// [crispy] player crosshair functions
static void HU_DrawCrosshair(void);
static byte *R_CrosshairColor(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DECLARATIONS ------------------------------------------------

boolean DebugSound;             // Debug flag for displaying sound info
boolean inventory;
int curpos;
int inv_ptr;
int ArtifactFlash;

// [crispy] for widescreen status bar background
pixel_t *st_backing_screen;

// [crispy] for conditional drawing of status bar elements
void (*V_DrawSBPatch)(int x, int y, patch_t *patch) = V_DrawPatch;

// [crispy] on/off status bar translucency
void SB_Translucent(boolean translucent)
{
    V_DrawSBPatch = translucent ? V_DrawAltTLPatch : V_DrawPatch;
}

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int DisplayTicker = 0;
static int HealthMarker;
//static int ChainWiggle;
static player_t *CPlayer;
static int SpinFlylump;
static int SpinMinotaurLump;
static int SpinSpeedLump;
static int SpinDefenseLump;

// [crispy] keep in sync with multiitem_t multiitem_he_crosshairtype[] in mn_menu.c
static crosshairpatch_t crosshairpatch_m[NUM_HE_CROSSHAIRTYPE] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
};
static crosshairpatch_t *crosshairpatch = crosshairpatch_m;

static int FontBNumBase;
static int PlayPalette;

static patch_t *PatchH2BAR;
static patch_t *PatchH2TOP;
static patch_t *PatchLFEDGE;
static patch_t *PatchRTEDGE;
static patch_t *PatchARMCLEAR;
static patch_t *PatchARTICLEAR;
static patch_t *PatchMANACLEAR;
static patch_t *PatchKILLS;
static patch_t *PatchMANAVIAL1;
static patch_t *PatchMANAVIAL2;
static patch_t *PatchMANAVIALDIM1;
static patch_t *PatchMANAVIALDIM2;
static patch_t *PatchMANADIM1;
static patch_t *PatchMANADIM2;
static patch_t *PatchMANABRIGHT1;
static patch_t *PatchMANABRIGHT2;
static patch_t *PatchCHAIN;
static patch_t *PatchSTATBAR;
static patch_t *PatchKEYBAR;
static patch_t *PatchLIFEGEM;
static patch_t *PatchSELECTBOX;
static patch_t *PatchINumbers[10];
static patch_t *PatchNEGATIVE;
static patch_t *PatchSmNumbers[10];
static patch_t *PatchINVBAR;
static patch_t *PatchWEAPONSLOT;
static patch_t *PatchWEAPONFULL;
static patch_t *PatchPIECE1;
static patch_t *PatchPIECE2;
static patch_t *PatchPIECE3;
static patch_t *PatchINVLFGEM1;
static patch_t *PatchINVLFGEM2;
static patch_t *PatchINVRTGEM1;
static patch_t *PatchINVRTGEM2;

// Toggle god mode
cheatseq_t CheatGodSeq = CHEAT("satan", 0);

// Toggle no clipping mode
cheatseq_t CheatNoClipSeq = CHEAT("casper", 0);

// Get all weapons and mana
cheatseq_t CheatWeaponsSeq = CHEAT("nra", 0);

// Get full health
cheatseq_t CheatHealthSeq =  CHEAT("clubmed", 0);

// Get all keys
cheatseq_t CheatKeysSeq = CHEAT("locksmith", 0);

// Toggle sound debug info
cheatseq_t CheatSoundSeq = CHEAT("noise", 0);

// Toggle ticker
cheatseq_t CheatTickerSeq = CHEAT("ticker", 0);

// Get all artifacts
cheatseq_t CheatArtifactAllSeq = CHEAT("indiana", 0);

// Get all puzzle pieces
cheatseq_t CheatPuzzleSeq = CHEAT("sherlock", 0);

// Warp to new level
cheatseq_t CheatWarpSeq = CHEAT("visit", 2);

// Become a pig
cheatseq_t CheatPigSeq = CHEAT("deliverance", 0);

// Kill all monsters
cheatseq_t CheatMassacreSeq = CHEAT("butcher", 0);

cheatseq_t CheatIDKFASeq = CHEAT("conan", 0);

cheatseq_t CheatQuickenSeq1 = CHEAT("martek", 0);

cheatseq_t CheatQuickenSeq2 = CHEAT("martekmartek", 0);

cheatseq_t CheatQuickenSeq3 = CHEAT("martekmartekmartek", 0);

// New class
cheatseq_t CheatClass1Seq = CHEAT("shadowcaster", 0);

cheatseq_t CheatClass2Seq = CHEAT("shadowcaster", 1);

cheatseq_t CheatInitSeq = CHEAT("init", 0);

cheatseq_t CheatVersionSeq = CHEAT("mrjones", 0);

cheatseq_t CheatDebugSeq = CHEAT("where", 0);

cheatseq_t CheatScriptSeq1 = CHEAT("puke", 0);

cheatseq_t CheatScriptSeq2 = CHEAT("puke", 1);

cheatseq_t CheatScriptSeq3 = CHEAT("puke", 2);

cheatseq_t CheatRevealSeq = CHEAT("mapsco", 0);

cheatseq_t CheatTrackSeq1 = CHEAT("`", 0);

cheatseq_t CheatTrackSeq2 = CHEAT("`", 2);

// [crispy] new cheat sequences
cheatseq_t CheatShowFpsSeq = CHEAT("showfps", 0); // Show FPS

static Cheat_t Cheats[] = {
    {CheatTrackFunc1, &CheatTrackSeq1},
    {CheatTrackFunc2, &CheatTrackSeq2},
    {CheatGodFunc, &CheatGodSeq},
    {CheatNoClipFunc, &CheatNoClipSeq},
    {CheatWeaponsFunc, &CheatWeaponsSeq},
    {CheatHealthFunc, &CheatHealthSeq},
    {CheatKeysFunc, &CheatKeysSeq},
    {CheatSoundFunc, &CheatSoundSeq},
    {CheatTickerFunc, &CheatTickerSeq},
    {CheatArtifactAllFunc, &CheatArtifactAllSeq},
    {CheatPuzzleFunc, &CheatPuzzleSeq},
    {CheatWarpFunc, &CheatWarpSeq},
    {CheatPigFunc, &CheatPigSeq},
    {CheatMassacreFunc, &CheatMassacreSeq},
    {CheatIDKFAFunc, &CheatIDKFASeq},
    {CheatQuickenFunc1, &CheatQuickenSeq1},
    {CheatQuickenFunc2, &CheatQuickenSeq2},
    {CheatQuickenFunc3, &CheatQuickenSeq3},
    {CheatClassFunc1, &CheatClass1Seq},
    {CheatClassFunc2, &CheatClass2Seq},
    {CheatInitFunc, &CheatInitSeq},
    {CheatVersionFunc, &CheatVersionSeq},
    {CheatDebugFunc, &CheatDebugSeq},
    {CheatScriptFunc1, &CheatScriptSeq1},
    {CheatScriptFunc2, &CheatScriptSeq2},
    {CheatScriptFunc3, &CheatScriptSeq3},
    {CheatRevealFunc, &CheatRevealSeq},

    // [crispy] new cheats
    {CheatShowFpsFunc, &CheatShowFpsSeq},
};

#define SET_CHEAT(cheat, seq) \
    { memcpy(cheat.sequence, seq, sizeof(seq)); \
      cheat.sequence_len = sizeof(seq) - 1; }

// CODE --------------------------------------------------------------------

//==========================================================================
//
// SB_Init
//
//==========================================================================

void SB_Init(void)
{
    int i;
    int startLump;

    PatchH2BAR = W_CacheLumpName("H2BAR", PU_STATIC);
    PatchH2TOP = W_CacheLumpName("H2TOP", PU_STATIC);
    PatchINVBAR = W_CacheLumpName("INVBAR", PU_STATIC);
    PatchLFEDGE = W_CacheLumpName("LFEDGE", PU_STATIC);
    PatchRTEDGE = W_CacheLumpName("RTEDGE", PU_STATIC);
    PatchSTATBAR = W_CacheLumpName("STATBAR", PU_STATIC);
    PatchKEYBAR = W_CacheLumpName("KEYBAR", PU_STATIC);
    PatchSELECTBOX = W_CacheLumpName("SELECTBOX", PU_STATIC);
    PatchARTICLEAR = W_CacheLumpName("ARTICLS", PU_STATIC);
    PatchARMCLEAR = W_CacheLumpName("ARMCLS", PU_STATIC);
    PatchMANACLEAR = W_CacheLumpName("MANACLS", PU_STATIC);
    PatchMANAVIAL1 = W_CacheLumpName("MANAVL1", PU_STATIC);
    PatchMANAVIAL2 = W_CacheLumpName("MANAVL2", PU_STATIC);
    PatchMANAVIALDIM1 = W_CacheLumpName("MANAVL1D", PU_STATIC);
    PatchMANAVIALDIM2 = W_CacheLumpName("MANAVL2D", PU_STATIC);
    PatchMANADIM1 = W_CacheLumpName("MANADIM1", PU_STATIC);
    PatchMANADIM2 = W_CacheLumpName("MANADIM2", PU_STATIC);
    PatchMANABRIGHT1 = W_CacheLumpName("MANABRT1", PU_STATIC);
    PatchMANABRIGHT2 = W_CacheLumpName("MANABRT2", PU_STATIC);
    PatchINVLFGEM1 = W_CacheLumpName("invgeml1", PU_STATIC);
    PatchINVLFGEM2 = W_CacheLumpName("invgeml2", PU_STATIC);
    PatchINVRTGEM1 = W_CacheLumpName("invgemr1", PU_STATIC);
    PatchINVRTGEM2 = W_CacheLumpName("invgemr2", PU_STATIC);

//      PatchCHAINBACK = W_CacheLumpName("CHAINBACK", PU_STATIC);
    startLump = W_GetNumForName("IN0");
    for (i = 0; i < 10; i++)
    {
        PatchINumbers[i] = W_CacheLumpNum(startLump + i, PU_STATIC);
    }
    PatchNEGATIVE = W_CacheLumpName("NEGNUM", PU_STATIC);
    FontBNumBase = W_GetNumForName("FONTB16");
    startLump = W_GetNumForName("SMALLIN0");
    for (i = 0; i < 10; i++)
    {
        PatchSmNumbers[i] = W_CacheLumpNum(startLump + i, PU_STATIC);
    }
    PlayPalette = W_GetNumForName("PLAYPAL");
    SpinFlylump = W_GetNumForName("SPFLY0");
    SpinMinotaurLump = W_GetNumForName("SPMINO0");
    SpinSpeedLump = W_GetNumForName("SPBOOT0");
    SpinDefenseLump = W_GetNumForName("SPSHLD0");

    // [crispy] initialize the crosshair types
    for (i = 0; i < NUM_HE_CROSSHAIRTYPE; i++)
    {
        patch_t *patch = NULL;

        if (i == CROSSHAIRTYPE_HE_DOT)
        {
            crosshairpatch[i].l = W_GetNumForName("FONTB28");
        }
        else if (i == CROSSHAIRTYPE_HE_CROSS1)
        {
            crosshairpatch[i].l = W_GetNumForName("TLGLC0");
        }
        else
        {
            crosshairpatch[i].l = W_GetNumForName("TLGLA0");
        }

        patch = W_CacheLumpNum(crosshairpatch[i].l, PU_STATIC);

        crosshairpatch[i].w = SHORT(patch->width) / 2;
        crosshairpatch[i].h = SHORT(patch->height) / 2;
        crosshairpatch[i].loffset = SHORT(patch->leftoffset);
        crosshairpatch[i].toffset = SHORT(patch->topoffset); 
    }

    st_backing_screen = (pixel_t *) Z_Malloc(MAXWIDTH * (ORIGSBARHEIGHT << 1) * sizeof(*st_backing_screen), PU_STATIC, 0);
    if (deathmatch)
    {
        PatchKILLS = W_CacheLumpName("KILLS", PU_STATIC);
    }
    SB_SetClassData();

    if (gamemode == shareware)
    {
	SET_CHEAT(CheatGodSeq, "bgokey");
	SET_CHEAT(CheatNoClipSeq, "rjohnson");
	SET_CHEAT(CheatWeaponsSeq, "crhinehart");
	SET_CHEAT(CheatHealthSeq,"sgurno");
	SET_CHEAT(CheatKeysSeq, "mraymondjudy");
	SET_CHEAT(CheatSoundSeq, "kschilder");
	SET_CHEAT(CheatTickerSeq, "rrettenmund");
	SET_CHEAT(CheatArtifactAllSeq, "braffel");
	SET_CHEAT(CheatPuzzleSeq, "tmoore");
	SET_CHEAT(CheatWarpSeq, "bpelletier");
	SET_CHEAT(CheatPigSeq, "ebiessman");
	SET_CHEAT(CheatMassacreSeq, "cstika");
	SET_CHEAT(CheatIDKFASeq, "rambo");
	SET_CHEAT(CheatQuickenSeq1, "quicken");
	SET_CHEAT(CheatQuickenSeq2, "quickenquicken");
	SET_CHEAT(CheatQuickenSeq3, "quickenquickenquicken");
	SET_CHEAT(CheatClass1Seq, "plipo");
	SET_CHEAT(CheatClass2Seq, "plipo");
	SET_CHEAT(CheatVersionSeq, "pmacarther");
	SET_CHEAT(CheatDebugSeq, "jsumwalt");
	SET_CHEAT(CheatScriptSeq1, "mwagabaza");
	SET_CHEAT(CheatScriptSeq2, "mwagabaza");
	SET_CHEAT(CheatScriptSeq3, "mwagabaza");
	SET_CHEAT(CheatRevealSeq, "reveal");
    }
}

//==========================================================================
//
// SB_SetClassData
//
//==========================================================================

void SB_SetClassData(void)
{
    int class;

    class = PlayerClass[consoleplayer]; // original player class (not pig)
    PatchWEAPONSLOT = W_CacheLumpNum(W_GetNumForName("wpslot0")
                                     + class, PU_STATIC);
    PatchWEAPONFULL = W_CacheLumpNum(W_GetNumForName("wpfull0")
                                     + class, PU_STATIC);
    PatchPIECE1 = W_CacheLumpNum(W_GetNumForName("wpiecef1")
                                 + class, PU_STATIC);
    PatchPIECE2 = W_CacheLumpNum(W_GetNumForName("wpiecef2")
                                 + class, PU_STATIC);
    PatchPIECE3 = W_CacheLumpNum(W_GetNumForName("wpiecef3")
                                 + class, PU_STATIC);
    PatchCHAIN = W_CacheLumpNum(W_GetNumForName("chain") + class, PU_STATIC);
    if (!netgame)
    {                           // single player game uses red life gem (the second gem)
        PatchLIFEGEM = W_CacheLumpNum(W_GetNumForName("lifegem")
                                      + maxplayers * class + 1, PU_STATIC);
    }
    else
    {
        PatchLIFEGEM = W_CacheLumpNum(W_GetNumForName("lifegem")
                                      + maxplayers * class + consoleplayer,
                                      PU_STATIC);
    }
    SB_state = -1;
    UpdateState |= I_FULLSCRN;
}

//==========================================================================
//
// SB_Ticker
//
//==========================================================================

void SB_Ticker(void)
{
    int delta;
    int curHealth;

    curHealth = players[consoleplayer].mo->health;
    if (curHealth < 0)
    {
        curHealth = 0;
    }
    if (curHealth < HealthMarker)
    {
        delta = (HealthMarker - curHealth) >> 2;
        if (delta < 1)
        {
            delta = 1;
        }
        else if (delta > 6)
        {
            delta = 6;
        }
        HealthMarker -= delta;
    }
    else if (curHealth > HealthMarker)
    {
        delta = (curHealth - HealthMarker) >> 2;
        if (delta < 1)
        {
            delta = 1;
        }
        else if (delta > 6)
        {
            delta = 6;
        }
        HealthMarker += delta;
    }
    SB_PaletteFlash(false);
}

//==========================================================================
//
// DrINumber
//
// Draws a three digit number.
//
//==========================================================================

static void DrINumber(signed int val, int x, int y)
{
    patch_t *patch;
    int oldval;

    oldval = val;
    if (val < 0)
    {
        val = -val;
        if (val > 99)
        {
            val = 99;
        }
        if (val > 9)
        {
            patch = PatchINumbers[val / 10];
            V_DrawSBPatch(x + 8, y, patch);
            V_DrawSBPatch(x, y, PatchNEGATIVE);
        }
        else
        {
            V_DrawSBPatch(x + 8, y, PatchNEGATIVE);
        }
        val = val % 10;
        patch = PatchINumbers[val];
        V_DrawSBPatch(x + 16, y, patch);
        return;
    }
    if (val > 99)
    {
        patch = PatchINumbers[val / 100];
        V_DrawSBPatch(x, y, patch);
    }
    val = val % 100;
    if (val > 9 || oldval > 99)
    {
        patch = PatchINumbers[val / 10];
        V_DrawSBPatch(x + 8, y, patch);
    }
    val = val % 10;
    patch = PatchINumbers[val];
    V_DrawSBPatch(x + 16, y, patch);
}

//==========================================================================
//
// DrRedINumber
//
// Draws a three digit number using the red font
//
//==========================================================================

static void DrRedINumber(signed int val, int x, int y)
{
    patch_t *patch;
    int oldval;

    oldval = val;
    if (val < 0)
    {
        val = 0;
    }
    if (val > 99)
    {
        patch =
            W_CacheLumpNum(W_GetNumForName("inred0") + val / 100, PU_CACHE);
        V_DrawSBPatch(x, y, patch);
    }
    val = val % 100;
    if (val > 9 || oldval > 99)
    {
        patch =
            W_CacheLumpNum(W_GetNumForName("inred0") + val / 10, PU_CACHE);
        V_DrawSBPatch(x + 8, y, patch);
    }
    val = val % 10;
    patch = W_CacheLumpNum(W_GetNumForName("inred0") + val, PU_CACHE);
    V_DrawSBPatch(x + 16, y, patch);
}

//==========================================================================
//
// DrBNumber
//
// Draws a three digit number using FontB
//
//==========================================================================

static void DrBNumber(signed int val, int x, int y)
{
    patch_t *patch;
    int xpos;
    int oldval;

    oldval = val;
    xpos = x;
    if (val < 0)
    {
        val = 0;
    }
    if (val > 99)
    {
        patch = W_CacheLumpNum(FontBNumBase + val / 100, PU_CACHE);
        V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2, y, patch);
    }
    val = val % 100;
    xpos += 12;
    if (val > 9 || oldval > 99)
    {
        patch = W_CacheLumpNum(FontBNumBase + val / 10, PU_CACHE);
        V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2, y, patch);
    }
    val = val % 10;
    xpos += 12;
    patch = W_CacheLumpNum(FontBNumBase + val, PU_CACHE);
    V_DrawShadowedPatch(xpos + 6 - SHORT(patch->width) / 2, y, patch);
}

//==========================================================================
//
// DrSmallNumber
//
// Draws a small two digit number.
//
//==========================================================================

static void DrSmallNumber(int val, int x, int y)
{
    patch_t *patch;

    if (val <= 0)
    {
        return;
    }
    if (val > 999)
    {
        val %= 1000;
    }
    if (val > 99)
    {
        patch = PatchSmNumbers[val / 100];
        V_DrawSBPatch(x, y, patch);
        patch = PatchSmNumbers[(val % 100) / 10];
        V_DrawSBPatch(x + 4, y, patch);
    }
    else if (val > 9)
    {
        patch = PatchSmNumbers[val / 10];
        V_DrawSBPatch(x + 4, y, patch);
    }
    val %= 10;
    patch = PatchSmNumbers[val];
    V_DrawSBPatch(x + 8, y, patch);
}

/*
//==========================================================================
//
// ShadeLine
//
//==========================================================================

static void ShadeLine(int x, int y, int height, int shade)
{
	byte *dest;
	byte *shades;

	shades = colormaps+9*256+shade*2*256;
	dest = I_VideoBuffer+y*SCREENWIDTH+x;
	while(height--)
	{
		*(dest) = *(shades+*dest);
		dest += SCREENWIDTH;
	}
}

//==========================================================================
//
// ShadeChain
//
//==========================================================================

static void ShadeChain(void)
{
	int i;

	for(i = 0; i < 16; i++)
	{
		ShadeLine(277+i, 190, 10, i/2);
		ShadeLine(19+i, 190, 10, 7-(i/2));
	}
}
*/

//==========================================================================
//
// DrawSoundInfo
//
// Displays sound debugging information.
//
//==========================================================================

static void DrawSoundInfo(void)
{
    int i;
    SoundInfo_t s;
    ChanInfo_t *c;
    char text[32];
    int x;
    int y;
    int xPos[7] = { 1, 75, 112, 156, 200, 230, 260 };

    if (leveltime & 16)
    {
        MN_DrTextA("*** SOUND DEBUG INFO ***", xPos[0], 20);
    }
    S_GetChannelInfo(&s);
    if (s.channelCount == 0)
    {
        return;
    }
    x = 0;
    MN_DrTextA("NAME", xPos[x++], 30);
    MN_DrTextA("MO.T", xPos[x++], 30);
    MN_DrTextA("MO.X", xPos[x++], 30);
    MN_DrTextA("MO.Y", xPos[x++], 30);
    MN_DrTextA("ID", xPos[x++], 30);
    MN_DrTextA("PRI", xPos[x++], 30);
    MN_DrTextA("DIST", xPos[x++], 30);
    for (i = 0; i < s.channelCount; i++)
    {
        c = &s.chan[i];
        x = 0;
        y = 40 + i * 10;
        if (c->mo == NULL)
        {                       // Channel is unused
            MN_DrTextA("------", xPos[0], y);
            continue;
        }
        M_snprintf(text, sizeof(text), "%s", c->name);
        M_ForceUppercase(text);
        MN_DrTextA(text, xPos[x++], y);
        M_snprintf(text, sizeof(text), "%d", c->mo->type);
        MN_DrTextA(text, xPos[x++], y);
        M_snprintf(text, sizeof(text), "%d", c->mo->x >> FRACBITS);
        MN_DrTextA(text, xPos[x++], y);
        M_snprintf(text, sizeof(text), "%d", c->mo->y >> FRACBITS);
        MN_DrTextA(text, xPos[x++], y);
        M_snprintf(text, sizeof(text), "%d", (int) c->id);
        MN_DrTextA(text, xPos[x++], y);
        M_snprintf(text, sizeof(text), "%d", c->priority);
        MN_DrTextA(text, xPos[x++], y);
        M_snprintf(text, sizeof(text), "%d", c->distance);
        MN_DrTextA(text, xPos[x++], y);
    }
    UpdateState |= I_FULLSCRN;
    BorderNeedRefresh = true;
}

//==========================================================================
//
// SB_Drawer
//
//==========================================================================

char patcharti[][10] = {
    {"ARTIBOX"},                // none
    {"ARTIINVU"},               // invulnerability
    {"ARTIPTN2"},               // health
    {"ARTISPHL"},               // superhealth
    {"ARTIHRAD"},               // healing radius
    {"ARTISUMN"},               // summon maulator
    {"ARTITRCH"},               // torch
    {"ARTIPORK"},               // egg
    {"ARTISOAR"},               // fly
    {"ARTIBLST"},               // blast radius
    {"ARTIPSBG"},               // poison bag
    {"ARTITELO"},               // teleport other
    {"ARTISPED"},               // speed
    {"ARTIBMAN"},               // boost mana
    {"ARTIBRAC"},               // boost armor
    {"ARTIATLP"},               // teleport
    {"ARTISKLL"},               // arti_puzzskull
    {"ARTIBGEM"},               // arti_puzzgembig
    {"ARTIGEMR"},               // arti_puzzgemred
    {"ARTIGEMG"},               // arti_puzzgemgreen1
    {"ARTIGMG2"},               // arti_puzzgemgreen2
    {"ARTIGEMB"},               // arti_puzzgemblue1
    {"ARTIGMB2"},               // arti_puzzgemblue2
    {"ARTIBOK1"},               // arti_puzzbook1
    {"ARTIBOK2"},               // arti_puzzbook2
    {"ARTISKL2"},               // arti_puzzskull2
    {"ARTIFWEP"},               // arti_puzzfweapon
    {"ARTICWEP"},               // arti_puzzcweapon
    {"ARTIMWEP"},               // arti_puzzmweapon
    {"ARTIGEAR"},               // arti_puzzgear1
    {"ARTIGER2"},               // arti_puzzgear2
    {"ARTIGER3"},               // arti_puzzgear3
    {"ARTIGER4"},               // arti_puzzgear4
};

int SB_state = -1;
static int oldarti = 0;
static int oldartiCount = 0;
static int oldfrags = -9999;
static int oldmana1 = -1;
static int oldmana2 = -1;
static int oldarmor = -1;
static int oldhealth = -1;
static int oldlife = -1;
static int oldpieces = -1;
static int oldweapon = -1;
static int oldkeys = -1;


// [crispy] Needed to support widescreen status bar.
void SB_ForceRedraw(void)
{
    SB_state = -1;
}

// [crispy] Create background texture which appears at each side of the status
// bar in widescreen rendering modes. The chosen textures match those which
// surround the non-fullscreen game window.
static void RefreshBackground(void)
{
    V_UseBuffer(st_backing_screen);

    if ((SCREENWIDTH >> crispy->hires) != ORIGWIDTH)
    {
        byte *src;
        pixel_t *dest;

        src = W_CacheLumpName("F_022", PU_CACHE);
        dest = st_backing_screen;

        V_FillFlat(SCREENHEIGHT - SBARHEIGHT, SCREENHEIGHT, 0, SCREENWIDTH, src, dest);

        // [crispy] preserve bezel bottom edge
        if (scaledviewwidth == SCREENWIDTH)
        {
            int x;
            patch_t *const patch = W_CacheLumpName("bordb", PU_CACHE);

            for (x = 0; x < WIDESCREENDELTA; x += 16)
            {
                V_DrawPatch(x - WIDESCREENDELTA, 0, patch);
                V_DrawPatch(ORIGWIDTH + WIDESCREENDELTA - x - 16, 0, patch);
            }
        }
    }

    V_RestoreBuffer();
    V_CopyRect(0, 0, st_backing_screen, SCREENWIDTH >> crispy->hires,
                   ORIGSBARHEIGHT, 0, ORIGHEIGHT - ORIGSBARHEIGHT);
}

extern int right_widget_w; // [crispy]

void SB_Drawer(void)
{
    // Sound info debug stuff
    if (DebugSound == true)
    {
        DrawSoundInfo();
    }
    CPlayer = &players[consoleplayer];
    if (viewheight == SCREENHEIGHT
        && !(automapactive && !crispy->automapoverlay))
    {
        DrawFullScreenStuff();
        SB_state = -1;
    }
    else
    {
        if (SB_state == -1)
        {
            RefreshBackground(); // [crispy] for widescreen

            // [crispy] support wide status bars with 0 offset
            if (SHORT(PatchH2BAR->width) > ORIGWIDTH &&
                    SHORT(PatchH2BAR->leftoffset) == 0)
            {
                V_DrawPatch((ORIGWIDTH - SHORT(PatchH2BAR->width)) / 2, 134,
                        PatchH2BAR);
            }
            else
            {
                V_DrawPatch(0, 134, PatchH2BAR);
            }

            oldhealth = -1;
        }
        DrawCommonBar();
        if (!inventory)
        {
            if (SB_state != 0)
            {
                // Main interface
                if (!(automapactive && !crispy->automapoverlay))
                {
                    V_DrawPatch(38, 162, PatchSTATBAR);
                }
                else
                {
                    V_DrawPatch(38, 162, PatchKEYBAR);
                }
                oldarti = 0;
                oldmana1 = -1;
                oldmana2 = -1;
                oldarmor = -1;
                oldpieces = -1;
                oldfrags = -9999;       //can't use -1, 'cuz of negative frags
                oldlife = -1;
                oldweapon = -1;
                oldkeys = -1;
            }
            if (!(automapactive && !crispy->automapoverlay))
            {
                DrawMainBar();
            }
            else
            {
                DrawKeyBar();
            }
            SB_state = 0;
        }
        else
        {
            DrawInventoryBar();
            SB_state = 1;
        }
    }
    DrawAnimatedIcons();

    // [crispy] draw player crosshair
    if (crispy->crosshair)
    {
        SB_Translucent(crispy->crosshair == CROSSHAIR_HE_TRANSLUCENT);
        HU_DrawCrosshair();
        SB_Translucent(TRANSLUCENT_HUD);
    }
}

//==========================================================================
//
// DrawAnimatedIcons
//
//==========================================================================

static void DrawAnimatedIcons(void)
{
    int frame;
    static boolean hitCenterFrame;
    int spinfly_x, spinspeed_x, spindefense_x, spinminotaur_x; // [crispy]

    // Wings of wrath
    if (CPlayer->powers[pw_flight])
    {
        spinfly_x = 20 - WIDESCREENDELTA; // [crispy]

        if (CPlayer->powers[pw_flight] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_flight] & 16))
        {
            frame = (leveltime / 3) & 15;
            if (CPlayer->mo->flags2 & MF2_FLY)
            {
                if (hitCenterFrame && (frame != 15 && frame != 0))
                {
                    V_DrawSBPatch(spinfly_x, 19,
                                W_CacheLumpNum(SpinFlylump + 15,
                                                PU_CACHE));
                }
                else
                {
                    V_DrawSBPatch(spinfly_x, 19,
                                W_CacheLumpNum(SpinFlylump + frame,
                                                PU_CACHE));
                    hitCenterFrame = false;
                }
            }
            else
            {
                if (!hitCenterFrame && (frame != 15 && frame != 0))
                {
                    V_DrawSBPatch(spinfly_x, 19,
                                W_CacheLumpNum(SpinFlylump + frame,
                                                PU_CACHE));
                    hitCenterFrame = false;
                }
                else
                {
                    V_DrawSBPatch(spinfly_x, 19,
                                W_CacheLumpNum(SpinFlylump + 15,
                                                PU_CACHE));
                    hitCenterFrame = true;
                }
            }
        }
        BorderTopRefresh = true;
        UpdateState |= I_MESSAGES;
    }

    // Speed Boots
    if (CPlayer->powers[pw_speed])
    {
        spinspeed_x = 60 - WIDESCREENDELTA; // [crispy]

        if (CPlayer->powers[pw_speed] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_speed] & 16))
        {
            frame = (leveltime / 3) & 15;
            V_DrawSBPatch(spinspeed_x, 19,
                        W_CacheLumpNum(SpinSpeedLump + frame,
                                        PU_CACHE));
        }
        BorderTopRefresh = true;
        UpdateState |= I_MESSAGES;
    }

    // Defensive power
    if (CPlayer->powers[pw_invulnerability])
    {
        spindefense_x = 260 + WIDESCREENDELTA; // [crispy]
        spindefense_x -= right_widget_w; // [crispy]

        if (CPlayer->powers[pw_invulnerability] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_invulnerability] & 16))
        {
            frame = (leveltime / 3) & 15;
            V_DrawSBPatch(spindefense_x, 19,
                        W_CacheLumpNum(SpinDefenseLump + frame,
                                        PU_CACHE));
        }
        BorderTopRefresh = true;
        UpdateState |= I_MESSAGES;
    }

    // Minotaur Active
    if (CPlayer->powers[pw_minotaur])
    {
        spinminotaur_x = 300 + WIDESCREENDELTA; // [crispy]
        spinminotaur_x -= right_widget_w; // [crispy]

        if (CPlayer->powers[pw_minotaur] > BLINKTHRESHOLD
            || !(CPlayer->powers[pw_minotaur] & 16))
        {
            frame = (leveltime / 3) & 15;
            V_DrawSBPatch(spinminotaur_x, 19,
                        W_CacheLumpNum(SpinMinotaurLump + frame,
                                        PU_CACHE));
        }
        BorderTopRefresh = true;
        UpdateState |= I_MESSAGES;
    }
}

//==========================================================================
//
// SB_PaletteFlash
//
// Sets the new palette based upon the current values of
// consoleplayer->damagecount and consoleplayer->bonuscount.
//
//==========================================================================

void SB_PaletteFlash(boolean forceChange)
{
    static int sb_palette = 0;
    int palette;
#ifndef CRISPY_TRUECOLOR
    byte *pal;
#endif

    if (forceChange)
    {
        sb_palette = -1;
    }
    if (gamestate == GS_LEVEL)
    {
        CPlayer = &players[consoleplayer];
        // [crispy] A11Y
        if (!a11y_palette_changes)
        {
            palette = 0;
        }
        else if (CPlayer->poisoncount)
        {
            palette = 0;
            palette = (CPlayer->poisoncount + 7) >> 3;
            if (palette >= NUMPOISONPALS)
            {
                palette = NUMPOISONPALS - 1;
            }
            palette += STARTPOISONPALS;
        }
        else if (CPlayer->damagecount)
        {
            palette = (CPlayer->damagecount + 7) >> 3;
            if (palette >= NUMREDPALS)
            {
                palette = NUMREDPALS - 1;
            }
            palette += STARTREDPALS;
        }
        else if (CPlayer->bonuscount)
        {
            palette = (CPlayer->bonuscount + 7) >> 3;
            if (palette >= NUMBONUSPALS)
            {
                palette = NUMBONUSPALS - 1;
            }
            palette += STARTBONUSPALS;
        }
        else if (CPlayer->mo->flags2 & MF2_ICEDAMAGE)
        {                       // Frozen player
            palette = STARTICEPAL;
        }
        else
        {
            palette = 0;
        }
    }
    else
    {
        palette = 0;
    }
    if (palette != sb_palette)
    {
        sb_palette = palette;
#ifndef CRISPY_TRUECOLOR
        pal = (byte *) W_CacheLumpNum(PlayPalette, PU_CACHE) + palette * 768;
        I_SetPalette(pal);
#else
        I_SetPalette(palette);
#endif
    }
}

//==========================================================================
//
// DrawCommonBar
//
//==========================================================================

void DrawCommonBar(void)
{
    int healthPos;

    V_DrawPatch(0, 134, PatchH2TOP);

    if (oldhealth != HealthMarker)
    {
        oldhealth = HealthMarker;
        healthPos = HealthMarker;
        if (healthPos < 0)
        {
            healthPos = 0;
        }
        if (healthPos > 100)
        {
            healthPos = 100;
        }
        V_DrawPatch(28 + (((healthPos * 196) / 100) % 9), 193, PatchCHAIN);
        V_DrawPatch(7 + ((healthPos * 11) / 5), 193, PatchLIFEGEM);
        V_DrawPatch(0, 193, PatchLFEDGE);
        V_DrawPatch(277, 193, PatchRTEDGE);
//              ShadeChain();
        UpdateState |= I_STATBAR;
    }
}

//==========================================================================
//
// DrawMainBar
//
//==========================================================================

void DrawMainBar(void)
{
    int i, j, k;
    int temp;
    patch_t *manaPatch1, *manaPatch2;
    patch_t *manaVialPatch1, *manaVialPatch2;

    manaPatch1 = NULL;
    manaPatch2 = NULL;
    manaVialPatch1 = NULL;
    manaVialPatch2 = NULL;

    // Ready artifact
    if (ArtifactFlash)
    {
        V_DrawPatch(144, 160, PatchARTICLEAR);
        V_DrawPatch(148, 164, W_CacheLumpNum(W_GetNumForName("useartia")
                                             + ArtifactFlash - 1, PU_CACHE));
        ArtifactFlash--;
        oldarti = -1;           // so that the correct artifact fills in after the flash
        UpdateState |= I_STATBAR;
    }
    else if (oldarti != CPlayer->readyArtifact
             || oldartiCount != CPlayer->inventory[inv_ptr].count)
    {
        V_DrawPatch(144, 160, PatchARTICLEAR);
        if (CPlayer->readyArtifact > 0)
        {
            V_DrawPatch(143, 163,
                        W_CacheLumpName(patcharti[CPlayer->readyArtifact],
                                        PU_CACHE));
            if (CPlayer->inventory[inv_ptr].count > 1)
            {
                DrSmallNumber(CPlayer->inventory[inv_ptr].count, 162, 184);
            }
        }
        oldarti = CPlayer->readyArtifact;
        oldartiCount = CPlayer->inventory[inv_ptr].count;
        UpdateState |= I_STATBAR;
    }

    // Frags
    if (deathmatch)
    {
        temp = 0;
        for (i = 0; i < maxplayers; i++)
        {
            temp += CPlayer->frags[i];
        }
        if (temp != oldfrags)
        {
            V_DrawPatch(38, 162, PatchKILLS);
            DrINumber(temp, 40, 176);
            oldfrags = temp;
            UpdateState |= I_STATBAR;
        }
    }
    else
    {
        temp = HealthMarker;
        if (temp < 0)
        {
            temp = 0;
        }
        else if (temp > 100)
        {
            temp = 100;
        }
        if (oldlife != temp)
        {
            oldlife = temp;
            V_DrawPatch(41, 178, PatchARMCLEAR);
            if (temp >= 25)
            {
                DrINumber(temp, 40, 176);
            }
            else
            {
                DrRedINumber(temp, 40, 176);
            }
            UpdateState |= I_STATBAR;
        }
    }
    // Mana
    temp = CPlayer->mana[0];
    if (oldmana1 != temp)
    {
        V_DrawPatch(77, 178, PatchMANACLEAR);
        DrSmallNumber(temp, 79, 181);
        manaVialPatch1 = (patch_t *) 1; // force a vial update
        if (temp == 0)
        {                       // Draw Dim Mana icon
            manaPatch1 = PatchMANADIM1;
        }
        else if (oldmana1 == 0)
        {
            manaPatch1 = PatchMANABRIGHT1;
        }
        oldmana1 = temp;
        UpdateState |= I_STATBAR;
    }
    temp = CPlayer->mana[1];
    if (oldmana2 != temp)
    {
        V_DrawPatch(109, 178, PatchMANACLEAR);
        DrSmallNumber(temp, 111, 181);
        manaVialPatch1 = (patch_t *) 1; // force a vial update
        if (temp == 0)
        {                       // Draw Dim Mana icon
            manaPatch2 = PatchMANADIM2;
        }
        else if (oldmana2 == 0)
        {
            manaPatch2 = PatchMANABRIGHT2;
        }
        oldmana2 = temp;
        UpdateState |= I_STATBAR;
    }
    if (oldweapon != CPlayer->readyweapon || manaPatch1 || manaPatch2
        || manaVialPatch1)
    {                           // Update mana graphics based upon mana count/weapon type
        if (CPlayer->readyweapon == WP_FIRST)
        {
            manaPatch1 = PatchMANADIM1;
            manaPatch2 = PatchMANADIM2;
            manaVialPatch1 = PatchMANAVIALDIM1;
            manaVialPatch2 = PatchMANAVIALDIM2;
        }
        else if (CPlayer->readyweapon == WP_SECOND)
        {
            if (!manaPatch1)
            {
                manaPatch1 = PatchMANABRIGHT1;
            }
            manaVialPatch1 = PatchMANAVIAL1;
            manaPatch2 = PatchMANADIM2;
            manaVialPatch2 = PatchMANAVIALDIM2;
        }
        else if (CPlayer->readyweapon == WP_THIRD)
        {
            manaPatch1 = PatchMANADIM1;
            manaVialPatch1 = PatchMANAVIALDIM1;
            if (!manaPatch2)
            {
                manaPatch2 = PatchMANABRIGHT2;
            }
            manaVialPatch2 = PatchMANAVIAL2;
        }
        else
        {
            manaVialPatch1 = PatchMANAVIAL1;
            manaVialPatch2 = PatchMANAVIAL2;
            if (!manaPatch1)
            {
                manaPatch1 = PatchMANABRIGHT1;
            }
            if (!manaPatch2)
            {
                manaPatch2 = PatchMANABRIGHT2;
            }
        }
        V_DrawPatch(77, 164, manaPatch1);
        V_DrawPatch(110, 164, manaPatch2);
        V_DrawPatch(94, 164, manaVialPatch1);
        for (i = 165; i < 187 - (22 * CPlayer->mana[0]) / MAX_MANA; i++)
        {
         for (j = 0; j <= crispy->hires; j++)
          for (k = 0; k <= crispy->hires; k++)
          {
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((95 + WIDESCREENDELTA) << crispy->hires) + k] = 0;
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((96 + WIDESCREENDELTA) << crispy->hires) + k] = 0;
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((97 + WIDESCREENDELTA) << crispy->hires) + k] = 0;
          }
        }
        V_DrawPatch(102, 164, manaVialPatch2);
        for (i = 165; i < 187 - (22 * CPlayer->mana[1]) / MAX_MANA; i++)
        {
         for (j = 0; j <= crispy->hires; j++)
          for (k = 0; k <= crispy->hires; k++)
          {
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((103 + WIDESCREENDELTA) << crispy->hires) + k] = 0;
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((104 + WIDESCREENDELTA) << crispy->hires) + k] = 0;
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((105 + WIDESCREENDELTA) << crispy->hires) + k] = 0;
          }
        }
        oldweapon = CPlayer->readyweapon;
        UpdateState |= I_STATBAR;
    }
    // Armor
    temp = AutoArmorSave[CPlayer->class]
        + CPlayer->armorpoints[ARMOR_ARMOR] +
        CPlayer->armorpoints[ARMOR_SHIELD] +
        CPlayer->armorpoints[ARMOR_HELMET] +
        CPlayer->armorpoints[ARMOR_AMULET];
    if (oldarmor != temp)
    {
        oldarmor = temp;
        V_DrawPatch(255, 178, PatchARMCLEAR);
        DrINumber(FixedDiv(temp, 5 * FRACUNIT) >> FRACBITS, 250, 176);
        UpdateState |= I_STATBAR;
    }
    // Weapon Pieces
    if (oldpieces != CPlayer->pieces)
    {
        DrawWeaponPieces();
        oldpieces = CPlayer->pieces;
        UpdateState |= I_STATBAR;
    }
}

//==========================================================================
//
// DrawInventoryBar
//
//==========================================================================

void DrawInventoryBar(void)
{
    int i;
    int x;

    x = inv_ptr - curpos;
    UpdateState |= I_STATBAR;
    V_DrawPatch(38, 162, PatchINVBAR);
    for (i = 0; i < 7; i++)
    {
        //V_DrawPatch(50+i*31, 160, W_CacheLumpName("ARTIBOX", PU_CACHE));
        if (CPlayer->inventorySlotNum > x + i
            && CPlayer->inventory[x + i].type != arti_none)
        {
            V_DrawPatch(50 + i * 31, 163,
                        W_CacheLumpName(patcharti
                                        [CPlayer->inventory[x + i].type],
                                        PU_CACHE));
            if (CPlayer->inventory[x + i].count > 1)
            {
                DrSmallNumber(CPlayer->inventory[x + i].count, 68 + i * 31,
                              185);
            }
        }
    }
    V_DrawPatch(50 + curpos * 31, 163, PatchSELECTBOX);
    if (x != 0)
    {
        V_DrawPatch(42, 163, !(leveltime & 4) ? PatchINVLFGEM1 :
                    PatchINVLFGEM2);
    }
    if (CPlayer->inventorySlotNum - x > 7)
    {
        V_DrawPatch(269, 163, !(leveltime & 4) ? PatchINVRTGEM1 :
                    PatchINVRTGEM2);
    }
}

//==========================================================================
//
// DrawKeyBar
//
//==========================================================================

void DrawKeyBar(void)
{
    int i;
    int xPosition;
    int temp;

    if (oldkeys != CPlayer->keys)
    {
        xPosition = 46;
        for (i = 0; i < NUM_KEY_TYPES && xPosition <= 126; i++)
        {
            if (CPlayer->keys & (1 << i))
            {
                V_DrawPatch(xPosition, 164,
                            W_CacheLumpNum(W_GetNumForName("keyslot1") + i,
                                           PU_CACHE));
                xPosition += 20;
            }
        }
        oldkeys = CPlayer->keys;
        UpdateState |= I_STATBAR;
    }
    temp = AutoArmorSave[CPlayer->class]
        + CPlayer->armorpoints[ARMOR_ARMOR] +
        CPlayer->armorpoints[ARMOR_SHIELD] +
        CPlayer->armorpoints[ARMOR_HELMET] +
        CPlayer->armorpoints[ARMOR_AMULET];
    if (oldarmor != temp)
    {
        for (i = 0; i < NUMARMOR; i++)
        {
            if (!CPlayer->armorpoints[i])
            {
                continue;
            }
            if (CPlayer->armorpoints[i] <=
                (ArmorIncrement[CPlayer->class][i] >> 2))
            {
                V_DrawTLPatch(150 + 31 * i, 164,
                              W_CacheLumpNum(W_GetNumForName("armslot1") +
                                             i, PU_CACHE));
            }
            else if (CPlayer->armorpoints[i] <=
                     (ArmorIncrement[CPlayer->class][i] >> 1))
            {
                V_DrawAltTLPatch(150 + 31 * i, 164,
                                 W_CacheLumpNum(W_GetNumForName("armslot1")
                                                + i, PU_CACHE));
            }
            else
            {
                V_DrawPatch(150 + 31 * i, 164,
                            W_CacheLumpNum(W_GetNumForName("armslot1") + i,
                                           PU_CACHE));
            }
        }
        oldarmor = temp;
        UpdateState |= I_STATBAR;
    }
}

//==========================================================================
//
// DrawWeaponPieces
//
//==========================================================================

static int PieceX[NUMCLASSES][3] = {
    {190, 225, 234},
    {190, 212, 225},
    {190, 205, 224},
    {0, 0, 0}                   // Pig is never used
};

static void DrawWeaponPieces(void)
{
    if (CPlayer->pieces == 7)
    {
        V_DrawPatch(190, 162, PatchWEAPONFULL);
        return;
    }
    V_DrawPatch(190, 162, PatchWEAPONSLOT);
    if (CPlayer->pieces & WPIECE1)
    {
        V_DrawPatch(PieceX[PlayerClass[consoleplayer]][0], 162, PatchPIECE1);
    }
    if (CPlayer->pieces & WPIECE2)
    {
        V_DrawPatch(PieceX[PlayerClass[consoleplayer]][1], 162, PatchPIECE2);
    }
    if (CPlayer->pieces & WPIECE3)
    {
        V_DrawPatch(PieceX[PlayerClass[consoleplayer]][2], 162, PatchPIECE3);
    }
}

//==========================================================================
//
// DrawFullScreenStuff
//
//==========================================================================

void DrawFullScreenStuff(void)
{
    int i;
    int x;
    int temp;
    int xPosGem1; // [crispy] for intersect detection
    int xPosManaPatch2; // [crispy] for intersect detection
    int sboffset; // [crispy] to apply WIDESCREENDELTA

    UpdateState |= I_FULLSCRN;
    
    // [crispy] check for widescreen HUD
    if (screenblocks == 12 || screenblocks >= 15)
    {
        sboffset = WIDESCREENDELTA;
    }
    else
    {
        sboffset = 0;
    }

    // [crispy] Crispy Hud
    if(screenblocks >= 13)
    {
        int j, k;
        patch_t *manaPatch1, *manaPatch2;
        patch_t *manaVialPatch1, *manaVialPatch2;

        manaPatch1 = NULL;
        manaPatch2 = NULL;
        manaVialPatch1 = NULL;
        manaVialPatch2 = NULL;
        xPosGem1 = 40;
        xPosManaPatch2 = 75 - sboffset;

        // Health
        temp = CPlayer->mo->health;
        if (temp >= 25)
        {
            DrINumber(temp, 5 - sboffset, 182);
        }
        else
        {
            DrRedINumber(temp, 5 - sboffset, 182);
        }
        // Armor
        temp = AutoArmorSave[CPlayer->class]
            + CPlayer->armorpoints[ARMOR_ARMOR] +
            CPlayer->armorpoints[ARMOR_SHIELD] +
            CPlayer->armorpoints[ARMOR_HELMET] +
            CPlayer->armorpoints[ARMOR_AMULET];
            oldarmor = temp;
        DrINumber(FixedDiv(temp, 5 * FRACUNIT) >> FRACBITS, 288 + sboffset, 182);
        // Frags
        if (deathmatch)
        {
            temp = 0;
            for (i = 0; i < maxplayers; i++)
            {
                if (playeringame[i])
                {
                    temp += CPlayer->frags[i];
                }
            }
            DrINumber(temp, 5 - sboffset, 165);
        }
        // Items, Itemflash and Selection Bar
        if (!inventory)
        {
            if (ArtifactFlash)
            {
                V_DrawSBPatch(113 - sboffset, 170, W_CacheLumpNum(W_GetNumForName("useartia")
                                                     + ArtifactFlash - 1, PU_CACHE));
                ArtifactFlash--;
            }
            else if (CPlayer->readyArtifact > 0)
            {
                V_DrawSBPatch(108 - sboffset, 169,
                            W_CacheLumpName(patcharti[CPlayer->readyArtifact],
                                            PU_CACHE));
                if (CPlayer->inventory[inv_ptr].count > 1)
                {
                    DrSmallNumber(CPlayer->inventory[inv_ptr].count, 127 - sboffset, 190);
                }
            }
        }
        else
        {
            x = inv_ptr - curpos;
            for (i = 0; i < 7; i++)
            {
                // [crispy] check for translucent HUD
                SB_Translucent(TRANSLUCENT_HUD);
                V_DrawSBPatch(50 + i * 31, 168, W_CacheLumpName("ARTIBOX",
                                                                PU_CACHE));
                SB_Translucent(false); // listed artifacts and selectbox are always opaque
                if (CPlayer->inventorySlotNum > x + i
                    && CPlayer->inventory[x + i].type != arti_none)
                {
                    V_DrawSBPatch(48 + i * 31, 167,
                                W_CacheLumpName(patcharti
                                                [CPlayer->inventory[x + i].type],
                                                PU_CACHE));
                    if (CPlayer->inventory[x + i].count > 1)
                    {
                        DrSmallNumber(CPlayer->inventory[x + i].count,
                                      66 + i * 31, 189);
                    }
                }
            }
            V_DrawSBPatch(48 + curpos * 31, 167, PatchSELECTBOX);
            // [crispy] check for translucent HUD
            SB_Translucent(TRANSLUCENT_HUD);
            if (x != 0)
            {
                V_DrawSBPatch(xPosGem1, 167, !(leveltime & 4) ? PatchINVLFGEM1 :
                            PatchINVLFGEM2);
            }
            if (CPlayer->inventorySlotNum - x > 7)
            {
                V_DrawSBPatch(268, 167, !(leveltime & 4) ?
                            PatchINVRTGEM1 : PatchINVRTGEM2);
            }
            // Check for Intersect
            if (xPosManaPatch2 + 15 >= xPosGem1)
            {
                return; // Stop drawing further widgets
            }
        }
        // Mana
        temp = CPlayer->mana[0];
        DrSmallNumber(temp, 44 - sboffset, 187);
        manaVialPatch1 = (patch_t *) 1; // force a vial update
        if (temp == 0)
        {                       // Draw Dim Mana icon
            manaPatch1 = PatchMANADIM1;
        }
        else if (oldmana1 == 0)
        {
            manaPatch1 = PatchMANABRIGHT1;
        }
        temp = CPlayer->mana[1];
        DrSmallNumber(temp, 76 - sboffset, 187);
        manaVialPatch1 = (patch_t *) 1; // force a vial update
        if (temp == 0)
        {                       // Draw Dim Mana icon
            manaPatch2 = PatchMANADIM2;
        }
        else if (oldmana2 == 0)
        {
            manaPatch2 = PatchMANABRIGHT2;
        }
        // Update mana graphics based upon mana count/weapon type
        if (CPlayer->readyweapon == WP_FIRST)
        {
            manaPatch1 = PatchMANADIM1;
            manaPatch2 = PatchMANADIM2;
            manaVialPatch1 = PatchMANAVIALDIM1;
            manaVialPatch2 = PatchMANAVIALDIM2;
        }
        else if (CPlayer->readyweapon == WP_SECOND)
        {
            if (!manaPatch1)
            {
                manaPatch1 = PatchMANABRIGHT1;
            }
            manaVialPatch1 = PatchMANAVIAL1;
            manaPatch2 = PatchMANADIM2;
            manaVialPatch2 = PatchMANAVIALDIM2;
        }
        else if (CPlayer->readyweapon == WP_THIRD)
        {
            manaPatch1 = PatchMANADIM1;
            manaVialPatch1 = PatchMANAVIALDIM1;
            if (!manaPatch2)
            {
                manaPatch2 = PatchMANABRIGHT2;
            }
            manaVialPatch2 = PatchMANAVIAL2;
        }
        else
        {
            manaVialPatch1 = PatchMANAVIAL1;
            manaVialPatch2 = PatchMANAVIAL2;
            if (!manaPatch1)
            {
                manaPatch1 = PatchMANABRIGHT1;
            }
            if (!manaPatch2)
            {
                manaPatch2 = PatchMANABRIGHT2;
            }
        }
        V_DrawSBPatch(42 - sboffset, 170, manaPatch1);
        V_DrawSBPatch(xPosManaPatch2, 170, manaPatch2);
        V_DrawSBPatch(59 - sboffset, 170, manaVialPatch1);
        for (i = 171; i < 193 - (22 * CPlayer->mana[0]) / MAX_MANA; i++)
        {
         for (j = 0; j <= crispy->hires; j++)
          for (k = 0; k <= crispy->hires; k++)
          {
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((60 + (sboffset > 0 ? 0 : WIDESCREENDELTA))<< crispy->hires) + k] = 0;
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((61 + (sboffset > 0 ? 0 : WIDESCREENDELTA))<< crispy->hires) + k] = 0;
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((62 + (sboffset > 0 ? 0 : WIDESCREENDELTA))<< crispy->hires) + k] = 0;
          }
        }
        V_DrawSBPatch(67 - sboffset, 170, manaVialPatch2);
        for (i = 171; i < 193 - (22 * CPlayer->mana[1]) / MAX_MANA; i++)
        {
         for (j = 0; j <= crispy->hires; j++)
          for (k = 0; k <= crispy->hires; k++)
          {
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((68 + (sboffset > 0 ? 0 : WIDESCREENDELTA))<< crispy->hires) + k] = 0;
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((69 + (sboffset > 0 ? 0 : WIDESCREENDELTA))<< crispy->hires) + k] = 0;
            I_VideoBuffer[SCREENWIDTH * ((i << crispy->hires) + j)
                          + ((70 + (sboffset > 0 ? 0 : WIDESCREENDELTA))<< crispy->hires) + k] = 0;
          }
        }
        // Weapon Pieces
        if (CPlayer->pieces == 7)
        {
            V_DrawSBPatch(228 + sboffset, 168, PatchWEAPONFULL);
        }
        else
        {
            V_DrawSBPatch(228 + sboffset, 168, PatchWEAPONSLOT);
            if (CPlayer->pieces & WPIECE1)
            {
                V_DrawSBPatch(PieceX[PlayerClass[consoleplayer]][0] + 38 + sboffset, 168, PatchPIECE1);
            }
            if (CPlayer->pieces & WPIECE2)
            {
                V_DrawSBPatch(PieceX[PlayerClass[consoleplayer]][1] + 38 + sboffset, 168, PatchPIECE2);
            }
            if (CPlayer->pieces & WPIECE3)
            {
                V_DrawSBPatch(PieceX[PlayerClass[consoleplayer]][2] + 38 + sboffset, 168, PatchPIECE3);
            }
        }
        return;
    }
    if (CPlayer->mo->health > 0)
    {
        DrBNumber(CPlayer->mo->health, 5 - sboffset, 180);
    }
    else
    {
        DrBNumber(0, 5 - sboffset, 180);
    }
    if (deathmatch)
    {
        temp = 0;
        for (i = 0; i < maxplayers; i++)
        {
            if (playeringame[i])
            {
                temp += CPlayer->frags[i];
            }
        }
        DrINumber(temp, 45 - sboffset, 185);
    }
    if (!inventory)
    {
        if (CPlayer->readyArtifact > 0)
        {
            V_DrawTLPatch(286 + sboffset, 170, W_CacheLumpName("ARTIBOX", PU_CACHE));
            V_DrawPatch(284 + sboffset, 169,
                        W_CacheLumpName(patcharti[CPlayer->readyArtifact],
                                        PU_CACHE));
            if (CPlayer->inventory[inv_ptr].count > 1)
            {
                DrSmallNumber(CPlayer->inventory[inv_ptr].count, 302 + sboffset, 192);
            }
        }
    }
    else
    {
        x = inv_ptr - curpos;
        for (i = 0; i < 7; i++)
        {
            V_DrawTLPatch(50 + i * 31, 168, W_CacheLumpName("ARTIBOX",
                                                            PU_CACHE));
            if (CPlayer->inventorySlotNum > x + i
                && CPlayer->inventory[x + i].type != arti_none)
            {
                V_DrawPatch(49 + i * 31, 167,
                            W_CacheLumpName(patcharti
                                            [CPlayer->inventory[x + i].type],
                                            PU_CACHE));
                if (CPlayer->inventory[x + i].count > 1)
                {
                    DrSmallNumber(CPlayer->inventory[x + i].count,
                                  66 + i * 31, 188);
                }
            }
        }
        V_DrawPatch(50 + curpos * 31, 167, PatchSELECTBOX);
        if (x != 0)
        {
            V_DrawPatch(40, 167, !(leveltime & 4) ? PatchINVLFGEM1 :
                        PatchINVLFGEM2);
        }
        if (CPlayer->inventorySlotNum - x > 7)
        {
            V_DrawPatch(268, 167, !(leveltime & 4) ?
                        PatchINVRTGEM1 : PatchINVRTGEM2);
        }
    }
}


//==========================================================================
//
// Draw_TeleportIcon
//
//==========================================================================
void Draw_TeleportIcon(void)
{
    patch_t *patch;
    patch = W_CacheLumpNum(W_GetNumForName("teleicon"), PU_CACHE);
    V_DrawPatch(100, 68, patch);
    UpdateState |= I_FULLSCRN;
    I_FinishUpdate();
    UpdateState |= I_FULLSCRN;
}

//==========================================================================
//
// Draw_SaveIcon
//
//==========================================================================
void Draw_SaveIcon(void)
{
    patch_t *patch;
    patch = W_CacheLumpNum(W_GetNumForName("saveicon"), PU_CACHE);
    V_DrawPatch(100, 68, patch);
    UpdateState |= I_FULLSCRN;
    I_FinishUpdate();
    UpdateState |= I_FULLSCRN;
}

//==========================================================================
//
// Draw_LoadIcon
//
//==========================================================================
void Draw_LoadIcon(void)
{
    patch_t *patch;
    patch = W_CacheLumpNum(W_GetNumForName("loadicon"), PU_CACHE);
    V_DrawPatch(100, 68, patch);
    UpdateState |= I_FULLSCRN;
    I_FinishUpdate();
    UpdateState |= I_FULLSCRN;
}



//==========================================================================
//
// SB_Responder
//
//==========================================================================

boolean SB_Responder(event_t * event)
{
    if (event->type == ev_keydown)
    {
        if (HandleCheats(event->data1))
        {                       // Need to eat the key
            return (true);
        }
    }
    return (false);
}

//==========================================================================
//
// HandleCheats
//
// Returns true if the caller should eat the key.
//
//==========================================================================

static boolean HandleCheats(byte key)
{
    int i;
    boolean eat;

    if (gameskill == sk_nightmare)
    {                           // Can't cheat in nightmare mode
        return (false);
    }
    else if (netgame)
    {                           // change CD track is the only cheat available in deathmatch
        eat = false;
        if (cdmusic)
        {
            if (CheatAddKey(&Cheats[0], key, &eat))
            {
                Cheats[0].func(&players[consoleplayer], &Cheats[0]);
                S_StartSound(NULL, SFX_PLATFORM_STOP);
            }
            if (CheatAddKey(&Cheats[1], key, &eat))
            {
                Cheats[1].func(&players[consoleplayer], &Cheats[1]);
                S_StartSound(NULL, SFX_PLATFORM_STOP);
            }
        }
        return eat;
    }
    if (players[consoleplayer].health <= 0)
    {                           // Dead players can't cheat
        return (false);
    }
    eat = false;
    for (i = 0; i<arrlen(Cheats); ++i)
    {
        if (CheatAddKey(&Cheats[i], key, &eat))
        {
            Cheats[i].func(&players[consoleplayer], &Cheats[i]);
            S_StartSound(NULL, SFX_PLATFORM_STOP);
        }
    }
    return (eat);
}

//==========================================================================
//
// CheatAddkey
//
// Returns true if the added key completed the cheat, false otherwise.
//
//==========================================================================

static boolean CheatAddKey(Cheat_t * cheat, byte key, boolean * eat)
{
/*
    if (!cheat->pos)
    {
        cheat->pos = cheat->sequence;
        cheat->currentArg = 0;
    }
    if (*cheat->pos == 0)
    {
        *eat = true;
        cheat->args[cheat->currentArg++] = key;
        cheat->pos++;
    }
    else if (CheatLookup[key] == *cheat->pos)
    {
        cheat->pos++;
    }
    else
    {
        cheat->pos = cheat->sequence;
        cheat->currentArg = 0;
    }
    if (*cheat->pos == 0xff)
    {
        cheat->pos = cheat->sequence;
        cheat->currentArg = 0;
        return (true);
    }
    return (false);
    */

    *eat = cht_CheckCheat(cheat->seq, key);

    return *eat;
}

//==========================================================================
//
// CHEAT FUNCTIONS
//
//==========================================================================

static void CheatGodFunc(player_t * player, Cheat_t * cheat)
{
    player->cheats ^= CF_GODMODE;
    if (player->cheats & CF_GODMODE)
    {
        P_SetMessage(player, TXT_CHEATGODON, true);
    }
    else
    {
        P_SetMessage(player, TXT_CHEATGODOFF, true);
    }
    SB_state = -1;
}

static void CheatNoClipFunc(player_t * player, Cheat_t * cheat)
{
    player->cheats ^= CF_NOCLIP;
    if (player->cheats & CF_NOCLIP)
    {
        P_SetMessage(player, TXT_CHEATNOCLIPON, true);
    }
    else
    {
        P_SetMessage(player, TXT_CHEATNOCLIPOFF, true);
    }
}

static void CheatWeaponsFunc(player_t * player, Cheat_t * cheat)
{
    int i;

    for (i = 0; i < NUMARMOR; i++)
    {
        player->armorpoints[i] = ArmorIncrement[player->class][i];
    }
    for (i = 0; i < NUMWEAPONS; i++)
    {
        player->weaponowned[i] = true;
    }
    for (i = 0; i < NUMMANA; i++)
    {
        player->mana[i] = MAX_MANA;
    }
    P_SetMessage(player, TXT_CHEATWEAPONS, true);
}

static void CheatHealthFunc(player_t * player, Cheat_t * cheat)
{
    if (player->morphTics)
    {
        player->health = player->mo->health = MAXMORPHHEALTH;
    }
    else
    {
        player->health = player->mo->health = MAXHEALTH;
    }
    P_SetMessage(player, TXT_CHEATHEALTH, true);
}

static void CheatKeysFunc(player_t * player, Cheat_t * cheat)
{
    player->keys = 2047;
    P_SetMessage(player, TXT_CHEATKEYS, true);
}

static void CheatSoundFunc(player_t * player, Cheat_t * cheat)
{
    DebugSound = !DebugSound;
    if (DebugSound)
    {
        P_SetMessage(player, TXT_CHEATSOUNDON, true);
    }
    else
    {
        P_SetMessage(player, TXT_CHEATSOUNDOFF, true);
    }
}

static void CheatTickerFunc(player_t * player, Cheat_t * cheat)
{
    DisplayTicker = !DisplayTicker;
    if (DisplayTicker)
    {
        P_SetMessage(player, TXT_CHEATTICKERON, true);
    }
    else
    {
        P_SetMessage(player, TXT_CHEATTICKEROFF, true);
    }

    I_DisplayFPSDots(DisplayTicker);
}

static void CheatArtifactAllFunc(player_t * player, Cheat_t * cheat)
{
    int i;
    int j;

    for (i = arti_none + 1; i < arti_firstpuzzitem; i++)
    {
        for (j = 0; j < 25; j++)
        {
            P_GiveArtifact(player, i, NULL);
        }
    }
    P_SetMessage(player, TXT_CHEATARTIFACTS3, true);
}

static void CheatPuzzleFunc(player_t * player, Cheat_t * cheat)
{
    int i;

    for (i = arti_firstpuzzitem; i < NUMARTIFACTS; i++)
    {
        P_GiveArtifact(player, i, NULL);
    }
    P_SetMessage(player, TXT_CHEATARTIFACTS3, true);
}

static void CheatInitFunc(player_t * player, Cheat_t * cheat)
{
    G_DeferedInitNew(gameskill, gameepisode, gamemap);
    P_SetMessage(player, TXT_CHEATWARP, true);
}

static void CheatWarpFunc(player_t * player, Cheat_t * cheat)
{
    int tens;
    int ones;
    int map;
    char mapName[9];
    char args[2];

    cht_GetParam(cheat->seq, args);

    tens = args[0] - '0';
    ones = args[1] - '0';
    if (tens < 0 || tens > 9 || ones < 0 || ones > 9)
    {                           // Bad map
        P_SetMessage(player, TXT_CHEATBADINPUT, true);
        return;
    }
    map = P_TranslateMap((args[0] - '0') * 10 + args[1] - '0');
    if (map == -1)
    {                           // Not found
        P_SetMessage(player, TXT_CHEATNOMAP, true);
        return;
    }
    if (map == gamemap)
    {                           // Don't try to teleport to current map
        P_SetMessage(player, TXT_CHEATBADINPUT, true);
        return;
    }
    M_snprintf(mapName, sizeof(mapName), "MAP%02d", map);
    if (W_CheckNumForName(mapName) == -1)
    {                       // Can't find
        P_SetMessage(player, TXT_CHEATNOMAP, true);
        return;
    }
    P_SetMessage(player, TXT_CHEATWARP, true);
    G_TeleportNewMap(map, 0);
}

static void CheatPigFunc(player_t * player, Cheat_t * cheat)
{
    if (player->morphTics)
    {
        P_UndoPlayerMorph(player);
    }
    else
    {
        P_MorphPlayer(player);
    }
    P_SetMessage(player, "SQUEAL!!", true);
}

static void CheatMassacreFunc(player_t * player, Cheat_t * cheat)
{
    int count;
    char buffer[80];

    count = P_Massacre();
    M_snprintf(buffer, sizeof(buffer), "%d MONSTERS KILLED\n", count);
    P_SetMessage(player, buffer, true);
}

static void CheatIDKFAFunc(player_t * player, Cheat_t * cheat)
{
    int i;
    if (player->morphTics)
    {
        return;
    }
    for (i = 1; i < NUMWEAPONS; i++)
    {
        player->weaponowned[i] = false;
    }

    // In the original code, NUMWEAPONS was 8. So the writes to weaponowned
    // overflowed the array. We must set the following fields to zero as
    // well:

    player->mana[0] = 0;
    player->mana[1] = 0;
    player->attackdown = 0;
    player->usedown = 0;

    player->pendingweapon = WP_FIRST;
    P_SetMessage(player, TXT_CHEATIDKFA, true);
}

static void CheatQuickenFunc1(player_t * player, Cheat_t * cheat)
{
    P_SetMessage(player, "TRYING TO CHEAT?  THAT'S ONE....", true);
}

static void CheatQuickenFunc2(player_t * player, Cheat_t * cheat)
{
    P_SetMessage(player, "THAT'S TWO....", true);
}

static void CheatQuickenFunc3(player_t * player, Cheat_t * cheat)
{
    P_DamageMobj(player->mo, NULL, player->mo, 10000);
    P_SetMessage(player, "THAT'S THREE!  TIME TO DIE.", true);
}

static void CheatClassFunc1(player_t * player, Cheat_t * cheat)
{
    P_SetMessage(player, "ENTER NEW PLAYER CLASS (0 - 2)", true);
}

static void CheatClassFunc2(player_t * player, Cheat_t * cheat)
{
    int i;
    int class;
    char args[2];

    cht_GetParam(cheat->seq, args);

    if (player->morphTics)
    {                           // don't change class if the player is morphed
        return;
    }
    class = args[0] - '0';
    if (class > 2 || class < 0)
    {
        P_SetMessage(player, "INVALID PLAYER CLASS", true);
        return;
    }
    player->class = class;
    for (i = 0; i < NUMARMOR; i++)
    {
        player->armorpoints[i] = 0;
    }
    PlayerClass[consoleplayer] = class;
    P_PostMorphWeapon(player, WP_FIRST);
    SB_SetClassData();
    SB_state = -1;
    UpdateState |= I_FULLSCRN;
}

static void CheatVersionFunc(player_t * player, Cheat_t * cheat)
{
    P_SetMessage(player, HEXEN_VERSIONTEXT, true);
}

static void CheatDebugFunc(player_t * player, Cheat_t * cheat)
{
    char textBuffer[50];
    M_snprintf(textBuffer, sizeof(textBuffer),
               "MAP %d (%d)  X:%5d  Y:%5d  Z:%5d",
               P_GetMapWarpTrans(gamemap),
               gamemap,
               player->mo->x >> FRACBITS,
               player->mo->y >> FRACBITS, player->mo->z >> FRACBITS);
    P_SetMessage(player, textBuffer, true);
}

static void CheatScriptFunc1(player_t * player, Cheat_t * cheat)
{
    P_SetMessage(player, "RUN WHICH SCRIPT(01-99)?", true);
}

static void CheatScriptFunc2(player_t * player, Cheat_t * cheat)
{
    P_SetMessage(player, "RUN WHICH SCRIPT(01-99)?", true);
}

static void CheatScriptFunc3(player_t * player, Cheat_t * cheat)
{
    int script;
    byte script_args[3];
    int tens, ones;
    char textBuffer[40];
    char args[2];

    cht_GetParam(cheat->seq, args);

    tens = args[0] - '0';
    ones = args[1] - '0';
    script = tens * 10 + ones;
    if (script < 1)
        return;
    if (script > 99)
        return;
    script_args[0] = script_args[1] = script_args[2] = 0;

    if (P_StartACS(script, 0, script_args, player->mo, NULL, 0))
    {
        M_snprintf(textBuffer, sizeof(textBuffer),
                   "RUNNING SCRIPT %.2d", script);
        P_SetMessage(player, textBuffer, true);
    }
}


static void CheatRevealFunc(player_t * player, Cheat_t * cheat)
{
    cheating = (cheating + 1) % 3;
}

//===========================================================================
//
// CheatTrackFunc1
//
//===========================================================================

static void CheatTrackFunc1(player_t * player, Cheat_t * cheat)
{
    char buffer[80];

    if (!cdmusic)
    {
        return;
    }

    if (I_CDMusInit() == -1)
    {
        P_SetMessage(player, "ERROR INITIALIZING CD", true);
    }

    M_snprintf(buffer, sizeof(buffer), "ENTER DESIRED CD TRACK (%.2d - %.2d):\n",
               I_CDMusFirstTrack(), I_CDMusLastTrack());
    P_SetMessage(player, buffer, true);
}

//===========================================================================
//
// CheatTrackFunc2
//
//===========================================================================

static void CheatTrackFunc2(player_t * player, Cheat_t * cheat)
{
    char buffer[80];
    int track;
    char args[2];

    cht_GetParam(cheat->seq, args);

    if (!cdmusic)
    {
        return;
    }

    track = (args[0] - '0') * 10 + (args[1] - '0');
    if (track < I_CDMusFirstTrack() || track > I_CDMusLastTrack())
    {
        P_SetMessage(player, "INVALID TRACK NUMBER\n", true);
        return;
    }

    if (track == S_GetCurrentCDTrack())
    {
        return;
    }

    if (!S_StartCustomCDTrack(track))
    {
        M_snprintf(buffer, sizeof(buffer),
                   "ERROR WHILE TRYING TO PLAY CD TRACK: %.2d\n", track);
        P_SetMessage(player, buffer, true);
    }
    else
    {
        // No error encountered while attempting to play the track
        M_snprintf(buffer, sizeof(buffer), "PLAYING TRACK: %.2d\n", track);
        P_SetMessage(player, buffer, true);
    }
}

// [crispy] "Cheat" to show FPS
static void CheatShowFpsFunc(player_t* player, Cheat_t* cheat)
{
    player->cheats ^= CF_SHOWFPS;
    if (player->cheats & CF_SHOWFPS)
    {
        P_SetMessage(player, TXT_SHOWFPSON, false);
    }
    else
    {
        P_SetMessage(player, TXT_SHOWFPSOFF, false);
    }
}

// [crispy] crosshair color selection
static byte *R_CrosshairColor (void)
{
    if (crispy->crosshaircolor == CROSSHAIRCOLOR_HE_GOLD)
    {
        return cr[CR_GOLD];
    }
    else if (crispy->crosshaircolor == CROSSHAIRCOLOR_HE_WHITE)
    {
        return cr[CR_GRAY];
    }
    else
    {
        return cr[CR_RED];
    }
}

// [crispy] static, non-projected crosshair
static void HU_DrawCrosshair (void)
{
    static int		lump;
    static patch_t*	patch;
    CPlayer = &players[consoleplayer];

    if (WeaponInfo[CPlayer->readyweapon][CPlayer->class].atkstate == S_PUNCHATK1_1 ||
        WeaponInfo[CPlayer->readyweapon][CPlayer->class].atkstate == S_CMACEATK_1 ||
        WeaponInfo[CPlayer->readyweapon][CPlayer->class].atkstate == S_SNOUTATK1 ||
        WeaponInfo[CPlayer->readyweapon][CPlayer->class].atkstate == S_FAXEATK_1 ||
        CPlayer->playerstate != PST_LIVE ||
        (automapactive && !crispy->automapoverlay) ||
        MenuActive ||
        paused)
	    return;

    if (lump != crosshairpatch[crispy->crosshairtype].l)
    {
        lump = crosshairpatch[crispy->crosshairtype].l;
        patch = W_CacheLumpNum(lump, PU_STATIC);
    }

    // crosshair color
    dp_translation = R_CrosshairColor();
    V_DrawSBPatch(ORIGWIDTH / 2 -
            crosshairpatch[crispy->crosshairtype].w + crosshairpatch[crispy->crosshairtype].loffset,
            ((screenblocks < 11) ? (ORIGHEIGHT - (SBARHEIGHT >> crispy->hires)) / 2 : ORIGHEIGHT / 2) -
            crosshairpatch[crispy->crosshairtype].h + crosshairpatch[crispy->crosshairtype].toffset,
            patch);
    dp_translation = NULL;
}
