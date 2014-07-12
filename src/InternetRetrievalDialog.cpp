/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Sean D'Epagnier                                 *
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

#include <wx/wx.h>
#include <wx/imaglist.h>
#include <wx/progdlg.h>
#include <wx/url.h>
#include <wx/wfstream.h>

#include <list>

#include "tinyxml/tinyxml.h"

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
      m_bLoaded(false), m_bDisableServers(false), m_bDisableRegions(false),
      m_bDisableFilter(true), m_bRebuilding(false)
{
}

InternetRetrievalDialog::~InternetRetrievalDialog()
{
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

    wxString regions;
    for(unsigned int i=0; i < m_lRegions->GetCount(); i++)
        if(m_lRegions->IsSelected(i))
            regions += m_lRegions->GetString(i) + _T(";");

    pConf->Write ( _T ( "Regions" ), regions);

    wxString selected;
    for(std::list<FaxUrl*>::iterator it = m_InternetRetrieval.begin();
        it != m_InternetRetrieval.end(); it++)
        if((*it)->Selected)
            selected += (*it)->Url + _T(";");
    pConf->Write ( _T ( "selected" ), selected );

    ClearInternetRetrieval();
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

    m_lUrls->InsertColumn(SCHEDULED, _("Scheduled"));
    m_lUrls->InsertColumn(SERVER, _("Server"));
    m_lUrls->InsertColumn(REGION, _("Region"));
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

    wxString regions;
    pConf->Read ( _T ( "Regions" ), &regions, _T(""));

    /* split at each ; to get all the names in a list */
    std::list<wxString> regionlist;
    while(regions.size()) {
        regionlist.push_back(regions.BeforeFirst(';'));
        regions = regions.AfterFirst(';');
    }

    wxString selected;
    pConf->Read ( _T ( "selected" ), &selected, _T("") );
    std::list<wxString> selectedlist;
    /* split at each ; to get all the names in a list */
    while(selected.size()) {
        selectedlist.push_back(selected.BeforeFirst(';'));
        selected = selected.AfterFirst(';');
    }

    s = wxFileName::GetPathSeparator();
    OpenXML(*GetpSharedDataLocation() + _T("plugins")
            + s + _T("weatherfax_pi") + s + _T("data") + s
            + _T("WeatherFaxInternetRetrieval.xml"));

    for(unsigned int i=0; i < m_lServers->GetCount(); i++)
        for(std::list<wxString>::iterator it = serverlist.begin();
            it != serverlist.end(); it++)
            if(m_lServers->GetString(i) == *it)
                m_lServers->SetSelection(i);

    RebuildRegions();
    for(unsigned int i=0; i < m_lRegions->GetCount(); i++)
        for(std::list<wxString>::iterator it = regionlist.begin();
            it != regionlist.end(); it++)
            if(m_lRegions->GetString(i) == *it)
                m_lRegions->SetSelection(i);

    for(std::list<wxString>::iterator it = selectedlist.begin();
        it != selectedlist.end(); it++)
        for(std::list<FaxUrl*>::iterator it2 = m_InternetRetrieval.begin();
            it2 != m_InternetRetrieval.end(); it2++)
            if((*it) == (*it2)->Url)
                (*it2)->Selected = true;

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
                FaxServer server;
                server.Name = wxString::FromUTF8(e->Attribute("Name"));

                m_Servers.push_back(server);

                wxString server_url = wxString::FromUTF8(e->Attribute("Url"));
                m_lServers->Append(server.Name);

                for(TiXmlElement* f = e->FirstChildElement(); f; f = f->NextSiblingElement()) {
                    if(!strcmp(f->Value(), "Region")) {
                        FaxRegion region;
                        region.Name = wxString::FromUTF8(f->Attribute("Name"));
                        region.Server = server.Name;

                        for(std::list<FaxRegion>::iterator it = m_Regions.begin();
                            it != m_Regions.end(); it++)
                            if(it->Name == region.Name && it->Server == region.Server)
                                goto duplicate_region;

                        m_Regions.push_back(region);
                    duplicate_region:

                        wxString region_url = server_url + wxString::FromUTF8(f->Attribute("Url"));

                        std::list<FaxUrl> urls;
                        std::list<FaxArea> Areas;

                        for(TiXmlElement* g = f->FirstChildElement(); g; g = g->NextSiblingElement()) {
                            if(!strcmp(g->Value(), "Iterator")) {
                                wxString s_start = wxString::FromUTF8(g->Attribute("From"));
                                wxString s_to = wxString::FromUTF8(g->Attribute("To"));
                                wxString s_by = wxString::FromUTF8(g->Attribute("By"));

                                if(s_start.size() == 0 || s_to.size() == 0 || s_by.size() == 0)
                                    FAIL(_("Invalid iterator: ") + wxString::FromUTF8(g->Value()));
                                
                                long start, to, by;
                                s_start.ToLong(&start);
                                s_to.ToLong(&to);
                                s_by.ToLong(&by);

                                for(TiXmlElement* h = g->FirstChildElement(); h; h = h->NextSiblingElement()) {
                                    if(!strcmp(h->Value(), "Map")) {
                                        FaxUrl url;
                                    
                                        url.Selected = false;
                                        url.Server = server.Name;
                                        url.Region = region.Name;
                         
                                        for(int index = start; index <= to; index += by) {
                                            wxString iurl = wxString::FromUTF8(h->Attribute("Url"));
                                            url.Url = region_url + wxString::Format
                                                (iurl, index);
                                            url.Contents = wxString::Format
                                                (wxString::FromUTF8(h->Attribute("Contents")), index);
                                            url.area_name = wxString::FromUTF8(h->Attribute("Area"));
                                        
                                            urls.push_back(url);
                                        }
                                    } else
                                        FAIL(_("Unrecognized xml node: ") + wxString::FromUTF8(g->Value()));
                                }
                            } else if(!strcmp(g->Value(), "Map")) {
                                FaxUrl url;
                                    
                                url.Selected = false;
                                url.Server = server.Name;
                                url.Region = region.Name;
                         
                                url.Url = region_url + wxString::FromUTF8(g->Attribute("Url"));
                                url.Contents = wxString::FromUTF8(g->Attribute("Contents"));
                                url.area_name = wxString::FromUTF8(g->Attribute("Area"));

                                urls.push_back(url);
                            } else if(!strcmp(g->Value(), "Area")) {
                                FaxArea Area;
                                Area.name = wxString::FromUTF8(g->Attribute("Name"));
                                Area.description = wxString::FromUTF8(g->Attribute("Description"));
                        
                                Area.lat1 = ParseLatLon(wxString::FromUTF8(g->Attribute("lat1")));
                                Area.lat2 = ParseLatLon(wxString::FromUTF8(g->Attribute("lat2")));
                                Area.lon1 = ParseLatLon(wxString::FromUTF8(g->Attribute("lon1")));
                                Area.lon2 = ParseLatLon(wxString::FromUTF8(g->Attribute("lon2")));

                                Areas.push_back(Area);
                            } else
                                FAIL(_("Unrecognized xml node: ") + wxString::FromUTF8(g->Value()));
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
                        FAIL(_("Unrecognized xml node: ") + wxString::FromUTF8(f->Value()));
                }
            } else
                FAIL(_("Unrecognized xml node: ") + wxString::FromUTF8(e->Value()));
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
        for(int i=0; i < m_lUrls->GetItemCount(); i++) {
            FaxUrl *faxurl = reinterpret_cast<FaxUrl*>
                (wxUIntToPtr(m_lUrls->GetItemData(i)));

            faxurl->Selected = sortorder == 1;
            UpdateItem(i);
        }
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

void InternetRetrievalDialog::OnAllRegions( wxCommandEvent& event )
{
    for(unsigned int i=0; i < m_lRegions->GetCount(); i++)
        m_lRegions->SetSelection(i);
    Filter();
}

void InternetRetrievalDialog::OnNoRegions( wxCommandEvent& event )
{
    m_lRegions->DeselectAll();
    Filter();
}

#if 0
/* put download in a thread because the wx routines are all blocking
   this allows the user to cancel interactively
 */
class DownloadThread : public wxThread
{
public:
    DownloadThread(wxString fu, wxString fn)
        : wxThread(wxTHREAD_JOINABLE), please_quit(false), url(fu), filename(fn),
          position(0), size(0), exitcode(0)
        {
            Create();
        }

    void *Entry()
        {
            wxURL wxurl(url);

            if(wxurl.GetError() != wxURL_NOERR) {
                exitcode = 1;
                return NULL;
            }

            wxInputStream *input = wxurl.GetInputStream();
            if(!input) {
                exitcode = 2;
                return NULL;
            }

            size = input->GetSize();
            if(size < 0) {
                exitcode = 3;
                return NULL;
            }

            wxFileOutputStream output(filename);
            while(!input->Eof() && !TestDestroy() && !please_quit) {
                if(input->CanRead()) {
                    char buffer[16384];
                    input->Read(buffer, sizeof buffer);
                    int lastread = input->LastRead();
                    output.Write(buffer, lastread);
                    position += lastread;
                } else
                    wxThread::Sleep(100);

                if(position >= size)
                    break;
            }
            return NULL;
        }

    bool please_quit;
    wxString url, filename;
    int position, size;
    
    int exitcode;
};
#endif

#include "wx/curl/dialog.h"
void InternetRetrievalDialog::OnRetrieve( wxCommandEvent& event )
{
    int count = 0;
    for(int i=0; i < m_lUrls->GetItemCount(); i++) {
        FaxUrl *faxurl = reinterpret_cast<FaxUrl*>
            (wxUIntToPtr(m_lUrls->GetItemData(i)));

        if(event.GetEventObject() == m_bRetrieve) {
            if(!faxurl->Selected)
                continue;
        } else
            if(m_lUrls->GetNextItem(i - 1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) != i)
                continue;

        count++;

        wxString path = weatherfax_pi::StandardPath();

        wxString filename = faxurl->Url;
        filename.Replace(_T("/"), _T("!"));
        filename.Replace(_T(":"), _T("!"));

        filename = path + wxFileName::GetPathSeparator() + filename;

        wxFileName fn(filename);
        if(fn.FileExists() && (wxDateTime::Now() - fn.GetModificationTime()).GetMinutes() < 180) {
            wxMessageDialog mdlg(this, _("Fax already retrieved less than 180 minutes ago.\n\
Use existing file?"), _("Weather Fax"), wxYES | wxNO | wxCANCEL);
            switch(mdlg.ShowModal()) {
            case wxID_YES:
                goto loadimage;
            case wxID_NO:
                break;
            default:
                return;
            }
        }

        {
#if 0
            wxProgressDialog progressdialog(_("WeatherFax InternetRetrieval"),
                                            _("Reading Headers: ") + faxurl->Contents, 1000, this,
                                            wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
            progressdialog.Update(0);
            
            DownloadThread *dl = new DownloadThread(faxurl->Url, filename);
            dl->Run();

            while(dl->IsRunning()) {
                bool ok;
                if(dl->size)
                    ok = progressdialog.Update(1000*dl->position / dl->size,
                                               _("Reading") + wxString(_T(": "))
                                               + faxurl->Contents);
                else
                    ok = progressdialog.Update(0);
                if(!ok) {
                    dl->please_quit = true;
                    return;
                }
                wxThread::Sleep(250);
            }

            dl->please_quit = true;

            switch(dl->exitcode) {
            case 1:
            case 2:
            {
                wxMessageDialog mdlg(this, _("Timed out waiting for headers for: ") +
                                     faxurl->Contents + _T("\n") +
                                     faxurl->Url + _T("\n") +
                                     _("Verify there is a working internet connection.") + _T("\n") +
                                     _("If the url is incorrect please edit the xml and/or post a bug report."),
                                     _("Weather Fax"), wxOK | wxICON_ERROR);
                mdlg.ShowModal();
            } return;
            case 3:
            { wxMessageDialog mdlg(this, _("Failed to read file size aborting."),
                                     _("Weather Fax"), wxOK | wxICON_INFORMATION);
                mdlg.ShowModal();
            } return;
            }
#else

        wxFileOutputStream output(filename);
        wxCurlDownloadDialog ddlg(faxurl->Url, &output, _("WeatherFax InternetRetrieval"),
                                  _("Reading Headers: ") + faxurl->Contents, wxNullBitmap, this,
                                  wxCTDS_CAN_PAUSE|wxCTDS_CAN_ABORT|wxCTDS_SHOW_ALL|wxCTDS_AUTO_CLOSE);

        switch(ddlg.RunModal()) {
        case wxCDRF_SUCCESS: break;
        case wxCDRF_FAILED:
        {
            wxMessageDialog mdlg(this, _("Failed to Download: ") +
                                 faxurl->Contents + _T("\n") +
                                 faxurl->Url + _T("\n") +
                                 _("Verify there is a working internet connection.") + _T("\n") +
                                 _("If the url is incorrect please edit the xml and/or post a bug report."),
                                 _("Weather Fax"), wxOK | wxICON_ERROR);
            mdlg.ShowModal();
            wxRemoveFile( filename );
        }
        case wxCDRF_USER_ABORTED: return;
        }
#endif
    }

    loadimage:
        m_weatherfax_pi.m_pWeatherFax->OpenImage
            (filename, faxurl->Server + _T(" - ") + faxurl->Region, faxurl->area_name, faxurl->Contents);

    }
    
    /* inform user if no faxes were selected */
    if(count == 0) {
        wxMessageDialog mdlg(this, _("No Faxes selected.") + wxString(_T("\n")) +
                             wxString(event.GetEventObject() == m_bRetrieve ?
                                      _(" Try clicking the selected (first) column") :
                                      _T("")), _("Weather Fax"), wxOK | wxICON_INFORMATION);
        mdlg.ShowModal();
    }
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

bool InternetRetrievalDialog::HasRegion(wxString region)
{
    for(unsigned int i=0; i < m_lRegions->GetCount(); i++)
        if(m_lRegions->IsSelected(i) && m_lRegions->GetString(i) == region)
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

    /* reset filter flags */
    for(std::list<FaxServer>::iterator it = m_Servers.begin();
        it != m_Servers.end(); it++)
        it->Filtered = true;

    for(std::list<FaxRegion>::iterator it = m_Regions.begin();
        it != m_Regions.end(); it++)
        it->Filtered = true;

    for(std::list<FaxUrl*>::iterator it = m_InternetRetrieval.begin();
        it != m_InternetRetrieval.end(); it++) {
        bool boat_in_area = (*it)->Area.ContainsLat(lat) && (*it)->Area.ContainsLon(lon);
        if(boat_in_area) {
            for(std::list<FaxServer>::iterator it2 = m_Servers.begin();
                it2 != m_Servers.end(); it2++)
                if(it2->Name == (*it)->Server)
                    it2->Filtered = false;

            for(std::list<FaxRegion>::iterator it2 = m_Regions.begin();
                it2 != m_Regions.end(); it2++)
                if(it2->Name == (*it)->Region && it2->Server == (*it)->Server)
                    it2->Filtered = false;
        }
            
        (*it)->Filtered = !(boat_in_area && HasServer((*it)->Server) && HasRegion((*it)->Region));
    }

    RebuildServers();
    RebuildRegions();
    RebuildList();
}

void InternetRetrievalDialog::RebuildServers()
{
    if(m_bDisableServers || m_bDisableRegions)
        return;

    for(std::list<FaxServer>::iterator it = m_Servers.begin(); it != m_Servers.end(); it++)
        it->Selected = HasServer(it->Name);

    m_bDisableFilter = true;
    m_lServers->Clear();
    for(std::list<FaxServer>::iterator it = m_Servers.begin(); it != m_Servers.end(); it++)
        if(!it->Filtered)
            m_lServers->SetSelection(m_lServers->Append(it->Name), it->Selected);
    m_bDisableFilter = false;
}

void InternetRetrievalDialog::RebuildRegions()
{
    if(m_bDisableRegions)
        return;

    /* remember which regions are selected */
    for(std::list<FaxRegion>::iterator it = m_Regions.begin(); it != m_Regions.end(); it++)
        it->Selected = HasRegion(it->Name);

    m_bDisableFilter = true;
    m_lRegions->Clear();
    for(std::list<FaxRegion>::iterator it = m_Regions.begin(); it != m_Regions.end(); it++) {
        /* avoid duplicate regions from different servers */
        for(unsigned int i=0; i < m_lRegions->GetCount(); i++)
            if(m_lRegions->GetString(i) == it->Name)
                goto skip;

        for(unsigned int i=0; i < m_lServers->GetCount(); i++)
            if(!it->Filtered && m_lServers->IsSelected(i) && m_lServers->GetString(i) == it->Server) {
                m_lRegions->SetSelection(m_lRegions->Append(it->Name), it->Selected);
                break;
            }
    skip:;
    }

    m_bDisableFilter = false;
}

void InternetRetrievalDialog::RebuildList()
{
    /* as we have a progress dialog, calling update calls event loop which
       can cause recursion here and problems */
    if(m_bRebuilding)
        return;
    m_bRebuilding = true;

    m_lUrls->DeleteAllItems();

    //   wxProgressDialog progressdialog(_("WeatherFax InternetRetrieval"), 
//                                    _("Populating List"), m_InternetRetrieval.size(),
//                                    this, wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
    int i=0;
    for(std::list<FaxUrl*>::iterator it = m_InternetRetrieval.begin();
        it != m_InternetRetrieval.end(); it++, i++) {
        //      if(i%100 == 0)
//            progressdialog.Update(i);
        if(!(*it)->Filtered) {
            wxListItem item;
            item.SetData(*it);
            item.SetId(m_lUrls->GetItemCount());
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
    m_lUrls->SetColumnWidth(SCHEDULED, 50);

    m_lUrls->SetItem(index, SERVER, url->Server);
    m_lUrls->SetColumnWidth(SERVER, 120 /*wxLIST_AUTOSIZE*/);

    m_lUrls->SetItem(index, REGION, url->Region);
    m_lUrls->SetColumnWidth(REGION, 120 /*wxLIST_AUTOSIZE*/);

    m_lUrls->SetItem(index, CONTENTS, url->Contents);
    m_lUrls->SetColumnWidth(CONTENTS, 350 /*wxLIST_AUTOSIZE*/);

    m_lUrls->SetItem(index, MAP_AREA, url->Area.AreaDescription());
    m_lUrls->SetColumnWidth(MAP_AREA, 150);
}
