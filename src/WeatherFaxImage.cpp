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
WX_DEFINE_LIST(WeatherFaxImageCoordinateList);

void WeatherFaxImage::FreeData()
{
    if(m_gltextures) {
        glDeleteTextures(m_numgltexturesh*m_numgltexturesw, m_gltextures);
        delete [] m_gltextures;
        m_gltextures = NULL;
    }

    delete m_CacheBitmap;
    m_CacheBitmap = NULL;
}

/* page 1 */
void WeatherFaxImage::MakePhasedImage()
{
    if(!m_origimg.IsOk()) {
        m_phasedimg.Create(1, 1);
        return;
    }

    /* subtract 1 to get rid of manged stuff from phasing or skew fix */
    int linelen = 3*m_origimg.GetWidth();
    unsigned char *d = m_origimg.GetData() + phasing*3;
    unsigned char *origimgend = d + linelen*m_origimg.GetHeight();

    m_phasedimg.Create(m_origimg.GetWidth(), m_origimg.GetHeight());
    unsigned char *dd = m_phasedimg.GetData();
    unsigned char *phasedimgend = dd + linelen*m_phasedimg.GetHeight();

    double skewpos = 0;
    while(d+linelen < origimgend && dd+linelen < phasedimgend) {
        switch(filter) {
        case 0: memcpy(dd, d, linelen); break;
        case 1:
            memcpy(dd, d, 3);
            for(int i = 3; i<linelen-3; i++)
                if(d[i-3] > 64 && d[i+3] > 64)
                    dd[i] = 255;
                else
                    dd[i] = d[i];
            memcpy(dd + linelen-3, d + linelen-3, 3);
            break;
        case 2:
            memcpy(dd, d, 6);
            for(int i = 6; i<linelen-6; i++) {
                int cnt = 0;
                for(int j=-6; j<9; j+=3)
                    if(d[i+j] > 64) cnt++;
                if(cnt > 3)
                    dd[i] = 255;
                else
                    dd[i] = d[i];
            }
            memcpy(dd + linelen-6, d + linelen-6, 6);
            break;
        case 3:
            memcpy(dd, d, 6);
            for(int i = 6; i<linelen-6; i++) {
                int cnt = 0;
                for(int j=-6; j<9; j+=3)
                    if(d[i+j] > 128) cnt++;
                if(cnt > 2)
                    dd[i] = 255;
                else
                    dd[i] = d[i];
            }
            memcpy(dd + linelen-6, d + linelen-6, 6);
            break;
        case 4:
            for(int i = 0; i<linelen; i++)
                dd[i] = d[i] < 192 ? d[i] : 255;
            break;
        case 5:
            for(int i = 0; i<linelen; i++)
                dd[i] = d[i] < 128 ? d[i] : 255;
            break;
        case 6:
            for(int i = 0; i<linelen; i++)
                dd[i] = d[i] < 64 ? d[i] : 255;
            break;
        }

        d += linelen;
        dd += linelen;
        skewpos += skew / 1000.0;

        while(skewpos < -1) {
            d+=3;
            skewpos++;
        }

        while(skewpos > 1 && dd+3 < phasedimgend && linelen > 3) {
            memcpy(dd, dd-3, 3);
            dd+=3;
            skewpos--;
        }
    }

    /* crop last line of phased image */
    m_phasedimg.Resize(wxSize(m_phasedimg.GetWidth(), m_phasedimg.GetHeight()-1),
                       wxPoint(0, 0));

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
    dx = px - m_Coords->inputpole.x;
    dy = py - m_Coords->inputpole.y;

    /* map coordinates */
    double theta, pp, x, y;
    switch(m_Coords->mapping) {
    case WeatherFaxImageCoordinates::MERCATOR:
        x = dx;
        y = dy;
        break;
    case WeatherFaxImageCoordinates::POLAR:
    case WeatherFaxImageCoordinates::CONIC:
    {
        /* for polar stereographic, or lambert conformal conic */
        dx *= m_Coords->inputtrueratio;

        theta = atan2(dx, fabs(dy));
        x = theta*m_phasedimg.GetWidth();

        double z = hypot(dx, dy) / inputheight;
        double q = 4/M_PI*atan(z);
        pp = q + (inputheight > 0 ? -1 : 1); /* inputheight < 0 for south polar */
    } break;
    case WeatherFaxImageCoordinates::FIXED_FLAT:
        x = dx;
        pp = dy / inputheight - 1;
        break;
    }

    /* if not mercator, it is fixed and needs conversion here */
    if(m_Coords->mapping != WeatherFaxImageCoordinates::MERCATOR) {
        double s = sin(pp * (M_PI/2));
        y = .5 * log((1 + s) / (1 - s));
        y *= m_phasedimg.GetHeight();
    }

    /* apply scale */
    x*=m_Coords->mappingmultiplier*m_Coords->mappingratio;
    y*=m_Coords->mappingmultiplier;

    /* apply offsets */
    mx = mercatoroffset.x + x;
    my = mercatoroffset.y + y;
}

