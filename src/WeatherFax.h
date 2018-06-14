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

#include <vector>

#include "WeatherFaxUI.h"

extern const char * box_xpm[];
extern const char * check_xpm[];

class FaxArea
{
public:
    wxString name, description;
    double lat1, lat2, lon1, lon2;

    FaxArea() : lat1(NAN), lat2(NAN), lon1(NAN), lon2(NAN) {}

    bool ContainsLat(double lat) { return wxIsNaN(lat) || (lat >= lat1 && lat <= lat2); }
    bool ContainsLon(double lon) { return wxIsNaN(lon) ||
            (lon2 - lon1 < 180 && lon >= lon1 && lon <= lon2) ||
            (lon2 - lon1 >= 180 && (lon <= lon1 || lon >= lon2)); }

    wxString LatArea(double lat) { return wxString::Format(_T("%.0f"), fabs(lat))
            + ((lat >= 0) ? _T("N") : _T("S")); }
    wxString LonArea(double lon) { return wxString::Format(_T("%.0f"), fabs(lon))
            + ((lon >= 0) ? _T("E") : _T("W")); }
    wxString AreaDescription() {
        return description +
            ((!wxIsNaN(lat1) && !wxIsNaN(lat2)) ?
             _T(" ") + LatArea(lat1) + _T("-") + LatArea(lat2) : _T("")) +
            ((!wxIsNaN(lon1) && !wxIsNaN(lon2)) ?
             _T(" ") + LonArea(lon1) + _T("-") + LonArea(lon2) : _T(""));
    }
};

#include "SchedulesDialog.h"
#include "InternetRetrievalDialog.h"

class weatherfax_pi;
class WeatherFaxWizard;

class WeatherFax : public WeatherFaxBase
{
public:
    WeatherFax( weatherfax_pi &_weatherfax_pi, wxWindow* parent);
    ~WeatherFax();

    void OnClose( wxCloseEvent& event ) { Hide(); }
    void EnableDisplayControls(bool enable);
    void OnFaxes( wxCommandEvent& event );
    void TransparencyChanged( wxScrollEvent& event );
    void WhiteTransparencyChanged( wxScrollEvent& event );
    void OnInvert( wxCommandEvent& event );
    void OnOpen( wxCommandEvent& event );
    void OnSaveAs( wxCommandEvent& event );
    void OnEdit( wxCommandEvent& event );
    void OnGoto( wxCommandEvent& event );
    void OnDelete( wxCommandEvent& event );
    void OnExport( wxCommandEvent& event );
    void OnPreferences( wxCommandEvent& event ) { m_weatherfax_pi.ShowPreferencesDialog( this ); }
    void OnClose( wxCommandEvent& event ) { Hide(); }
    void OnCapture( wxCommandEvent& event );
    void OnSchedules( wxCommandEvent& event );
    void OnInternet( wxCommandEvent& event );
    void OnUpdateData( wxCommandEvent& event );
    void OnAbout( wxCommandEvent& event );

    bool Show( bool show = true );
    void WizardFinished(WeatherFaxWizard *wizard);
    bool WizardCleanup(WeatherFaxWizard *wizard);
    WeatherFaxWizard *OpenWav(wxString filename, long offset=0, wxString station=_T(""), wxString area=_T(""), wxString contents=_T(""));
    void OpenImage(wxString filename, wxString station=_T(""), wxString area=_T(""), wxString contents=_T(""));
    void Goto(int selection);

    void UpdateMenuStates();
    void StopDecoder(WeatherFaxWizard *wizard);

    WeatherFaxImageCoordinateList m_BuiltinCoords, m_UserCoords;

    std::vector<WeatherFaxImage*>m_Faxes;
    SchedulesDialog m_SchedulesDialog;
    InternetRetrievalDialog m_InternetRetrievalDialog;

    weatherfax_pi &m_weatherfax_pi;

protected:
    void OnDeleteWizardTimer( wxTimerEvent & );

    std::list<WeatherFaxWizard *> m_AudioWizards;
    wxTimer m_tDeleteAudioWizard;

private:
    bool DownloadFile( wxString filename );
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
