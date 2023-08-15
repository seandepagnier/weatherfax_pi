/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Sean D'Epagnier                                 *
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

#include <wx/wx.h>
#include <wx/progdlg.h>

#include "defs.h"
#include "WeatherFaxImage.h"

#ifdef __OCPN__ANDROID__
#include <qopengl.h>
#include "GL/gl_private.h"
#include "GLES2/gl2.h"
#include <qdebug.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "pi_shaders.h"
#include "pidc.h"

WX_DEFINE_LIST(WeatherFaxImageCoordinateList);

#if !defined(GL_CLAMP_TO_EDGE)
#define GL_CLAMP_TO_EDGE			0x812F
#endif

#if !defined(GL_TEXTURE_RECTANGLE_ARB)
#define GL_TEXTURE_RECTANGLE_ARB          0x84F5
#endif

static int texture_format;

static GLboolean QueryExtension( const char *extName )
{
    /*
     ** Search for extName in the extensions string. Use of strstr()
     ** is not sufficient because extension names can be prefixes of
     ** other extension names. Could use strtok() but the constant
     ** string returned by glGetString might be in read-only memory.
     */
    char *p;
    char *end;
    int extNameLen;

    extNameLen = strlen( extName );

    p = (char *) glGetString( GL_EXTENSIONS );
    if( NULL == p ) {
        return GL_FALSE;
    }

    end = p + strlen( p );

    while( p < end ) {
        int n = strcspn( p, " " );
        if( ( extNameLen == n ) && ( strncmp( extName, p, n ) == 0 ) ) {
            return GL_TRUE;
        }
        p += ( n + 1 );
    }
    return GL_FALSE;
}


wxString WeatherFaxImageCoordinates::MapName(MapType type)
{
    switch(type) {
    case MERCATOR:   return _T("Mercator");
    case POLAR:      return _T("Polar");
    case CONIC:      return _T("Conic");
    case FIXED_FLAT: return _T("FixedFlat");
    default: break;
    }
    return _T("");
}

