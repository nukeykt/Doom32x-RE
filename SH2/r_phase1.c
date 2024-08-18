#include "doomdef.h"
#include "r_local.h"

typedef	struct
{
    int	first;
    int last;
} cliprange_t;


#define MAXSEGS		32

/* newend is one past the last valid seg */
cliprange_t* newend;
cliprange_t	solidsegs[MAXSEGS];

seg_t *curline;
angle_t lineangle1;
sector_t *frontsector;

void R_RenderBSPNode(int a1);

void R_BSP(void)
{
    solidsegs[0].first = -2;
    solidsegs[0].last = -1;
    solidsegs[1].first = SCREENWIDTH;
    solidsegs[1].last = SCREENWIDTH+1;

    newend = solidsegs + 2;

    R_RenderBSPNode(numnodes - 1);

    phasetime[1] = samplecount;
}

void R_StoreWallRange(int start, int stop)
{
    viswall_t *rw;

    rw = lastwallcmd++;
    rw->seg = curline;
    rw->start = start;
    rw->stop = stop;
    rw->angle1 = lineangle1;
}

void R_ClipSolidWallSegment(int first, int last)
{
    cliprange_t *next;
    cliprange_t *start;

    start = solidsegs;
    while (start->last < first-1)
        start++;

    if (first < start->first)
    {
        if (last < start->first-1)
        {
            R_StoreWallRange(first, last);
            next = newend;
            newend++;
            while (next != start)
            {
                *next = *(next-1);
                next--;
            }
            next->first = first;
            next->last = last;
            return;
        }
        R_StoreWallRange(first, start->first - 1);
        start->first = first;
    }

    if (last <= start->last)
        return;

    next = start;
    while (last >= (next+1)->first-1)
    {
        R_StoreWallRange(next->last + 1, (next+1)->first - 1);
        next++;

        if (last <= next->last)
        {
            start->last = next->last;
            goto crunch;
        }
    }

    R_StoreWallRange(next->last + 1, last);
    start->last = last;

crunch:
    if (next == start)
        return;

    while (next++ != newend)
    {
        *++start = *next;
    }
    newend = start + 1;
}

void R_ClipPassWallSegment(int first, int last)
{
    cliprange_t *start;

    start = solidsegs;
    while (start->last < first-1)
        start++;

    if (first < start->first)
    {
        if (last < start->first-1)
        {
            R_StoreWallRange(first, last);
            return;
        }

        R_StoreWallRange(first, start->first - 1);
    }

    if (last <= start->last)
        return;

    while (last >= (start+1)->first-1)
    {
        R_StoreWallRange(start->last + 1, (start+1)->first - 1);
        start++;

        if (last <= start->last)
            return;
    }

    R_StoreWallRange(start->last + 1, last);
}

