/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier, based on imgkap.c by M'dJ
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Sean D'Epagnier                                 *
 *   sean at depagnier dot com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#define VERS   "1.0b"

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>		/* for malloc() */
#include <string.h>		/* for strncmp() */
#include <math.h>

#include <time.h>       /* for date in kap */

#include "weatherfax_pi.h"
#include "WeatherFaxImage.h"
#include "wximgkap.h"

/* color type and mask */

typedef struct
{
    uint8_t rgbRed, rgbGreen, rgbBlue, rgbReserved;
} RGBQUAD_t;

typedef union
{
    RGBQUAD_t  q;
    uint32_t p;
} Color32;

#define RGBMASK 0x00FFFFFF



static const double WGSinvf                           = 298.257223563;       /* WGS84 1/f */
static const double WGSexentrik                       = 0.081819;             /* e = 1/WGSinvf; e = sqrt(2*e -e*e) ;*/

static inline double lontox(double l)
{
    return l*M_PI/180;
}

static inline double lattoy_WS84(double l)
{
    double e = WGSexentrik;
    double s = sinl(l*M_PI/180.0);

    return log(tan(M_PI/4 + l * M_PI/360)*pow((1. - e * s)/(1. + e * s), e/2.));
}

static double postod(double lat0, double lon0, double lat1, double lon1)
{
    double x,v,w;

    lat0 = lat0 * M_PI / 180.;
    lat1 = lat1 * M_PI / 180.;
    lon0 = lon0 * M_PI / 180.;
    lon1 = lon1 * M_PI / 180.;

    v = sin((lon0 - lon1)/2.0);
    w = cos(lat0) * cos(lat1) * v * v;
    x = sin((lat0 - lat1)/2.0);
    return (180. * 60. / M_PI) * 2.0 * asinl(sqrtl(x*x + w));
}


/*------------------ Single Memory algorithm ------------------*/

#define MYBSIZE 1572880

typedef struct smymemory
{
    struct smymemory *next;
    uint32_t size;
} mymemory;

static mymemory *mymemoryfirst;
static mymemory *mymemorycur;

void * myalloc(int size)

{
    void *s = NULL;
    mymemory *mem = mymemorycur;

    if (mem && ((mem->size + size) > MYBSIZE)) mem = 0;

    if (!mem)
    {
        mem = (mymemory *)calloc(MYBSIZE,1);
        if (mem == NULL) return 0;
        mem->size = sizeof(mymemory);

        if (mymemorycur) mymemorycur->next = mem;
        mymemorycur = mem;
        if (!mymemoryfirst) mymemoryfirst = mem;
    }

    s = ((int8_t *)mem + mem->size);
    mem->size += size;
    return s;
}

void myfree(void)

{
    struct smymemory *mem, *next;

    mem = mymemoryfirst;
    while (mem)
    {
        next = mem->next;
        free(mem);
        mem = next;
    }
    mymemoryfirst = 0;
    mymemorycur = 0;
}

/*------------------ Histogram algorithm ------------------*/

typedef struct
{
    Color32 color;
    uint32_t count;
    int16_t num;
} helem;

typedef struct shistogram
{
    Color32 color;
    uint32_t count;
    int16_t num;
    int16_t used;
    struct shistogram *child ;
} histogram;


#define HistIndex2(p,l) ((((p.q.rgbRed >> l) & 0x03) << 4) | (((p.q.rgbGreen >> l) & 0x03) << 2) |	((p.q.rgbBlue >> l) & 0x03) )
#define HistSize(l) (l?sizeof(histogram):sizeof(helem))
#define HistInc(h,l) (histogram *)(((char *)h)+HistSize(l))
#define HistIndex(h,p,l) (histogram *)((char *)h+HistSize(l)*HistIndex2(p,l))

static histogram *HistAddColor (histogram *h, Color32 pixel )
{
    char level;

    for (level=6;level>=0;level -=2)
    {
        h = HistIndex(h,pixel,level) ;

        if (h->color.p == pixel.p) break;
        if (!h->count && !h->num)
        {
            h->color.p = pixel.p;
            break;
        }
        if (!h->child)
        {
            h->child = (histogram *)myalloc(HistSize(level)*64);
            if (h->child == NULL) return 0;
        }
        h = h->child;
    }

    h->count++;
    return h;
}

