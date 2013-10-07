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


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/stdpaths.h>

#include "weatherfax_pi.h"
#include "WeatherFaxImage.h"
#include "WeatherFax.h"
#include "icons.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new weatherfax_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}

//---------------------------------------------------------------------------------------------------------
//
//    WeatherFax PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

weatherfax_pi::weatherfax_pi(void *ppimgr)
    : opencpn_plugin_18(ppimgr)
{
    // Create the PlugIn icons
    initialize_images();
}

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

int weatherfax_pi::Init(void)
{
    AddLocaleCatalog( _T("opencpn-weatherfax_pi") );

    // Set some default private member parameters
    m_weatherfax_dialog_x = 0;
    m_weatherfax_dialog_y = 0;

    ::wxDisplaySize(&m_display_width, &m_display_height);

    //    Get a pointer to the opencpn display canvas, to use as a parent for the POI Manager dialog
    m_parent_window = GetOCPNCanvasWindow();

    //    Get a pointer to the opencpn configuration object
    m_pconfig = GetOCPNConfigObject();

    //    And load the configuration items
    LoadConfig();

    m_leftclick_tool_id  = InsertPlugInTool(_T(""), _img_weatherfax,
                                            _img_weatherfax, wxITEM_NORMAL,
                                            _("WeatherFax"), _T(""), NULL,
                                            WEATHERFAX_TOOL_POSITION, 0, this);

    m_pWeatherFax = new WeatherFax(*this, m_parent_window);
    m_pWeatherFax->Move(wxPoint(m_weatherfax_dialog_x, m_weatherfax_dialog_y));

    return (WANTS_OVERLAY_CALLBACK |
            WANTS_OPENGL_OVERLAY_CALLBACK |
            WANTS_TOOLBAR_CALLBACK    |
            INSTALLS_TOOLBAR_TOOL     |
            WANTS_NMEA_EVENTS         |
            WANTS_PREFERENCES         |
            WANTS_CONFIG
        );
}

bool weatherfax_pi::DeInit(void)
{
      //    Record the dialog position
    wxPoint p = m_pWeatherFax->GetPosition();
    SetWeatherFaxX(p.x);
    SetWeatherFaxY(p.y);

    m_pWeatherFax->Close();
    delete m_pWeatherFax;
    m_pWeatherFax = NULL;

    SaveConfig();

    RemovePlugInTool(m_leftclick_tool_id);

    return true;
}

int weatherfax_pi::GetAPIVersionMajor()
{
    return MY_API_VERSION_MAJOR;
}

int weatherfax_pi::GetAPIVersionMinor()
{
    return MY_API_VERSION_MINOR;
}

int weatherfax_pi::GetPlugInVersionMajor()
{
    return PLUGIN_VERSION_MAJOR;
}

int weatherfax_pi::GetPlugInVersionMinor()
{
    return PLUGIN_VERSION_MINOR;
}

wxBitmap *weatherfax_pi::GetPlugInBitmap()
{
    return new wxBitmap(_img_weatherfax->ConvertToImage().Copy());
}

wxString weatherfax_pi::GetCommonName()
{
    return _("WeatherFax");
}

wxString weatherfax_pi::GetShortDescription()
{
    return _("Weather Fax PlugIn for OpenCPN");
}

wxString weatherfax_pi::GetLongDescription()
{
    return _("Weather Fax PlugIn for OpenCPN\n\
Read weather fax encoded data as audio or image. \n\
Overlay this on top of charts. \n\
Enable OpenGL (in Display options) for best results. \n\
\n\
The Weather Fax plugin was written by Sean D'Epagnier\n\
");
}

int weatherfax_pi::GetToolbarToolCount(void)
{
    return 1;
}

void weatherfax_pi::SetColorScheme(PI_ColorScheme cs)
{
    if (NULL == m_pWeatherFax)
        return;
    DimeWindow(m_pWeatherFax);
}

void weatherfax_pi::RearrangeWindow()
{
    if (NULL == m_pWeatherFax)
        return;

    SetColorScheme(PI_ColorScheme());

    m_pWeatherFax->Fit();
}

