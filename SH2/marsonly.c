/* marsonly.c */

#include "doomdef.h"
#include "r_local.h"
#include "marsonly.h"

int DAT_06000800 = 1;

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
