///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WEATHERFAXUI_H__
#define __WEATHERFAXUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/listbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/slider.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class WeatherFaxDialogBase
///////////////////////////////////////////////////////////////////////////////
class WeatherFaxDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxButton* m_bNew;
		wxButton* m_bEdit;
		wxButton* m_bDelete;
		wxCheckBox* m_cInvert;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnListBox( wxCommandEvent& event ) { event.Skip(); }
		virtual void NewFaxClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void EditFaxClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void DeleteFaxClicked( wxCommandEvent& event ) { event.Skip(); }
		virtual void TransparencyChanged( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnInvert( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxStaticBoxSizer* sbFax;
		wxListBox* m_lFaxes;
		wxSlider* m_sTransparency;
		
		WeatherFaxDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Weather Fax"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL ); 
		~WeatherFaxDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class EditFaxDialogBase
///////////////////////////////////////////////////////////////////////////////
class EditFaxDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxScrolledWindow* m_swFaxArea;
		wxStaticBitmap* m_bFaxBitmap;
		wxButton* m_bSetCoordinates;
		wxButton* m_bSplit;
		wxStdDialogButtonSizer* m_sdbSizer3;
		wxButton* m_sdbSizer3OK;
		
		// Virtual event handlers, overide them in your derived class
		virtual void BitmapClick( wxMouseEvent& event ) { event.Skip(); }
		virtual void SetCoordinates( wxCommandEvent& event ) { event.Skip(); }
		virtual void Split( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		EditFaxDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Edit Fax"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 600,400 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER ); 
		~EditFaxDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class WeatherFaxPrefsDialog
///////////////////////////////////////////////////////////////////////////////
class WeatherFaxPrefsDialog : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText29;
		wxStaticText* m_staticText30;
		wxStaticText* m_staticText31;
		wxStaticText* m_staticText32;
		wxStaticText* m_staticText33;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
		wxSpinCtrl* m_sImageWidth;
		wxSpinCtrl* m_sBitsPerPixel;
		wxSpinCtrl* m_sCarrier;
		wxSpinCtrl* m_sDeviation;
		wxChoice* m_cFilter;
		wxCheckBox* m_cbSkip;
		wxCheckBox* m_cbInclude;
		
		WeatherFaxPrefsDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Weather Fax Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE ); 
		~WeatherFaxPrefsDialog();
	
};

#endif //__WEATHERFAXUI_H__