WeatherFaxImageCoordinates::MapType WeatherFaxImageCoordinates::GetMapType(wxString name)
{
    for(int i=0; i < MAP_TYPES; i++) {
        WeatherFaxImageCoordinates::MapType type = (WeatherFaxImageCoordinates::MapType)i;
        if(name == MapName(type))
            return type;
    }
    return MERCATOR;
}

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
        m_origimg.Create(1, 1); /* small image; so orig image is always ok to work with */
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
    
    int lastphase_pos = linelen;
    unsigned char *pcb = NULL;
    if(phase_correct_line_by_line)
        pcb = new unsigned char[linelen];
    while(d+linelen < origimgend && dd+linelen < phasedimgend) {
        int phase_pos;
        if(phase_correct_line_by_line) {
            int n = linelen/3 * .05;
            int i;
            int mintotal = -1, min = 0;
            for(i = 0; i<linelen/3; i++) {
                // assume nominally phased
                if(i > linelen/3*.1 && i < linelen/3*.9)
                    continue;

                int total = 0, j;
                for(j = 0; j<n; j++)
                    total += (n/2-abs(j-n/2))*d[3*(i+j)%linelen];
                if(total < mintotal || mintotal == -1) {
                    mintotal = total;
                    min = i;
                }
            }
            //phase_pos = (min+n/2) % linelen;
            phase_pos = min;

            // don't apply small changes in phase position
            if(abs(lastphase_pos - phase_pos) < 3)
                phase_pos = lastphase_pos;
            else
                lastphase_pos = phase_pos;
        }
        
        if(bfilter)
            for(int i = 0; i<linelen; i++)
                dd[i] = d[i] < filter ? d[i] : 255;
        else
            memcpy(dd, d, linelen);

        if(phase_correct_line_by_line) {
            memcpy(pcb, dd+3*phase_pos,linelen-3*phase_pos);
            memcpy(pcb+linelen-3*phase_pos, dd, 3*phase_pos);
            memcpy(dd, pcb, linelen);
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
    delete [] pcb;

    /* crop last line of phased image */
#if 0
    m_phasedimg.Resize(wxSize(m_phasedimg.GetWidth(), m_phasedimg.GetHeight()-1),
                       wxPoint(0, 0));
#else
//    m_phasedimg = wxImage(m_phasedimg.GetWidth(), m_phasedimg.GetHeight()-1, m_phasedimg.GetData(), true);

#endif

    /* apply rotation */
    switch(m_Coords->rotation) {
    case WeatherFaxImageCoordinates::CCW:
        m_phasedimg = m_phasedimg.Rotate90(false);
        break;
    case WeatherFaxImageCoordinates::R180:
        m_phasedimg = m_phasedimg.Rotate90(true);
    case WeatherFaxImageCoordinates::CW:
        m_phasedimg = m_phasedimg.Rotate90(true);
    default:
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
    double theta, pp = 0, x = 0, y = 0;
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

        double z = sqrt(dx*dx + dy*dy) / inputheight;
        double q = 4/M_PI*atan(z);
        pp = q + (inputheight > 0 ? -1 : 1); /* inputheight < 0 for south polar */
    } break;
    case WeatherFaxImageCoordinates::FIXED_FLAT:
        x = dx;
        pp = dy / inputheight - 1;
        break;
    default: break;
    }

    /* if not mercator, it is fixed and needs conversion here */
    if(m_Coords->mapping != WeatherFaxImageCoordinates::MERCATOR) {
        double s = sin(pp * (M_PI/2));
        y = .5 * log((1 + s) / (1 - s));
        y *= m_phasedimg.GetHeight();
    }

    /* apply scale */
    x*=m_Coords->mappingmultiplier;
    y*=m_Coords->mappingmultiplier/m_Coords->mappingratio;

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
    x /= m_Coords->mappingmultiplier;
    y /= m_Coords->mappingmultiplier/m_Coords->mappingratio;

    /* if not mercator, it is fixed and needs conversion here */
    double pp;
    if(m_Coords->mapping != WeatherFaxImageCoordinates::MERCATOR) {
        y /= m_phasedimg.GetHeight();
        pp = 4/M_PI*atan(exp(y)) - 1;
    }

    /* unmap coordinates */
    double dx = 0, dy = 0;
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
    default: break;
    }

    /* apply offsets */
    px = dx + m_Coords->inputpole.x;
    py = dy + m_Coords->inputpole.y;
}

bool WeatherFaxImage::MakeMappedImage(wxWindow *parent, bool paramsonly)
{
    if(!m_phasedimg.IsOk()) {
        m_phasedimg.Create(1, 1); /* small image; so image is always ok to work with */
        return false;
    }

    int w = m_phasedimg.GetWidth(), h = m_phasedimg.GetHeight();

    inputheight = m_Coords->inputequator - m_Coords->inputpole.y;

    /* determine location of offsets and image boundaries */
    mercatoroffset.x = 0;
    mercatoroffset.y = 0;

    double p1x, p2x, p3x, p4x, p5x, p6x;
    double p1y, p2y, p3y, p4y, p5y, p6y;

    /* four corners of input */
    InputToMercator(0, 0, p1x, p1y);
    InputToMercator(w, 0, p2x, p2y);
    InputToMercator(w, h, p3x, p3y);
    InputToMercator(0, h, p4x, p4y);

    double minp = wxMin(p1x, p4x);
    double maxp = wxMax(p2x, p3x);

    mercatoroffset.x = -minp;

    if(isnan(minp) || isnan(maxp))
        return false;

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

    if(isnan(minp) || isnan(maxp))
        return false;

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
    double maxscale = maxsize*1024*1024 / ((double)mw * mh);
    if(maxscale < 1) {
        wxMessageDialog w
            ( parent, wxString::Format(
                _("Resulting image larger than %dMB\n\
Try changing size parameter to a smaller value. (less than %.2f)\naborting\n"),
                maxsize, sqrt(maxscale)*m_Coords->mappingmultiplier),
              _("Mapping Failed"), wxOK | wxICON_ERROR );
        w.ShowModal();
        return false;
    }

    /* now generate mercator image from input image */
    m_mappedimg.Create(mw, mh);
    wxProgressDialog progressdialog(
        _("Mapping Weather Fax Image"), _("Weather Fax Mapper"), mw, parent,
        wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);

    unsigned char *d = m_phasedimg.GetData(), *md = m_mappedimg.GetData();
    for(int x=0; x<mw; x++) {
        if(x%200 == 0 && !progressdialog.Update(x))
            return false;

        for(int y=0; y<mh; y++) {
            double px, py;
            unsigned char *cd = md + 3*(y*mw + x);

            MercatorToInput(x, y, px, py);

            if(px >= 0 && py >= 0 && px < w-1 && py < h-1)
                ImageInterpolatedValue(d, w, px, py, cd);
            else
                cd[0] = cd[1] = cd[2] = 255;
        }
    }
    return true;
}

