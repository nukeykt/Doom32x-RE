/* comnjag.c */

#include "doomdef.h"

void C_Init (void)
{
}

/*
======================
=
= NumToStr
=
======================
*/

void NumToStr (int num, char *str)
{
	int	dig;

	str[4] = 0;
	str[0] = str[1] = str[2] = str[3] = ' ';
	if (num < 0)
		return;
		
	if (num < 10)
	{
		str[3] = '0'+num;
		return;
	}
	if (num < 100)
	{
		dig = num/10;
		str[2] = '0'+dig;
		num -= dig*10;
		str[3] = '0'+num;
		return;
	}

	if (num < 1000)
	{
		dig = num/100;
		str[1] = '0'+dig;
		num -= dig*100;
		dig = num/10;
		str[2] = '0'+dig;
		num -= dig*10;
		str[3] = '0'+num;
		return;
	}
	
	if (num < 10000)
	{
		dig = num/1000;
		str[0] = '0'+dig;
		num -= dig*1000;
		dig = num/100;
		str[1] = '0'+dig;
		num -= dig*100;
		dig = num/10;
		str[2] = '0'+dig;
		num -= dig*10;
		str[3] = '0'+num;
		return;
	}
	str[0] = str[1] = str[2] = str[3] = '+';
}


/*
=============
=
= PrintNumber
=
=============
*/

void PrintNumber (int x, int y, int num)
{
	static char	str[16];
	
	NumToStr (num,str);
	/*I_Print8 (x,y,str);*/
}

/*============================================================================= */
int		cx = 1, cy = 1;

/*
==================
=
= D_printf
=
==================
*/

void D_printf (char *str, ...)
{
	static char	buf[256];

	D_vsprintf (buf,str,((int *)&str)+1);
	/*I_Print8(1, cy, buf);
	cy ++;*/
}

void Print8(int x, int y, char *s);
void _clear(void);

void D_printf2 (char *str, ...)
{
	static char	buf[256];

	D_vsprintf (buf,str,((int *)&str)+1);
	Print8(1, cy, buf);
	cy++;
	if (cy > 25)
	{
		_clear();
		cy = 1;
	}
}


short ShortSwap (short dat)
{
	return (((unsigned short)dat<<8) + ((unsigned short)dat>>8))&0xffff;
}

long LongSwap (long dat)
{
	unsigned    a,b,c,d;
	
	a = (dat>>24)&0xff;
	b = (dat>>16)&0xff;
	c = (dat>>8)&0xff;
	d = dat&0xff;
	return (d<<24)+(c<<16)+(b<<8)+a;
}
