/* r_main.c */

#include "doomdef.h"
#include "r_local.h"
#include "marsonly.h"

/*===================================== */

/* */
/* subsectors */
/* */
subsector_t		**lastvissubsector;

/* */
/* walls */
/* */
viswall_t	*lastwallcmd;

/* */
/* planes */
/* */
visplane_t	*lastvisplane;

/* */
/* sprites */
/* */
vissprite_t	vissprites[MAXVISSPRITES], *lastsprite_p, *vissprite_p;

/* */
/* openings / misc refresh memory */
/* */
unsigned short	openings[MAXOPENINGS], *lastopening;


/*===================================== */

boolean		phase1completed;

pixel_t		*workingscreen;


fixed_t		viewx, viewy, viewz;
angle_t		viewangle;
fixed_t		viewcos, viewsin;
player_t	*viewplayer;

int			validcount = 1;		/* increment every time a check is made */
int			framecount;		/* incremented every frame */


boolean		fixedcolormap;

int			lightlevel;			/* fixed light level */
int			extralight;			/* bumped light from gun blasts */

/* */
/* sky mapping */
/* */
int			skytexture;


/* */
/* precalculated math */
/* */
angle_t		clipangle,doubleclipangle;
fixed_t		*finecosine = &finesine[FINEANGLES/4];

/*
===============================================================================
=
= R_PointToAngle
=
===============================================================================
*/


extern	int	tantoangle[SLOPERANGE+1];

