/* R_data.c */

#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"

int		firstflat, lastflat, numflats;

int			numtextures;
texture_t	textures[MAXTEXTURES];

int			*flattranslation;		/* for global animation */
int			*texturetranslation;	/* for global animation */

texture_t	*skytexturep;


/*============================================================================ */

/*
==================
=
= R_InitTextures
=
= Initializes the texture list with the textures from the world map
=
==================
*/

	int			*maptex;

void R_InitTextures (void)
{
	maptexture_t	*mtexture;
	texture_t		*texture;
	int			i,j,c;
	int			offset;
	int			*directory;
	

/* */
/* load the map texture definitions from textures.lmp */
/* */
	maptex = W_CacheLumpName ("TEXTURE1", PU_STATIC);
	numtextures = LITTLELONG(*maptex);
	directory = maptex+1;
	
	for (i=0 ; i<numtextures ; i++, directory++)
	{
		offset = LITTLELONG(*directory);
		mtexture = (maptexture_t *) ( (byte *)maptex + offset);
		texture = &textures[i];
		texture->width = LITTLESHORT(mtexture->width);
		texture->height = LITTLESHORT(mtexture->height);
		D_memcpy (texture->name, mtexture->name, 8);
		for (j=0 ; j<8 ; j++)
		{
			c = texture->name[j];
			if (c >= 'a' && c<='z')
				texture->name[j] = c - ('a'-'A');
		}
		texture->data = NULL;		/* not cached yet */
		texture->lumpnum = W_CheckNumForName (texture->name);
if (texture->lumpnum == -1)
	texture->lumpnum = 0;
	}

	Z_Free (maptex);
	
/* */
/* translation table for global animation */
/* */
	texturetranslation = Z_Malloc ((numtextures+1)*4, PU_STATIC, 0);
	for (i=0 ; i<numtextures ; i++)
		texturetranslation[i] = i;	
}



/*
================
=
= R_InitFlats
=
=================
*/

void R_InitFlats (void)
{
	int		i;
	
	firstflat = W_GetNumForName ("F_START") + 1;
	lastflat = W_GetNumForName ("F_END") - 1;
	numflats = lastflat - firstflat + 1;
	
/* translation table for global animation */
	flattranslation = Z_Malloc ((numflats+1)*4, PU_STATIC, 0);
	for (i=0 ; i<numflats ; i++)
		flattranslation[i] = i;
}


/*
================
=
= R_InitData
=
= Locates all the lumps that will be used by all views
= Must be called after W_Init
=================
*/

void R_InitData (void)
{
	R_InitTextures ();
	R_InitFlats ();
}


/*============================================================================= */

/*
================
=
= R_FlatNumForName
=
================
*/

#ifdef i386
#define HIBIT (1<<7)
#else
#define HIBIT (1<<31)
#endif
void strupr (char *s);

int	R_FlatNumForName (char *name)
{
	int		i;
	static char	namet[16];

	i = W_CheckNumForName (name);
	if (i == -1)
	{
		namet[8] = 0;
		D_memcpy (namet, name,8);
		I_Error ("R_FlatNumForName: %s not found",namet);
	}
	
	i -= firstflat;
	if (i>numflats)
		I_Error ("R_FlatNumForName: %s past f_end",namet);
	return i;
}


/*
================
=
= R_CheckTextureNumForName
=
================
*/

int	R_CheckTextureNumForName (char *name)
{
	int		i,c;
	char	temp[8];
		
	if (name[0] == '-')		/* no texture marker */
		return 0;
	
	for (i=0 ; i<8 ; i++)
	{
		c = name[i];
		if (c >= 'a' && c<='z')
			c -= ('a'-'A');
		temp[i] = c;
	}
	
	for (i=0 ; i<numtextures ; i++)
		if (!D_strncasecmp(textures[i].name, temp, 8))
			return i;
		
	return 0;	/* FIXME -1; */
}


/*
================
=
= R_TextureNumForName
=
================
*/

int	R_TextureNumForName (char *name)
{
	int		i;
	
	i = R_CheckTextureNumForName (name);
	if (i==-1)
		I_Error ("R_TextureNumForName: %s not found",name);
	
	return i;
}


