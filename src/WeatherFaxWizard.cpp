/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Sean D'Epagnier                                 *
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

#include <wx/fileconf.h>

#include "weatherfax_pi.h"
#include "FaxDecoder.h"
#include "WeatherFaxImage.h"
#include "WeatherFax.h"
#include "DecoderOptionsDialog.h"
#include "WeatherFaxWizard.h"
#include "icons.h"

WeatherFaxWizard::WeatherFaxWizard( WeatherFaxImage &img,
                                    bool use_decoder, wxString decoder_filename,
                                    WeatherFax &parent,
                                    WeatherFaxImageCoordinateList *coords,
                                    wxString newcoordbasename)
    : WeatherFaxWizardBase( &parent ), m_decoder(*this, decoder_filename),
      m_DecoderOptionsDialog(use_decoder ? new DecoderOptionsDialog(*this) : NULL),
      m_parent(parent), m_wfimg(img), m_curCoords(img.m_Coords),
      m_NewCoordBaseName(newcoordbasename.empty() ? wxString(_("New Coord")) : newcoordbasename),
      m_Coords(coords ? *coords : m_BuiltinCoords)
{

    wxIcon icon;
    icon.CopyFromBitmap(*_img_weatherfax);
    SetIcon(icon);

    m_sPhasing->SetValue(m_wfimg.phasing);
    m_sSkew->SetValue(m_wfimg.skew);
    m_cFilter->SetSelection(m_wfimg.filter);

    /* reset image */
    m_wfimg.m_origimg.Create(1, 1); /* small image; so orig image is always ok to work with */
    
    MakeNewCoordinates();

    m_wfimg.MakePhasedImage();

    m_sPhasing->SetRange(0, m_wfimg.m_phasedimg.GetWidth()-1);
    m_swFaxArea1->SetScrollbars(1, 1, m_wfimg.m_phasedimg.GetWidth(), m_wfimg.m_phasedimg.GetHeight());
    
    m_cRotation->SetSelection(m_curCoords->rotation);

    if(use_decoder && m_decoder.m_inputtype != FaxDecoder::NONE) {
        m_DecoderOptionsDialog->SetIcon(icon);
        StartDecoder();
    } else {
        m_thDecoder = NULL;
        m_bStopDecoding->Disable();

        if(m_DecoderOptionsDialog)
            m_DecoderOptionsDialog->m_sMinusSaturationThreshold->Disable();
        else
            m_bDecoderOptions->Disable();
    }
}

WeatherFaxWizard::~WeatherFaxWizard()
{
    StopDecoder();
    delete m_DecoderOptionsDialog;

    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T ( "/Settings/WeatherFax" ) );

    wxPoint p = GetPosition();
    pConf->Write ( _T ( "WizardX" ), p.x);
    pConf->Write ( _T ( "WizardY" ), p.y);

    wxSize s = GetSize();
    pConf->Write ( _T ( "WizardW" ), s.x);
    pConf->Write ( _T ( "WizardH" ), s.y);
}

void WeatherFaxWizard::StartDecoder()
{
    /* periodically check for updates */
    m_tDecoder.Connect(wxEVT_TIMER, wxTimerEventHandler( WeatherFaxWizard::OnDecoderTimer ), NULL, this);
    m_tDecoder.Start(1000, wxTIMER_ONE_SHOT);

    m_bDecoderStopped = false;

    /* run decoder in a separate thread */
    m_thDecoder = new DecoderThread(m_decoder);
    m_thDecoder->Run();

    m_bStopDecoding->SetLabel(_("Stop"));
    m_bStopDecoding->Enable();
}

void WeatherFaxWizard::StopDecoder()
{
    if(!m_thDecoder)
        return;

    m_tDecoder.Stop();
    m_decoder.m_bEndDecoding = true;

    m_bStopDecoding->Disable();

    if(m_bDecoderStopped)
        m_decoder.m_DecoderStopMutex.Unlock();

    m_thDecoder->Wait(); /* wait for decoder thread to end */
    delete m_thDecoder;
    m_thDecoder = NULL;
}

