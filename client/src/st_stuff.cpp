// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2006-2020 by The Odamex Team.
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
// DESCRIPTION:
//		Status bar code.
//		Does the face/direction indicator animation.
//		Does palette indicators as well (red pain/berserk, bright pickup)
//		[RH] Widget coordinates are relative to the console, not the screen!
//
//-----------------------------------------------------------------------------


#include "odamex.h"

#include "i_video.h"
#include "z_zone.h"
#include "m_random.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"
#include "p_inter.h"
#include "am_map.h"
#include "m_cheat.h"
#include "s_sound.h"
#include "v_video.h"
#include "v_text.h"
#include "gstrings.h"
#include "c_dispatch.h"
#include "cl_main.h"
#include "gi.h"
#include "cl_demo.h"
#include "c_console.h"
#include "g_gametype.h"
#include "d_event.h"

#include "p_ctf.h"


static bool st_needrefresh = true;

static bool st_stopped = true;


// lump number for PLAYPAL
static int		lu_palette;

EXTERN_CVAR(sv_allowredscreen)
EXTERN_CVAR(st_scale)
EXTERN_CVAR(screenblocks)
EXTERN_CVAR(g_lives)

// [RH] Status bar background
IWindowSurface* stbar_surface;
IWindowSurface* stnum_surface;

// functions in st_new.c
void ST_initNew();
void ST_unloadNew();

extern bool simulated_connection;

//
// STATUS BAR DATA
//

// N/256*100% probability
//	that the normal face state will change
#define ST_FACEPROBABILITY		96

// Location of status bar face
#define ST_FX					(143)
#define ST_FY					(0)

// Should be set to patch width
//	for tall numbers later on
#define ST_TALLNUMWIDTH 		(tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES 		5
#define ST_NUMSTRAIGHTFACES 	3
#define ST_NUMTURNFACES 		2
#define ST_NUMSPECIALFACES		3

