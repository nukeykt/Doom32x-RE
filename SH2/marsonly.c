/* marsonly.c */

#include <stdarg.h>
#include "doomdef.h"
#include "r_local.h"
#include "marsonly.h"
#include "st_main.h"

int DAT_06000800 = 1;

unsigned short DAT_06000808 = 0;

unsigned short DAT_06000810 = 0;

unsigned short DAT_0600081a = 0;
int DAT_0600081c = 0;
int DAT_06000820 = 0;
int DAT_06000824 = 0;
int DAT_06000828 = 0;
int DAT_0600082c = 0;
unsigned short DAT_06000830 = 0;
unsigned short DAT_06000832 = 0;
volatile int DAT_06000834 = 0;
int DAT_06000838 = 0;
volatile int DAT_0600083c = 0;
int DAT_06000840 = 0;

int DAT_06001174;

int lastticcount; /* 60012e0 */

int DAT_060012e4;
int DAT_060012e8;
unsigned short DAT_060012ec;
unsigned short DAT_060012ee;
unsigned short DAT_060012f0;
unsigned short DAT_060012f2;
int DAT_060012f4;
int DAT_060012f8;
int DAT_060012fc;
int DAT_06001300;

volatile unsigned short *DAT_06007524;

volatile int* DAT_0600752c;

volatile int* DAT_06007538;
volatile int* DAT_0600753c;
volatile int* DAT_06007540;

volatile unsigned short *DAT_06007544;
unsigned short DAT_06007548;

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

    v1 = ReadCom1();
    while (v1 != ReadCom1())
    {
        Delay(50);
        v1 = ReadCom1();
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

/* TODO */
int I_ReadControls(void)
{
    int i;
    int v1;

    DAT_060012f0 = FUN_020366ac(&DAT_060012f2);

    switch (DAT_060012f0)
    {
        case 0xceed:
            DAT_06007548 = 0;
            DAT_0600081a = 1;
            DAT_060012f0 = FUN_020366ac(&DAT_060012f2);
            while (DAT_060012f0 != 0xadec)
            {
                DAT_060012f0 = FUN_020366ac(&DAT_060012f2);
            }
            if (DAT_060012f2 & 0x8000)
                DAT_06000810 = 1;
            else
                DAT_06000810 = 0;
            DAT_0600081a = 0;
            break;
        case 0xabcd:
            DAT_06007548 = DAT_060012f2;
            break;
        case 0xfabd:
            break;
        case 0xfead:
            DAT_06000808 = 1;
            break;
    }

    if (FUN_02036540())
    {
        DAT_060012ec =
            (stbar.health) |
            ((stbar.face & 31) << 8) |
            (stbar.cards[0] << 13) |
            (stbar.cards[1] << 14) |
            (stbar.cards[2] << 15);
        DAT_060012ee = 0;
        for (i = 0; i < 6; i++)
        {
            DAT_060012ee |= stbar.weaponowned[i] << i;
        }
        DAT_060012ee <<= 8;
        DAT_060012ee |= stbar.armor;

        FUN_020365e8(0x39fb, stbar.ammo, DAT_060012ec, DAT_060012ee);
    }

    v1 = 0;
    if (DAT_06007548 & 8)
        v1 |= 0x800000;
    if (DAT_06007548 & 4)
        v1 |= 0x400000;
    if (DAT_06007548 & 1)
        v1 |= 0x100000;
    if (DAT_06007548 & 2)
        v1 |= 0x200000;
    if (DAT_06007548 & 0x40)
        v1 |= 0x20000000;
    if (DAT_06007548 & 0x10)
        v1 |= 0x2000000;
    if (DAT_06007548 & 0x20)
        v1 |= 0x2000;
    if (DAT_06007548 & 0x4000)
        v1 |= 0x80000;
    if (DAT_06007548 & 0x2000)
        v1 |= 0x80;
    if (DAT_06007548 & 0x1000)
        v1 |= 0x8;
    if (DAT_06007548 & 0x100)
        v1 |= 0x100;
    if (DAT_06007548 & 0x200)
        v1 |= 0x200;
    if (DAT_06007548 & 0x400)
        v1 |= 0x400;
    if (DAT_06007548 & 0x8000)
        v1 |= 0x8000 | (DAT_06007548 & 0xf00);

    DAT_0600081c = v1;

    return v1;
}

/* TODO */
void I_SetPalette (byte *palette)
{
    int i;
    unsigned short *dst = (unsigned short*)0x20004200;
    for (i = 0; i < 256; i++)
    {
        byte r = (*palette++) >> 3;
        byte g = (*palette++) >> 3;
        byte b = (*palette++) >> 3;
        *dst++ = r | (g << 5) | (b << 10);
    }
}

/* 
================ 
= 
= FixedMul  
=
= Perform a signed 16.16 by 16.16 mutliply
================ 
*/ 
 
fixed_t FixedMul (fixed_t a, fixed_t b) 
{ 
/* this code is very slow, but exactly simulates the proper assembly */
/* operation that C doesn't let you represent well */
	int             sign; 
	unsigned        a1,a2,b1,b2; 
    unsigned    	c; 
	 
	sign = a^b; 
	if (a<0) 
		a = -a; 
	if (b<0) 
		b = -b; 
	a1 = a&0xffff; 
	a2 = (unsigned)a>>16; 
	b1 = b&0xffff; 
	b2 = (unsigned)b>>16; 
	c = (a1*b1) >> 16; 
	c += a2*b1; 
	c += b2*a1; 
	c += (b2*a2)<<16; 
	if (sign < 0) 
		c = -c; 
    return c; 
} 

/* 
================ 
= 
= FixedDiv  
= 
= Perform a signed 16.16 by 16.16 divide (a/b)
================ 
*/ 

/* TODO */
fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    int i;
    volatile int *s = (volatile int*)0xffffff00;
    s[0] = b;
    s[4] = a >> 16;
    s[5] = a << 16;

    asm("nop nop nop nop nop nop nop nop nop nop "
        "nop nop nop nop nop nop nop nop nop nop "
        "nop nop nop nop nop nop nop nop nop nop "
        "nop nop nop nop nop nop nop nop nop nop ": : );

    return s[7];
}
 

