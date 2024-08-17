#include "doomdef.h"
#include "p_local.h"
#include "marsonly.h"

int	playertics, thinkertics, sighttics, basetics, latetics;
int	tictics;

boolean		gamepaused;
jagobj_t	*pausepic;

/*
===============================================================================

								THINKERS

All thinkers should be allocated by Z_Malloc so they can be operated on uniformly.  The actual
structures will vary in size, but the first element must be thinker_t.

Mobjs are similar to thinkers, but kept seperate for more optimal list
processing
===============================================================================
*/

thinker_t	thinkercap;	/* both the head and tail of the thinker list */
mobj_t		mobjhead;	/* head and tail of mobj list */
int			activethinkers;	/* debug count */
int			activemobjs;	/* debug count */

/*
===============
=
= P_InitThinkers
=
===============
*/

void P_InitThinkers (void)
{
	thinkercap.prev = thinkercap.next  = &thinkercap;
	mobjhead.next = mobjhead.prev = &mobjhead;
}


/*
===============
=
= P_AddThinker
=
= Adds a new thinker at the end of the list
=
===============
*/

void P_AddThinker (thinker_t *thinker)
{
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;
}


/*
===============
=
= P_RemoveThinker
=
= Deallocation is lazy -- it will not actually be freed until its
= thinking turn comes up
=
===============
*/

void P_RemoveThinker (thinker_t *thinker)
{
	thinker->function = (think_t)-1;
}

/*
===============
=
= P_RunThinkers
=
===============
*/

void P_RunThinkers (void)
{
	thinker_t	*currentthinker;
	
	activethinkers = 0;
	
	currentthinker = thinkercap.next;
	while (currentthinker != &thinkercap)
	{
		if (currentthinker->function == (think_t)-1)
		{	/* time to remove it */
			currentthinker->next->prev = currentthinker->prev;
			currentthinker->prev->next = currentthinker->next;
			Z_Free (currentthinker);
		}
		else
		{
			if (currentthinker->function)
			{
				currentthinker->function (currentthinker);
			}
			activethinkers++;
		}
		currentthinker = currentthinker->next;
	}
}


/*============================================================================= */

/*
===============
=
= P_CheckSights
=
= Check sights of all mobj thinkers that are going to change state this
= tic and have MF_COUNTKILL set
===============
*/

void P_CheckSights2 ();

void P_CheckSights (void)
{
#ifdef JAGUAR
	extern	int p_sight_start;
	DSPFunction (&p_sight_start);
#else
	P_CheckSights2 ();
#endif
}

/*============================================================================= */

/* 
=================== 
= 
= P_RunMobjBase  
=
= Run stuff that doesn't happen every tick
=================== 
*/ 

void	P_RunMobjBase (void)
{
#ifdef JAGUAR
	extern	int p_base_start;
	 
	DSPFunction (&p_base_start);
#else
	P_RunMobjBase2 ();
#endif
}

/*============================================================================= */

/* 
=================== 
= 
= P_RunMobjLate  
=
= Run stuff that doesn't happen every tick
=================== 
*/ 

void	P_RunMobjLate (void)
{
	mobj_t	*mo;
	mobj_t	*next;
	
	for (mo=mobjhead.next ; mo != &mobjhead ; mo=next)
	{
		next = mo->next;	/* in case mo is removed this time */
		if (mo->latecall)
		{
			mo->latecall(mo);
		}
	}
}

/*============================================================================= */

/*
==============
=
= P_CheckCheats
=
==============
*/
 
void P_CheckCheats (void)
{
	int		buttons, oldbuttons;
	int		i;
	player_t	*p;
	
	for (i=0 ; i<MAXPLAYERS ; i++)
	{
		if (!playeringame[i])
			continue;
		buttons = ticbuttons[i];
		oldbuttons = oldticbuttons[i];
	
		if ( (buttons & BT_PAUSE) && !(oldbuttons&BT_PAUSE) )
			gamepaused ^= 1;
	}
		
	buttons = ticbuttons[0];
	oldbuttons = oldticbuttons[0];

	if (buttons == 0x400 && oldbuttons != 0x4000)
	{	/* free stuff */
		cheated = true;
		p=&players[0];
		for (i=0 ; i<NUMCARDS ; i++)
			p->cards[i] = true;			
		p->armorpoints = 200;
		p->armortype = 2;
		for (i=0;i<NUMWEAPONS;i++) p->weaponowned[i] = true;
		for (i=0;i<NUMAMMO;i++) p->ammo[i] = p->maxammo[i] = 500;
	}
	if (buttons == 0x200 && oldbuttons != 0x200)
	{	/* godmode */
		cheated = true;
		players[0].cheats ^= CF_GODMODE;
	}
}
  

int playernum;

void G_DoReborn (int playernum); 

/*
=================
=
= P_Ticker
=
=================
*/

int		ticphase;

int P_Ticker (void)
{
	int		start;
	int		ticstart;
	player_t	*pl;

	ticphase = 0;
	
	ticstart = samplecount;

	gameaction = ga_nothing;
	
	gametic++; 		 
 
/* */
/* check for pause and cheats */
/* */
	P_CheckCheats ();

/* */
/* run player actions */
/* */
	start = samplecount;
	for (playernum=0,pl=players ; playernum<MAXPLAYERS ; playernum++,pl++)
		if (playeringame[playernum])
		{
			if (pl->playerstate == PST_REBORN) 
				G_DoReborn (playernum); 
			ticphase = 11;
			AM_Control (pl);
			ticphase = 12;
			P_PlayerThink (pl);
			ticphase = 13;
		}
	playertics = samplecount - start;
	
	ticphase = 1;
	
	start = samplecount;
	P_RunThinkers ();
	thinkertics = samplecount - start;

	ticphase = 2;
		
	start = samplecount;
	P_CheckSights ();	
	sighttics = samplecount - start;

	ticphase = 3;

	start = samplecount;
	P_RunMobjBase ();
	basetics = samplecount - start;

	ticphase = 4;

	start = samplecount;
	P_RunMobjLate ();
	latetics = samplecount - start;

	ticphase = 5;

	P_UpdateSpecials ();

	P_RespawnSpecials ();

	ticphase = 6;
	
	ST_Ticker ();			/* update status bar */

	AM_Ticker ();

	ticphase = 7;
		
	tictics = samplecount - ticstart;
	
	return gameaction;		/* may have been set to ga_died, ga_completed, */
							/* or ga_secretexit */
}


/* 
============= 
= 
= DrawPlaque 
= 
============= 
*/ 
 
void FUN_02038b0c (void)
{
}


/* 
============= 
= 
= P_Drawer 
= 
= draw current display 
============= 
*/ 
 
 
void P_Drawer (void) 
{ 	
	ST_Drawer ();
	R_RenderPlayerView ();
	if (players[consoleplayer].automapflags & AF_ACTIVE)
		AM_Drawer ();

	I_Update ();
} 

void P_Start (void)
{
}

void P_Stop (void)
{
}