/* the exact inverse of InputToMercator */
void WeatherFaxImage::MercatorToInput(double mx, double my, double &px, double &py)
{
    double x, y;

    /* apply offsets */
    x = mx - mercatoroffset.x;
    y = my - mercatoroffset.y;

    /* apply scale */
    x /= m_Coords->mappingmultiplier*m_Coords->mappingratio;
    y /= m_Coords->mappingmultiplier;

    /* if not mercator, it is fixed and needs conversion here */
    double pp;
    if(m_Coords->mapping != WeatherFaxImageCoordinates::MERCATOR) {
        y /= m_phasedimg.GetHeight();
        pp = 4/M_PI*atan(exp(y)) - 1;
    }

    /* unmap coordinates */
    double dx, dy;
    switch(m_Coords->mapping) {
    case WeatherFaxImageCoordinates::MERCATOR:
        dx = x;
        dy = y;
        break;
    case WeatherFaxImageCoordinates::POLAR:
    case WeatherFaxImageCoordinates::CONIC:
    {
        /* for polar stereographic, or lambert conformal conic */
        double q = inputheight > 0 ? 1 + pp : 1 - pp; /* inputheight < 0 for south polar */
        double z = tan(q * M_PI / 4);
        double d = z * inputheight;

        double theta = x / m_phasedimg.GetWidth();
        dx = fabs(d)*sin(theta), dy = d*cos(theta);
        dx /= m_Coords->inputtrueratio;
    } break;
    case WeatherFaxImageCoordinates::FIXED_FLAT:
        dx = x;
        dy = (1+pp) * fabs(inputheight);
        break;
    }

    /* apply offsets */
    px = dx + m_Coords->inputpole.x;
    py = dy + m_Coords->inputpole.y;
}

