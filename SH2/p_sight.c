#include "doomdef.h"
#include "p_local.h"

fixed_t sightzstart;
fixed_t topslope;
fixed_t bottomslope;
divline_t strace;
fixed_t t2x;
fixed_t t2y;
int t1xs;
int t1ys;
int t2xs;
int t2ys;
boolean sightreturn;
mobj_t *sight1;
mobj_t *sight2;

void P_CheckSight2(void);
boolean PS_CrossBSPNode(int bspnum);

void P_CheckSights2(void)
{
    mobj_t *mo;

    for (mo = mobjhead.next; mo != &mobjhead; mo = mo->next)
    {
        if (!(mo->flags&MF_COUNTKILL))
            continue;

        if (mo->tics != 1)
            continue;

        mo->flags &= ~MF_SEETARGET;

        if ((int)mo->target < 0x6000000)
            continue;

        sight1 = mo;
        sight2 = mo->target;

        P_CheckSight2();

        if (sightreturn)
            mo->flags |= MF_SEETARGET;
    }
}

void P_CheckSight2(void)
{
    mobj_t *t1;
    mobj_t *t2;
    int s1;
    int s2;
    int pnum;
    int bytenum;
    int bitnum;

    t1 = sight1;
    t2 = sight2;

    s1 = t1->subsector->sector - sectors;
    s2 = t2->subsector->sector - sectors;

    pnum = s1 * numsectors + s2;

    bytenum = pnum >> 3;
    bitnum = 1 << (pnum&7);

    if (rejectmatrix[bytenum] & bitnum)
    {
        sightreturn = false;
        return;
    }

    ++validcount;

    strace.x = (t1->x & ~0x1ffff) | 0x10000;
    strace.y = (t1->y & ~0x1ffff) | 0x10000;
    t2x = (t2->x & ~0x1ffff) | 0x10000;
    t2y = (t2->y & ~0x1ffff) | 0x10000;
    strace.dx = t2x - strace.x;
    strace.dy = t2y - strace.y;

    t1xs = strace.x >> 16;
    t1ys = strace.y >> 16;
    t2xs = t2x >> 16;
    t2ys = t2y >> 16;

    sightzstart = t1->z + t1->height - (t1->height>>2);
    topslope = (t2->z+t2->height) - sightzstart;
    bottomslope = (t2->z) - sightzstart;

    sightreturn = PS_CrossBSPNode(numnodes-1);
}

fixed_t	PS_SightCrossLine(line_t *line)
{
    int s1;
    int s2;
    int p1x;
    int p1y;
    int p2x;
    int p2y;
    int p3x;
    int p3y;
    int p4x;
    int p4y;
    int dx;
    int dy;
    int ndx;
    int ndy;

    p1x = line->v1->x >> 16;
    p1y = line->v1->y >> 16;
    p2x = line->v2->x >> 16;
    p2y = line->v2->y >> 16;

    p3x = t1xs;
    p3y = t1ys;
    p4x = t2xs;
    p4y = t2ys;

    dx = p2x - p3x;
    dy = p2y - p3y;

    ndx = p4x - p3x;
    ndy = p4y - p3y;

    s1 = (ndy * dx) < (dy * ndx);

    dx = p1x - p3x;
    dy = p1y - p3y;

    s2 = (ndy * dx) < (dy * ndx);

    if (s1 == s2)
        return -1;

    ndx = p1y - p2y;
    ndy = p2x - p1x;

    s1 = ndx*dx + ndy*dy;

    dx = p4x - p1x;
    dy = p4y - p1y;

    s2 = ndx*dx + ndy*dy;

    s2 = FixedDiv(s1,(s1+s2));

    return s2;
}

boolean PS_CrossSubsector(subsector_t *sub)
{
    seg_t *seg;
    line_t *line;
    int count;
    sector_t *front, *back;
    fixed_t opentop, openbottom;
    fixed_t frac, slope;

    count = sub->numlines;
    seg = &segs[sub->firstline];

    for (; count; seg++, count--)
    {
        line = seg->linedef;
        if (line->validcount == validcount)
            continue;

        line->validcount = validcount;

        frac = PS_SightCrossLine(line);
        if (frac < 4 || frac > FRACUNIT)
            continue;

        back = line->backsector;
        if (!back)
            return false;
        front = line->frontsector;

        if (front->floorheight == back->floorheight
            && front->ceilingheight == back->ceilingheight)
            continue;

        if (front->ceilingheight < back->ceilingheight)
            opentop = front->ceilingheight;
        else
            opentop = back->ceilingheight;
        if (front->floorheight > back->floorheight)
            openbottom = front->floorheight;
        else
            openbottom = back->floorheight;

        if (openbottom >= opentop)
            return false;

        frac >>= 2;

        if (front->floorheight != back->floorheight)
        {
            slope = (((openbottom - sightzstart) << 6) / frac) << 8;
            if (slope > bottomslope)
                bottomslope = slope;
        }

        if (front->ceilingheight != back->ceilingheight)
        {
            slope = (((opentop - sightzstart) << 6) / frac) << 8;
            if (slope < topslope)
                topslope = slope;
        }

        if (topslope <= bottomslope)
            return false;
    }

    return true;
}

int PS_DivlineSide(fixed_t x, fixed_t y, node_t *node)
{
    fixed_t dx;
    fixed_t dy;
    fixed_t left;
    fixed_t right;

    dx = x - node->x;
    dy = y - node->y;

    left = (node->dy>>FRACBITS)*dx;
    right = dy*(node->dx>>FRACBITS);

    if (right < left)
        return 0;
    return 1;
}

boolean PS_CrossBSPNode(int bspnum)
{
    node_t *bsp;
    int side;

    if (bspnum & NF_SUBSECTOR)
    {
        bspnum &= ~NF_SUBSECTOR;
        if (bspnum >= numsubsectors)
            I_Error("PS_CrossSubsector: ss %i with numss = %i", bspnum, numsubsectors);
        return PS_CrossSubsector(&subsectors[bspnum]);
    }

    bsp = &nodes[bspnum];

    side = PS_DivlineSide(strace.x, strace.y, bsp);

    if (!PS_CrossBSPNode(bsp->children[side]))
        return false;

    if (side == PS_DivlineSide(t2x, t2y, bsp))
        return true;

    return PS_CrossBSPNode(bsp->children[side^1]);
}