static int HistGetColorNum (histogram *h, Color32 pixel)
{
    char level;

    for (level=6;level>=0;level -=2)
    {
        /* 0 < index < 64 */
        h = HistIndex(h,pixel,level) ;
        if (h->color.p == pixel.p) break;
        if (!level)
            break; // erreur
        if (!h->child) break;
        h = h->child;
    }
    if (!h->num)
        level = 0;
    if (h->num < 0) return -1-h->num;
    return h->num-1;
}

#define HistColorsCount(h) HistColorsCountLevel(h,6)

static int32_t HistColorsCountLevel (histogram *h,int level)
{
    int i;
    uint32_t count = 0;

    for (i=0;i<64;i++)
    {
        if (h->count) count++;
        if (level)
        {
            if(h->child) count += HistColorsCountLevel(h->child,level-2);
        }
        h = HistInc(h,level);
    }
    return count;
}


/*--------------- reduce begin -------------*/

typedef struct
{
    histogram   *h;

    int32_t 	nbin;
    int32_t 	nbout;

    int32_t     colorsin;
    int32_t     colorsout;

    int         nextcote;
    int         maxcote;
    int         limcote[8];

    uint64_t    count;
    uint64_t    red;
    uint64_t    green;
    uint64_t    blue;

} reduce;

static inline int HistDist(Color32 a, Color32 b)
{
   int c,r;

   c = a.q.rgbRed - b.q.rgbRed;
   r = c*c;

   c = a.q.rgbGreen - b.q.rgbGreen;
   r += c*c;

   c = a.q.rgbBlue - b.q.rgbBlue;
   r += c*c;

   return sqrt((double)r);
}

static int HistReduceDist(reduce *r, histogram *h, histogram *e, int cote, int level)
{
    int i;
    int used = 1;
    int curcote;
    int limcote = r->limcote[level];

    for (i=0;i<64;i++)
    {
        if (h->count && !h->num)
        {

            curcote = HistDist((Color32)e->color,(Color32)h->color);

            if (curcote <= cote)
            {
                    uint64_t c;

                    c = h->count;

                    r->count += c;
                    r->red += c * (uint64_t)((Color32)h->color).q.rgbRed ;
                    r->green +=  c * (uint64_t)((Color32)h->color).q.rgbGreen;
                    r->blue +=  c * (uint64_t)((Color32)h->color).q.rgbBlue;

                    h->num = r->nbout;
                    h->count = 0;
                    r->nbin++;
            }
            else
            {
                    if (r->nextcote > curcote)
                        r->nextcote = curcote;
                    used = 0;
            }
        }
        if (level && h->child && !h->used)
        {
            limcote += cote ;

            curcote = HistDist((Color32)e->color,(Color32)h->color);

            if (curcote <= limcote)
                h->used = HistReduceDist(r,h->child,e,cote,level-2);
            if (!h->used)
            {
                if ((curcote > limcote) && (r->nextcote > limcote))
                    r->nextcote = curcote ;
                used = 0;
            }
            limcote -= cote ;
        }
        h = HistInc(h,level);
    }
    return used;
}

