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


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "weatherfax_pi.h"
#include "WeatherFaxDialog.h"

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

namespace weatherfax_icon
{
#include "weatherfax_icon.h"
}


//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

weatherfax_pi::weatherfax_pi(void *ppimgr)
    : opencpn_plugin_18(ppimgr)
{
      int w = weatherfax_icon::width, h = weatherfax_icon::height;
      char *img_data = (char*)malloc(3*w*h); /* wximage needs malloc */
      const char *data = weatherfax_icon::header_data;
      for(int i=0; i<w*h; i++)
          HEADER_PIXEL(data, (img_data+3*i))
      wxImage img_weatherfax(w, h, (unsigned char*)img_data);
      bmp_weatherfax_pi = new wxBitmap(img_weatherfax);

      m_CacheBitmap = NULL;
      m_gltexture = 0;

      m_image = NULL;
}

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

      m_leftclick_tool_id  = InsertPlugInTool(_T(""), bmp_weatherfax_pi, bmp_weatherfax_pi, wxITEM_NORMAL,
            _("WeatherFax"), _T(""), NULL,
             WEATHERFAX_TOOL_POSITION, 0, this);

      m_pWeatherFaxDialog = NULL;

      return (WANTS_OVERLAY_CALLBACK |
              WANTS_OPENGL_OVERLAY_CALLBACK |
              WANTS_TOOLBAR_CALLBACK    |
              INSTALLS_TOOLBAR_TOOL     |
              WANTS_PREFERENCES         |
              WANTS_CONFIG
           );
}

bool weatherfax_pi::DeInit(void)
{
      //    Record the dialog position
      if (NULL != m_pWeatherFaxDialog)
      {
            wxPoint p = m_pWeatherFaxDialog->GetPosition();
            SetWeatherFaxDialogX(p.x);
            SetWeatherFaxDialogY(p.y);

            m_pWeatherFaxDialog->Close();
            delete m_pWeatherFaxDialog;
            m_pWeatherFaxDialog = NULL;
      }
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
      return bmp_weatherfax_pi;
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
      if (NULL == m_pWeatherFaxDialog)
            return;
      DimeWindow(m_pWeatherFaxDialog);
}

void weatherfax_pi::RearrangeWindow()
{
      if (NULL == m_pWeatherFaxDialog)
            return;

      SetColorScheme(PI_ColorScheme());

      m_pWeatherFaxDialog->Fit();
}

void weatherfax_pi::OnToolbarToolCallback(int id)
{
      if(!m_pWeatherFaxDialog)
      {
            m_pWeatherFaxDialog = new WeatherFaxDialog(*this, m_parent_window);
            m_pWeatherFaxDialog->m_sTransparency->SetValue(m_iTransparency);
            m_pWeatherFaxDialog->m_sWhiteTransparency->SetValue(m_iWhiteTransparency);
            m_pWeatherFaxDialog->Move(wxPoint(m_weatherfax_dialog_x, m_weatherfax_dialog_y));
      }

      RearrangeWindow();
      m_pWeatherFaxDialog->Show(!m_pWeatherFaxDialog->IsShown());

      wxPoint p = m_pWeatherFaxDialog->GetPosition();
      m_pWeatherFaxDialog->Move(0,0);        // workaround for gtk autocentre dialog behavior
      m_pWeatherFaxDialog->Move(p);
}

bool weatherfax_pi::GetOverlayCoords(PlugIn_ViewPort *vp, wxPoint &p1, wxPoint &p2, int &w, int &h)
{
    if(!m_image || !m_image->m_Coords)
        return false;

    double lat1 = m_image->m_Coords->lat1, lon1 = m_image->m_Coords->lon1;
    double lat2 = m_image->m_Coords->lat2, lon2 = m_image->m_Coords->lon2;

    /* skip segments that go the wrong way around the world */
    if(lon1+180 < vp->clon && lon2+180 > vp->clon)
        return false;
    if(lon1-180 < vp->clon && lon2-180 > vp->clon)
        return false;
    
    GetCanvasPixLL( vp, &p1, lat1, lon1 );
    GetCanvasPixLL( vp, &p2, lat2, lon2 );
    w = m_image->m_Coords->p2.x - m_image->m_Coords->p1.x;
    h = m_image->m_Coords->p2.y - m_image->m_Coords->p1.y;

    if(w < 0 || h < 0)
        return false;

    return true;
}

