#include "odamex.h"

#include <stdlib.h>
#include <string.h>

#include "sprite.h"

#include <sstream>

// global variables from info.h

const char** sprnames;
int num_spritenum_t_types;

void D_Initialize_sprnames(const char** source, int count)
{
    // [CMB] Pre-allocate sprnames to support current limits on types
    sprnames = (const char**) M_Calloc(count + 1, sizeof(*sprnames));
    for(int i = 0; i < count; i++)
    {
        // [CMB] this is important: we are setting each array slot in sprnames to a statically allocated cons char*
        // during doom initialization. This saves us a free on indices that already exist. For newly allocated
        // sprites, that pointer exists until program termination.
        sprnames[i] = source[i];
    }
    sprnames[count] = NULL;
    num_spritenum_t_types = count;
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
    // search the array for the sprite name you are looking for
    for(int i = 0; i < num_spritenum_t_types; i++)
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
    while(limit >= num_spritenum_t_types)
    {
        int old_num_spritenum_t_types = num_spritenum_t_types;
        num_spritenum_t_types *= 2;
        sprnames = (const char**) M_Realloc(sprnames, num_spritenum_t_types * sizeof(*sprnames));
        // memset to 0 because the end of the elements in the array needs to NULL terminated
        memset(sprnames + old_num_spritenum_t_types, 0, (num_spritenum_t_types - old_num_spritenum_t_types) * sizeof(*sprnames));
    }
}
