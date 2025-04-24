// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2006 by Randy Heit (ZDoom).
// Copyright (C) 2006-2024 by The Odamex Team.
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
//	ACS interpreter using DavidPH's ACSVM
//
//-----------------------------------------------------------------------------

#include "odamex.h"

#include "p_acsinterpreter.h"
#include "w_wad.h"
#include "i_system.h"

#include "ACSVM/Action.hpp"
#include "ACSVM/BinaryIO.hpp"
#include "ACSVM/Code.hpp"
#include "ACSVM/CodeData.hpp"
#include "ACSVM/Error.hpp"
#include "ACSVM/Module.hpp"
#include "ACSVM/Scope.hpp"
#include "ACSVM/Script.hpp"
#include "ACSVM/Serial.hpp"

namespace ACS
{

enum
{
	SCRIPT_CLOSED		= 0,
	SCRIPT_OPEN			= 1,
	SCRIPT_RESPAWN		= 2,
	SCRIPT_DEATH		= 3,
	SCRIPT_ENTER		= 4,
	SCRIPT_PICKUP		= 5,
	SCRIPT_T1RETURN		= 6,
	SCRIPT_T2RETURN		= 7,
	SCRIPT_LIGHTNING	= 12,
	SCRIPT_DISCONNECT	= 14,
};

ACSEnv::ACSEnv() : ACSVM::Environment()
{
	addCodeDataACS0(86, {"", 0, addCallFunc(CF_EndPrint)});
}

ACSVM::ModuleName ACSEnv::getModuleName(const OLumpName& name)
{
	size_t lumpnum = W_GetNumForName(name);
	return { nullptr, nullptr, lumpnum };
}

void ACSEnv::loadModule(ACSVM::Module* module)
{
	size_t lumpnum = module->name.i;
	byte* data = static_cast<byte*>(W_CacheLumpNum(lumpnum, PU_LEVACS));
	module->readBytecode(data, W_LumpLength(lumpnum));
}

void ACSInit(ACSEnv &env, nonstd::span<OLumpName> names)
{
	// Load modules.
	std::vector<ACSVM::Module *> modules;
	for(const OLumpName& name : names)
		modules.push_back(env.getModule(env.getModuleName(name)));

	// Create and activate scopes.
	ACSVM::GlobalScope *global = env.getGlobalScope(0);  global->active = true;
	// note: elsewhere in odamex this is referred to as world scope
	ACSVM::HubScope    *hub    = global->getHubScope(0); hub   ->active = true;
	ACSVM::MapScope    *map    = hub->getMapScope(0);    map   ->active = true;

	// Register modules with map scope.
	map->addModules(modules.data(), modules.size());

	// Start Open scripts.
	map->scriptStartType(SCRIPT_OPEN, {});
}

} // namespace ACS