void weatherfax_pi::OnToolbarToolCallback(int id)
{
    m_pWeatherFax->Show(!m_pWeatherFax->IsShown());

    RearrangeWindow();

    wxPoint p = m_pWeatherFax->GetPosition();
    m_pWeatherFax->Move(0,0);        // workaround for gtk autocentre dialog behavior
    m_pWeatherFax->Move(p);
}

bool weatherfax_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    if(!m_pWeatherFax || !m_pWeatherFax->IsShown())
        return true;

    for(unsigned int i=0; i<m_pWeatherFax->m_lFaxes->GetCount(); i++)
        if(m_pWeatherFax->m_lFaxes->IsChecked(i) ||
           (m_pWeatherFax->m_cbDisplaySelected->GetValue() &&
            (int)i == m_pWeatherFax->m_lFaxes->GetSelection()))
            m_pWeatherFax->m_Faxes[i]->RenderImage(dc, vp);

    return true;
}

bool weatherfax_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    if(!m_pWeatherFax || !m_pWeatherFax->IsShown())
        return true;

    for(unsigned int i=0; i<m_pWeatherFax->m_lFaxes->GetCount(); i++)
        if(m_pWeatherFax->m_lFaxes->IsChecked(i) ||
           (m_pWeatherFax->m_cbDisplaySelected->GetValue() &&
            (int)i == m_pWeatherFax->m_lFaxes->GetSelection()))
            m_pWeatherFax->m_Faxes[i]->RenderImageGL(vp);

    return true;
}

wxString weatherfax_pi::StandardPath()
{
    wxStandardPathsBase& std_path = wxStandardPathsBase::Get();
#ifdef __WXMSW__
    wxString stdPath  = std_path.GetConfigDir();
#endif
#ifdef __WXGTK__
    wxString stdPath  = std_path.GetUserDataDir();
#endif
#ifdef __WXOSX__
    wxString stdPath  = std_path.GetUserConfigDir();   // should be ~/Library/Preferences	
#endif

    return stdPath + wxFileName::GetPathSeparator();
}

bool weatherfax_pi::LoadConfig(void)
{
    wxFileConfig *pConf = m_pconfig;

    if(pConf)
    {
        pConf->SetPath ( _T( "/Settings/WeatherFax" ) );
        pConf->Read ( _T( "Path" ),  &m_path, _T ( "" ) );
        pConf->Read ( _T( "ExportPath" ),  &m_export_path, _T ( "" ) );

        m_weatherfax_dialog_x =  pConf->Read ( _T ( "DialogPosX" ), 20L );
        m_weatherfax_dialog_y =  pConf->Read ( _T ( "DialogPosY" ), 20L );

        pConf->SetPath ( _T ( "/Settings/WeatherFax/Audio" ) );
        pConf->Read ( _T ( "ImageWidth" ), &m_ImageWidth, 576*2 );
        pConf->Read ( _T ( "BitsPerPixel" ), &m_BitsPerPixel, 8 );
        pConf->Read ( _T ( "Carrier" ), &m_Carrier, 1900 );
        pConf->Read ( _T ( "Deviation" ), &m_Deviation, 400 );
        pConf->Read ( _T ( "Filter" ), &m_Filter, 1 );
        pConf->Read ( _T ( "SkipHeaderDetection" ), &m_bSkipHeaderDetection, 0 );
        pConf->Read ( _T ( "IncludeHeadersInImage" ), &m_bIncludeHeadersInImage, 0 );

        pConf->SetPath ( _T ( "/Settings/WeatherFax/Schedules" ) );
        pConf->Read ( _T ( "LoadAtStart" ), &m_bLoadSchedulesStart, 0 );

        pConf->SetPath ( _T ( "/Settings/WeatherFax/Export" ) );
        pConf->Read ( _T ( "Colors" ), &m_iExportColors, 64 );
        pConf->Read ( _T ( "DepthMeters" ), &m_bExportDepthMeters, true );
        pConf->Read ( _T ( "SoundingDatum" ), &m_sExportSoundingDatum, _T("LOWEST LOW WATER"));

        pConf->SetPath ( _T ( "/Directories" ) );
        wxString def;
        def = ::wxGetCwd() + _T("/plugins");
        pConf->Read ( _T ( "WeatherFaxDataLocation" ), &m_weatherfax_dir, def);
        return true;
    } else
        return false;
}

