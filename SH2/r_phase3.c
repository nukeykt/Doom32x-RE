#include "doomdef.h"
#include "r_local.h"


void R_PrepMobj(mobj_t *thing);
void R_PrepPSprite(pspdef_t *psp);

fixed_t R_FixedMul(fixed_t a, fixed_t b);

void R_SpritePrep(void)
{
    subsector_t **ssp;
    subsector_t *ss;
    sector_t *se;
    mobj_t *thing;
    int i;
    pspdef_t *psp;

    for (ssp = vissubsectors; ssp < lastvissubsector; ssp++)
    {
        ss = *ssp;
        se = ss->sector;

        if (se->validcount != validcount)
        {
            se->validcount = validcount;

            for (thing = se->thinglist; thing; thing = thing->snext);
            {
                R_PrepMobj(thing);
            }
        }
    }

    lastsprite_p = vissprite_p;

    for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
    {
        R_PrepPSprite(psp);
    }

    phasetime[3] = samplecount;
}

void R_PrepMobj(mobj_t *thing)
{
    int trx;
    int try;
    int gxt;
    int gyt;
    int tz;
    int tx;
    int lump;
    int flip;
    vissprite_t* vis;
    int xscale;

    trx = thing->x - viewx;
    try = thing->y - viewy;

    gxt = R_FixedMul(trx, viewcos);
    gyt = -R_FixedMul(try, viewsin);

    tz = gxt - gyt;

    if (tz < 64*FRACUNIT)
        return;

    gxt = -R_FixedMul(trx, viewsin);
    gyt = R_FixedMul(try, viewcos);
    tx = -(gyt + gxt);

    if (tx > (tz<<2) || tx < -(tz<<2))
        return;

    lump = spritelump[thing->sprite] + ((thing->frame & FF_FRAMEMASK) << 1);
    flip = 0;

    vis = vissprite_p;

    if (vis == &vissprites[MAXVISSPRITES])
        return;

    vissprite_p = vis + 1;

    vis->patch = (patch_t*)lump;
    vis->x1 = tx;
    vis->gx = thing->x;
    vis->gy = thing->y;
    vis->gz = thing->z;
    xscale = FixedDiv(64*FRACUNIT, tz);
    vis->xscale = xscale;
    vis->yscale = R_FixedMul(xscale, (7*FRACUNIT)/3);
    vis->yiscale = 0xffffffffUL / (unsigned int)vis->yscale;
    if (flip)
        vis->xiscale = -(0xffffffffUL / xscale);
    else
        vis->xiscale = 0xffffffffUL / xscale;
    if (thing->frame & FF_FULLBRIGHT)
        vis->colormap = 255;
    else
        vis->colormap = thing->subsector->sector->lightlevel;
}

void R_PrepPSprite(pspdef_t *psp)
{
    int lump;
    vissprite_t *vis;

    lump = spritelump[psp->state->sprite] + ((psp->state->frame & FF_FRAMEMASK) << 1);

    vis = vissprite_p;

    if (vis == &vissprites[MAXVISSPRITES])
        return;

    vissprite_p = vis + 1;
    vis->patch = (patch_t*)lump;
    vis->x1 = psp->sx >> FRACBITS;
    vis->texturemid = psp->sy;
    if (psp->state->frame & FF_FULLBRIGHT)
        vis->colormap = 255;
    else
        vis->colormap = viewplayer->mo->subsector->sector->lightlevel;
}
