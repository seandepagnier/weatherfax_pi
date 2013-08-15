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

#include <vector>

#include "WeatherFaxUI.h"
#include "SchedulesDialog.h"
#include "InternetRetrievalDialog.h"

class weatherfax_pi;

class WeatherFax: public WeatherFaxBase
{
public:
    WeatherFax( weatherfax_pi &_weatherfax_pi, wxWindow* parent);

    ~WeatherFax();

    void EnableDisplayControls(bool enable);
    void OnFaxes( wxCommandEvent& event );
    void OnFaxesToggled( wxCommandEvent& event );
    void TransparencyChanged( wxScrollEvent& event );
    void WhiteTransparencyChanged( wxScrollEvent& event );
    void OnInvert( wxCommandEvent& event );
    void OnOpen( wxCommandEvent& event );
    void OnEdit( wxCommandEvent& event );
    void OnDelete( wxCommandEvent& event );
    void OnClose( wxCommandEvent& event ) { Hide(); }
    void OnCapture( wxCommandEvent& event );
    void OnSchedules( wxCommandEvent& event );
    void OnInternet( wxCommandEvent& event );
    void OnAbout( wxCommandEvent& event );

    void OpenWav(wxString filename, wxString station=_T(""), wxString area=_T(""));
    void OpenImage(wxString filename);

    void UpdateMenuStates();

    WeatherFaxImageCoordinateList m_Coords;

    std::vector<WeatherFaxImage*>m_Faxes;

protected:
    SchedulesDialog m_SchedulesDialog;
    InternetRetrievalDialog m_InternetRetrievalDialog;

    weatherfax_pi &m_weatherfax_pi;
};

class FaxDecoder;
class DecoderThread : public wxThread
{
public:
    DecoderThread(FaxDecoder &decoder)
        : wxThread(wxTHREAD_JOINABLE), m_decoder(decoder) { Create(); }
    void *Entry();
private:
    FaxDecoder &m_decoder;
};
