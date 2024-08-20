#include "doomdef.h"
#include "r_local.h"

void *R_LoadPixels(int lumpnum);

void R_Cache(void)
{
    viswall_t *wall;
    vissprite_t *spr;

    for (wall = viswalls; wall < lastwallcmd; wall++)
    {
        if ((wall->actionbits & AC_TOPTEXTURE) != 0 && (int)wall->t_texture->data < 4096)
        {
            wall->t_texture->data = R_LoadPixels((int)wall->t_texture->data);
        }
        if ((wall->actionbits & AC_BOTTOMTEXTURE) != 0 && (int)wall->b_texture->data < 4096)
        {
            wall->b_texture->data = R_LoadPixels((int)wall->b_texture->data);
        }
        if ((int)wall->floorpic < 4096)
        {
            wall->floorpic = R_LoadPixels((int)wall->floorpic);
        }
        if ((int)wall->ceilingpic == -1)
        {
            if ((int)skytexturep->data < 4096)
            {
                skytexturep->data = R_LoadPixels((int)skytexturep->data);
            }
        }
        else if ((int)wall->ceilingpic < 4096)
        {
            wall->ceilingpic = R_LoadPixels((int)wall->ceilingpic);
        }
    }

    for (spr = vissprites; spr < vissprite_p; spr++)
    {
        if ((int)spr->pixels < 4096)
        {
            spr->pixels = R_LoadPixels((int)spr->pixels);
        }
    }

    phasetime[5] = samplecount;
}

void *R_LoadPixels(int lumpnum)
{
    return W_POINTLUMPNUM(lumpnum);
}
