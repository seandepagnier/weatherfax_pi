/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Sean D'Epagnier                                 *
 *   sean at depagnier dot com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/progdlg.h>

#include "WeatherFaxImage.h"

/* page 1 */
void WeatherFaxImage::MakePhasedImage()
{
    /* subtract 1 to get rid of manged stuff from phasing or skew fix */
    m_phasedimg.Create(m_origimg.GetWidth(), m_origimg.GetHeight()-1);
    unsigned char *dd = m_phasedimg.GetData();
    unsigned char *d = m_origimg.GetData() + phasing*3;
    int linelen = 3*m_origimg.GetWidth(), imgsize = linelen*m_origimg.GetHeight();
    double skewpos = 0;
    while(d+linelen < m_origimg.GetData()+imgsize &&
          dd+linelen < m_phasedimg.GetData()+imgsize) {
        switch(filter) {
        case 0: memcpy(dd, d, linelen); break;
        case 1:
            for(int i = 0; i<linelen; i++)
                dd[i] = (d[i]>>5)<<5;
            break;
        case 2:
            for(int i = 0; i<linelen; i++)
                dd[i] = d[i] < 192 ? d[i] : 255;
            break;
        }            

        d += linelen;
        dd += linelen;
        skewpos += skew / 100.0;

        while(skewpos < 1) {
            d+=3;
            skewpos++;
        }

        while(skewpos > 1) {
            dd[0] = 255;
            dd[1] = 255;
            dd[2] = 255;
            dd+=3;
            skewpos--;
        }
    }

    /* apply rotation */
    switch(rotation) {
    case 1:
        m_phasedimg = m_phasedimg.Rotate90(false);
        break;
    case 3:
        m_phasedimg = m_phasedimg.Rotate90(true);
    case 2:
        m_phasedimg = m_phasedimg.Rotate90(true);
        break;
    }

    /* invalidate mapped image */
    m_mappedimg = wxNullImage;
}

/* page 2 */

inline unsigned char ImageValueMono(unsigned char *data, int w, int x, int y)
{
    return data[3*(y*w + x) + 0];
}

void ImageValue(unsigned char *data, int w, int x, int y, unsigned char c[3])
{
    for(int i = 0; i<3; i++)
        c[i] = data[3*(y*w + x) + i];
}

inline unsigned char InterpColorMono(unsigned char c0, unsigned char c1, double d)
{
    return (1-d)*c0 + d*c1;
}

void InterpColor(unsigned char c0[3], unsigned char c1[3], double d, unsigned char c[3])
{
    for(int i=0; i<3; i++)
        c[i] = (1-d)*c0[i] + d*c1[i];
}

void ImageInterpolatedValueMono(unsigned char *data, int w, double x, double y, unsigned char c[3])
{
    int x0 = floor(x), x1 = ceil(x), y0 = floor(y), y1 = ceil(y);
    unsigned char nc[4], xc[2];
    nc[0] = ImageValueMono(data, w, x0, y0);
    nc[1] = ImageValueMono(data, w, x1, y0);
    nc[2] = ImageValueMono(data, w, x0, y1);
    nc[3] = ImageValueMono(data, w, x1, y1);

    double d0 = x - x0, d1 = y - y0;
    xc[0] = InterpColorMono(nc[0], nc[1], d0);
    xc[1] = InterpColorMono(nc[2], nc[3], d0);
    c[0] = c[1] = c[2] = InterpColorMono(xc[0], xc[1], d1);
}

void ImageInterpolatedValue(unsigned char *data, int w, double x, double y, unsigned char c[3])
{
    int x0 = floor(x), x1 = ceil(x), y0 = floor(y), y1 = ceil(y);
    unsigned char nc[4][3], xc[2][3];
    ImageValue(data, w, x0, y0, nc[0]);
    ImageValue(data, w, x1, y0, nc[1]);
    ImageValue(data, w, x0, y1, nc[2]);
    ImageValue(data, w, x1, y1, nc[3]);

    double d0 = x - x0, d1 = y - y0;
    InterpColor(nc[0], nc[1], d0, xc[0]);
    InterpColor(nc[2], nc[3], d0, xc[1]);
    InterpColor(xc[0], xc[1], d1, c);
}

void WeatherFaxImage::InputToMercator(double px, double py, double &mx, double &my)
{
    double dx, dy;

    /* apply offsets */
    dx = px - inputpole.x;
    dy = fabs(py - inputpole.y);

    /* map coordinates */
    double theta, pp, x, y;
    switch(mapping) {
    case MERCATOR:
        x = dx;
        y = dy;
        break;
    case POLAR:
        /* for polar stereographic */
        theta = atan2(dx, dy);
        x = theta*m_phasedimg.GetWidth();
        pp = 1 - 4/M_PI*atan(hypot(dx, dy) / fabs(inputheight));
        break;
    case FIXED_FLAT:
        x = dx;
        pp = 1 - dy / inputheight;
        break;
    }

    /* if not mercator, it is fixed and needs conversion here */
    if(mapping != MERCATOR) {
        double s = sin(pp * (M_PI/2));
        y = .5 * log((1 + s) / (1 - s));
        y *= m_phasedimg.GetHeight();
    }

    /* apply scale */
    x*=mappingwidthmultiplier;
    y*=mappingheightmultiplier;

    /* apply offsets */
    mx = mercatoroffset.x + x;
    /* input height is negative for south-polar images */
    if(inputheight > 0)
        my = mercatoroffset.y - y;
    else
        my = y - mercatoroffset.y;
}