fixed_t FixedDiv2 (fixed_t a, fixed_t b) 
{ 
/* this code is VERY slow, but exactly simulates the proper assembly */
/* operation that C doesn't let you represent well */
	unsigned        aa,bb,c; 
	unsigned        bit; 
	int             sign; 
	 
	sign = a^b; 
	if (a<0) 
		aa = -a; 
	else 
		aa = a; 
	if (b<0) 
		bb = -b; 
	else 
		bb = b; 
	if ( (aa>>14) >= bb) 
		return sign<0 ? MININT : MAXINT; 
	bit = 0x10000; 
	while (aa > bb) 
	{ 
		bb <<= 1; 
		bit <<= 1; 
	} 
	c = 0; 
	 
	do 
	{ 
		if (aa >= bb) 
		{ 
			aa -=bb; 
			c |= bit; 
		} 
		aa <<=1; 
		bit >>= 1; 
	} while (bit && aa); 
	 
	if (sign < 0) 
		c = -c; 
    return c; 
} 

void FUN_02036fa8();

void FUN_02036db4()
{
    FUN_02036fa8();
}

unsigned short* FUN_02036dc8()
{
    int i;
    unsigned short *dst = (unsigned short*)0x24000200;
    for (i = 0; i < 65536; i++)
    {
        *dst++ = 0;
    }

    return (unsigned short*)0x24000200;
}

int I_GetTics(void)
{
    return ticcount;
}

int FUN_02036df4(void)
{
    return 0;
}

int FUN_02036df8(void)
{
    return 1;
}

void FUN_02036dfc(void)
{
    *(int*)0xfffffe92 |= 0x10;
}

