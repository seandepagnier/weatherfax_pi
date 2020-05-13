/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2017 by Sean D'Epagnier                                 *
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

#include <wx/wx.h>
#include <wx/imaglist.h>
#include <wx/progdlg.h>

#include <list>

#include "tinyxml/tinyxml.h"

#include "weatherfax_pi.h"
#include "WeatherFaxImage.h"
#include "WeatherFax.h"

SchedulesDialog::SchedulesDialog( weatherfax_pi &_weatherfax_pi, wxWindow* parent)
#ifndef __WXOSX__
    : SchedulesDialogBase( parent ),
#else
    : SchedulesDialogBase( parent, wxID_ANY, _("HF Radio Schedules"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER|wxSTAY_ON_TOP ),
#endif
      m_CaptureWizard(NULL), m_weatherfax_pi(_weatherfax_pi),
      m_ExternalCaptureProcess(NULL), m_CurrentSchedule(NULL), m_bLoaded(false),
      m_bDisableFilter(true), m_bKilled(false), m_bRebuilding(false)
{
#ifdef OCPN_USE_PORTAUDIO
    m_rbAudioCapture->Enable();
#endif
#if defined(API_VERSION_MAJOR) == 1 && defined(API_VERSION_MINOR) < 10
    m_cbSound->Disable();
    m_fpSound->Disable();
#endif
#ifdef __OCPN__ANDROID__
    wxSize s = ::wxGetDisplaySize();
    SetSize(0, 0, s.x, s.y-40);
#endif
}

SchedulesDialog::~SchedulesDialog()
{
    wxFileConfig *pConf = GetOCPNConfigObject();

    /* remove from config all cordinate sets */
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Schedules" ) );

    pConf->Write ( _T ( "ContainsLat" ), m_tContainsLat->GetValue() );
    pConf->Write ( _T ( "ContainsLon" ), m_tContainsLon->GetValue() );

    wxString stations;
    for(unsigned int i=0; i < m_lStations->GetCount(); i++)
        if(m_lStations->IsSelected(i))
            stations += m_lStations->GetString(i) + _T(";");
    pConf->Write ( _T ( "Stations" ), stations);

    pConf->Write ( _T ( "khzmin" ), m_skhzmin->GetValue() );
    pConf->Write ( _T ( "khzmax" ), m_skhzmax->GetValue() );

    pConf->Write ( _T ( "HasArea" ), m_cbHasArea->GetValue() );
    pConf->Write ( _T ( "HasValidTime" ), m_cbHasValidTime->GetValue() );

    pConf->Write ( _T ( "messagebox" ), m_cbMessageBox->GetValue() );
    pConf->Write ( _T ( "sound" ), m_cbSound->GetValue() );
    pConf->Write ( _T ( "soundfile" ), m_fpSound->GetPath() );
    pConf->Write ( _T ( "externalalarm" ), m_cbExternalAlarm->GetValue() );
    pConf->Write ( _T ( "externalalarmcommand" ), m_tExternalAlarmCommand->GetValue() );
    pConf->Write ( _T ( "skipifprevfax" ), m_cbSkipIfPrevFax->GetValue() );

    pConf->Write ( _T ( "noaction" ), m_rbNoAction->GetValue() );
    pConf->Write ( _T ( "audiocapture" ), m_rbAudioCapture->GetValue() );
    pConf->Write ( _T ( "externalcapture" ), m_rbExternalCapture->GetValue() );
    pConf->Write ( _T ( "externalcapturecommand" ), m_cExternalCapture->GetValue() );
    pConf->Write ( _T ( "externalconversioncommand" ), m_tExternalConversion->GetValue() );
    pConf->Write ( _T ( "manualcapture" ), m_rbExternalCapture->GetValue() );

    wxString captures;
    for(std::list<Schedule*>::iterator it = m_CaptureSchedules.begin();
        it != m_CaptureSchedules.end(); it++)
        captures += (*it)->Station + wxString::Format(_T(",%04d;"), (*it)->Time);
    pConf->Write ( _T ( "captures" ), captures );

    ClearSchedules();

    if(m_ExternalCaptureProcess) {
        m_ExternalCaptureProcess->Disconnect(wxEVT_END_PROCESS, wxProcessEventHandler
                              ( SchedulesDialog::OnTerminate ), NULL, this);

        StopExternalProcess();
    }
}

void SchedulesDialog::Load(bool force)
{
    if(m_bLoaded && !force)
        return;

    m_bLoaded = true;

    // create a image list for the list with check
    wxImageList *imglist = new wxImageList(20, 20);
    imglist->Add(wxBitmap(box_xpm));
    imglist->Add(wxBitmap(check_xpm));
    m_lSchedules->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

    m_lSchedules->InsertColumn(CAPTURE, _("Capture"));
    m_lSchedules->InsertColumn(STATION, _("Station"));
    m_lSchedules->InsertColumn(FREQUENCIES, _("Frequencies"));
    m_lSchedules->InsertColumn(TIME, _("Time (UTC)"));
    m_lSchedules->InsertColumn(CONTENTS, _("Contents"));
    m_lSchedules->InsertColumn(VALID_TIME, _("Valid Time"));
    m_lSchedules->InsertColumn(DURATION, _("Duration"));
    m_lSchedules->InsertColumn(MAP_AREA, _("Map Area"));

    m_AlarmTimer.Connect(wxEVT_TIMER, wxTimerEventHandler
                    ( SchedulesDialog::OnAlarmTimer ), NULL, this);
    m_CaptureTimer.Connect(wxEVT_TIMER, wxTimerEventHandler
                    ( SchedulesDialog::OnCaptureTimer ), NULL, this);
    m_EndCaptureTimer.Connect(wxEVT_TIMER, wxTimerEventHandler
                    ( SchedulesDialog::OnEndCaptureTimer ), NULL, this);
    m_ProgressTimer.Connect(wxEVT_TIMER, wxTimerEventHandler
                    ( SchedulesDialog::OnProgressTimer ), NULL, this);
    m_ProgressTimer.Start(30 * 1000, false);

    wxFileConfig *pConf = GetOCPNConfigObject();

    /* remove from config all cordinate sets */
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Schedules" ) );

    wxString s;
    pConf->Read ( _T ( "ContainsLat" ), &s, _T("") );
    m_tContainsLat->SetValue(s);
    pConf->Read ( _T ( "ContainsLon" ), &s, _T("") );
    m_tContainsLon->SetValue(s);

    wxString stations;
    pConf->Read ( _T ( "Stations" ), &stations, _T(""));

    std::list<wxString> stationlist;
    /* split at each ; to get all the names in a list */
    while(stations.size()) {
        stationlist.push_back(stations.BeforeFirst(';'));
        stations = stations.AfterFirst(';');
    }

    wxString scheduled;
    pConf->Read ( _T ( "Scheduled" ), &scheduled, _T("") );
    std::list<wxString> scheduledlist;
    /* split at each ; to get all the names in a list */
    while(scheduled.size()) {
        scheduledlist.push_back(scheduled.BeforeFirst(';'));
        scheduled = scheduled.AfterFirst(';');
    }

    int i;
    pConf->Read ( _T ( "khzmin" ), &i, 0 );
    m_skhzmin->SetValue(i);
    pConf->Read ( _T ( "khzmax" ), &i, 30000 );
    m_skhzmax->SetValue(i);

    bool b;
    pConf->Read ( _T ( "HasArea" ), &b, false );
    m_cbHasArea->SetValue(b);
    pConf->Read ( _T ( "HasValidTime" ), &b, false );
    m_cbHasValidTime->SetValue(b);

    pConf->Read ( _T ( "messagebox" ), &b, false );
    m_cbMessageBox->SetValue(b);
    pConf->Read ( _T ( "sound" ), &b, false );
    m_cbSound->SetValue(b);
    pConf->Read ( _T ( "soundfile" ), &s, _T("") );
    m_fpSound->SetPath(s);
    pConf->Read ( _T ( "externalalarm" ), &b, false );
    m_cbExternalAlarm->SetValue(b);
    pConf->Read ( _T ( "externalalarmcommand" ), &s, m_tExternalAlarmCommand->GetValue() );
    m_tExternalAlarmCommand->SetValue(s);
    pConf->Read ( _T ( "skipifprevfax" ), &b, true );
    m_cbSkipIfPrevFax->SetValue(b);

    pConf->Read ( _T ( "noaction" ), &b, true );
    m_rbNoAction->SetValue(b);
    pConf->Read ( _T ( "audiocapture" ), &b, false );
    m_rbAudioCapture->SetValue(b);
    pConf->Read ( _T ( "externalcapture" ), &b, false );
    m_rbExternalCapture->SetValue(b);
    pConf->Read ( _T ( "externalcapturecommand" ), &s, m_cExternalCapture->GetValue() );
    m_cExternalCapture->SetValue(s);
    pConf->Read ( _T ( "externalconversioncommand" ), &s, m_tExternalConversion->GetValue() );
    m_tExternalConversion->SetValue(s);
    pConf->Read ( _T ( "manualcapture" ), &b, false );
    m_rbExternalCapture->SetValue(b);

    wxString captures;
    pConf->Read ( _T ( "captures" ), &captures, _T("") );
    std::list<wxString> capturelist;
    /* split at each ; to get all the names in a list */
    while(captures.size()) {
        capturelist.push_back(captures.BeforeFirst(';'));
        captures = captures.AfterFirst(';');
    }

    s = wxFileName::GetPathSeparator();
    if( wxFileExists( m_weatherfax_pi.StandardPath() + _T("WeatherFaxSchedules.xml") ) )
        OpenXML( m_weatherfax_pi.StandardPath() + _T("WeatherFaxSchedules.xml") );
    else
        OpenXML(GetPluginDataDir("weatherfax_pi") + s + _T("data") + s
            + _T("WeatherFaxInternetRetrieval.xml"));

    m_lStations->DeselectAll();
    for(unsigned int i=0; i < m_lStations->GetCount(); i++)
        for(std::list<wxString>::iterator it = stationlist.begin();
            it != stationlist.end(); it++)
            if(m_lStations->GetString(i) == *it)
                m_lStations->SetSelection(i);

    for(std::list<wxString>::iterator it = capturelist.begin();
        it != capturelist.end(); it++) {
        wxString station = (*it).BeforeLast(',');
        long time;
        (*it).AfterLast(',').ToLong(&time);

        for(std::list<Schedule*>::iterator it2 = m_Schedules.begin();
            it2 != m_Schedules.end(); it2++)
            if(station == (*it2)->Station && time == (*it2)->Time) {
                AddScheduleToCapture(*it2);
                break;
            }
    }
    UpdateProgress();

    m_bDisableFilter = false;
    Filter();
}

#define FAIL(X) do { error = X; goto failed; } while(0)

void SchedulesDialog::ClearSchedules()
{
    for(std::list<Schedule*>::iterator it = m_Schedules.begin();
        it != m_Schedules.end(); it++)
        delete *it;
    m_Schedules.clear();
}

static double ParseLatLon(wxString s)
{
    if(s.empty())
        return NAN;

    wxChar ns = s.Last();
    int sign = (ns == 'S' || ns == 'W') ? -1 : 1;
    double d;
    s.ToDouble(&d);
    return sign * d;
}

bool SchedulesDialog::OpenXML(wxString filename)
{
    ClearSchedules();
    m_lStations->Clear();

    TiXmlDocument doc;
    wxString error;
    wxProgressDialog *progressdialog = NULL;
    wxDateTime start = wxDateTime::UNow();

    if(!doc.LoadFile(filename.mb_str()))
        FAIL(_("Failed to load file: ") + filename);
    else {
        TiXmlElement *root = doc.RootElement();
        if(strcmp(root->Value(), "OCPNWeatherFaxSchedules"))
            FAIL(_("Invalid xml file"));

        int count = 0;
        for(TiXmlElement* e = root->FirstChildElement(); e; e = e->NextSiblingElement())
            count++;

        int i=0;
        for(TiXmlElement* e = root->FirstChildElement(); e; e = e->NextSiblingElement(), i++) {
            if(progressdialog) {
                if(!progressdialog->Update(i))
                    return true;
            } else {
                wxDateTime now = wxDateTime::UNow();
                if((now-start).GetMilliseconds() > 500 && i < count/3) {
                    progressdialog = new wxProgressDialog(
                        _("WeatherFax Schedules"), _("Loading"), count, this,
                        wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
                }
            }

            if(!strcmp(e->Value(), "Station")) {
                wxString station = wxString::FromUTF8(e->Attribute("Name"));

                m_lStations->Append(station);
                std::list<Schedule> schedules;
                std::list<FaxArea> Areas;

                for(TiXmlElement* f = e->FirstChildElement(); f; f = f->NextSiblingElement()) {
                    if(!strcmp(f->Value(), "Frequency")) {
                        wxString frequencies_str = wxString::FromUTF8(f->Attribute("khz"));
                        std::vector<double>frequencies;
                        while(frequencies_str.size()) {
                            double frequency;
                            frequencies_str.BeforeFirst(',').ToDouble(&frequency);
                            frequencies.push_back(frequency);
                            frequencies_str = frequencies_str.AfterFirst(',');
                        }

                        for(TiXmlElement* g = f->FirstChildElement(); g; g = g->NextSiblingElement()) {
                            if(!strcmp(g->Value(), "Map")) {
                                Schedule schedule;
                                    
                                schedule.Capture = false;
                                schedule.Station = station;
                                schedule.Frequencies = frequencies;
                                    
                                g->Attribute("Time", &schedule.Time);
                                
                                if(schedules.size()) {
                                    Schedule lastschedule = schedules.back();
                                    if(lastschedule.Duration == -1) {
                                        schedules.pop_back();
                                        lastschedule.Duration = schedule.Time%100 - lastschedule.Time%100;
                                        if(lastschedule.Duration < 0)
                                            lastschedule.Duration += 60;
                                        schedules.push_back(lastschedule);
                                    }
                                }                                    
                                    
                                schedule.Contents = wxString::FromUTF8(g->Attribute("Contents"));

                                schedule.ValidTime = -1;
                                g->Attribute("Valid", &schedule.ValidTime);

                                schedule.area_name = wxString::FromUTF8(g->Attribute("Area"));
                                    
                                schedule.Duration = -1;
                                g->Attribute("Duration", &schedule.Duration);
                                    
                                schedules.push_back(schedule);
                            } else
                                FAIL(_("Unrecognized xml node"));
                        }
                    } else if(!strcmp(f->Value(), "Area")) {
                        FaxArea Area;
                        Area.name = wxString::FromUTF8(f->Attribute("Name"));
                        Area.description = wxString::FromUTF8(f->Attribute("Description"));
                        
                        Area.lat1 = ParseLatLon(wxString::FromUTF8(f->Attribute("lat1")));
                        Area.lat2 = ParseLatLon(wxString::FromUTF8(f->Attribute("lat2")));
                        Area.lon1 = ParseLatLon(wxString::FromUTF8(f->Attribute("lon1")));
                        Area.lon2 = ParseLatLon(wxString::FromUTF8(f->Attribute("lon2")));

                        Areas.push_back(Area);
                    } else
                        FAIL(_("Unrecognized xml node"));
                }

                for(std::list<Schedule>::iterator it = schedules.begin();
                    it != schedules.end(); it++) {
                    if((*it).Duration == -1)
                        FAIL(_("Failed to find Duration for: ") + (*it).Contents);
                    if((*it).area_name.size()) {
                        for(std::list<FaxArea>::iterator it2 = Areas.begin();
                            it2 != Areas.end(); it2++)
                            if((*it).area_name == (*it2).name) {
                                (*it).Area = *it2;
                                goto resolved;
                            }
                        FAIL(_("Failed to match Area: ") + (*it).area_name);
                    }
                resolved:
                    Schedule *s = new Schedule(*it);
                    m_Schedules.push_back(s);
                }
            } else
                FAIL(_("Unrecognized xml node"));
        }
    }

    delete progressdialog;
    return true;

failed:
    delete progressdialog;

    wxMessageDialog mdlg(this, error, _("Weather Fax"), wxOK | wxICON_ERROR);
    mdlg.ShowModal();

    return false;
}

void SchedulesDialog::OnSchedulesLeftDown( wxMouseEvent& event )
{
    wxPoint pos = event.GetPosition();
    int flags = 0;
    long index = m_lSchedules->HitTest(pos, flags);
    
    if (index > -1 && event.GetX() < m_lSchedules->GetColumnWidth(0))
    {
        // Process the clicked item
        Schedule *schedule =
            reinterpret_cast<Schedule*>(wxUIntToPtr(m_lSchedules->GetItemData(index)));
        if(schedule->Capture)
            RemoveScheduleToCapture(schedule);
        else
            AddScheduleToCapture(schedule);

        m_lSchedules->SetItemImage(index, schedule->Capture ? 1 : 0);
        UpdateProgress();
    }

    // Allow wx to process...
    event.Skip();
}


static int sortcol, sortorder = 1;
// sort callback. Sort by body.
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortSchedules(wxIntPtr item1, wxIntPtr item2, wxIntPtr list)
#else
int wxCALLBACK SortSchedules(long item1, long item2, long list)
#endif            
{
    wxListCtrl *lc = (wxListCtrl*)list;

    wxListItem it1, it2;

    it1.SetId(lc->FindItem(-1, item1));
    it1.SetColumn(sortcol);
    
    it2.SetId(lc->FindItem(-1, item2));
    it2.SetColumn(sortcol);
    
    lc->GetItem(it1);
    lc->GetItem(it2);

    if(sortcol == SchedulesDialog::CAPTURE) {
        return sortorder * (it1.GetImage() > it2.GetImage() ? 1 : -1);
    } else
    if(sortcol == SchedulesDialog::FREQUENCIES ||
       sortcol == SchedulesDialog::TIME ||
       sortcol == SchedulesDialog::VALID_TIME ||
       sortcol == SchedulesDialog::DURATION) {
        double a, b;
        it1.GetText().ToDouble(&a);
        it2.GetText().ToDouble(&b);
        return sortorder * ((a > b) ? 1 : -1);
    } else
        return sortorder * it1.GetText().Cmp(it2.GetText());
}

void SchedulesDialog::OnSchedulesSort( wxListEvent& event )
{
    sortcol = event.GetColumn();
    sortorder = -sortorder;

#if 0
    if(sortcol == 0) {
        m_CaptureSchedules.clear();
        for(std::list<Schedule*>::iterator it = m_Schedules.begin();
            it != m_Schedules.end(); it++) {
            (*it)->Capture = sortorder == 1;
            if(sortorder == 1)
                m_CaptureSchedules.push_back(*it);
        }

        RebuildList();
    } else
#endif
        if(m_lSchedules->GetItemCount() > 500)  {
            wxMessageDialog mdlg(this, _("Sorting this many schedules might take too long"),
                                 _("weatherfax"), wxOK | wxICON_ERROR);
            mdlg.ShowModal();
        } else
#if wxCHECK_VERSION(2, 9, 0)
            m_lSchedules->SortItems(SortSchedules, (wxIntPtr)m_lSchedules);
#else
            m_lSchedules->SortItems(SortSchedules, (long)m_lSchedules);
#endif
}

void SchedulesDialog::OnBoatPosition( wxCommandEvent& event )
{
    m_bDisableFilter = true;
    m_tContainsLat->SetValue(wxString::Format(_T("%.5f"), m_weatherfax_pi.m_lastfix.Lat));
    m_bDisableFilter = false;
    m_tContainsLon->SetValue(wxString::Format(_T("%.5f"), m_weatherfax_pi.m_lastfix.Lon));
}

void SchedulesDialog::OnReset( wxCommandEvent& event )
{
    m_bDisableFilter = true;
    m_tContainsLat->SetValue(_T(""));
    m_bDisableFilter = false;
    m_tContainsLon->SetValue(_T(""));
}

void SchedulesDialog::OnAllStations( wxCommandEvent& event )
{
    for(unsigned int i=0; i < m_lStations->GetCount(); i++)
        m_lStations->SetSelection(i);
    Filter();
}

void SchedulesDialog::OnNoStations( wxCommandEvent& event )
{
    m_lStations->DeselectAll();
    Filter();
}

void SchedulesDialog::OnAllFrequencies( wxCommandEvent& event )
{
    m_skhzmin->SetValue(0);
    m_skhzmax->SetValue(30000);
    Filter();
}

void SchedulesDialog::OnClearCaptures( wxCommandEvent& event )
{
    for(std::list<Schedule*>::iterator it = m_Schedules.begin();
        it != m_Schedules.end(); it++)
        (*it)->Capture = false;
    m_CaptureSchedules.clear();
    m_CurrentSchedule = NULL;

    RebuildList();
    UpdateProgress();
}

void SchedulesDialog::OnExternalCommandChoice( wxCommandEvent& event )
{
    switch(m_cExternalCapture->GetSelection()) {
    case 0:
        m_tExternalConversion->SetValue("");
        break;
    case 1:
        m_tExternalConversion->SetValue("sox -b 16 -r 8k -e signed-integer -t raw -c 1 %input");
        break;
    }
}

void SchedulesDialog::OnClose( wxCommandEvent& event )
{
    Hide();
}

bool SchedulesDialog::HasStation(wxString station)
{
    for(unsigned int i=0; i < m_lStations->GetCount(); i++)
        if(m_lStations->IsSelected(i) && m_lStations->GetString(i) == station)
            return true;

    return false;
}

bool SchedulesDialog::AnyFrequency(Schedule *s)
{
    double khzmin = m_skhzmin->GetValue(), khzmax = m_skhzmax->GetValue();
    for(unsigned int i=0; i<s->Frequencies.size(); i++)
        if(s->Frequencies[i]>=khzmin && s->Frequencies[i]<=khzmax)
            return true;
    return false;
}

void SchedulesDialog::Filter()
{
    if(m_bDisableFilter)
        return;

    double lat, lon;
    if(!m_tContainsLat->GetValue().ToDouble(&lat))
        lat = NAN;
    if(!m_tContainsLon->GetValue().ToDouble(&lon))
        lon = NAN;

    for(std::list<Schedule*>::iterator it = m_Schedules.begin();
        it != m_Schedules.end(); it++)
        (*it)->Filtered = !((*it)->Area.ContainsLat(lat) && (*it)->Area.ContainsLon(lon) &&
                            HasStation((*it)->Station) &&
                            AnyFrequency(*it) &&
                            ((*it)->area_name.size() || !m_cbHasArea->GetValue()) &&
                            ((*it)->ValidTime >= 0 || !m_cbHasValidTime->GetValue()));
    
    RebuildList();
}

void SchedulesDialog::RebuildList()
{
    /* as we have a progress dialog, calling update calls event loop which
       can cause recursion here and problems */
    if(m_bRebuilding)
        return;
    m_bRebuilding = true;

    m_lSchedules->DeleteAllItems();

    wxProgressDialog *progressdialog = NULL;
    wxDateTime start = wxDateTime::UNow();

    int i=0;
    int count = m_Schedules.size();
    for(std::list<Schedule*>::iterator it = m_Schedules.begin();
        it != m_Schedules.end(); it++, i++) {
        if(i%250 == 0) {
            if(progressdialog)
                progressdialog->Update(i);
            else {
                wxDateTime now = wxDateTime::UNow();
                if((now-start).GetMilliseconds() > 500 && i < count/3)
                    progressdialog = new wxProgressDialog
                        (_("WeatherFax Schedules"), _("Populating List"), m_Schedules.size(), this,
                         wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
            }
        }

        if(!(*it)->Filtered) {
            wxListItem item;
            item.SetData(*it);
            item.SetId(m_lSchedules->GetItemCount());
            UpdateItem(m_lSchedules->InsertItem(item));
        }
    }

    delete progressdialog;
    m_bRebuilding = false;
}

void SchedulesDialog::UpdateItem(long index)
{
    Schedule *schedule = reinterpret_cast<Schedule*>
        (wxUIntToPtr(m_lSchedules->GetItemData(index)));

    m_lSchedules->SetItemImage(index, schedule->Capture ? 1 : 0);
    m_lSchedules->SetColumnWidth(CAPTURE, wxLIST_AUTOSIZE);

    m_lSchedules->SetItem(index, STATION, schedule->Station);
    m_lSchedules->SetColumnWidth(STATION, wxLIST_AUTOSIZE);

    m_lSchedules->SetItem(index, FREQUENCIES, schedule->frequencies_str());
    m_lSchedules->SetColumnWidth(FREQUENCIES, wxLIST_AUTOSIZE);

    m_lSchedules->SetItem(index, TIME, wxString::Format(_T("%04d"),schedule->Time));
    m_lSchedules->SetColumnWidth(TIME, wxLIST_AUTOSIZE);

    m_lSchedules->SetItem(index, CONTENTS, schedule->Contents);
    m_lSchedules->SetColumnWidth(CONTENTS, wxLIST_AUTOSIZE);

    m_lSchedules->SetItem(index, VALID_TIME, wxString::Format(_T("%02d"), schedule->ValidTime));
    m_lSchedules->SetColumnWidth(VALID_TIME, wxLIST_AUTOSIZE);

    m_lSchedules->SetItem(index, DURATION, wxString::Format(_T("%02d"), schedule->Duration));
    m_lSchedules->SetColumnWidth(DURATION, wxLIST_AUTOSIZE);

    m_lSchedules->SetItem(index, MAP_AREA, schedule->Area.AreaDescription());
    m_lSchedules->SetColumnWidth(MAP_AREA, wxLIST_AUTOSIZE);
}

void SchedulesDialog::AddScheduleToCapture(Schedule *s)
{
    int start = s->StartSeconds(), end = start + s->Duration*60;
    std::list<Schedule*>::iterator it, itp = m_CaptureSchedules.end();
    for(it = m_CaptureSchedules.begin(); it != m_CaptureSchedules.end();) {
        int itstart = (*it)->StartSeconds(), itend = itstart + (*it)->Duration*60;
        if(end > itstart && start < itend) {
            wxMessageDialog mdlg(this, _("Capturing fax: ") + s->Contents +
                                 _(" Conflicts with already scheduled fax: ") + (*it)->Contents +
                                 _(" disable this fax? "),
                                 _("weatherfax schedules"), wxYES_NO | wxICON_WARNING);
            if(mdlg.ShowModal() == wxID_YES) {
                (*it)->Capture = false;
                for(long i=0; i<m_lSchedules->GetItemCount(); i++) {
                    Schedule *schedule = reinterpret_cast<Schedule*>
                        (wxUIntToPtr(m_lSchedules->GetItemData(i)));
                    if(schedule == *it) {
                        m_lSchedules->SetItemImage(i, 0);
                        break;
                    }
                }
                it = m_CaptureSchedules.erase(it);
                continue;
            } else
                return;
        }

        if(start < itstart) {
            if(itp == m_CaptureSchedules.end())
                itp = it;
        }
        it++;
    }

    s->Capture = true;
    m_CaptureSchedules.insert(itp, s);
    if(s == m_CaptureSchedules.front())
        UpdateTimer();
}

void SchedulesDialog::RemoveScheduleToCapture(Schedule *s)
{
    s->Capture = false;
    for(std::list<Schedule*>::iterator it = m_CaptureSchedules.begin();
        it != m_CaptureSchedules.end(); it++)
        if(*it == s) {
            if(m_CaptureSchedules.erase(it) == m_CaptureSchedules.begin())
                UpdateTimer();
            break;
        }
}

void SchedulesDialog::UpdateTimer()
{
    if(m_CaptureSchedules.size() == 0) {
        m_AlarmTimer.Stop();
        m_CaptureTimer.Stop();
    } else {
        Schedule *s = m_CaptureSchedules.front();
        m_AlarmTimer.Start(1000 * (s->StartSeconds() - 60), true);
        /* start 10 seconds early to ensure we get the start signal
           even if clock is slightly wrong */
        m_CaptureTimer.Start(1000 * s->StartSeconds() - 10, true);
    }
}

void SchedulesDialog::UpdateProgress()
{
    Schedule *s = m_CurrentSchedule;
    wxString l;
    if(!s) {
        if(m_CaptureSchedules.size() == 0)
            l = _("No Capture Set");
        else {
            Schedule *f = m_CaptureSchedules.front();
            l = f->Contents + wxString(_T("\n")) + _("starting in");
            int ss = f->StartSeconds();
            if(ss < 60)
                l += wxString::Format( _T(" %d "), ss) + _("second(s)");
            else {
                if(ss >= 3600) {
                    int h = ss/3600;
                    l += wxString::Format( _T(" %d "), h) + _("hour(s)");
                    ss -= h*3600;
                }
                l += wxString::Format( _T(" %d "), ss/60) + _("minute(s)");
            }
        }
        m_gCaptureStatus->SetValue(0);
    } else {
        l = _("Current fax: ") + s->Contents + _T(" on ") + 
            s->frequencies_str() + _T("khz");

        int seconds = s->Seconds(), range = s->Duration*60;
        if(seconds >= range)
            seconds = range-1;
        m_gCaptureStatus->SetRange(range);
        m_gCaptureStatus->SetValue(seconds);
    }

    if(m_stCaptureStatus->GetLabel() != l) {
        m_stCaptureStatus->SetLabel(l);
        m_stCaptureStatus->Fit();
    }

    m_bClearCaptures->Enable(m_CaptureSchedules.size() > 0);
}

void SchedulesDialog::OnAlarmTimer( wxTimerEvent & )
{
    Schedule *s = m_CaptureSchedules.front();

    if(m_cbSkipIfPrevFax && m_CurrentSchedule && s->Station == m_CurrentSchedule->Station)
        return;
    
#if defined(API_VERSION_MAJOR) > 1 || defined(API_VERSION_MINOR) > 9
    if(m_cbSound->GetValue())
        PlugInPlaySound(m_fpSound->GetPath());
#endif

    if(m_cbExternalAlarm->GetValue())
        wxProcess::Open(m_tExternalAlarmCommand->GetValue());

    if(m_cbMessageBox->GetValue()) {
        wxMessageDialog mdlg(this, _("Tune ssb radio to") +
                             s->frequencies_str() + _T(" khz") +
                             _("subtracting") + _T(" 1.9 khz") +
                             _("to receive fax for") + _T(" ") + s->Contents,
                             _("Weather Fax Schedule Beginning Soon"), wxOK);
        mdlg.ShowModal();
    }

}

void SchedulesDialog::OnCaptureTimer( wxTimerEvent &event )
{
    if(m_CurrentSchedule)
        OnEndCaptureTimer( event );

    m_CurrentSchedule = m_CaptureSchedules.front();
    m_CaptureSchedules.pop_front();
    m_CaptureSchedules.push_back(m_CurrentSchedule); // capture again tomorrow

    /* end 10 seconds early to ensure it ends before next starting fax */
    m_EndCaptureTimer.Start(1000 * 60 * m_CurrentSchedule->Duration);

    if(m_rbExternalCapture->GetValue()) {
        if(m_ExternalCaptureProcess) {
            wxMessageDialog mdlg(this, _("Already capturing, cannot capture: ") +
                                 m_CurrentSchedule->Contents, _("weatherfax"), wxOK | wxICON_ERROR);
            mdlg.ShowModal();
        } else {
            m_ExternalCaptureFilename = wxFileName::CreateTempFileName(_T("OCPNWFCapture"));
            wxString command = m_cExternalCapture->GetValue();
            
            int upconverter_frequency = 0;
            if(command.Contains("rtl")) {
                FaxDecoderCaptureSettings CaptureSettings = m_weatherfax_pi.m_CaptureSettings;
                upconverter_frequency = CaptureSettings.rtlsdr_errorppm;
            }
            command.Replace("%frequency", wxString::Format(_T("%d"), (int)(upconverter_frequency + 1000*m_CurrentSchedule->Frequencies[0] - 1900)));
            if(!command.Replace("%output", m_ExternalCaptureFilename))
                command += _T(" ") + m_ExternalCaptureFilename;
                        
            if((m_ExternalCaptureProcess = wxProcess::Open(command))) {
                m_ExternalCaptureProcess->Connect(wxEVT_END_PROCESS, wxProcessEventHandler
                                                  ( SchedulesDialog::OnTerminate ), NULL, this);
                m_bKilled = false;
            } else {
                wxMessageDialog mdlg(this, _("Failed to launch: ") + command,
                                     _("weatherfax"), wxOK | wxICON_ERROR);
                mdlg.ShowModal();
            }
        }
    } else if(m_rbAudioCapture->GetValue()) {
        if(m_CaptureWizard) {
                wxMessageDialog mdlg(this, _("Fault in weather fax plugin\n\
Currently capturing hf weather fax."), _("weatherfax"), wxOK | wxICON_ERROR);
                mdlg.ShowModal();
        } else
            m_CaptureWizard = m_weatherfax_pi.m_pWeatherFax->OpenWav
                (_T(""), 0, m_CurrentSchedule->Station, m_CurrentSchedule->area_name,
                 m_CurrentSchedule->Contents);
    }

    UpdateTimer();
    UpdateProgress();
}

void SchedulesDialog::OnEndCaptureTimer( wxTimerEvent & )
{
    StopExternalProcess();

    if(!m_CurrentSchedule)
        return;

    if(m_rbAudioCapture->GetValue()) {
        if(m_CaptureWizard)
            m_weatherfax_pi.m_pWeatherFax->StopDecoder(m_CaptureWizard);
        m_CaptureWizard = NULL;
    } else {
        bool open = true;
        wxString filename;
        if(m_rbExternalCapture->GetValue()) {
            filename = m_ExternalCaptureFilename;

            wxString command = m_tExternalConversion->GetValue();
            if(!command.empty()) {
                command.Replace("%input", filename);
                filename = wxFileName::CreateTempFileName(_T("OCPNWFCapture")) + _T(".wav");
                if(!command.Replace("%output", filename))
                    command += _T(" ") + filename;

                wxArrayString output, error;
                if(::wxExecute(command, output, error) != 0) {
                    wxMessageDialog mdlg(this, _("Failed to launch: ") + command +
                                         _("\n\nlog:\n") + ::wxJoin(output, '\n') +
                                         _("\n\nerror log:\n") + ::wxJoin(error, '\n'),
                                         _("weatherfax"), wxOK | wxICON_ERROR);
                    mdlg.ShowModal();
                    open = false;
                }
            }
        } else if(m_rbManualCapture->GetValue()) {
            wxFileDialog openDialog( this, _( "Open Weather Fax Input File" ),
                                     m_weatherfax_pi.m_path, wxT ( "" ),
                                     _ ( "WAV files (*.wav)|*.WAV;*.wav|All files (*.*)|*.*" ),
                                     wxFD_OPEN);
            if( openDialog.ShowModal() != wxID_OK )
                open = false;

            m_weatherfax_pi.m_path = openDialog.GetDirectory();        
            filename = openDialog.GetPath();
        }

        if(open)
            m_weatherfax_pi.m_pWeatherFax->OpenWav
                (filename, 0, m_CurrentSchedule->Station, m_CurrentSchedule->area_name,
                 m_CurrentSchedule->Contents);
    }

    m_weatherfax_pi.m_pWeatherFax->UpdateMenuStates();

    m_CurrentSchedule = NULL;
    UpdateTimer();
    UpdateProgress();
}

void SchedulesDialog::OnProgressTimer( wxTimerEvent & )
{
    UpdateProgress();
}

void SchedulesDialog::OnTerminate(wxProcessEvent& event)
{
    if(event.GetPid() == m_ExternalCaptureProcess->GetPid()) {
        if(!m_bKilled) {
            wxMessageDialog mdlg(this, _("External Capture Execution failed"),
                                 _("weatherfax"), wxOK | wxICON_ERROR);
            mdlg.ShowModal();
        }
        m_ExternalCaptureProcess = NULL;
    }
}

void SchedulesDialog::StopExternalProcess()
{
    if(!m_ExternalCaptureProcess)
        return;

    int pid = m_ExternalCaptureProcess->GetPid();
    m_bKilled = true;
    wxProcess::Kill(pid);
    wxThread::Sleep(10);
    if(wxProcess::Exists(pid))
        wxProcess::Kill(pid, wxSIGKILL);
}
