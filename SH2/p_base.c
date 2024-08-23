#include "doomdef.h"
#include "p_local.h"

mobj_t *basething;
int testx;
int testy;
int testfloorz;
int testceilingz;
int testdropoffz;
subsector_t *testsubsec;
line_t *ceilingline;
mobj_t *hitthing;
int testbbox[4];
int testflags;

boolean PB_TryMove(int tryx, int tryy);
boolean PB_CheckPosition(void);
void PB_UnsetThingPosition(mobj_t *thing);
void PB_SetThingPosition(mobj_t *thing);
subsector_t *PB_PointInSubsector(fixed_t x, fixed_t y);
boolean PB_MoveThingsIterator(int x, int y);
boolean PB_MoveLinesIterator(int x, int y);

void P_RunMobjBase2(void)
{
    activemobjs = 0;

    for (basething = mobjhead.next; basething != &mobjhead; basething = basething->next)
    {
        if (basething->player)
            continue;

        activemobjs++;

        basething->latecall = NULL;

        P_MobjThinker(basething);
    }
}

#define STOPSPEED 0x1000
#define FRICTION 0xd240

void P_XYMovement(mobj_t *mo)
{
    int xuse;
    int yuse;
    int xleft;
    int yleft;

    xleft = xuse = mo->momx & ~7;
    yleft = yuse = mo->momy & ~7;

    while (xuse > MAXMOVE || xuse < -MAXMOVE
        || yuse > MAXMOVE || yuse < -MAXMOVE)
    {
        xuse >>= 1;
        yuse >>= 1;
    }

    while (xleft || yleft)
    {
        xleft -= xuse;
        yleft -= yuse;

        if (!PB_TryMove(mo->x + xuse, mo->y + yuse))
        {
            if (mo->flags & MF_SKULLFLY)
            {
                mo->extradata = (int)hitthing;
                mo->latecall = L_SkullBash;
            }
            if (mo->flags & MF_MISSILE)
            {
                if (ceilingline && ceilingline->backsector &&
                    ceilingline->backsector->ceilingpic == -1)
                {
                    mo->latecall = P_RemoveMobj;
                    return;
                }
                mo->extradata = (int)hitthing;
                mo->latecall = L_MissileHit;
                return;
            }

            mo->momx = mo->momy = 0;
            return;
        }
    }

    if (mo->flags & (MF_MISSILE|MF_SKULLFLY))
        return;

    if (mo->z > mo->floorz)
        return;

    if (mo->flags & MF_CORPSE)
        if (mo->floorz != mo->subsector->sector->floorheight)
            return;

    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
        && mo->momy > -STOPSPEED && mo->momy < STOPSPEED)
    {
        mo->momx = 0;
        mo->momy = 0;
    }
    else
    {
        mo->momx = (mo->momx>>8)*(FRICTION>>8);
        mo->momy = (mo->momy>>8)*(FRICTION>>8);
    }
}

void P_FloatChange(mobj_t* mo)
{
    int dx;
    int dy;
    int dist;
    int delta;

    dx = mo->target->x - mo->x;
    if (dx < 0) dx = -dx;
    dy = mo->target->y - mo->y;
    if (dy < 0) dy = -dy;

    if (dx < dy)
        dist = dx + dy - (dx >> 1);
    else
        dist = dx + dy - (dy >> 1);

    delta = (mo->target->z + (mo->height>>1)) - mo->z;

    if (delta < 0)
    {
        if (dist < -delta * 3)
        {
            mo->z -= FLOATSPEED;
        }
        return;
    }
    if (dist < delta * 3)
        mo->z += FLOATSPEED;
}

