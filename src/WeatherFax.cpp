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

#include <list>

#include "tinyxml/tinyxml.h"

#include "weatherfax_pi.h"
#include "FaxDecoder.h"
#include "WeatherFaxImage.h"
#include "WeatherFax.h"
#include "DecoderOptionsDialog.h"
#include "WeatherFaxWizard.h"
#include "AboutDialog.h"
#include "icons.h"

#include "georef.h"

#include "wximgkap.h"

/* XPM */
const char * box_xpm[] = {
"20 20 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #333333",
"....................",
".++++++++++++++++++.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".+................+.",
".++++++++++++++++++.",
"...................."};

/* XPM */
const char * check_xpm[] = {
"20 20 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #333333",
"....................",
".++++++++++++++++++.",
".+................+.",
".+.++..........++.+.",
".+.+++........+++.+.",
".+..+++......+++..+.",
".+...+++....+++...+.",
".+....+++..+++....+.",
".+.....++++++.....+.",
".+......++++......+.",
".+......++++......+.",
".+.....++++++.....+.",
".+....+++..+++....+.",
".+...+++....+++...+.",
".+..+++......+++..+.",
".+.+++........+++.+.",
".+.++..........++.+.",
".+................+.",
".++++++++++++++++++.",
"...................."};

/* replace characters a in input with b */
wxString ReplaceChar(wxString input, wxChar a, wxChar b)
{
    wxString output;
    for(unsigned int i=0; i<input.length(); i++)
        if(input[i] == a)
            output += b;
        else
            output += input[i];
    return output;
}

double AttributeDouble(TiXmlElement *e, const char *name, double def)
{
    const char *attr = e->Attribute(name);
    if(!attr)
        return def;
    char *end;
    double d = strtod(attr, &end);
    if(end == attr)
        return def;
    return d;
}

int AttributeInt(TiXmlElement *e, const char *name, int def)
{
    const char *attr = e->Attribute(name);
    if(!attr)
        return def;
    char *end;
    long d = strtol(attr, &end, 10);
    if(end == attr)
        return def;
    return d;
}

#define FAIL(X) do { error = X; goto failed; } while(0)
static void LoadCoordinatesFromXml(WeatherFaxImageCoordinateList &coords, wxString coordinatesets)
{
    TiXmlDocument doc;

    wxString error;
    wxString coordinatesets_path = weatherfax_pi::StandardPath();
    wxString s = wxFileName::GetPathSeparator();
    wxString default_coordinatesets_path = *GetpSharedDataLocation() + _T("plugins")
        + s + _T("weatherfax_pi") + s + _T("data") + s;

    if(!doc.LoadFile((coordinatesets_path + coordinatesets).mb_str()) &&
       !doc.LoadFile((default_coordinatesets_path + coordinatesets).mb_str()))
        FAIL(_("Failed to load coordinate sets"));
    else {
        TiXmlHandle root(doc.RootElement());

        if(strcmp(root.Element()->Value(), "WeatherFaxCoordinates"))
            FAIL(_("Invalid xml file"));

        for(TiXmlElement* e = root.FirstChild().Element(); e; e = e->NextSiblingElement())
            if(!strcmp(e->Value(), "Coordinate")) {
                wxString name = wxString::FromUTF8(e->Attribute("Name"));

                WeatherFaxImageCoordinates *coord = new WeatherFaxImageCoordinates(name);

                coord->p1.x = AttributeInt(e, "X1", 0);
                coord->p1.y = AttributeInt(e, "Y1", 0);
                coord->lat1 = AttributeDouble(e, "Lat1", 0);
                coord->lon1 = AttributeDouble(e, "Lon1", 0);

                coord->p2.x = AttributeInt(e, "X2", 0);
                coord->p2.y = AttributeInt(e, "Y2", 0);
                coord->lat2 = AttributeDouble(e, "Lat2", 0);
                coord->lon2 = AttributeDouble(e, "Lon2", 0);

                wxString rotation = wxString::FromUTF8(e->Attribute("Rotation"));
                if(rotation == _T("CW"))
                    coord->rotation = WeatherFaxImageCoordinates::CW;
                else if(rotation == _T("CCW"))
                    coord->rotation = WeatherFaxImageCoordinates::CCW;
                else if(rotation == _T("180"))
                    coord->rotation = WeatherFaxImageCoordinates::R180;
                else
                    coord->rotation = WeatherFaxImageCoordinates::NONE;

                coord->mapping = WeatherFaxImageCoordinates::GetMapType
                    (wxString::FromUTF8(e->Attribute("Mapping")));
  
                coord->inputpole.x = AttributeInt(e, "InputPoleX", 0);
                coord->inputpole.y = AttributeInt(e, "InputPoleY", 0);
                coord->inputequator = AttributeDouble(e, "InputEquator", 0);

                coord->inputtrueratio = AttributeDouble(e, "InputTrueRatio", 1.0);
                coord->mappingmultiplier = AttributeDouble(e, "MappingMultiplier", 1.0);
                coord->mappingratio = AttributeDouble(e, "MappingRatio", 1.0);

                coords.Append(coord);
            } else
                FAIL(_("Unrecognized xml node: ") + wxString::FromUTF8(e->Value()));
    }
    return;
failed:
    wxLogMessage(_("Weather Fax") + wxString(_T(" : ")) + error);
//    wxMessageDialog mdlg(NULL, error, _("Weather Fax"), wxOK | wxICON_ERROR);
//    mdlg.ShowModal();
}

