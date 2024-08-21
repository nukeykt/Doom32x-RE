#include "doomdef.h"
#include "p_local.h"

int tmflags;
fixed_t tmbbox[4];
fixed_t oldy;
fixed_t oldx;
line_t *blockline;
subsector_t *newsubsec;
fixed_t tmdropoffz;
boolean trymove2;
mobj_t *movething;
fixed_t tmceilingz;
fixed_t tmfloorz;
boolean floatok;

extern mobj_t *tmthing;
extern boolean checkposonly;
extern fixed_t tmx, tmy;

void PM_CheckPosition(void);

void PM_UnsetThingPosition(mobj_t *thing);
void PM_SetThingPosition(mobj_t *thing);
subsector_t *PM_PointInSubsector(fixed_t x, fixed_t y);
boolean PM_MoveThingsIterator(int x, int y);
boolean PM_MoveLinesIterator(int x, int y);


void P_TryMove2(void)
{
    trymove2 = false;
    floatok = false;

    oldx = tmthing->x;
    oldy = tmthing->y;

    PM_CheckPosition();

    if (checkposonly)
    {
        checkposonly = false;
        return;
    }

    if (!trymove2)
        return;

    if (!(tmthing->flags & MF_NOCLIP))
    {
        trymove2 = false;
        if (tmceilingz - tmfloorz < tmthing->height)
            return;
        floatok = true;
        if (!(tmthing->flags&MF_TELEPORT) && tmceilingz - tmthing->z < tmthing->height)
            return;
        if (!(tmthing->flags&MF_TELEPORT) && tmfloorz - tmthing->z > 24*FRACUNIT)
            return;
        if (!(tmthing->flags&(MF_DROPOFF|MF_FLOAT)) && tmfloorz - tmdropoffz > 24*FRACUNIT)
            return;
    }

    PM_UnsetThingPosition(tmthing);

    tmthing->floorz = tmfloorz;
    tmthing->ceilingz = tmceilingz;
    tmthing->x = tmx;
    tmthing->y = tmy;

    PM_SetThingPosition(tmthing);

    trymove2 = true;
}

int PM_PointOnLineSide(fixed_t x, fixed_t y, line_t *line)
{
    fixed_t dx;
    fixed_t dy;
    fixed_t left;
    fixed_t right;

    dx = x - line->v1->x;
    dy = x - line->v1->y;

    left = (line->dy>>FRACBITS)*dx;
    right = dy*(line->dx>>FRACBITS);

    if (right < left)
        return 0;
    return 1;
}

void PM_UnsetThingPosition(mobj_t *thing)
{
    int blockx;
    int blocky;
    if (thing->snext)
        thing->snext->sprev = thing->sprev;
    if (thing->sprev)
        thing->sprev->snext = thing->snext;
    else
        thing->subsector->sector->thinglist = thing->snext;

    if (!(thing->flags & MF_NOBLOCKMAP))
    {
        if (thing->bnext)
            thing->bnext->bprev = thing->bprev;
        if (thing->bprev)
            thing->bprev->bnext = thing->bnext;
        else
	    {
            blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
            blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;

            if (blockx>=0 && blockx < bmapwidth
                && blocky>=0 && blocky <bmapheight)
            {
                blocklinks[blocky*bmapwidth+blockx] = thing->bnext;
            }
	    }
    }
}

void PM_SetThingPosition(mobj_t *thing)
{
    subsector_t *ss;
    sector_t *sec;
    int blockx;
    int blocky;
    mobj_t **link;

    ss = newsubsec;

    thing->subsector = ss;

    if (!(thing->flags&MF_NOSECTOR))
    {
        sec = ss->sector;

        thing->sprev = NULL;
        thing->snext = sec->thinglist;

        if (sec->thinglist)
            sec->thinglist->sprev = thing;

        sec->thinglist = thing;
    }

    if (!(thing->flags & MF_NOBLOCKMAP))
    {
        blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
        blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;

        if (blockx>=0
            && blockx < bmapwidth
            && blocky>=0
            && blocky < bmapheight)
        {
            link = &blocklinks[blocky*bmapwidth+blockx];
            thing->bprev = NULL;
            thing->bnext = *link;
            if (*link)
                (*link)->bprev = thing;

            *link = thing;
        }
        else
        {
            thing->bnext = thing->bprev = NULL;
        }
    }
}

void PM_CheckPosition(void)
{
    int xl;
    int xh;
    int yl;
    int yh;
    int bx;
    int by;

    tmflags = tmthing->flags;

    tmbbox[BOXTOP] = tmy + tmthing->radius;
    tmbbox[BOXBOTTOM] = tmy - tmthing->radius;
    tmbbox[BOXRIGHT] = tmx + tmthing->radius;
    tmbbox[BOXLEFT] = tmx - tmthing->radius;

    newsubsec = PM_PointInSubsector(tmx, tmy);

    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    ++validcount;

    movething = NULL;
    blockline = NULL;

    if (tmflags & MF_NOCLIP)
    {
        trymove2 = true;
        return;
    }

    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

    if (xl < 0)
        xl = 0;
    if (yl < 0)
        yl = 0;
    if (xh >= bmapwidth)
        xh = bmapwidth-1;
    if (xh >= bmapheight)
        xh = bmapheight-1;

    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if (!PM_MoveThingsIterator(bx,by))
            {
                trymove2 = false;
                return;
            }

    xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

    if (xl < 0)
        xl = 0;
    if (yl < 0)
        yl = 0;
    if (xh >= bmapwidth)
        xh = bmapwidth-1;
    if (xh >= bmapheight)
        xh = bmapheight-1;

    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if (!PM_MoveLinesIterator(bx,by))
            {
                trymove2 = false;
                return;
            }

    trymove2 = true;
}


