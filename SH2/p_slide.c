#include "doomdef.h"
#include "p_local.h"


line_t *specialline;
int slidex;
int slidey;
int slidedx;
int slidedy;
int endbox[4];
int blockfrac;
int blocknvx;
int blocknvy;
int p1x;
int p1y;
int p2x;
int p2y;
int p3x;
int p3y;
int p4x;
int p4y;
int nvx;
int nvy;
short *list;
line_t *ld;
int offset;

extern mobj_t *slidething;

int P_CompletableFrac(int dx, int dy);
void SL_CheckSpecialLines(int x1, int y1, int x2, int y2);
void SL_CheckLine(line_t *ld);

void P_SlideMove(void)
{
    int dx;
    int dy;
    int i;
    int frac;
    int rx;
    int ry;
    int slide;
    int offset;

    dx = slidething->momx;
    dy = slidething->momy;

    slidex = slidething->x;
    slidey = slidething->y;

    for (i = 0; i < 3; i++)
    {
        frac = P_CompletableFrac(dx, dy);
        if (frac != 65536)
            frac -= 4096;
        if (frac < 0)
            frac = 0;
        rx = FixedMul(frac, dx);
        ry = FixedMul(frac, dy);

        slidex += rx;
        slidey += ry;

        if (frac == 65536)
        {
            slidething->momx = dx;
            slidething->momy = dy;
            SL_CheckSpecialLines(slidething->x, slidething->y, slidex, slidey);
            return;
        }

        dx -= rx;
        dy -= ry;
        slide = FixedMul(dx, blocknvx);
        slide += FixedMul(dy, blocknvy);

        dx = FixedMul(slide, blocknvx);
        dy = FixedMul(slide, blocknvy);
    }

    slidex = slidething->x;
    slidey = slidething->y;
    slidething->momx = slidething->momy = 0;
}

#define CLIPRADIUS 23

