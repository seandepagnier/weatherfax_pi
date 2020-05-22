/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by Sean D'Epagnier                                 *
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
#include "icons.h"
#include "plugingl/pidc.h"

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
    : opencpn_plugin_116(ppimgr)
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
#ifdef OCPN_USE_SVG
    m_leftclick_tool_id = InsertPlugInToolSVG(_T( "WeatherFax" ), _svg_weatherfax, _svg_weatherfax_rollover, _svg_weatherfax_toggled,
                                                wxITEM_CHECK, _("WeatherFax"), _T( "" ), NULL, WEATHERFAX_TOOL_POSITION, 0, this);
#else
    m_leftclick_tool_id  = InsertPlugInTool(_T("WeatherFax"), _img_weatherfax,
                                            _img_weatherfax, wxITEM_NORMAL,
                                            _("WeatherFax"), _T(""), NULL,
                                            WEATHERFAX_TOOL_POSITION, 0, this);
#endif
    m_pWeatherFax = NULL;

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
    SaveConfig();
    //    Record the dialog position
    if(m_pWeatherFax) {
        m_pWeatherFax->m_SchedulesDialog.Close();
        m_pWeatherFax->m_InternetRetrievalDialog.Close();
        m_pWeatherFax->Close();
        delete m_pWeatherFax;
        m_pWeatherFax = NULL;
    }

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
    // return _("WeatherFax");
	return _T(PLUGIN_COMMON_NAME);
		
}

wxString weatherfax_pi::GetShortDescription()
{
    return _("Weather Fax PlugIn for OpenCPN");
}

wxString weatherfax_pi::GetLongDescription()
{
    return _("Weather Fax PlugIn for OpenCPN\n\
Can open image files directly, or decode audio faxes to an image.\n\
With simple calibration, resulting image is overlaid on top of charts.\n\
Converts images in mercator, polar, conic and uniform coordinates.\n\
Can convert any image into a raster chart.\n\
Builtin database for HF radio fax stations via SSB.\n\
Builtin database for internet retrieval from meterological sites.\n\
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
    if(!m_pWeatherFax) {
        // Set some default private member parameters
        m_weatherfax_dialog_x = 0;
        m_weatherfax_dialog_y = 0;

        ::wxDisplaySize(&m_display_width, &m_display_height);

        //    Get a pointer to the opencpn display canvas, to use as a parent for the POI Manager dialog
        m_parent_window = GetOCPNCanvasWindow();

        //    And load the configuration items
        LoadConfig();

        m_pWeatherFax = new WeatherFax(*this, m_parent_window);
        m_pWeatherFax->Move(wxPoint(m_weatherfax_dialog_x, m_weatherfax_dialog_y));
    }

    m_pWeatherFax->Show(!m_pWeatherFax->IsShown());

    if(!m_pWeatherFax->IsShown()) {
        m_pWeatherFax->m_SchedulesDialog.Hide();
        m_pWeatherFax->m_InternetRetrievalDialog.Hide();
    }

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
        if(m_pWeatherFax->m_lFaxes->IsSelected(i))
            m_pWeatherFax->m_Faxes[i]->RenderImage(dc, vp);

    return true;
}

bool weatherfax_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    piDC pidc;
    pidc.SetVP(vp);
    
    if(!m_pWeatherFax || !m_pWeatherFax->IsShown())
        return true;

    for(unsigned int i=0; i<m_pWeatherFax->m_lFaxes->GetCount(); i++)
        if(m_pWeatherFax->m_lFaxes->IsSelected(i))
            m_pWeatherFax->m_Faxes[i]->RenderImageGL(vp);

    return true;
}