int SlopeDiv (unsigned num, unsigned den)
{
	unsigned ans;
	if (den < 512)
		return SLOPERANGE;
	ans = (num<<3)/(den>>8);
	return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{	
	int		x;
	int		y;
	
	x = x2 - x1;
	y = y2 - y1;
	
	if ( (!x) && (!y) )
		return 0;
	if (x>= 0)
	{	/* x >=0 */
		if (y>= 0)
		{	/* y>= 0 */
			if (x>y)
				return tantoangle[ SlopeDiv(y,x)];     /* octant 0 */
			else
				return ANG90-1-tantoangle[ SlopeDiv(x,y)];  /* octant 1 */
		}
		else
		{	/* y<0 */
			y = -y;
			if (x>y)
				return -tantoangle[SlopeDiv(y,x)];  /* octant 8 */
			else
				return ANG270+tantoangle[ SlopeDiv(x,y)];  /* octant 7 */
		}
	}
	else
	{	/* x<0 */
		x = -x;
		if (y>= 0)
		{	/* y>= 0 */
			if (x>y)
				return ANG180-1-tantoangle[ SlopeDiv(y,x)]; /* octant 3 */
			else
				return ANG90+ tantoangle[ SlopeDiv(x,y)];  /* octant 2 */
		}
		else
		{	/* y<0 */
			y = -y;
			if (x>y)
				return ANG180+tantoangle[ SlopeDiv(y,x)];  /* octant 4 */
			else
				return ANG270-1-tantoangle[ SlopeDiv(x,y)];  /* octant 5 */
		}
	}	
#ifndef LCC	
	return 0;
#endif
}


/*
===============================================================================
=
= R_PointOnSide
=
= Returns side 0 (front) or 1 (back)
===============================================================================
*/

static int	R_PointOnSide (int x, int y, node_t *node)
{
	fixed_t	dx,dy;
	fixed_t	left, right;

	if (!node->dx)
	{
		if (x <= node->x)
			return node->dy > 0;
		return node->dy < 0;
	}
	if (!node->dy)
	{
		if (y <= node->y)
			return node->dx < 0;
		return node->dx > 0;
	}
	
	dx = (x - node->x);
	dy = (y - node->y);
	
	left = (node->dy>>16) * (dx>>16);
	right = (dy>>16) * (node->dx>>16);
	
	if (right < left)
		return 0;		/* front side */
	return 1;			/* back side */
}



/*
==============
=
= R_PointInSubsector
=
==============
*/

struct subsector_s *R_PointInSubsector (fixed_t x, fixed_t y)
{
	node_t	*node;
	int		side, nodenum;
	
	if (!numnodes)				/* single subsector is a special case */
		return subsectors;
		
	nodenum = numnodes-1;

	while (! (nodenum & NF_SUBSECTOR) )
	{
		node = &nodes[nodenum];
		side = R_PointOnSide (x, y, node);
		nodenum = node->children[side];
	}
	
	return &subsectors[nodenum & ~NF_SUBSECTOR];
	
}

/*============================================================================= */


/*
==============
=
= R_Init
=
==============
*/

void R_Init (void)
{
D_printf ("R_InitData\n");
	R_InitData ();
D_printf ("Done\n");

	clipangle = xtoviewangle[0];
	doubleclipangle = clipangle*2;
	
	framecount = 0;
	viewplayer = &players[0];
}

/*============================================================================= */

extern int ticsinframe;

extern int checkpostics, shoottics;
extern int lasttics;

extern	int	playertics, thinkertics, sighttics, basetics, latetics;
extern	int	tictics;

extern	int		soundtics;

/*
===================
=
= R_DebugScreen
=
===================
*/

void R_DebugScreen (void)
{
#ifdef JAGUAR

#if 1
	PrintNumber (15,1, vblsinframe);

	PrintNumber (15,2, phasetime[8] - phasetime[0]);
	PrintNumber (15,3, tictics);
	PrintNumber (15,4, soundtics);
#endif

#if 1
	PrintNumber (15,6, playertics);
	PrintNumber (15,7, thinkertics);
	PrintNumber (15,8, sighttics);
	PrintNumber (15,9, basetics);
	PrintNumber (15,10, latetics);
#endif

#if 0
	int	i;
	
	PrintNumber (15,1, phasetime[8] - phasetime[0]);
	for (i=0 ; i < 8 ; i++)
		PrintNumber (15,3+i,phasetime[i+1]-phasetime[i]);
#endif


#endif

}
/*============================================================================= */

int	shadepixel;
extern	int	workpage;
extern	pixel_t	*screens[2];	/* [SCREENWIDTH*SCREENHEIGHT];  */

/*
==================
=
= R_Setup
=
==================
*/

void R_Setup (void)
{
	int		damagecount, bonuscount;
	player_t *player;
	
/* */
/* set up globals for new frame */
/* */
	workingscreen = (pixel_t*)framebuffer_p;

#ifdef JAGUAR	
	*(pixel_t  **)0xf02224 = workingscreen;	/* a2 base pointer */
	*(int *)0xf02234 = 0x10000;				/* a2 outer loop add (+1 y) */
	*(int *)0xf0226c = *(int *)0xf02268 = 0;		/* pattern compare */
#endif

	framecount++;	
	validcount++;
		
	viewplayer = player = &players[displayplayer];
	viewx = player->mo->x;
	viewy = player->mo->y;
	viewz = player->viewz;
	viewangle = player->mo->angle;

	viewsin = finesine[viewangle>>ANGLETOFINESHIFT];
	viewcos = finecosine[viewangle>>ANGLETOFINESHIFT];
		
	extralight = player->extralight << 6;
	fixedcolormap = player->fixedcolormap;
		
	player = &players[consoleplayer];

	damagecount = player->damagecount;
	bonuscount = player->bonuscount;

	if (bonuscount > 4)
		bonuscount = 4;
	if (damagecount)
	{
		if (damagecount < 5)
			shadepixel = 1;
		else if (damagecount < 9)
			shadepixel = 2;
		else if (damagecount < 13)
			shadepixel = 3;
		else if (damagecount < 17)
			shadepixel = 4;
		else if (damagecount < 21)
			shadepixel = 5;
		else
			shadepixel = 6;
	}
	else if (bonuscount)
		shadepixel = bonuscount + 8;
	else if (player->powers[pw_invulnerability] > 60
		|| (player->powers[pw_invulnerability] & 4))
		shadepixel = 12;
	else if (player->powers[pw_ironfeet] > 60
		|| (player->powers[pw_ironfeet] & 4))
		shadepixel = 13;
	else if (player->powers[pw_strength]
		&& (player->powers[pw_strength] < 64))
		shadepixel = 9 - (player->powers[pw_strength] >> 3);
	else
		shadepixel = 0;

/* */
/* plane filling */
/*	 */
	lastvisplane = (visplane_t*)0x2401329c;		/* visplanes[0] is left empty */
	lastwallcmd = (viswall_t*)0x2400f980;			/* no walls added yet */
	lastvissubsector = (subsector_t**)0x2400f580;	/* no subsectors visible yet */
	
/*	 */
/* clear sprites */
/* */
	vissprite_p = vissprites;
	lastopening = openings;

	phasetime[0] = samplecount;
}


void R_BSP (void);
void R_WallPrep (void);
void R_SpritePrep (void);
boolean R_LatePrep (void);
void R_Cache (void);
void R_SegCommands (void);
void R_DrawPlanes (void);
void R_Sprites (void);
void R_Update (void);


/*
==============
=
= R_RenderView
=
==============
*/

int		phasetime[9];

extern	ref1_start;
extern	ref2_start;
extern	ref3_start;
extern	ref4_start;
extern	ref5_start;
extern	ref6_start;
extern	ref7_start;
extern	ref8_start;


extern	boolean	debugscreenactive;

void R_RenderPlayerView (void)
{
/* */
/* initial setup */
/* */
	FUN_02037058 ();

	R_Setup ();
	R_BSP ();
	R_WallPrep ();
	R_SpritePrep ();
/* the rest of the refresh can be run in parallel with the next game tic */
	if (R_LatePrep ())
		R_Cache ();
	R_SegCommands ();
	R_DrawPlanes ();
	R_Sprites ();
	R_Update ();
}

