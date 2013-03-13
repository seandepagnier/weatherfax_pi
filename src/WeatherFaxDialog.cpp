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
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#include "weatherfax_pi.h"
#include "FaxDecoder.h"
#include "WeatherFaxImage.h"
#include "WeatherFaxDialog.h"
#include "WeatherFaxWizard.h"

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

WeatherFaxDialog::WeatherFaxDialog( weatherfax_pi &_weatherfax_pi, wxWindow* parent)
    : WeatherFaxDialogBase( parent ), m_weatherfax_pi(_weatherfax_pi)
{
    wxFileConfig *pConf = m_weatherfax_pi.m_pconfig;
    pConf->SetPath ( _T ( "/Settings/WeatherFax/CoordinateSets" ) );

    wxString names;
    pConf->Read ( _T ( "Names" ), &names, _T(""));

    /* split at each ; to get all the names in a list */
    std::list<wxString> namelist;
    while(names.size()) {
        namelist.push_back(names.BeforeFirst(';'));
        names = names.AfterFirst(';');
    }

    for(std::list<wxString>::iterator it = namelist.begin();
        it != namelist.end(); it++) {
        wxString name = *it;
#if defined (__WIN32__)
        /* replace any _ with spaces because windows is bad */
        name = ReplaceChar(name, '_', ' ');
#endif
        pConf->SetPath ( _T ( "/Settings/WeatherFax/CoordinateSets/" ) + *it );
        WeatherFaxImageCoordinates *newcoord = new WeatherFaxImageCoordinates(name);

        pConf->Read ( _T ( "Point1X" ), &newcoord->p1.x, 0);
        pConf->Read ( _T ( "Point1Y" ), &newcoord->p1.y, 0);
        pConf->Read ( _T ( "Point1Lat" ), &newcoord->lat1, 0);
        pConf->Read ( _T ( "Point1Lon" ), &newcoord->lon1, 0);
        pConf->Read ( _T ( "Point2X" ), &newcoord->p2.x, 0);
        pConf->Read ( _T ( "Point2Y" ), &newcoord->p2.y, 0);
        pConf->Read ( _T ( "Point2Lat" ), &newcoord->lat2, 0);
        pConf->Read ( _T ( "Point2Lon" ), &newcoord->lon2, 0);

        pConf->Read ( _T ( "mapping" ), (int*)&newcoord->mapping, 0);
        pConf->Read ( _T ( "inputpolex" ), &newcoord->inputpole.x, 0);
        pConf->Read ( _T ( "inputpoley" ), &newcoord->inputpole.y, 0);
        pConf->Read ( _T ( "inputequator" ), &newcoord->inputequator, 0);
        pConf->Read ( _T ( "inputtrueratio" ), &newcoord->inputtrueratio, 1.0);
        pConf->Read ( _T ( "mappingmultiplier" ), &newcoord->mappingmultiplier, 1.0);
        pConf->Read ( _T ( "mappingratio" ), &newcoord->mappingratio, 1.0);

        m_Coords.Append(newcoord);
    }
    UpdateButtonStates();
}