#define ST_FACESTRIDE \
		  (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES		2

#define ST_NUMFACES \
		  (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET			(ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET			(ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET		(ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET		(ST_EVILGRINOFFSET + 1)
#define ST_GODFACE				(ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE 			(ST_GODFACE+1)

#define ST_FACESX				(143)
#define ST_FACESY				(0)

#define ST_EVILGRINCOUNT		(2*TICRATE)
#define ST_STRAIGHTFACECOUNT	(TICRATE/2)
#define ST_TURNCOUNT			(1*TICRATE)
#define ST_OUCHCOUNT			(1*TICRATE)
#define ST_RAMPAGEDELAY 		(2*TICRATE)

#define ST_MUCHPAIN 			20


// Location and size of statistics,
//	justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//		 Problem is, is the stuff rendered
//		 into a buffer,
//		 or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH			3
#define ST_AMMOX				(44)
#define ST_AMMOY				(3)

// HEALTH number pos.
#define ST_HEALTHWIDTH			3
#define ST_HEALTHX				(90)
#define ST_HEALTHY				(3)

// Weapon pos.
#define ST_ARMSX				(111)
#define ST_ARMSY				(4)
#define ST_ARMSBGX				(104)
#define ST_ARMSBGY				(0)
#define ST_ARMSXSPACE			12
#define ST_ARMSYSPACE			10

// Flags pos.
#define ST_FLAGSBGX				(106)
#define ST_FLAGSBGY				(0)

// Frags pos.
#define ST_FRAGSX				(138)
#define ST_FRAGSY				(3)
#define ST_FRAGSWIDTH			2

// ARMOR number pos.
#define ST_ARMORWIDTH			3
#define ST_ARMORX				(221)
#define ST_ARMORY				(3)

// Flagbox positions.
#define ST_FLGBOXX				(236)
#define ST_FLGBOXY				(0)
#define ST_FLGBOXBLUX			(239)
#define ST_FLGBOXBLUY			(3)
#define ST_FLGBOXREDX			(239)
#define ST_FLGBOXREDY			(18)

// Key icon positions.
#define ST_KEY0WIDTH			8
#define ST_KEY0HEIGHT			5
#define ST_KEY0X				(239)
#define ST_KEY0Y				(3)
#define ST_KEY1WIDTH			ST_KEY0WIDTH
#define ST_KEY1X				(239)
#define ST_KEY1Y				(13)
#define ST_KEY2WIDTH			ST_KEY0WIDTH
#define ST_KEY2X				(239)
#define ST_KEY2Y				(23)

// Ammunition counter.
#define ST_AMMO0WIDTH			3
#define ST_AMMO0HEIGHT			6
#define ST_AMMO0X				(288)
#define ST_AMMO0Y				(5)
#define ST_AMMO1WIDTH			ST_AMMO0WIDTH
#define ST_AMMO1X				(288)
#define ST_AMMO1Y				(11)
#define ST_AMMO2WIDTH			ST_AMMO0WIDTH
#define ST_AMMO2X				(288)
#define ST_AMMO2Y				(23)
#define ST_AMMO3WIDTH			ST_AMMO0WIDTH
#define ST_AMMO3X				(288)
#define ST_AMMO3Y				(17)

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH		3
#define ST_MAXAMMO0HEIGHT		5
#define ST_MAXAMMO0X			(314)
#define ST_MAXAMMO0Y			(5)
#define ST_MAXAMMO1WIDTH		ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X			(314)
#define ST_MAXAMMO1Y			(11)
#define ST_MAXAMMO2WIDTH		ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X			(314)
#define ST_MAXAMMO2Y			(23)
#define ST_MAXAMMO3WIDTH		ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X			(314)
#define ST_MAXAMMO3Y			(17)

// pistol
#define ST_WEAPON0X 			(110)
#define ST_WEAPON0Y 			(4)

// shotgun
#define ST_WEAPON1X 			(122)
#define ST_WEAPON1Y 			(4)

// chain gun
#define ST_WEAPON2X 			(134)
#define ST_WEAPON2Y 			(4)

// missile launcher
#define ST_WEAPON3X 			(110)
#define ST_WEAPON3Y 			(13)

// plasma gun
#define ST_WEAPON4X 			(122)
#define ST_WEAPON4Y 			(13)

 // bfg
#define ST_WEAPON5X 			(134)
#define ST_WEAPON5Y 			(13)

// WPNS title
#define ST_WPNSX				(109)
#define ST_WPNSY				(23)

 // DETH title
#define ST_DETHX				(109)
#define ST_DETHY				(23)

// [RH] Turned these into variables
// Size of statusbar.
// Now ([RH] truly) sensitive for scaling.
int						ST_HEIGHT;
int						ST_WIDTH;
int						ST_X;
int						ST_Y;

// used for making messages go away
static int				st_msgcounter=0;

// whether in automap or first-person
static st_stateenum_t	st_gamestate;

// whether left-side main status bar is active
static bool			st_statusbaron;

// whether status bar chat is active
static bool			st_chat;

// value of st_chat before message popped up
static bool			st_oldchat;

// whether chat window has the cursor on
static bool			st_cursoron;

// !deathmatch && st_statusbaron
static bool			st_armson;

// !deathmatch
static bool			st_fragson;

// main bar left
static lumpHandle_t sbar;

// 0-9, tall numbers
// [RH] no longer static
lumpHandle_t tallnum[10];

// tall % sign
// [RH] no longer static
lumpHandle_t tallpercent;

// 0-9, short, yellow (,different!) numbers
static lumpHandle_t shortnum[10];

// 3 key-cards, 3 skulls, [RH] 3 combined
lumpHandle_t keys[NUMCARDS + NUMCARDS / 2];

// face status patches [RH] no longer static
lumpHandle_t faces[ST_NUMFACES];

// negative number patch
lumpHandle_t negminus;

// LAME graphic
patch_t* lamenum;

// face background
static lumpHandle_t faceback;

// classic face background
static lumpHandle_t faceclassic[4];

 // main bar right
static lumpHandle_t armsbg;

// score/flags
static lumpHandle_t flagsbg;

// weapon ownership patches
static lumpHandle_t arms[6][2];

// ready-weapon widget
static st_number_t		w_ready;

 // in deathmatch only, summary of frags stats
static st_number_t		w_frags;

// health widget
static st_percent_t 	w_health;

// weapon ownership widgets
static st_multicon_t	w_arms[6];

// face status widget
static st_multicon_t	w_faces;

// keycard widgets
static st_multicon_t	w_keyboxes[3];

// armor widget
static st_percent_t 	w_armor;

// ammo widgets
static st_number_t		w_ammo[4];

// max ammo widgets
static st_number_t		w_maxammo[4];

// lives widget
static st_number_t		w_lives;

// number of frags so far in deathmatch
static int		st_fragscount;

// used to use appopriately pained face
static int		st_oldhealth = -1;

// used for evil grin
static bool		oldweaponsowned[NUMWEAPONS+1];

 // count until face changes
static int		st_facecount = 0;

// current face index, used by w_faces
// [RH] not static anymore
int				st_faceindex = 0;

// holds key-type for each key box on bar
static int		keyboxes[3];

// copy of player info
static int		st_health, st_armor;
static int		st_ammo[4], st_maxammo[4];
static int		st_weaponowned[6] = {0}, st_current_ammo;
static int		st_lives;

// a random number per tick
static int		st_randomnumber;

//
// Heretic State Variables
//
static int chainhealth; // current position of the gem
static int chainwiggle; // small randomized addend for chain y coord.

static lumpHandle_t statbar;
static lumpHandle_t invbar;
static lumpHandle_t sbar_back;
static lumpHandle_t godeyes_left;
static lumpHandle_t godeyes_right;
static lumpHandle_t sbar_topleft;
static lumpHandle_t sbar_topright;
static lumpHandle_t chain;
static lumpHandle_t lifegem;
static lumpHandle_t ltface;
static lumpHandle_t rtface;

// health widget
st_number_t w_numhealth;

// armor widget
st_number_t w_numarmor;

// these are now in d_dehacked.cpp
extern byte cheat_mus_seq[9];
extern byte cheat_choppers_seq[11];
extern byte cheat_god_seq[6];
extern byte cheat_ammo_seq[6];
extern byte cheat_ammonokey_seq[5];
extern byte cheat_noclip_seq[11];
extern byte cheat_commercial_noclip_seq[7];
extern byte cheat_powerup_seq[7][10];
extern byte cheat_clev_seq[10];
extern byte cheat_mypos_seq[8];

// Now what?
static byte CheatNoclip[] = {'i', 'd', 's', 'p', 'i', 's', 'p', 'o', 'p', 'd', 255};
static byte CheatNoclip2[] = {'i', 'd', 'c', 'l', 'i', 'p', 255};
static byte CheatMus[] = {'i', 'd', 'm', 'u', 's', 0, 0, 255};
static byte CheatChoppers[] = {'i', 'd', 'c', 'h', 'o', 'p', 'p', 'e', 'r', 's', 255};
static byte CheatGod[] = {'i', 'd', 'd', 'q', 'd', 255};
static byte CheatAmmo[] = {'i', 'd', 'k', 'f', 'a', 255};
static byte CheatAmmoNoKey[] = {'i', 'd', 'f', 'a', 255};
static byte CheatClev[] = {'i', 'd', 'c', 'l', 'e', 'v', 0, 0, 255};
static byte CheatMypos[] = {'i', 'd', 'm', 'y', 'p', 'o', 's', 255};
static byte CheatAmap[] = {'i', 'd', 'd', 't', 255};

static byte CheatPowerup[7][10] = {{'i', 'd', 'b', 'e', 'h', 'o', 'l', 'd', 'v', 255},
                                   {'i', 'd', 'b', 'e', 'h', 'o', 'l', 'd', 's', 255},
                                   {'i', 'd', 'b', 'e', 'h', 'o', 'l', 'd', 'i', 255},
                                   {'i', 'd', 'b', 'e', 'h', 'o', 'l', 'd', 'r', 255},
                                   {'i', 'd', 'b', 'e', 'h', 'o', 'l', 'd', 'a', 255},
                                   {'i', 'd', 'b', 'e', 'h', 'o', 'l', 'd', 'l', 255},
                                   {'i', 'd', 'b', 'e', 'h', 'o', 'l', 'd', 255}};

cheatseq_t DoomCheats[] = {
    {CheatMus, 0, 1, 0, {0, 0}, CHEAT_ChangeMusic},
    {CheatPowerup[6], 0, 1, 0, {0, 0}, CHEAT_BeholdMenu},
    {CheatMypos, 0, 1, 0, {0, 0}, CHEAT_IdMyPos},
    {CheatAmap, 0, 0, 0, {0, 0}, CHEAT_AutoMap},
    {CheatGod, 0, 0, 0, {CHT_IDDQD, 0}, CHEAT_SetGeneric},
    {CheatAmmo, 0, 0, 0, {CHT_IDKFA, 0}, CHEAT_SetGeneric},
    {CheatAmmoNoKey, 0, 0, 0, {CHT_IDFA, 0}, CHEAT_SetGeneric},
    {CheatNoclip, 0, 0, 0, {CHT_NOCLIP, 0}, CHEAT_SetGeneric},  // Special check given !
    {CheatNoclip2, 0, 0, 0, {CHT_NOCLIP, 1}, CHEAT_SetGeneric}, // Special Check given !
    {CheatPowerup[0], 0, 0, 0, {CHT_BEHOLDV, 0}, CHEAT_SetGeneric},
    {CheatPowerup[1], 0, 0, 0, {CHT_BEHOLDS, 0}, CHEAT_SetGeneric},
    {CheatPowerup[2], 0, 0, 0, {CHT_BEHOLDI, 0}, CHEAT_SetGeneric},
    {CheatPowerup[3], 0, 0, 0, {CHT_BEHOLDR, 0}, CHEAT_SetGeneric},
    {CheatPowerup[4], 0, 0, 0, {CHT_BEHOLDA, 0}, CHEAT_SetGeneric},
    {CheatPowerup[5], 0, 0, 0, {CHT_BEHOLDL, 0}, CHEAT_SetGeneric},
    {CheatChoppers, 0, 0, 0, {CHT_CHAINSAW, 0}, CHEAT_SetGeneric},
    {CheatClev, 0, 0, 0, {0, 0}, CHEAT_ChangeLevel}};

//
// STATUS BAR CODE
//

void ST_Stop();
void ST_createWidgets();

int ST_StatusBarHeight(int surface_width, int surface_height)
{
	if (!R_StatusBarVisible())
		return 0;

	if (st_scale)
		return 32 * surface_height / 200;
	else
		return 32;
}

int ST_StatusBarWidth(int surface_width, int surface_height)
{
	if (!R_StatusBarVisible())
		return 0;

	if (!st_scale)
		return 320;

	// [AM] Scale status bar width according to height, unless there isn't
	//      enough room for it.  Fixes widescreen status bar scaling.
	// [ML] A couple of minor changes for true 4:3 correctness...
	if (I_IsProtectedResolution(surface_width, surface_height))
		return 10 * ST_StatusBarHeight(surface_width, surface_height);
	else
		return 4 * surface_height / 3;
}

int ST_StatusBarX(int surface_width, int surface_height)
{
	if (!R_StatusBarVisible())
		return 0;

	if (consoleplayer().spectator && displayplayer_id == consoleplayer_id)
		return 0;
	else
		return (surface_width - ST_StatusBarWidth(surface_width, surface_height)) / 2;
}

int ST_StatusBarY(int surface_width, int surface_height)
{
	if (!R_StatusBarVisible())
		return surface_height;

	if (consoleplayer().spectator && displayplayer_id == consoleplayer_id)
		return surface_height;
	else
		return surface_height - ST_StatusBarHeight(surface_width, surface_height);
}


//
// ST_ForceRefresh
//
//
void ST_ForceRefresh()
{
	st_needrefresh = true;
}


CVAR_FUNC_IMPL (st_scale)
{
	R_SetViewSize((int)screenblocks);
	ST_ForceRefresh();
}


EXTERN_CVAR (sv_allowcheats)

// Respond to keyboard input events, intercept cheats.
// [RH] Cheats eatkey the last keypress used to trigger them
bool ST_Responder(event_t *ev)
{
	bool eat = false;

	// Filter automap on/off.
	if (ev->type == ev_keyup && ((ev->data1 & 0xffff0000) == AM_MSGHEADER))
	{
		switch(ev->data1)
		{
		case AM_MSGENTERED:
			st_gamestate = AutomapState;
			ST_ForceRefresh();
			break;

		case AM_MSGEXITED:
			st_gamestate = FirstPersonState;
			break;
		}
	}

	// if a user keypress...
	else if (ev->type == ev_keydown && ev->data3)
	{
		cheatseq_t* cheats = DoomCheats;
		for (int i = 0; i < COUNT_CHEATS(DoomCheats); i++, cheats++)
		{
			if (CHEAT_AddKey(cheats, (byte)ev->data1, &eat))
			{
				if (cheats->DontCheck || CHEAT_AreCheatsEnabled())
				{
					eat |= cheats->Handler(cheats);
				}
			}
		}
    }

    return eat;
}

// Console cheats
BEGIN_COMMAND (god)
{
	if (!CHEAT_AreCheatsEnabled())
		return;

	CHEAT_DoCheat(&consoleplayer(), CHT_GOD);
	CL_SendCheat(CHT_GOD);
}
END_COMMAND (god)

BEGIN_COMMAND (notarget)
{
	if (!CHEAT_AreCheatsEnabled())
		return;

	CHEAT_DoCheat(&consoleplayer(), CHT_NOTARGET);
	CL_SendCheat(CHT_NOTARGET);
}
END_COMMAND (notarget)

BEGIN_COMMAND (fly)
{
	if (!consoleplayer().spectator && !CHEAT_AreCheatsEnabled())
		return;

	CHEAT_DoCheat(&consoleplayer(), CHT_FLY);

	if (!consoleplayer().spectator)
	{
		CL_SendCheat(CHT_FLY);
	}
}
END_COMMAND (fly)

BEGIN_COMMAND (noclip)
{
	if (!CHEAT_AreCheatsEnabled())
		return;

	CHEAT_DoCheat(&consoleplayer(), CHT_NOCLIP);
	CL_SendCheat(CHT_NOCLIP);
}
END_COMMAND (noclip)

EXTERN_CVAR (chasedemo)

BEGIN_COMMAND (chase)
{
	if (demoplayback)
	{
		if (chasedemo)
		{
			chasedemo.Set(0.0f);
			for (Players::iterator it = players.begin();it != players.end();++it)
				it->cheats &= ~CF_CHASECAM;
		}
		else
		{
			chasedemo.Set(1.0f);
			for (Players::iterator it = players.begin();it != players.end();++it)
				it->cheats |= CF_CHASECAM;
		}
	}
	else
	{
		if (!CHEAT_AreCheatsEnabled())
			return;

		CHEAT_DoCheat(&consoleplayer(), CHT_CHASECAM);
		
	}
}
END_COMMAND (chase)

BEGIN_COMMAND (idmus)
{
	LevelInfos& levels = getLevelInfos();

	if (argc > 1)
	{
		char *map;
		if (gameinfo.flags & GI_MAPxx)
		{
			const int l = atoi(argv[1]);
			if (l <= 99)
				map = CalcMapName (0, l);
			else
			{
				Printf (PRINT_HIGH, "%s\n", GStrings(STSTR_NOMUS));
				return;
			}
		}
		else
		{
			map = CalcMapName (argv[1][0] - '0', argv[1][1] - '0');
		}

		level_pwad_info_t& info = levels.findByName(map);
		if (level.levelnum != 0)
		{
			if (info.music[0])
			{
				S_ChangeMusic(info.music.c_str(), 1);
				Printf (PRINT_HIGH, "%s\n", GStrings(STSTR_MUS));
			}
		}
		else
		{
			Printf(PRINT_HIGH, "%s\n", GStrings(STSTR_NOMUS));
		}
	}
}
END_COMMAND (idmus)

BEGIN_COMMAND (give)
{
	if (!CHEAT_AreCheatsEnabled())
		return;

	if (argc < 2)
		return;

	const std::string name = C_ArgCombine(argc - 1, (const char**)(argv + 1));
	if (name.length())
	{
		CHEAT_GiveTo(&consoleplayer(), name.c_str());
		CL_SendGiveCheat(name.c_str());
	}
}
END_COMMAND (give)

BEGIN_COMMAND (fov)
{
	if (!CHEAT_AreCheatsEnabled() || !m_Instigator)
		return;

	if (argc != 2)
		Printf(PRINT_HIGH, "FOV is %g\n", m_Instigator->player->fov);
	else
	{
		m_Instigator->player->fov = clamp((float)atof(argv[1]), 45.0f, 135.0f);
		R_ForceViewWindowResize();
	}
}
END_COMMAND (fov)

BEGIN_COMMAND(buddha)
{
	if (!CHEAT_AreCheatsEnabled())
		return;

	CHEAT_DoCheat(&consoleplayer(), CHT_BUDDHA);
	CL_SendCheat(CHT_BUDDHA);
}
END_COMMAND(buddha)

/***************************************************************/
/**				   DOOM STATUS BAR FUNCTIONS				  **/
/***************************************************************/

static int ST_calcPainOffset()
{
	static int lastcalc;
	static int oldhealth = -1;

	const int health = clamp(displayplayer().health, -1, 100);

	if (health != oldhealth)
	{
		lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
		oldhealth = health;
	}

	return lastcalc;
}


static lumpHandle_t LoadFaceGraphic(char* name, int namespc = ns_global)
{
	int lump = W_CheckNumForName(name, namespc);
	if (lump == -1)
	{
		char othername[9];
		strcpy(othername, name);
		othername[0] = 'S';
		othername[1] = 'T';
		othername[2] = 'F';
		lump = W_GetNumForName(othername);
	}
	return W_CachePatchHandle(lump, PU_STATIC);
}


//
// ST_refreshBackground
//
// Draws the status bar background to an off-screen 320x32 buffer.
//
static void ST_refreshBackground()
{
	const IWindowSurface* surface = R_GetRenderingSurface();
	const int surface_width = surface->getWidth();
	const int surface_height = surface->getHeight();

	// [RH] If screen is wider than the status bar, draw stuff around status bar.
	if (surface_width > ST_WIDTH)
	{
		R_DrawBorder(0, ST_Y, ST_X, surface_height);
		R_DrawBorder(surface_width - ST_X, ST_Y, surface_width, surface_height);
	}

	stbar_surface->lock();

	const DCanvas* stbar_canvas = stbar_surface->getDefaultCanvas();
	stbar_canvas->DrawPatch(W_ResolvePatchHandle(sbar), 0, 0);

	if (sv_gametype == GM_CTF)
	{
		stbar_canvas->DrawPatch(W_ResolvePatchHandle(flagsbg), ST_FLAGSBGX, ST_FLAGSBGY);
	}
	else if (G_IsCoopGame())
	{
		stbar_canvas->DrawPatch(W_ResolvePatchHandle(armsbg), ST_ARMSBGX, ST_ARMSBGY);
	}

	if (multiplayer)
	{
		if (!demoplayback)
		{
			// [RH] Always draw faceback with the player's color
			//		using a translation rather than a different patch.
			V_ColorMap = translationref_t(translationtables + displayplayer_id * 256,
			                              displayplayer_id);
			stbar_canvas->DrawTranslatedPatch(W_ResolvePatchHandle(faceback), ST_FX,
			                                  ST_FY);
		}
		else
		{
			stbar_canvas->DrawPatch(
			    W_ResolvePatchHandle(faceclassic[displayplayer_id - 1]), ST_FX, ST_FY);
		}
	}

	stbar_surface->unlock();
}


//
// This is a not-very-pretty routine which handles
//	the face states and their timing.
// the precedence of expressions is:
//	dead > evil grin > turned head > straight ahead
//
void ST_updateFaceWidget()
{
	int 		i;
	static int	lastattackdown = -1;
	static int	priority = 0;

	player_t *plyr = &displayplayer();

	if (priority < 10)
	{
		// dead
		if (!plyr->health)
		{
			priority = 9;
			st_faceindex = ST_DEADFACE;
			st_facecount = 1;
		}
	}

	if (priority < 9)
	{
		if (plyr->bonuscount)
		{
			// picking up bonus
			bool doevilgrin = false;

			for (i = 0; i < NUMWEAPONS; i++)
			{
				if (oldweaponsowned[i] != plyr->weaponowned[i])
				{
					doevilgrin = true;
					oldweaponsowned[i] = plyr->weaponowned[i];
				}
			}
			if (doevilgrin)
			{
				// evil grin if just picked up weapon
				priority = 8;
				st_facecount = ST_EVILGRINCOUNT;
				st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
			}
		}
	}

	if (priority < 8)
	{
		if (plyr->damagecount
			&& plyr->attacker
			&& plyr->attacker != plyr->mo)
		{
			// being attacked
			priority = 7;

			if (st_oldhealth - plyr->health > ST_MUCHPAIN)
			{
				st_facecount = ST_TURNCOUNT;
				st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
			}
			else
			{
				angle_t diffang;
				const angle_t badguyangle = R_PointToAngle2(plyr->mo->x,
				                                            plyr->mo->y,
				                                            plyr->attacker->x,
				                                            plyr->attacker->y);

				if (badguyangle > plyr->mo->angle)
				{
					// whether right or left
					diffang = badguyangle - plyr->mo->angle;
					i = diffang > ANG180;
				}
				else
				{
					// whether left or right
					diffang = plyr->mo->angle - badguyangle;
					i = diffang <= ANG180;
				} // confusing, aint it?

				st_facecount = ST_TURNCOUNT;
				st_faceindex = ST_calcPainOffset();

				if (diffang < ANG45)
				{
					// head-on
					st_faceindex += ST_RAMPAGEOFFSET;
				}
				else if (i)
				{
					// turn face right
					st_faceindex += ST_TURNOFFSET;
				}
				else
				{
					// turn face left
					st_faceindex += ST_TURNOFFSET+1;
				}
			}
		}
	}

	if (priority < 7)
	{
		// getting hurt because of your own damn stupidity
		if (plyr->damagecount)
		{
			if (st_oldhealth - plyr->health > ST_MUCHPAIN)
			{
				priority = 7;
				st_facecount = ST_TURNCOUNT;
				st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
			}
			else
			{
				priority = 6;
				st_facecount = ST_TURNCOUNT;
				st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
			}
		}
	}

	if (priority < 6)
	{
		// rapid firing
		if (plyr->attackdown)
		{
			if (lastattackdown == -1)
				lastattackdown = ST_RAMPAGEDELAY;
			else if (!--lastattackdown)
			{
				priority = 5;
				st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
				st_facecount = 1;
				lastattackdown = 1;
			}
		}
		else
			lastattackdown = -1;
	}

	if (priority < 5)
	{
		// invulnerability
		if ((plyr->cheats & CF_GODMODE) || plyr->powers[pw_invulnerability])
		{
			priority = 4;

			st_faceindex = ST_GODFACE;
			st_facecount = 1;
		}
	}

	// look left or look right if the facecount has timed out
	if (!st_facecount)
	{
		st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
		st_facecount = ST_STRAIGHTFACECOUNT;
		priority = 0;
	}

	st_facecount--;
}

void ST_updateWidgets()
{
	const player_t *plyr = &displayplayer();

	if (weaponinfo[plyr->readyweapon].ammotype == am_noammo)
		st_current_ammo = ST_DONT_DRAW_NUM;
	else
		st_current_ammo = plyr->ammo[weaponinfo[plyr->readyweapon].ammotype];

	w_ready.data = plyr->readyweapon;

	st_health = plyr->health;
	st_armor = plyr->armorpoints;

	for (int i = 0; i < 4; i++)
	{
		st_ammo[i] = plyr->ammo[i];
		st_maxammo[i] = plyr->maxammo[i];
	}

	for (int i = 0; i < 6; i++)
	{
		// denis - longwinded so compiler optimization doesn't skip it (fault in my gcc?)
		if(plyr->weaponowned[i + 1])
			st_weaponowned[i] = 1;
		else
			st_weaponowned[i] = 0;
	}

	// update keycard multiple widgets
	for (int i = 0; i < 3; i++)
	{
		keyboxes[i] = plyr->cards[i] ? i : -1;

		// [RH] show multiple keys per box, too
		if (plyr->cards[i+3])
			keyboxes[i] = (keyboxes[i] == -1) ? i+3 : i + 6;
	}

	// refresh everything if this is him coming back to life
	ST_updateFaceWidget();

	// used by w_arms[] widgets
	st_armson = st_statusbaron && G_IsCoopGame();

	// used by w_frags widget
	st_fragson = !G_IsCoopGame() && st_statusbaron;

	//	[Toke - CTF]
	if (sv_gametype == GM_CTF)
		st_fragscount = GetTeamInfo(plyr->userinfo.team)->Points; // denis - todo - scoring for ctf
	else
		st_fragscount = plyr->fragcount;	// [RH] Just use cumulative total

	if (G_IsLivesGame())
		st_lives = plyr->lives;
	else
		st_lives = ST_DONT_DRAW_NUM;

	// get rid of chat window if up because of message
	if (!--st_msgcounter)
		st_chat = st_oldchat;
}


void ST_Ticker()
{
	if (!multiplayer && !demoplayback && (ConsoleState == c_down || ConsoleState == c_falling))
		return;
	st_randomnumber = M_Random();
	gameinfo.statusBar->Ticker();
	st_oldhealth = displayplayer().health;
}


void ST_drawWidgets(bool force_refresh)
{
	// used by w_arms[] widgets
	st_armson = st_statusbaron && G_IsCoopGame();

	// used by w_frags widget
	st_fragson = !G_IsCoopGame() && st_statusbaron;

	w_ready.update(force_refresh);

	for (int i = 0; i < 4; i++)
	{
		w_ammo[i].update(force_refresh);
		w_maxammo[i].update(force_refresh);
	}

	w_health.update(force_refresh);
	w_armor.update(force_refresh);

	for (int i = 0; i < 6; i++)
		w_arms[i].update(force_refresh);

	w_faces.update(force_refresh);

	for (int i = 0; i < 3; i++)
		w_keyboxes[i].update(force_refresh);

	w_frags.update(force_refresh);

	w_lives.update(true, G_IsLivesGame()); // Force refreshing to avoid tens
		                                                 // to be hidden by Doomguy's face
}


static void ST_HticRefreshBackground();
    //
// ST_Drawer
//
// If st_scale is disabled, the status bar is drawn directly to the rendering
// surface, with stbar_surface (the status bar background) being blitted to the
// rendering surface without scaling and then all of the widgets being drawn
// on top of it.
//
// If st_scale is enabled, the status bar is drawn to an unscaled 320x32 pixel
// off-screen surface stnum_surface. First stbar_surface (the status bar
// background) is blitted to stnum_surface, then the widgets are then drawn
// on top of it. Finally, stnum_surface is blitted onto the rendering surface
// using scaling to match the size in 320x200 resolution.
//
// Now ST_Drawer recalculates the ST_WIDTH, ST_HEIGHT, ST_X, and ST_Y globals.
//
void ST_Drawer()
{
	if (st_needrefresh)
		st_statusbaron = R_StatusBarVisible();

	if (st_statusbaron)
	{
		IWindowSurface* surface = R_GetRenderingSurface();
		const int surface_width = surface->getWidth();
		const int surface_height = surface->getHeight();

		ST_WIDTH = ST_StatusBarWidth(surface_width, surface_height);
		ST_HEIGHT = ST_StatusBarHeight(surface_width, surface_height);

		ST_X = ST_StatusBarX(surface_width, surface_height);
		ST_Y = ST_StatusBarY(surface_width, surface_height);

		stbar_surface->lock();
		stnum_surface->lock();

		if (st_needrefresh)
		{
			// draw status bar background to off-screen buffer then blit to surface
			if (gamemission == heretic)
				ST_HticRefreshBackground();
			else
				ST_refreshBackground();

			if (st_scale)
				stnum_surface->blit(stbar_surface, 0, 0, stbar_surface->getWidth(), stbar_surface->getHeight(),
						0, 0, stnum_surface->getWidth(), stnum_surface->getHeight());
			else
				surface->blit(stbar_surface, 0, 0, stbar_surface->getWidth(), stbar_surface->getHeight(),
						ST_X, ST_Y, ST_WIDTH, ST_HEIGHT);
		}
		
		// refresh all widgets
		gameinfo.statusBar->Drawer();

		if (st_scale)
			surface->blit(stnum_surface, 0, 0, stnum_surface->getWidth(), stnum_surface->getHeight(),
					ST_X, ST_Y, ST_WIDTH, ST_HEIGHT);	

		stbar_surface->unlock();
		stnum_surface->unlock();

		st_needrefresh = false;
	}
}


static void ST_loadGraphics()
{
	char namebuf[9];
	namebuf[8] = 0;

	// Load the numbers, tall and short
	for (int i = 0; i < 10; i++)
	{
		sprintf(namebuf, "STTNUM%d", i);
		tallnum[i] = W_CachePatchHandle(namebuf, PU_STATIC);

		sprintf(namebuf, "STYSNUM%d", i);
		shortnum[i] = W_CachePatchHandle(namebuf, PU_STATIC);
	}

	// Load percent key.
	// Note: why not load STMINUS here, too?
	tallpercent = W_CachePatchHandle("STTPRCNT", PU_STATIC);

	// key cards
	for (int i = 0; i < NUMCARDS + NUMCARDS / 2; i++)
	{
		sprintf(namebuf, "STKEYS%d", i);
		keys[i] = W_CachePatchHandle(namebuf, PU_STATIC);
	}

	// arms background
	armsbg = W_CachePatchHandle("STARMS", PU_STATIC);

	// flags background
	flagsbg = W_CachePatchHandle("STFLAGS", PU_STATIC);

	// arms ownership widgets
	for (int i = 0; i < 6; i++)
	{
		sprintf(namebuf, "STGNUM%d", i + 2);

		// gray #
		arms[i][0] = W_CachePatchHandle(namebuf, PU_STATIC);

		// yellow #
		arms[i][1] = shortnum[i + 2];
	}

	// face backgrounds for different color players
	// [RH] only one face background used for all players
	//		different colors are accomplished with translations
	faceback = W_CachePatchHandle("STFBANY", PU_STATIC);

	// [Nes] Classic vanilla lifebars.
	for (int i = 0; i < 4; i++)
	{
		sprintf(namebuf, "STFB%d", i);
		faceclassic[i] = W_CachePatchHandle(namebuf, PU_STATIC);
	}

	// status bar background bits
	sbar = W_CachePatchHandle("STBAR", PU_STATIC);

	// face states
	int facenum = 0;

	namebuf[0] = 'S';
	namebuf[1] = 'T';
	namebuf[2] = 'F';

	for (int i = 0; i < ST_NUMPAINFACES; i++)
	{
		for (int j = 0; j < ST_NUMSTRAIGHTFACES; j++)
		{
			sprintf(namebuf + 3, "ST%d%d", i, j);
			faces[facenum++] = LoadFaceGraphic(namebuf, ns_global);
		}
		sprintf(namebuf + 3, "TR%d0", i); // turn right
		faces[facenum++] = LoadFaceGraphic(namebuf, ns_global);
		sprintf(namebuf + 3, "TL%d0", i); // turn left
		faces[facenum++] = LoadFaceGraphic(namebuf, ns_global);
		sprintf(namebuf + 3, "OUCH%d", i); // ouch!
		faces[facenum++] = LoadFaceGraphic(namebuf, ns_global);
		sprintf(namebuf + 3, "EVL%d", i); // evil grin ;)
		faces[facenum++] = LoadFaceGraphic(namebuf, ns_global);
		sprintf(namebuf + 3, "KILL%d", i); // pissed off
		faces[facenum++] = LoadFaceGraphic(namebuf, ns_global);
	}
	strcpy(namebuf + 3, "GOD0");
	faces[facenum++] = LoadFaceGraphic(namebuf, ns_global);
	strcpy(namebuf + 3, "DEAD0");
	faces[facenum] = LoadFaceGraphic(namebuf, ns_global);
}

static void ST_unloadGraphics()
{
	// unload the numbers, tall and short
	for (int i = 0; i < 10; i++)
	{
		::tallnum[i].clear();
		::shortnum[i].clear();
	}

	// unload tall percent
	::tallpercent.clear();

	// unload arms background
	::armsbg.clear();

	// unload flags background
	::flagsbg.clear();

	// unload gray #'s
	for (int i = 0; i < 6; i++)
	{
		::arms[i][0].clear();
	}

	// unload the key cards
	for (int i = 0; i < NUMCARDS + NUMCARDS / 2; i++)
	{
		::keys[i].clear();
	}

	::sbar.clear();
	::faceback.clear();

	for (int i = 0; i < ST_NUMFACES; i++)
	{
		::faces[i].clear();
	}

	::negminus.clear();
}

static void ST_unloadData()
{
	// todo - heretic unloading
	ST_unloadGraphics();
	ST_unloadNew();
}


void ST_createWidgets()
{
	// ready weapon ammo
	w_ready.init(ST_AMMOX, ST_AMMOY, tallnum, &st_current_ammo, &st_statusbaron,
	             ST_AMMOWIDTH);

	// health percentage
	w_health.init(ST_HEALTHX, ST_HEALTHY, tallnum, &st_health, &st_statusbaron,
	              tallpercent);

	// weapons owned
	for (int i = 0; i < 6; i++)
	{
		w_arms[i].init(ST_ARMSX + (i % 3) * ST_ARMSXSPACE,
		               ST_ARMSY + (i / 3) * ST_ARMSYSPACE, arms[i], &st_weaponowned[i],
		               &st_armson);
	}

	// frags sum
	w_frags.init(ST_FRAGSX, ST_FRAGSY, tallnum, &st_fragscount, &st_fragson,
	             ST_FRAGSWIDTH);

	// faces
	w_faces.init(ST_FACESX, ST_FACESY, faces, &st_faceindex, &st_statusbaron);

	// armor percentage - should be colored later
	w_armor.init(ST_ARMORX, ST_ARMORY, tallnum, &st_armor, &st_statusbaron, tallpercent);

	// keyboxes 0-2
	w_keyboxes[0].init(ST_KEY0X, ST_KEY0Y, keys, &keyboxes[0], &st_statusbaron);

	w_keyboxes[1].init(ST_KEY1X, ST_KEY1Y, keys, &keyboxes[1], &st_statusbaron);

	w_keyboxes[2].init(ST_KEY2X, ST_KEY2Y, keys, &keyboxes[2], &st_statusbaron);

	// ammo count (all four kinds)
	w_ammo[0].init(ST_AMMO0X, ST_AMMO0Y, shortnum, &st_ammo[0], &st_statusbaron,
	               ST_AMMO0WIDTH);

	w_ammo[1].init(ST_AMMO1X, ST_AMMO1Y, shortnum, &st_ammo[1], &st_statusbaron,
	               ST_AMMO1WIDTH);

	w_ammo[2].init(ST_AMMO2X, ST_AMMO2Y, shortnum, &st_ammo[2], &st_statusbaron,
	               ST_AMMO2WIDTH);

	w_ammo[3].init(ST_AMMO3X, ST_AMMO3Y, shortnum, &st_ammo[3], &st_statusbaron,
	               ST_AMMO3WIDTH);

	// max ammo count (all four kinds)
	w_maxammo[0].init(ST_MAXAMMO0X, ST_MAXAMMO0Y, shortnum, &st_maxammo[0],
	                  &st_statusbaron, ST_MAXAMMO0WIDTH);

	w_maxammo[1].init(ST_MAXAMMO1X, ST_MAXAMMO1Y, shortnum, &st_maxammo[1],
	                  &st_statusbaron, ST_MAXAMMO1WIDTH);

	w_maxammo[2].init(ST_MAXAMMO2X, ST_MAXAMMO2Y, shortnum, &st_maxammo[2],
	                  &st_statusbaron, ST_MAXAMMO2WIDTH);

	w_maxammo[3].init(ST_MAXAMMO3X, ST_MAXAMMO3Y, shortnum, &st_maxammo[3],
	                  &st_statusbaron, ST_MAXAMMO3WIDTH);

	// Number of lives (not always rendered)
	w_lives.init(ST_FX + 34, ST_FY + 25, shortnum, &st_lives, &st_statusbaron, 2);
}

void ST_DoomDrawer()
{
	ST_drawWidgets(st_needrefresh);
}

void ST_DoomTicker()
{
	ST_updateWidgets();
}

void ST_DoomStart()
{
	st_faceindex = 0;

	for (int i = 0; i < NUMWEAPONS; i++)
		oldweaponsowned[i] = consoleplayer().weaponowned[i];

	ST_createWidgets();
	st_needrefresh = true;
}

void ST_DoomInit()
{
	ST_loadGraphics();
}

/***************************************************************/
/**				  HERETIC STATUS BAR FUNCTIONS				  **/
/***************************************************************/

void ST_HticCreateWidgets();

//
// ST_chainShadow
//
// Draws two 16x10 shaded areas on the ends of the life chain, using
// the light-fading colormaps to darken what's already been drawn.
//
void ST_HticShadeChain(int left, int right, int top, int height)
{
	int i;
	int pitch;
	int diff;
	byte* top_p;

	/*
	if (st_scale)
	{
		pitch = BG->pitch;
		top_p = BG->buffer;
	}
	else
	{
		top += ST_Y;
		left += ST_X;
		right += ST_X;
		pitch = FG->pitch;
		top_p = FG->buffer;
	}

	top_p += top * pitch + left;
	diff = right + 15 - left;

	for (i = 0; i < 16; i++)
	{
		unsigned int* darkener = Col2RGB8[18 + i * 2];
		int h = height;
		byte* dest = top_p;
		do
		{
			unsigned int lbg = darkener[*dest] | 0x1f07c1f;
			unsigned int rbg = darkener[*(dest + diff)] | 0x1f07c1f;
			*dest = RGB32k[0][0][lbg & (lbg >> 15)];
			*(dest + diff) = RGB32k[0][0][rbg & (rbg >> 15)];
			dest += pitch;
		} while (--h);
		top_p++;
		diff -= 2;
	}

	STlib_scaleRect(left, top, 16, height);
	STlib_scaleRect(right, top, 16, height);*/
}

//
// ST_HticDrawChainWidget
//
// Draws the chain & gem health indicator at the bottom
//
void ST_HticDrawChainWidget()
{
	const player_t* plyr = &consoleplayer();
	int y = 32;
	int chainpos = clamp(chainhealth, 0, 100);

	// the total length between the left- and right-most gem
	// positions is 256 pixels, so scale the chainpos by that
	// amount (gem can range from 17 to 273)
	chainpos = (chainpos << 8) / 100;

	// jiggle y coordinate when chain is moving
	if (plyr->health != chainhealth)
		y += chainwiggle;

	const DCanvas* stbar_canvas = stbar_surface->getDefaultCanvas();
	// draw the chain -- links repeat every 17 pixels, so we
	// wrap the chain back to the starting position every 17
	stbar_canvas->DrawPatch(W_ResolvePatchHandle(chain), 2 + (chainpos % 17), y);

	// draw the gem (17 is the far left pos., 273 is max)
	// TODO: fix life gem for multiplayer modes
	stbar_canvas->DrawPatch(W_ResolvePatchHandle(lifegem), 17 + chainpos, y);
}

static void ST_HticDrawWidgets(bool refresh)
{
	w_numhealth.update(refresh);
	w_ready.update(refresh);
	w_numarmor.update(refresh);

	if (sv_gametype != GM_CTF) // [Toke - CTF] Dont display keys in ctf mode
		for (int i = 0; i < 3; i++)
			w_keyboxes[i].update(refresh);

	ST_HticDrawChainWidget();
}

static void ST_HticRefreshBackground()
{
	const IWindowSurface* surface = R_GetRenderingSurface();
	const int surface_width = surface->getWidth();
	const int surface_height = surface->getHeight();

	// [RH] If screen is wider than the status bar,
	//      draw stuff around status bar.
	if (surface_width > ST_WIDTH)
	{
		R_DrawBorder(0, ST_Y, ST_X, surface_height);
		R_DrawBorder(surface_width - ST_X, ST_Y, surface_width, surface_height);
	}

	stbar_surface->lock();

	const DCanvas* stbar_canvas = stbar_surface->getDefaultCanvas();
	stbar_canvas->DrawPatch(W_ResolvePatchHandle(sbar_back), 0, 0);

	// TODO: Inventory bar
	stbar_canvas->DrawPatch(W_ResolvePatchHandle(statbar), 34, 2);

	const int st_edgeheight = 200 - gameinfo.statusBar->height - W_ResolvePatchHandle(sbar_topleft)->height() + 1;
	
	// draw the tops of the faces
	stbar_canvas->DrawPatchIndirect(W_ResolvePatchHandle(sbar_topleft), 0, st_edgeheight);
	stbar_canvas->DrawPatchIndirect(W_ResolvePatchHandle(sbar_topright), 290, st_edgeheight);
	
	ST_HticDrawChainWidget();
	
	// draw face patches to cover over spare ends of chain
	stbar_canvas->DrawPatch(W_ResolvePatchHandle(ltface), 0, 32);
	stbar_canvas->DrawPatch(W_ResolvePatchHandle(rtface), 276, 32);

	player_t* plyr = &consoleplayer();
	if ((plyr->cheats & CF_GODMODE) || plyr->powers[pw_invulnerability])
	{
		stbar_canvas->DrawPatch(W_ResolvePatchHandle(godeyes_left), 16, 9);
		stbar_canvas->DrawPatch(W_ResolvePatchHandle(godeyes_right), 287, 9);
	}

	ST_HticShadeChain(19, 277, 32, 10);
	/*
	BG->Blit(0, 0, 320, gameinfo.StatusBar->height, stnum_screen, 0, 0, 320,
	         gameinfo.StatusBar->height);
	*/

	stbar_surface->unlock();
}

//
// ST_HticLoadGraphics
//
void ST_HticLoadGraphics()
{
	const std::string bignums = "IN%d";
	const std::string smallnums = "SMALLIN%d";

	// Load the numbers, tall and short
	for (int i = 0; i < 10; i++)
	{
		char namebuf[9];
		sprintf(namebuf, bignums.c_str(), i);
		tallnum[i] = W_CachePatchHandle(namebuf, PU_STATIC);
		sprintf(namebuf, smallnums.c_str(), i);
		shortnum[i] = W_CachePatchHandle(namebuf, PU_STATIC);
	}

	lamenum = W_CachePatch("LAME");

	keys[0] = W_CachePatchHandle("YKEYICON", PU_STATIC);
	keys[1] = W_CachePatchHandle("GKEYICON", PU_STATIC);
	keys[2] = W_CachePatchHandle("BKEYICON", PU_STATIC);

	negminus = W_CachePatchHandle("NEGNUM", PU_STATIC);

	// update the status bar patch for the appropriate game mode
	statbar = W_CachePatchHandle((multiplayer ? "STATBAR" : "LIFEBAR"));

	sbar_back = W_CachePatchHandle("BARBACK", PU_STATIC);
	godeyes_left = W_CachePatchHandle("GOD1", PU_STATIC);
	godeyes_right = W_CachePatchHandle("GOD2", PU_STATIC);
	sbar_topleft = W_CachePatchHandle("LTFCTOP", PU_STATIC);
	sbar_topright = W_CachePatchHandle("RTFCTOP", PU_STATIC);

	chain = W_CachePatchHandle("CHAIN", PU_STATIC);
	lifegem = W_CachePatchHandle("LIFEGEM2", PU_STATIC);
	ltface = W_CachePatchHandle("LTFACE", PU_STATIC);
	rtface = W_CachePatchHandle("RTFACE", PU_STATIC);
}

void ST_HticCreateWidgets()
{
	// health percentage
	w_numhealth.init(87, 12, tallnum, &st_health, &st_statusbaron, 3);

	// ready weapon ammo
	w_ready.init(135, 4, tallnum, &st_current_ammo, &st_statusbaron, ST_AMMOWIDTH);

	// armor percentage - should be colored later
	w_numarmor.init(254, 12, tallnum, &st_armor, &st_statusbaron, 3);

	// keyboxes 0-2
	w_keyboxes[0].init(153, 6, keys, &keyboxes[0], &st_statusbaron);
	w_keyboxes[1].init(153, 14, keys, &keyboxes[1], &st_statusbaron);
	w_keyboxes[2].init(153, 22, keys, &keyboxes[2], &st_statusbaron);
}

void ST_HticUpdateWidgets()
{
	static int largeammo = ST_DONT_DRAW_NUM;
	player_t* plyr = &consoleplayer();

	if (weaponinfo[plyr->readyweapon].ammotype == am_noammo)
		w_ready.num = &largeammo;
	else
		w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammotype];

	w_ready.data = plyr->readyweapon;

	// update the chain health value
	st_health = plyr->health;
	st_armor = plyr->armorpoints;

	if (st_health != chainhealth)
	{
		int max, min, sgn = 1;

		// set lower bound to zero
		if (st_health < 0)
			st_health = 0;

		// determine the max and min of the two values, and whether or
		// not to add or subtract from the chain position w/sgn
		if (st_health > chainhealth)
		{
			max = st_health;
			min = chainhealth;
		}
		else
		{
			sgn = -1;
			max = chainhealth;
			min = st_health;
		}

		// consider 1/4th of the difference
		int diff = (max - min) >> 2;

		// don't move less than 1 or more than 8 units per tic
		if (diff < 1)
			diff = 1;
		else if (diff > 8)
			diff = 8;

		chainhealth += (sgn * diff);
	}

	// update chain wiggle value
	if (level.time & 1)
		chainwiggle = st_randomnumber & 1;

	// update keycard multiple widgets
	for (int i = 0; i < 3; i++)
	{
		keyboxes[i] = plyr->cards[i] ? i : -1;

		// [RH] show multiple keys per box, too
		// if (plyr->cards[i+3])
		//	keyboxes[i] = (keyboxes[i] == -1) ? i+3 : i+6;
	}
}

//
// ST_HticStart
//
void ST_HticStart()
{
	ST_HticCreateWidgets();

	st_needrefresh = true;
}

//
// ST_HticTicker
//
// Processing code for Heretic status bar
//
void ST_HticTicker()
{
	ST_HticUpdateWidgets();
}

//
// ST_HticDrawer
//
// Draws the Heretic status bar
//
void ST_HticDrawer()
{
	ST_HticDrawWidgets(st_needrefresh);
}

//
// ST_HticInit
//
// Initializes the Heretic status bar:
// * Caches most patch graphics used throughout
//
void ST_HticInit()
{
	ST_HticLoadGraphics();
}

/***************************************************************/
/**				   COMMON STATUS BAR FUNCTIONS				  **/
/***************************************************************/

void ST_Start()
{
	if (!st_stopped)
		ST_Stop();

	ST_ForceRefresh();

	st_gamestate = FirstPersonState;

	st_statusbaron = true;
	st_oldchat = st_chat = false;
	st_cursoron = false;

	st_oldhealth = -1;

	for (int i = 0; i < NUMWEAPONS; i++)
		oldweaponsowned[i] = displayplayer().weaponowned[i];

	for (int i = 0; i < 3; i++)
		keyboxes[i] = -1;

	negminus =
	    W_CachePatchHandle((gamemission == heretic) ? "NEGNUM" : "STTMINUS", PU_STATIC);

	ST_initNew();
	gameinfo.statusBar->Start();
	
	st_stopped = false;
}

void ST_Stop()
{
	if (st_stopped)
		return;

	st_stopped = true;
}

void ST_Init()
{
	if (stbar_surface == NULL)
		stbar_surface = I_AllocateSurface(320, (gamemission == heretic) ? 42 : 32, 8);
	if (stnum_surface == NULL)
		stnum_surface = I_AllocateSurface(320, (gamemission == heretic) ? 42 : 32, 8);

	lu_palette = W_GetNumForName("PLAYPAL");

	gameinfo.statusBar->Init();
}


void STACK_ARGS ST_Shutdown()
{
	ST_unloadData();

	I_FreeSurface(stbar_surface);
	I_FreeSurface(stnum_surface);
}


VERSION_CONTROL (st_stuff_cpp, "$Id$")
