#include "doomdef.h"
#include "p_local.h"


line_t *shootline;
mobj_t *shootmobj;
fixed_t shootslope;
fixed_t shootx;
fixed_t shooty;
fixed_t shootz;
fixed_t aimmidslope;
divline_t shootdiv;
fixed_t shootx2;
fixed_t shooty2;
fixed_t firstlinefrac;
int shootdivpositive;
fixed_t old_frac;
void *old_value;
boolean old_isline;
boolean hitsolid;
int ssx1;
int ssy1;
int ssx2;
int ssy2;
vertex_t tv1;
vertex_t tv2;

line_t thingline = { &tv1, &tv2 };

extern mobj_t *shooter;
extern angle_t attackangle;
extern fixed_t attackrange;
extern fixed_t opentop;
extern fixed_t openbottom;
extern fixed_t aimtopslope;
extern fixed_t aimbottomslope;

boolean PA_CrossBSPNode(int bspnum);
fixed_t R_FixedMul(fixed_t a, fixed_t b);

boolean PA_DoIntercept(void *value, boolean isline, int frac);
boolean PA_ShootLine(line_t *li, fixed_t interceptfrac);
boolean PA_ShootThing(mobj_t *th, fixed_t interceptfrac);

void P_Shoot2(void)
{
    mobj_t *t1;
    angle_t angle;

    t1 = shooter;

    shootline = NULL;
    shootmobj = NULL;

    angle = attackangle >> ANGLETOFINESHIFT;

    shootdiv.x = t1->x;
    shootdiv.y = t1->y;
    shootx2 = t1->x + (attackrange>>FRACBITS)*finecosine[angle];
    shooty2 = t1->y + (attackrange>>FRACBITS)*finesine[angle];
    shootdiv.dx = shootx2 - shootdiv.x;
    shootdiv.dy = shooty2 - shootdiv.y;
    shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

    shootdivpositive = (shootdiv.dx ^ shootdiv.dy) > 0;

    ssx1 = shootdiv.x >> 16;
    ssy1 = shootdiv.y >> 16;
    ssx2 = shootx2 >> 16;
    ssy2 = shooty2 >> 16;

    aimmidslope = (aimtopslope + aimbottomslope)>>1;

    old_frac = 0;

    PA_CrossBSPNode(numnodes-1);

    if (!shootmobj)
        PA_DoIntercept(0, false, FRACUNIT);

    if (shootmobj)
        return;

    if (!shootline)
        return;

    firstlinefrac -= FixedDiv(4*FRACUNIT,attackrange);

    shootx = shootdiv.x + R_FixedMul(shootdiv.dx, firstlinefrac);
    shooty = shootdiv.y + R_FixedMul(shootdiv.dy, firstlinefrac);
    shootz = shootz + R_FixedMul(aimmidslope, R_FixedMul(firstlinefrac, attackrange));
}

boolean PA_DoIntercept(void *value, boolean isline, int frac)
{
    int temp;

    if (old_frac < frac)
    {
        temp = (int)old_value;
        old_value = value;
        value = (void*)temp;

        temp = old_isline;
        old_isline = isline;
        isline = temp;

        temp = old_frac;
        old_frac = frac;
        frac = temp;
    }

    if (frac == 0 || frac >= FRACUNIT)
        return true;

    if (isline)
        return PA_ShootLine((line_t*)value, frac);

    return PA_ShootThing((mobj_t*)value, frac);
}

boolean PA_ShootLine(line_t *li, fixed_t interceptfrac)
{
    fixed_t slope;
    fixed_t dist;
    sector_t *front;
    sector_t *back;

    if (!(li->flags & ML_TWOSIDED))
    {
        if (!shootline)
        {
            shootline = li;
            firstlinefrac = interceptfrac;
        }
        old_frac = 0;
        return false;
    }

    front = li->frontsector;
    back = li->backsector;

    if (front->ceilingheight < back->ceilingheight)
        opentop = front->ceilingheight;
    else
        opentop = back->ceilingheight;

    if (front->floorheight > back->floorheight)
        openbottom = front->ceilingheight;
    else
        openbottom = back->ceilingheight;

    dist = R_FixedMul(attackrange, interceptfrac);

    if (li->frontsector->floorheight != li->backsector->floorheight)
    {
        slope = FixedDiv(openbottom - shootz, dist);
        if (slope >= aimmidslope && !shootline)
        {
            shootline = li;
            firstlinefrac = interceptfrac;
        }
        if (slope > aimbottomslope)
            aimbottomslope = slope;
    }

    if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
    {
        slope = FixedDiv(opentop - shootz, dist);
        if (slope <= aimmidslope && !shootline)
        {
            shootline = li;
            firstlinefrac = interceptfrac;
        }
        if (slope < aimtopslope)
            aimtopslope = slope;
    }

    if (aimtopslope <= aimbottomslope)
        return false;

    return true;
}