bool weatherfax_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    wxPoint p1, p2;
    int w, h;
    if(!GetOverlayCoords(vp, p1, p2, w, h))
        return false;

    int iw = p2.x-p1.x, ih = p2.y-p1.y;

    if(!m_CacheBitmap || m_bNeedUpdateImageDC ||
       m_CacheBitmap->GetWidth() != iw || m_CacheBitmap->GetHeight() != ih) {
        m_bNeedUpdateImageDC = false;

        delete m_CacheBitmap;
        m_CacheBitmap = NULL;

        /* dont go too huge */
        if(w > 8192 || h > 8192)
            return false;

        wxRect r(m_image->m_Coords->p1.x, m_image->m_Coords->p1.y, w, h);
        wxImage subimg = m_image->PhasedImage().GetSubImage(r);

        if(!subimg.IsOk())
            return false;

#if 0
        wxImage stretchedimg = subimg.Scale(iw, ih); /* doesn't support saturated transparency, or invert */
#else        
        unsigned char *subdata = subimg.GetData();
        unsigned char *stretcheddata = (unsigned char*)malloc(iw*ih*3); /* malloc needed for wximage */
        for(int y=0; y<ih; y++) {
            int sy = y*h/ih;
            for(int x=0; x<iw; x++) {
                int sx = x*w/iw;
                for(int c=0; c<3; c++) {
                    wxUint8 v = subdata[3*(w*sy+sx)+c];
                    if(v > 255-m_iWhiteTransparency)
                        v = 255;
                    else if(m_bInvert)
                        v = 255-v;
                    stretcheddata[3*(iw*y+x)+c] = v;
                }
            }
        }

        wxImage stretchedimg(iw, ih, stretcheddata);
#endif

        m_CacheBitmap = new wxBitmap(stretchedimg);
        if(m_iWhiteTransparency)
            m_CacheBitmap->SetMask(new wxMask(*m_CacheBitmap, wxColour(255, 255, 255)));
    }

    dc.DrawBitmap(*m_CacheBitmap, p1.x, p1.y, true);

    return true;
}

bool weatherfax_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    wxPoint p1, p2;
    int w, h;
    if(!GetOverlayCoords(vp, p1, p2, w, h))
        return false;

    if(m_bNeedUpdateImageGL) {
        m_bNeedUpdateImageGL = false;

        if(!m_gltexture)
            glGenTextures(1, &m_gltexture);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_gltexture);
        
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        
#if 1 /* I think we can delete these */
        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
        glPixelStorei( GL_UNPACK_SKIP_ROWS, 0 );
