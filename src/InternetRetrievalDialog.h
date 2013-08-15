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

#include <list>
#include <vector>

#include "wx/process.h"

#include "WeatherFaxUI.h"

class weatherfax_pi;

#if 0
class ScheduleArea
{
public:
    wxString name, description;
    double lat1, lat2, lon1, lon2;

    ScheduleArea() : lat1(NAN), lat2(NAN), lon1(NAN), lon2(NAN) {}

    bool ContainsLat(double lat) { return isnan(lat) || (lat >= lat1 && lat <= lat2); }
    bool ContainsLon(double lon) { return isnan(lon) ||
            (lon2 - lon1 < 180 && lon >= lon1 && lon <= lon2) ||
            (lon2 - lon1 >= 180 && (lon <= lon1 || lon >= lon2)); }

    wxString LatArea(double lat) { return wxString::Format(_T("%.0f"), fabs(lat))
            + ((lat >= 0) ? _T("N") : _T("S")); }
    wxString LonArea(double lon) { return wxString::Format(_T("%.0f"), fabs(lon))
            + ((lon >= 0) ? _T("E") : _T("W")); }
    wxString AreaDescription() {
        return description +
            ((!isnan(lat1) && !isnan(lat2)) ?
             _T(" ") + LatArea(lat1) + _T("-") + LatArea(lat2) : _T("")) +
            ((!isnan(lon1) && !isnan(lon2)) ?
             _T(" ") + LonArea(lon1) + _T("-") + LonArea(lon2) : _T(""));
    }
};
#endif

class FaxUrl
{
public:
    bool Filtered;
    bool Selected;

    wxString Station;
    wxString Contents;
    wxString area_name;

//    ScheduleArea Area;

};

class InternetRetrievalDialog: public InternetRetrievalDialogBase
{
public:
    enum {SELECTED, STATION, CONTENTS, MAP_AREA};

    InternetRetrievalDialog( weatherfax_pi &_weatherfax_pi, wxWindow* parent);
    ~InternetRetrievalDialog();

    void Load();
    void ClearInternetRetrieval();
    bool OpenXML(wxString filename);

    void OnInternetRetrievalLeftDown( wxMouseEvent& event );
    void OnInternetRetrievalSort( wxListEvent& event );
    void OnFilter( wxCommandEvent& event ) { Filter(); }
    void OnBoatPosition( wxCommandEvent& event );
    void OnReset( wxCommandEvent& event );
    void OnAllStations( wxCommandEvent& event );
    void OnNoStations( wxCommandEvent& event );
    void OnRetrieve( wxCommandEvent& event );
    void OnClose( wxCommandEvent& event );

    bool HasStation(wxString station);
    void Filter();
    void RebuildList();
    void UpdateItem(long index);

private:

    void StopExternalProcess();

    weatherfax_pi &m_weatherfax_pi;

    std::list<FaxUrl*> m_InternetRetrieval;
    std::list<FaxUrl*> m_CaptureInternetRetrieval;

    bool m_bLoaded, m_bDisableFilter;
    bool m_bKilled;

    bool m_bRebuilding;
};