boolean PA_ShootThing(mobj_t *th, fixed_t interceptfrac)
{
    fixed_t frac;
    fixed_t dist;
    fixed_t thingaimtopslope, thingaimbottomslope;

    if (th == shooter)
        return true;

    if (!(th->flags&MF_SHOOTABLE))
        return true;

    dist = R_FixedMul(attackrange, interceptfrac);
    thingaimtopslope = FixedDiv(th->z + th->height - shootz, dist);
    if (thingaimtopslope < aimbottomslope)
        return true;
    thingaimbottomslope = FixedDiv(th->z - shootz, dist);
    if (thingaimbottomslope > aimtopslope)
        return true;

    if (thingaimtopslope > aimtopslope)
        thingaimtopslope = aimtopslope;
    if (thingaimbottomslope < aimbottomslope)
        thingaimbottomslope = aimbottomslope;

    shootslope = (thingaimtopslope+thingaimbottomslope)/2;

    shootmobj = th;

    frac = interceptfrac - FixedDiv(10*FRACUNIT, attackrange);
    shootx = shootdiv.x + FixedMul(shootdiv.dx, frac);
    shooty = shootdiv.y + FixedMul(shootdiv.dy, frac);
    shootz = shootz + FixedMul(shootslope, FixedMul(frac, attackrange));

    return true;
}

fixed_t PA_SightCrossLine(line_t *line)
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

    p3x = ssx1;
    p3y = ssy1;
    p4x = ssx2;
    p4y = ssy2;

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

boolean PA_CrossSubsector(subsector_t *sub)
{
    seg_t *seg;
    line_t *line;
    int count;
    fixed_t frac;
    mobj_t *thing;

    for (thing = sub->sector->thinglist; thing; thing = thing->snext)
    {
        if (thing->subsector != sub)
            continue;

        if (shootdivpositive)
        {
            tv1.x = thing->x - thing->radius;
            tv1.y = thing->y + thing->radius;

            tv2.x = thing->x + thing->radius;
            tv2.y = thing->y - thing->radius;
        }
        else
        {
            tv1.x = thing->x - thing->radius;
            tv1.y = thing->y - thing->radius;

            tv2.x = thing->x + thing->radius;
            tv2.y = thing->y + thing->radius;
        }

        frac = PA_SightCrossLine(&thingline);

        if (frac < 0 || frac > FRACUNIT)
            continue;
        if (!PA_DoIntercept((void*)thing, false, frac))
            return false;
    }

    count = sub->numlines;
    seg = &segs[sub->firstline];

    for (; count; seg++, count--)
    {
        line = seg->linedef;
        if (line->validcount == validcount)
            continue;

        line->validcount = validcount;

        frac = PA_SightCrossLine(line);
        if (frac < 0 || frac > FRACUNIT)
            continue;

        if (!PA_DoIntercept((void*)line, true, frac))
            return false;
    }

    return true;
}

int PA_DivlineSide(fixed_t x, fixed_t y, node_t *node)
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

boolean PA_CrossBSPNode(int bspnum)
{
    node_t *bsp;
    int side;

    if (bspnum & NF_SUBSECTOR)
    {
        bspnum &= ~NF_SUBSECTOR;
        if (bspnum >= numsubsectors)
            I_Error("PA_CrossSubsector: ss %i with numss = %i", bspnum, numsubsectors);
        return PA_CrossSubsector(&subsectors[bspnum]);
    }

    bsp = &nodes[bspnum];

    side = PA_DivlineSide(shootdiv.x, shootdiv.y, bsp);

    if (!PA_CrossBSPNode(bsp->children[side]))
        return false;

    if (side == PA_DivlineSide(shootx2, shooty2, bsp))
        return true;

    return PA_CrossBSPNode(bsp->children[side^1]);
}