int P_CompletableFrac(int dx, int dy)
{
    int xl;
    int xh;
    int yl;
    int yh;
    int bx;
    int by;
    int offset;
    short *list;
    line_t *ld;

    blockfrac = 65536;
    slidedx = dx;
    slidedy = dy;

    endbox[BOXTOP] = slidey + CLIPRADIUS*FRACUNIT;
    endbox[BOXBOTTOM] = slidey - CLIPRADIUS*FRACUNIT;
    endbox[BOXRIGHT] = slidex + CLIPRADIUS*FRACUNIT;
    endbox[BOXLEFT] = slidex - CLIPRADIUS*FRACUNIT;

    if (dx > 0)
        endbox[BOXRIGHT] += dx;
    else
        endbox[BOXLEFT] += dx;
    
    if (dy > 0)
        endbox[BOXTOP] += dy;
    else
        endbox[BOXBOTTOM] += dy;

    ++validcount;

    xl = (endbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (endbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (endbox[BOXBOTTOM] - bmaporgx) >> MAPBLOCKSHIFT;
    yh = (endbox[BOXTOP] - bmaporgx) >> MAPBLOCKSHIFT;

    if (xl < 0)
        xl = 0;
    if (yl < 0)
        yl = 0;
    if (xh >= bmapwidth)
        xh = bmapwidth-1;
    if (yh >= bmapheight)
        yh = bmapheight-1;

    for (bx = xl; bx <= xh; bx++)
    {
        for (by = yl; by <= yh; by++)
        {
            offset = by * bmapwidth + bx;
            offset = *(blockmap+offset);
            for (list = blockmaplump + offset; *list != -1; list++)
            {
                ld = &lines[*list];
                if (ld->validcount == validcount)
                    continue;
                ld->validcount = validcount;

                SL_CheckLine(ld);
            }
        }
    }

    if (blockfrac < 4096)
    {
        blockfrac = 0;
        specialline = 0;
        return 0;
    }

    return blockfrac;
}

int SL_PointOnSide(fixed_t x, fixed_t y)
{
    fixed_t dx;
    fixed_t dy;
    fixed_t dist;
    fixed_t left;
    fixed_t right;

    dx = x - p1x;
    dy = y - p1y;

    dist = FixedMul(dx, nvx);
    dist += FixedMul(dy, nvy);

    if (dist > FRACUNIT)
        return 1;
    if (dist < -FRACUNIT)
        return -1;
    return 0;
}

int SL_CrossFrac(void)
{
    int dx;
    int dy;
    int dist1;
    int dist2;
    int frac;

    dx = p3x - p1x;
    dy = p3y - p1y;

    dist1 = FixedMul(dx, nvx);
    dist1 += FixedMul(dy, nvy);

    dx = p4x - p1x;
    dy = p4y - p1y;

    dist2 = FixedMul(dx, nvx);
    dist2 += FixedMul(dy, nvy);

    if ((dist1 < 0) == (dist2 < 0))
        return 65536;

    frac = FixedDiv(dist1, dist1 - dist2);

    return frac;
}

boolean CheckLineEnds(void)
{
    int snx;
    int sny;
    int dx;
    int dy;
    int dist1;
    int dist2;

    snx = p4y - p3y;
    sny = -(p4x - p3x);
    dx = p1x - p3x;
    dy = p1y - p3y;

    dist1 = FixedMul(dx, snx);
    dist1 += FixedMul(dy, sny);

    dx = p2x - p3x;
    dy = p2y - p3y;

    dist2 = FixedMul(dx, snx);
    dist2 += FixedMul(dy, sny);

    if ((dist1 < 0) == (dist2 < 0))
        return true;
    return false;
}

void ClipToLine(void)
{
    int side2;
    int side3;
    int frac;

    p3x = slidex - CLIPRADIUS*nvx;
    p3y = slidey - CLIPRADIUS*nvy;
    p4x = p3x + slidedx;
    p4y = p3y + slidedy;

    side2 = SL_PointOnSide(p3x, p3y);

    if (side2 == -1)
        return;

    side3 = SL_PointOnSide(p4x, p4y);

    if (side3 == 0)
        return;

    if (side3 == 1)
        return;

    if (side2 == 0)
    {
        frac = 0;
        goto blockmove;
    }

    frac = SL_CrossFrac();

    if (frac < blockfrac)
    {
blockmove: /* L174 */
        blockfrac = frac;
        blocknvx = -nvy;
        blocknvy = nvx;
    }
}

void SL_CheckLine(line_t *ld)
{
    sector_t* front;
    sector_t* back;
    int openbottom;
    int opentop;
    int side1;
    int temp;

    if (endbox[BOXRIGHT] < ld->bbox[BOXLEFT]
        || endbox[BOXLEFT] > ld->bbox[BOXRIGHT]
        || endbox[BOXTOP] < ld->bbox[BOXBOTTOM]
        || endbox[BOXBOTTOM] > ld->bbox[BOXTOP])
        return;

    if (!ld->backsector || (ld->flags & ML_BLOCKING))
        goto findfrac;

    front = ld->frontsector;
    back = ld->backsector;

    if (front->floorheight > back->floorheight)
        openbottom = front->floorheight;
    else
        openbottom = back->floorheight;

    if (openbottom - slidething->z > 24*FRACUNIT)
        goto findfrac;

    if (front->ceilingheight < back->ceilingheight)
        opentop = front->ceilingheight;
    else
        opentop = back->ceilingheight;

    if (opentop - openbottom >= 56*FRACUNIT)
        return;

findfrac: /* L194 */

    p1x = ld->v1->x;
    p1y = ld->v1->y;
    p2x = ld->v2->x;
    p2x = ld->v2->y;

    nvx = finesine[ld->fineangle];
    nvy = -finecosine[ld->fineangle];

    side1 = SL_PointOnSide(slidex, slidey);

    if (side1 == 0)
        return;

    if (side1 == -1)
    {
        if (!ld->backsector)
            return;

        temp = p1x;
        p1x = p2x;
        p2x = temp;
        temp = p1y;
        p1y = p2y;
        p2y = temp;
        nvx = -nvx;
        nvy = -nvy;
    }

    ClipToLine();
}

int SL_PointOnSide2(int x1, int y1, int x2, int y2, int x3, int y3)
{
    int nx;
    int ny;
    int dist;

    x1 = x1- x2;
    y1 = y1 - y2;

    nx = y3 - y2;
    ny = x2 - x3;

    dist = FixedMul(x1, nx);
    dist += FixedMul(y1, ny);

    if (dist < 0)
        return -1;
    return 1;
}

void SL_CheckSpecialLines(int x1, int y1, int x2, int y2)
{
    int xl;
    int xh;
    int yl;
    int yh;
    int bxl;
    int bxh;
    int byl;
    int byh;
    int bx;
    int by;
    int x3;
    int y3;
    int x4;
    int y4;
    int side1;
    int side2;
    if (x1 < x2)
    {
        xl = x1;
        xh = x2;
    }
    else
    {
        xl = x2;
        xh = x1;
    }
    if (y1 < y2)
    {
        yl = y1;
        yh = y2;
    }
    else
    {
        yl = y2;
        yh = y1;
    }

    bxl = (xl - bmaporgx) >> MAPBLOCKSHIFT;
    bxh = (xh - bmaporgx) >> MAPBLOCKSHIFT;
    byl = (yl - bmaporgy) >> MAPBLOCKSHIFT;
    byh = (yh - bmaporgy) >> MAPBLOCKSHIFT;

    if (bxl < 0)
        bxl = 0;
    if (byl < 0)
        byl = 0;
    if (bxh >= bmapwidth)
        bxh = bmapwidth-1;
    if (byh >= bmapheight)
        byh = bmapheight-1;

    specialline = NULL;
    ++validcount;

    for (bx = bxl; bx <= bxh; bx++)
    {
        for (by = byl; by <= byh; by++)
        {
            offset = by * bmapwidth + bx;
            offset = *(blockmap+offset);
            for (list = blockmaplump + offset; *list != -1; list++)
            {
                ld = &lines[*list];
                if (!ld->special)
                    continue;
                if (ld->validcount == validcount)
                    continue;
                ld->validcount = validcount;

                if (xh < ld->bbox[BOXLEFT]
                    || xl > ld->bbox[BOXRIGHT]
                    || yh < ld->bbox[BOXBOTTOM]
                    || yl > ld->bbox[BOXTOP])
                    continue;

                x3 = ld->v1->x;
                y3 = ld->v1->y;
                x4 = ld->v2->x;
                y4 = ld->v2->y;

                side1 = SL_PointOnSide2(x1, y1, x3, y3, x4, y4);
                side2 = SL_PointOnSide2(x1, y1, x3, y3, x4, y4);
                if (side1 == side2)
                    continue;

                side1 = SL_PointOnSide2(x3, y3, x1, y1, x2, y2);
                side2 = SL_PointOnSide2(x4, y4, x1, y1, x2, y2);
                if (side1 == side2)
                    continue;

                specialline = ld;
                return;
            }
        }
    }
}
