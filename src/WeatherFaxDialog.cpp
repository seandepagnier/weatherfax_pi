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

#include "FaxDecoder.h"
#include "WeatherFaxDialog.h"

WeatherFaxDialog::~WeatherFaxDialog()
{
    for(unsigned int i=0; i<m_lFaxes->GetCount(); i++)
        delete (WeatherFaxImage*)m_lFaxes->GetClientData(i);
}

void WeatherFaxDialog::OnListBox( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection >= 0) {
        WeatherFaxImage *image = (WeatherFaxImage*)m_lFaxes->GetClientData(selection);
        if(image->valid_location)
            m_weatherfax_pi.SetImageToRender(image);
        else
            m_weatherfax_pi.SetImageToRender(NULL);
    } else
        m_weatherfax_pi.SetImageToRender(NULL);
}

void WeatherFaxDialog::NewFaxClicked( wxCommandEvent& event )
{
    wxFileDialog openDialog
        ( this, _( "Open Weather Fax Input File" ),
          m_weatherfax_pi.m_path, wxT ( "" ),
          _ ( "\
WAV files (*.wav)|*.WAV;*.wav|\
Supported Image Files|*.BMP;*.bmp;*.XBM;*.xbm;*.XPM;*.xpm;\
*.TIF;*.tif;*.GIF;*.gif;*.JPEG;*.jpeg;*.JPG;*.jpg;*.PNM;*.pnm;\
*.PCX;*.pcx;*.PICT;*.pict;*.TGA;*.tga|\
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
            decoder.DecodeFaxFromAudio(filename);
            FaxImageList images = decoder.images;
            int i=0;
            for(FaxImageList::iterator it = images.begin(); it !=  images.end(); it++) {
                WeatherFaxImage *img = new WeatherFaxImage(**it);
                m_lFaxes->Append(filenamec.GetFullName()+_("-")
                                 +wxString::Format(_T("%d"), ++i), img);
            }
        } else {
            wxImage wimg;
            if(wimg.LoadFile(filename)) {
                WeatherFaxImage *img = new WeatherFaxImage(wimg);
                int n = m_lFaxes->Append(filenamec.GetFullName(), img);
                m_lFaxes->SetSelection(n);
                EditFaxClicked(event);
            } else {
                wxMessageDialog w( this, _("Failed to load input file: ") + filename, _("Failed"),
                                   wxOK | wxICON_ERROR );
                w.ShowModal();
            }
        }
    }
}

void WeatherFaxDialog::EditFaxClicked( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0)
        return;
    EditFaxDialog dialog(*(WeatherFaxImage*)m_lFaxes->GetClientData(selection),
                         m_lFaxes->GetString(selection), *this);
    dialog.ShowModal();

    WeatherFaxImage *image = (WeatherFaxImage*)m_lFaxes->GetClientData(selection);
    if(image->valid_location)
        m_weatherfax_pi.SetImageToRender(image);
    else {
        wxMessageDialog w( this, _("Image edit failed (did you forget to set the coordinates?)"),
                           _("Failed"), wxOK | wxICON_ERROR );
        w.ShowModal();
//        m_lFaxes->Delete(selection);
    }
}

