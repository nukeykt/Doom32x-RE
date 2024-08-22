/* st_main.c -- status bar */

#include "doomdef.h"
#include "st_main.h"

stbar_t	stbar;
/*jagobj_t* micronums[NUMMICROS];
int		micronums_x[NUMMICROS] = {249,261,272,249,261,272};
int		micronums_y[NUMMICROS] = {15,15,15,25,25,25};*/

int		facetics;
int		newface;
int		card_x[NUMCARDS] = {KEYX,KEYX,KEYX,KEYX+3, KEYX+3, KEYX+3};
int		card_y[NUMCARDS] = {BLUKEYY,YELKEYY,REDKEYY,BLUKEYY,YELKEYY,REDKEYY};

boolean	gibdraw;
int		gibframe;
int		gibdelay;

int		spclfaceSprite[NUMSPCLFACES] = 
		{0,sbf_facelft,sbf_facergt,sbf_ouch,sbf_gotgat,sbf_mowdown};
boolean doSpclFace;
spclface_e	spclFaceType;

/*jagobj_t	*sbar;
byte		*sbartop;
jagobj_t	*faces[NUMFACES];
jagobj_t	*sbobj[NUMSBOBJ];*/

sbflash_t	flashCards[NUMCARDS];	/* INFO FOR FLASHING CARDS & SKULLS */

sbflash_t DAT_020470a8;

/*================================================== */
/* */
/*  Init this stuff EVERY LEVEL */
/* */
/*================================================== */
void ST_InitEveryLevel(void)
{
	int		i;
	stbar.gotgibbed = false;
	gibdraw = false;	/* DON'T DRAW GIBBED HEAD SEQUENCE */
	doSpclFace = false;
	stbar.specialFace = f_none;
	
	for (i = 0; i < NUMCARDS; i++)
	{
		stbar.tryopen[i] = false;
		flashCards[i].active = false;
		flashCards[i].y = card_y[i];
	}
}


/*
====================
=
= ST_Ticker
=
====================
*/

void ST_Ticker (void)
{
	int		ind;
	
	/* */
	/* Animate face */
	/* */
	if (--facetics <= 0)
	{
		facetics = M_Random ()&15;
		newface = M_Random ()&3;
		if (newface == 3)
			newface = 1;
		doSpclFace = false;
	}
	
	/* */
	/* Draw special face? */
	/* */
	if (stbar.specialFace)
	{
		doSpclFace = true;
		spclFaceType = stbar.specialFace;
		facetics = 15;
		stbar.specialFace = f_none;
	}
	
	/* */
	/* Did we get gibbed? */
	/* */
	if (stbar.gotgibbed && !gibdraw)
	{
		gibdraw = true;
		gibframe = 0;
		gibdelay = GIBTIME;
		stbar.gotgibbed = false;
	}

	if (DAT_020470a8.active && !--DAT_020470a8.delay)
	{
		DAT_020470a8.delay = FLASHDELAY;
		DAT_020470a8.doDraw ^= 1;
		if (!--DAT_020470a8.times)
			DAT_020470a8.active = false;
		if (DAT_020470a8.doDraw && DAT_020470a8.active)
			S_StartSound(NULL,sfx_itemup);
	}
	
	/* */
	/* Tried to open a CARD or SKULL door? */
	/* */
	for (ind = 0; ind < NUMCARDS; ind++)
	{
		/* CHECK FOR INITIALIZATION */
		if (stbar.tryopen[ind])
		{
			stbar.tryopen[ind] = false;
			flashCards[ind].active = true;
			flashCards[ind].delay = FLASHDELAY;
			flashCards[ind].times = FLASHTIMES+1;
			flashCards[ind].doDraw = false;
		}
		
		/* MIGHT AS WELL DO TICKING IN THE SAME LOOP! */
		if (flashCards[ind].active && !--flashCards[ind].delay)
		{
			flashCards[ind].delay = FLASHDELAY;
			flashCards[ind].doDraw ^= 1;
			if (!--flashCards[ind].times)
				flashCards[ind].active = false;
			if (flashCards[ind].doDraw && flashCards[ind].active)
				S_StartSound(NULL,sfx_itemup);
		}
	}
}


/*
====================
=
= ST_Drawer
=
====================
*/

void ST_Drawer (void)
{
	int			i;
	int			ind;
	player_t	*p;
	
	p = &players[consoleplayer];
	
	/* */
	/* Ammo */
	/* */
	i = weaponinfo[p->readyweapon].ammo;
	if (i == am_noammo)
		i = 0;
	else
		i = p->ammo[i];
	
	stbar.ammo = i;
	
	/* */
	/* Health */
	/* */
	stbar.health = p->health;

	/* */
	/* Armor */
	/* */
	stbar.armor = p->armorpoints;

	/* */
	/* Cards & skulls */
	/* */
	for (ind = 0; ind < NUMCARDS; ind++)
	{
		stbar.cards[ind] = (flashCards[ind].active && flashCards[ind].doDraw) | p->cards[ind];
	}
	
	/* */
	/* Weapons & level */
	/* */
	stbar.currentMap = gamemap;
		
	for (ind = 0; ind < NUMMICROS; ind++)
	{
		stbar.weaponowned[ind] = p->weaponowned[ind+1];
	}
		
	/*                    */
	/* God mode cheat */
	/* */
	i = p->cheats & CF_GODMODE;
	if (stbar.godmode != i)
		stbar.godmode = i;

	/* */
	/* face change */
	/* */
	if (stbar.godmode)
		stbar.face = GODFACE;
	else
	if (!stbar.health)
		stbar.face = DEADFACE;
	else
	if (doSpclFace)
	{
		int	base = stbar.health / 20;
		base = base > 4 ? 4 : base;
		base = 4 - base;
		base *= 3;
		stbar.face = base;
	}
	else
	{
		int	base = stbar.health/20;
		base = base > 4 ? 4 : base;
		base = 4 - base;
		base *= 3;
		stbar.face = base + newface;
	}
}