WeatherFax::WeatherFax( weatherfax_pi &_weatherfax_pi, wxWindow* parent)
    : WeatherFaxBase( parent ),
      m_SchedulesDialog(_weatherfax_pi, this),
      m_InternetRetrievalDialog(_weatherfax_pi, this),
      m_weatherfax_pi(_weatherfax_pi)
{
    wxIcon icon;
    icon.CopyFromBitmap(*_img_weatherfax);
    m_SchedulesDialog.SetIcon(icon);
    m_InternetRetrievalDialog.SetIcon(icon);

    UpdateMenuStates();

    LoadCoordinatesFromXml(m_BuiltinCoords, _T("CoordinateSets.xml"));
    LoadCoordinatesFromXml(m_UserCoords, _T("UserCoordinateSets.xml"));

    if(m_weatherfax_pi.m_bLoadSchedulesStart)
        m_SchedulesDialog.Load();

    m_tDeleteAudioWizard.Connect(wxEVT_TIMER, wxTimerEventHandler( WeatherFax::OnDeleteWizardTimer ), NULL, this);
}

static void SaveCoordinatesToXml(WeatherFaxImageCoordinateList &coords, wxString filename)
{
    TiXmlDocument doc;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "utf-8", "" );
    doc.LinkEndChild( decl );

    TiXmlElement * root = new TiXmlElement( "WeatherFaxCoordinates" );
    doc.LinkEndChild( root );

    for(unsigned int i=0; i<coords.GetCount(); i++) {
        TiXmlElement *c = new TiXmlElement( "Coordinate" );

        c->SetAttribute("Name", coords[i]->name.mb_str());

        c->SetAttribute("X1", wxString::Format(_T("%d"), coords[i]->p1.x).mb_str());
        c->SetAttribute("Y1", wxString::Format(_T("%d"), coords[i]->p1.y).mb_str());
        c->SetAttribute("Lat1", wxString::Format(_T("%.5f"), coords[i]->lat1).mb_str());
        c->SetAttribute("Lon1", wxString::Format(_T("%.5f"), coords[i]->lon1).mb_str());
        
        c->SetAttribute("X2", wxString::Format(_T("%d"), coords[i]->p2.x).mb_str());
        c->SetAttribute("Y2", wxString::Format(_T("%d"), coords[i]->p2.y).mb_str());
        c->SetAttribute("Lat2", wxString::Format(_T("%.5f"), coords[i]->lat2).mb_str());
        c->SetAttribute("Lon2", wxString::Format(_T("%.5f"), coords[i]->lon2).mb_str());

        switch(coords[i]->rotation) {
        case WeatherFaxImageCoordinates::CW:  c->SetAttribute("Rotation", "CW"); break;
        case WeatherFaxImageCoordinates::CCW: c->SetAttribute("Rotation", "CCW"); break;
        case WeatherFaxImageCoordinates::R180: c->SetAttribute("Rotation", "180"); break;
        default: break;
        }
    
        c->SetAttribute("Mapping", WeatherFaxImageCoordinates::MapName(coords[i]->mapping).mb_str());

        if(coords[i]->mapping != WeatherFaxImageCoordinates::MERCATOR) {
            if(coords[i]->mapping != WeatherFaxImageCoordinates::FIXED_FLAT)
                c->SetAttribute("InputPoleX", wxString::Format
                                (_T("%d"), coords[i]->inputpole.x).mb_str());
            c->SetAttribute("InputPoleY", wxString::Format
                            (_T("%d"), coords[i]->inputpole.y).mb_str());
            c->SetAttribute("InputEquator", wxString::Format
                            (_T("%.5f"), coords[i]->inputequator).mb_str());
            c->SetAttribute("InputTrueRatio", wxString::Format
                            (_T("%.4f"), coords[i]->inputtrueratio).mb_str());
        }

        c->SetAttribute("MappingMultiplier", wxString::Format
                        (_T("%.4f"), coords[i]->mappingmultiplier).mb_str());
        c->SetAttribute("MappingRatio", wxString::Format
                        (_T("%.4f"), coords[i]->mappingratio).mb_str());

        root->LinkEndChild(c);
    }

    wxString coordinatesets_path = weatherfax_pi::StandardPath();
    if(!doc.SaveFile((coordinatesets_path + filename).mb_str()))
        wxLogMessage(_("Weather Fax") + wxString(_T(": ")) + _("Failed to save xml file: ") + filename);
}

