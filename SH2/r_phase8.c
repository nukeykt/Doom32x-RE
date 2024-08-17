#include "doomdef.h"
#include "r_local.h"
#include "marsonly.h"

vissprite_t *lastsprite_p;

int spropening[SCREENWIDTH];

void R_ClipVisSprite(vissprite_t *a1);
void R_DrawVisSprite(vissprite_t *a1);

void R_Sprites(void)
{
    int count;
    int i;
    int bestscale;
    int stopx;
    vissprite_t *best;
    vissprite_t *ds;

    count = lastsprite_p - vissprites;
    best = NULL;
    for (i = 0; i < count; i++)
    {
        bestscale = MAXINT;
        for (ds = vissprites; ds != lastsprite_p; ds++)
        {
            if (ds->xscale < bestscale)
            {
                bestscale = ds->xscale;
                best = ds;
            }
        }
        if (best->patch)
        {
            R_ClipVisSprite(best);
            R_DrawVisSprite(best);
        }
        best->xscale = MAXINT;
    }

    for (;lastsprite_p < vissprite_p; lastsprite_p++)
    {
        stopx = lastsprite_p->x2 + 1;
        for (i = lastsprite_p->x1; i < stopx; i++)
        {
            spropening[i] = SCREENHEIGHT;
        }

        R_DrawVisSprite(lastsprite_p);
    }

    phasetime[8] = samplecount;
}

boolean R_SegBehindPoint(viswall_t *ds, fixed_t dx, fixed_t dy)
{
    fixed_t x1;
    fixed_t y1;
    fixed_t sdx;
    fixed_t sdy;
    seg_t *v;

    v = ds->seg;

    x1 = v->v1->x;
    y1 = v->v1->y;

    sdx = v->v2->x - x1;
    sdy = v->v2->y - y1;

    dx -= x1;
    dy -= y1;

    sdx >>= FRACBITS;
    sdy >>= FRACBITS;
    dx >>= FRACBITS;
    dy >>= FRACBITS;

    dx *= sdy;
    sdx *= dy;
    if (sdx < dx)
        return true;
    return false;
}


void R_ClipVisSprite(vissprite_t *vis)
{
    int x1;
    int x2;
    int gz;
    int gzt;
    int scalefrac;
    int x;
    int r1;
    int r2;
    int silhouette;
    int opening;
    int top;
    int bottom;
    byte *topsil;
    byte *bottomsil;
    viswall_t *ds;

    x1 = vis->x1;
    x2 = vis->x2;

    gz = (vis->gz - viewz) >> 10;
    gzt = (vis->gzt - viewz) >> 10;

    scalefrac = vis->yscale;

    for (x = vis->x1; x <= x2; x++)
    {
        spropening[x] = SCREENHEIGHT;
    }

    for (ds = lastwallcmd - 1; ds >= viswalls; ds--)
    {
        if (ds->start > x2 || ds->stop < x1 ||
            (ds->scalefrac < scalefrac && ds->scale2 < scalefrac) ||
            !(ds->actionbits&(AC_TOPSIL|AC_BOTTOMSIL|AC_SOLIDSIL)))
            continue;

        if (ds->scalefrac <= scalefrac || ds->scale2 <= scalefrac)
        {
            if (R_SegBehindPoint(ds, vis->gx, vis->gy))
                continue;
        }

        r1 = ds->start < x1 ? x1 : ds->start;
        r2 = ds->stop > x2 ? x2 : ds->stop;

        silhouette = ds->actionbits & (AC_TOPSIL|AC_BOTTOMSIL|AC_SOLIDSIL);

        if (silhouette == AC_SOLIDSIL)
        {
            for (x = r1; x <= r2; x++)
            {
                spropening[x] = SCREENHEIGHT << 8;
            }
            continue;
        }

        topsil = ds->topsil;
        bottomsil = ds->bottomsil;
        
        if (silhouette == AC_BOTTOMSIL)
        {
            for (x = r1; x <= r2; x++)
            {
                opening = spropening[x];
                if ((opening & 0xff) == SCREENHEIGHT)
                {
                    spropening[x] = (opening&0xff00) + bottomsil[x];
                }
            }
        }
        else if (silhouette == AC_TOPSIL)
        {
            for (x = r1; x <= r2; x++)
            {
                opening = spropening[x];
                if ((opening & 0xff00) == 0)
                {
                    spropening[x] = (topsil[x]<<8) + (opening&0xff);
                }
            }
        }
        else if (silhouette == (AC_TOPSIL|AC_BOTTOMSIL))
        {
            for (x = r1; x <= r2; x++)
            {
                top = spropening[x];
                bottom = top&0xff;
                top >>= 8;
                if (bottom == SCREENHEIGHT)
                    bottom = bottomsil[x];
                if (top == 0)
                    top = topsil[x];
                spropening[x] = (top<<8) + bottom;
            }
        }
    }
}

fixed_t R_FixedMul(fixed_t a, fixed_t b);

void R_DrawVisSprite(vissprite_t *vis)
{
    int iscale;
    int xfrac;
    int spryscale;
    int sprtop;
    int x;
    int stopx;
    int fracstep;
    int texturecolumn;
    int topclip;
    int bottomclip;
    int oldcolread;
    int top;
    int bottom;
    int frac;
    patch_t *patch;
    pixel_t *pixels;
    column_t *column;

    patch = vis->patch;
    pixels = vis->pixels;
    iscale = vis->yiscale;
    xfrac = vis->startfrac;
    spryscale = vis->yscale;

    sprtop = CENTERYFRAC - R_FixedMul(vis->texturemid, spryscale);

    stopx = vis->x2 + 1;
    fracstep = vis->xscale;

    for (x = vis->x1; x < stopx; x++, xfrac += fracstep)
    {
        texturecolumn = xfrac >> FRACBITS;
        if (texturecolumn < 0 || texturecolumn >= patch->width)
            I_Error("R_DrawSpriteRange: bad texturecolumn");

        column = (column_t*)((byte*)patch + patch->columnofs[texturecolumn]);

        bottomclip = spropening[x];
        oldcolread = column->topdelta;

        topclip = bottomclip >> 8;
        bottomclip = (bottomclip & 255) - 1;
        
        for (; oldcolread != 0xff; column++, oldcolread = column->topdelta)
        {
            top = sprtop + ((spryscale * oldcolread) << 8);
            bottom = top + ((spryscale * column->length) << 8);

            top = (top+FRACUNIT-1)>>FRACBITS;
            bottom = (bottom-1)>>FRACBITS;
            if (bottom > bottomclip)
                bottom = bottomclip;

            if (top > bottom)
                continue;

            if (top < topclip)
            {
                if (bottom < topclip)
                    continue;
                frac = (topclip - top) * iscale;
                top = topclip;
            }
            else
                frac = 0;

            frac = frac + (column->dataofs << FRACBITS);

            R_DrawColumn(x, bottom, top, 0, frac, iscale, (inpixel_t*)pixels);
        }
    }
}