static void HistReduceLevel(reduce *r, histogram *h, int level)
{
    int i;

    for (i=0;i<64;i++)
    {
        if (level && h->child && !h->used)
        {
            HistReduceLevel(r, h->child,level-2);
            if (!r->colorsout) break;
        }

        if (h->count && !h->num)
        {
            int32_t cote = 0;
            int32_t nbcolors;
            int32_t curv;

            r->count = r->red = r->green = r->blue = 0;
            r->nbin = 0;
            r->nextcote = 0;
            r->nbout++;

            cote = (int32_t)(pow((double)((1<<24)/(double)r->colorsout),1.0/3.0)/2)-1;
            r->maxcote = sqrt(3.0*cote*cote);

            curv = 0;
            nbcolors = (r->colorsin +r->colorsout -1)/r->colorsout;

            while (r->nbin < nbcolors)
            {
                curv += nbcolors - r->nbin;
                cote = (int32_t)(pow(curv,1.0/3.0)/2) - 1;
                cote = sqrt(3.0*cote*cote);

                if (r->nextcote > cote)
                    cote = r->nextcote;

                r->nextcote = r->maxcote+1;

                if (cote >= r->maxcote)
                        break;

                h->used = HistReduceDist(r,r->h,h,cote,6);

                if (!r->count)
                {
                    fprintf(stderr,"Erreur quantize\n");
                    return;
                }
            }

            r->colorsin -= r->nbin;
            r->colorsout--;
            {
                histogram *e;
                Color32 pixel ;
                uint64_t c,cc;

                c = r->count; cc = c >> 1 ;
                pixel.q.rgbRed = (uint8_t)((r->red + cc) / c);
                pixel.q.rgbGreen = (uint8_t)((r->green + cc) / c);
                pixel.q.rgbBlue = (uint8_t)((r->blue + cc) / c);
                pixel.q.rgbReserved = 0;

                e = HistAddColor(r->h,pixel);
                e->count = r->count;
                e->num = -r->nbout;

            }

            if (!r->colorsout) break;
        }
        h = HistInc(h,level);
    }

}

static int HistReduce(histogram *h, int colorsin, int colorsout)
{
    reduce r;

    r.h = h;

    r.nbout = 0;

    if (!colorsout || !colorsin) return 0;

    if (colorsout > 0x7FFF) colorsout = 0x7FFF;
    if (colorsout > colorsin) colorsout = colorsin;
    r.colorsin = colorsin;
    r.colorsout = colorsout;

    r.limcote[2] = sqrt((double)3*3*3) ;
    r.limcote[4] = sqrt((double)3*15*15) ;
    r.limcote[6] = sqrt((double)3*63*63) ;

    HistReduceLevel(&r,h,6);

    return r.nbout;
}

/*--------------- reduce end -------------*/

static int _HistGetList(histogram *h,helem **e,int nbcolors,char level)
{
    int i;
    int nb;
//    int r;

    nb = 0;
    for (i=0;i<64;i++)
    {
        if (h->count && (h->num < 0))
        {
            //r = -1-h->num;
            e[-1-h->num] = (helem *)h;
            nb++;
        }

        if (level && h->child) nb += _HistGetList(h->child,e,nbcolors-nb,level-2);
        if (nb > nbcolors)
                return 0;
        h = HistInc(h,level);
    }
    return nb;
}


static int HistGetPalette(Color32 *palkap,uint8_t *colors,histogram *h,int nbcolors, int nb)
{
    int i,j;
    helem **e = (helem **)malloc(nbcolors*sizeof(helem *));
    if (e == NULL)
    {
		fprintf(stderr,"ERROR - Intern malloc\n");
		return 0;
    }

    /* get colors used */
    if ((i= _HistGetList(h,e,nbcolors,6)) != nbcolors)
    {
        free(e);
        return 0;
    }

    /* sort palette desc count */
    for (i=0;i<nbcolors;i++)
    {
        for (j=i+1;j<nbcolors;j++)
            if (e[j]->count > e[i]->count)
            {
                h =  (histogram *)e[i];
                e[i] = e[j];
                e[j] = (helem *)h;
            }
    }

    /* create palette with unique color and step first or put last in first */

    j = nb+nbcolors;
    if (nb) palkap[0].p = 0;
    nb = 1;

    for (i=0;i<nbcolors;i++)
    {
        colors[-1-e[i]->num] = (i+nb)%nbcolors;
        palkap[(i+nb)%nbcolors].p = e[i]->color.p;
    }
    free(e);
    return j;
}

#define HistFree(h) myfree()


/*------------------ End of Histogram ------------------*/


/* read in kap file */

//* si size < 0 lit jusqu'a \n (elimine \r) et convertie NO1 int r = (c - 9) & 0xFF; */