/* the exact inverse of InputToMercator */
void WeatherFaxImage::MercatorToInput(double mx, double my, double &px, double &py)
{
    double x = mx, y = my;

    /* apply offsets */
    x = x - mercatoroffset.x;
    /* input height is negative for south-polar images */
    if(inputheight > 0)
        y = mercatoroffset.y - y;
    else
        y = y - mercatoroffset.y;

    /* apply scale */
    x /= mappingwidthmultiplier;
    y /= mappingheightmultiplier;

    /* if not mercator, it is fixed and needs conversion here */
    double pp;
    if(mapping != MERCATOR) {
        y /= m_phasedimg.GetHeight();
        pp = 4/M_PI*atan(exp(y)) - 1;
    }

    /* unmap coordinates */
    double dx, dy;
    switch(mapping) {
    case MERCATOR:
        dx = x;
        dy = y;
        break;
    case POLAR:
    {
        /* for polar stereographic */
        double d = tan((1 - pp) * M_PI / 4)*inputheight;
        double theta = x / m_phasedimg.GetWidth();
        if(inputheight < 0) /* south polar has negative height */
            theta = -theta;
        dx = d*sin(theta), dy = d*cos(theta);
    } break;
    case FIXED_FLAT:
        dx = x;
        dy = (1-pp) * inputheight;
        break;
    }

    /* apply offsets */
    px = dx + inputpole.x;
    py = dy + inputpole.y;
}

void WeatherFaxImage::MakeMappedImage(wxWindow *parent)
{
    /* in this case we can just copy */
    if(mappingheightmultiplier == 1 && mappingwidthmultiplier == 1 &&
       mapping == MERCATOR) {
        m_mappedimg = m_phasedimg;
        return;
    }

    int w = m_phasedimg.GetWidth(), h = m_phasedimg.GetHeight();

    inputheight = inputequator - inputpole.y;

    /* determine location of offsets and image boundaries */
    mercatoroffset.x = 0;
    mercatoroffset.y = 0;

    /* four corners of input */
    double p1x, p2x, p3x, p4x, p5x, p6x;
    double p1y, p2y, p3y, p4y, p5y, p6y;
    InputToMercator(0, 0, p1x, p1y);
    InputToMercator(w, 0, p2x, p2y);
    InputToMercator(w, h, p3x, p3y);
    InputToMercator(0, h, p4x, p4y);

    int minp = wxMin(p1x, p4x);
    int maxp = wxMax(p2x, p3x);

    mercatoroffset.x = -minp;

    int mw = maxp - minp;

    /* for polar, one of the two points along vertical meridian often
       are the extreme, so expand height based on these */
    InputToMercator(inputpole.x, 0, p5x, p5y);
    InputToMercator(inputpole.x, h, p6x, p6y);

    minp = wxMin(p1y, p2y);
    minp = wxMin(minp, p5y);

    mercatoroffset.y = -minp;

    maxp = wxMax(p3y, p4y);
    maxp = wxMax(maxp, p6y);

    int mh = maxp - minp;

    if(mw < 0 || mh < 0) {
        wxMessageDialog w( parent, _("Resulting image has negative dimensions, aborting\n"),
                           _("Mapping Failed"), wxOK | wxICON_ERROR );
        w.ShowModal();
        return;
    }

    const int maxsize = 100;
        /* bigger than 100 megabytes is pretty huge, user probably fcked up */
    if(mw * mh > maxsize*1024*1024) {
        wxMessageDialog w
            ( parent, wxString::Format(_("Resulting image larger than %dMB, aborting\n"),
                                     maxsize),
              _("Mapping Failed"), wxOK | wxICON_ERROR );
        w.ShowModal();
        return ;
    }

/* now generate mercator image from input image */
    m_mappedimg.Create(mw, mh);
    wxProgressDialog progressdialog(
        _("Mapping Weather Fax Image"), _("Weather Fax Mapper"), mw, parent,
        wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME | wxPD_CAN_ABORT);

    unsigned char *d = m_phasedimg.GetData(), *md = m_mappedimg.GetData();
    for(int x=0; x<mw; x++) {
        if(x%200 == 0 && !progressdialog.Update(x))
            break;

        for(int y=0; y<mh; y++) {
            double px, py;
            unsigned char *cd = md + 3*(y*mw + x);

            MercatorToInput(x, y, px, py);

            if(px >= 0 && py >= 0 && px < w-1 && py < h-1)
#if 0 /* mono with interpolation for speed */
                ImageInterpolatedValueMono
#elif 1                
                ImageInterpolatedValue
#else /* faster without interpolation, looks like crap */
                ImageValue
#endif
                    (d, w, px, py, cd);
            else
                cd[0] = cd[1] = cd[2] = 255;
        }
    }
}
