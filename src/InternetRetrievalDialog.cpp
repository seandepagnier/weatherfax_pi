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

#include <wx/imaglist.h>
#include <wx/progdlg.h>
#include <wx/wx.h>

#include <list>

// #include "../../../include/tinyxml.h"
#include "tinyxml.h"

#include "weatherfax_pi.h"
#include "WeatherFaxImage.h"
#include "WeatherFax.h"

#include "FaxDecoder.h"

/* XPM */
static const char * check_xpm[] = {
"20 20 7 1",
" 	c #595959",
".	c #676767",
"+	c #818181",
"@	c #868686",
"#	c #969696",
"$	c #AAAAAA",
"%	c #E3E3E3",
"$%%%%%%%%%%%%%%$%%%%",
"%%%%%%%%%%%%%%%%%%%+",
"%%%%%%%%%%%%%%%%%+  ",
"%%%%%%%%%%%%%%%%+ +$",
"%%%%%%%%%%%%%%%%  $%",
"%%%%%%%%%%%%%%%  +%%",
"%%%%%%%%%%%%%%   %%%",
"%%%%%%%%%%%%%   .%%%",
"%%%+%%%%%%%%   +%%%%",
"%%+ +%%%%%%+  +%%%%%",
"%%+  +%%%%+  +%%%%%%",
"%%+   +%%+  +%%%%%%%",
"%%%    ++   %%%%%%%%",
"%%%%       +%%%%%%%%",
"%%%%%     +%%%%%%%%%",
"%%%%%+   +%%%%%%%%%%",
"%%%%%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%%%%%",
"%%%%%%%%%%%%%%%%$%%%",
"%%%%%%%%%%%%%%%%%%%%"};

InternetRetrievalDialog::InternetRetrievalDialog( weatherfax_pi &_weatherfax_pi, wxWindow* parent)
    : InternetRetrievalDialogBase( parent ), m_weatherfax_pi(_weatherfax_pi),
      m_bLoaded(false), m_bDisableFilter(true), m_bRebuilding(false)
{
}

InternetRetrievalDialog::~InternetRetrievalDialog()
{
    ClearInternetRetrieval();

    wxFileConfig *pConf = m_weatherfax_pi.m_pconfig;

    /* remove from config all cordinate sets */
    pConf->SetPath ( _T ( "/Settings/WeatherFax/InternetRetrieval" ) );

    pConf->Write ( _T ( "ContainsLat" ), m_tContainsLat->GetValue() );
    pConf->Write ( _T ( "ContainsLon" ), m_tContainsLon->GetValue() );

    wxString servers;
    for(unsigned int i=0; i < m_lServers->GetCount(); i++)
        if(m_lServers->IsSelected(i))
            servers += m_lServers->GetString(i) + _T(";");

    pConf->Write ( _T ( "Servers" ), servers);
}

void InternetRetrievalDialog::Load()
{
    if(m_bLoaded)
        return;

    m_bLoaded = true;

    // create a image list for the list with check
    wxImageList *imglist = new wxImageList(20, 20, true, 1);
    imglist->Add(wxBitmap(check_xpm));
    m_lUrls->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

    m_lUrls->InsertColumn(SELECTED, _("Selected"));
    m_lUrls->InsertColumn(SERVER, _("Server"));
    m_lUrls->InsertColumn(CONTENTS, _("Contents"));
    m_lUrls->InsertColumn(MAP_AREA, _("Map Area"));

    wxFileConfig *pConf = m_weatherfax_pi.m_pconfig;

    /* remove from config all cordinate sets */
    pConf->SetPath ( _T ( "/Settings/WeatherFax/InternetRetrieval" ) );

    wxString s;
    pConf->Read ( _T ( "ContainsLat" ), &s, _T("") );
    m_tContainsLat->SetValue(s);
    pConf->Read ( _T ( "ContainsLon" ), &s, _T("") );
    m_tContainsLon->SetValue(s);

    wxString servers;
    pConf->Read ( _T ( "Servers" ), &servers, _T(""));

    /* split at each ; to get all the names in a list */
    std::list<wxString> serverlist;
    while(servers.size()) {
        serverlist.push_back(servers.BeforeFirst(';'));
        servers = servers.AfterFirst(';');
    }

    s = wxFileName::GetPathSeparator();
    OpenXML(*GetpSharedDataLocation() + _T("plugins")
            + s + _T("weatherfax") + s + _T("data") + s
            + _T("WeatherFaxInternetRetrieval.xml"));

    m_lServers->DeselectAll();
    for(unsigned int i=0; i < m_lServers->GetCount(); i++)
        for(std::list<wxString>::iterator it = serverlist.begin();
            it != serverlist.end(); it++)
            if(m_lServers->GetString(i) == *it)
                m_lServers->SetSelection(i);

    m_bDisableFilter = false;
    Filter();
}

