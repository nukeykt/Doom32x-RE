/* s_sound.c */
#include "doomdef.h"
#include "marsonly.h"

sfxchannel_t	sfxchannels[SFXCHANNELS];

int DAT_06007260;

int				finalquad;			/* the last quad mixed by update. */

#define abs(x) ((x)<0 ? -(x) : (x))


/*
==================
=
= S_Init
=
==================
*/

void S_Init(void)
{
	int i;
	sfxchannel_t	*channel;
  	for (channel=sfxchannels,i=0 ; i<SFXCHANNELS ; i++,channel++)
	{
		channel->id = i;
	}
}

/*
==================
=
= S_StartSound
=
==================
*/

int FUN_0204e7b2(int a1);
void SND_Start(int a1, int a2, int a3);

void S_StartSound(mobj_t *origin, int sound_id)
{
	sfxchannel_t	*channel, *newchannel;
	int 			i;
	int 		dist_approx;
	player_t 	*player;
	int 		dx, dy;
	short		vol;
	int vol2;
	sfxinfo_t	*sfx;

	if (DAT_06000810)
		return;

/* Get sound effect data pointer */
	sfx = &S_sfx[sound_id];

	player = &players[consoleplayer];
	
	newchannel = NULL;
	
/* reject sounds started at the same instant and singular sounds */
	for (channel=sfxchannels,i=0 ; i<SFXCHANNELS ; i++,channel++)
	{
		if (channel->sfx == sfx)
		{
			if (channel->startquad == finalquad)
			{
				return;		/* exact sound allready started */
			}

			if (sfx->singularity)
			{
				newchannel = channel;	/* overlay this	 */
				goto gotchannel;
			}
		}
		if (channel->origin == origin)
		{	/* cut off whatever was coming from this origin */
			newchannel = channel;
			goto gotchannel;
		}

		if (!FUN_0204e7b2(channel->id))
			newchannel = channel;	/* this is a dead channel, ok to reuse */
	}

/* if there weren't any dead channels, try to kill an equal or lower */
/* priority channel */

	if (!newchannel)
	{
		for (newchannel=sfxchannels,i=0 ; i<SFXCHANNELS ; i++, newchannel++)
			if (newchannel->priority <= sfx->priority)
				goto gotchannel;
		if (origin == player->mo)
		{
			SND_Start(DAT_06007260, sound_id, 255);
			DAT_06007260 ^= 1;
		}
		return;		/* couldn't override a channel */
	}


/* */
/* fill in the new values */
/* */
gotchannel:
	newchannel->sfx = sfx;
	newchannel->origin = origin;
	newchannel->startquad = finalquad;

	if (!origin || origin == player->mo)
	{
		SND_Start(newchannel->id, sound_id, 255);
	}
	else
	{
		dx = abs(origin->x - player->mo->x);
		dy = abs(origin->y - player->mo->y);
		dist_approx = dx + dy - ((dx < dy ? dx : dy) >> 1);
		vol = dist_approx >> 20;
		if (vol > 127)
			return;		/* too far away */
		vol2 = (127 - vol) << 1;
		if (vol2 < 0)
			return;

		SND_Start(newchannel->id, sound_id, vol2);
	}
}