int P1_SlopeDiv (unsigned num, unsigned den)
{
	unsigned ans;
	if (den < 512)
		return SLOPERANGE;
	ans = (num<<3)/(den>>8);
	return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

angle_t P1_PointToAngle (fixed_t x, fixed_t y)
{	
	x -= viewx;
	y -= viewy;
	
	if ( (!x) && (!y) )
		return 0;
	if (x>= 0)
	{	/* x >=0 */
		if (y>= 0)
		{	/* y>= 0 */
			if (x>y)
				return tantoangle[ P1_SlopeDiv(y,x)];     /* octant 0 */
			else
				return ANG90-1-tantoangle[ P1_SlopeDiv(x,y)];  /* octant 1 */
		}
		else
		{	/* y<0 */
			y = -y;
			if (x>y)
				return -tantoangle[P1_SlopeDiv(y,x)];  /* octant 8 */
			else
				return ANG270+tantoangle[ P1_SlopeDiv(x,y)];  /* octant 7 */
		}
	}
	else
	{	/* x<0 */
		x = -x;
		if (y>= 0)
		{	/* y>= 0 */
			if (x>y)
				return ANG180-1-tantoangle[ P1_SlopeDiv(y,x)]; /* octant 3 */
			else
				return ANG90+ tantoangle[ P1_SlopeDiv(x,y)];  /* octant 2 */
		}
		else
		{	/* y<0 */
			y = -y;
			if (x>y)
				return ANG180+tantoangle[ P1_SlopeDiv(y,x)];  /* octant 4 */
			else
				return ANG270-1-tantoangle[ P1_SlopeDiv(x,y)];  /* octant 5 */
		}
	}	
#ifndef LCC	
	return 0;
#endif
}

void R_AddLine(seg_t* line)
{
    int x1;
    int x2;
    angle_t angle1;
    angle_t angle2;
    angle_t span;
    angle_t tspan;
    sector_t *backsector;

    curline = line;

    angle1 = P1_PointToAngle(line->v1->x, line->v1->y);
    angle2 = P1_PointToAngle(line->v2->x, line->v2->y);

    span = angle1 - angle2;

    if (span >= ANG180)
        return;

    lineangle1 = angle1;
    angle1 -= viewangle;
    angle2 -= viewangle;

    tspan = angle1 + clipangle;
    if (tspan > doubleclipangle)
    {
        tspan -= doubleclipangle;

        if (tspan >= span)
            return;

        angle1 = clipangle;
    }

    tspan = clipangle - angle2;
    if (tspan > doubleclipangle)
    {
        tspan -= doubleclipangle;

        if (tspan >= span)
            return;

        angle2 = -clipangle;
    }

    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;

    x1 = viewangletox[angle1];
    x2 = viewangletox[angle2];

    if (x1 == x2)
        return;

    backsector = line->backsector;

    if (!backsector)
        goto clipsolid;

    if (backsector->ceilingheight <= frontsector->floorheight
        || backsector->floorheight >= frontsector->ceilingheight)
        goto clipsolid;

    if (backsector->ceilingheight != frontsector->ceilingheight
        || backsector->floorheight != frontsector->floorheight)
        goto clippass;

    if (backsector->ceilingpic == frontsector->ceilingpic
        && backsector->floorpic == frontsector->floorpic
        && backsector->lightlevel == frontsector->lightlevel
        && curline->sidedef->midtexture == 0)
        return;

clippass:
    R_ClipPassWallSegment(x1, x2-1);
    return;

clipsolid:
    R_ClipSolidWallSegment(x1, x2-1);
}

int	checkcoord[12][4] =
{
    {3,0,2,1},
    {3,0,2,0},
    {3,1,2,0},
    {0},
    {2,0,2,1},
    {0,0,0,0},
    {3,1,3,0},
    {0},
    {2,0,3,1},
    {2,1,3,1},
    {2,1,3,0}
};

boolean R_CheckBBox(fixed_t* bspcoord)
{
    int boxx;
    int boxy;
    int boxpos;

    fixed_t x1;
    fixed_t y1;
    fixed_t x2;
    fixed_t y2;

    angle_t angle1;
    angle_t angle2;
    angle_t span;
    angle_t tspan;

    cliprange_t* start;

    int sx1;
    int sx2;


    if (viewx <= bspcoord[BOXLEFT])
        boxx = 0;
    else if (viewx < bspcoord[BOXRIGHT])
        boxx = 1;
    else
        boxx = 2;

    if (viewy >= bspcoord[BOXTOP])
        boxy = 0;
    else if (viewy < bspcoord[BOXBOTTOM])
        boxy = 1;
    else
        boxy = 2;

    boxpos = (boxy<<2)+boxx;

    if (boxpos == 5)
        return true;

    x1 = bspcoord[checkcoord[boxpos][0]];
    y1 = bspcoord[checkcoord[boxpos][1]];
    x2 = bspcoord[checkcoord[boxpos][2]];
    y2 = bspcoord[checkcoord[boxpos][3]];

    angle1 = P1_PointToAngle(x1, y1) - viewangle;
    angle2 = P1_PointToAngle(x2, y2) - viewangle;

    span = angle1 - angle2;

    if (span >= ANG180)
        return true;

    tspan = angle1 + clipangle;
    if (tspan > doubleclipangle)
    {
        tspan -= doubleclipangle;

        if (tspan >= span)
            return;

        angle1 = clipangle;
    }

    tspan = clipangle - angle2;
    if (tspan > doubleclipangle)
    {
        tspan -= doubleclipangle;

        if (tspan >= span)
            return;

        angle2 = -clipangle;
    }

    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;

    sx1 = viewangletox[angle1];
    sx2 = viewangletox[angle2];

    if (sx1 == sx2)
        return false;
    sx2--;

    start = solidsegs;
    while (start->last < sx2)
        start++;

    if (sx1 >= start->first && sx2 <= start->last)
        return false;

    return true;
}


void R_Subsector(int num)
{
    int count;
    seg_t* line;
    subsector_t* sub;

    if (num >= numsubsectors)
        I_Error("R_Subsector: ss %i with numss = %i",
            num,
            numsubsectors);

    sub = &subsectors[num];
    frontsector = sub->sector;
    *lastvissubsector = sub;
    lastvissubsector++;
    count = sub->numlines;
    line = &segs[sub->firstline];

    while (count--)
    {
        R_AddLine(line);
        line++;
    }
}

void R_RenderBSPNode(int bspnum)
{
    node_t* bsp;

    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1)
            R_Subsector(0);
        else
            R_Subsector(bspnum & (~NF_SUBSECTOR));
        return;
    }

    bsp = &nodes[bspnum];

    if (((viewy - bsp->y) >> 16) * (bsp->dx >> 16) < ((viewx - bsp->x) >> 16) * (bsp->dy >> 16))
    {
        R_RenderBSPNode(bsp->children[0]);
        if (R_CheckBBox(bsp->bbox[1]))
            R_RenderBSPNode(bsp->children[1]);
    }
    else
    {
        R_RenderBSPNode(bsp->children[1]);
        if (R_CheckBBox(bsp->bbox[0]))
            R_RenderBSPNode(bsp->children[0]);
    }
}
