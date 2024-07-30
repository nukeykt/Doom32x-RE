/* marsonly.c */

#include <stdarg.h>
#include "doomdef.h"
#include "r_local.h"
#include "marsonly.h"

int DAT_06000800 = 1;

int lastticcount;
int DAT_060012e4;
int DAT_060012e8;

void FUN_02036310()
{
}

void main()
{
    blankMode();
    mdPri();
    paletteMode();
    clear();
    sleep(1);
    swapbuffers();
    parInitLineTable(224);
    clear();
    sleep(1);
    swapbuffers();
    parInitLineTable(224);
    framebuffer_p = (byte*)0x24000200;
    cheated = false;
    D_DoomMain();
}

/* TODO */
void FUN_020363ca()
{
    byte *p;
    int i;
    for (i = 0; i < 64000; i++)
    {
        framebuffer_p[i] = (byte)0xa5;
    }
    for (i = 0; i < 64000; i++)
    {
        p = framebuffer_p + i;
        if (*p != 0xa5)
        {
            D_printf("%X = %x (a5)\n", p, *p);
            while (1)
                ;
        }
    }
    for (i = 0; i < 64000; i++)
    {
        framebuffer_p[i] = (byte)0x5a;
    }
    for (i = 0; i < 64000; i++)
    {
        p = framebuffer_p + i;
        if (*p != 0x5a)
        {
            D_printf("%X = %x (5a)\n", p, *p);
            while (1)
                ;
        }
    }
}


/*
================
=
= I_Error
=
================
*/


void I_Error (char *error, ...) 
{
    char errormessage[256];
    va_list	argptr;
    va_start(argptr, error);
	D_vsprintf (errormessage,error,argptr);
    va_end(argptr);
    initPPalette();
    D_printf2("I_Error:  ");
    D_printf2(errormessage);
	while (1)
	;
} 

/* 
================ 
= 
= I_Init  
=
= Called after all other subsystems have been started
================ 
*/ 
 
/* TODO */
void I_Init (void) 
{	 
    int lump1, lump2;

    lump1 = W_GetNumForName("COLORMAP");
    lump2 = W_GetNumForName("PLAYPALS");

    playpal = wadfileptr + lumpinfo[lump1].filepos;
    colormap = wadfileptr + lumpinfo[lump2].filepos;

    I_SetPalette(playpal);
} 

int FUN_02036540(void)
{
    volatile unsigned short v1;

    v1 = ReadCom2();
    while (v1 != ReadCom2())
    {
        Delay(50);
        v1 = ReadCom2();
    }

    return (int)v1 == 0xbeef;
}

void FUN_0203658c(void)
{
    volatile unsigned short v1;
    int i = 0;

    v1 = ReadCom2();
    while (v1 != ReadCom2())
    {
        if (v1 == 0xbeef)
            break;
        Delay(50);
        v1 = ReadCom2();
        i++;
        if (i > 100000)
            break;
    }
}


void FUN_020365e8(unsigned short a1, unsigned short a2, unsigned short a3, unsigned short a4)
{
    unsigned short v1;

    v1 = ReadCom2();
    while (v1 != ReadCom2())
    {
        Delay(50);
        v1 = ReadCom2();
    }

    if ((int)v1 == 0xbeef)
    {
        WriteCom3(a2);
        WriteCom4(a3);
        WriteCom5(a4);
        WriteCom2(a1);

        FUN_0203658c();
    }
}

unsigned short FUN_020366ac(unsigned short *a1)
{
    unsigned short v1;

    v1 = ReadCom2();
    while (v1 != ReadCom2())
    {
        Delay(50);
        v1 = ReadCom2();
    }
    if ((int)v1 != 0xbeef)
    {
        *a1 = ReadCom0();
        WriteCom1(0xbeef);
    }
    return v1;
}

void FUN_02036710()
{
    DAT_06007520 = DAT_06001174 - DAT_060012e4;
    DAT_060012e4 = DAT_06001174;
    lasttics = ticcount - lastticcount;
    while (lasttics < 3)
    {
        sleep(1);
        lasttics = ticcount - lastticcount;
    }
    lastticcount = ticcount;
    swapbuffers();
    DAT_06000804++;
    if (DAT_06007528 != DAT_060012e8)
    {
        I_SetPalette(playpal + 768 * DAT_060012e8);
    }
    DAT_06007528 = DAT_060012e8;
    DAT_06000814++;
}

/* 
==================== 
= 
= I_WadBase  
=
= Return a pointer to the wadfile.  In a cart environment this will
= just be a pointer to rom.  In a simulator, load the file from disk.
==================== 
*/ 
 
byte *I_WadBase (void)
{
	return (byte *)0x20bb000; 
}


/* 
==================== 
= 
= I_ZoneBase  
=
= Return the location and size of the heap for dynamic memory allocation
==================== 
*/ 
 
#define	STARTHEAP	0x600c000
#define	ENDHEAP		0x603f000

byte *I_ZoneBase (int *size)
{
	*size = ENDHEAP-STARTHEAP;               /* leave 64k for stack  */
	return (byte *)STARTHEAP; 
}
