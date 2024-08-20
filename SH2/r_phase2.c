#include "doomdef.h"
#include "r_local.h"

sector_t emptysector = {
    0,0,
    -2,-2,-2
};

void R_WallPrep(void)
{
    viswall_t *segl;
    seg_t *seg;
    line_t *li;
    side_t *si;
    sector_t *front_sector;
    int f_ceilingpic;
    int f_lightlevel;
    int f_floorheight;
    int f_ceilingheight;
    sector_t *back_sector;
    int b_ceilingpic;
    int b_lightlevel;
    int b_floorheight;
    int b_ceilingheight;
    int b_texturemid;
    int t_texturemid;
    int actionbits;
    unsigned int rw_x;
    unsigned int rw_stopx;
    int skyhack;
    unsigned int width;

    for (segl = viswalls; segl < lastwallcmd; segl++)
    {
        seg = segl->seg;
        li = seg->linedef;
        si = seg->sidedef;
        li->flags |= ML_MAPPED;
        front_sector = seg->frontsector;
        f_ceilingpic = front_sector->ceilingpic;
        f_lightlevel = front_sector->lightlevel;
        f_floorheight = front_sector->floorheight - viewz;
        f_ceilingheight = front_sector->ceilingheight - viewz;

        segl->floorpic = (pixel_t*)flattranslation[front_sector->floorpic];
        if (f_ceilingpic == -1)
            segl->ceilingpic = (pixel_t*)-1;
        else
            segl->ceilingpic = (pixel_t*)flattranslation[f_ceilingpic];

        back_sector = seg->backsector;
        if (!back_sector)
            back_sector = &emptysector;
        b_ceilingpic = back_sector->ceilingpic;
        b_lightlevel = back_sector->lightlevel;
        b_floorheight = back_sector->floorheight - viewz;
        b_ceilingheight = back_sector->ceilingheight - viewz;
        b_texturemid = 0;
        t_texturemid = 0;
        actionbits = 0;

        rw_x = segl->start;
        rw_stopx = segl->stop + 1;

        skyhack = f_ceilingpic == -1 && b_ceilingpic == -1;

        if (f_floorheight < 0 &&
            (front_sector->floorpic != back_sector->floorpic ||
                f_floorheight != b_floorheight ||
                f_lightlevel != b_lightlevel ||
                b_ceilingheight == b_floorheight))
        {
            segl->floorheight = segl->floornewheight = f_floorheight >> FIXEDTOHEIGHT;
            actionbits |= (AC_ADDFLOOR|AC_NEWFLOOR);
        }

        if (skyhack && (f_ceilingheight > 0 || f_ceilingpic == -1) &&
            (f_ceilingpic != b_ceilingpic ||
                f_ceilingheight != b_ceilingheight ||
                f_lightlevel != b_lightlevel ||
                b_ceilingheight == b_floorheight))
        {
            segl->ceilingheight = segl->ceilingnewheight = f_ceilingheight >> FIXEDTOHEIGHT;
            if (f_ceilingpic == -1)
                actionbits |= AC_ADDSKY|AC_NEWCEILING;
            else
                actionbits |= AC_ADDCEILING|AC_NEWCEILING;
        }

        segl->t_topheight = b_ceilingheight >> FIXEDTOHEIGHT;

        if (back_sector == &emptysector)
        {
            segl->t_texture = &textures[texturetranslation[si->midtexture]];
            if (li->flags & ML_DONTPEGBOTTOM)
                t_texturemid = f_floorheight + (segl->t_texture->height << FRACBITS);
            else
                t_texturemid = f_ceilingheight;
            t_texturemid += si->rowoffset;
            segl->t_bottomheight = f_floorheight >> FIXEDTOHEIGHT;
            actionbits |= AC_TOPTEXTURE|AC_SOLIDSIL;
        }
        else
        {
            if (b_floorheight > f_floorheight)
            {
                segl->b_texture = &textures[texturetranslation[si->bottomtexture]];
                if (li->flags & ML_DONTPEGBOTTOM)
                    b_texturemid = f_ceilingheight;
                else
                    b_texturemid = b_floorheight;

                b_texturemid += si->rowoffset;
                segl->b_topheight = segl->floornewheight = b_floorheight >> FIXEDTOHEIGHT;
                segl->b_bottomheight = f_floorheight >> FIXEDTOHEIGHT;

                actionbits |= AC_NEWFLOOR|AC_BOTTOMTEXTURE;
            }

            if (b_ceilingheight < f_ceilingheight && !skyhack)
            {
                segl->t_texture = &textures[texturetranslation[si->toptexture]];
                if (li->flags & ML_DONTPEGBOTTOM)
                    t_texturemid = f_ceilingheight;
                else
                    t_texturemid = b_ceilingheight + (segl->t_texture->height << FRACBITS);

                t_texturemid += si->rowoffset;
                segl->t_bottomheight = segl->ceilingnewheight = b_ceilingheight >> FIXEDTOHEIGHT;
                actionbits |= AC_NEWCEILING|AC_TOPTEXTURE;
            }

            if (b_floorheight >= f_ceilingheight || b_ceilingheight <= f_floorheight)
                actionbits |= AC_SOLIDSIL;
            else
            {
                width = (rw_stopx - rw_x + 1) >> 1;
                if ((b_floorheight > 0 && b_floorheight > f_floorheight) ||
                    (f_floorheight < 0 && f_floorheight > b_floorheight))
                {
                    actionbits |= AC_BOTTOMSIL;
                    segl->bottomsil = (byte*)lastopening - rw_x;
                    lastopening += width;
                }
                if (!skyhack
                    && ((b_ceilingheight <= 0 && b_ceilingheight < f_ceilingheight) ||
                    (f_ceilingheight > 0 && b_ceilingheight > f_ceilingheight)))
                {
                    actionbits |= AC_TOPSIL;
                    segl->topsil = (byte*)lastopening - rw_x;
                    lastopening += width;
                }
            }
        }

        segl->actionbits = actionbits;
        segl->t_texturemid = t_texturemid;
        segl->b_texturemid = b_texturemid;
        segl->seglightlevel = f_lightlevel;
        segl->offset = si->textureoffset + seg->offset;
    }

    phasetime[2] = samplecount;
}
