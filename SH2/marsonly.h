#ifndef MARSONLY_H
#define MARSONLY_H


void blankMode(void);
void mdPri(void);
void paletteMode(void);
void clear();
void sleep(int);
void swapbuffers(void);
void parInitLineTable(int);

extern int DAT_06000800;

extern byte *colormap;

extern byte *framebuffer_p;
extern boolean cheated;

typedef struct
{
	short f_0;
	short f_2;
	short f_4;
	byte *f_6;
	inpixel_t *f_a;
	fixed_t f_e;
	fixed_t f_12;
} drawcol_t;

void R_DrawColumnASM(drawcol_t *a1);

static inline void R_DrawColumn (int dc_x, int dc_yl, int dc_yh, int light, fixed_t frac, fixed_t fracstep, inpixel_t *dc_source)
{
	int			count;
	drawcol_t info;

	count = dc_yh - dc_yl;
	if (count < 0)
		return;

#ifdef RANGECHECK 
	if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
		I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif
	info.f_0 = dc_x << 1;
	info.f_2 = dc_yl;
	info.f_4 = count + 1;
	info.f_6 = colormap + light * 512;
	info.f_a = dc_source;
	info.f_e = frac;
	info.f_12 = fracstep;
	if (DAT_06000800)
		R_DrawColumn2(&info);
}

typedef struct
{
	short f_0;
	short f_2;
	short f_4;
	byte *f_6;
	inpixel_t *f_a;
	fixed_t f_e;
	fixed_t f_12;
	fixed_t f_16;
	fixed_t f_1a;
} drawspan_t;

void R_DrawSpanASM(drawspan_t *a1);

static inline void R_DrawSpan (int ds_y, int ds_x1, int ds_x2, int light, fixed_t ds_xfrac, fixed_t ds_yfrac, fixed_t ds_xstep, fixed_t ds_ystep, inpixel_t *ds_source)
{
	drawspan_t info;
#ifdef RANGECHECK
	if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=SCREENWIDTH  
	|| (unsigned)ds_y>SCREENHEIGHT) 
		I_Error ("R_DrawSpan: %i to %i at %i",ds_x1,ds_x2,ds_y); 
#endif
	info.f_0 = ds_x1 << 1;
	info.f_2 = ds_y;
	info.f_4 = ds_x2 - ds_x1 + 1;
	info.f_6 = colormap + light * 512;
	info.f_a = ds_source;
	info.f_e = ds_xfrac;
	info.f_12 = ds_xstep;
	info.f_16 = ds_yfrac;
	info.f_1a = ds_ystep;
	if (DAT_06000800)
		R_DrawSpanASM(&info);
}


#endif
