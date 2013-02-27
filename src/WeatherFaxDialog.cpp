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

#include "FaxDecoder.h"
#include "WeatherFaxDialog.h"


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(WeatherFaxImageCoordinateList);

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

        m_Coords.Append(newcoord);
    }
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
        delete m_Coords[i];
    }

    for(unsigned int i=0; i<m_lFaxes->GetCount(); i++)
        delete (WeatherFaxImage*)m_lFaxes->GetClientData(i);
}

void WeatherFaxDialog::OnListBox( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection >= 0) {
        WeatherFaxImage *image = (WeatherFaxImage*)m_lFaxes->GetClientData(selection);
        m_weatherfax_pi.SetImageToRender(image);
    } else
        m_weatherfax_pi.SetImageToRender(NULL);
}

void WeatherFaxDialog::NewFaxClicked( wxCommandEvent& event )
{
    wxFileDialog openDialog
        ( this, _( "Open Weather Fax Input File" ),
          m_weatherfax_pi.m_path, wxT ( "" ),
          _ ( "\
Supported Image Files|*.BMP;*.bmp;*.XBM;*.xbm;*.XPM;*.xpm;\
*.TIF;*.tif;*.TIFF;*.tiff;*.GIF;*.gif;*.JPEG;*.jpeg;*.JPG;*.jpg;*.PNM;*.pnm;*.PNG;*.png;\
*.PCX;*.pcx;*.PICT;*.pict;*.TGA;*.tga|\
WAV files (*.wav)|*.WAV;*.wav|\
All files (*.*)|*.*" ), wxFD_OPEN);

#if 1
    if( openDialog.ShowModal() == wxID_OK ) {
        wxString filename = openDialog.GetPath();
#else
        if(1) { /* for debugging */
        wxString filename = _("/home/sean/pz1.png");
#endif
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
            decoder.DecodeFaxFromAudio(filename, this);
            FaxImageList images = decoder.images;
            int i=0, n = -1;
            for(FaxImageList::iterator it = images.begin(); it !=  images.end(); it++) {
                WeatherFaxImage *img = new WeatherFaxImage(**it);
                n = m_lFaxes->Append(filenamec.GetFullName()+_("-")
                                     +wxString::Format(_T("%d"), ++i), img);
            }
            if(n == -1) {
                wxMessageDialog w( this, _("Failed to decode input file: ") + filename, _("Failed"),
                                   wxOK | wxICON_ERROR );
                w.ShowModal();
            } else {
                m_lFaxes->SetSelection(n);
                EditFaxClicked(event);
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

    WeatherFaxImage &image = *(WeatherFaxImage*)m_lFaxes->GetClientData(selection);
    WeatherFaxImage backupimage = image;
    EditFaxWizard wizard(image, m_lFaxes->GetString(selection), *this, m_Coords);
    if(!wizard.RunWizard(wizard.m_pages[0])) {
        image = backupimage;
        return;
    }

    wizard.StoreCoords();
    wizard.StoreMappingParams();

    m_weatherfax_pi.SetImageToRender(&image);
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

void WeatherFaxDialog::WhiteTransparencyChanged( wxScrollEvent& event )
{
    m_weatherfax_pi.UpdateImage();
    m_weatherfax_pi.SetWhiteTransparency(event.GetPosition());
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::OnInvert( wxCommandEvent& event )
{
    m_weatherfax_pi.UpdateImage();
    m_weatherfax_pi.SetInvert(event.IsChecked());
    RequestRefresh( m_parent );
}

EditFaxWizard::EditFaxWizard( WeatherFaxImage &img, wxString name,
                              WeatherFaxDialog &parent,
                              WeatherFaxImageCoordinateList &coords)
    : EditFaxWizardBase( &parent ), m_parent(parent),
      m_wfimg(img), m_curCoords(img.m_Coords),
      m_name(name), m_Coords(coords)
{
    m_sPhasing->SetRange(0, m_wfimg.m_origimg.GetWidth()-1);
    m_sPhasing->SetValue(m_wfimg.phasing);
    m_sSkew->SetValue(m_wfimg.skew);
    m_cFilter->SetSelection(m_wfimg.filter);
    m_cRotation->SetSelection(m_wfimg.rotation);
    m_wfimg.MakePhasedImage();

    m_swFaxArea1->SetScrollbars(1, 1, m_wfimg.m_phasedimg.GetWidth(), m_wfimg.m_phasedimg.GetHeight()-1);

        /* make a new coord, yet making sure it has a unique name */
        wxString newcoordname;
        int cc = m_Coords.GetCount();
        for(int n=0, i=-1; i != cc; n++) {
            if(n)
                newcoordname = wxString::Format(_("New Coord %d"), n);
            else
                newcoordname = _("New Coord");
            if(!cc)
                break;
            for(i=0; i<cc; i++)
                if(m_Coords[i]->name == newcoordname)
                    break;
        }
        
        m_cbCoordSet->Append(newcoordname);
        int sel = 0;
        for(int i=0; i<cc; i++) {
            if(m_curCoords == m_Coords[i])
                sel = i+1;
            m_cbCoordSet->Append(m_Coords[i]->name);
        }

        m_cbCoordSet->SetSelection(sel);
        m_newCoords = new WeatherFaxImageCoordinates(newcoordname);
        m_newCoords->lat1 = m_newCoords->lat2 = m_newCoords->lon1 = m_newCoords->lon2 = 0;
}

EditFaxWizard::~EditFaxWizard()
{
    /* add coordinates to set if it is the new one, but make
       sure it has a unique name */
    int sel = m_cbCoordSet->GetSelection();
    if(sel == -1)
        sel = m_SelectedIndex;

    if(sel == 0 &&
       (m_newCoords->lat1 || m_newCoords->lat2 ||
        m_newCoords->lon1 || m_newCoords->lon2)) {
        int cc = m_Coords.GetCount();
        wxString newname = m_newCoords->name, newnumberedname;
        for(int n=0, i=-1; i != cc; n++) {
            newnumberedname = newname;
            if(n)
                newnumberedname += wxString::Format(_(" %d"), n);

            if(!cc)
                break;
            for(i=0; i<cc; i++)
                if(m_Coords[i]->name == newnumberedname)
                    break;
        }
        m_newCoords->name = newnumberedname;
        m_Coords.Append(m_newCoords);
    } else
        delete m_newCoords;
}

void EditFaxWizard::OnWizardPageChanged( wxWizardEvent& event )
{
    if(event.GetPage() == m_pages[1]) {
        m_cMapping->SetSelection(m_wfimg.mapping);
        UpdateMappingControls();

        m_sMappingValue1->SetValue(m_wfimg.inputpole.x);
        m_sMappingValue2->SetValue(m_wfimg.inputpole.y);
        m_sMappingValue3->SetValue(m_wfimg.inputequator);

        m_tMappingMultiplier->SetValue
            (wxString::Format(_("%.2f"), m_wfimg.mappingwidthmultiplier));

        ApplyMapping();
    } else if(event.GetPage() == m_pages[2]) {
        m_rbCoord1->SetValue(true);
        m_rbCoord2->SetValue(false);

        m_sCoord1X->SetRange(0, m_wfimg.m_mappedimg.GetWidth());
        m_sCoord2X->SetRange(0, m_wfimg.m_mappedimg.GetWidth());
        m_sCoord1Y->SetRange(0, m_wfimg.m_mappedimg.GetHeight());
        m_sCoord2Y->SetRange(0, m_wfimg.m_mappedimg.GetHeight());

        Refresh();
        
        SetCoords();

        m_swFaxArea3->SetScrollbars
            (1, 1, m_wfimg.m_mappedimg.GetWidth(), m_wfimg.m_mappedimg.GetHeight());
    }
}

void EditFaxWizard::UpdateMappingControls()
{
    switch((WeatherFaxImage::MapType)m_wfimg.mapping) {
    case WeatherFaxImage::MERCATOR: /* nomapping */
        m_stMappingLabel1->SetLabel(_("N/A       "));
        m_sMappingValue1->Disable();
        m_stMappingLabel2->SetLabel(_("N/A       "));
        m_sMappingValue2->Disable();
        m_stMappingLabel3->SetLabel(_("N/A           "));
        m_sMappingValue3->Disable();
        m_bGetMappingParameters->Disable();
        break;
    case WeatherFaxImage::POLAR:
        m_stMappingLabel1->SetLabel(_("pole X"));
        m_sMappingValue1->Enable();
        m_stMappingLabel2->SetLabel(_("pole Y"));
        m_sMappingValue2->Enable();
        m_stMappingLabel3->SetLabel(_("equator Y"));
        m_sMappingValue3->Enable();
        m_bGetMappingParameters->Enable();
        break;
    case WeatherFaxImage::FIXED_FLAT:
        m_stMappingLabel1->SetLabel(_("N/A       "));
        m_sMappingValue1->Disable();
        m_stMappingLabel2->SetLabel(_("pole Y"));
        m_sMappingValue2->Enable();
        m_stMappingLabel3->SetLabel(_("equator Y"));
        m_sMappingValue3->Enable();
        m_bGetMappingParameters->Enable();
        break;
    }
}

void EditFaxWizard::OnMappingChoice( wxCommandEvent& event )
{
    m_wfimg.mapping = (WeatherFaxImage::MapType)m_cMapping->GetSelection();
    UpdateMappingControls();
}

void EditFaxWizard::OnGetMappingParameters( wxCommandEvent& event )
{
    OnResetMapping( event );

    wxMessageDialog w( this, _("Click image along vertical meridian\n"),
                       _("Mapping"), wxOK  );
    w.ShowModal();
    mappingparamstate = 1;
}

void EditFaxWizard::OnBitmapClickPage2( wxMouseEvent& event )
{
    /* logic to determine input pole and equator */
    switch(mappingparamstate) {
    case 1:
    {
        wxTextEntryDialog pd( this, _("Enter Latitude (positive value)"), _("Latitude") );

        if( pd.ShowModal() == wxID_OK ) {
            mapping1 = m_swFaxArea2->CalcUnscrolledPosition(event.GetPosition());
            pd.GetValue().ToDouble(&mapping1lat);

            wxMessageDialog w
                ( this, _("Now Click image along same vertical meridian at a different latitude\n"),
                  _("Mapping"), wxOK  );
            w.ShowModal();
            mappingparamstate = 2;
        } else
            mappingparamstate = 0;
    } break;
    case 2:
    {
        wxTextEntryDialog pd( this, _("Enter Latitude"), _("Latitude") );
        if( pd.ShowModal() == wxID_OK ) {
            mapping2 = m_swFaxArea2->CalcUnscrolledPosition(event.GetPosition());
            pd.GetValue().ToDouble(&mapping2lat);

            /* average x values */
            m_sMappingValue1->SetValue((mapping1.x + mapping2.x) / 2);

            double pp1, pp2;
            switch((WeatherFaxImage::MapType)m_wfimg.mapping) {
            case WeatherFaxImage::POLAR:
                pp1 = tan((1 - mapping1lat / 90) * M_PI/4);
                pp2 = tan((1 - mapping2lat / 90) * M_PI/4);
                break;
            case WeatherFaxImage::FIXED_FLAT:
                pp1 = 1 - mapping1lat / 90;
                pp2 = 1 - mapping2lat / 90;
                break;
            default:
                wxMessageDialog w
                    ( this, _("Calculating Mapping not supported\n"),
                      _("Mapping"), wxOK | wxICON_WARNING );
                w.ShowModal();
                return;
            }

            /* best fit:
               pp1 = ((mapping1.y - inputpoley) / (inputequator - inputpoley))
               pp2 = ((mapping2.y - inputpoley) / (inputequator - inputpoley))

               solve for inputpoley and inputequator
            */
            double inputpoley = ( mapping2.y*pp1 - mapping1.y*pp2 ) / (pp1 - pp2);
            double inputequator = ( mapping2.y*pp1 - mapping1.y*pp2 - mapping2.y + mapping1.y )
                / (pp1 - pp2);

            m_sMappingValue2->SetValue(inputpoley);
            m_sMappingValue3->SetValue(inputequator);

            mappingparamstate = 2;
        } else
            mappingparamstate = 0;
    } break;
    }
}

void EditFaxWizard::OnBitmapClickPage3( wxMouseEvent& event )
{
    wxPoint p = m_swFaxArea3->CalcUnscrolledPosition(event.GetPosition());
        
    if(m_rbCoord1->GetValue()) {
        m_sCoord1X->SetValue(p.x);
        m_sCoord1Y->SetValue(p.y);
        m_rbCoord1->SetValue(false);
        m_rbCoord2->SetValue(true);
    } else if(m_rbCoord2->GetValue()) {
        m_sCoord2X->SetValue(p.x);
        m_sCoord2Y->SetValue(p.y);
        m_rbCoord2->SetValue(false);
        m_rbCoord1->SetValue(true);
    }
    
    Refresh();
}

void EditFaxWizard::OnCoordSet( wxCommandEvent& event )
{
    StoreCoords();
    SetCoords();
}

void EditFaxWizard::OnCoordText( wxCommandEvent& event )
{
    int index = m_cbCoordSet->GetSelection();
    if(index == -1)
        index = m_SelectedIndex;
    else if(index != m_SelectedIndex)
        return;
        
    m_cbCoordSet->SetString(index, m_cbCoordSet->GetValue());
    m_curCoords->name = m_cbCoordSet->GetValue();
}

void EditFaxWizard::OnRemoveCoords( wxCommandEvent& event )
{
    int selection = m_cbCoordSet->GetSelection();
    if(selection == 0) /* don't delete first item, button should be disabled anyway */
        return;

    m_cbCoordSet->Delete(selection);
    m_Coords.DeleteNode(m_Coords.Item(selection-1));
    m_cbCoordSet->SetSelection(selection-1);

    SetCoords();
}

void EditFaxWizard::StoreMappingParams()
{
    m_wfimg.inputpole.x = m_sMappingValue1->GetValue();
    m_wfimg.inputpole.y = m_sMappingValue2->GetValue();
    m_wfimg.inputequator = m_sMappingValue3->GetValue();

    double mappingmultiplier;
    m_tMappingMultiplier->GetValue().ToDouble(&mappingmultiplier);
    m_wfimg.mappingwidthmultiplier = mappingmultiplier;
    m_wfimg.mappingheightmultiplier = mappingmultiplier;
}

void EditFaxWizard::ApplyMapping()
{
    if(!m_wfimg.m_mappedimg.IsOk())
        m_wfimg.MakeMappedImage(this);

    m_swFaxArea2->SetScrollbars
        (1, 1, m_wfimg.m_mappedimg.GetWidth(), m_wfimg.m_mappedimg.GetHeight());

    Refresh();
}

void EditFaxWizard::OnApplyMapping( wxCommandEvent& event )
{
    StoreMappingParams();
    /* invalidate mapped image */
    m_wfimg.m_mappedimg = wxNullImage;
    ApplyMapping();
}

void EditFaxWizard::OnResetMapping( wxCommandEvent& event )
{
    m_wfimg.m_mappedimg = m_wfimg.m_phasedimg;
    m_swFaxArea2->SetScrollbars
        (1, 1, m_wfimg.m_mappedimg.GetWidth(), m_wfimg.m_mappedimg.GetHeight());
    Refresh();
}

void EditFaxWizard::UpdatePage1()
{
    m_wfimg.phasing = m_sPhasing->GetValue();
    m_wfimg.skew = m_sSkew->GetValue();
    m_wfimg.filter = m_cFilter->GetSelection();
    m_wfimg.rotation = m_cRotation->GetSelection();
    m_wfimg.MakePhasedImage();
    Refresh();
}

void EditFaxWizard::UpdatePage1( wxCommandEvent& event )
{
    UpdatePage1();
}

void EditFaxWizard::UpdatePage1( wxScrollEvent& event )
{
    UpdatePage1();
}

void EditFaxWizard::StoreCoords()
{
    m_curCoords->p1.x = m_sCoord1X->GetValue();
    m_curCoords->p1.y = m_sCoord1Y->GetValue();
    m_curCoords->lat1 = m_sCoord1Lat->GetValue();
    m_curCoords->lon1 = m_sCoord1Lon->GetValue();
    m_curCoords->p2.x = m_sCoord2X->GetValue();
    m_curCoords->p2.y = m_sCoord2Y->GetValue();
    m_curCoords->lat2 = m_sCoord2Lat->GetValue();
    m_curCoords->lon2 = m_sCoord2Lon->GetValue();
}

void EditFaxWizard::SetCoords()
{
    int index = m_cbCoordSet->GetSelection();
    if(index == -1 ) /* should never occur, but prevent crash */
        return;

    if(index == 0) {
        m_curCoords = m_newCoords;
        m_bRemoveCoordSet->Disable();
    } else {
        m_curCoords = m_Coords[index-1];
        m_bRemoveCoordSet->Enable();
    }
    m_SelectedIndex = index;

    int w = m_wfimg.m_mappedimg.GetWidth(), h = m_wfimg.m_mappedimg.GetHeight()-1;
    m_sCoord1X->SetRange(0, w);
    m_sCoord2X->SetRange(0, w);
    m_sCoord1Y->SetRange(0, h);
    m_sCoord2Y->SetRange(0, h);

    m_sCoord1X->SetValue(m_curCoords->p1.x);
    m_sCoord1Y->SetValue(m_curCoords->p1.y);
    m_sCoord1Lat->SetValue(m_curCoords->lat1);
    m_sCoord1Lon->SetValue(m_curCoords->lon1);

    m_sCoord2X->SetValue(m_curCoords->p2.x);
    m_sCoord2Y->SetValue(m_curCoords->p2.y);
    m_sCoord2Lat->SetValue(m_curCoords->lat2);
    m_sCoord2Lon->SetValue(m_curCoords->lon2);

    Refresh();
}

void EditFaxWizard::OnSpin( wxSpinEvent& event )
{
    Refresh();
}

void EditFaxWizard::OnPaintImage( wxPaintEvent& event)
{
    wxScrolledWindow *window = dynamic_cast<wxScrolledWindow*>(event.GetEventObject());
    if(!window)
        return;

    wxPaintDC dc( window );
    
    wxBitmap bmp(GetCurrentPage() == m_pages[0] ? m_wfimg.m_phasedimg : m_wfimg.m_mappedimg);

    int x, y;
    window->GetViewStart(&x, &y);

    int w, h;
    window->GetSize(&w, &h);

    if( bmp.IsOk() ) {

        wxMemoryDC mdc(bmp);
        dc.Blit( 0, 0, w, h, &mdc, x, y);
        mdc.SelectObject( wxNullBitmap );
    }

    int x1 = m_sCoord1X->GetValue(), y1 = m_sCoord1Y->GetValue();
    int x2 = m_sCoord2X->GetValue(), y2 = m_sCoord2Y->GetValue();

    if(GetCurrentPage() == m_pages[2]) {
        dc.SetBrush(wxBrush(*wxBLACK));

        dc.SetPen(wxPen( *wxRED, 3 ));
        dc.DrawLine(x1-x, 0, x1-x, h);
        dc.DrawLine(0, y1-y, w, y1-y);

        dc.SetPen(wxPen( *wxGREEN, 3 ));
        dc.DrawLine(x2-x, 0, x2-x, h);
        dc.DrawLine(0, y2-y, w, y2-y);
    }
}