wxString weatherfax_pi::StandardPath()
{
    wxString s = wxFileName::GetPathSeparator();
    wxString stdPath  = *GetpPrivateApplicationDataLocation();

    stdPath += s + _T("plugins");
    if (!wxDirExists(stdPath))
      wxMkdir(stdPath);

    stdPath += s + _T("weatherfax");

#ifdef __WXOSX__
    // Compatibility with pre-OCPN-4.2; move config dir to
    // ~/Library/Preferences/opencpn if it exists
    {
        wxStandardPathsBase& std_path = wxStandardPathsBase::Get();
        wxString s = wxFileName::GetPathSeparator();
        wxString oldPath = (std_path.GetUserConfigDir() + s + _T("plugins") + s + _T("weatherfax"));
        if (wxDirExists(oldPath) && !wxDirExists(stdPath)) {
		    wxLogMessage("weatherfax_pi: moving config dir %s to %s", oldPath, stdPath);
		    wxRenameFile(oldPath, stdPath);
        }
    }
#endif

    if (!wxDirExists(stdPath))
      wxMkdir(stdPath);

    stdPath += s; // is this necessary?
    return stdPath;
}

bool weatherfax_pi::LoadConfig(void)
{
    wxFileConfig *pConf = GetOCPNConfigObject();

    if(!pConf)
        return false;

    pConf->SetPath ( _T( "/Settings/WeatherFax" ) );
    pConf->Read ( _T( "Path" ),  &m_path, _T ( "" ) );
    pConf->Read ( _T( "ExportPath" ),  &m_export_path, _T ( "" ) );

    m_weatherfax_dialog_x =  pConf->Read ( _T ( "DialogPosX" ), 20L );
    m_weatherfax_dialog_y =  pConf->Read ( _T ( "DialogPosY" ), 20L );

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Schedules" ) );
    pConf->Read ( _T ( "LoadAtStart" ), &m_bLoadSchedulesStart, 0 );

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Capture" ) );
    int type;
    pConf->Read ( _T ( "type"), &type, (int)FaxDecoderCaptureSettings::AUDIO );
    m_CaptureSettings.type = (FaxDecoderCaptureSettings::Type)type;

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Capture/audio" ) );
    pConf->Read ( _T ( "deviceindex" ), &m_CaptureSettings.audio_deviceindex, 0);
    pConf->Read ( _T ( "samplerate" ), &m_CaptureSettings.audio_samplerate, 8000);

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Capture/rtlsdr" ) );
    pConf->Read ( _T ( "deviceindex" ), &m_CaptureSettings.rtlsdr_deviceindex, 0);
    pConf->Read ( _T ( "errorppm" ), &m_CaptureSettings.rtlsdr_errorppm, 0);
    pConf->Read ( _T ( "upconverter_mhz" ), &m_CaptureSettings.rtlsdr_upconverter_mhz, 125);

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Export" ) );
    pConf->Read ( _T ( "Colors" ), &m_iExportColors, 64 );
    pConf->Read ( _T ( "DepthMeters" ), &m_bExportDepthMeters, true );
    pConf->Read ( _T ( "SoundingDatum" ), &m_sExportSoundingDatum, _T("LOWEST LOW WATER"));
    
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Updates" ) );
    pConf->Read( _T("UpdateDataBaseUrl"), &m_UpdateDataBaseUrl, _T("https://raw.githubusercontent.com/seandepagnier/weatherfax_pi/master/data/") );

    return true;
}

