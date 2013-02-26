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
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/radiobut.h>
#include <wx/wizard.h>
#include <wx/dynarray.h>
WX_DEFINE_ARRAY_PTR( wxWizardPageSimple*, WizardPages );

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
		virtual void WhiteTransparencyChanged( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnInvert( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxStaticBoxSizer* sbFax;
		wxListBox* m_lFaxes;
		wxSlider* m_sTransparency;
		wxSlider* m_sWhiteTransparency;
		
		WeatherFaxDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Weather Fax"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL ); 
		~WeatherFaxDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class EditFaxWizardBase
///////////////////////////////////////////////////////////////////////////////
class EditFaxWizardBase : public wxWizard 
{
	private:
	
	protected:
		wxScrolledWindow* m_swFaxArea1;
		wxStaticText* m_staticText16;
		wxChoice* m_cFilter;
		wxStaticText* m_staticText9;
		wxSlider* m_sPhasing;
		wxStaticText* m_staticText17;
		wxChoice* m_cRotation;
		wxStaticText* m_staticText101;
		wxSlider* m_sSkew;
		wxScrolledWindow* m_swFaxArea2;
		wxStaticText* m_staticText15;
		wxChoice* m_cMapping;
		wxStaticText* m_staticText21;
		wxStaticText* m_staticText22;
		wxStaticText* m_staticText24;
		wxTextCtrl* m_tMappingMultiplier;
		wxStaticText* m_stMappingLabel1;
		wxSpinCtrl* m_sMappingValue1;
		wxStaticText* m_stMappingLabel2;
		wxSpinCtrl* m_sMappingValue2;
		wxStaticText* m_stMappingLabel3;
		wxSpinCtrl* m_sMappingValue3;
		wxButton* m_bResetMapping;
		wxButton* m_bApplyMapping;
		wxButton* m_bGetMappingParameters;
		wxScrolledWindow* m_swFaxArea3;
		wxStaticText* m_staticText10;
		wxComboBox* m_cbCoordSet;
		wxButton* m_bRemoveCoordSet;
		wxRadioButton* m_rbCoord1;
		wxSpinCtrl* m_sCoord1X;
		wxSpinCtrl* m_sCoord1Y;
		wxStaticText* m_staticText6;
		wxSpinCtrl* m_sCoord1Lat;
		wxSpinCtrl* m_sCoord1Lon;
		wxRadioButton* m_rbCoord2;
		wxSpinCtrl* m_sCoord2X;
		wxSpinCtrl* m_sCoord2Y;
		wxStaticText* m_staticText8;
		wxSpinCtrl* m_sCoord2Lat;
		wxSpinCtrl* m_sCoord2Lon;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnSetSizes( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnWizardPageChanged( wxWizardEvent& event ) { event.Skip(); }
		virtual void OnBitmapClickPage1( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnPaintImage( wxPaintEvent& event ) { event.Skip(); }
		virtual void UpdatePage1( wxCommandEvent& event ) { event.Skip(); }
		virtual void UpdatePage1( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnBitmapClickPage2( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnMappingChoice( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnResetMapping( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnApplyMapping( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnGetMappingParameters( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBitmapClickPage3( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnCoordSet( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCoordText( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRemoveCoords( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSpin( wxSpinEvent& event ) { event.Skip(); }
		
	
	public:
		
		EditFaxWizardBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Edit Fax Image"), const wxBitmap& bitmap = wxNullBitmap, const wxPoint& pos = wxDefaultPosition, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		WizardPages m_pages;
		~EditFaxWizardBase();
	
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
