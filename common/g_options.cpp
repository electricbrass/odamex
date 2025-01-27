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

typedef void (*OptionTypeFunctionPtr)(unsigned int, unsigned int*, unsigned int);

void OptionTypeBool(unsigned int value, unsigned int* data, unsigned int flag)
{
    // just treat non-zero as true
    if (value)
        *data |= flag;
    else
        *data &= ~flag;
}

void OptionTypeInt(unsigned int value, unsigned int* data, unsigned int max)
{
    *data = clamp(value, 0u, max);
}

struct mbf21_option_t
{
    const char* name;
    unsigned int* data = nullptr;
    unsigned int flags = 0;
    OptionTypeFunctionPtr fn = &OptionTypeBool;
};

struct OptionsSetter
{
    std::vector<mbf21_option_t> options;
    OptionsSetter(level_pwad_info_t& ref)
    {
        options = {
            { "weapon_recoil" },
            { "monsters_remember" },
            { "monster_infighting" },
            { "monster_backing" },
            { "monster_avoid_hazards" },
            { "monkeys" },
            { "monster_friction" },
            { "help_friends" },
            { "player_helpers", nullptr, 3, &OptionTypeInt },
            { "friend_distance", nullptr, 999, &OptionTypeInt },
            { "dog_jumping" },
            { "comp_telefrag" },
            { "comp_dropoff" },
            { "comp_vile" },
            { "comp_pain", &ref.flags, LEVEL_COMPAT_LIMITPAIN },
            { "comp_skull" },
            { "comp_blazing" },
            { "comp_doorlight" },
            { "comp_model" },
            { "comp_god" },
            { "comp_falloff" },
            { "comp_floors" },
            { "comp_skymap" },
            { "comp_pursuit" },
            { "comp_doorstuck" },
            { "comp_staylift" },
            { "comp_zombie" },
            { "comp_stairs" },
            { "comp_infcheat" },
            { "comp_zerotags" },
            { "comp_respawn" },
            { "comp_soul" },
            { "comp_ledgeblock" },
            { "comp_friendlyspawn" },
            { "comp_voodooscroller" },
            { "comp_reservedlineflag" }
        };
    }
};

static std::vector<std::pair<std::string, uint32_t>> modifiedOptions;

void G_ParseOptions()
{
    modifiedOptions.clear();
    level_pwad_info_t dummy = level_pwad_info_t();
    OptionsSetter setter(dummy);
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
            auto it = setter.options.begin();
            for (; it != setter.options.end(); it++)
            {
                if (os.compareTokenNoCase(it->name))
                {
                    std::string name = os.getToken();
                    os.mustScanInt();
                    modifiedOptions.emplace_back(name, os.getTokenInt());
                    break;
                }
            }

            if (it == setter.options.end())
            {
                os.warning("Unknown OPTIONS key \"%s\"", os.getToken());
                // skip the value
                os.scan();
            }
        }
	}
}