/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2015 by Sean D'Epagnier                                 *
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
#include "WeatherFaxImage.h"
#include "WeatherFax.h"
#include "WeatherFaxWizard.h"
#include "DecoderOptionsDialog.h"

DecoderOptionsDialog::DecoderOptionsDialog(WeatherFaxWizard &wizard)
    : DecoderOptionsDialogBase(&wizard), m_wizard(wizard)
{
    Hide();

    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Audio" ) );

    m_sImageWidth->SetValue(pConf->Read ( _T ( "ImageWidth" ), 1024L ));
    m_sBitsPerPixel->SetValue(pConf->Read ( _T ( "BitsPerPixel" ), 8L ));
    m_sCarrier->SetValue(pConf->Read ( _T ( "Carrier" ), 1900L ));
    m_sDeviation->SetValue(pConf->Read ( _T ( "Deviation" ), 400L ));
    m_sMinusSaturationThreshold->SetValue(pConf->Read ( _T ( "MinusSaturationThreshold" ), 15L ));
    m_cFilter->SetSelection(pConf->Read ( _T ( "Filter" ), FaxDecoder::firfilter::MIDDLE ));
    m_cbSkip->SetValue((bool)pConf->Read ( _T ( "SkipHeaderDetection" ), 0L ));
    m_cbInclude->SetValue((bool)pConf->Read ( _T ( "IncludeHeadersInImage" ), 0L ));

    origwidth = m_sImageWidth->GetValue();

//    FaxDecoder &decoder = m_wizard.m_decoder;

    ConfigureDecoder(true);
}

void DecoderOptionsDialog::OnDone( wxCommandEvent& event )
{
    wxFileConfig *pConf = GetOCPNConfigObject();

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Audio" ) );

    bool spin_options_changed =
        m_sBitsPerPixel->GetValue() != pConf->Read ( _T ( "BitsPerPixel" ), 8L ) ||
        m_sCarrier->GetValue() != pConf->Read ( _T ( "Carrier" ), 1900L ) ||
        m_sDeviation->GetValue() != pConf->Read ( _T ( "Deviation" ), 400L ) ||
        m_sMinusSaturationThreshold->GetValue() != pConf->Read ( _T ( "MinusSaturationThreshold" ), 15L );

    pConf->Write ( _T ( "ImageWidth" ), m_sImageWidth->GetValue());
    pConf->Write ( _T ( "BitsPerPixel" ), m_sBitsPerPixel->GetValue());
    pConf->Write ( _T ( "Carrier" ), m_sCarrier->GetValue());
    pConf->Write ( _T ( "Deviation" ), m_sDeviation->GetValue());
    pConf->Write ( _T ( "MinusSaturationThreshold" ), m_sMinusSaturationThreshold->GetValue());
    pConf->Write ( _T ( "Filter" ), m_cFilter->GetSelection());
    pConf->Write ( _T ( "SkipHeaderDetection" ), m_cbSkip->GetValue());
    pConf->Write ( _T ( "IncludeHeadersInImage" ), m_cbInclude->GetValue());

    FaxDecoder &decoder = m_wizard.m_decoder;
    bool capture = decoder.m_CaptureSettings.type == FaxDecoderCaptureSettings::AUDIO ||
        decoder.m_CaptureSettings.type == FaxDecoderCaptureSettings::RTLSDR;

//    Hide();
    EndModal(wxID_OK);

    if(origwidth != m_sImageWidth->GetValue() ||
       (!capture && spin_options_changed)) {
        origwidth = m_sImageWidth->GetValue();
        ResetDecoder();
    }
}

void DecoderOptionsDialog::ResetDecoder()
{
    m_wizard.StopDecoder();
    ConfigureDecoder(true);
    m_wizard.StartDecoder();
}

void DecoderOptionsDialog::ConfigureResetDecoder()
{
    FaxDecoder &decoder = m_wizard.m_decoder;
    if(decoder.m_CaptureSettings.type == FaxDecoderCaptureSettings::FILE)
        ResetDecoder();
    else
        ConfigureDecoder(false);
}

void DecoderOptionsDialog::ConfigureDecoder(bool reset)
{
    FaxDecoder &decoder = m_wizard.m_decoder;
    if(!decoder.Configure(
           origwidth, m_sBitsPerPixel->GetValue(),
           m_sCarrier->GetValue(), m_sDeviation->GetValue(),
           (enum FaxDecoder::firfilter::Bandwidth)m_cFilter->GetSelection(),
           (double)-m_sMinusSaturationThreshold->GetValue()/10 - 1,
           m_cbSkip->GetValue(), m_cbInclude->GetValue(), reset)) {
        wxMessageDialog w( NULL, _("Failed to configure capture."),
                           _("Failure"), wxOK | wxICON_ERROR );
        w.ShowModal();
    }
}