WeatherFax::~WeatherFax()
{
    SaveCoordinatesToXml(m_BuiltinCoords, _T("CoordinateSets.xml"));
    SaveCoordinatesToXml(m_UserCoords, _T("UserCoordinateSets.xml"));

    for(unsigned int i=0; i<m_Faxes.size(); i++)
        delete m_Faxes[i];
}

void WeatherFax::EnableDisplayControls(bool enable)
{
    m_sTransparency->Enable(enable);
    m_sWhiteTransparency->Enable(enable);
    m_cInvert->Enable(enable);
}

void WeatherFax::OnFaxes( wxCommandEvent& event )
{
    UpdateMenuStates();

    /* take first selection.. we could do an average for sliders, but whatever */
    int selection;
    for(selection = 0; !m_lFaxes->IsSelected(selection); selection++)
        if(selection == (int)m_Faxes.size())
            return;

    WeatherFaxImage &img = *m_Faxes[selection];
    m_sTransparency->SetValue(img.m_iTransparency);
    m_sWhiteTransparency->SetValue(img.m_iWhiteTransparency);
    m_cInvert->SetValue(img.m_bInvert);

    RequestRefresh( m_parent );
}

void WeatherFax::WizardFinished(WeatherFaxWizard *wizard)
{
    int selection = m_lFaxes->Append(wizard->FaxName);
    m_Faxes.push_back(&wizard->GetImage());

    m_lFaxes->DeselectAll();
    m_lFaxes->SetSelection(selection);
    Goto(selection);

    RequestRefresh( m_parent );
    UpdateMenuStates();
    
    WeatherFaxImageCoordinateList &CurrentWizardBuiltinCoordList = wizard->GetBuiltinCoords();
    if(CurrentWizardBuiltinCoordList.GetCount())
        m_BuiltinCoords.Append(CurrentWizardBuiltinCoordList[0]);
}

bool WeatherFax::WizardCleanup(WeatherFaxWizard *wizard)
{
    std::list<WeatherFaxWizard *>::iterator it;
    for(it = m_AudioWizards.begin(); it != m_AudioWizards.end(); it++)
        if(*it == wizard)
            break;

    if(it == m_AudioWizards.end())
        return false;

    if(m_AudioWizards.size() <= 1)
        m_mAudioCapture->Enable();

    m_tDeleteAudioWizard.Start(10, wxTIMER_ONE_SHOT);
    return true;
}

WeatherFaxWizard *WeatherFax::OpenWav(wxString filename, wxString station, wxString area, wxString contents)
{
    int transparency = m_sTransparency->GetValue();
    int whitetransparency = m_sWhiteTransparency->GetValue();
    bool invert = m_cInvert->GetValue();

    WeatherFaxImage *img = new WeatherFaxImage(wxNullImage, transparency, whitetransparency, invert);
    wxString name = station.size() && area.size() ? (station + _T(" - ") + area) : _T("");

    for(unsigned int i=0; i<m_BuiltinCoords.GetCount(); i++)
        if(name == m_BuiltinCoords[i]->name)
            img->m_Coords = m_BuiltinCoords[i];

    WeatherFaxWizard *wizard = new WeatherFaxWizard
        (*img, true, filename, *this, name.size() ? NULL : &m_UserCoords, name);

    if(wizard->m_decoder.m_inputtype == FaxDecoder::NONE) {
        delete img;
        delete wizard;
        return NULL;
    }

    m_mAudioCapture->Enable(false);

    wizard->FaxName  = station.size() && contents.size() ? (station + _T(" - ") + contents) :
        filename.size() ? filename : wxString(_("Audio Capture") );
    wizard->ShowPage(wizard->m_pages[0]);
    wizard->Show();

    m_AudioWizards.push_back(wizard);
    return wizard;
}