void P_ZMovement(mobj_t *mo)
{
    mo->z += mo->momz;

    if ((mo->flags & MF_FLOAT) && mo->target)
        P_FloatChange(mo);

    if (mo->z <= mo->floorz)
    {
        if (mo->momz < 0)
            mo->momz = 0;
        mo->z = mo->floorz;
        if (mo->flags & MF_MISSILE)
        {
            mo->latecall = P_ExplodeMissile;
            return;
        }
    }
    else if (!(mo->flags & MF_NOGRAVITY))
    {
        if (!mo->momz)
            mo->momz = -GRAVITY*2;
        else
            mo->momz -= GRAVITY;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {
        if (mo->momz > 0)
            mo->momz = 0;
        mo->z = mo->ceilingz - mo->height;
        if (mo->flags & MF_MISSILE)
        {
            mo->latecall = P_ExplodeMissile;
            return;
        }
    }
}

void P_MobjThinker(mobj_t *mobj)
{
    statenum_t state;
    state_t *st;

    if (mobj->momx || mobj->momy)
    {
        P_XYMovement(mobj);
        if (mobj->latecall)
            return;
    }

    if ((mobj->z != mobj->floorz) || mobj->momz)
    {
        P_ZMovement(mobj);
        if (mobj->latecall)
            return;
    }

    if (mobj->tics == -1)
        return;

    mobj->tics--;

    if (mobj->tics > 0)
        return;

    state = mobj->state->nextstate;

    if (state == S_NULL)
    {
        mobj->latecall = P_RemoveMobj;
        return;
    }

    st = &states[state];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    mobj->latecall = st->action;
}

boolean PB_TryMove(int tryx, int tryy)
{
    int oldx;
    int oldy;

    testx = tryx;
    testy = tryy;

    if (!PB_CheckPosition())
        return false;

    if (testceilingz - testfloorz < basething->height)
        return false;

    if (testceilingz - basething->z < 0)
        return false;

    if (testfloorz - basething->z > 24*FRACUNIT)
        return false;

    if (!(testflags & (MF_DROPOFF|MF_FLOAT))
        && testfloorz - testdropoffz > 24*FRACUNIT)
        return false;

    PB_UnsetThingPosition(basething);

    oldx = basething->x;
    oldy = basething->y;

    basething->floorz = testfloorz;
    basething->ceilingz = testceilingz;
    basething->x = testx;
    basething->y = testy;

    PB_SetThingPosition(basething);

    return true;
}

void PB_UnsetThingPosition(mobj_t *thing)
{
    int blockx;
    int blocky;
    if (thing->snext)
        thing->snext->sprev = thing->sprev;
    if (thing->sprev)
        thing->sprev->snext = thing->snext;
    else
        thing->subsector->sector->thinglist = thing->snext;

    if (!(testflags & MF_NOBLOCKMAP))
    {
        if (thing->bnext)
            thing->bnext->bprev = thing->bprev;
        if (thing->bprev)
            thing->bprev->bnext = thing->bnext;
        else
	    {
            blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
            blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;
            blocklinks[blocky*bmapwidth+blockx] = thing->bnext;
	    }
    }
}

void PB_SetThingPosition(mobj_t *thing)
{
	subsector_t		*ss;
	sector_t		*sec;
	int				blockx, blocky;
	mobj_t			**link;
	
/* */
/* link into subsector */
/* */
    ss = testsubsec;

	thing->subsector = ss;
	/* invisible things don't go into the sector links */
	sec = ss->sector;
	
	thing->sprev = NULL;
	thing->snext = sec->thinglist;
	if (sec->thinglist)
		sec->thinglist->sprev = thing;
	sec->thinglist = thing;
	
/* */
/* link into blockmap */
/* */
	if ( ! (testflags & MF_NOBLOCKMAP) )
	{	/* inert things don't need to be in blockmap		 */
		blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
		blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;
		if (blockx>=0 && blockx < bmapwidth && blocky>=0 && blocky <bmapheight)
		{
			link = &blocklinks[blocky*bmapwidth+blockx];
			thing->bprev = NULL;
			thing->bnext = *link;
			if (*link)
				(*link)->bprev = thing;
			*link = thing;
		}
		else
		{	/* thing is off the map */
			thing->bnext = thing->bprev = NULL;
		}
	}
}

boolean PB_CheckPosition(void)
{
    int xl;
    int xh;
    int yl;
    int yh;
    int bx;
    int by;
    int r;

    testflags = basething->flags;

    r = basething->radius;
    testbbox[BOXTOP] = testy + r;
    testbbox[BOXBOTTOM] = testy - r;
    testbbox[BOXRIGHT] = testx + r;
    testbbox[BOXLEFT] = testx - r;

    testsubsec = PB_PointInSubsector(testx, testy);

    testfloorz = testdropoffz = testsubsec->sector->floorheight;
    testceilingz = testsubsec->sector->ceilingheight;

    ++validcount;

    ceilingline = NULL;
    hitthing = NULL;

    xl = (testbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (testbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (testbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (testbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

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
        {
            if (!PB_MoveThingsIterator(bx,by))
            {
                return false;
            }
            if (!PB_MoveLinesIterator(bx,by))
            {
                return false;
            }
        }

    return true;
}


int PB_PointOnSide(fixed_t x, fixed_t y, node_t *node)
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

subsector_t *PB_PointInSubsector(fixed_t x, fixed_t y)
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
        side = PB_PointOnSide(x, y, node);
        nodenum = node->children[side];
    }

    return &subsectors[nodenum & ~NF_SUBSECTOR];
}

boolean PB_BoxCrossLine(line_t *ld)
{
    fixed_t x1, y1, x2, y2;
    fixed_t lx, ly;
    fixed_t ldx, ldy;
    fixed_t dx1, dy1;
    fixed_t dx2, dy2;
    boolean side1, side2;

    if (testbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
        || testbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || testbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
        || testbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
        return false;

    y1 = testbbox[BOXTOP];
    y2 = testbbox[BOXBOTTOM];

    if (ld->slopetype == ST_POSITIVE)
    {
        x1 = testbbox[BOXLEFT];
        x2 = testbbox[BOXRIGHT];
    }
    else
    {
        x1 = testbbox[BOXRIGHT];
        x2 = testbbox[BOXLEFT];
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

boolean PB_CheckLine(line_t *ld)
{
    fixed_t opentop, openbottom;
    fixed_t lowfloor;
    sector_t *front, *back;

    if (!ld->backsector)
        return false;

    if (!(testflags & MF_MISSILE)
        && (ld->flags & (ML_BLOCKING|ML_BLOCKMONSTERS)))
            return false;

    front = ld->frontsector;
    back = ld->backsector;

    if (front->ceilingheight < back->ceilingheight)
        opentop = front->ceilingheight;
    else
        opentop = back->ceilingheight;
    if (front->floorheight > back->floorheight)
    {
        openbottom = front->floorheight;
        lowfloor = back->floorheight;
    }
    else
    {
        openbottom = back->floorheight;
        lowfloor = front->floorheight;
    }

    if (opentop < testceilingz)
    {
        testceilingz = opentop;
        ceilingline = ld;
    }
    if (openbottom > testfloorz)
        tmfloorz = openbottom;
    if (lowfloor < testdropoffz)
        testdropoffz = lowfloor;

    return true;
}

boolean PB_CheckThing(mobj_t *thing)
{
    fixed_t blockdist;
    int delta;

    if (!(thing->flags & MF_SOLID ))
        return true;
    blockdist = thing->radius + basething->radius;

    delta = thing->x - testx;
    if (delta < 0)
        delta = -delta;
    if (delta >= blockdist)
        return true;
    delta = thing->y - testy;
    if (delta < 0)
        delta = -delta;
    if (delta >= blockdist)
        return true;

    if (thing == basething)
        return true;

    if (testflags & MF_SKULLFLY)
    {
        hitthing = thing;
        return false;
    }

    if (testflags & MF_MISSILE)
    {
        if (basething->z > thing->z + thing->height)
            return true;
        if (basething->z + basething->height < thing->z)
            return true;
        if (basething->target->type == thing->type)
        {
            if (thing == basething->target)
                return true;
            if (thing->type != MT_PLAYER)
                return false;
        }
        if (!(thing->flags & MF_SHOOTABLE))
            return !(thing->flags & MF_SOLID);

        hitthing = thing;
        return false;
    }

    return !(thing->flags & MF_SOLID);
}

boolean PB_MoveLinesIterator(int x, int y)
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

        if (!PB_BoxCrossLine(ld))
            continue;

        if (!PB_CheckLine(ld))
            return false;
    }

    return true;
}

boolean PB_MoveThingsIterator(int x, int y)
{
    mobj_t *mobj;

    for (mobj = blocklinks[y*bmapwidth+x]; mobj; mobj->bnext)
    {
        if (!PB_CheckThing(mobj))
            return false;
    }
    return true;
}
