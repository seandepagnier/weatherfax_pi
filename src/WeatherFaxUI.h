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
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/slider.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/filepicker.h>
#include <wx/radiobut.h>
#include <wx/notebook.h>
#include <wx/gauge.h>
#include <wx/dialog.h>
#include <wx/splitter.h>
#include <wx/scrolwin.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/wizard.h>
#include <wx/dynarray.h>
WX_DEFINE_ARRAY_PTR( wxWizardPageSimple*, WizardPages );

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class WeatherFaxBase
///////////////////////////////////////////////////////////////////////////////
class WeatherFaxBase : public wxFrame 
{
	private:
	
	protected:
		wxCheckBox* m_cInvert;
		wxMenuBar* m_menubar1;
		wxMenu* m_menu1;
		wxMenuItem* m_mEdit;
		wxMenuItem* m_mGoto;
		wxMenuItem* m_mExport;
		wxMenuItem* m_mDelete;
		wxMenu* m_menu2;
		wxMenuItem* m_mAudioCapture;
		wxMenu* m_menu3;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnFaxes( wxCommandEvent& event ) { event.Skip(); }
		virtual void TransparencyChanged( wxScrollEvent& event ) { event.Skip(); }
		virtual void WhiteTransparencyChanged( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnInvert( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOpen( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEdit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnGoto( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnExport( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDelete( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPreferences( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClose( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCapture( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSchedules( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnInternet( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxStaticBoxSizer* sbFax;
		wxListBox* m_lFaxes;
		wxSlider* m_sTransparency;
		wxSlider* m_sWhiteTransparency;
		
		WeatherFaxBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Weather Fax"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxFRAME_FLOAT_ON_PARENT|wxFRAME_NO_TASKBAR|wxRESIZE_BORDER|wxSYSTEM_MENU|wxTAB_TRAVERSAL );
		
		~WeatherFaxBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SchedulesDialogBase
///////////////////////////////////////////////////////////////////////////////
class SchedulesDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxListCtrl* m_lSchedules;
		wxNotebook* m_notebook1;
		wxPanel* m_panel1;
		wxStaticText* m_staticText24;
		wxTextCtrl* m_tContainsLat;
		wxStaticText* m_staticText25;
		wxTextCtrl* m_tContainsLon;
		wxButton* m_bBoatPosition;
		wxButton* m_bReset;
		wxListBox* m_lStations;
		wxButton* m_bAll;
		wxButton* m_bNone;
		wxSpinCtrl* m_skhzmin;
		wxStaticText* m_staticText27;
		wxSpinCtrl* m_skhzmax;
		wxStaticText* m_staticText28;
		wxButton* m_bAllFrequencies;
		wxCheckBox* m_cbHasArea;
		wxCheckBox* m_cbHasValidTime;
		wxButton* m_bClearCaptures;
		wxPanel* m_panel2;
		wxCheckBox* m_cbMessageBox;
		wxCheckBox* m_cbSound;
		wxFilePickerCtrl* m_fpSound;
		wxCheckBox* m_cbExternalAlarm;
		wxTextCtrl* m_tExternalAlarmCommand;
		wxPanel* m_panel3;
		wxRadioButton* m_rbNoAction;
		wxRadioButton* m_rbAudioCapture;
		wxRadioButton* m_rbExternalCapture;
		wxTextCtrl* m_tExternalCapture;
		wxRadioButton* m_rbManualCapture;
		wxPanel* m_panel7;
		wxStaticText* m_staticText33;
		wxStaticText* m_stCaptureStatus;
		wxGauge* m_gCaptureStatus;
		wxButton* m_bClose;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnSchedulesLeftDown( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnSchedulesSort( wxListEvent& event ) { event.Skip(); }
		virtual void OnFilter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBoatPosition( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnReset( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAllStations( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNoStations( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFilterSpin( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnAllFrequencies( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClearCaptures( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClose( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		SchedulesDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("HF Radio Schedules"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER ); 
		~SchedulesDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class InternetRetrievalDialogBase
///////////////////////////////////////////////////////////////////////////////
class InternetRetrievalDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxNotebook* m_notebook2;
		wxPanel* m_panel4;
		wxSplitterWindow* m_splitter1;
		wxPanel* m_panel7;
		wxListCtrl* m_lUrls;
		wxPanel* m_panel8;
		wxStaticText* m_staticText24;
		wxTextCtrl* m_tContainsLat;
		wxStaticText* m_staticText25;
		wxTextCtrl* m_tContainsLon;
		wxButton* m_bBoatPosition;
		wxButton* m_bReset;
		wxButton* m_bRetrieveScheduled;
		wxButton* m_bRetrieveSelected;
		wxListBox* m_lServers;
		wxButton* m_bAllServers;
		wxButton* m_bNoServers;
		wxListBox* m_lRegions;
		wxButton* m_bAllRegions;
		wxButton* m_bNoRegions;
		wxPanel* m_panel5;
		wxStaticText* m_staticText34;
		wxStaticText* m_staticText41;
		wxStaticText* m_staticText42;
		wxTextCtrl* m_textCtrl11;
		wxTextCtrl* m_textCtrl14;
		wxStaticText* m_staticText421;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnRetrieve( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnUrlsLeftDown( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnUrlsSort( wxListEvent& event ) { event.Skip(); }
		virtual void OnUrlSelected( wxListEvent& event ) { event.Skip(); }
		virtual void OnFilter( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBoatPosition( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnReset( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRetrieve( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFilterServers( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAllServers( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNoServers( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnFilterRegions( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAllRegions( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNoRegions( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		InternetRetrievalDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Internet Retrieval"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 680,480 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~InternetRetrievalDialogBase();
		
		void m_splitter1OnIdle( wxIdleEvent& )
		{
			m_splitter1->SetSashPosition( 300 );
			m_splitter1->Disconnect( wxEVT_IDLE, wxIdleEventHandler( InternetRetrievalDialogBase::m_splitter1OnIdle ), NULL, this );
		}
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class WeatherFaxWizardBase
///////////////////////////////////////////////////////////////////////////////
class WeatherFaxWizardBase : public wxWizard 
{
	private:
	
	protected:
		wxScrolledWindow* m_swFaxArea1;
		wxButton* m_bStopDecoding;
		wxButton* m_bDecoderOptions;
		wxScrolledWindow* m_bPhasingArea;
		wxStaticText* m_staticText16;
		wxChoice* m_cFilter;
		wxStaticText* m_staticText9;
		wxSlider* m_sPhasing;
		wxStaticText* m_staticText17;
		wxChoice* m_cRotation;
		wxStaticText* m_staticText101;
		wxSlider* m_sSkew;
		wxFlexGridSizer* m_fgSizerUnMappedCoords;
		wxComboBox* m_cbCoordSet;
		wxButton* m_bRemoveCoordSet;
		wxFlexGridSizer* m_fgSizerLatLonUnMapped;
		wxRadioButton* m_rbCoord1UnMapped;
		wxStaticText* m_staticText6;
		wxStaticText* m_stMinutes;
		wxSpinCtrl* m_sCoord1YUnMapped;
		wxSpinCtrl* m_sCoord1LatUnMapped;
		wxTextCtrl* m_tCoord1LatMinutesUnMapped;
		wxSpinCtrl* m_sCoord1XUnMapped;
		wxSpinCtrl* m_sCoord1LonUnMapped;
		wxTextCtrl* m_tCoord1LonMinutesUnMapped;
		wxRadioButton* m_rbCoord2UnMapped;
		wxStaticText* m_staticText8;
		wxSpinCtrl* m_sCoord2YUnMapped;
		wxSpinCtrl* m_sCoord2LatUnMapped;
		wxTextCtrl* m_tCoord2LatMinutesUnMapped;
		wxSpinCtrl* m_sCoord2XUnMapped;
		wxSpinCtrl* m_sCoord2LonUnMapped;
		wxTextCtrl* m_tCoord2LonMinutesUnMapped;
		wxCheckBox* m_cbShowLatLonMinutes;
		wxStaticText* m_staticText43;
		wxScrolledWindow* m_swFaxArea2;
		wxStaticText* m_staticText15;
		wxChoice* m_cMapping;
		wxStaticText* m_staticText21;
		wxStaticText* m_staticText22;
		wxStaticText* m_staticText24;
		wxTextCtrl* m_tMappingMultiplier;
		wxStaticText* m_staticText20;
		wxTextCtrl* m_tMappingRatio;
		wxStaticText* m_staticText211;
		wxTextCtrl* m_tTrueRatio;
		wxStaticText* m_stMappingLabel1;
		wxSpinCtrl* m_sMappingPoleX;
		wxStaticText* m_stMappingLabel2;
		wxSpinCtrl* m_sMappingPoleY;
		wxStaticText* m_stMapping;
		wxSpinCtrl* m_sMappingEquatorY;
		wxButton* m_bGetMapping;
		wxButton* m_bGetEquator;
		wxButton* m_bGetAspectRatio;
		wxButton* m_bInformation;
		wxRadioButton* m_rbCoord1;
		wxStaticText* m_staticText61;
		wxSpinCtrl* m_sCoord1Y;
		wxTextCtrl* m_tCoord1Lat;
		wxSpinCtrl* m_sCoord1X;
		wxTextCtrl* m_tCoord1Lon;
		wxRadioButton* m_rbCoord2;
		wxStaticText* m_staticText81;
		wxSpinCtrl* m_sCoord2Y;
		wxTextCtrl* m_tCoord2Lat;
		wxSpinCtrl* m_sCoord2X;
		wxTextCtrl* m_tCoord2Lon;
		wxScrolledWindow* m_swFaxArea3;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnSetSizes( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnWizardCancel( wxWizardEvent& event ) { event.Skip(); }
		virtual void OnWizardFinished( wxWizardEvent& event ) { event.Skip(); }
		virtual void OnWizardPageChanged( wxWizardEvent& event ) { event.Skip(); }
		virtual void OnBitmapClickPage1( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnPaintImage( wxPaintEvent& event ) { event.Skip(); }
		virtual void OnStopDecoding( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDecoderOptions( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPaintPhasing( wxPaintEvent& event ) { event.Skip(); }
		virtual void UpdatePage1( wxCommandEvent& event ) { event.Skip(); }
		virtual void UpdatePage1Scroll( wxScrollEvent& event ) { event.Skip(); }
		virtual void UpdatePage1Rotation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCoordSet( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCoordText( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRemoveCoords( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSpin( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnShowLatLonMinutes( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBitmapClickPage2( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnMappingChoice( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnUpdateMapping( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnUpdateMappingSpin( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnGetMapping( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnGetEquator( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnGetAspectRatio( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnInformation( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnBitmapClickPage3( wxMouseEvent& event ) { event.Skip(); }
		
	
	public:
		wxFlexGridSizer* m_fgSizer434;
		
		WeatherFaxWizardBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Weather Fax Image"), const wxBitmap& bitmap = wxNullBitmap, const wxPoint& pos = wxDefaultPosition, long style = wxCAPTION|wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER|wxSYSTEM_MENU );
		WizardPages m_pages;
		~WeatherFaxWizardBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class WeatherFaxPrefsDialog
///////////////////////////////////////////////////////////////////////////////
class WeatherFaxPrefsDialog : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText36;
		wxStaticText* m_staticText37;
		wxStaticText* m_staticText45;
		wxStaticText* m_staticText38;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
		wxCheckBox* m_cbLoadSchedulesStart;
		wxSpinCtrl* m_sExportColors;
		wxRadioButton* m_rbExportDepthMeters;
		wxRadioButton* m_rbExportDepthFathoms;
		wxTextCtrl* m_tExportSoundingDatum;
		
		WeatherFaxPrefsDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Weather Fax Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE ); 
		~WeatherFaxPrefsDialog();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DecoderOptionsDialogBase
///////////////////////////////////////////////////////////////////////////////
class DecoderOptionsDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText29;
		wxStaticText* m_staticText30;
		wxStaticText* m_staticText31;
		wxStaticText* m_staticText32;
		wxStaticText* m_staticText33;
		wxStaticText* m_staticText41;
		wxStaticText* m_staticText39;
		wxChoice* m_cSampleRate;
		wxButton* m_bDone;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnOptionsSpin( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnResetOptions( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOptions( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDone( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxSpinCtrl* m_sImageWidth;
		wxSpinCtrl* m_sBitsPerPixel;
		wxSpinCtrl* m_sCarrier;
		wxSpinCtrl* m_sDeviation;
		wxChoice* m_cFilter;
		wxSpinCtrl* m_sMinusSaturationThreshold;
		wxCheckBox* m_cbSkip;
		wxCheckBox* m_cbInclude;
		
		DecoderOptionsDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Fax Decoding Options"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~DecoderOptionsDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class AboutDialogBase
///////////////////////////////////////////////////////////////////////////////
class AboutDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText34;
		wxStaticText* m_stVersion;
		wxStaticText* m_staticText110;
		wxButton* m_bAboutAuthor;
		wxButton* m_bClose;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnAboutAuthor( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnClose( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		AboutDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("About Weatherfax"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~AboutDialogBase();
	
};

#endif //__WEATHERFAXUI_H__
