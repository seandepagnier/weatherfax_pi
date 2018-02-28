/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2018 by Sean D'Epagnier                                 *
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

struct FaxUrl
{
    bool Filtered;
    bool Scheduled;

    wxString Server;
    wxString Region;
    wxString Url;
    wxString Contents;
    wxString area_name;

    FaxArea Area;

    int hour_offset, hour_round, hour_round_offset;
};

struct FaxRegion
{
    bool Filtered;
    bool Selected;

    wxString Name;
    wxString Server;
};

struct FaxServer
{
    bool Filtered;
    bool Selected;

    wxString Name;
};

class InternetRetrievalDialog: public InternetRetrievalDialogBase
{
public:
    enum {SCHEDULED, SERVER, REGION, CONTENTS, MAP_AREA};

    InternetRetrievalDialog( weatherfax_pi &_weatherfax_pi, wxWindow* parent);
    ~InternetRetrievalDialog();

    void Load(bool force = false);
    void ClearInternetRetrieval();
    bool OpenXML(wxString filename);

    void OnUrlsLeftDown( wxMouseEvent& event );
    void OnUrlsSort( wxListEvent& event );
    void OnUrlSelected( wxListEvent& event );
    void OnFilter( wxCommandEvent& event ) { Filter(); }
    void OnFilterServers( wxCommandEvent& event )
    { m_bDisableServers = true; Filter(); m_bDisableServers = false; }
    void OnFilterRegions( wxCommandEvent& event )
    { m_bDisableRegions = true; Filter(); m_bDisableRegions = false; }
    void OnBoatPosition( wxCommandEvent& event );
    void OnReset( wxCommandEvent& event );
    void OnAllServers( wxCommandEvent& event );
    void OnNoServers( wxCommandEvent& event );
    void OnAllRegions( wxCommandEvent& event );
    void OnNoRegions( wxCommandEvent& event );
    void OnRetrieve( wxMouseEvent& event ) { wxCommandEvent evt; OnRetrieve(evt); }
    void OnRetrieve( wxCommandEvent& event );
    void OnClose( wxCommandEvent& event );

    bool HasServer(wxString server);
    bool HasRegion(wxString region);
    void Filter();
    void RebuildServers();
    void RebuildRegions();
    void RebuildList();
    void UpdateItem(long index);

private:

    void StopExternalProcess();

    weatherfax_pi &m_weatherfax_pi;

    std::list<FaxServer> m_Servers;
    std::list<FaxRegion> m_Regions;
    std::list<FaxUrl*> m_InternetRetrieval;

    bool m_bLoaded;
    bool m_bDisableServers, m_bDisableRegions, m_bDisableFilter;
    bool m_bKilled;

    bool m_bRebuilding;
};
