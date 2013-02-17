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

struct WeatherFaxImageCoordinates
{
    WeatherFaxImageCoordinates(wxString n) : name(n) {}
    wxString name;
    wxPoint p1, p2;
    double lat1, lon1, lat2, lon2;
};

WX_DECLARE_LIST(WeatherFaxImageCoordinates, WeatherFaxImageCoordinateList);

class WeatherFaxImage : public wxImage
{
public:
    WeatherFaxImage(wxImage img) : wxImage(img), m_Coords(NULL), phasing(0) {  }
    WeatherFaxImageCoordinates *m_Coords;

    wxImage PhasedImage() {
        return wxImage(GetWidth(), GetHeight() - 1,
                       GetData() + phasing*3, true);
    }

    int phasing;
};

class WeatherFaxDialog: public WeatherFaxDialogBase
{
public:
    WeatherFaxDialog( weatherfax_pi &_weatherfax_pi, wxWindow* parent);

    ~WeatherFaxDialog();

    void OnListBox( wxCommandEvent& event );
    void NewFaxClicked( wxCommandEvent& event );
    void EditFaxClicked( wxCommandEvent& event );
    void DeleteFaxClicked( wxCommandEvent& event );
    void TransparencyChanged( wxScrollEvent& event );
    void WhiteTransparencyChanged( wxScrollEvent& event );
    void OnInvert( wxCommandEvent& event );

    WeatherFaxImageCoordinateList m_Coords;

protected:
    weatherfax_pi &m_weatherfax_pi;
};

enum EditState {COORD, SPLITIMAGE};
class EditFaxDialog : public EditFaxDialogBase
{
public:
    EditFaxDialog( WeatherFaxImage &img, wxString name, WeatherFaxDialog &parent,
                   WeatherFaxImageCoordinateList &coords);
    ~EditFaxDialog();

    void OnSize( wxSizeEvent& event ) { Fit(); }
    void OnBitmapClick( wxMouseEvent& event );
    void OnCoordSet( wxCommandEvent& event );
    void OnCoordText( wxCommandEvent& event );
    void OnRemoveCoords( wxCommandEvent& event );
    void OnSplitImage( wxCommandEvent& event );
    void OnPhasing( wxScrollEvent& event );
    void StoreCoords();

protected:

    wxString NewCoordName();
    void SetCoords();
    void OnSpin( wxSpinEvent& event );
    void UpdateCoords();
    void OnPaintImage( wxPaintEvent& event);

    WeatherFaxDialog &m_parent;
    WeatherFaxImage &m_img;
    WeatherFaxImageCoordinates *&m_curCoords;

    wxString m_name;
    int m_splits;

    EditState m_EditState;
    
    WeatherFaxImageCoordinates *m_newCoords;
    WeatherFaxImageCoordinateList &m_Coords;

    bool m_skippaint;
    int m_SelectedIndex;
};