bool weatherfax_pi::SaveConfig(void)
{
    wxFileConfig *pConf = m_pconfig;

    if(!pConf)
        return false;

    pConf->SetPath ( _T ( "/Settings/WeatherFax" ) );
    pConf->Write ( _T ( "Path" ), m_path );
    pConf->Write ( _T ( "ExportPath" ), m_export_path );
    
    pConf->Write ( _T ( "DialogPosX" ),   m_weatherfax_dialog_x );
    pConf->Write ( _T ( "DialogPosY" ),   m_weatherfax_dialog_y );
    
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Audio" ) );
    pConf->Write ( _T ( "ImageWidth" ), m_ImageWidth );
    pConf->Write ( _T ( "BitsPerPixel" ), m_BitsPerPixel );
    pConf->Write ( _T ( "Carrier" ), m_Carrier );
    pConf->Write ( _T ( "Deviation" ), m_Deviation );
    pConf->Write ( _T ( "Filter" ), m_Filter );
    pConf->Write ( _T ( "SkipHeaderDetection" ), m_bSkipHeaderDetection );
    pConf->Write ( _T ( "IncludeHeadersInImage" ), m_bIncludeHeadersInImage );
    
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Schedules" ) );
    pConf->Write ( _T ( "LoadAtStart" ), m_bLoadSchedulesStart );
    
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Export" ) );
    pConf->Write ( _T ( "Colors" ), m_iExportColors );
    pConf->Write ( _T ( "DepthMeters" ), m_bExportDepthMeters );
    pConf->Write ( _T ( "SoundingDatum" ), m_sExportSoundingDatum );

    pConf->SetPath ( _T ( "/Directories" ) );
    pConf->Write ( _T ( "WeatherFaxDataLocation" ), m_weatherfax_dir );

    return true;
}

void weatherfax_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{
    m_lastfix = pfix;
}

void weatherfax_pi::ShowPreferencesDialog( wxWindow* parent )
{
    WeatherFaxPrefsDialog *dialog =
        new WeatherFaxPrefsDialog( parent, wxID_ANY, _("WeatherFax Preferences"),
                                   wxPoint( m_weatherfax_dialog_x, m_weatherfax_dialog_y),
                                   wxDefaultSize, wxDEFAULT_DIALOG_STYLE );
    
    dialog->m_sImageWidth->SetValue(m_ImageWidth);
    dialog->m_sBitsPerPixel->SetValue(m_BitsPerPixel);
    dialog->m_sCarrier->SetValue(m_Carrier);
    dialog->m_sDeviation->SetValue(m_Deviation);
    dialog->m_cFilter->SetSelection(m_Filter);
    dialog->m_cbSkip->SetValue(m_bSkipHeaderDetection);
    dialog->m_cbInclude->SetValue(m_bIncludeHeadersInImage);

    dialog->m_cbLoadSchedulesStart->SetValue(m_bLoadSchedulesStart);

    dialog->m_sExportColors->SetValue(m_iExportColors);
    dialog->m_rbExportDepthMeters->SetValue(m_bExportDepthMeters);
    dialog->m_tExportSoundingDatum->SetValue(m_sExportSoundingDatum);
    
    dialog->Fit();
    wxColour cl;
    GetGlobalColor(_T("DILG1"), &cl);
    dialog->SetBackgroundColour(cl);
    
    if(dialog->ShowModal() == wxID_OK)
    {
        m_ImageWidth = dialog->m_sImageWidth->GetValue();
        m_BitsPerPixel = dialog->m_sBitsPerPixel->GetValue();
        m_Carrier = dialog->m_sCarrier->GetValue();
        m_Deviation = dialog->m_sDeviation->GetValue();
        m_Filter = dialog->m_cFilter->GetSelection();
        m_bSkipHeaderDetection = dialog->m_cbSkip->GetValue();
        m_bIncludeHeadersInImage = dialog->m_cbInclude->GetValue();

        m_bLoadSchedulesStart = dialog->m_cbLoadSchedulesStart->GetValue();

        m_iExportColors = dialog->m_sExportColors->GetValue();
        m_bExportDepthMeters = dialog->m_rbExportDepthMeters->GetValue();
        m_sExportSoundingDatum = dialog->m_tExportSoundingDatum->GetValue();

        SaveConfig();
    }
    delete dialog;
}