void WeatherFax::OpenImage(wxString filename, wxString station, wxString area, wxString contents)
{
    int transparency = m_sTransparency->GetValue();
    int whitetransparency = m_sWhiteTransparency->GetValue();
    bool invert = m_cInvert->GetValue();

    WeatherFaxImageCoordinateList BuiltinCoordList;
    wxImage wimg;
    if (!wimg.CanRead(filename))
        ::wxInitAllImageHandlers();

    if(!wimg.LoadFile(filename)) {
#ifdef WIN32
        // attempt to convert using PVW32Con.exe
        wxString s = wxFileName::GetPathSeparator();
        wxString pvw32con = *GetpSharedDataLocation() + _T("plugins")
            + s + _T("weatherfax_pi") + s + _T("PVW32Con.exe");
        wxString cmd = pvw32con + _T(" \"") + filename + _T("\" -t --o \"" + filename + _T("\""));

        int ret = ::wxExecute(cmd, wxEXEC_SYNC);

        wxLogMessage(_("try to execute :") + cmd + _T("\nreturn code ") + wxString::Format(_T("%d"), ret));
        if(!wimg.LoadFile(filename))
#endif
        {
            wxMessageDialog mdlg(this, _("Failed to load input file: ") + filename,
                                 _("Weather Fax"), wxOK | wxICON_ERROR);
            mdlg.ShowModal();
            return;
        }
    }

    WeatherFaxImage *img = new WeatherFaxImage(wimg, transparency, whitetransparency, invert);
    wxString name = station.size() && area.size() ? (station + _T(" - ") + area) : _T("");

    for(unsigned int i=0; i<m_BuiltinCoords.GetCount(); i++)
        if(name == m_BuiltinCoords[i]->name) {
            img->m_Coords = m_BuiltinCoords[i];
            img->MakePhasedImage();
            if(img->MakeMappedImage(this))
                goto wizarddone;
        }

    {
        WeatherFaxWizard wizard(*img, false, _T(""), *this, name.size() ? &BuiltinCoordList : &m_UserCoords, name);
        if(wizard.RunWizard(wizard.m_pages[1])) {
            if(name.size() == 0) {
                wxFileName filenamec(filename);
                name = filenamec.GetFullName();
            }
        } else {
            delete img;
            return;
        }
    }

wizarddone:
    name = station.size() && contents.size() ? (station + _T(" - ") + contents) : filename;
    int selection = m_lFaxes->Append(name);
    m_Faxes.push_back(img);

    m_lFaxes->DeselectAll();
    m_lFaxes->SetSelection(selection);
    Goto(selection);

    RequestRefresh( m_parent );
    UpdateMenuStates();

    if(BuiltinCoordList.GetCount())
        m_BuiltinCoords.Append(BuiltinCoordList[0]);
}

void WeatherFax::Goto(int selection)
{
    WeatherFaxImage &image = *m_Faxes[selection];
    double lat0 = image.m_Coords->lat(0), lat1 = image.m_Coords->lat(image.m_mappedimg.GetHeight());
    double lon0 = image.m_Coords->lon(0), lon1 = image.m_Coords->lon(image.m_mappedimg.GetWidth());
    if(lon0 - lon1 > 180)
        lon1 += 360;

    double distance;
#if 0 // for opencpn 3.3 and later
    DistanceBearingMercator_Plugin(lat0, lon0, lat1, lon1, NULL, &distance);
#else
    WFDistanceBearingMercator(lat0, lon0, lat1, lon1, NULL, &distance);
#endif
    if(!isnan(distance))
        JumpToPosition((lat0 + lat1) / 2, (lon0 + lon1) / 2, .5/distance);
}

