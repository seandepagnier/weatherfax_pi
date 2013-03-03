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

double square(double x) { return x*x; }

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

void WeatherFaxDialog::OnFaxes( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0)
        return;

    WeatherFaxImage &img = *(WeatherFaxImage*)m_lFaxes->GetClientData(selection);
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
         wxMessageDialog w( this, _("Failed to set stuff up with dsp, this only works on linux"), _("Failure"),
                            wxOK | wxICON_ERROR );
         w.ShowModal();
         return;
     }

     WeatherFaxImage *img = new WeatherFaxImage(wxNullImage, transparency, whitetransparency, invert);
     EditFaxWizard wizard(*img, &decoder, *this, m_Coords);

    if(wizard.RunWizard(wizard.m_pages[0])) {
        static int dspc;
        int selection = m_lFaxes->Append(_("dsp - ") + wxString::Format(_T("%d"), dspc), img);
        m_lFaxes->Check(selection, false);
    } else
        delete img;
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
            EditFaxWizard wizard(*img, &decoder, *this, m_Coords);

            if(wizard.RunWizard(wizard.m_pages[0])) {
                int selection = m_lFaxes->Append(filename, img);
                m_lFaxes->Check(selection, false);
            } else
                delete img;

        } else {
            wxImage wimg;
            if(wimg.LoadFile(filename)) {
                WeatherFaxImage *img = new WeatherFaxImage(wimg, transparency, whitetransparency, invert);
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
    EditFaxWizard wizard(image, NULL, *this, m_Coords);
    if(!wizard.RunWizard(wizard.m_pages[0])) {
        m_lFaxes->Check(selection, false);
        image = backupimage;
        return;
    }

    wizard.StoreCoords();
    wizard.StoreMappingParams();

    m_lFaxes->Check(selection, true);
    image.FreeData();
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::DeleteFaxClicked( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0)
        return;
    delete (WeatherFaxImage*)m_lFaxes->GetClientData(selection);
    m_lFaxes->Delete(selection);
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::TransparencyChanged( wxScrollEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0)
        return;
    WeatherFaxImage &image = *(WeatherFaxImage*)m_lFaxes->GetClientData(selection);
    image.m_iTransparency = event.GetPosition();
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::WhiteTransparencyChanged( wxScrollEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0)
        return;
    WeatherFaxImage &image = *(WeatherFaxImage*)m_lFaxes->GetClientData(selection);
    image.m_iWhiteTransparency = event.GetPosition();
    image.FreeData();
    RequestRefresh( m_parent );
}

void WeatherFaxDialog::OnInvert( wxCommandEvent& event )
{
    int selection = m_lFaxes->GetSelection();
    if(selection < 0)
        return;
    WeatherFaxImage &image = *(WeatherFaxImage*)m_lFaxes->GetClientData(selection);
    image.m_bInvert = event.IsChecked();
    image.FreeData();
    RequestRefresh( m_parent );
}

void *DecoderThread::Entry() {
    m_decoder.DecodeFax();
    return NULL;
}

EditFaxWizard::EditFaxWizard( WeatherFaxImage &img, FaxDecoder *decoder,
                              WeatherFaxDialog &parent,
                              WeatherFaxImageCoordinateList &coords)
    : EditFaxWizardBase( &parent ), m_decoder(decoder), m_parent(parent),
      m_wfimg(img), m_curCoords(img.m_Coords),
      m_Coords(coords)
{
    m_sPhasing->SetRange(0, m_wfimg.m_origimg.GetWidth()-1);
    m_sPhasing->SetValue(m_wfimg.phasing);
    m_sSkew->SetValue(m_wfimg.skew);
    m_cFilter->SetSelection(m_wfimg.filter);
    m_cRotation->SetSelection(m_wfimg.rotation);
    m_wfimg.MakePhasedImage();

    m_swFaxArea1->SetScrollbars(1, 1, m_wfimg.m_phasedimg.GetWidth(), m_wfimg.m_phasedimg.GetHeight());

    MakeNewCoordinates();

    if(m_decoder) {
        /* periodically check for updates */
        m_tDecoder.Connect(wxEVT_TIMER, wxTimerEventHandler( EditFaxWizard::OnDecoderTimer ), NULL, this);
        m_tDecoder.Start(500, wxTIMER_ONE_SHOT);

        /* run decoder in a separate thread */
        m_thDecoder = new DecoderThread(*m_decoder);
        m_thDecoder->Run();
    } else {
        m_thDecoder = NULL;
        m_bStopDecoding->Disable();
        m_sMinusSaturationThreshold->Disable();
    }
}

EditFaxWizard::~EditFaxWizard()
{
    if(m_decoder) {
        m_decoder->m_bEndDecoding = true;
        m_thDecoder->Wait(); /* wait for decoder thread to end */
        delete m_thDecoder;
    }

    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T ( "/Settings/WeatherFax" ) );

    wxPoint p = GetPosition();
    pConf->Write ( _T ( "WizardX" ), p.x);
    pConf->Write ( _T ( "WizardY" ), p.y);

    wxSize s = GetSize();
    pConf->Write ( _T ( "WizardW" ), s.x);
    pConf->Write ( _T ( "WizardH" ), s.y);

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

void EditFaxWizard::MakeNewCoordinates()
{
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


void EditFaxWizard::OnDecoderTimer( wxTimerEvent & )
{
    if(m_decoder->m_DecoderMutex.Lock() == wxMUTEX_NO_ERROR) {
        m_decoder->minus_saturation_threshold =
            -(1 + (double)m_sMinusSaturationThreshold->GetValue()/10);

        if(m_decoder->imageline > m_wfimg.m_origimg.GetHeight()) {
            int w = m_decoder->m_imagewidth, h = m_decoder->imageline;
            m_wfimg.m_origimg = wxImage( w, h );
            memcpy(m_wfimg.m_origimg.GetData(), m_decoder->imgdata, w*h*3);

            int oldh = m_wfimg.m_phasedimg.GetHeight();

            m_wfimg.MakePhasedImage();

            int pw = m_wfimg.m_phasedimg.GetWidth(), ph = m_wfimg.m_phasedimg.GetHeight();

            int x, y;
            m_swFaxArea1->GetViewStart(&x, &y);
            int sw, sh;
            m_swFaxArea1->GetSize(&sw, &sh);
            if(y >= oldh - sh)
                y = ph;
            if(y < 0)
                y = 0;
            m_swFaxArea1->SetScrollbars(1, 1, pw, ph, x, y);
            m_swFaxArea1->Refresh();
        }
        m_decoder->m_DecoderMutex.Unlock();
        m_bPhasingArea->Refresh();
    }
    m_tDecoder.Start(1000, wxTIMER_ONE_SHOT);
}

void EditFaxWizard::OnStopDecoding( wxCommandEvent& event )
{
    m_decoder->m_bEndDecoding = true;
}

void EditFaxWizard::OnPaintPhasing( wxPaintEvent& event )
{
    if(!m_decoder)
        return;

    wxWindow *window = dynamic_cast<wxWindow*>(event.GetEventObject());
    if(!window)
        return;

    wxPaintDC dc( window );

    dc.SetBrush(wxBrush(*wxBLACK));
    dc.SetPen(wxPen( *wxBLACK, 1 ));

    int blocksize = m_decoder->blocksize;
    int w, h;
    int s = 4;
    window->GetSize(&w, &h);
    for(int x = 0; x<w; x++) {
        int i = x * blocksize / w;
        int y = h*((m_decoder->line ? m_decoder->datadouble[i] : 0) +(s/2))/s;
        dc.DrawLine(x, h/2, x, y);
    }

    int p;
    dc.SetPen(wxPen( *wxRED, 1 ));
    p = h*(1+(s/2))/s;
    dc.DrawLine(0, p, w, p);
    p = h*(-1+(s/2))/s;
    dc.DrawLine(0, p, w, p);
    
    dc.SetPen(wxPen( *wxGREEN, 1 ));
    p = h*(m_decoder->minus_saturation_threshold+(s/2))/s;
    dc.DrawLine(0, p, w, p);
}

void EditFaxWizard::OnWizardPageChanged( wxWizardEvent& event )
{
    if(event.GetPage() == m_pages[1]) {
        m_tDecoder.Stop();
        m_thDecoder->Kill();

        ResetMapping();
        m_cMapping->SetSelection(m_wfimg.mapping);
        UpdateMappingControls();

        m_sMappingPoleX->SetValue(m_wfimg.inputpole.x);
        m_sMappingPoleY->SetValue(m_wfimg.inputpole.y);
        m_sMappingEquatorY->SetValue(m_wfimg.inputequator);
        m_tTrueRatio->SetValue(wxString::Format(_T("%f"), m_wfimg.inputtrueratio));

        m_tMappingMultiplier->SetValue
            (wxString::Format(_("%.2f"), m_wfimg.mappingmultiplier));

        m_tMappingRatio->SetValue
            (wxString::Format(_("%.2f"), m_wfimg.mappingratio));

        m_rbCoord1->SetValue(true);
        m_rbCoord2->SetValue(false);

        m_sCoord1X->SetRange(0, m_wfimg.m_mappedimg.GetWidth());
        m_sCoord2X->SetRange(0, m_wfimg.m_mappedimg.GetWidth());
        m_sCoord1Y->SetRange(0, m_wfimg.m_mappedimg.GetHeight());
        m_sCoord2Y->SetRange(0, m_wfimg.m_mappedimg.GetHeight());

        SetCoords();
    }
}

void EditFaxWizard::OnSetSizes( wxInitDialogEvent& event )
{
    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T ( "/Settings/WeatherFax" ) );

    wxPoint p = GetPosition();
    pConf->Read ( _T ( "WizardX" ), &p.x, p.x);
    pConf->Read ( _T ( "WizardY" ), &p.y, p.y);
    SetPosition(p);

    wxSize s = wxSize(640, 480);
    pConf->Read ( _T ( "WizardW" ), &s.x, s.x);
    pConf->Read ( _T ( "WizardH" ), &s.y, s.y);
    SetSize(s);
}

void EditFaxWizard::UpdateMappingControls()
{
    switch((WeatherFaxImage::MapType)m_wfimg.mapping) {
    case WeatherFaxImage::MERCATOR: /* nomapping */
        m_sMappingPoleX->Disable();
        m_sMappingPoleY->Disable();
        m_sMappingEquatorY->Disable();
        m_tTrueRatio->Disable();
        m_bGetMappingParameters->Disable();
        break;
    case WeatherFaxImage::POLAR:
        m_sMappingPoleX->Enable();
        m_sMappingPoleY->Enable();
        m_sMappingEquatorY->Enable();
        m_tTrueRatio->Enable();
        m_bGetMappingParameters->Enable();
        break;
    case WeatherFaxImage::FIXED_FLAT:
        m_sMappingPoleX->Disable();
        m_sMappingPoleY->Enable();
        m_sMappingEquatorY->Enable();
        m_tTrueRatio->Disable();
        m_bGetMappingParameters->Enable();
        break;
    }
}

void EditFaxWizard::OnMappingChoice( wxCommandEvent& event )
{
    m_wfimg.mapping = (WeatherFaxImage::MapType)m_cMapping->GetSelection();
    UpdateMappingControls();
}

void EditFaxWizard::GetMappingParametersFixedFlat()
{
    double mapping1lat = m_sCoord1Lat->GetValue();
    double mapping2lat = m_sCoord2Lat->GetValue();

    double pp1 = 1 - mapping1lat / 90;
    double pp2 = 1 - mapping2lat / 90;

    double mapping1y = m_sCoord1Y->GetValue(),  mapping2y = m_sCoord2Y->GetValue();

    /* 
       pp1 = (mapping1y - inputpoley)/(inputequator - inputpoley)
       pp2 = (mapping2y - inputpoley)/(inputequator - inputpoley)

       inputpoley = (mapping2y*pp1-mapping1y*pp2)/(pp1-pp2)
       inputequator = (mapping1y*(1-pp2)+mapping2y*pp1-mapping2y)/(pp1-pp2)
    */

    double q = pp1-pp2;

    double inputpoley = (mapping2y*pp1 - mapping1y*pp2) / q;
    double inputequator = inputpoley + (mapping1y - mapping2y) / q;

    m_sMappingPoleY->SetValue(inputpoley);
    m_sMappingEquatorY->SetValue(inputequator);
}

void EditFaxWizard::GetMappingParametersPolar()
{
    double mapping1x = m_sCoord1X->GetValue(), mapping1y = m_sCoord1Y->GetValue();
    double mapping2x = m_sCoord2X->GetValue(), mapping2y = m_sCoord2Y->GetValue();

    double mapping1lat = m_sCoord1Lat->GetValue();
    double mapping2lat = m_sCoord2Lat->GetValue();
    double mapping1lon = m_sCoord1Lon->GetValue();
    double mapping2lon = m_sCoord2Lon->GetValue();

    bool south = false;

    if(mapping1lat * mapping2lat < 0) {
        wxMessageDialog w
            ( this, _("Warning, latitudes on different sides of equator \
not recommended because of ambiguity of north or south pole"),
              _("Mapping"), wxOK | wxICON_WARNING );
        w.ShowModal();
        /* use south if closest point to a pole is south */
        if((fabs(mapping1lat) < fabs(mapping2lat) && mapping1lat < 0) ||
           (fabs(mapping2lat) < fabs(mapping1lat) && mapping2lat < 0)) {
            south = true;
            mapping1lat = -mapping1lat;
            mapping2lat = -mapping2lat;
        }
    } else {
        /* use south pole */
        if(mapping1lat < 0) {
            south = true;
            mapping1lat = -mapping1lat;
            mapping2lat = -mapping2lat;
        }
    }

    double pp1 = 1 - mapping1lat / 90;
    double pp2 = 1 - mapping2lat / 90;

    /* for polar stereograph */
    pp1 = tan(pp1 * M_PI/4);
    pp2 = tan(pp2 * M_PI/4);

    /* 
       pp1 = sqrt((mapping1x - inputpole.x)^2 + (mapping1y - inputpole.y)^2)/(inputequator - inputpole.y)
       pp2 = sqrt((mapping2x - inputpole.x)^2 + (mapping2y - inputpole.y)^2)/(inputequator - inputpole.y)

       inputpole.x = mapping1x

       mapping2lon - mapping1lon = atan2((mapping2x - inputpole.x)*inputtrueratio,
       mapping2y - inputpole.y)

       (pp1*(inputequator - inputpoley))^2 = ((mapping1x - mapping1x)^2 + (mapping1y - inputpoley)^2)
       (pp2*(inputequator - inputpoley))^2 = ((mapping2x - mapping1x)^2 + (mapping2y - inputpoley)^2)
       d = sqrt(((mapping2x - inputpole.x)*inputtrueratio)^2 + (mapping2y - inputpole.y)^2)
       (mapping2x - mapping1x)*inputtrueratio = d*sin(mapping2lon - mapping1lon)
       mapping2y - inputpoley = d*cos(mapping2lon - mapping1lon)

       sm2lm1l = sin(mapping2lon - mapping1lon)
       cm2lm1l = cos(mapping2lon - mapping1lon)

       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = ((mapping2x - mapping1x)^2 + (mapping2y - inputpoley)^2)
       d = sqrt(((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)
       (mapping2x - mapping1x)*inputtrueratio = d*sm2lm1l
       mapping2y - inputpoley = d*cm2lm1l

       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = ((mapping2x - mapping1x)^2 + (mapping2y - inputpoley)^2)
       ((mapping2x - mapping1x)*inputtrueratio)^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)*sm2lm1l^2
       (mapping2y - inputpoley)^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)*cm2lm1l^2


       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = ((mapping2x - mapping1x)^2 + (mapping2y - inputpoley)^2)
       (mapping2y - inputpoley)^2 = ((((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)*sm2lm1l^2 + (mapping2y - inputpoley)^2)*cm2lm1l^2

       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = ((mapping2x - mapping1x)^2 + (mapping2y - inputpoley)^2)
       ((mapping2y - inputpoley)^2*(1 - sm2lm1l^2*cm2lm1l^2 - cm2lm1l^2)) / (sm2lm1l^2*cm2lm1l^2)  = ((mapping2x - mapping1x)*inputtrueratio)^2


       inputtrueratio = sqrt(((mapping2y - inputpoley)^2*(1 - sm2lm1l^2*cm2lm1l^2 - cm2lm1l^2)) / (sm2lm1l^2*cm2lm1l^2)) / (mapping2x - mapping1x)

       // inputtrueratio then inputequator theninputpoley
       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = ((mapping2x - mapping1x)^2 + (mapping2y - inputpoley)^2)
       inputtrueratio = sqrt(1 - sm2lm1l^2*cm2lm1l^2 - cm2lm1l^2) * (mapping2y - inputpoley) / ((mapping2x - mapping1x) * sm2lm1l * cm2lm1l)

       inputequator = (mapping1y - inputpoley) / pp1 + inputpoley
       (pp2*(inputequator - inputpoley))^2 = ((mapping2x - mapping1x)^2 + (mapping2y - inputpoley)^2)

       (pp2*((mapping1y - inputpoley) / pp1))^2 = ((mapping2x - mapping1x)^2 + (mapping2y - inputpoley)^2)

       mapping1y^2*pp2^2/pp1^2 - 2*inputpoley*mapping1y*pp2^2/pp1^2 + inputpoley^2*pp2^2/pp1^2
       = mapping2y^2 - 2*inputpoley*mapping2y + mapping2x^2 - 2*mapping1x*mapping2x + mapping1x^2 + inputpoley^2

       inputpoley^2*(pp2^2/pp1^2 - 1) + inputpoley*2*(mapping2y - mapping1y*pp2^2/pp1^2)
       = mapping2y^2 - mapping1y^2*pp2^2/pp1^2 + (mapping2x - mapping1x)^2

       a = pp2^2/pp1^2 - 1
       b = 2*(mapping2y - mapping1y*pp2^2/pp1^2)
       c = mapping1y^2*pp2^2/pp1^2 - mapping2y^2 - (mapping2x - mapping1x)^2
             
       inputpoley = (-b - sqrt(b^2 - 4*a*c)) / (2*a)

    */

    double inputpolex = mapping1x;

    double q = pp2*pp2/(pp1*pp1);
    double a = q - 1;
    double b = 2*(mapping2y - mapping1y*q);
    double c = square(mapping1y)*q - square(mapping2y) - square(mapping2x - mapping1x);
    double inputpoley1 = (-b + sqrt(b*b - 4*a*c)) / (2*a);
    double inputpoley2 = (-b - sqrt(b*b - 4*a*c)) / (2*a);

    /* pick correct root */
//    double inputpoley = mapping1y > mapping2y ? inputpoley1 : inputpoley2;
    double inputpoley = (south ? pp1 < pp2 : pp1 > pp2) ? inputpoley1 : inputpoley2;

    double inputequator = (mapping1y - inputpoley) / pp1 + inputpoley;

    double headingdiff = mapping2lon - mapping1lon;
    if(headingdiff >= 180) headingdiff -= 360;
    if(headingdiff <= -180) headingdiff += 360;
    double sm2lm1l = sin(M_PI/180*headingdiff);
    double cm2lm1l = cos(M_PI/180*headingdiff);

    double inputtrueratio = sqrt(1 - square(sm2lm1l*cm2lm1l) - square(cm2lm1l))
        * (mapping2y - inputpoley) / ((mapping2x - mapping1x) * sm2lm1l * cm2lm1l);

    if(south) /* take negative root for south */
        inputtrueratio = -inputtrueratio;

    m_sMappingPoleX->SetValue(inputpolex);
    m_sMappingPoleY->SetValue(inputpoley);
    m_sMappingEquatorY->SetValue(inputequator);
    m_tTrueRatio->SetValue(wxString::Format(_T("%.3f"), inputtrueratio));
}

void EditFaxWizard::OnGetMappingParameters( wxCommandEvent& event )
{
    switch((WeatherFaxImage::MapType)m_wfimg.mapping) {
    case WeatherFaxImage::POLAR: GetMappingParametersPolar(); break;
    case WeatherFaxImage::FIXED_FLAT: GetMappingParametersFixedFlat(); break;
    default:
        wxMessageDialog w
            ( this, _("Calculating Mapping not supported for map type\n"),
              _("Mapping"), wxOK | wxICON_WARNING );
        w.ShowModal();
        return;
    }
    Refresh();
}

void EditFaxWizard::OnBitmapClickPage2( wxMouseEvent& event )
{
    wxPoint p = m_swFaxArea2->CalcUnscrolledPosition(event.GetPosition());
        
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
    m_wfimg.inputpole.x = m_sMappingPoleX->GetValue();
    m_wfimg.inputpole.y = m_sMappingPoleY->GetValue();
    m_wfimg.inputequator = m_sMappingEquatorY->GetValue();
    double inputtrueratio;
    m_tTrueRatio->GetValue().ToDouble(&inputtrueratio);
    m_wfimg.inputtrueratio = inputtrueratio;

    double mappingmultiplier;
    m_tMappingMultiplier->GetValue().ToDouble(&mappingmultiplier);
    m_wfimg.mappingmultiplier = mappingmultiplier;

    double mappingratio;
    m_tMappingRatio->GetValue().ToDouble(&mappingratio);
    m_wfimg.mappingratio = mappingratio;
}

void EditFaxWizard::ResetMapping()
{
    m_wfimg.m_mappedimg = m_wfimg.m_phasedimg;

    double x1 = m_sCoord1X->GetValue(), y1 = m_sCoord1Y->GetValue(), mx1, my1;
    double x2 = m_sCoord2X->GetValue(), y2 = m_sCoord2Y->GetValue(), mx2, my2;

    SetCoordRanges();

    if(m_bmappingapplied) {
        m_wfimg.MercatorToInput(x1, y1, mx1, my1);
        m_sCoord1X->SetValue(mx1);
        m_sCoord1Y->SetValue(my1);

        m_wfimg.MercatorToInput(x2, y2, mx2, my2);
        m_sCoord2X->SetValue(mx2);
        m_sCoord2Y->SetValue(my2);

        m_bmappingapplied = false;
    }
    
    Refresh();
}

bool EditFaxWizard::ApplyMapping()
{
    if(!m_wfimg.m_mappedimg.IsOk())
        if(!m_wfimg.MakeMappedImage(this))
            return false;

    double x1 = m_sCoord1X->GetValue(), y1 = m_sCoord1Y->GetValue(), mx1, my1;
    double x2 = m_sCoord2X->GetValue(), y2 = m_sCoord2Y->GetValue(), mx2, my2;
 
    SetCoordRanges();

    m_wfimg.InputToMercator(x1, y1, mx1, my1);
    m_sCoord1X->SetValue(mx1);
    m_sCoord1Y->SetValue(my1);
    
    m_wfimg.InputToMercator(x2, y2, mx2, my2);
    m_sCoord2X->SetValue(mx2);
    m_sCoord2Y->SetValue(my2);
    
    Refresh();

    m_bmappingapplied = true;
    return true;
}

void EditFaxWizard::SetCoordRanges()
{
    int w = m_wfimg.m_mappedimg.GetWidth(), h = m_wfimg.m_mappedimg.GetHeight()-1;
    m_sCoord1X->SetRange(0, w);
    m_sCoord2X->SetRange(0, w);
    m_sCoord1Y->SetRange(0, h);
    m_sCoord2Y->SetRange(0, h);

    m_swFaxArea2->SetScrollbars
        (1, 1, m_wfimg.m_mappedimg.GetWidth(), m_wfimg.m_mappedimg.GetHeight());
}

void EditFaxWizard::OnApplyMapping( wxCommandEvent& event )
{
    if(m_bmappingapplied) {
        ResetMapping();

        m_cMapping->Enable();
        m_tMappingMultiplier->Enable();
        m_tMappingRatio->Enable();
        m_sMappingPoleX->Enable();
        m_sMappingPoleY->Enable();
        m_sMappingEquatorY->Enable();
        m_tTrueRatio->Enable();
        m_bGetMappingParameters->Enable();

        m_bApplyMapping->SetLabel(_T("Apply"));
    } else {
        StoreMappingParams();
        /* invalidate mapped image */
        m_wfimg.m_mappedimg = wxNullImage;
        if(!ApplyMapping())
            ResetMapping();
        else {
            m_cMapping->Disable();
            m_tMappingMultiplier->Disable();
            m_tMappingRatio->Disable();
            m_sMappingPoleX->Disable();
            m_sMappingPoleY->Disable();
            m_sMappingEquatorY->Disable();
            m_tTrueRatio->Disable();
            m_bGetMappingParameters->Disable();

            m_bApplyMapping->SetLabel(_T("Reset"));
        }
    }
}

void EditFaxWizard::OnInformation( wxCommandEvent& event )
{
    wxMessageDialog w( this, _("\
Mapping allows for scaling, and conversion between coordinate systems\n\
Latitudes are +N -S, Longitudes +E -W\n\
\n\
For polar mapping mode:\n\
Get Mapping parameters calculates the mapping from\n\
the coordinates given, but the first coordinate's X\n\
value must align with the pole in polar mode\n\
(on the vertical meridian)\n\
Once the coordinates are found, the blue lines should\n\
align with the latitudes if not, the mapping parameters\n\
can be tweaked until they do.\n\n\
It is also possible to set the coordinates, get the\n\
mapping parameters, apply, then adjust the coordinates\n\
reset, and get the mapping again, and apply again.\
"), _("Information"),
                       wxOK | wxICON_INFORMATION );
    w.ShowModal();
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

    double x1 = m_curCoords->p1.x, y1 = m_curCoords->p1.y;
    if(m_bmappingapplied) {
        m_sCoord1X->SetValue(x1);
        m_sCoord1Y->SetValue(y1);
    } else {
        double mx1, my1;
        m_wfimg.MercatorToInput(x1, y1, mx1, my1);
        m_sCoord1X->SetValue(mx1);
        m_sCoord1Y->SetValue(my1);
    }

    m_sCoord1Lat->SetValue(m_curCoords->lat1);
    m_sCoord1Lon->SetValue(m_curCoords->lon1);

    double x2 = m_curCoords->p2.x, y2 = m_curCoords->p2.y;
    if(m_bmappingapplied) {
        m_sCoord2X->SetValue(x2);
        m_sCoord2Y->SetValue(y2);
    } else {
        double mx2, my2;
        m_wfimg.MercatorToInput(x2, y2, mx2, my2);
        m_sCoord2X->SetValue(mx2);
        m_sCoord2Y->SetValue(my2);
    }

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

    if(GetCurrentPage() == m_pages[1]) {
        int x1 = m_sCoord1X->GetValue(), y1 = m_sCoord1Y->GetValue();
        int x2 = m_sCoord2X->GetValue(), y2 = m_sCoord2Y->GetValue();

        dc.SetBrush(wxBrush(*wxBLACK));

        dc.SetPen(wxPen( *wxRED, 1 ));
        dc.DrawLine(x1-x, 0, x1-x, h);
        dc.DrawLine(0, y1-y, w, y1-y);

        dc.SetPen(wxPen( *wxGREEN, 1 ));
        dc.DrawLine(x2-x, 0, x2-x, h);
        dc.DrawLine(0, y2-y, w, y2-y);

        if(!m_bmappingapplied && m_wfimg.mapping == WeatherFaxImage::POLAR) {

            int inputpolex = m_sMappingPoleX->GetValue();
            int inputpoley = m_sMappingPoleY->GetValue();
            double inputtrueratio;
            m_tTrueRatio->GetValue().ToDouble(&inputtrueratio);
            m_wfimg.inputtrueratio = inputtrueratio;

            dc.SetPen(wxPen( *wxBLUE, 1 ));
            dc.SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));

            double d;

            d = y1 - inputpoley;
            dc.DrawEllipse(inputpolex - x - d/inputtrueratio,
                           inputpoley - y - d, 2*d/inputtrueratio, 2*d);

            d = hypot((x2 - inputpolex)*inputtrueratio, y2 - inputpoley);
            dc.DrawEllipse(inputpolex - x - d/inputtrueratio,
                           inputpoley - y - d, 2*d/inputtrueratio, 2*d);
        }

    }
}
