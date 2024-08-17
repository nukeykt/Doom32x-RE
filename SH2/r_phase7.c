#include "doomdef.h"
#include "r_local.h"
#include "marsonly.h"


byte spanstart[SCREENHEIGHT];

int planex;
int planey;
int planeangle;
int basexscale;
int baseyscale;
fixed_t planeheight;
pixel_t* planesource;
byte *DAT_06008c84;

void R_PlaneLoop(visplane_t *p);
void R_MapPlane(int a1);

#define OPENMARK 0xff00

void R_DrawPlanes(void)
{
    unsigned int angle;
    int light;
    visplane_t *p;
    if (lastvisplane - visplanes > MAXVISPLANES)
        I_Error("R_DrawPlanes: visplane overflow (%i)", lastvisplane - visplanes);

    planex = viewx;
    planey = -viewy;
    planeangle = viewangle;
    angle = (planeangle - ANG90) >> ANGLETOFINESHIFT;

    basexscale = -(finecosine[angle] / 64);
    baseyscale = finesine[angle] / 64;

    for (p = visplanes + 1; p < lastvisplane; p++)
    {
        if (p->minx <= p->maxx)
        {
            planesource = p->picnum;
            planeheight = p->lightlevel;
            if (planeheight < 0)
                planeheight = -planeheight;
            DAT_06008c84 = colormap + (255 - (p->lightlevel >> 3)) * 512;
            p->open[p->maxx+1] = OPENMARK;
            p->open[p->minx-1] = OPENMARK;
            R_PlaneLoop(p);
        }
    }
    phasetime[7] = samplecount;
}

void R_PlaneLoop(visplane_t* p)
{
    int x;
    int stopx;
    int t1;
    int t2;
    int b1;
    int b2;
    int oldtop;
    int oldbottom;
    unsigned short *open;

    x = p->minx;
    open = p->open;
    stopx = p->maxx + 1;
    oldtop = p->open[x-1];
    oldbottom = oldtop&0xff;
    oldtop >>= 8;
    for (; x <= stopx; x++)
    {
        t1 = oldtop;
        b1 = oldbottom;
        t2 = open[x];
        b2 = t2&0xff;
        t2 >>= 8;
        oldtop = t2;
        oldbottom = b2;

        if (t1 != t2)
        {
            while (t1 < t2 && t1 <= b1)
            {
                R_MapPlane(((x-1)<<16) + (t1<<8) + spanstart[t1]);
                t1++;
            }
            while (t2 < t1 && t2 <= b2)
            {
                spanstart[t2] = x;
                t2++;
            }
        }
        if (b1 != b2)
        {
            while (b1 > b2 && b1 >= t1)
            {
                R_MapPlane(((x-1)<<16) + (b1<<8) + spanstart[b1]);
                b1--;
            }
            while (b2 < b1 && b2 >= t2)
            {
                spanstart[b2] = x;
                b2--;
            }
        }
    }
}