void WeatherFax::OnOpen( wxCommandEvent& event )
{
    wxFileDialog openDialog
        ( this, _( "Open Weather Fax Input File" ),
          m_weatherfax_pi.m_path, wxT ( "" ),
          _ ( "\
Supported Files|*.BMP;*.bmp;*.XBM;*.xbm;*.XPM;*.xpm;\
*.TIF;*.tif;*.TIFF;*.tiff;*.GIF;*.gif;*.JPEG;*.jpeg;*.JPG;*.jpg;*.PNM;*.pnm;*.PNG;*.png;\
*.PCX;*.pcx;*.PICT;*.pict;*.TGA;*.tga;*.WAV;*.wav|\
All files (*.*)|*.*" ), wxFD_OPEN);

    if( openDialog.ShowModal() == wxID_OK ) {
        wxString filename = openDialog.GetPath();
        wxFileName filenamec(filename);
        m_weatherfax_pi.m_path = openDialog.GetDirectory();        
        if(filenamec.GetExt() == _T("wav") || filenamec.GetExt() == _T("WAV")) {
            if(m_AudioWizards.size()) {
                wxMessageDialog mdlg(this, _("Cannot open audio weather fax already in progress."),
                                     _("Weather Fax"), wxOK | wxICON_ERROR);
                mdlg.ShowModal();
                return;
            }
            OpenWav(filename);
        } else
            OpenImage(filename);
    }
}

void WeatherFax::OnSaveAs( wxCommandEvent& event )
{
    for(int selection = 0; selection < (int)m_Faxes.size(); selection++) {
        if(!m_lFaxes->IsSelected(selection))
            continue;

        WeatherFaxImage &image = *m_Faxes[selection];

        wxFileDialog saveDialog
            ( this, _( "Save Weather Fax To Image" ),
              m_weatherfax_pi.m_export_path, image.m_Coords->name + _T(".png"),
              _ ( "\
Image Files|*.BMP;*.bmp|*.PNG;*.png|*.TIFF;*.tiff\
All files (*.*)|*.*" ), wxFD_SAVE);

        if( saveDialog.ShowModal() == wxID_OK ) {
            wxString filename = saveDialog.GetPath();
            m_weatherfax_pi.m_export_path = saveDialog.GetDirectory();        

            if(!image.m_mappedimg.SaveFile(filename)) {
                wxMessageDialog mdlg(this, _("Failed to save file: ") + filename,
                                 _("Weather Fax"), wxOK | wxICON_ERROR);
                mdlg.ShowModal();
            }
        }
    }
}

void WeatherFax::OnEdit( wxCommandEvent& event )
{
    /* take first selection.. we could instead edit all in order... */
    int selection;
    for(selection = 0; !m_lFaxes->IsSelected(selection); selection++)
        if(selection == (int)m_Faxes.size())
            return;

    WeatherFaxImage &image = *m_Faxes[selection];
    WeatherFaxImage backupimage = image;

    bool builtin=false;
    WeatherFaxImageCoordinateList BuiltinCoordList;
    for(unsigned int i=0; i<m_BuiltinCoords.GetCount(); i++)
        if(m_BuiltinCoords[i] == image.m_Coords) {
            BuiltinCoordList.Append(image.m_Coords);
            builtin = true;
        }

    WeatherFaxWizard wizard(image, false, _T(""), *this, builtin ? &BuiltinCoordList : &m_UserCoords, _T(""));
    if(wizard.RunWizard(wizard.m_pages[0]))
        image.FreeData();
    else
        image = backupimage;

    m_parent->SetFocus(); /* try this to see if it helps or not */
    RequestRefresh( m_parent );
}

void WeatherFax::OnGoto( wxCommandEvent& event )
{
    /* take first selection.. we could instead average or something... */
    int selection;
    for(selection = 0; !m_lFaxes->IsSelected(selection); selection++)
        if(selection == (int)m_Faxes.size())
            return;

    Goto(selection);
}

void WeatherFax::OnExport( wxCommandEvent& event )
{
    for(int selection = 0; selection < (int)m_Faxes.size(); selection++) {
        if(!m_lFaxes->IsSelected(selection))
            continue;

        WeatherFaxImage &image = *m_Faxes[selection];
    
        wxFileDialog saveDialog
            ( this, _( "Save Weather Fax To KAP" ),
              m_weatherfax_pi.m_export_path, image.m_Coords->name + _T(".kap"),
              _ ( "\
KAP Files|*.KAP;*.kap|\
All files (*.*)|*.*" ), wxFD_SAVE);

        if( saveDialog.ShowModal() == wxID_OK ) {
            wxString filename = saveDialog.GetPath();
            m_weatherfax_pi.m_export_path = saveDialog.GetDirectory();        

            wximgtokap(image, m_weatherfax_pi.m_iExportColors,
                       m_weatherfax_pi.m_bExportDepthMeters ? METERS : FATHOMS,
                       m_weatherfax_pi.m_sExportSoundingDatum.mb_str(), filename.mb_str());
        } else
            break;
    }
}