bool weatherfax_pi::SaveConfig(void)
{
    wxFileConfig *pConf = GetOCPNConfigObject();

    if(!pConf)
        return false;

    pConf->SetPath ( _T ( "/Settings/WeatherFax" ) );
    pConf->Write ( _T ( "Path" ), m_path );
    pConf->Write ( _T ( "ExportPath" ), m_export_path );

    if(m_pWeatherFax) {
        wxPoint p = m_pWeatherFax->GetPosition();
        pConf->Write ( _T ( "DialogPosX" ),   p.x );
        pConf->Write ( _T ( "DialogPosY" ),   p.y );
    }
        
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Schedules" ) );
    pConf->Write ( _T ( "LoadAtStart" ), m_bLoadSchedulesStart );

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Capture" ) );
    pConf->Write ( _T ( "type"), (long)m_CaptureSettings.type );

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Capture/audio" ) );
    pConf->Write ( _T ( "deviceindex" ), m_CaptureSettings.audio_deviceindex);
    pConf->Write ( _T ( "samplerate" ), m_CaptureSettings.audio_samplerate);

    pConf->SetPath ( _T ( "/Settings/WeatherFax/Capture/rtlsdr" ) );
    pConf->Write ( _T ( "deviceindex" ), m_CaptureSettings.rtlsdr_deviceindex);
    pConf->Write ( _T ( "errorppm" ), m_CaptureSettings.rtlsdr_errorppm);
    pConf->Write ( _T ( "upconverter_mhz" ), m_CaptureSettings.rtlsdr_upconverter_mhz);
    
    pConf->SetPath ( _T ( "/Settings/WeatherFax/Export" ) );
    pConf->Write ( _T ( "Colors" ), m_iExportColors );
    pConf->Write ( _T ( "DepthMeters" ), m_bExportDepthMeters );
    pConf->Write ( _T ( "SoundingDatum" ), m_sExportSoundingDatum );

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
    
    dialog->m_cbLoadSchedulesStart->SetValue(m_bLoadSchedulesStart);

    dialog->m_sDeviceIndex->SetRange(0, FaxDecoder::AudioDeviceCount() - 1);

#ifndef BUILTIN_RTLAIS
    dialog->m_cbCaptureType->RemovePage(1);
    //TODO: This is actually broken at least on macOS, but let it be for now as we may build with rtlsdr there
#else
    dialog->m_cbCaptureType->SetSelection(m_CaptureSettings.type == FaxDecoderCaptureSettings::RTLSDR);
#endif
    dialog->m_srtlsdr_deviceindex->SetValue(m_CaptureSettings.rtlsdr_deviceindex);
    dialog->m_srtlsdr_errorppm->SetValue(m_CaptureSettings.rtlsdr_errorppm);
    dialog->m_srtlsdr_upconverter_mhz->SetValue(m_CaptureSettings.rtlsdr_upconverter_mhz);

    dialog->m_sExportColors->SetValue(m_iExportColors);
    dialog->m_rbExportDepthMeters->SetValue(m_bExportDepthMeters);
    dialog->m_tExportSoundingDatum->SetValue(m_sExportSoundingDatum);
    
    dialog->Fit();
    wxColour cl;
    GetGlobalColor(_T("DILG1"), &cl);
    dialog->SetBackgroundColour(cl);
    
    if(dialog->ShowModal() == wxID_OK)
    {
        m_bLoadSchedulesStart = dialog->m_cbLoadSchedulesStart->GetValue();

        wxString t = dialog->m_cbCaptureType->GetPageText(dialog->m_cbCaptureType->GetSelection());
        if(t == "audio")
            m_CaptureSettings.type = FaxDecoderCaptureSettings::AUDIO;
        else if(t == "rtlsdr")
            m_CaptureSettings.type = FaxDecoderCaptureSettings::RTLSDR;

        m_CaptureSettings.rtlsdr_deviceindex = dialog->m_srtlsdr_deviceindex->GetValue();
        m_CaptureSettings.rtlsdr_errorppm = dialog->m_srtlsdr_errorppm->GetValue();
        m_CaptureSettings.rtlsdr_upconverter_mhz = dialog->m_srtlsdr_upconverter_mhz->GetValue();
        
        m_CaptureSettings.audio_deviceindex = dialog->m_sDeviceIndex->GetValue();
        m_CaptureSettings.audio_samplerate = wxAtol( dialog->m_cSampleRate->GetString(dialog->m_cSampleRate->GetSelection()) );

        m_iExportColors = dialog->m_sExportColors->GetValue();
        m_bExportDepthMeters = dialog->m_rbExportDepthMeters->GetValue();
        m_sExportSoundingDatum = dialog->m_tExportSoundingDatum->GetValue();

        SaveConfig();
    }
    delete dialog;
}

