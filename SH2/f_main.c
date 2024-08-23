/* f_main.c -- finale */

#include "doomdef.h"
#include "r_local.h"

extern int mystrlen (char *string);

extern byte *framebuffer_p;

/*
==================
=
= BufferedDrawSprite
=
= Cache and draw a game sprite to the 8 bit buffered screen
==================
*/

void BufferedDrawSprite (int sprite, int frame, int rotation)
{
	patch_t		*patch;
	byte		*pixels, *src, *dest, pix;
	int			count;
	int			x, sprleft, sprtop;
	column_t	*column;
	int			lump;
	int			texturecolumn;
	
	if ((unsigned)sprite >= NUMSPRITES)
		I_Error ("BufferedDrawSprite: invalid sprite number %i "
		,sprite);

	lump = spritelump[sprite] * 2 + frame;

	patch = (patch_t *)W_POINTLUMPNUM(lump);
	pixels = Z_Malloc (lumpinfo[lump+1].size, PU_STATIC, NULL);
	W_ReadLump (lump+1,pixels);
	 	
/* */
/* coordinates are in a 160*112 screen (doubled pixels) */
/* */
	sprtop = 90;
	sprleft = 80;
	
	sprtop -= patch->topoffset;
	sprleft -= patch->leftoffset;
	
/* */
/* draw it by hand */
/* */
	for (x=0 ; x<patch->width ; x++)
	{
		texturecolumn = x;
			
		column = (column_t *) ((byte *)patch +
		 BIGSHORT(patch->columnofs[texturecolumn]));

/* */
/* draw a masked column */
/* */
		for ( ; column->topdelta != 0xff ; column++) 
		{
	/* calculate unclipped screen coordinates for post */
			dest = framebuffer_p + (short)(sprtop+column->topdelta)*(short)640+(sprleft + x)*2;
			count = column->length;
			src = pixels + column->dataofs;
			while (count--)
			{
				pix = *src++;
				if (!pix)
					pix = 8;
				dest[0] = dest[1] = dest[320] = dest[321] = pix;
				dest += 640;
			}
		}
	}
	
	Z_Free (pixels);	
}


/*============================================================================ */

int DAT_06000fbc = 0;
int DAT_06000fc0 = 0;

typedef struct
{
	char		*name;
	mobjtype_t	type;
} castinfo_t;

castinfo_t	castorder[] = {
{"\"Zombieman\"", MT_POSSESSED},
{"\"Shotgun Guy\"", MT_SHOTGUY},
{"\"Imp\"", MT_TROOP},
{"\"Demon\"", MT_SERGEANT},
{"\"Lost Soul\"", MT_SKULL},
{"\"Cacodemon\"", MT_HEAD},
{"\"Baron of Hell\"", MT_BRUISER},

{NULL,0}
};

int			castnum;
int			casttics;
state_t		*caststate;
boolean		castdeath;
int			castframes;
int			castonmelee;
boolean		castattacking;

typedef enum
{
	fin_endtext,
	fin_charcast
} final_e;

final_e	status;
#define TEXTTIME	3
#define STARTX		8
#define STARTY		8
boolean textprint;	/* is endtext done printing? */
int		textindex;
int		textdelay;
int		text_x;
int		text_y;
#define SPACEWIDTH	8
#define NUMENDOBJ	28
jagobj_t	*endobj[NUMENDOBJ];
#if 0
/* '*' = newline */
char	endtextstring[] =
	"you did it! by turning*"
	"the evil of the horrors*"
	"of hell in upon itself*"
	"you have destroyed the*"
	"power of the demons.**"
	"their dreadful invasion*"
	"has been stopped cold!*"
	"now you can retire to*"
	"a lifetime of frivolity.**"
	"  congratulations!";
#endif

/* '*' = newline */
char	endtextstring[] =
	"ID Software salutes you!*"
	"*"
	"The horrors of hell*"
	"  could not kill you.*"
	"  Their most cunning*"
	"  traps were no match*"
	"  for you.  You have*"
	"  proven yourself the*"
	"  best of all!*"
	"*"
	"   Congratulations!*"
	"8";

/*=============================================== */
/* */
/* Print a string in big font - LOWERCASE INPUT ONLY! */
/* */
/*=============================================== */
void F_PrintString(char *string, int v2)
{
	int		index;
	int		val;
	int x;
	int y;

	x = 8;
	y = 8;

	index = 0;
	while (--v2 != -1)
	{
		if (string[index] == '*')
		{
			x = 8;
			y++;
		}
		else if (string[index] == '8')
		{
			DAT_06000fbc = 1;
			return;
		}
		index++;
	}
}

#if 0
/*=============================================== */
/* */
/* Print character cast strings */
/* */
/*=============================================== */
void F_CastPrint(char *string)
{
	int		i,width,slen;
	
	width = 0;
	slen = mystrlen(string);
	for (i = 0;i < slen; i++)
		switch(string[i])
		{
			case ' ': width += SPACEWIDTH; break;
			default : width += endobj[string[i] - 'a']->width;
		}

	text_x = 160 - (width >> 1);
	text_y = 20;
	F_PrintString(string);
}
#endif

/*
=======================
=
= F_Start
=
=======================
*/

void _marsPri(void);
void FUN_020373b2(void);

