// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2006-2025 by The Odamex Team.
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
//   OPTIONS lump parser
//
//-----------------------------------------------------------------------------

#include "odamex.h"

#include "oscanner.h"
#include "w_wad.h"

typedef void (*OptionTypeFunctionPtr)(OScanner&, int*, unsigned int);

struct mbf21_option_t
{
    const char* name;
    OptionTypeFunctionPtr fn;
    int* data;
    unsigned int flags = 0;
};

void OptionTypeBool(OScanner& os, int* data, unsigned int flag)
{
    os.mustScanInt();
    // just treat non-zero as true
    if (os.getTokenInt())
        *data |= flag;
    else
        *data &= ~flag;
}

void OptionTypeInt(OScanner& os, int* data, unsigned int max)
{
    os.mustScanInt();
    *data = clamp(os.getTokenInt(), 0, (int) max);
}

static constexpr int NUM_MBF21_OPTIONS = 36;

static constexpr std::array<mbf21_option_t, NUM_MBF21_OPTIONS> options = {{
    { "weapon_recoil", &OptionTypeBool, nullptr },
    { "monsters_remember", &OptionTypeBool, nullptr },
    { "monster_infighting", &OptionTypeBool, nullptr },
    { "monster_backing", &OptionTypeBool, nullptr },
    { "monster_avoid_hazards", &OptionTypeBool, nullptr },
    { "monkeys", &OptionTypeBool, nullptr },
    { "monster_friction", &OptionTypeBool, nullptr },
    { "help_friends", &OptionTypeBool, nullptr },
    { "player_helpers", &OptionTypeInt, nullptr, 3 },
    { "friend_distance", &OptionTypeInt, nullptr, 999 },
    { "dog_jumping", &OptionTypeBool, nullptr },
    { "comp_telefrag", &OptionTypeBool, nullptr },
    { "comp_dropoff", &OptionTypeBool, nullptr },
    { "comp_vile", &OptionTypeBool, nullptr },
    { "comp_pain", &OptionTypeBool, nullptr },
    { "comp_skull", &OptionTypeBool, nullptr },
    { "comp_blazing", &OptionTypeBool, nullptr },
    { "comp_doorlight", &OptionTypeBool, nullptr },
    { "comp_model", &OptionTypeBool, nullptr },
    { "comp_god", &OptionTypeBool, nullptr },
    { "comp_fallof", &OptionTypeBool, nullptr },
    { "comp_floors", &OptionTypeBool, nullptr },
    { "comp_skymap", &OptionTypeBool, nullptr },
    { "comp_pursuit", &OptionTypeBool, nullptr },
    { "comp_doorstuck", &OptionTypeBool, nullptr },
    { "comp_staylift", &OptionTypeBool, nullptr },
    { "comp_zombie", &OptionTypeBool, nullptr },
    { "comp_stairs", &OptionTypeBool, nullptr },
    { "comp_infcheat", &OptionTypeBool, nullptr },
    { "comp_zerotags", &OptionTypeBool, nullptr },
    { "comp_respawn", &OptionTypeBool, nullptr },
    { "comp_soul", &OptionTypeBool, nullptr },
    { "comp_ledgeblock", &OptionTypeBool, nullptr },
    { "comp_friendlyspawn", &OptionTypeBool, nullptr },
    { "comp_voodooscroller", &OptionTypeBool, nullptr },
    { "comp_reservedlineflag", &OptionTypeBool, nullptr }
}};

void G_ParseOptions()
{
    int lump = -1;
	while ((lump = W_FindLump("OPTIONS", lump)) != -1)
	{
		char* buffer = static_cast<char*>(W_CacheLumpNum(lump, PU_CACHE));

		const OScannerConfig config = {
		    "OPTIONS", // lumpName
		    true,      // semiComments
		    false,     // cComments
		};
		OScanner os = OScanner::openBuffer(config, buffer, buffer + W_LumpLength(lump));

        while (os.scan())
        {
            auto it = options.begin();
            for (; it != options.end(); it++)
            {
                if (os.compareTokenNoCase(it->name))
                {
                    if (it->data == nullptr)
                        os.warning("Unimplemented OPTIONS key \"%s\"", os.getToken());
                    else
                        it->fn(os, it->data, it->flags);
                    break;
                }
            }

            if (it == options.end())
            {
                os.warning("Unknown OPTIONS key \"%s\"", os.getToken());
                // skip the value
                os.scan();
            }
        }
	}
}