void WeatherFaxWizard::MakeNewCoordinates()
{
    /* make a new coord, yet making sure it has a unique name */
    wxString newcoordname;
    int cc = m_Coords.GetCount();
    for(int n=0, i=-1; i != cc; n++) {
        if(n)
            newcoordname = wxString::Format(m_NewCoordBaseName + _T(" %d"), n);
        else
            newcoordname = m_NewCoordBaseName;
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

    m_newCoords = new WeatherFaxImageCoordinates(newcoordname);
    SetCoords(sel);

    if(m_Coords.GetCount() == 0)
        m_cbCoordSet->Disable();
}

void WeatherFaxWizard::OnDecoderTimer( wxTimerEvent & )
{
    if(m_decoder.m_DecoderMutex.Lock() == wxMUTEX_NO_ERROR) {
        if(!m_thDecoder->IsRunning())
            m_bStopDecoding->Disable();

        int w = m_decoder.m_imagewidth, h = m_decoder.m_imageline;
        if(h && (!m_wfimg.m_origimg.IsOk() || h != m_wfimg.m_origimg.GetHeight())) {
            m_wfimg.m_origimg = wxImage( w, h );
            memcpy(m_wfimg.m_origimg.GetData(), m_decoder.m_imgdata, w*h*3);

            m_sPhasing->SetRange(0, m_wfimg.m_origimg.GetWidth()-1);

            int oldh = m_wfimg.m_phasedimg.GetHeight();

            m_wfimg.MakePhasedImage();

            int pw = m_wfimg.m_phasedimg.GetWidth(), ph = m_wfimg.m_phasedimg.GetHeight();

            int x, y;
            m_swFaxArea1->GetViewStart(&x, &y);
            int sw, sh;
            m_swFaxArea1->GetSize(&sw, &sh);
            if(y >= oldh - sh)
                y += ph - oldh;
            if(y < 0)
                y = 0;
            m_swFaxArea1->SetScrollbars(1, 1, pw, ph, x, y);
            m_swFaxArea1->Refresh();
        }
        m_decoder.m_DecoderMutex.Unlock();
        m_bPhasingArea->Refresh();
    }
    m_tDecoder.Start(1000, wxTIMER_ONE_SHOT);
}

void WeatherFaxWizard::OnStopDecoding( wxCommandEvent& event )
{
    if((m_bDecoderStopped = !m_bDecoderStopped)) {
        m_bStopDecoding->SetLabel(_("Start"));
        m_decoder.m_DecoderStopMutex.Lock();
    } else {
        m_bStopDecoding->SetLabel(_("Stop"));
        m_decoder.m_DecoderStopMutex.Unlock();
    }
}

void WeatherFaxWizard::OnDecoderOptions( wxCommandEvent& event )
{
    m_DecoderOptionsDialog->ShowModal();
}

void WeatherFaxWizard::OnPaintPhasing( wxPaintEvent& event )
{
    wxWindow *window = dynamic_cast<wxWindow*>(event.GetEventObject());
    if(!window)
        return;

    wxPaintDC dc( window );

    if(!m_thDecoder)
        return;

    dc.SetBrush(wxBrush(*wxBLACK));
    dc.SetPen(wxPen( *wxBLACK, 1 ));

    int blocksize = m_decoder.m_blocksize;
    int w, h;
    int s = 4;
    window->GetSize(&w, &h);
    for(int x = 0; x<w; x++) {
        int i = x * blocksize / w;
        int y = h*((m_decoder.m_imageline ? m_decoder.datadouble[i] : 0) +(s/2))/s;
        dc.DrawLine(x, h/2, x, y);
    }

    int p;
    dc.SetPen(wxPen( *wxRED, 1 ));
    p = h*(1+(s/2))/s;
    dc.DrawLine(0, p, w, p);
    p = h*(-1+(s/2))/s;
    dc.DrawLine(0, p, w, p);
    
    dc.SetPen(wxPen( wxColour(32,192,32), 1 ));
    p = h*(m_decoder.m_minus_saturation_threshold+(s/2))/s;
    dc.DrawLine(0, p, w, p);
}

void WeatherFaxWizard::OnWizardPageChanged( wxWizardEvent& event )
{
    if(event.GetPage() == m_pages[1]) {
        if(!event.GetDirection()) {
            /* if we backed up, convert the mapped coordinates back to the input */
            double x1, y1, mx1, my1;
            double x2, y2, mx2, my2;

            x1 = m_sCoord1X->GetValue(), y1 = m_sCoord1Y->GetValue();
            x2 = m_sCoord2X->GetValue(), y2 = m_sCoord2Y->GetValue();

            m_wfimg.MercatorToInput(x1, y1, mx1, my1);
            m_sCoord1XUnMapped->SetValue(round(mx1));
            m_sCoord1YUnMapped->SetValue(round(my1));

            m_wfimg.MercatorToInput(x2, y2, mx2, my2);
            m_sCoord2XUnMapped->SetValue(round(mx2));
            m_sCoord2YUnMapped->SetValue(round(my2));
    
            double coord1lat, coord1lon, coord2lat, coord2lon;
            m_tCoord1Lat->GetValue().ToDouble(&coord1lat);
            m_tCoord1Lon->GetValue().ToDouble(&coord1lon);
            m_tCoord2Lat->GetValue().ToDouble(&coord2lat);
            m_tCoord2Lon->GetValue().ToDouble(&coord2lon);
            WriteMappingLatLon(coord1lat, coord1lon, coord2lat, coord2lon);
        }

        SetUnMappedCoordRanges();
        UpdateMappingControls();

        m_rbCoord1UnMapped->SetValue(true);
        m_rbCoord2UnMapped->SetValue(false);
    } else if(event.GetPage() == m_pages[2]) {
        StoreMappingParams();
        /* invalidate mapped image */
        m_wfimg.m_mappedimg = wxNullImage;
        m_rbCoord1UnMapped->SetValue(true);
        m_rbCoord2UnMapped->SetValue(false);

        if(!ApplyMapping()) {
            wxMessageDialog w
                ( this, _("Failed to apply mapping\nCheck Mapping Correction Parameters"),
                  _("Mapping"), wxOK | wxICON_ERROR );
            w.ShowModal();
            ShowPage(m_pages[1], true);
        } else if(m_curCoords->mapping == WeatherFaxImageCoordinates::MERCATOR &&
                m_curCoords->mappingmultiplier == 1 &&
                  m_curCoords->mappingratio == 1) {
            wxWizardEvent dummy;
            OnWizardFinished( dummy );
            if(IsModal())
                EndModal(wxID_OK);
            else
                Hide();
        }
    }
}

void WeatherFaxWizard::OnWizardCancel( wxWizardEvent& event )
{
    delete m_newCoords;

    if(m_parent.WizardCleanup(this)) {
        m_tDecoder.Stop();
        delete &m_wfimg;
    }
}

void WeatherFaxWizard::OnWizardFinished( wxWizardEvent& event )
{
    /* add coordinates to set if it is the new one, but make
       sure it has a unique name */
    int sel = m_cbCoordSet->GetSelection();
    if(sel == -1)
        sel = m_SelectedIndex;

    if(sel == 0) {
        int cc = m_Coords.GetCount();
        wxString newname = m_newCoords->name, newnumberedname;
        for(int n=0, i=-1; i != cc; n++) {
            newnumberedname = newname;
            if(n)
                newnumberedname += wxString::Format(_T(" %d"), n);

            if(!cc)
                break;
            for(i=0; i<cc; i++)
                if(m_Coords[i]->name == newnumberedname)
                    break;
        }
        m_newCoords->name = newnumberedname;
        m_Coords.Append(m_newCoords);
    }

    StoreCoords();
    StoreMappingParams();

    if(m_parent.WizardCleanup(this))
        m_parent.WizardFinished(this);
}

void WeatherFaxWizard::OnSetSizes( wxInitDialogEvent& event )
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

void WeatherFaxWizard::UpdateMappingControls()
{
    switch((WeatherFaxImageCoordinates::MapType)m_cMapping->GetSelection()) {
    case WeatherFaxImageCoordinates::MERCATOR: /* nomapping */
        m_sMappingPoleX->Disable(),    m_sMappingPoleX->SetValue(0);
        m_sMappingPoleY->Disable(),    m_sMappingPoleY->SetValue(0);
        m_sMappingEquatorY->Disable(), m_sMappingEquatorY->SetValue(0);
        m_tTrueRatio->Disable(),       m_tTrueRatio->SetValue(_T("1.0"));
        m_bGetMapping->Disable();
        m_bGetEquator->Disable();
        break;
    case WeatherFaxImageCoordinates::POLAR:
        m_sMappingPoleX->Enable();
        m_sMappingPoleY->Enable();
        m_sMappingEquatorY->Enable();
        m_tTrueRatio->Enable();
        m_bGetMapping->Enable();
        m_bGetEquator->Enable();
        break;
    case WeatherFaxImageCoordinates::CONIC:
        m_sMappingPoleX->Enable();
        m_sMappingPoleY->Enable();
        m_sMappingEquatorY->Enable();
        m_tTrueRatio->Enable();
        m_bGetMapping->Enable();
        m_bGetEquator->Enable();
        break;
    case WeatherFaxImageCoordinates::FIXED_FLAT:
        m_sMappingPoleX->Disable();
        m_sMappingPoleY->Enable();
        m_sMappingEquatorY->Enable();
        m_tTrueRatio->Disable();
        m_bGetMapping->Enable();
        m_bGetEquator->Disable();
        break;
    default: break;
    }
}

void WeatherFaxWizard::OnMappingChoice( wxCommandEvent& event )
{
    UpdateMappingControls();
    Refresh();
}

void WeatherFaxWizard::GetMappingPolar(bool onlyequator)
{
    double mapping1x = m_sCoord1XUnMapped->GetValue(), mapping1y = m_sCoord1YUnMapped->GetValue();
    double mapping2x = m_sCoord2XUnMapped->GetValue(), mapping2y = m_sCoord2YUnMapped->GetValue();

    double mapping1lat, mapping1lon, mapping2lat, mapping2lon;
    ReadMappingLatLon(mapping1lat, mapping1lon, mapping2lat, mapping2lon);

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
            mapping1lat = -mapping1lat;
            mapping2lat = -mapping2lat;
            south = true;
        }
    } else {
        /* use south pole */
        if(mapping1lat < 0) {
            mapping1lat = -mapping1lat;
            mapping2lat = -mapping2lat;
            south = true;
        }
    }

    double pp1 = 1 - mapping1lat / 90;
    double pp2 = 1 - mapping2lat / 90;

        /* for polar stereograph */
        pp1 = tan(pp1 * M_PI/4);
        pp2 = tan(pp2 * M_PI/4);

    /* 
       pp1 = sqrt(((mapping1x - inputpole.x)*inputtrueratio)^2 + (mapping1y - inputpole.y)^2)/(inputequator - inputpole.y)
       pp2 = sqrt(((mapping2x - inputpole.x)*inputtrueratio)^2 + (mapping2y - inputpole.y)^2)/(inputequator - inputpole.y)

       inputpole.x = mapping1x

       mapping2lon - mapping1lon = atan2((mapping2x - inputpole.x)*inputtrueratio,
       mapping2y - inputpole.y)

       (pp1*(inputequator - inputpoley))^2 = (((mapping1x - mapping1x)*inputtrueratio)^2 + (mapping1y - inputpoley)^2)
       (pp2*(inputequator - inputpoley))^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)
       d = sqrt(((mapping2x - inputpole.x)*inputtrueratio)^2 + (mapping2y - inputpole.y)^2)
       (mapping2x - mapping1x)*inputtrueratio = d*sin(mapping2lon - mapping1lon)
       mapping2y - inputpoley = d*cos(mapping2lon - mapping1lon)

       sm2lm1l = sin(mapping2lon - mapping1lon)
       cm2lm1l = cos(mapping2lon - mapping1lon)

       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)
       d = sqrt(((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)
       (mapping2x - mapping1x)*inputtrueratio = d*sm2lm1l
       mapping2y - inputpoley = d*cm2lm1l

       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)
       ((mapping2x - mapping1x)*inputtrueratio)^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)*sm2lm1l^2
       (mapping2y - inputpoley)^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)*cm2lm1l^2

       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)
       (mapping2y - inputpoley)^2 = ((((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)*sm2lm1l^2 + (mapping2y - inputpoley)^2)*cm2lm1l^2

       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)
       ((mapping2y - inputpoley)^2*(1 - sm2lm1l^2*cm2lm1l^2 - cm2lm1l^2)) / (sm2lm1l^2*cm2lm1l^2)  = ((mapping2x - mapping1x)*inputtrueratio)^2


       inputtrueratio = sqrt(((mapping2y - inputpoley)^2*(1 - sm2lm1l^2*cm2lm1l^2 - cm2lm1l^2)) / (sm2lm1l^2*cm2lm1l^2)) / (mapping2x - mapping1x)

       // inputtrueratio then inputequator theninputpoley
       pp1*(inputequator - inputpoley) = mapping1y - inputpoley
       (pp2*(inputequator - inputpoley))^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)
       inputtrueratio = sqrt(1 - sm2lm1l^2*cm2lm1l^2 - cm2lm1l^2) * (mapping2y - inputpoley) / ((mapping2x - mapping1x) * sm2lm1l * cm2lm1l)

       inputequator = (mapping1y - inputpoley) / pp1 + inputpoley
       (pp2*(inputequator - inputpoley))^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)

       (pp2*((mapping1y - inputpoley) / pp1))^2 = (((mapping2x - mapping1x)*inputtrueratio)^2 + (mapping2y - inputpoley)^2)

       mapping1y^2*pp2^2/pp1^2 - 2*inputpoley*mapping1y*pp2^2/pp1^2 + inputpoley^2*pp2^2/pp1^2
       = mapping2y^2 - 2*inputpoley*mapping2y + (mapping2x^2 - 2*mapping1x*mapping2x + mapping1x^2)*inputtrueratio^2 + inputpoley^2

       inputpoley^2*(pp2^2/pp1^2 - 1) + inputpoley*2*(mapping2y - mapping1y*pp2^2/pp1^2)
       = mapping2y^2 - mapping1y^2*pp2^2/pp1^2 + ((mapping2x - mapping1x)*inputtrueratio)^2

inputpoley^2*(pp2^2/pp1^2 - 1) + inputpoley*2*(mapping2y - mapping1y*pp2^2/pp1^2)
       = mapping2y^2 - mapping1y^2*pp2^2/pp1^2 + ((mapping2x - mapping1x) * (mapping2y - inputpoley) / ((mapping2x - mapping1x) * sm2lm1l * cm2lm1l))^2 * (1 - sm2lm1l^2*cm2lm1l^2 - cm2lm1l^2)

inputpoley^2*(pp2^2/pp1^2 - 1) + inputpoley*2*(mapping2y - mapping1y*pp2^2/pp1^2)
       = mapping2y^2 - mapping1y^2*pp2^2/pp1^2 + mapping2y^2/(cm2lm1l^2*sm2lm1l^2)-mapping2y^2/sm2lm1l^2
       -2*inputpoley*mapping2y/(cm2lm1l^2*sm2lm1l^2) + 2*inputpoley*mapping2y/sm2lm1l^2
       + inputpoley^2/(cm2lm1l^2*sm2lm1l^2) - inputpoley^2/sm2lm1l^2-mapping2y^2
       + 2*inputpoley*mapping2y-inputpoley^2



inputpoley^2* (pp2^2/pp1^2 - 1/(cm2lm1l^2*sm2lm1l^2) + 1/sm2lm1l^2) +
inputpoley  * 2*(mapping2y/(cm2lm1l^2*sm2lm1l^2) - mapping2y/sm2lm1l^2 - mapping1y*pp2^2/pp1^2) +
mapping1y^2*pp2^2/pp1^2 - mapping2y^2/(cm2lm1l^2*sm2lm1l^2) + mapping2y^2/sm2lm1l^2 = 0

       e = cm2lm1l^2
       d = (cm2lm1l*sm2lm1l)^2
       q = pp2^2/pp1^2
inputpoley^2* (q + (e - 1) / d) +
inputpoley  * 2*(mapping2y*(1 - e)/d - mapping1y*q) +
mapping1y^2*q + mapping2y^2*(e - 1)/d = 0

       a = q + (e - 1) / d
       b = 2*(mapping2y*(1 - e)/d - mapping1y*q)
       c = mapping1y^2*q + mapping2y^2*(e - 1)/d
             
       inputpoley = (-b - sqrt(b^2 - 4*a*c)) / (2*a)

    */

    double inputpolex = mapping1x;
    double headingdiff = mapping2lon - mapping1lon;
    if(headingdiff >= 180) headingdiff -= 360;
    if(headingdiff <= -180) headingdiff += 360;
    double sm2lm1l = sin(M_PI/180*headingdiff);
    double cm2lm1l = cos(M_PI/180*headingdiff);

    double e = square(cm2lm1l);
    double d = square(cm2lm1l*sm2lm1l);
    double q = square(pp2)/square(pp1);
    double a = q + (e - 1)/d;
    double b = 2*(mapping2y*(1 - e)/d - mapping1y*q);
    double c = square(mapping1y)*q + square(mapping2y)*(e - 1)/d;
    double inputpoley1 = (-b + sqrt(b*b - 4*a*c)) / (2*a);
    double inputpoley2 = (-b - sqrt(b*b - 4*a*c)) / (2*a);

    /* pick correct root */
    double inputpoley;
    if(onlyequator) /* read from user input pole y */
        inputpoley = m_sMappingPoleY->GetValue();
    else
        inputpoley = (south ? pp1 < pp2 : pp1 > pp2) ? inputpoley1 : inputpoley2;

    double inputequator = (mapping1y - inputpoley) / pp1 + inputpoley;

    double inputtrueratio = (south?-1:1)*sqrt(1 - square(sm2lm1l*cm2lm1l) - square(cm2lm1l))
        * (mapping2y - inputpoley) / ((mapping2x - mapping1x) * sm2lm1l * cm2lm1l);

    if(isnan(inputpoley) || isnan(inputequator) || isnan(inputtrueratio) || inputtrueratio <= 0) {
        wxMessageDialog w
            ( this, _("Calculating Mapping Failed.\n\
Perhaps you have chosen the wrong mapping type for this image, \
or the coordinates specified are not correct\n"),
              _("Mapping"), wxOK | wxICON_ERROR );
        w.ShowModal();
        return;
    }

    m_sMappingEquatorY->SetValue(inputequator);

    if(!onlyequator) {
        m_sMappingPoleX->SetValue(inputpolex);
        m_sMappingPoleY->SetValue(inputpoley);
        m_tTrueRatio->SetValue(wxString::Format(_T("%.3f"), inputtrueratio));
    }
}

