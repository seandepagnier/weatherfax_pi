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

#include "../../../include/tinyxml.h"

#include "weatherfax_pi.h"
#include "FaxDecoder.h"
#include "WeatherFaxImage.h"
#include "WeatherFax.h"
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
WeatherFax::WeatherFax( weatherfax_pi &_weatherfax_pi, wxWindow* parent)
    : WeatherFaxBase( parent ),
      m_SchedulesDialog(_weatherfax_pi, this),
      m_InternetRetrievalDialog(_weatherfax_pi, this),
      m_weatherfax_pi(_weatherfax_pi)
{
    UpdateMenuStates();

    wxString error;
    wxString coordinatesets_path = weatherfax_pi::StandardPath();
    wxString s = wxFileName::GetPathSeparator();
    wxString default_coordinatesets_path = *GetpSharedDataLocation() + _T("plugins")
        + s + _T("weatherfax") + s + _T("data") + _T("/");
    wxString coordinatesets = _T("WeatherFaxCoordinateSets.xml");
    
    TiXmlDocument doc;

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

                coord->mapping = WeatherFaxImageCoordinates::GetMapType
                    (wxString::FromUTF8(e->Attribute("Mapping")));
  
                coord->inputpole.x = AttributeInt(e, "InputPoleX", 0);
                coord->inputpole.y = AttributeInt(e, "InputPoleY", 0);
                coord->inputequator = AttributeDouble(e, "InputEquator", 0);

                coord->inputtrueratio = AttributeDouble(e, "InputTrueRatio", 1.0);
                coord->mappingmultiplier = AttributeDouble(e, "MappingMultiplier", 1.0);
                coord->mappingratio = AttributeDouble(e, "MappingRatio", 1.0);

                coord->Station = wxString::FromUTF8(e->Attribute("Station"));
                coord->Area = wxString::FromUTF8(e->Attribute("Area"));

                m_Coords.Append(coord);
            }
    }

    if(m_weatherfax_pi.m_bLoadSchedulesStart)
        m_SchedulesDialog.Load();

    return;
failed:
    wxMessageDialog mdlg(NULL, error, _("Weather Fax"), wxOK | wxICON_ERROR);
    mdlg.ShowModal();
}

WeatherFax::~WeatherFax()
{
    wxString coordinatesets_path = weatherfax_pi::StandardPath();
    wxString coordinatesets = _T("WeatherFaxCoordinateSets.xml");
    
    TiXmlDocument doc;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "utf-8", "" );
    doc.LinkEndChild( decl );

    TiXmlElement * root = new TiXmlElement( "WeatherFaxCoordinates" );
    doc.LinkEndChild( root );

    for(unsigned int i=0; i<m_Coords.GetCount(); i++) {
        TiXmlElement *c = new TiXmlElement( "Coordinate" );

        c->SetAttribute("Name", m_Coords[i]->name.mb_str());

        c->SetAttribute("X1", wxString::Format(_T("%d"), m_Coords[i]->p1.x).mb_str());
        c->SetAttribute("Y1", wxString::Format(_T("%d"), m_Coords[i]->p1.y).mb_str());
        c->SetAttribute("Lat1", wxString::Format(_T("%.5f"), m_Coords[i]->lat1).mb_str());
        c->SetAttribute("Lon1", wxString::Format(_T("%.5f"), m_Coords[i]->lon1).mb_str());

        c->SetAttribute("X2", wxString::Format(_T("%d"), m_Coords[i]->p2.x).mb_str());
        c->SetAttribute("Y2", wxString::Format(_T("%d"), m_Coords[i]->p2.y).mb_str());
        c->SetAttribute("Lat2", wxString::Format(_T("%.5f"), m_Coords[i]->lat2).mb_str());
        c->SetAttribute("Lon2", wxString::Format(_T("%.5f"), m_Coords[i]->lon2).mb_str());

        c->SetAttribute("Mapping", WeatherFaxImageCoordinates::MapName(m_Coords[i]->mapping).mb_str());

        if(m_Coords[i]->mapping != WeatherFaxImageCoordinates::MERCATOR) {
            if(m_Coords[i]->mapping != WeatherFaxImageCoordinates::FIXED_FLAT)
                c->SetAttribute("InputPoleX", wxString::Format
                                (_T("%d"), m_Coords[i]->inputpole.x).mb_str());
            c->SetAttribute("InputPoleY", wxString::Format
                            (_T("%d"), m_Coords[i]->inputpole.y).mb_str());
            c->SetAttribute("InputEquator", wxString::Format
                            (_T("%.5f"), m_Coords[i]->inputequator).mb_str());
            c->SetAttribute("InputTrueRatio", wxString::Format
                            (_T("%.4f"), m_Coords[i]->inputtrueratio).mb_str());

            if(!m_Coords[i]->Station.empty())
                c->SetAttribute("Station", m_Coords[i]->Station.mb_str());
            if(!m_Coords[i]->Area.empty())
                c->SetAttribute("Area", m_Coords[i]->Area.mb_str());

        }

        c->SetAttribute("MappingMultiplier", wxString::Format
                        (_T("%.4f"), m_Coords[i]->mappingmultiplier).mb_str());
        c->SetAttribute("MappingRatio", wxString::Format
                        (_T("%.4f"), m_Coords[i]->mappingratio).mb_str());

        root->LinkEndChild(c);
    }

    wxString filename = coordinatesets_path + coordinatesets;
    if(!doc.SaveFile(filename.mb_str())) {
        wxMessageDialog mdlg(this, _("Failed to save xml file: ") + filename,
                             _("Weather Fax"), wxOK | wxICON_ERROR);
        mdlg.ShowModal();
    }

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

    FaxDecoder decoder(*this,
                       m_weatherfax_pi.m_ImageWidth,
                       m_weatherfax_pi.m_BitsPerPixel,
                       m_weatherfax_pi.m_Carrier,
                       m_weatherfax_pi.m_Deviation,
                       (enum Bandwidth)m_weatherfax_pi.m_Filter,
                       m_weatherfax_pi.m_bSkipHeaderDetection,
                       m_weatherfax_pi.m_bIncludeHeadersInImage);
    
    if(!decoder.DecodeFaxFromFilename(filename))
        return;

    WeatherFaxImage *img = new WeatherFaxImage(wxNullImage, transparency, whitetransparency, invert);

    for(unsigned int i=0; i<m_Coords.GetCount(); i++)
        if(station.size() && m_Coords[i]->Station == station &&
           area.size() && m_Coords[i]->Area == area)
            img->m_Coords = m_Coords[i];

    WeatherFaxWizard wizard(*img, &decoder, *this, m_Coords,
                            station.size() && area.size() ? (station + _T(" - ") + area) : _T(""));
    
    if(wizard.RunWizard(wizard.m_pages[0])) {
        int selection = m_lFaxes->Append(area.size() ? (station + _T(" - ") + area) : filename );
        m_Faxes.push_back(img);
        
        wizard.StoreCoords();
        wizard.StoreMappingParams();

        m_lFaxes->SetSelection(selection);
                
        RequestRefresh( m_parent );
        UpdateMenuStates();
    } else
        delete img;
}