void WeatherFaxDialog::DeleteFaxClicked( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0)
        return;
    delete (WeatherFaxImage*)m_lFaxes->GetClientData();
    m_lFaxes->Delete(selection);
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::TransparencyChanged( wxScrollEvent& event )
{
    m_weatherfax_pi.SetTransparency(event.GetPosition());
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::OnInvert( wxCommandEvent& event )
{
    m_weatherfax_pi.m_bNeedUpdateImageDC = true;
    m_weatherfax_pi.m_bNeedUpdateImageGL = true;
    m_weatherfax_pi.SetInvert(event.IsChecked());
    RequestRefresh( m_parent );
}

void EditFaxDialog::BitmapClick( wxMouseEvent& event )
{
    switch(m_EditState) {
        case NONE:
        {
            wxMessageDialog w( this, _("Select an option to edit the image\n"),
                               _("Edit Image"), wxOK);
            w.ShowModal();
        } break;
        case COORD1:
        {
            m_img.p1 = m_swFaxArea->CalcUnscrolledPosition(event.GetPosition());
            wxTextEntryDialog *lat = new wxTextEntryDialog
                ( this, _("Enter Latitude (-90 to 90)"), _("Latitude") );

            if( lat->ShowModal() == wxID_OK ) {
                wxTextEntryDialog *lon = new wxTextEntryDialog
                    ( this, _("Enter Longitude (-180 to 180)"), _("Longitude") );
                double dlat, dlon;
                if( lon->ShowModal() == wxID_OK ) {
                    if(lat->GetValue().ToDouble(&dlat) && dlat >= -90 && dlat <= 90 &&
                       lon->GetValue().ToDouble(&dlon) && dlon >= -180 && dlon <= 180) {
                        m_img.lat1 = dlat;
                        m_img.lon1 = dlon;

                        wxMessageDialog w( this,
                                           _("Click the image to specify second lat lon coordinate\n"),
                                           _("Coord Image"), wxOK);
                        w.ShowModal();
                        m_EditState = COORD2;
                    } else {
                        wxMessageDialog w( this, _("Failed to convert input values to lat/lon"),
                                           _("Failed"), wxOK | wxICON_ERROR );
                        w.ShowModal();
                        m_EditState = NONE;
                    }
                } else
                    m_EditState = NONE;
            } else
                m_EditState = NONE;
        } break;
        case COORD2:
        {
            m_img.p2 = m_swFaxArea->CalcUnscrolledPosition(event.GetPosition());
            wxTextEntryDialog *lat = new wxTextEntryDialog
                ( this, _("Enter Latitude (-90 to 90)"), _("Latitude") );
            if( lat->ShowModal() == wxID_OK ) {
                wxTextEntryDialog *lon = new wxTextEntryDialog
                    ( this, _("Enter Longitude (-180 to 180)"), _("Longitude") );
                double dlat, dlon;
                if( lon->ShowModal() == wxID_OK ) {
                    if(lat->GetValue().ToDouble(&dlat) && dlat >= -90 && dlat <= 90 &&
                       lon->GetValue().ToDouble(&dlon) && dlon >= -180 && dlon <= 180) {
                        m_img.lat2 = dlat;
                        m_img.lon2 = dlon;
                        m_img.valid_location = true;
                    } else {
                        wxMessageDialog w( this, _("Failed to convert input values to lat/lon"),
                                           _("Failed"), wxOK | wxICON_ERROR );
                        w.ShowModal();
                    }
                }
            }
            m_EditState = NONE;
        } break;
        case SPLIT:
        {
            wxPoint split = m_swFaxArea->CalcUnscrolledPosition(event.GetPosition());
            wxRect r1(0, split.y, m_img.GetWidth(), m_img.GetHeight()-split.y);
            wxImage img2 = m_img.GetSubImage(r1);
            m_parent.m_lFaxes->Append(m_name+_("-")
                                      +wxString::Format(_T("%d"), m_splits++),
                                      new WeatherFaxImage(img2));

            wxRect r2(0, 0, m_img.GetWidth(), split.y);
            m_img = m_img.GetSubImage(r2);
            UpdateBitmap();
        } break;
    }
}

void EditFaxDialog::SetCoordinates( wxCommandEvent& event )
{
    wxMessageDialog w( this, _("Click the image to specify first lat lon coordinate\n"),
                       _("Coord Image"), wxOK);
    w.ShowModal();
    m_EditState = COORD1;
}

void EditFaxDialog::Split( wxCommandEvent& event )
{
    wxMessageDialog w( this, _("Click the image to split image vertically.\n"),
                       _("Split Image"), wxOK | wxCANCEL );
    if(w.ShowModal() == wxID_OK)
        m_EditState = SPLIT;
    else
        m_EditState = NONE;
}

void EditFaxDialog::UpdateBitmap()
{
    wxBitmap bmp(m_img);
    m_bFaxBitmap->SetBitmap(bmp);
}
