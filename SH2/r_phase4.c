#include "doomdef.h"
#include "r_local.h"


boolean cacheneeded;

angle_t normalangle;

fixed_t hyp;

fixed_t R_FixedMul(fixed_t a, fixed_t b);

void R_FinishWallPrep(viswall_t *wc);
void R_FinishSprite(vissprite_t *vis);
void R_FinishPSprite(vissprite_t *vis);

boolean R_LatePrep(void)
{
    viswall_t *wall;
    vissprite_t *spr;

    cacheneeded = false;

    for (wall = viswalls; wall < lastwallcmd; wall++)
    {
        R_FinishWallPrep(wall);
    }

    for (spr = vissprites; spr < lastsprite_p; spr++)
    {
        R_FinishSprite(spr);
    }

    for (spr = lastsprite_p; spr < vissprite_p; spr++)
    {
        R_FinishPSprite(spr);
    }

    phasetime[4] = samplecount;

    return cacheneeded;
}

void R_FinishSprite(vissprite_t *vis)
{
    int lump;
    patch_t *patch;
    int tx;
    int xscale;
    int x1;
    int x2;

    lump = (int)vis->patch;
    vis->patch = patch = (patch_t*)W_POINTLUMPNUM(lump);
    vis->pixels = (pixel_t*)W_POINTLUMPNUM(lump+1);

    tx = vis->x1;
    xscale = vis->xscale;

    tx -= patch->leftoffset << FRACBITS;

    x1 = (64*FRACUNIT + R_FixedMul(tx, xscale)) >> FRACBITS;

    if (x1 > SCREENWIDTH)
    {
        vis->patch = NULL;
        return;
    }

    tx += patch->width << FRACBITS;

    x2 = (64*FRACUNIT + R_FixedMul(tx, xscale)) >> FRACBITS;

    if (x2 < 0)
    {
        vis->patch = NULL;
        return;
    }

    vis->gzt = vis->gz + (patch->height << FRACBITS);

    vis->texturemid = vis->gzt - viewz;

    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= SCREENWIDTH ? SCREENWIDTH-1 : x2;
    if (vis->xiscale < 0)
        vis->startfrac = (patch->width << FRACBITS) - 1;
    else
        vis->startfrac = 0;

    if (x1 < 0)
        vis->startfrac -= vis->xiscale * x1;
}

void R_FinishPSprite(vissprite_t *vis)
{
    int lump;
    patch_t *patch;
    int x1;
    int x2;

    lump = (int)vis->patch;
    vis->patch = patch = (patch_t*)W_POINTLUMPNUM(lump);
    vis->pixels = (pixel_t*)W_POINTLUMPNUM(lump+1);

    vis->texturemid = 100*FRACUNIT - (vis->texturemid - (patch->topoffset<<FRACBITS));
    
    x1 = vis->x1 - patch->leftoffset;

    if (x1 > SCREENWIDTH)
        return;

    x2 = x1 + patch->width - 1;

    if (x2 < 0)
        return;

    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= SCREENWIDTH ? SCREENWIDTH-1 : x2;

    vis->xscale = FRACUNIT;
    vis->yscale = FRACUNIT;
    vis->yiscale = FRACUNIT;
    vis->xiscale = FRACUNIT;
    vis->startfrac = 0;
    vis->patch = patch;
}


fixed_t R_PointToDist(int x, int y)
{
    int angle;
    fixed_t dx;
    fixed_t dy;
    fixed_t temp;
    fixed_t dist;

    dx = abs(x - viewx);
    dy = abs(y - viewy);

    if (dy > dx)
    {
        temp = dx;
        dx = dy;
        dy = temp;
    }

    angle = (tantoangle[FixedDiv(dy,dx)>>DBITS]+ANG90)>>ANGLETOFINESHIFT;

    dist = FixedDiv(dx, finesine[angle]);

    return dist;
}

fixed_t R_ScaleFromGlobalAngle(fixed_t rw_distance, angle_t visangle)
{
    fixed_t scale;
    angle_t anglea;
    angle_t angleb;
    fixed_t sinea;
    fixed_t sineb;
    fixed_t num;
    int den;

    anglea = ANG90 + (visangle-viewangle);
    angleb = ANG90 + (visangle-normalangle);

    sinea = finesine[anglea>>ANGLETOFINESHIFT];
    sineb = finesine[angleb>>ANGLETOFINESHIFT];
    num = R_FixedMul(0x8cccc0, sineb);
    den = R_FixedMul(rw_distance,sinea);

    if (den > (num>>16))
    {
        scale = FixedDiv(num, den);
        if (scale > 64*FRACUNIT)
            scale = 64*FRACUNIT;
        else if (scale < 256)
            scale = 256;
    }
    else
        scale = 64*FRACUNIT;

    return scale;
}

void R_SetupCalc(viswall_t *wc)
{
    angle_t offsetangle;
    fixed_t sineval;
    fixed_t rw_offset;

    offsetangle = normalangle - wc->angle1;

    if (offsetangle > ANG180)
        offsetangle = -offsetangle;
    if (offsetangle > ANG90)
        offsetangle = ANG90;

    sineval = finesine[offsetangle>>ANGLETOFINESHIFT];

    rw_offset = R_FixedMul(hyp, sineval);

    if (normalangle - wc->angle1 < ANG180)
        rw_offset = -rw_offset;

    wc->offset += rw_offset;
    wc->centerangle = ANG90 + viewangle - normalangle;
}

void R_FinishWallPrep(viswall_t *wc)
{
    seg_t *seg;
    angle_t offsetangle;
    angle_t distangle;
    fixed_t sineval;
    fixed_t rw_distance;
    fixed_t scale2;
    fixed_t scalefrac;

    if (wc->actionbits & AC_TOPTEXTURE)
        wc->t_texture->data = W_POINTLUMPNUM(wc->t_texture->lumpnum);
    if (wc->actionbits & AC_BOTTOMTEXTURE)
        wc->b_texture->data = W_POINTLUMPNUM(wc->b_texture->lumpnum);
    wc->floorpic = W_POINTLUMPNUM(firstflat + (int)wc->floorpic);
    if ((int)wc->ceilingpic == -1)
    {
        skytexturep->data = W_POINTLUMPNUM((int)skytexturep->lumpnum);
    }
    else
        wc->ceilingpic = W_POINTLUMPNUM(firstflat + (int)wc->ceilingpic);

    seg = wc->seg;

    normalangle = seg->angle + ANG90;
    offsetangle = abs(normalangle-wc->angle1);

    if (offsetangle > ANG90)
        offsetangle = ANG90;

    distangle = ANG90 - offsetangle;
    hyp = R_PointToDist(seg->v1->x, seg->v1->y);
    sineval = finesine[distangle>>ANGLETOFINESHIFT];
    wc->distance = rw_distance = FixedMul(hyp, sineval);

    scalefrac = scale2 = wc->scalefrac = R_ScaleFromGlobalAngle(rw_distance, viewangle + xtoviewangle[wc->start]);

    if (wc->stop > wc->start)
    {
        scale2 = R_ScaleFromGlobalAngle(rw_distance, viewangle + xtoviewangle[wc->stop]);
        wc->scalestep = abs((scale2 - scalefrac) / (wc->stop - wc->start));
    }

    wc->scale2 = scale2;

    if (wc->actionbits&(AC_BOTTOMTEXTURE|AC_TOPTEXTURE))
    {
        wc->actionbits |= AC_CALCTEXTURE;
        R_SetupCalc(wc);
    }
}