bool WeatherFaxImage::MakeMappedImage(wxWindow *parent, bool paramsonly)
{
    int w = m_phasedimg.GetWidth(), h = m_phasedimg.GetHeight();

    inputheight = m_Coords->inputequator - m_Coords->inputpole.y;

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
    InputToMercator(m_Coords->inputpole.x, 0, p5x, p5y);
    InputToMercator(m_Coords->inputpole.x, h, p6x, p6y);

    minp = wxMin(p1y, p2y);
    minp = wxMin(minp, p5y);

    mercatoroffset.y = -minp;

    maxp = wxMax(p3y, p4y);
    maxp = wxMax(maxp, p6y);

    int mh = maxp - minp;

    /* only computing parameters, not the actual mapping */
    if(paramsonly)
        return true;

    /* in this simple yet common case we can quickly copy */
    if(m_Coords->mappingmultiplier == 1 &&
       m_Coords->mappingratio == 1 &&
       m_Coords->mapping == WeatherFaxImageCoordinates::MERCATOR) {
        m_mappedimg = m_phasedimg;
        return true;
    }

    if(mw < 0 || mh < 0) {
        wxMessageDialog w( parent, _("Resulting image has negative dimensions, aborting\n"),
                           _("Mapping Failed"), wxOK | wxICON_ERROR );
        w.ShowModal();
        return false;
    }

    const int maxsize = 30;
        /* bigger than 30 megabytes is pretty huge and possibly too slow */
    if(mw * mh > maxsize*1024*1024) {
        wxMessageDialog w
            ( parent, wxString::Format(_("Resulting image larger than %dMB, aborting\n"),
                                     maxsize),
              _("Mapping Failed"), wxOK | wxICON_ERROR );
        w.ShowModal();
        return false;
    }

/* now generate mercator image from input image */
    m_mappedimg.Create(mw, mh);
    wxProgressDialog progressdialog(
        _("Mapping Weather Fax Image"), _("Weather Fax Mapper"), mw, parent,
        wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME | wxPD_CAN_ABORT);

    unsigned char *d = m_phasedimg.GetData(), *md = m_mappedimg.GetData();
    for(int x=0; x<mw; x++) {
        if(x%200 == 0 && !progressdialog.Update(x))
            return false;

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
    return true;
}

bool WeatherFaxImage::GetOverlayCoords(PlugIn_ViewPort *vp, wxPoint &p0, wxPoint &pwh, int &w, int &h)
{
    if(!m_Coords)
        return false;

    wxImage &img = m_mappedimg;

    if(!img.IsOk())
        return false;

    w=img.GetWidth();
    h=img.GetHeight();

    double lon0 = m_Coords->lon(0), lonw = m_Coords->lon(w);

    /* skip coordinates that go the long way around the world */
    if(lon0+180 < vp->clon && lonw+180 > vp->clon)
        return false;
    if(lon0-180 < vp->clon && lonw-180 > vp->clon)
        return false;

    double lat1 = m_Coords->lat1, lon1 = m_Coords->lon1;
    double lat2 = m_Coords->lat2, lon2 = m_Coords->lon2;
    
    wxPoint p1, p2;
    GetCanvasPixLL( vp, &p1, lat1, lon1 );
    GetCanvasPixLL( vp, &p2, lat2, lon2 );

    /* now extrapolate to 0-w and 0-h from p1 p2 pa etc.. 
    (m_Coords->p2.x-m_Coords->p1.x)/(p2.x-p1.x) = (m_Coords->p1.x - 0) / (p1.x - p0.x)
    p0.x = p1.x - (p2.x-p1.x) * (m_Coords->p1.x - 0) /
    (m_Coords->p2.x-m_Coords->p1.x)
    */

    double divx = (m_Coords->p2.x-m_Coords->p1.x);
    double divy = (m_Coords->p2.y-m_Coords->p1.y);
    p0.x = p1.x - (p2.x-p1.x) * (m_Coords->p1.x - 0) / divx;
    p0.y = p1.y - (p2.y-p1.y) * (m_Coords->p1.y - 0) / divy;

    pwh.x = p1.x - (p2.x-p1.x) * (m_Coords->p1.x - w) / divx;
    pwh.y = p1.y - (p2.y-p1.y) * (m_Coords->p1.y - h) / divy;
    return true;
}

void WeatherFaxImage::RenderImage(wxDC &dc, PlugIn_ViewPort *vp)
{
    wxPoint p1, p2;
    int w, h;
    if(!GetOverlayCoords(vp, p1, p2, w, h))
        return;

    wxImage &img = m_mappedimg;

    int iw = p2.x-p1.x, ih = p2.y-p1.y;

    if(!m_CacheBitmap || m_CacheBitmap->GetWidth() != iw || m_CacheBitmap->GetHeight() != ih) {
        delete m_CacheBitmap;
        m_CacheBitmap = NULL;

        /* dont go too huge */
        if(iw > 4096 || ih > 4096)
            return;

        wxRect r(m_Coords->p1.x, m_Coords->p1.y, w, h);

#if 0
        wxImage stretchedimg = subimg.Scale(iw, ih); /* doesn't support saturated transparency, or invert */
#else        
        unsigned char *subdata = img.GetData();
        unsigned char *stretcheddata = (unsigned char*)malloc(iw*ih*3); /* malloc needed for wximage */
        for(int y=0; y<ih; y++) {
            int sy = y*h/ih;
            for(int x=0; x<iw; x++) {
                int sx = x*w/iw;
                for(int c=0; c<3; c++) {
                    wxUint8 v = subdata[3*(w*sy+sx)+c];
                    /* this is wonky because sometimes it makes colors bright
                       rather than white (triggering mask) but it looks cool... */
                    if(v > 255-m_iWhiteTransparency)
                        v = 255;
                    else if(m_bInvert)
                        v = 255-v;
                    stretcheddata[3*(iw*y+x)+c] = v;
                }
            }
        }

        wxImage stretchedimg(iw, ih, stretcheddata);
#endif

        m_CacheBitmap = new wxBitmap(stretchedimg);
        if(m_iWhiteTransparency)
            m_CacheBitmap->SetMask(new wxMask(*m_CacheBitmap, wxColour(255, 255, 255)));
    }

    dc.DrawBitmap(*m_CacheBitmap, p1.x, p1.y, true);
}

void WeatherFaxImage::RenderImageGL(PlugIn_ViewPort *vp)
{
    const int maxtexsize = 1024; /* all gfx should support at least this */

    wxPoint p1, p2;
    int w, h;
    if(!GetOverlayCoords(vp, p1, p2, w, h))
        return;

    wxImage &img = m_mappedimg;

    if(!m_gltextures) {
        m_numgltexturesh = ceil((double)h/maxtexsize);
        m_numgltexturesw = ceil((double)w/maxtexsize);
        m_gltextures = new unsigned int[m_numgltexturesh*m_numgltexturesw];
        glGenTextures(m_numgltexturesh*m_numgltexturesw, m_gltextures);

        unsigned char *idata = new unsigned char[4*maxtexsize*maxtexsize];
        for(unsigned int ty = 0;  ty<m_numgltexturesh; ty++) {
            for(unsigned int tx = 0; tx<m_numgltexturesw; tx++) {
                unsigned int th = (ty == m_numgltexturesh-1) ? h - ty*maxtexsize : maxtexsize;
                unsigned int tw = (tx == m_numgltexturesw-1) ? w - tx*maxtexsize : maxtexsize;

                int tox = tx*maxtexsize;
                int toy = ty*maxtexsize;

                glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_gltextures[ty*m_numgltexturesw + tx]);
        
                glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP );
                glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP );
                glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        
#if 1 /* I think we can delete these */
                glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
                glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
                glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );
#endif
                unsigned char *data = img.GetData();
                for(unsigned int y=0; y<th; y++) {
                    for(unsigned int x=0; x<tw; x++) {
                        int o = 3*((toy+y)*w + tox+x);
                        int p = 4*(y*tw+x);
                        wxUint8 r = data[o + 0], g = data[o + 1], b = data[o + 2];
                        if(m_bInvert)
                            idata[p + 0] = r,  idata[p + 1] = g, idata[p + 2] = b;
                        else
                            idata[p + 0] = 255-r,  idata[p + 1] = 255-g, idata[p + 2] = 255-b;
                
                        wxUint8 a = 255 + -(r+g+b)/3*m_iWhiteTransparency/255; /* white alpha */
                        idata[p+3] = a;
                    }
                }

                glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA,
                         tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, idata);
            }
        }
        delete [] idata;
    }
        
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT);

    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_MULTISAMPLE );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    glColor4ub(255, 255, 255, 255-m_iTransparency);

    for(unsigned int ty = 0;  ty<m_numgltexturesh; ty++) {
        for(unsigned int tx = 0; tx<m_numgltexturesw; tx++) {
            unsigned int th = (ty == m_numgltexturesh-1) ? h - ty*maxtexsize : maxtexsize;
            unsigned int tw = (tx == m_numgltexturesw-1) ? w - tx*maxtexsize : maxtexsize;

            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_gltextures[ty*m_numgltexturesw + tx]);
    
            wxPoint p1u = wxPoint(p1.x + (p2.x-p1.x)*maxtexsize*tx/w,
                                  p1.y + (p2.y-p1.y)*maxtexsize*ty/h);
            wxPoint p2u = wxPoint(p1.x + (p2.x-p1.x)*(maxtexsize*tx+tw)/w,
                                  p1.y + (p2.y-p1.y)*(maxtexsize*ty+th)/h);

            glBegin(GL_QUADS);
            glTexCoord2i(0,   0), glVertex2i(p1u.x, p1u.y);
            glTexCoord2i(tw,  0), glVertex2i(p2u.x, p1u.y);
            glTexCoord2i(tw, th), glVertex2i(p2u.x, p2u.y);
            glTexCoord2i(0,  th), glVertex2i(p1u.x, p2u.y);
            glEnd();
        }
    }
    
    glPopAttrib();
}