WeatherFaxDialog::~WeatherFaxDialog()
{
    wxFileConfig *pConf = m_weatherfax_pi.m_pconfig;

    /* remove from config all cordinate sets */
    pConf->SetPath ( _T ( "/Settings/WeatherFax" ) );
    pConf->DeleteGroup(_T("CoordinateSets"));

    pConf->SetPath ( _T ( "/Settings/WeatherFax/CoordinateSets" ) );
    wxString names;
    for(unsigned int i=0; i<m_Coords.GetCount(); i++) {
#if defined (__WIN32__)
        /* space does bad things on windows */
        m_Coords[i]->name = ReplaceChar(m_Coords[i]->name, ' ', '_');
#endif
        /* names are not allowed semicolon because we need it */
        m_Coords[i]->name = ReplaceChar(m_Coords[i]->name, ';', ',');
        names += m_Coords[i]->name + _(";");
    }
    
    pConf->Write ( _T ( "Names" ), names);

    for(unsigned int i=0; i<m_Coords.GetCount(); i++) {
        pConf->SetPath ( _T ( "/Settings/WeatherFax/CoordinateSets/" ) + m_Coords[i]->name );
        pConf->Write ( _T ( "Point1X" ), m_Coords[i]->p1.x);
        pConf->Write ( _T ( "Point1Y" ), m_Coords[i]->p1.y);
        pConf->Write ( _T ( "Point1Lat" ), m_Coords[i]->lat1);
        pConf->Write ( _T ( "Point1Lon" ), m_Coords[i]->lon1);
        pConf->Write ( _T ( "Point2X" ), m_Coords[i]->p2.x);
        pConf->Write ( _T ( "Point2Y" ), m_Coords[i]->p2.y);
        pConf->Write ( _T ( "Point2Lat" ), m_Coords[i]->lat2);
        pConf->Write ( _T ( "Point2Lon" ), m_Coords[i]->lon2);

        pConf->Write ( _T ( "mapping" ), (int)m_Coords[i]->mapping);
        pConf->Write ( _T ( "inputpolex" ), m_Coords[i]->inputpole.x);
        pConf->Write ( _T ( "inputpoley" ), m_Coords[i]->inputpole.y);
        pConf->Write ( _T ( "inputequator" ), m_Coords[i]->inputequator);
        pConf->Write ( _T ( "inputtrueratio" ), m_Coords[i]->inputtrueratio);
        pConf->Write ( _T ( "mappingmultiplier" ), m_Coords[i]->mappingmultiplier);
        pConf->Write ( _T ( "mappingratio" ), m_Coords[i]->mappingratio);

        delete m_Coords[i];
    }

    for(unsigned int i=0; i<m_Faxes.size(); i++)
        delete m_Faxes[i];
}

void WeatherFaxDialog::OnFaxes( wxCommandEvent& event )
{
    UpdateButtonStates();

    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &img = *m_Faxes[selection];
    m_sTransparency->SetValue(img.m_iTransparency);
    m_sWhiteTransparency->SetValue(img.m_iWhiteTransparency);
    m_cInvert->SetValue(img.m_bInvert);
}

void WeatherFaxDialog::OnFaxesToggled( wxCommandEvent& event )
{
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::CaptureFaxClicked( wxCommandEvent& event )
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

     if(!decoder.DecodeFaxFromDSP()) {
         wxMessageDialog w( this, _("Failed to set stuff up with dsp, this only works on linux"),
                            _("Failure"),
                            wxOK | wxICON_ERROR );
         w.ShowModal();
         return;
     }

     WeatherFaxImage *img = new WeatherFaxImage(wxNullImage, transparency, whitetransparency, invert);
     WeatherFaxWizard wizard(*img, &decoder, *this, m_Coords);

    if(wizard.RunWizard(wizard.m_pages[0])) {
        static int dspc;
        int selection = m_lFaxes->Append(_("dsp - ") + wxString::Format(_T("%d"), dspc));
        m_Faxes.push_back(img);

        wizard.StoreCoords();
        wizard.StoreMappingParams();

        m_lFaxes->Check(selection, true);
    } else
        delete img;

    UpdateButtonStates();
}