void WeatherFax::OnDelete( wxCommandEvent& event )
{
    for(int selection = 0; selection < (int)m_Faxes.size(); selection++) {
        if(!m_lFaxes->IsSelected(selection))
            continue;

        delete m_Faxes[selection];
        m_Faxes.erase(m_Faxes.begin() + selection);

        m_lFaxes->Delete(selection);
        UpdateMenuStates();
        
        RequestRefresh( m_parent );
        selection--;
    }
}

void WeatherFax::TransparencyChanged( wxScrollEvent& event )
{
    bool update = false;
    for(int selection = 0; selection < (int)m_Faxes.size(); selection++) {
        if(!m_lFaxes->IsSelected(selection))
            continue;

        WeatherFaxImage &image = *m_Faxes[selection];
        image.m_iTransparency = event.GetPosition();
        update = true;
    }

    if(update)
        RequestRefresh( m_parent );
}

void WeatherFax::WhiteTransparencyChanged( wxScrollEvent& event )
{
    bool update = false;
    for(int selection = 0; selection < (int)m_Faxes.size(); selection++) {
        if(!m_lFaxes->IsSelected(selection))
            continue;

        WeatherFaxImage &image = *m_Faxes[selection];
        image.m_iWhiteTransparency = event.GetPosition();
        image.FreeData();
        update = true;
    }

    if(update)
        RequestRefresh( m_parent );
}

void WeatherFax::OnInvert( wxCommandEvent& event )
{
    bool update = false;
    for(int selection = 0; selection < (int)m_Faxes.size(); selection++) {
        if(!m_lFaxes->IsSelected(selection))
            continue;

        WeatherFaxImage &image = *m_Faxes[selection];
        image.m_bInvert = event.IsChecked();
        image.FreeData();
        update = true;
    }

    if(update)
        RequestRefresh( m_parent );
}

void WeatherFax::OnCapture( wxCommandEvent& event )
{
    OpenWav(_T(""));
}

void WeatherFax::OnSchedules( wxCommandEvent& event )
{
    m_SchedulesDialog.Load();
    m_SchedulesDialog.Show();
}

void WeatherFax::OnInternet( wxCommandEvent& event )
{
    m_InternetRetrievalDialog.Load();
    m_InternetRetrievalDialog.Show();
}

void WeatherFax::OnAbout( wxCommandEvent& event )
{
    AboutDialog dlg(this);
    dlg.ShowModal();
}

bool WeatherFax::Show( bool show )
{
    for(std::list<WeatherFaxWizard *>::iterator it = m_AudioWizards.begin();
        it != m_AudioWizards.end(); it++)
        (*it)->Show(show);
    return WeatherFaxBase::Show(show);
}

void WeatherFax::UpdateMenuStates()
{
    wxArrayInt Selections;
    bool e = m_lFaxes->GetSelections(Selections) != 0;
    m_mSaveAs->Enable(e);
    m_mEdit->Enable(e);
    m_mGoto->Enable(e);
    m_mExport->Enable(e);
    m_mDelete->Enable(e);
    EnableDisplayControls(e);
}

void WeatherFax::StopDecoder(WeatherFaxWizard *wizard)
{
    // only stop if it's in the list
    for(std::list<WeatherFaxWizard *>::iterator it = m_AudioWizards.begin();
        it != m_AudioWizards.end(); it++)
        if(*it == wizard) {
            wizard->StopDecoder();
            break;
        }
}

void WeatherFax::OnDeleteWizardTimer( wxTimerEvent & )
{
    for(std::list<WeatherFaxWizard *>::iterator it = m_AudioWizards.begin();
        it != m_AudioWizards.end(); it++) {
        if(!(*it)->IsShown()) {
            if(m_SchedulesDialog.m_CaptureWizard == *it)
                m_SchedulesDialog.m_CaptureWizard = NULL;
            delete *it;
            m_AudioWizards.erase(it);
            return;
        }
    }
}

void *DecoderThread::Entry() {
    m_decoder.DecodeFax();
    return NULL;
}
