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

#include "../../../include/ocpn_plugin.h"

struct WeatherFaxImageCoordinates
{
WeatherFaxImageCoordinates(wxString n) : name(n),
        p1(wxPoint(0, 0)), p2(wxPoint(0, 0)), lat1(0), lon1(0), lat2(0), lon2(0),
        mapping(MERCATOR), inputpole(wxPoint(0,0)), inputequator(0), inputtrueratio(1),
        mappingmultiplier(1), mappingratio(1)
        {}

    wxString name;
    wxPoint p1, p2;
    double lat1, lon1, lat2, lon2;

    enum MapType {MERCATOR, POLAR, CONIC, FIXED_FLAT};
    MapType mapping;
    wxPoint inputpole;
    double inputequator, /* y value */ inputtrueratio;
    double mappingmultiplier, mappingratio;

/*
  (p1.x - x) / (lat1 - lat(x)) = (p2.x - p1.x) / (lat2 - lat1)
  lat1 - lat(x) = (p1.x - x) * (lat2 - lat1) / (p2.x - p1.x)
  lat(x) = lat1 - (p1.x - x) * (lat2 - lat1) / (p2.x - p1.x)
*/
    double lon(int x) { return lon1 - (p1.x - x) * (lon2 - lon1) / (p2.x - p1.x); }
};

#include <wx/listimpl.cpp>
WX_DECLARE_LIST(WeatherFaxImageCoordinates, WeatherFaxImageCoordinateList);

class WeatherFaxImage
{
public:
        WeatherFaxImage(wxImage img, int transparency, int whitetransparency, bool invert)
            : m_origimg(img),
        phasing(0), skew(0), filter(0), rotation(0),
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
    int phasing, skew, filter, rotation;
    wxImage m_phasedimg;

    /* page 2 */
    void InputToMercator(double px, double py, double &mx, double &my);
    void MercatorToInput(double mx, double my, double &px, double &py);
    bool MakeMappedImage(wxWindow *parent, bool paramsonly);

    double inputheight; /* used internally */
    wxPoint mercatoroffset;

    wxImage m_mappedimg;
    WeatherFaxImageCoordinates *m_Coords;

    /* rendering */
    void FreeData();
    bool GetOverlayCoords(PlugIn_ViewPort *vp, wxPoint &p0, wxPoint &pwh, int &w, int &h);
    void RenderImage(wxDC &dc, PlugIn_ViewPort *vp);
    void RenderImageGL(PlugIn_ViewPort *vp);

    wxBitmap *m_CacheBitmap;
    unsigned int *m_gltextures, m_numgltexturesw, m_numgltexturesh;

    int               m_iTransparency;
    int               m_iWhiteTransparency;
    bool              m_bInvert;
};