void WeatherFaxWizard::GetMappingFixedFlat()
{
    double mapping1lat, mapping1lon, mapping2lat, mapping2lon;
    ReadMappingLatLon(mapping1lat, mapping1lon, mapping2lat, mapping2lon);

    double pp1 = 1 - mapping1lat / 90;
    double pp2 = 1 - mapping2lat / 90;

    double mapping1y = m_sCoord1YUnMapped->GetValue(),  mapping2y = m_sCoord2YUnMapped->GetValue();

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

void WeatherFaxWizard::GetAspectRatio()
{
    /* calculate aspect ratio so that resulting image is mercator and can be
       easily exported to a kap file */

    wxPoint p1, p2;
    double mapping1lat, mapping1lon, mapping2lat, mapping2lon;
    ReadMappingLatLon(mapping1lat, mapping1lon, mapping2lat, mapping2lon);

    PlugIn_ViewPort smvp;
    smvp.clat = 0, smvp.clon = 0, smvp.view_scale_ppm = 1, smvp.skew = 0, smvp.rotation = 0;
    smvp.m_projection_type = PI_PROJECTION_MERCATOR;
    smvp.pix_width = smvp.pix_height = 1024;
    if(fabs(mapping1lon - mapping2lon) > 180)
        smvp.clon = 180;

    GetCanvasPixLL(&smvp, &p1, mapping1lat, mapping1lon);
    GetCanvasPixLL(&smvp, &p2, mapping2lat, mapping2lon);

    double p1x, p1y, p2x, p2y;

    double x1 = m_sCoord1XUnMapped->GetValue(), y1 = m_sCoord1YUnMapped->GetValue();
    double x2 = m_sCoord2XUnMapped->GetValue(), y2 = m_sCoord2YUnMapped->GetValue();

    double dx1 = p2.x - p1.x, dy1 = p2.y - p1.y;
    WeatherFaxImageCoordinates oldcoords = *m_curCoords; /* don't clobber incase of cancel */
    StoreMappingParams();
    m_curCoords->mappingratio = 1;
    m_wfimg.MakeMappedImage(this, true);
    m_wfimg.InputToMercator(x1, y1, p1x, p1y);
    m_wfimg.InputToMercator(x2, y2, p2x, p2y);
    double dx2 = p2x - p1x, dy2 = p2y - p1y;
    *m_curCoords = oldcoords; /* restore coordinates */

    m_tMappingRatio->SetValue(wxString::Format(_T("%.4f"), dx1 / dy1 * dy2 / dx2));
}

void WeatherFaxWizard::OnGetMapping( wxCommandEvent& event )
{
    switch((WeatherFaxImageCoordinates::MapType)m_cMapping->GetSelection()) {
    case WeatherFaxImageCoordinates::POLAR:
        GetMappingPolar(false);
        break;
    case WeatherFaxImageCoordinates::FIXED_FLAT:
        GetMappingFixedFlat();
        break;
    default:
        wxMessageDialog w( this, _("Calculating Mapping not yet supported.\n"),
                           _("Mapping"), wxOK | wxICON_WARNING );
        w.ShowModal();
        return;
    }
    GetAspectRatio();
    Refresh();
}

void WeatherFaxWizard::OnGetEquator( wxCommandEvent& event )
{
    switch((WeatherFaxImageCoordinates::MapType)m_cMapping->GetSelection()) {
    case WeatherFaxImageCoordinates::POLAR:
        GetMappingPolar(true);
        break;
    default:
        wxMessageDialog w( this, _("Calculating Equator not yet supported.\n"),
                           _("Mapping"), wxOK | wxICON_WARNING );
        w.ShowModal();
        return;
    }
    Refresh();
}

void WeatherFaxWizard::OnGetAspectRatio( wxCommandEvent& event )
{
    GetAspectRatio();
}

void WeatherFaxWizard::OnBitmapClickPage2( wxMouseEvent& event )
{
    wxPoint p = m_swFaxArea2->CalcUnscrolledPosition(event.GetPosition());
        
    if(m_rbCoord1UnMapped->GetValue()) {
        m_sCoord1XUnMapped->SetValue(p.x);
        m_sCoord1YUnMapped->SetValue(p.y);
        m_rbCoord1UnMapped->SetValue(false);
        m_rbCoord2UnMapped->SetValue(true);
    } else if(m_rbCoord2UnMapped->GetValue()) {
        m_sCoord2XUnMapped->SetValue(p.x);
        m_sCoord2YUnMapped->SetValue(p.y);
        m_rbCoord2UnMapped->SetValue(false);
        m_rbCoord1UnMapped->SetValue(true);
    }
    
    Refresh();
}

void WeatherFaxWizard::OnBitmapClickPage3( wxMouseEvent& event )
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

void WeatherFaxWizard::OnCoordSet( wxCommandEvent& event )
{
    int index = event.GetSelection();
    if(index == -1 ) /* should never occur, but prevent crash */
        return;

    SetCoords(index);
}

void WeatherFaxWizard::OnCoordText( wxCommandEvent& event )
{
    int index = m_cbCoordSet->GetSelection();
    if(index == -1)
        index = m_SelectedIndex;
    else if(index != m_SelectedIndex)
        return;

    static bool recursion = false;
    if(!recursion) {
        recursion = true;
        m_cbCoordSet->SetString(m_SelectedIndex, event.GetString());
    }
    recursion = false;
    m_curCoords->name = event.GetString();
}

void WeatherFaxWizard::OnRemoveCoords( wxCommandEvent& event )
{
    int selection = m_SelectedIndex;
    if(selection == 0) /* don't delete first item, button should be disabled anyway */
        return;

    m_cbCoordSet->Delete(selection);
    m_Coords.DeleteNode(m_Coords.Item(selection-1));
    m_cbCoordSet->SetSelection(selection-1);
    SetCoords(selection-1);
}

void WeatherFaxWizard::StoreMappingParams()
{
    m_curCoords->rotation = (WeatherFaxImageCoordinates::RotationType)m_cRotation->GetSelection();
    m_curCoords->mapping = (WeatherFaxImageCoordinates::MapType)m_cMapping->GetSelection();

    m_curCoords->inputpole.x = m_sMappingPoleX->GetValue();
    m_curCoords->inputpole.y = m_sMappingPoleY->GetValue();
    m_curCoords->inputequator = m_sMappingEquatorY->GetValue();

    double inputtrueratio;
    m_tTrueRatio->GetValue().ToDouble(&inputtrueratio);
    m_curCoords->inputtrueratio = inputtrueratio;

    double mappingmultiplier;
    m_tMappingMultiplier->GetValue().ToDouble(&mappingmultiplier);
    m_curCoords->mappingmultiplier = mappingmultiplier;

    double mappingratio;
    m_tMappingRatio->GetValue().ToDouble(&mappingratio);
    m_curCoords->mappingratio = mappingratio;
}

bool WeatherFaxWizard::ApplyMapping()
{
    if(!m_wfimg.m_mappedimg.IsOk())
        if(!m_wfimg.MakeMappedImage(this))
            return false;

    double x1 = m_sCoord1XUnMapped->GetValue(), y1 = m_sCoord1YUnMapped->GetValue(), mx1, my1;
    double x2 = m_sCoord2XUnMapped->GetValue(), y2 = m_sCoord2YUnMapped->GetValue(), mx2, my2;
 
    SetCoordRanges();

    m_wfimg.InputToMercator(x1, y1, mx1, my1);
    m_sCoord1X->SetValue(mx1);
    m_sCoord1Y->SetValue(my1);
    
    m_wfimg.InputToMercator(x2, y2, mx2, my2);
    m_sCoord2X->SetValue(mx2);
    m_sCoord2Y->SetValue(my2);

    double mapping1lat, mapping1lon, mapping2lat, mapping2lon;
    ReadMappingLatLon(mapping1lat, mapping1lon, mapping2lat, mapping2lon);

    m_tCoord1Lat->SetValue(wxString::Format(_T("%.6f"), mapping1lat));
    m_tCoord1Lon->SetValue(wxString::Format(_T("%.6f"), mapping1lon));
    m_tCoord2Lat->SetValue(wxString::Format(_T("%.6f"), mapping2lat));
    m_tCoord2Lon->SetValue(wxString::Format(_T("%.6f"), mapping2lon));

#if !wxCHECK_VERSION(3, 0, 0)
    m_cbCoordSet->SetValue(m_curCoords->name);
#endif

    Refresh();
    return true;
}

void WeatherFaxWizard::SetUnMappedCoordRanges()
{
    int w = m_wfimg.m_origimg.GetWidth(), h = m_wfimg.m_origimg.GetHeight();
    m_sCoord1XUnMapped->SetRange(0, w);
    m_sCoord2XUnMapped->SetRange(0, w);
    m_sCoord1YUnMapped->SetRange(0, h);
    m_sCoord2YUnMapped->SetRange(0, h);

    m_swFaxArea2->SetScrollbars(1, 1, w, h);
}

void WeatherFaxWizard::SetCoordRanges()
{
    int w = m_wfimg.m_mappedimg.GetWidth(), h = m_wfimg.m_mappedimg.GetHeight();
    m_sCoord1X->SetRange(0, w);
    m_sCoord2X->SetRange(0, w);
    m_sCoord1Y->SetRange(0, h);
    m_sCoord2Y->SetRange(0, h);

    m_swFaxArea3->SetScrollbars
        (1, 1, m_wfimg.m_mappedimg.GetWidth(), m_wfimg.m_mappedimg.GetHeight());
}

void WeatherFaxWizard::OnInformation( wxCommandEvent& event )
{
    wxMessageDialog w( this, _("\
Mapping allows for scaling, and conversion between coordinate systems\n\
Latitudes are +N -S, Longitudes +E -W\n\
\n\
For polar mapping mode:\n\
Get Mapping parameters calculates the mapping from \
the coordinates given, but the first coordinate's X \
value (vertical red line) must align with the pole in polar mode \
on the vertical meridian\n\
Once the coordinates are entered, the mapping should make the the blue latitudes \
align.  If they are far off, check the input coordinates.\n\
Once they are close:\n\
First adjust pole X to center the latitudes. Next adjust the Pole Y value \
until the blue latitude curve closest to the pole is correct. \
Now, if the second blue latitude is too wide, then \
the true width ratio needs to be increased, otherwise it should be decreased. \
Repeat adjusting the Pole Y and true width ratio until both align. \
It is easiest to calibrate correctly if you use the farthest spaced \
latitudes available.\n\
Once the blue latitudes align perfectly, recompute the equator to ensure the \
resulting mapping is correct. At this point, the coordinates can \
be changed to better locations if needed. If true width is very close to 1.0 \
it probably should be exactly 1.0.\n\
It is also possible to go to the next step, adjust the coordinates, then come \
back to this step and get the mapping parameters again to improve the estimate.\
"), _("Information"), wxOK | wxICON_INFORMATION );
    w.ShowModal();
}

void WeatherFaxWizard::ReadMappingLatLon(double &mapping1lat, double &mapping1lon,
                                          double &mapping2lat, double &mapping2lon)
{
    double mapping1latminutes, mapping1lonminutes, mapping2latminutes, mapping2lonminutes;
    mapping1lat = m_sCoord1LatUnMapped->GetValue();
    m_tCoord1LatMinutesUnMapped->GetValue().ToDouble(&mapping1latminutes);
    mapping1lat += ((mapping1lat > 0) ? mapping1latminutes : - mapping1latminutes) / 60;

    mapping1lon = m_sCoord1LonUnMapped->GetValue();
    m_tCoord1LonMinutesUnMapped->GetValue().ToDouble(&mapping1lonminutes);
    mapping1lon += ((mapping1lon > 0) ? mapping1lonminutes : - mapping1lonminutes) / 60;

    mapping2lat = m_sCoord2LatUnMapped->GetValue();
    m_tCoord2LatMinutesUnMapped->GetValue().ToDouble(&mapping2latminutes);
    mapping2lat += ((mapping2lat > 0) ? mapping2latminutes : - mapping2latminutes) / 60;

    mapping2lon = m_sCoord2LonUnMapped->GetValue();
    m_tCoord2LonMinutesUnMapped->GetValue().ToDouble(&mapping2lonminutes);
    mapping2lon += ((mapping2lon > 0) ? mapping2lonminutes : - mapping2lonminutes) / 60;
}

void WeatherFaxWizard::WriteMappingLatLon(double mapping1lat, double mapping1lon,
                                          double mapping2lat, double mapping2lon)
{
    m_sCoord1LatUnMapped->SetValue(trunc(mapping1lat)), mapping1lat -= trunc(mapping1lat);
    m_tCoord1LatMinutesUnMapped->SetValue(wxString::Format(_T("%.4f"), 60*fabsf(mapping1lat)));
    m_sCoord1LonUnMapped->SetValue(trunc(mapping1lon)), mapping1lon -= trunc(mapping1lon);
    m_tCoord1LonMinutesUnMapped->SetValue(wxString::Format(_T("%.4f"), 60*fabsf(mapping1lon)));
    m_sCoord2LatUnMapped->SetValue(trunc(mapping2lat)), mapping2lat -= trunc(mapping2lat);
    m_tCoord2LatMinutesUnMapped->SetValue(wxString::Format(_T("%.4f"), 60*fabsf(mapping2lat)));
    m_sCoord2LonUnMapped->SetValue(trunc(mapping2lon)), mapping2lon -= trunc(mapping2lon);
    m_tCoord2LonMinutesUnMapped->SetValue(wxString::Format(_T("%.4f"), 60*fabsf(mapping2lon)));
}

void WeatherFaxWizard::UpdatePage1()
{
    m_wfimg.phasing = m_sPhasing->GetValue();
    m_wfimg.skew = m_sSkew->GetValue();
    m_wfimg.filter = m_cFilter->GetSelection();
    m_curCoords->rotation = (WeatherFaxImageCoordinates::RotationType)m_cRotation->GetSelection();
    m_wfimg.MakePhasedImage();
    Refresh();
}

void WeatherFaxWizard::UpdatePage1Rotation ( wxCommandEvent& event )
{
    UpdatePage1();
    m_swFaxArea1->SetScrollbars(1, 1, m_wfimg.m_phasedimg.GetWidth(), m_wfimg.m_phasedimg.GetHeight());
}

void WeatherFaxWizard::StoreCoords()
{
    m_curCoords->p1.x = m_sCoord1X->GetValue();
    m_curCoords->p1.y = m_sCoord1Y->GetValue();
    m_tCoord1Lat->GetValue().ToDouble(&m_curCoords->lat1);
    m_tCoord1Lon->GetValue().ToDouble(&m_curCoords->lon1);
    m_curCoords->p2.x = m_sCoord2X->GetValue();
    m_curCoords->p2.y = m_sCoord2Y->GetValue();
    m_tCoord2Lat->GetValue().ToDouble(&m_curCoords->lat2);
    m_tCoord2Lon->GetValue().ToDouble(&m_curCoords->lon2);
}

void WeatherFaxWizard::SetCoords(int index)
{
    m_cbCoordSet->SetSelection(index);

    if(index) {
        m_curCoords = m_Coords[index-1];
        m_bRemoveCoordSet->Enable();
    } else {
        m_curCoords = m_newCoords;
        m_bRemoveCoordSet->Disable();
    }

    m_SelectedIndex = index;

#if !wxCHECK_VERSION(3, 0, 0)
    m_cbCoordSet->SetValue(m_curCoords->name);
#endif

    double x1 = m_curCoords->p1.x, y1 = m_curCoords->p1.y;
    double x2 = m_curCoords->p2.x, y2 = m_curCoords->p2.y;

    SetUnMappedCoordRanges();

    m_wfimg.MakeMappedImage(this, true);
    double mx1, my1;
    m_wfimg.MercatorToInput(x1, y1, mx1, my1);
    m_sCoord1XUnMapped->SetValue(mx1);
    m_sCoord1YUnMapped->SetValue(my1);

    double mx2, my2;
    m_wfimg.MercatorToInput(x2, y2, mx2, my2);
    m_sCoord2XUnMapped->SetValue(mx2);
    m_sCoord2YUnMapped->SetValue(my2);
    
    WriteMappingLatLon(m_curCoords->lat1, m_curCoords->lon1, m_curCoords->lat2, m_curCoords->lon2);
    
    m_cMapping->SetSelection((int)m_curCoords->mapping);
    UpdateMappingControls();
    
    m_sMappingPoleX->SetValue(m_curCoords->inputpole.x);
    m_sMappingPoleY->SetValue(m_curCoords->inputpole.y);
    m_sMappingEquatorY->SetValue(m_curCoords->inputequator);
    m_tTrueRatio->SetValue(wxString::Format(_T("%.4f"), m_curCoords->inputtrueratio));

    m_tMappingMultiplier->SetValue
        (wxString::Format(_T("%.2f"), m_curCoords->mappingmultiplier));
    
    m_tMappingRatio->SetValue
        (wxString::Format(_T("%.2f"), m_curCoords->mappingratio));

    Refresh();
}

void WeatherFaxWizard::OnSpin( wxSpinEvent& event )
{
    Refresh();
}

void WeatherFaxWizard::OnShowLatLonMinutes( wxCommandEvent& event )
{
    bool b = event.IsChecked();
    m_stMinutes->Show(b);
    m_tCoord1LatMinutesUnMapped->Show(b);
    m_tCoord1LonMinutesUnMapped->Show(b);
    m_tCoord2LatMinutesUnMapped->Show(b);
    m_tCoord2LonMinutesUnMapped->Show(b);

    // why this works, I will probably never know
    int w, h;
    GetSize(&w, &h);
    SetSize(w + 1, h);
    SetSize(w - 1, h);
}

void WeatherFaxWizard::OnPaintImage( wxPaintEvent& event)
{
    wxScrolledWindow *window = dynamic_cast<wxScrolledWindow*>(event.GetEventObject());
    if(!window)
        return;

    wxPaintDC dc( window );
    dc.SetBrush(wxBrush(*wxTRANSPARENT_BRUSH));
    
    wxBitmap bmp(GetCurrentPage() == m_pages[2] ? m_wfimg.m_mappedimg : m_wfimg.m_phasedimg);

    int x, y;
    window->GetViewStart(&x, &y);

    int w, h;
    window->GetSize(&w, &h);

    if( bmp.IsOk() ) {
        wxMemoryDC mdc(bmp);
        int bw = bmp.GetWidth(), bh = bmp.GetHeight();
        if(w > bw) w = bw;
        if(h > bh) h = bh;
        dc.Blit( 0, 0, w, h, &mdc, x, y);
        mdc.SelectObject( wxNullBitmap );
    }

    if(GetCurrentPage() != m_pages[0]) {
        int x1, x2, y1, y2;
        if(GetCurrentPage() == m_pages[1]) {
             x1 = m_sCoord1XUnMapped->GetValue(), y1 = m_sCoord1YUnMapped->GetValue();
             x2 = m_sCoord2XUnMapped->GetValue(), y2 = m_sCoord2YUnMapped->GetValue();
        } else {
             x1 = m_sCoord1X->GetValue(), y1 = m_sCoord1Y->GetValue();
             x2 = m_sCoord2X->GetValue(), y2 = m_sCoord2Y->GetValue();
        }

        dc.SetPen(wxPen( *wxRED, 1 ));
        dc.DrawLine(x1-x, 0, x1-x, h);
        dc.DrawLine(0, y1-y, w, y1-y);

        dc.SetPen(wxPen( wxColour(32,192,32), 1 ));
        dc.DrawLine(x2-x, 0, x2-x, h);
        dc.DrawLine(0, y2-y, w, y2-y);

        if(GetCurrentPage() == m_pages[1] &&
           (m_cMapping->GetSelection() == (int)WeatherFaxImageCoordinates::POLAR ||
            m_cMapping->GetSelection() == (int)WeatherFaxImageCoordinates::CONIC)) {
            int inputpolex = m_sMappingPoleX->GetValue();
            int inputpoley = m_sMappingPoleY->GetValue();
            double inputtrueratio;
            m_tTrueRatio->GetValue().ToDouble(&inputtrueratio);

            dc.SetPen(wxPen( *wxBLUE, 1 ));
            dc.SetBrush(wxBrush(*wxBLACK, wxBRUSHSTYLE_TRANSPARENT));

            double d;

            d = hypot((x1 - inputpolex)*inputtrueratio, y1 - inputpoley);
            dc.DrawEllipse(inputpolex - x - d/inputtrueratio,
                           inputpoley - y - d, 2*d/inputtrueratio, 2*d);

            d = hypot((x2 - inputpolex)*inputtrueratio, y2 - inputpoley);
            dc.DrawEllipse(inputpolex - x - d/inputtrueratio,
                           inputpoley - y - d, 2*d/inputtrueratio, 2*d);
        }
    }
}