void WeatherFax::OpenImage(wxString filename)
{
    int transparency = m_sTransparency->GetValue();
    int whitetransparency = m_sWhiteTransparency->GetValue();
    bool invert = m_cInvert->GetValue();

    wxImage wimg;
    if(wimg.LoadFile(filename)) {
        WeatherFaxImage *img = new WeatherFaxImage(wimg, transparency, whitetransparency, invert);
        WeatherFaxWizard wizard(*img, NULL, *this, m_Coords, _T(""));
        
        if(wizard.RunWizard(wizard.m_pages[1])) {
            wxFileName filenamec(filename);

            int selection = m_lFaxes->Append(filenamec.GetFullName());
            m_Faxes.push_back(img);
            
            wizard.StoreCoords();
            wizard.StoreMappingParams();
            
            m_lFaxes->SetSelection(selection);

            RequestRefresh( m_parent );
            UpdateMenuStates();
        } else
            delete img;
    } else {
        wxMessageDialog w( this, _("Failed to load input file: ") + filename, _("Failed"),
                           wxOK | wxICON_ERROR );
        w.ShowModal();
    }
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
    WeatherFaxWizard wizard(image, NULL, *this, m_Coords, _T(""));
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
    int transparency = m_sTransparency->GetValue();
    int whitetransparency = m_sWhiteTransparency->GetValue();
    bool invert = m_cInvert->GetValue();

     FaxDecoder decoder(*this,
                        m_weatherfax_pi.m_ImageWidth,
                        m_weatherfax_pi.m_BitsPerPixel,
                        m_weatherfax_pi.m_Carrier,
                        m_weatherfax_pi.m_Deviation,
                        (enum Bandwidth)m_weatherfax_pi.m_Filter,
                        m_weatherfax_pi.m_bSkipHeaderDetection,
                        /*m_weatherfax_pi.m_bIncludeHeadersInImage*/true);

     decoder.SetSampleRate(8000);

     if(!decoder.DecodeFaxFromPortAudio() && !decoder.DecodeFaxFromDSP()) {
         wxMessageDialog w( this, _("Failed to set stuff up with portaudio or dsp\nCapture failed."),
                            _("Failure"), wxOK | wxICON_ERROR );
         w.ShowModal();
         return;
     }

     WeatherFaxImage *img = new WeatherFaxImage(wxNullImage, transparency, whitetransparency, invert);
     WeatherFaxWizard wizard(*img, &decoder, *this, m_Coords, _T(""));

    if(wizard.RunWizard(wizard.m_pages[0])) {
        static int capture_count;
        m_lFaxes->Append(_T("Capture - ") + wxString::Format(_T("%d"), capture_count++));
        m_Faxes.push_back(img);

        wizard.StoreCoords();
        wizard.StoreMappingParams();

        RequestRefresh( m_parent );
        UpdateMenuStates();
    } else
        delete img;
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