static inline int fgetkapc(int typein, FILE *in)
{
    int c;


    c = getc(in);
    if (typein == FIF_NO1) return (c - 9) & 0xff;
    return c;
}

/* function read and write kap index */

static int bsb_write_index(FILE *fp, uint16_t height, uint32_t *index)
{
    uint8_t l;

	/* Write index table */
	while (height--)
	{
		/* Indices must be written as big-endian */
        l = (*index >> 24) & 0xff;
        fputc(l, fp);
        l = (*index >> 16) & 0xff;
        fputc(l, fp);
		l = (*index >> 8) & 0xff;
        fputc(l, fp);
		l = *index & 0xff;
        fputc(l, fp);
        index++;
	}
	return 1;
}

/* bsb compress number, not value 0 at first write */

static uint16_t bsb_compress_nb(uint8_t *p, uint16_t nb, uint8_t pixel, uint16_t max)
{
    uint16_t count = 0;

    if (nb > max)
    {
        count = bsb_compress_nb(p,nb>>7,pixel|0x80,max);
        p[count++] = (nb & 0x7F) | (pixel & 0x80);
        return count;
    }
    pixel |= nb ;
    if (!pixel) p[count++] = 0x80;
    p[count++] = pixel ;
    return count;
}


/* write line bsb */

int bsb_compress_row(const uint8_t *buf_in, uint8_t *buf_out, uint16_t bits_out, uint16_t line, uint16_t widthin, uint16_t widthout)
{
	uint16_t	ibuf,run_length ;
	uint16_t    ipixelin,ipixelout,xout;
	uint8_t	    last_pix;
	uint16_t    dec, max;

	dec = 7-bits_out;
	max = (1<<dec) -1;

    /*      write the line number */
    ibuf = bsb_compress_nb(buf_out,line,0,0x7F);

 	ipixelin = ipixelout = 0;

	while ( ipixelin < widthin )
	{
		last_pix = buf_in[ipixelin];
		ipixelin++;
		ipixelout++;

		/* Count length of same pixel */
		run_length = 0;
		if (ipixelin == 1592)
            ipixelin = 1592;
		while ( (ipixelin < widthin) && (buf_in[ipixelin] == last_pix) )
		{
			ipixelin++;
			ipixelout++;
			run_length++;
		}

		/* Extend, like but faster (total time/2) than xout = round((double)ipixelin*widthout/widthin); */
        xout = ((uint32_t)((ipixelin<<1)+1)*widthout)/((uint32_t)widthin<<1);
        if (xout > ipixelout)
        {
            run_length += xout - ipixelout;
            ipixelout = xout;
        }

        /* write pixel*/
        ibuf += bsb_compress_nb(buf_out+ibuf,run_length,last_pix<<dec,max);
	}
	buf_out[ibuf++] = 0;
	return ibuf;
}

/* bsb uncompress number */

static uint16_t bsb_uncompress_nb(int typein,FILE *in, uint8_t *pixel, uint8_t decin, uint8_t maxin)
{
    uint8_t c;
    uint16_t count;

    c = fgetkapc(typein,in);

    count = (c & 0x7f);
    *pixel = count >> decin;
    count &= maxin;
    while (c & 0x80)
    {
        c = fgetkapc(typein,in);
        count = (count << 7) + (c & 0x7f);
    }
    return count+1;
}

/* read line bsb */