#define FAIL(X) do { error = X; goto failed; } while(0)

void InternetRetrievalDialog::ClearInternetRetrieval()
{
    for(std::list<FaxUrl*>::iterator it = m_InternetRetrieval.begin();
        it != m_InternetRetrieval.end(); it++)
        delete *it;
    m_InternetRetrieval.clear();
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

bool InternetRetrievalDialog::OpenXML(wxString filename)
{
    ClearInternetRetrieval();
    m_lServers->Clear();

    TiXmlDocument doc;
    wxString error;
    wxProgressDialog *progressdialog = NULL;
//    wxDateTime start = wxDateTime::Now();

    if(!doc.LoadFile(filename.mb_str()))
        FAIL(_("Failed to load file: ") + filename);
    else {
        TiXmlElement *root = doc.RootElement();
        if(strcmp(root->Value(), "OCPNWeatherFaxInternetRetrieval"))
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
//                wxDateTime now = wxDateTime::Now();
                if(1/*(now-start).GetMilliseconds() > 1000 && i < count/2*/) {
                    progressdialog = new wxProgressDialog(
                        _("WeatherFax InternetRetrieval"), _("Loading"), count, this,
                        wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
                }
            }

            if(!strcmp(e->Value(), "Server")) {
                wxString server = wxString::FromUTF8(e->Attribute("Name"));
                m_lServers->Append(server);
                std::list<FaxUrl> urls;
                std::list<FaxArea> Areas;

                for(TiXmlElement* f = e->FirstChildElement(); f; f = f->NextSiblingElement()) {
                    if(!strcmp(f->Value(), "Map")) {
                        FaxUrl url;
                                    
                        url.Selected = false;
                        url.Server = server;
                         
                        url.Url = wxString::FromUTF8(f->Attribute("Url"));
                        url.Contents = wxString::FromUTF8(f->Attribute("Contents"));

                        urls.push_back(url);
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

                for(std::list<FaxUrl>::iterator it = urls.begin();
                    it != urls.end(); it++) {
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
                    FaxUrl *s = new FaxUrl(*it);
                    m_InternetRetrieval.push_back(s);
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

void InternetRetrievalDialog::OnUrlsLeftDown( wxMouseEvent& event )
{
    wxPoint pos = event.GetPosition();
    int flags = 0;
    long index = m_lUrls->HitTest(pos, flags);
    
    if (index > -1 && event.GetX() < m_lUrls->GetColumnWidth(0))
    {
        // Process the clicked item
        FaxUrl *url =
            reinterpret_cast<FaxUrl*>(wxUIntToPtr(m_lUrls->GetItemData(index)));
        url->Selected = !url->Selected;

        m_lUrls->SetItemImage(index, url->Selected ? 0 : -1);
    }

    // Allow wx to process...
    event.Skip();
}


static int sortcol, sortorder = 1;
// sort callback. Sort by body.
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortUrl(long item1, long item2, wxIntPtr list)
#else
int wxCALLBACK SortUrl(long item1, long item2, long list)
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
    
    if(0) { /* numeric */
        double a, b;
        it1.GetText().ToDouble(&a);
        it2.GetText().ToDouble(&b);
        return sortorder * ((a > b) ? 1 : -1);
    } else
        return sortorder * it1.GetText().Cmp(it2.GetText());
}

void InternetRetrievalDialog::OnUrlsSort( wxListEvent& event )
{
    sortcol = event.GetColumn();
    sortorder = -sortorder;

    if(sortcol == 0) {
#if 0
        m_SelectedInternetRetrieval.clear();
        for(std::list<FaxUrl*>::iterator it = m_InternetRetrieval.begin();
            it != m_InternetRetrieval.end(); it++) {
            (*it)->Selected = sortorder == 1;
            if(sortorder == 1)
                m_SelectedInternetRetrieval.push_back(*it);
        }

        RebuildList();
#endif
    } else
        if(m_lUrls->GetItemCount() > 500)  {
            wxMessageDialog mdlg(this, _("Sorting this many urls might take too long"),
                                 _("weatherfax"), wxOK | wxICON_ERROR);
            mdlg.ShowModal();
        } else
            m_lUrls->SortItems(SortUrl, (long)m_lUrls);
}

void InternetRetrievalDialog::OnBoatPosition( wxCommandEvent& event )
{
    m_bDisableFilter = true;
    m_tContainsLat->SetValue(wxString::Format(_T("%.5f"), m_weatherfax_pi.m_lastfix.Lat));
    m_bDisableFilter = false;
    m_tContainsLon->SetValue(wxString::Format(_T("%.5f"), m_weatherfax_pi.m_lastfix.Lon));
}

void InternetRetrievalDialog::OnReset( wxCommandEvent& event )
{
    m_bDisableFilter = true;
    m_tContainsLat->SetValue(_T(""));
    m_bDisableFilter = false;
    m_tContainsLon->SetValue(_T(""));
}

void InternetRetrievalDialog::OnAllServers( wxCommandEvent& event )
{
    for(unsigned int i=0; i < m_lServers->GetCount(); i++)
        m_lServers->SetSelection(i);
    Filter();
}

void InternetRetrievalDialog::OnNoServers( wxCommandEvent& event )
{
    m_lServers->DeselectAll();
    Filter();
}

void InternetRetrievalDialog::OnRetrieve( wxCommandEvent& event )
{
    
}

void InternetRetrievalDialog::OnClose( wxCommandEvent& event )
{
    Hide();
}

bool InternetRetrievalDialog::HasServer(wxString server)
{
    for(unsigned int i=0; i < m_lServers->GetCount(); i++)
        if(m_lServers->IsSelected(i) && m_lServers->GetString(i) == server)
            return true;

    return false;
}

void InternetRetrievalDialog::Filter()
{
    if(m_bDisableFilter)
        return;

    double lat, lon;
    if(!m_tContainsLat->GetValue().ToDouble(&lat))
        lat = NAN;
    if(!m_tContainsLon->GetValue().ToDouble(&lon))
        lon = NAN;

    for(std::list<FaxUrl*>::iterator it = m_InternetRetrieval.begin();
        it != m_InternetRetrieval.end(); it++)
        (*it)->Filtered = !(/*(*it)->Area.ContainsLat(lat) && (*it)->Area.ContainsLon(lon) &&*/
                            HasServer((*it)->Server));
    RebuildList();
}

void InternetRetrievalDialog::RebuildList()
{
    /* as we have a progress dialog, calling update calls event loop which
       can cause recursion here and problems */
    if(m_bRebuilding)
        return;
    m_bRebuilding = true;

    m_lUrls->DeleteAllItems();

    wxProgressDialog progressdialog(_("WeatherFax InternetRetrieval"), 
                                    _("Populating List"), m_InternetRetrieval.size(),
                                    this, wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
    int i=0;
    for(std::list<FaxUrl*>::iterator it = m_InternetRetrieval.begin();
        it != m_InternetRetrieval.end(); it++, i++) {
        if(i%100 == 0)
            progressdialog.Update(i);
        if(!(*it)->Filtered) {
            wxListItem item;
            item.SetData(*it);
            UpdateItem(m_lUrls->InsertItem(item));
        }
    }
    m_bRebuilding = false;
}

void InternetRetrievalDialog::UpdateItem(long index)
{
    FaxUrl *url = reinterpret_cast<FaxUrl*>
        (wxUIntToPtr(m_lUrls->GetItemData(index)));

    m_lUrls->SetItemImage(index, url->Selected ? 0 : -1);
    m_lUrls->SetColumnWidth(SELECTED, 50);

    m_lUrls->SetItem(index, SERVER, url->Server);
    m_lUrls->SetColumnWidth(SERVER, 100 /*wxLIST_AUTOSIZE*/);

    m_lUrls->SetItem(index, CONTENTS, url->Contents);
    m_lUrls->SetColumnWidth(CONTENTS, 350 /*wxLIST_AUTOSIZE*/);

    m_lUrls->SetItem(index, MAP_AREA, _T("")/*url->Area.AreaDescription()*/);
    m_lUrls->SetColumnWidth(MAP_AREA, 150);
}
