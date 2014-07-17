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

#include "weatherfax_pi.h"
#include "FaxDecoder.h"
#include "DecoderOptionsDialog.h"

DecoderOptionsDialog::DecoderOptionsDialog(wxWindow *parent, FaxDecoder &decoder)
    : DecoderOptionsDialogBase(parent), m_decoder(decoder)
{
    Hide();

    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Audio" ) );

    m_sImageWidth->SetValue(pConf->Read ( _T ( "ImageWidth" ), 1024L ));
    m_sBitsPerPixel->SetValue(pConf->Read ( _T ( "BitsPerPixel" ), 8L ));
    m_sCarrier->SetValue(pConf->Read ( _T ( "Carrier" ), 1900L ));
    m_sDeviation->SetValue(pConf->Read ( _T ( "Deviation" ), 400L ));
    m_cFilter->SetSelection(pConf->Read ( _T ( "Filter" ), FaxDecoder::firfilter::MIDDLE ));
    m_cbSkip->SetValue(pConf->Read ( _T ( "SkipHeaderDetection" ), 0L ));
    m_cbInclude->SetValue(pConf->Read ( _T ( "IncludeHeadersInImage" ), 0L ));

    ConfigureDecoder();

    bool capture = m_decoder.m_inputtype != FaxDecoder::FILENAME;
    m_cSampleRate->Enable(capture);

    if(!capture) {
        m_cSampleRate->Insert(wxString::Format(_T("%d"), m_decoder.m_SampleRate), 0);
        m_cSampleRate->SetSelection(0);
    }
}

void DecoderOptionsDialog::OnDone( wxCommandEvent& event )
{
    wxFileConfig *pConf = GetOCPNConfigObject();

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Audio" ) );

    pConf->Write ( _T ( "ImageWidth" ), m_sImageWidth->GetValue());
    pConf->Write ( _T ( "BitsPerPixel" ), m_sBitsPerPixel->GetValue());
    pConf->Write ( _T ( "Carrier" ), m_sCarrier->GetValue());
    pConf->Write ( _T ( "Deviation" ), m_sDeviation->GetValue());
    pConf->Write ( _T ( "Filter" ), m_cFilter->GetSelection());
    pConf->Write ( _T ( "SkipHeaderDetection" ), m_cbSkip->GetValue());
    pConf->Write ( _T ( "IncludeHeadersInImage" ), m_cbInclude->GetValue());

    Hide();
}

void DecoderOptionsDialog::ConfigureDecoder()
{
    long samplerate;
    m_cSampleRate->GetStringSelection().ToLong(&samplerate);

    if(!m_decoder.Configure(
           m_sImageWidth->GetValue(), m_sBitsPerPixel->GetValue(),
           m_sCarrier->GetValue(), m_sDeviation->GetValue(),
           (enum FaxDecoder::firfilter::Bandwidth)m_cFilter->GetSelection(),
           (double)-m_sMinusSaturationThreshold->GetValue()/10 - 1,
           m_cbSkip->GetValue(), m_cbInclude->GetValue(),
           samplerate)) {
        wxMessageDialog w( NULL, _("Failed to configure capture."),
                           _("Failure"), wxOK | wxICON_ERROR );
        w.ShowModal();
    }
}