int bsb_uncompress_row(int typein, FILE *in, uint8_t *buf_out, uint16_t bits_in,uint16_t bits_out, uint16_t width)
{
	uint16_t    count;
	uint8_t     pixel;
	uint8_t     decin, maxin;
    uint16_t    xout = 0;

	decin = 7-bits_in;
	maxin = (1<<decin) - 1;

    /* read the line number */
    count = bsb_uncompress_nb(typein, in,&pixel,0,0x7F);

    /* no test count = line number */
    switch (bits_out)
    {
        case 1:
            while ( width )
            {
                count = bsb_uncompress_nb(typein,in,&pixel, decin,maxin);
                if (count > width) count = width;
                width -= count;
                while (count)
                {
                    buf_out[xout>>3] |= pixel<<(7-(xout&0x7));
                    xout++;
                    count--;
                }
            }
            break;
        case 4:
             while ( width )
             {
                count = bsb_uncompress_nb(typein,in,&pixel, decin,maxin);
                if (count > width) count = width;
                width -= count;
                while (count)
                {
                    buf_out[xout>>1] |= pixel<<(4-((xout&1)<<2));
                    xout++;
                    count--;
                }
            }
            break;
        case 8:
            while ( width )
            {
                count = bsb_uncompress_nb(typein,in,&pixel, decin,maxin);
                if (count > width) count = width;
                width -= count;
                while (count)
                {
                    *buf_out++ = pixel;
                    count--;
                }
            }
            break;
    }
	/* read last byte (0) */
	getc(in);
	return 0;
}

static void read_line(uint8_t *in, uint16_t bits, int width, uint8_t *colors, histogram *hist, uint8_t *out)
{
    int i;
    uint8_t c = 0;

    switch (bits)
    {
        case 1:
            for (i=0;i<width;i++)
            {
                out[i] = (in[i>>3] >> (7-(i & 0x7))) & 1;
            }
            return;
        case 4:
            for (i=0;i<width;i++)
            {
                c = in[i >> 1];
                out[i++] = colors[(c>>4) & 0xF];
                out[i] = colors[c & 0xF];
            }
            return;
        case 8:
            for (i=0;i<width;i++)
            {
                out[i] = colors[in[i]];
            }
            return;
        case 24:
        {
            Color32 cur, last;
            last.p = 0xFFFFFFFF;

            for (i=0;i<width;i++)
            {
                cur.p = ( *(uint32_t *)in & RGBMASK);
                if (last.p != cur.p)
                {
                    c = HistGetColorNum(hist, cur);
                    c = colors[HistGetColorNum(hist, cur)];
                    last = cur;
                }
                out[i] = c;
                in += 3;
            }
        }
    }
}


static uint32_t GetHistogram(wxImage &img,uint32_t bits,uint16_t width,uint16_t height,histogram *hist)
{
    uint32_t    i,j;
    Color32     cur;
    uint8_t     *line;
    histogram   *h = hist;

    if(bits != 24) {
        fprintf(stderr,"ERROR - input not 24 bits: %d\n", bits);
        return 0;
    }

    for (i=0;i<height;i++)
    {
        line = img.GetData() + i*width*3;
        for (j=0;j<width;j++)
        {
            cur.p = *(uint32_t *)(line) & RGBMASK;
            line += 3;
            if (h->color.p == cur.p)
            {
                h->count++;
                continue;
            }
            h = HistAddColor (hist, cur);
        }
    }

    return HistColorsCount(hist);
}

