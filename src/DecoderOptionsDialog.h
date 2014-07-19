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

#include "WeatherFaxUI.h"

class DecoderOptionsDialog : public DecoderOptionsDialogBase
{
public:
    DecoderOptionsDialog(WeatherFaxWizard &wizard);

    void OnOptionsSpin( wxSpinEvent& event ) { ConfigureDecoder(false); }
    void OnOptions( wxCommandEvent& event ) { ConfigureDecoder(false); }
    void OnResetOptions( wxCommandEvent& event ) { ConfigureResetDecoder(); }
    void OnDone( wxCommandEvent& event );
    void ResetDecoder();
    void ConfigureResetDecoder();
    void ConfigureDecoder(bool reset);

private:
    int origwidth;

    WeatherFaxWizard &m_wizard;
};