#endif
        
        wxRect r(m_image->m_Coords->p1.x, m_image->m_Coords->p1.y,
                 m_image->m_Coords->p2.x - m_image->m_Coords->p1.x,
                 m_image->m_Coords->p2.y - m_image->m_Coords->p1.y);
        wxImage subimg = m_image->PhasedImage().GetSubImage(r);

        if(!subimg.IsOk())
            return false;

        unsigned char *data = subimg.GetData();
        unsigned char *idata = NULL;
        unsigned int size = subimg.GetWidth()*subimg.GetHeight();
        idata = new unsigned char[4*size];
        for(unsigned int i=0; i<size; i++) {
            wxUint8 r = data[3*i + 0], g = data[3*i + 1], b = data[3*i + 2];
            if(m_bInvert)
                idata[4*i + 0] = r,  idata[4*i + 1] = g, idata[4*i + 2] = b;
            else
                idata[4*i + 0] = 255-r,  idata[4*i + 1] = 255-g, idata[4*i + 2] = 255-b;

            wxUint8 a = 255 + -r*m_iWhiteTransparency/255; /* white alpha */
            idata[4*i+3] = a;
        }

        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA,
                     subimg.GetWidth(), subimg.GetHeight(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, idata);
        delete [] idata;
    }
        
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT);

    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_MULTISAMPLE );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    glColor4ub(255, 255, 255, 255-m_iTransparency);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_gltexture);
    
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0), glVertex2i(p1.x, p1.y);
    glTexCoord2i(w, 0), glVertex2i(p2.x, p1.y);
    glTexCoord2i(w, h), glVertex2i(p2.x, p2.y);
    glTexCoord2i(0, h), glVertex2i(p1.x, p2.y);
    glEnd();

    glPopAttrib();

    return true;
}

void weatherfax_pi::SetImageToRender(WeatherFaxImage *image)
{
    m_image = image;
    m_bNeedUpdateImageDC = true;
    m_bNeedUpdateImageGL = true;

    RequestRefresh( m_parent_window );
}

bool weatherfax_pi::LoadConfig(void)
{
      wxFileConfig *pConf = m_pconfig;

      if(pConf)
      {
            pConf->SetPath ( _T( "/Settings/WeatherFax" ) );
            pConf->Read ( _T( "Transparency" ),  &m_iTransparency, 64 );
            pConf->Read ( _T( "WhiteTransparency" ),  &m_iWhiteTransparency, 64 );
            pConf->Read ( _T( "Invert" ),  &m_bInvert, 0 );
            pConf->Read ( _T( "Path" ),  &m_path, _T ( "" ) );

            m_weatherfax_dialog_x =  pConf->Read ( _T ( "DialogPosX" ), 20L );
            m_weatherfax_dialog_y =  pConf->Read ( _T ( "DialogPosY" ), 20L );

            pConf->SetPath ( _T ( "/Settings/WeatherFax/Audio" ) );
            pConf->Read ( _T ( "ImageWidth" ), &m_ImageWidth, 576 );
            pConf->Read ( _T ( "BitsPerPixel" ), &m_BitsPerPixel, 8 );
            pConf->Read ( _T ( "Carrier" ), &m_Carrier, 1900 );
            pConf->Read ( _T ( "Deviation" ), &m_Deviation, 400 );
            pConf->Read ( _T ( "Filter" ), &m_Filter, 1 );
            pConf->Read ( _T ( "SkipHeaderDetection" ), &m_bSkipHeaderDetection, 0 );
            pConf->Read ( _T ( "IncludeHeadersInImage" ), &m_bIncludeHeadersInImage, 0 );

            pConf->SetPath ( _T ( "/Directories" ) );
            wxString def;
            def = ::wxGetCwd() + _T("/plugins");
            pConf->Read ( _T ( "WeatherFaxDataLocation" ), &m_weatherfax_dir, def);
            return true;
      }
      else
            return false;
}

bool weatherfax_pi::SaveConfig(void)
{
      wxFileConfig *pConf = m_pconfig;

      if(pConf)
      {
            pConf->SetPath ( _T ( "/Settings/WeatherFax" ) );
            pConf->Write ( _T ( "Transparency" ), m_iTransparency );
            pConf->Write ( _T ( "WhiteTransparency" ), m_iWhiteTransparency );
            pConf->Write ( _T ( "Invert" ), m_bInvert );
            pConf->Write ( _T ( "Path" ), m_path );

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

            pConf->SetPath ( _T ( "/Directories" ) );
            pConf->Write ( _T ( "WeatherFaxDataLocation" ), m_weatherfax_dir );

            return true;
      }
      else
            return false;
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
        SaveConfig();
    }
    delete dialog;
}
