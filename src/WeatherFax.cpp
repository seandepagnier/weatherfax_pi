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

#include "tinyxml/tinyxml.h"

#include "weatherfax_pi.h"
#include "FaxDecoder.h"
#include "WeatherFaxImage.h"
#include "WeatherFax.h"
#include "DecoderOptionsDialog.h"
#include "WeatherFaxWizard.h"
#include "AboutDialog.h"

#include "wximgkap.h"

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
        + s + _T("weatherfax") + s + _T("data") + s;

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
    UpdateMenuStates();

    LoadCoordinatesFromXml(m_BuiltinCoords, _T("CoordinateSets.xml"));
    LoadCoordinatesFromXml(m_UserCoords, _T("UserCoordinateSets.xml"));

    if(m_weatherfax_pi.m_bLoadSchedulesStart)
        m_SchedulesDialog.Load();
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

    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &img = *m_Faxes[selection];
    m_sTransparency->SetValue(img.m_iTransparency);
    m_sWhiteTransparency->SetValue(img.m_iWhiteTransparency);
    m_cInvert->SetValue(img.m_bInvert);

    RequestRefresh( m_parent );
}

void WeatherFax::OnFaxesToggled( wxCommandEvent& event )
{
    RequestRefresh( m_parent );
}

void WeatherFax::OpenWav(wxString filename, wxString station, wxString area)
{
    int transparency = m_sTransparency->GetValue();
    int whitetransparency = m_sWhiteTransparency->GetValue();
    bool invert = m_cInvert->GetValue();

    WeatherFaxImage *img = new WeatherFaxImage(wxNullImage, transparency, whitetransparency, invert);
    WeatherFaxImageCoordinateList BuiltinCoordList;

    wxString name = station.size() && area.size() ? (station + _T(" - ") + area) : _T("");

    for(unsigned int i=0; i<m_BuiltinCoords.GetCount(); i++)
        if(name == m_BuiltinCoords[i]->name)
            img->m_Coords = m_BuiltinCoords[i];

    WeatherFaxWizard wizard(*img, false, _T(""), *this, name.size() ? BuiltinCoordList : m_UserCoords, name);
    
    if(wizard.m_decoder.m_inputtype != FaxDecoder::NONE &&
       wizard.RunWizard(wizard.m_pages[0])) {
        int selection = m_lFaxes->Append(area.size() ? (station + _T(" - ") + area) :
                                         filename.size() ? filename :
                                         wxString(_("Audio Capture") ) );
        m_Faxes.push_back(img);
        
        wizard.StoreCoords();
        wizard.StoreMappingParams();

        m_lFaxes->SetSelection(selection);
                
        RequestRefresh( m_parent );
        UpdateMenuStates();
    } else
        delete img;
}

void WeatherFax::OpenImage(wxString filename, wxString station, wxString area)
{
    int transparency = m_sTransparency->GetValue();
    int whitetransparency = m_sWhiteTransparency->GetValue();
    bool invert = m_cInvert->GetValue();

    wxImage wimg;
    WeatherFaxImageCoordinateList BuiltinCoordList;
    if(!wimg.LoadFile(filename)) {
        wxLogMessage(_("Weather Fax") + wxString(_T(": ")) + _("Failed to load input file: ") + filename);
        return;
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
        WeatherFaxWizard wizard(*img, false, _T(""), *this, name.size() ? BuiltinCoordList : m_UserCoords, name);

        if(wizard.RunWizard(wizard.m_pages[1])) {
            if(name.size() == 0) {
                wxFileName filenamec(filename);
                name = filenamec.GetFullName();
            }

            wizard.StoreCoords();
            wizard.StoreMappingParams();
        } else {
            delete img;
            return;
        }
    }

wizarddone:
    int selection = m_lFaxes->Append(name);
    m_Faxes.push_back(img);

    m_lFaxes->SetSelection(selection);

    RequestRefresh( m_parent );
    UpdateMenuStates();

    if(BuiltinCoordList.GetCount())
        m_BuiltinCoords.Append(BuiltinCoordList[0]);
}

void WeatherFax::Export(wxString filename)
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &image = *m_Faxes[selection];
    
    wximgtokap(image, m_weatherfax_pi.m_iExportColors,
               m_weatherfax_pi.m_bExportDepthMeters ? METERS : FATHOMS,
               m_weatherfax_pi.m_sExportSoundingDatum.mb_str(), filename.mb_str());
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
        if(filenamec.GetExt() == _T("wav") || filenamec.GetExt() == _T("WAV"))
            OpenWav(filename);
        else
            OpenImage(filename);
    }
}

void WeatherFax::OnEdit( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
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

    WeatherFaxWizard wizard(image, false, _T(""), *this, builtin ? BuiltinCoordList : m_UserCoords, _T(""));
    if(wizard.RunWizard(wizard.m_pages[0])) {
        wizard.StoreCoords();
        wizard.StoreMappingParams();

        image.FreeData();
    } else
        image = backupimage;

    m_parent->SetFocus(); /* try this to see if it helps or not */
    RequestRefresh( m_parent );
}

void WeatherFax::OnExport( wxCommandEvent& event )
{
    wxFileDialog saveDialog
        ( this, _( "Save Weather Fax To KAP" ),
          m_weatherfax_pi.m_export_path, wxT ( "" ),
          _ ( "\
Supported Files|*.KAP;*.kap|\
All files (*.*)|*.*" ), wxFD_SAVE);

    if( saveDialog.ShowModal() == wxID_OK ) {
        wxString filename = saveDialog.GetPath();
        m_weatherfax_pi.m_export_path = saveDialog.GetDirectory();        
        Export(filename);
    }
}

void WeatherFax::OnDelete( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    delete m_Faxes[selection];
    m_Faxes.erase(m_Faxes.begin() + selection);

    m_lFaxes->Delete(selection);
    UpdateMenuStates();

    RequestRefresh( m_parent );
}

void WeatherFax::TransparencyChanged( wxScrollEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &image = *m_Faxes[selection];
    image.m_iTransparency = event.GetPosition();
    RequestRefresh( m_parent );
}

void WeatherFax::WhiteTransparencyChanged( wxScrollEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &image = *m_Faxes[selection];
    image.m_iWhiteTransparency = event.GetPosition();
    image.FreeData();
    RequestRefresh( m_parent );
}

void WeatherFax::OnInvert( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &image = *m_Faxes[selection];
    image.m_bInvert = event.IsChecked();
    image.FreeData();
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

void WeatherFax::UpdateMenuStates()
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size()) {
        m_mEdit->Enable(false);
        m_mExport->Enable(false);
        m_mDelete->Enable(false);
        EnableDisplayControls(false);
    } else {
        m_mEdit->Enable();
        m_mExport->Enable();
        m_mDelete->Enable();
        EnableDisplayControls(true);
    }
}

void *DecoderThread::Entry() {
    m_decoder.DecodeFax();
    return NULL;
}
