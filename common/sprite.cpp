#include "odamex.h"

#include <stdlib.h>
#include <string.h>

#include "sprite.h"
#include "doom_obj_container.h"

#include <sstream>

//----------------------------------------------------------------------------------------------

template<>
inline void DoomObjectContainer<const char*, spritenum_t>::clear()
{
	if (this->container != nullptr)
	{
		for (int i = 0; i < this->num_types; i++)
		{
			char* p = (char*)this->container[i];
			M_Free(p);
		}
	}
}

//----------------------------------------------------------------------------------------------

// global variables from info.h

DoomObjectContainer<const char*, spritenum_t> sprnames(::NUMSPRITES);
size_t num_spritenum_t_types()
{
	return sprnames.capacity();
}

void D_Initialize_sprnames(const char** source, int count)
{
	sprnames.clear();
	sprnames.resize(count + 1); // make space for ending NULL (other parts of code depend on this)
    if (source)
    {
		for (int i = 0; i < count; i++)
		{
			sprnames[i] = strdup(source[i]);
		}
    }
	sprnames[count] = NULL;
	// [CMB] Useful debug logging
#if defined _DEBUG
	Printf(PRINT_HIGH, "D_Allocate_sprnames:: allocated %d sprites.\n", count);
#endif
}

/**
 * @brief find the index for a given key
 * @param src_sprnames the sprite names to search for the index
 * @param key the key for a sprite either as a 4 character string or index value
 *            if it is not either, return -1.
 *            if the key is not found, return -1.
 * @param offset the offset for the odamex sprnames
 */
int D_FindOrgSpriteIndex(const char** src_sprnames, const char* key)
{
	int i = 0;
    // search the array for the sprite name you are looking for
	for (; src_sprnames[i]; ++i)
    {
        if(!strncmp(src_sprnames[i], key, 4))
        {
            return i;
        }
    }
    // check if this is an actual number - we aren't validating this index here
    std::istringstream stream(key);
    int spridx;
    bool ok = !(stream >> spridx).fail();
    return ok ? spridx : -1;
}

/**
 * @brief ensure the sprnames array of sprite names has the correct capacity
 *
 * @param limit the new size for sprnames. This will realloc and zero beyond the current maximum.
 */
void D_EnsureSprnamesCapacity(int limit)
{
	int newCapacity = sprnames.capacity();
	while (limit >= newCapacity)
	{
		newCapacity *= 2;
	}
	sprnames.resize(newCapacity);
}
