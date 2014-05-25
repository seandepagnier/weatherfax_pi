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

#include "ocpn_plugin.h"

struct WeatherFaxImageCoordinates
{
WeatherFaxImageCoordinates(wxString n) : name(n),
        p1(wxPoint(0, 0)), p2(wxPoint(0, 0)), lat1(0), lon1(0), lat2(0), lon2(0),
        rotation(NONE), mapping(MERCATOR), inputpole(wxPoint(0,0)), inputequator(0), inputtrueratio(1),
        mappingmultiplier(1), mappingratio(1)
        {}

    wxString name;
    wxPoint p1, p2;
    double lat1, lon1, lat2, lon2;

    enum RotationType {NONE, CCW, CW, R180};
    RotationType rotation;

    enum MapType {MERCATOR, POLAR, CONIC, FIXED_FLAT, MAP_TYPES};
    MapType mapping;
    wxPoint inputpole;
    double inputequator, /* y value */ inputtrueratio;
    double mappingmultiplier, mappingratio;

/*
  (p1.x - x) / (lat1 - lat(x)) = (p2.x - p1.x) / (lat2 - lat1)
  lat1 - lat(x) = (p1.x - x) * (lat2 - lat1) / (p2.x - p1.x)
  lat(x) = lat1 - (p1.x - x) * (lat2 - lat1) / (p2.x - p1.x)
*/
    double lon(int x) { 
        double d = (lon2 - lon1);
        while(d <= -180) d += 360;
        while(d >=  180) d -= 360;
        return lon1 - (p1.x - x) * d / (p2.x - p1.x);
    }

    double lat(int y) {
        double s1 = sin(lat1/90 * (M_PI/2));
        double y1 = .5 * log((1 + s1) / (1 - s1));
        double s2 = sin(lat2/90 * (M_PI/2));
        double y2 = .5 * log((1 + s2) / (1 - s2));

        //      y1/(p1.y-eq) = y2/(p2.y-eq);
        // y1*p2.y - y1*eq = y2*p1.y - y2*eq;
        double eq = (y1*p2.y - y2*p1.y) / (y1 - y2);

        //        y1/(p1.y-eq) = yy/(y-eq);
        //        y1*(y-eq)/(p1.y-eq) = yy;
        double yy = y1*(y-eq)/(p1.y-eq);

        return 90*(4/M_PI*atan(exp(yy)) - 1);
    }

    static wxString MapName(enum MapType type);
    static MapType GetMapType(wxString name);
};

#include <wx/listimpl.cpp>
WX_DECLARE_LIST(WeatherFaxImageCoordinates, WeatherFaxImageCoordinateList);

class WeatherFaxImage
{
public:
        WeatherFaxImage(wxImage img, int transparency, int whitetransparency, bool invert)
            : m_origimg(img),
        phasing(0), skew(0), filter(0),
        m_Coords(NULL),
        m_CacheBitmap(NULL), m_gltextures(NULL), m_numgltexturesw(0), m_numgltexturesh(0),
        m_iTransparency(transparency), m_iWhiteTransparency(whitetransparency), m_bInvert(invert)
    {}

    /* copy constructor, make sure we don't share cached data because we aren't ref counting */
    WeatherFaxImage(WeatherFaxImage &img)
    {
        *this = img;
        m_CacheBitmap = NULL;
        m_gltextures = NULL;
    }

    ~WeatherFaxImage() { FreeData(); }

    wxImage m_origimg;

    /* page 1 */
    void MakePhasedImage();
    int phasing, skew, filter;
    wxImage m_phasedimg;

    /* page 2 */
    void InputToMercator(double px, double py, double &mx, double &my);
    void MercatorToInput(double mx, double my, double &px, double &py);
    bool MakeMappedImage(wxWindow *parent, bool paramsonly=false);

    double inputheight; /* used internally */
    wxPoint mercatoroffset;

    wxImage m_mappedimg;
    WeatherFaxImageCoordinates *m_Coords;

    /* rendering */
    void FreeData();
    bool GetOverlayCoords(PlugIn_ViewPort *vp, wxPoint p[3], int &w, int &h);
    void RenderImage(wxDC &dc, PlugIn_ViewPort *vp);
    void RenderImageGL(PlugIn_ViewPort *vp);

    wxBitmap *m_CacheBitmap;
    unsigned int *m_gltextures, m_numgltexturesw, m_numgltexturesh;

    int               m_iTransparency;
    int               m_iWhiteTransparency;
    bool              m_bInvert;
};
