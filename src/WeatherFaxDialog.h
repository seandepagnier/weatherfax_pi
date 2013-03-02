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
#include "WeatherFaxImage.h"

class WeatherFaxDialog: public WeatherFaxDialogBase
{
public:
    WeatherFaxDialog( weatherfax_pi &_weatherfax_pi, wxWindow* parent);

    ~WeatherFaxDialog();

    void OnFaxes( wxCommandEvent& event );
    void OnFaxesToggled( wxCommandEvent& event );
    void CaptureFaxClicked( wxCommandEvent& event );
    void OpenFaxClicked( wxCommandEvent& event );
    void EditFaxClicked( wxCommandEvent& event );
    void DeleteFaxClicked( wxCommandEvent& event );
    void TransparencyChanged( wxScrollEvent& event );
    void WhiteTransparencyChanged( wxScrollEvent& event );
    void OnInvert( wxCommandEvent& event );

    WeatherFaxImageCoordinateList m_Coords;

protected:
    weatherfax_pi &m_weatherfax_pi;
};

class EditFaxWizard : public EditFaxWizardBase
{
public:
    EditFaxWizard( WeatherFaxImage &img, wxString name, WeatherFaxDialog &parent,
                   WeatherFaxImageCoordinateList &coords);
    ~EditFaxWizard();

    void OnSetSizes( wxInitDialogEvent& event );
//    void OnSize( wxSizeEvent& event ) { Fit(); }
    void UpdateMappingControls();
    void OnWizardPageChanged( wxWizardEvent& event );
    void OnMappingChoice( wxCommandEvent& event );
    void GetMappingParametersFixedFlat();
    void GetMappingParametersPolar();
    void OnGetMappingParameters( wxCommandEvent& event );
    void OnBitmapClickPage2( wxMouseEvent& event );
    void OnBitmapClickPage3( wxMouseEvent& event );
    void OnCoordSet( wxCommandEvent& event );
    void OnCoordText( wxCommandEvent& event );
    void OnRemoveCoords( wxCommandEvent& event );
    void StoreMappingParams();
    void ResetMapping();
    bool ApplyMapping();
    
    void SetCoordRanges();
    void OnApplyMapping( wxCommandEvent& event );
    void OnUpdateMapping( wxSpinEvent& event ) { Refresh(); }
    void OnUpdateMapping( wxCommandEvent& event ) { Refresh(); }
    void OnInformation( wxCommandEvent& event );

    void StoreCoords();

protected:

    void UpdatePage1();
    void UpdatePage1( wxCommandEvent& event );
    void UpdatePage1( wxScrollEvent& event );

    wxString NewCoordName();
    void SetCoords();
    void OnSpin( wxSpinEvent& event );
    void OnPaintImage( wxPaintEvent& event);

    WeatherFaxDialog &m_parent;
    WeatherFaxImage &m_wfimg;
    WeatherFaxImageCoordinates *&m_curCoords;

    wxString m_name;

    WeatherFaxImageCoordinates *m_newCoords;
    WeatherFaxImageCoordinateList &m_Coords;

    bool m_skippaint;

    /* page 2 */
    int m_SelectedIndex;
    bool m_bmappingapplied;
};
