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

#include "ACSVM/Action.hpp"
#include "ACSVM/BinaryIO.hpp"
#include "ACSVM/Code.hpp"
#include "ACSVM/CodeData.hpp"
#include "ACSVM/Error.hpp"
#include "ACSVM/Module.hpp"
#include "ACSVM/Scope.hpp"
#include "ACSVM/Script.hpp"
#include "ACSVM/Serial.hpp"

void ACSEnv::loadModule(ACSVM::Module* module)
{

}

void ACSInit(ACSVM::Environment &env, char const *const *namev, std::size_t namec)
{
	// Load modules.
	std::vector<ACSVM::Module *> modules;
	for(std::size_t i = 1; i < namec; ++i)
		modules.push_back(env.getModule(env.getModuleName(namev[i])));

	// Create and activate scopes.
	ACSVM::GlobalScope *global = env.getGlobalScope(0);  global->active = true;
	ACSVM::HubScope    *hub    = global->getHubScope(0); hub   ->active = true;
	ACSVM::MapScope    *map    = hub->getMapScope(0);    map   ->active = true;

	// Register modules with map scope.
	map->addModules(modules.data(), modules.size());

	// Start Open scripts.
	map->scriptStartType(1, {});
}