void WeatherFaxDialog::OpenFaxClicked( wxCommandEvent& event )
{
    int transparency = m_sTransparency->GetValue();
    int whitetransparency = m_sWhiteTransparency->GetValue();
    bool invert = m_cInvert->GetValue();

    wxFileDialog openDialog
        ( this, _( "Open Weather Fax Input File" ),
          m_weatherfax_pi.m_path, wxT ( "" ),
          _ ( "\
Supported Image Files|*.BMP;*.bmp;*.XBM;*.xbm;*.XPM;*.xpm;\
*.TIF;*.tif;*.TIFF;*.tiff;*.GIF;*.gif;*.JPEG;*.jpeg;*.JPG;*.jpg;*.PNM;*.pnm;*.PNG;*.png;\
*.PCX;*.pcx;*.PICT;*.pict;*.TGA;*.tga|\
WAV files (*.wav)|*.WAV;*.wav|\
All files (*.*)|*.*" ), wxFD_OPEN);

    if( openDialog.ShowModal() == wxID_OK ) {
        wxString filename = openDialog.GetPath();
        wxFileName filenamec(filename);
        m_weatherfax_pi.m_path = openDialog.GetDirectory();        
        if(filenamec.GetExt() == _("wav") || filenamec.GetExt() == _("WAV")) {
            FaxDecoder decoder(*this,
                               m_weatherfax_pi.m_ImageWidth,
                               m_weatherfax_pi.m_BitsPerPixel,
                               m_weatherfax_pi.m_Carrier,
                               m_weatherfax_pi.m_Deviation,
                               (enum Bandwidth)m_weatherfax_pi.m_Filter,
                               m_weatherfax_pi.m_bSkipHeaderDetection,
                               m_weatherfax_pi.m_bIncludeHeadersInImage);

            if(!decoder.DecodeFaxFromFilename(filename)) {
                wxMessageDialog w( this, _("Failed to load input file: ") + filename, _("Failed"),
                                   wxOK | wxICON_ERROR );
                w.ShowModal();
                return;
            }
            WeatherFaxImage *img = new WeatherFaxImage(wxNullImage, transparency, whitetransparency, invert);
            WeatherFaxWizard wizard(*img, &decoder, *this, m_Coords);

            if(wizard.RunWizard(wizard.m_pages[0])) {
                int selection = m_lFaxes->Append(filename);
                m_Faxes.push_back(img);

                wizard.StoreCoords();
                wizard.StoreMappingParams();
                
                m_lFaxes->Check(selection, true);
            } else
                delete img;

        } else {
            wxImage wimg;
            if(wimg.LoadFile(filename)) {
                WeatherFaxImage *img = new WeatherFaxImage(wimg, transparency, whitetransparency, invert);
                WeatherFaxWizard wizard(*img, NULL, *this, m_Coords);

                if(wizard.RunWizard(wizard.m_pages[0])) {
                    int selection = m_lFaxes->Append(filenamec.GetFullName());
                    m_Faxes.push_back(img);

                    wizard.StoreCoords();
                    wizard.StoreMappingParams();

                    m_lFaxes->SetSelection(selection);
                    m_lFaxes->Check(selection, true);
                } else
                    delete img;
            } else {
                wxMessageDialog w( this, _("Failed to load input file: ") + filename, _("Failed"),
                                   wxOK | wxICON_ERROR );
                w.ShowModal();
            }
        }
    }
    UpdateButtonStates();
}

void WeatherFaxDialog::EditFaxClicked( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &image = *m_Faxes[selection];
    WeatherFaxImage backupimage = image;
    WeatherFaxWizard wizard(image, NULL, *this, m_Coords);
    if(wizard.RunWizard(wizard.m_pages[0])) {
        wizard.StoreCoords();
        wizard.StoreMappingParams();

        m_lFaxes->Check(selection, true);
        image.FreeData();
    } else
        image = backupimage;

    RequestRefresh( m_parent );
}

void WeatherFaxDialog::DeleteFaxClicked( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    delete m_Faxes[selection];
    m_Faxes.erase(m_Faxes.begin() + selection);

    m_lFaxes->Delete(selection);
    UpdateButtonStates();

    RequestRefresh( m_parent );
}

void WeatherFaxDialog::TransparencyChanged( wxScrollEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &image = *m_Faxes[selection];
    image.m_iTransparency = event.GetPosition();
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::WhiteTransparencyChanged( wxScrollEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &image = *m_Faxes[selection];
    image.m_iWhiteTransparency = event.GetPosition();
    image.FreeData();
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::OnInvert( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size())
        return;

    WeatherFaxImage &image = *m_Faxes[selection];
    image.m_bInvert = event.IsChecked();
    image.FreeData();
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::UpdateButtonStates()
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0 || selection >= (int)m_Faxes.size()) {
        m_bEdit->Disable();
        m_bDelete->Disable();
    } else {
        m_bEdit->Enable();
        m_bDelete->Enable();
    }
}

void *DecoderThread::Entry() {
    m_decoder.DecodeFax();
    return NULL;
}