bool WeatherFaxImage::GetOverlayCoords(PlugIn_ViewPort *vp, wxPoint p[3], int &w, int &h)
{
    if(!m_Coords)
        return false;

    wxImage &img = m_mappedimg;

    if(!img.IsOk())
        return false;

    w=img.GetWidth();
    h=img.GetHeight();

    double lat0 = m_Coords->lat(0), lon0 = m_Coords->lon(0);
    double lat1 = m_Coords->lat(h), lon1 = m_Coords->lon(w);

    /* skip coordinates that go the long way around the world */
    if(lon0+180 < vp->clon && lon1+180 > vp->clon)
        return false;
    if(lon0-180 < vp->clon && lon1-180 > vp->clon)
        return false;

    /* prefer double precision version when it is available */
    GetCanvasPixLL( vp, &p[0], lat0, lon0 );
    GetCanvasPixLL( vp, &p[1], lat0, lon1 );
    GetCanvasPixLL( vp, &p[2], lat1, lon0 );

    return true;
}

void WeatherFaxImage::RenderImage(wxDC &dc, PlugIn_ViewPort *vp)
{
    wxPoint p[4];
    int w, h;
    if(vp->rotation)
        return;

    if(!GetOverlayCoords(vp, p, w, h))
        return;

    wxPoint p1 = p[0], p2 = wxPoint(p[1].x, p[2].y);

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

    wxPoint p[3];
    int w, h;
    if(!GetOverlayCoords(vp, p, w, h))
        return;

    wxImage &img = m_mappedimg;

    if(!m_gltextures) {
        if(!texture_format) {
            if( QueryExtension( "GL_ARB_texture_non_power_of_two" ) ||
                QueryExtension( "GL_OES_texture_npot" ) )
                texture_format = GL_TEXTURE_2D;
            else if( QueryExtension( "GL_ARB_texture_rectangle" ) )
                texture_format = GL_TEXTURE_RECTANGLE_ARB;
            else {
                static bool once = false;
                if(!once) {
                    wxMessageDialog w( NULL, _("Graphics hardware not supported (Disable OpenGL)\n"),
                                   _("Weather Fax"), wxOK | wxICON_ERROR );
                    w.ShowModal();
                    once = true;
                }
                return;
            }
        }

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

                glBindTexture(texture_format, m_gltextures[ty*m_numgltexturesw + tx]);
        
                glTexParameteri( texture_format, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
                glTexParameteri( texture_format, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
                glTexParameteri( texture_format, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                glTexParameteri( texture_format, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        
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

                glTexImage2D(texture_format, 0, GL_RGBA,
                         tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, idata);
            }
        }
        delete [] idata;
    }
        
#ifdef USE_GLSL
    glEnable(texture_format);
    glEnable( GL_BLEND );
    glUseProgram( pi_texture_2DA_shader_program );

    // Get pointers to the attributes in the program.
    GLint mPosAttrib = glGetAttribLocation( pi_texture_2DA_shader_program, "aPos" );
    GLint mUvAttrib  = glGetAttribLocation( pi_texture_2DA_shader_program, "aUV" );
    
    // Set up the texture sampler to texture unit 0
    GLint texUni = glGetUniformLocation( pi_texture_2DA_shader_program, "uTex" );
    glUniform1i( texUni, 0 );

        // Disable VBO's (vertex buffer objects) for attributes.
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    
    float colorv[4] = {0, 0, 0, -m_iTransparency/255.0f};

    GLint colloc = glGetUniformLocation(pi_texture_2DA_shader_program,"color");
    glUniform4fv(colloc, 1, colorv);


    // Rotate 
    float angle = 0;
    mat4x4 I, Q;
    mat4x4_identity(I);
    mat4x4_rotate_Z(Q, I, angle);
    
    // Translate
    Q[3][0] = 0;
    Q[3][1] = 0;
    
    GLint matloc = glGetUniformLocation(pi_texture_2DA_shader_program,"TransformMatrix");
    glUniformMatrix4fv( matloc, 1, GL_FALSE, (const GLfloat*)Q); 
    
    // Select the active texture unit.
    glActiveTexture( GL_TEXTURE0 );
#else
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT);

    glEnable(texture_format);
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    glColor4ub(255, 255, 255, 255-m_iTransparency);
#endif

    for(unsigned int ty = 0;  ty<m_numgltexturesh; ty++) {
        for(unsigned int tx = 0; tx<m_numgltexturesw; tx++) {
            unsigned int th = (ty == m_numgltexturesh-1) ? h - ty*maxtexsize : maxtexsize;
            unsigned int tw = (tx == m_numgltexturesw-1) ? w - tx*maxtexsize : maxtexsize;

            glBindTexture(texture_format, m_gltextures[ty*m_numgltexturesw + tx]);
    
            /* interpolate coordinates correctly even with rotation */
            double mtx = maxtexsize*tx, mty = maxtexsize*ty;
            wxPoint p1 = p[0], p2 = p[1], p3 = p[2];
            double q1x = p2.x-p1.x, q2x = p3.x-p1.x, q1y = p2.y-p1.y, q2y = p3.y-p1.y;
            double f1x = mtx/w, f2x = (mtx+tw)/w, f1y = mty/h, f2y = (mty+th)/h;
            double vx1x = q1x*f1x, vx1y = q1y*f1x, vx2x = q1x*f2x, vx2y = q1y*f2x;
            double vy1x = q2x*f1y, vy1y = q2y*f1y, vy2x = q2x*f2y, vy2y = q2y*f2y;
            double pu1x = p1.x+vx1x+vy1x, pu2x = p1.x+vx2x+vy1x, pu3x = p1.x+vx2x+vy2x, pu4x = p1.x+vx1x+vy2x;
            double pu1y = p1.y+vx1y+vy1y, pu2y = p1.y+vx2y+vy1y, pu3y = p1.y+vx2y+vy2y, pu4y = p1.y+vx1y+vy2y;

            if(texture_format == GL_TEXTURE_2D)
                tw = th = 1;

#ifdef USE_GLSL
            float co1[8], tco1[8];
            tco1[0] = 0,  tco1[1] = 0;     co1[0] = pu1x, co1[1] = pu1y;
            tco1[2] = tw, tco1[3] = 0;     co1[2] = pu2x, co1[3] = pu2y;
            tco1[4] = 0, tco1[5] = th;    co1[4] = pu4x, co1[5] = pu4y;
            tco1[6] = tw,  tco1[7] = th;    co1[6] = pu3x, co1[7] = pu3y;
  
            glVertexAttribPointer( mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, co1 );
            // Set the attribute mPosAttrib with the vertices in the screen coordinates...
            glEnableVertexAttribArray( mPosAttrib );
            glVertexAttribPointer( mUvAttrib, 2, GL_FLOAT, GL_FALSE, 0, tco1 );
            // Set the attribute mUvAttrib with the vertices in the GL coordinates...
            glEnableVertexAttribArray( mUvAttrib );
    
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#else
            glBegin(GL_QUADS);
            glTexCoord2i(0,   0), glVertex2d(pu1x, pu1y);
            glTexCoord2i(tw,  0), glVertex2d(pu2x, pu2y);
            glTexCoord2i(tw, th), glVertex2d(pu3x, pu3y);
            glTexCoord2i(0,  th), glVertex2d(pu4x, pu4y);
            glEnd();
#endif
        }
    }

    glDisable(texture_format);

#ifndef USE_GLSL
    glPopAttrib();
#endif

}