void FUN_02036e0a(void)
{
    while (1);
}

void FUN_02036e0e(void)
{
    DAT_06000820 = 1;
    DAT_06000824 = 1;
}

void FUN_02036e24(void)
{
    int *p;

    p = (int*)((int)&DAT_06000820 | 0x20000000);

    while (*p != 1)
    {
        Delay(50);
    }
    DAT_06000820 = 0;
    DAT_06000824 = 0;
}

void FUN_02036e68(void)
{
    int* p;

    p = (int*)((int)&DAT_06000824 | 0x20000000);

    while (*p == 1)
    {
        Delay(50);
    }
    DAT_06000824 = 1;
    DAT_06000820 = 1;
}

void FUN_02036eac(void)
{
    int* p;

    p = (int*)((int)&DAT_06000828 | 0x20000000);

    while (*p != 1)
    {
        Delay(50);
    }
    DAT_06000828 = 0;
    DAT_0600082c = 0;
}

void FUN_02036ef0(void)
{
    int* p;

    p = (int*)((int)&DAT_0600082c | 0x20000000);

    while (*p == 1)
    {
        Delay(50);
    }
    DAT_0600082c = 1;
    DAT_06000828 = 1;
}

void Delay(int a1)
{
    do
    {
        a1--;
    } while (a1);
}

void FUN_02036f40(void)
{
    DAT_06007544 = (unsigned short*)((int)&DAT_06000830 | 0x20000000);
    DAT_06007524 = (unsigned short*)((int)&DAT_06000832 | 0x20000000);
}

int FUN_02036f68(void)
{
    return *DAT_06007524 + 0x7f == *DAT_06007544;
}

int FUN_02036f88(void)
{
    return *DAT_06007544 == *DAT_06007524;
}

void FUN_02036fa8(void)
{
    DAT_06000830 = *DAT_06007544 + 1;
}

void FUN_02036fc0(void)
{
    DAT_06000832 = *DAT_06007524 + 1;
}

void FUN_02036fd8(void)
{
    DAT_060012f4 = DAT_06001174;
}

void FUN_02036fec(void)
{
    unsigned int t = DAT_06001174;
    if (t > DAT_060012f4)
        WriteCom2(t - (unsigned short)DAT_060012f4);
    else
        WriteCom2(t - (unsigned short)DAT_060012f4);
}

void FUN_02037018(void)
{
    DAT_060012f8 = DAT_06001174;
}

void FUN_0203702c(void)
{
    unsigned int t = DAT_06001174;
    if (t > DAT_060012f8)
        WriteCom5(t - (unsigned short)DAT_060012f8);
    else
        WriteCom5(t - (unsigned short)DAT_060012f8);
}

void FUN_02037058(void)
{
    DAT_060012fc = DAT_06001174;
}

void R_Update(void)
{
    unsigned int t = DAT_06001174;
    if (t <= DAT_060012fc)
        t += 0x10000;

    WriteCmd7(t - DAT_060012fc);
}

void FUN_0203709c(void)
{
    DAT_06001300 = DAT_06001174;
}

void FUN_020370b0(void)
{
    unsigned int t = DAT_06001174;
    if (t <= DAT_06001300)
        t += 0x10000;

    WriteCmd7(t - DAT_06001300);
}

void FUN_020370e0(void)
{
    DAT_0600753c = (int*)((int)&DAT_06000834 | 0x20000000);
    DAT_0600752c = (int*)((int)&DAT_06000838 | 0x20000000);
    DAT_06007540 = (int*)((int)&DAT_0600083c | 0x20000000);
    DAT_06007538 = (int*)((int)&DAT_06000840 | 0x20000000);
}

void FUN_02037128(void)
{
    while (*DAT_0600753c != 0)
    {
        Delay(49);
    }
    DAT_06000834 = 1;
    while (*DAT_06007540 == 0)
    {
        Delay(49);
    }
    DAT_0600083c = 0;
}
