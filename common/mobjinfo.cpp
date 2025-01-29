#include "odamex.h"

#include "mobjinfo.h"
#include "doom_obj_container.h"
#include "z_zone.h"
#include "p_local.h"

void D_ResetMobjInfo(mobjinfo_t* m, int32_t idx);
void D_ResetMobjInfoSpawnMap(mobjinfo_t* m, int32_t idx);

DoomObjectContainer<mobjinfo_t*> mobjinfo(::NUMMOBJTYPES, &D_ResetMobjInfo);
DoomObjectContainer<mobjinfo_t*, int> spawn_map;

size_t num_mobjinfo_types()
{
	return mobjinfo.size();
}


void D_ResetMobjInfo(mobjinfo_t* m, int32_t idx)
{
    m->droppeditem = MT_NULL;
    m->infighting_group = IG_DEFAULT;
    m->projectile_group = PG_DEFAULT;
    m->splash_group = SG_DEFAULT;
    m->altspeed = NO_ALTSPEED;
    m->meleerange = MELEERANGE;
	m->translucency = 0x10000;
	m->altspeed = NO_ALTSPEED;
	m->ripsound = "";
}

void D_Initialize_Mobjinfo(mobjinfo_t* source, int count)
{

	// initialize the associative array
	mobjinfo.clear();
    mobjinfo.reserve(count);
	if (source)
	{

		mobjinfo_t mobjinfo_source;
		int32_t idx = 0;
		for(int i = 0; i < count; ++i)
		{
			mobjinfo_source = source[i];
			idx = mobjinfo_source.type;
			mobjinfo_t* newmobjinfo = (mobjinfo_t*) Z_Malloc(sizeof(mobjinfo_t), PU_STATIC, NULL);
			*newmobjinfo = mobjinfo_source;

			mobjinfo.insert(newmobjinfo, idx);

			// [CMB] -1 is used as the placeholder for now; id24 allows negative range for doomednum
			if (mobjinfo_source.doomednum != -1)
			{
				mobjinfo_t* spawn_info = mobjinfo.find(idx)->second;
				spawn_map.insert(spawn_info, mobjinfo_source.doomednum);
			}
		}
	}
#if defined _DEBUG
    Printf(PRINT_HIGH,"D_Allocate_mobjinfo:: allocated %d actors.\n", count);
#endif
}