void F_Start (void)
{
	status = fin_endtext;		/* END TEXT PRINTS FIRST */
	textprint = false;
	textindex = 0;
	textdelay = TEXTTIME;
	text_x = STARTX;
	text_y = STARTY;

	castnum = 0;
	caststate = &states[mobjinfo[castorder[castnum].type].seestate];
	casttics = caststate->tics;
	castdeath = false;
	castframes = 0;
	castonmelee = 0;
	castattacking = false;

	_marsPri();
	FUN_020373b2();
}

void F_Stop (void)
{
}




/*
=======================
=
= F_Ticker
=
=======================
*/

void SND_Start(int a1, int a2, int a3);

int F_Ticker (void)
{
	int		st;
	int		buttons, oldbuttons;

	
/* */
/* check for press a key to kill actor */
/* */
	buttons = ticbuttons[consoleplayer];
	oldbuttons = oldticbuttons[consoleplayer];
	
	if (status == fin_endtext)
	{
		if (textindex == (3*15)/TEXTTIME)
			textprint = true;
			
		if (( ((buttons & BT_A) && !(oldbuttons & BT_A) )
		|| ((buttons & BT_B) && !(oldbuttons & BT_B) )
		|| ((buttons & BT_C) && !(oldbuttons & BT_C) ) ) &&
		DAT_06000fbc == true)
		{
			status = fin_charcast;
/*			S_StartSound (NULL, mobjinfo[castorder[castnum].type].seesound); */
		}
		return 0;
	}
	
	if (!castdeath)
	{
		if ( ((buttons & BT_A) && !(oldbuttons & BT_A) )
		|| ((buttons & BT_B) && !(oldbuttons & BT_B) )
		|| ((buttons & BT_C) && !(oldbuttons & BT_C) ) )
		{
		/* go into death frame */
			castdeath = true;
			caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
			casttics = caststate->tics;
			castframes = 0;
			castattacking = false;
		}
	}
	

/* */
/* advance state */
/* */
	if (--casttics > 0)
		return 0;			/* not time to change state yet */
		
	if (caststate->tics == -1 || caststate->nextstate == S_NULL)
	{	/* switch from deathstate to next monster */
		castnum++;
		castdeath = false;
		if (castorder[castnum].name == NULL)
			castnum = 0;
		if (mobjinfo[castorder[castnum].type].seesound)
			SND_Start (1, mobjinfo[castorder[castnum].type].seesound, 255);
		caststate = &states[mobjinfo[castorder[castnum].type].seestate];
		castframes = 0;
	}
	else
	{	/* just advance to next state in animation */
		if (caststate == &states[S_PLAY_ATK1])
			goto stopattack;	/* Oh, gross hack! */
		st = caststate->nextstate;
		caststate = &states[st];
		castframes++;
/*============================================== */
/* sound hacks.... */
{
	int sfx;
	
		switch (st)
		{
		case S_PLAY_ATK1: sfx = sfx_shotgn; break;
		case S_POSS_ATK2: sfx = sfx_pistol; break;
		case S_SPOS_ATK2: sfx = sfx_shotgn; break;
		case S_TROO_ATK3: sfx = sfx_claw; break;
		case S_SARG_ATK2: sfx = sfx_sgtatk; break;
		case S_BOSS_ATK2: 
		case S_HEAD_ATK2: sfx = sfx_firsht; break;
		case S_SKULL_ATK2: sfx = sfx_sklatk; break;
		default: sfx = 0; break;
		}
		
		if (sfx)
			SND_Start (1, sfx, 255);
}
/*============================================== */
	}
	
	if (castframes == 12)
	{	/* go into attack frame */
		castattacking = true;
		if (castonmelee)
			caststate=&states[mobjinfo[castorder[castnum].type].meleestate];
		else
			caststate=&states[mobjinfo[castorder[castnum].type].missilestate];
		castonmelee ^= 1;
		if (caststate == &states[S_NULL])
		{
			if (castonmelee)
				caststate=
				&states[mobjinfo[castorder[castnum].type].meleestate];
			else
				caststate=
				&states[mobjinfo[castorder[castnum].type].missilestate];
		}
	}
	
	if (castattacking)
	{
		if (castframes == 24 
		||	caststate == &states[mobjinfo[castorder[castnum].type].seestate] )
		{
stopattack:
			castattacking = false;
			castframes = 0;
			caststate = &states[mobjinfo[castorder[castnum].type].seestate];
		}
	}
	
	casttics = caststate->tics;
	if (casttics == -1)
		casttics = 15;
		
	return 0;		/* finale never exits */
}



/*
=======================
=
= F_Drawer
=
=======================
*/

void FUN_020376c4(void);

void Prints(int a1, int a2, char* a3);

void sleep(int a1);
void _swapbuffers();

void F_Drawer (void)
{
		
	switch(status)
	{
		case fin_endtext:
			if (!--textdelay)
			{
				if (!DAT_06000fc0)
					SND_Start(0, sfx_telept, 255);
			}
			F_PrintString(endtextstring, textindex);
			textdelay = TEXTTIME;
			textindex++;
			DAT_06000fc0 = 1;
			break;
			
		case fin_charcast:
			FUN_020376c4 ();
				
			BufferedDrawSprite (caststate->sprite,
					caststate->frame&FF_FRAMEMASK,0);

			Prints(19 - (mystrlen(castorder[castnum].name)>>1), 3, castorder[castnum].name);
			break;
	}
	sleep(1);
	_swapbuffers();
	sleep(1);
}

