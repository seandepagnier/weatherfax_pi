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

struct WeatherFaxImageCoordinates
{
    WeatherFaxImageCoordinates(wxString n) : name(n) {}
    wxString name;
    wxPoint p1, p2;
    double lat1, lon1, lat2, lon2;

/*
  (p1.x - x) / (lat1 - lat(x)) = (p2.x - p1.x) / (lat2 - lat1)
  lat1 - lat(x) = (p1.x - x) * (lat2 - lat1) / (p2.x - p1.x)
  lat(x) = lat1 - (p1.x - x) * (lat2 - lat1) / (p2.x - p1.x)
*/
    double lon(int x) { return lon1 - (p1.x - x) * (lon2 - lon1) / (p2.x - p1.x); }
};

WX_DECLARE_LIST(WeatherFaxImageCoordinates, WeatherFaxImageCoordinateList);

class WeatherFaxImage
{
public:
    WeatherFaxImage(wxImage img) : m_origimg(img),
        phasing(0), skew(0), filter(0), rotation(0),
        mapping(MERCATOR), inputpole(wxPoint(0,0)), inputequator(0),
        mappingwidthmultiplier(1), mappingheightmultiplier(1),
        m_Coords(NULL) {  }

    wxImage m_origimg;

    /* page 1 */
    void MakePhasedImage();
    int phasing, skew, filter, rotation;
    wxImage m_phasedimg;

    /* page 2 */
    void InputToMercator(double px, double py, double &mx, double &my);
    void MercatorToInput(double mx, double my, double &px, double &py);
    void MakeMappedImage(wxWindow *parent);
    enum MapType {MERCATOR, POLAR, FIXED_FLAT};
    MapType mapping;
    wxPoint inputpole;
    double inputequator; /* y value */
    double mappingwidthmultiplier, mappingheightmultiplier;

    double inputheight; /* used internally */
    wxPoint mercatoroffset;

    wxImage m_mappedimg;

    /* page 3 */
    WeatherFaxImageCoordinates *m_Coords;
};