static int writewximgkap(FILE *out, wxImage &img, uint16_t widthout, uint16_t heightout, int max_colors)
{
    uint16_t	 i,len,cur,last;
    int          num_colors;

    uint32_t    *index;
    uint8_t		*buf_in,*buf_out;
    int         bits_in,bits_out;

    Color32     palkap[256];
    uint8_t     colors[256];
    histogram   hist[64];

    uint32_t    widthin,heightin;
    widthin = img.GetWidth();
    heightin = img.GetHeight();

    memset(hist,0,sizeof(hist));
    bits_in = 24; /* wximage is always 24  bits */

    /* read histogram */
    num_colors = GetHistogram(img,bits_in,widthin,heightin,hist);
    if (!num_colors)
    {
        fprintf(stderr,"ERROR - No Colors or bitmap bits %d\n",num_colors);
        return 2;
    }

    /* reduce colors */
    num_colors = HistReduce(hist,num_colors,max_colors);

    bits_out = ceil(log((double)num_colors) / log(2.0));

    /* if possible do not use colors 0 */
    len = ((1<<bits_out) > num_colors);

    /* sort palette with 0 blank and get index */
    num_colors = HistGetPalette(palkap,colors,hist,num_colors,len);
    if (!num_colors)
    {
        fprintf(stderr,"ERROR - internal GetPalette\n");
        return 2;
    }

    /* get index of colors for bist */
//    if (bits_in < 24) GetColors(colors,hist,palette);

	fprintf(out, "IFM/%d\r\n",bits_out);

    /* Write RGB tags for colormap */
    for (i = len; i < num_colors; i++)
    {
        fprintf(out, "RGB/%d,%d,%d,%d\r\n",
                i,
                palkap[i].q.rgbRed,
                palkap[i].q.rgbGreen,
                palkap[i].q.rgbBlue
            );
    }

    fputc(0x1a, out);
    fputc('\0', out);
    fputc(bits_out, out);
    
    buf_in = (uint8_t *)malloc((widthin + 4)/4*4);
    /* max space bsb encoded line can take */
    buf_out = (uint8_t *)malloc((widthout*2 + 8)/4*4);
    index = (uint32_t *)malloc((heightout + 1) * sizeof(uint32_t));
    if ((buf_in == NULL) || (buf_out == NULL) || (index == NULL))
    {
        fprintf(stderr,"ERROR - mem malloc\n");
        
        return 2;
    }
    
    last = -1;
    for (i = 0; i<heightout; i++)
    {
        /* Extend on height */
        cur = round((double)i * heightin / heightout);
        if (cur != last)
        {
            last = cur;
            read_line(img.GetData() + /*(heightin-cur-1)*/cur*widthin*3,
                      bits_in, widthin, colors, hist,buf_in);
        }
        
        /* Compress raster and write to BSB file */
        
        len = bsb_compress_row(buf_in, buf_out, bits_out, i, widthin,widthout);
        
        /* Record index table */
        index[i] = ftell(out);
        
        /* write data*/
        fwrite(buf_out, len, 1, out);
    }
    
    free(buf_in);
    free(buf_out);
    HistFree(hist);
    
    /* record start-of-index-table file tion in the index table */
    index[heightout] = ftell(out);
    
    bsb_write_index(out, heightout+1, index);
    
    free(index);
    return 0;
}

