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

#include "weatherfax_pi.h"
#include "WeatherFaxUI.h"

class WeatherFaxImage : public wxImage
{
public:
    WeatherFaxImage(wxImage img) : wxImage(img), valid_location(false)
    {
#if 0 /* hack for quick testing */
        p1.x = 0;   p1.y = 0;
        p2.x = 256; p2.y = 256;
        lat1 = 0;   lon1 = 150;
        lat2 = -30; lon2 = 180;
        valid_location = true;
#endif
    }

    bool valid_location;
    wxPoint p1, p2;
    double lat1, lon1, lat2, lon2;
};

class WeatherFaxDialog: public WeatherFaxDialogBase
{
public:
    WeatherFaxDialog( weatherfax_pi &_weatherfax_pi, wxWindow* parent)
        : WeatherFaxDialogBase( parent ), m_weatherfax_pi(_weatherfax_pi) {}
    ~WeatherFaxDialog();

    void OnListBox( wxCommandEvent& event );
    void NewFaxClicked( wxCommandEvent& event );
    void EditFaxClicked( wxCommandEvent& event );
    void DeleteFaxClicked( wxCommandEvent& event );
    void TransparencyChanged( wxScrollEvent& event );
    void OnInvert( wxCommandEvent& event );

protected:
    weatherfax_pi &m_weatherfax_pi;
};

enum EditState {NONE, COORD1, COORD2, SPLIT};
class EditFaxDialog: public EditFaxDialogBase
{
public:
    EditFaxDialog( WeatherFaxImage &img, wxString name, WeatherFaxDialog &parent)
        : EditFaxDialogBase( &parent ), m_parent(parent), m_name(name), m_splits(0),
        m_EditState(NONE), m_img(img) { UpdateBitmap(); }

    void OnSize( wxSizeEvent& event ) { Fit(); }
    void BitmapClick( wxMouseEvent& event );
    void SetCoordinates( wxCommandEvent& event );
    void Crop( wxCommandEvent& event );
    void Split( wxCommandEvent& event );
    void Invert( wxCommandEvent& event );

protected:

    WeatherFaxDialog &m_parent;

    wxString m_name;
    int m_splits;

    EditState m_EditState;
    
    void UpdateBitmap();

    WeatherFaxImage &m_img;
};