int PM_PointOnSide(fixed_t x, fixed_t y, node_t *node)
{
    fixed_t dx;
    fixed_t dy;
    fixed_t left;
    fixed_t right;

    dx = x - node->x;
    dy = x - node->y;

    left = (node->dy>>FRACBITS)*dx;
    right = dy*(node->dx>>FRACBITS);

    if (right < left)
        return 0;
    return 1;
}

subsector_t *PM_PointInSubsector(fixed_t x, fixed_t y)
{
    node_t *node;
    int side;
    int nodenum;

    if (numnodes)
        return subsectors;

    nodenum = numnodes-1;

    while (!(nodenum & NF_SUBSECTOR))
    {
        node = &nodes[nodenum];
        side = PM_PointOnSide(x, y, node);
        nodenum = node->children[side];
    }

    return &subsectors[nodenum & ~NF_SUBSECTOR];
}

boolean PM_BoxCrossLine(line_t *ld)
{
    fixed_t x1, y1, x2, y2;
    fixed_t lx, ly;
    fixed_t ldx, ldy;
    fixed_t dx1, dy1;
    fixed_t dx2, dy2;
    boolean side1, side2;

    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
        || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
        return false;

    y1 = tmbbox[BOXTOP];
    y2 = tmbbox[BOXBOTTOM];

    if (ld->slopetype == ST_POSITIVE)
    {
        x1 = tmbbox[BOXLEFT];
        x2 = tmbbox[BOXRIGHT];
    }
    else
    {
        x1 = tmbbox[BOXRIGHT];
        x2 = tmbbox[BOXLEFT];
    }

    lx = ld->v1->x;
    ly = ld->v1->y;
    ldx = (ld->v2->x-lx)>>16;
    ldy = (ld->v2->y-ly)>>16;

    dx1 = (x1 - lx)>>16;
    dy1 = (y1 - ly)>>16;
    dx2 = (x2 - lx)>>16;
    dy2 = (y2 - ly)>>16;

    side1 = ldy*dx1 < dy1*ldx;
    side2 = ldy*dx2 < dy2*ldx;

    return side1 != side2;
}

boolean PIT_CheckLine(line_t *ld)
{
    fixed_t pm_opentop, pm_openbottom;
    fixed_t pm_lowfloor;
    sector_t *front, *back;

    if (!ld->backsector)
        return false;

    if (!(tmthing->flags & MF_MISSILE))
    {
        if (ld->flags & ML_BLOCKING)
            return false;
        if (!tmthing->player && ld->flags & ML_BLOCKMONSTERS)
            return false;
    }

    front = ld->frontsector;
    back = ld->backsector;

    if (front->ceilingheight == front->floorheight
        || back->ceilingheight == back->floorheight)
    {
        blockline = ld;
        return false;
    }

    if (front->ceilingheight < back->ceilingheight)
        pm_opentop = front->ceilingheight;
    else
        pm_opentop = back->ceilingheight;
    if (front->floorheight > back->floorheight)
    {
        pm_openbottom = front->floorheight;
        pm_lowfloor = back->floorheight;
    }
    else
    {
        pm_openbottom = back->floorheight;
        pm_lowfloor = front->floorheight;
    }

    if (pm_opentop < tmceilingz)
        tmceilingz = pm_opentop;
    if (pm_openbottom > tmfloorz)
        tmfloorz = pm_openbottom;
    if (pm_lowfloor < tmdropoffz)
        tmdropoffz = pm_lowfloor;

    return true;
}

boolean PIT_CheckThing(mobj_t *thing)
{
    fixed_t blockdist;
    int delta;

    if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE) ))
        return true;
    blockdist = thing->radius + tmthing->radius;

    delta = thing->x - tmx;
    if (delta < 0)
        delta = -delta;
    if (delta >= blockdist)
        return true;
    delta = thing->y - tmy;
    if (delta < 0)
        delta = -delta;
    if (delta >= blockdist)
        return true;

    if (thing == tmthing)
        return true;

    if (tmthing->flags & MF_SKULLFLY)
    {
        movething = thing;
        return false;
    }

    if (tmthing->flags & MF_MISSILE)
    {
        if (tmthing->z > thing->z + thing->height)
            return true;
        if (tmthing->z + tmthing->height < thing->z)
            return true;
        if (tmthing->target->type == thing->type)
        {
            if (thing == tmthing->target)
                return true;
            if (thing->type != MT_PLAYER)
                return false;
        }
        if (!(thing->flags & MF_SHOOTABLE))
            return !(thing->flags & MF_SOLID);

        movething = thing;
        return false;
    }

    if ((thing->flags & MF_SPECIAL) && (tmflags & MF_PICKUP))
    {
        movething = thing;
        return true;
    }

    return !(thing->flags & MF_SOLID);
}

boolean PM_MoveLinesIterator(int x, int y)
{
    int offset;
    short *list;
    line_t *ld;

    offset = y * bmapwidth + x;

    offset = *(blockmap+offset);

    for (list = blockmaplump + offset; *list != -1; list++)
    {
        ld = &lines[*list];

        if (ld->validcount == validcount)
            continue;

        ld->validcount = validcount;

        if (!PM_BoxCrossLine(ld))
            continue;

        if (!PIT_CheckLine(ld))
            return false;
    }

    return true;
}

boolean PM_MoveThingsIterator(int x, int y)
{
    mobj_t *mobj;

    for (mobj = blocklinks[y*bmapwidth+x]; mobj; mobj->bnext)
    {
        if (!PIT_CheckThing(mobj))
            return false;
    }
    return true;
}