int wximgtokap(WeatherFaxImage &image, int colors, int units, const char *sd, const char *fileout)
{
    uint16_t    dpi,widthout,heightout;
    uint32_t    widthin,heightin;
    double      scale;
    double dx,dy ;
    char        datej[20];
    int         result;
    const char  *sunits;

	FILE		*out;

    sunits = "METERS";
    if (units != METERS) sunits = "FATHOMS";

    /* get latitude and longitude  */

    /* Read image file */

    wxImage &img = image.m_mappedimg;
    widthin = img.GetWidth();
    heightin = img.GetHeight();

    out = fopen(fileout, "wb");
    if (! out)
    {
        fprintf(stderr,"ERROR - Can't open KAP file %s\n",fileout);
        return 2;
    };

    /* Header comment file outut */

    /* Read date */
    {
        time_t      t;
        struct tm   *date;

        time(&t) ;
        date = localtime(&t);
        strftime(datej, sizeof(datej), "%d/%m/%Y",date);
    }

    /* Header comment file outut */
    fprintf(out,"! 2013 opencpn WeatherFax %d.%d file generator\r\n",
            PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    fprintf(out,"! Map created at %s\r\n",datej);

    /* calculate size */
    double lon0 = image.m_Coords->lon(0), lon1 = image.m_Coords->lon(widthin);
    double lat0 = image.m_Coords->lat(0), lat1 = image.m_Coords->lat(heightin);
    dpi = 254;

#if 0
    double lx = lontox(lon1)-lontox(lon0);
    if (lx < 0) lx = -lx;
    double ly = lattoy_WS84(lat0)-lattoy_WS84(lat1);
    if (ly < 0) ly = -ly;

    /* calculate extend widthout heightout */
    dx = heightin * lx / ly - widthin;
    dy = widthin * ly / lx - heightin;

    widthout = widthin ;
    if (dy < 0) widthout = (int)round(widthin + dx) ;
    heightout = (int)round(widthout * ly / lx) ;

    fprintf(out,"! Extend widthin %d heightin %d to widthout %d heightout %d\r\n",
            widthin,heightin,widthout,heightout);

    scale = (1-(widthin/lx) / (heightin/ly)) *100;
    if ((scale > 5) || (scale < -5))
    {
   		fprintf(stderr,"ERROR - size of image is not correct\n");
        fprintf(stderr,"\tExtend widthin %d heightin %d to widthout %d heightout %d\n",
            widthin,heightin,widthout,heightout);
        fclose(out);
        return 2;
    }
#else
    widthout = widthin;
    heightout = heightin;
#endif

    /* calculate resolution en size in meters */

    dx = postod((lat0+lat1)/2,lon0,(lat0+lat1)/2,lon1);
    dy = postod(lat0,lon0,lat1,lon0);
    fprintf(out,"! Size in milles %.2f x %.2f\r\n",dx,dy) ;

    scale = round(dy*18520000.0*dpi/(heightout*254));

    if (units == METERS)
    {
        dx = dx*1852.0/(double)widthout;
        dy = dy*1852.0/(double)heightout;
    }
    else
    {
        dx = dx*1157500./((double)widthout*1143.);
        dy = dy*1157500./((double)heightout*1143.);
    }

    fprintf(out,"! Resolution units %s - %.2fx%.2f -> %.0f at %d dpi\r\n", sunits, dx,dy,scale,dpi) ;


    /* Write KAP header */
    fputs("VER/3.0\r\n", out);
    fputs("OST/1\r\n", out);
    fprintf(out,"CED/SE=1,RE=1,ED=%s\r\n",datej);

    /* write filename and date*/
    fprintf(out,"BSB/NA=%s\r\n", (const char*)image.m_Coords->name.mb_str());

    fprintf(out,"    NU=UNKNOWN,RA=%d,%d,DU=%d\r\n",widthout,heightout,dpi);
    fprintf(out,"KNP/SC=%0.f,GD=WGS 84,PR=MERCATOR,PP=%.2f\r\n", scale,0.0);
    fputs("    PI=UNKNOWN,SP=UNKNOWN,SK=0.0,TA=90\r\n", out);
    fprintf(out,"    UN=%s,SD=%s,DX=%.2f,DY=%.2f\r\n", sunits, sd,dx,dy);

#if 0
    fprintf(out,"REF/1,%u,%u,%f,%f\r\n",0,       0,        lat0,lon0);
    fprintf(out,"REF/2,%u,%u,%f,%f\r\n",widthout,0,        lat0,lon1);
    fprintf(out,"REF/3,%u,%u,%f,%f\r\n",widthout,heightout,lat1,lon1);
    fprintf(out,"REF/4,%u,%u,%f,%f\r\n",0,       heightout,lat1,lon0);
#endif

#if 1
    double latr0 = image.m_Coords->lat1, lonr0 = image.m_Coords->lon1;
    double latr1 = image.m_Coords->lat2, lonr1 = image.m_Coords->lon2;
    unsigned int x0 = image.m_Coords->p1.x, y0 = image.m_Coords->p1.y;
    unsigned int x1 = image.m_Coords->p2.x, y1 = image.m_Coords->p2.y;

    fprintf(out,"REF/1,%u,%u,%f,%f\r\n",x0,y0,latr0,lonr0);
    fprintf(out,"REF/2,%u,%u,%f,%f\r\n",x1,y0,latr0,lonr1);
    fprintf(out,"REF/3,%u,%u,%f,%f\r\n",x1,y1,latr1,lonr1);
    fprintf(out,"REF/4,%u,%u,%f,%f\r\n",x0,y1,latr1,lonr0);
#endif

    fprintf(out,"PLY/1,%f,%f\r\n",lat0,lon0);
    fprintf(out,"PLY/2,%f,%f\r\n",lat0,lon1);
    fprintf(out,"PLY/3,%f,%f\r\n",lat1,lon1);
    fprintf(out,"PLY/4,%f,%f\r\n",lat1,lon0);

    fprintf(out,"DTM/%.6f,%.6f\r\n", 0.0, 0.0);

    result = writewximgkap(out,img,widthout,heightout,colors);
    fclose(out);

    return result;
}
