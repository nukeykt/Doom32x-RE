#include "doomdef.h"
#include "r_local.h"
#include "marsonly.h"


short clipboundtop[SCREENWIDTH];
short clipboundbottom[SCREENWIDTH];

void R_SegLoop(viswall_t *w);

void R_DrawTexture(drawtex_t *tex, viswall_t *w);

void R_SegCommands(void)
{
    int i;
    viswall_t *w;
    drawtex_t drawtex;
    texture_t *tex;

    for (i = 0; i < SCREENWIDTH; i++)
    {
        clipboundtop[i] = -1;
        clipboundbottom[i] = SCREENHEIGHT;
    }

    for (w = (viswall_t*)0x2400f980; w < lastwallcmd; w++)
    {
        if (w->actionbits & AC_TOPTEXTURE)
        {
            drawtex.topheight = w->t_topheight;
            drawtex.bottomheight = w->t_bottomheight;
            drawtex.texturemid = w->t_texturemid;
            tex = w->t_texture;
            drawtex.width = tex->width;
            drawtex.height = tex->height;
            drawtex.data = tex->data;
            R_DrawTexture(&drawtex, w);
        }
        if (w->actionbits & AC_BOTTOMTEXTURE)
        {
            drawtex.topheight = w->b_topheight;
            drawtex.bottomheight = w->b_bottomheight;
            drawtex.texturemid = w->b_texturemid;
            tex = w->b_texture;
            drawtex.width = tex->width;
            drawtex.height = tex->height;
            drawtex.data = tex->data;
            R_DrawTexture(&drawtex, w);
        }

        R_SegLoop(w);
    }

    phasetime[6] = samplecount;
}

#define OPENMARK 0xff00

visplane_t *R_FindPlane(visplane_t *check, fixed_t height, pixel_t *picnum, int lightlevel, int start, int stop)
{
    int i;
    for (; check < lastvisplane; check++)
    {
        if (height == check->height
            && picnum == check->picnum
            && lightlevel == check->lightlevel
            && check->open[start] == OPENMARK)
        {
            if (start < check->minx)
                check->minx = start;
            if (stop > check->maxx)
                check->maxx = stop;
            return check;
        }
    }

    check = lastvisplane;
    ++lastvisplane;
    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = start;
    check->maxx = stop;

    for (i = 0; i < SCREENWIDTH/2; i++)
    {
        ((int*)check->open)[i] = (OPENMARK<<16)|OPENMARK;
    }
    return check;
}

void R_SegLoop(viswall_t *w)
{
    int x;
    int scale;
    int scalefrac;
    int actionbits;
    visplane_t *floorplane, *ceilingplane;
    int ceilingclipx, floorclipx;
    int top, bottom;
    int low, high;
    int colnum;

    scalefrac = w->scalefrac;
    actionbits = w->actionbits;
    floorplane = ceilingplane = (visplane_t*)0x24013180;

    for (x = w->start; x <= w->stop; x++)
    {
        scale = scalefrac >> FIXEDTOSCALE;
        scalefrac += w->scalestep;
        if (scale >= 0x8000)
            scale = 0x7fff;
        floorclipx = clipboundbottom[x];
        ceilingclipx = clipboundtop[x];

        if (w->actionbits & AC_ADDSKY)
        {
            top = ceilingclipx + 1;
            bottom = CENTERY - 1 - ((scale * w->ceilingheight) >> (HEIGHTBITS+SCALEBITS));
            if (bottom >= floorclipx)
                bottom = floorclipx - 1;

            if (top <= bottom)
            {
                colnum = ((viewangle + xtoviewangle[x]) >> ANGLETOSKYSHIFT) & 255;
                R_DrawColumn(x, top, bottom, 0, top * 91022, 91022, (inpixel_t*)&skytexturep->data[colnum * 64]);
            }
        }
        if (actionbits & AC_ADDFLOOR)
        {
            top = CENTERY - ((scale*w->floorheight) >> (HEIGHTBITS+SCALEBITS));
            if (top <= ceilingclipx)
                top = ceilingclipx + 1;
            bottom = floorclipx - 1;
            if (top <= bottom)
            {
                if (floorplane->open[x] != OPENMARK)
                {
                    floorplane = R_FindPlane(floorplane, w->floorheight, w->floorpic, w->seglightlevel,
                        x, w->stop);
                }
                floorplane->open[x] = (top << 8) + bottom;
            }
        }
        if (actionbits & AC_ADDCEILING)
        {
            top = ceilingclipx + 1;
            bottom = CENTERY - 1 - ((scale*w->ceilingheight) >> (HEIGHTBITS+SCALEBITS));
            if (bottom >= floorclipx)
                bottom = floorclipx - 1;
            if (top <= bottom)
            {
                if (ceilingplane->open[x] != OPENMARK)
                {
                    ceilingplane = R_FindPlane(ceilingplane, w->ceilingheight, w->ceilingpic, w->seglightlevel,
                        x, w->stop);
                }
                ceilingplane->open[x] = (top << 8) + bottom;
            }
        }

        if (actionbits & (AC_BOTTOMSIL|AC_NEWFLOOR))
        {
            low = CENTERY - ((scale*w->floornewheight) >> (HEIGHTBITS+SCALEBITS));
            if (low > floorclipx)
                low = floorclipx;
            if (low < 0)
                low = 0;
        }
        if (actionbits & (AC_TOPSIL|AC_NEWCEILING))
        {
            high = CENTERY - 1 - ((scale*w->ceilingnewheight) >> (HEIGHTBITS+SCALEBITS));
            if (high < ceilingclipx)
                high = ceilingclipx;
            if (high > SCREENHEIGHT-1)
                high = SCREENHEIGHT-1;
        }
        if (actionbits & AC_BOTTOMSIL)
            w->bottomsil[x] = low;
        if (actionbits & AC_TOPSIL)
            w->topsil[x] = high+1;
        if (actionbits & AC_NEWFLOOR)
            clipboundbottom[x] = low;
        if (actionbits & AC_NEWCEILING)
            clipboundtop[x] = high;
    }
}
