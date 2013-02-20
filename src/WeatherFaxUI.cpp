///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "WeatherFaxUI.h"

///////////////////////////////////////////////////////////////////////////

WeatherFaxDialogBase::WeatherFaxDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer8->AddGrowableCol( 0 );
	fgSizer8->AddGrowableRow( 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	sbFax = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Fax") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer7->AddGrowableCol( 0 );
	fgSizer7->AddGrowableRow( 0 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_lFaxes = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer7->Add( m_lFaxes, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	
	m_bNew = new wxButton( this, wxID_ANY, _("New"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_bNew, 0, wxALL, 5 );
	
	m_bEdit = new wxButton( this, wxID_ANY, _("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_bEdit, 0, wxALL, 5 );
	
	m_bDelete = new wxButton( this, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( m_bDelete, 0, wxALL, 5 );
	
	
	fgSizer7->Add( bSizer4, 1, wxEXPAND, 5 );
	
	
	sbFax->Add( fgSizer7, 1, wxEXPAND, 5 );
	
	
	fgSizer8->Add( sbFax, 1, wxEXPAND, 0 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Transparency") ), wxVERTICAL );
	
	m_sTransparency = new wxSlider( this, wxID_ANY, 50, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	sbSizer4->Add( m_sTransparency, 0, wxEXPAND, 5 );
	
	m_sWhiteTransparency = new wxSlider( this, wxID_ANY, 50, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	sbSizer4->Add( m_sWhiteTransparency, 0, wxEXPAND, 5 );
	
	m_cInvert = new wxCheckBox( this, wxID_ANY, _("Invert"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer4->Add( m_cInvert, 0, wxALL, 5 );
	
	
	fgSizer8->Add( sbSizer4, 1, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer8 );
	this->Layout();
	fgSizer8->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_lFaxes->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( WeatherFaxDialogBase::OnListBox ), NULL, this );
	m_bNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WeatherFaxDialogBase::NewFaxClicked ), NULL, this );
	m_bEdit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WeatherFaxDialogBase::EditFaxClicked ), NULL, this );
	m_bDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WeatherFaxDialogBase::DeleteFaxClicked ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_cInvert->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WeatherFaxDialogBase::OnInvert ), NULL, this );
}

WeatherFaxDialogBase::~WeatherFaxDialogBase()
{
	// Disconnect Events
	m_lFaxes->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( WeatherFaxDialogBase::OnListBox ), NULL, this );
	m_bNew->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WeatherFaxDialogBase::NewFaxClicked ), NULL, this );
	m_bEdit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WeatherFaxDialogBase::EditFaxClicked ), NULL, this );
	m_bDelete->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WeatherFaxDialogBase::DeleteFaxClicked ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( WeatherFaxDialogBase::TransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_sWhiteTransparency->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( WeatherFaxDialogBase::WhiteTransparencyChanged ), NULL, this );
	m_cInvert->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WeatherFaxDialogBase::OnInvert ), NULL, this );
	
}

EditFaxDialogBase::EditFaxDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer14;
	fgSizer14 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer14->AddGrowableCol( 0 );
	fgSizer14->AddGrowableRow( 0 );
	fgSizer14->SetFlexibleDirection( wxBOTH );
	fgSizer14->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_swFaxArea = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_swFaxArea->SetScrollRate( 5, 5 );
	fgSizer14->Add( m_swFaxArea, 1, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Configure Image Coordinates") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer9;
	fgSizer9 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer9->AddGrowableRow( 1 );
	fgSizer9->SetFlexibleDirection( wxBOTH );
	fgSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText10 = new wxStaticText( this, wxID_ANY, _("Coord Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	fgSizer8->Add( m_staticText10, 0, wxALL, 5 );
	
	m_cbCoordSet = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer8->Add( m_cbCoordSet, 0, wxALL, 5 );
	
	m_bRemoveCoordSet = new wxButton( this, wxID_ANY, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer8->Add( m_bRemoveCoordSet, 0, wxALL, 5 );
	
	
	fgSizer9->Add( fgSizer8, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 0, 6, 0, 0 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_rbCoord1 = new wxRadioButton( this, wxID_ANY, _("Coord 1 X/Y"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	fgSizer7->Add( m_rbCoord1, 0, wxALL, 5 );
	
	m_sCoord1X = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer7->Add( m_sCoord1X, 0, wxALL, 5 );
	
	m_sCoord1Y = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer7->Add( m_sCoord1Y, 0, wxALL, 5 );
	
	m_staticText6 = new wxStaticText( this, wxID_ANY, _("Lat/Lon"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	fgSizer7->Add( m_staticText6, 0, wxALL, 5 );
	
	m_sCoord1Lat = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -90, 90, 0 );
	fgSizer7->Add( m_sCoord1Lat, 0, wxALL, 5 );
	
	m_sCoord1Lon = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -180, 180, 0 );
	fgSizer7->Add( m_sCoord1Lon, 0, wxALL, 5 );
	
	m_rbCoord2 = new wxRadioButton( this, wxID_ANY, _("Coord 2 X/Y"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_rbCoord2, 0, wxALL, 5 );
	
	m_sCoord2X = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer7->Add( m_sCoord2X, 0, wxALL, 5 );
	
	m_sCoord2Y = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	fgSizer7->Add( m_sCoord2Y, 0, wxALL, 5 );
	
	m_staticText8 = new wxStaticText( this, wxID_ANY, _("Lat/Lon"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	fgSizer7->Add( m_staticText8, 0, wxALL, 5 );
	
	m_sCoord2Lat = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -90, 90, 0 );
	fgSizer7->Add( m_sCoord2Lat, 0, wxALL, 5 );
	
	m_sCoord2Lon = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -180, 180, 0 );
	fgSizer7->Add( m_sCoord2Lon, 0, wxALL, 5 );
	
	
	fgSizer9->Add( fgSizer7, 1, wxEXPAND, 5 );
	
	
	sbSizer4->Add( fgSizer9, 1, 0, 5 );
	
	
	fgSizer14->Add( sbSizer4, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 1, 0, 0, 0 );
	fgSizer10->AddGrowableCol( 3 );
	fgSizer10->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_bSplitImage = new wxButton( this, wxID_ANY, _("Split Image"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_bSplitImage, 0, wxALL, 5 );
	
	m_bPolarToMercator = new wxButton( this, wxID_ANY, _("Polar->Mercator"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_bPolarToMercator, 0, wxALL, 5 );
	
	wxString m_cFilterChoices[] = { _("No Filter"), _("Lowpass 1"), _("Lowpass 2") };
	int m_cFilterNChoices = sizeof( m_cFilterChoices ) / sizeof( wxString );
	m_cFilter = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cFilterNChoices, m_cFilterChoices, 0 );
	m_cFilter->SetSelection( 0 );
	fgSizer10->Add( m_cFilter, 0, wxALL, 5 );
	
	m_staticText9 = new wxStaticText( this, wxID_ANY, _("Phasing"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	fgSizer10->Add( m_staticText9, 0, wxALL, 5 );
	
	m_sPhasing = new wxSlider( this, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_INVERSE );
	fgSizer10->Add( m_sPhasing, 0, wxALL|wxEXPAND, 5 );
	
	m_sdbSizer3 = new wxStdDialogButtonSizer();
	m_sdbSizer3OK = new wxButton( this, wxID_OK );
	m_sdbSizer3->AddButton( m_sdbSizer3OK );
	m_sdbSizer3->Realize();
	
	fgSizer10->Add( m_sdbSizer3, 1, wxEXPAND, 5 );
	
	
	fgSizer14->Add( fgSizer10, 1, wxEXPAND|wxFIXED_MINSIZE, 5 );
	
	
	this->SetSizer( fgSizer14 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_swFaxArea->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( EditFaxDialogBase::OnBitmapClick ), NULL, this );
	m_swFaxArea->Connect( wxEVT_PAINT, wxPaintEventHandler( EditFaxDialogBase::OnPaintImage ), NULL, this );
	m_cbCoordSet->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( EditFaxDialogBase::OnCoordSet ), NULL, this );
	m_cbCoordSet->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( EditFaxDialogBase::OnCoordText ), NULL, this );
	m_bRemoveCoordSet->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditFaxDialogBase::OnRemoveCoords ), NULL, this );
	m_sCoord1X->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( EditFaxDialogBase::OnSpin ), NULL, this );
	m_sCoord1Y->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( EditFaxDialogBase::OnSpin ), NULL, this );
	m_sCoord2X->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( EditFaxDialogBase::OnSpin ), NULL, this );
	m_sCoord2Y->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( EditFaxDialogBase::OnSpin ), NULL, this );
	m_bSplitImage->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditFaxDialogBase::OnSplitImage ), NULL, this );
	m_bPolarToMercator->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditFaxDialogBase::OnPolarToMercator ), NULL, this );
	m_sPhasing->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
}

EditFaxDialogBase::~EditFaxDialogBase()
{
	// Disconnect Events
	m_swFaxArea->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( EditFaxDialogBase::OnBitmapClick ), NULL, this );
	m_swFaxArea->Disconnect( wxEVT_PAINT, wxPaintEventHandler( EditFaxDialogBase::OnPaintImage ), NULL, this );
	m_cbCoordSet->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( EditFaxDialogBase::OnCoordSet ), NULL, this );
	m_cbCoordSet->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( EditFaxDialogBase::OnCoordText ), NULL, this );
	m_bRemoveCoordSet->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditFaxDialogBase::OnRemoveCoords ), NULL, this );
	m_sCoord1X->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( EditFaxDialogBase::OnSpin ), NULL, this );
	m_sCoord1Y->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( EditFaxDialogBase::OnSpin ), NULL, this );
	m_sCoord2X->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( EditFaxDialogBase::OnSpin ), NULL, this );
	m_sCoord2Y->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( EditFaxDialogBase::OnSpin ), NULL, this );
	m_bSplitImage->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditFaxDialogBase::OnSplitImage ), NULL, this );
	m_bPolarToMercator->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( EditFaxDialogBase::OnPolarToMercator ), NULL, this );
	m_sPhasing->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	m_sPhasing->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( EditFaxDialogBase::OnPhasing ), NULL, this );
	
}

WeatherFaxPrefsDialog::WeatherFaxPrefsDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Audio Decoding Options") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer10->SetFlexibleDirection( wxBOTH );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText29 = new wxStaticText( this, wxID_ANY, _("Image Width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText29->Wrap( -1 );
	fgSizer10->Add( m_staticText29, 0, wxALL, 5 );
	
	m_sImageWidth = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 16384, 576 );
	fgSizer10->Add( m_sImageWidth, 0, wxALL, 5 );
	
	m_staticText30 = new wxStaticText( this, wxID_ANY, _("Bits per Pixel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText30->Wrap( -1 );
	fgSizer10->Add( m_staticText30, 0, wxALL, 5 );
	
	m_sBitsPerPixel = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 8, 8 );
	fgSizer10->Add( m_sBitsPerPixel, 0, wxALL, 5 );
	
	m_staticText31 = new wxStaticText( this, wxID_ANY, _("Carrier"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText31->Wrap( -1 );
	fgSizer10->Add( m_staticText31, 0, wxALL, 5 );
	
	m_sCarrier = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10000, 1900 );
	fgSizer10->Add( m_sCarrier, 0, wxALL, 5 );
	
	m_staticText32 = new wxStaticText( this, wxID_ANY, _("Deviation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText32->Wrap( -1 );
	fgSizer10->Add( m_staticText32, 0, wxALL, 5 );
	
	m_sDeviation = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10000, 400 );
	fgSizer10->Add( m_sDeviation, 0, wxALL, 5 );
	
	m_staticText33 = new wxStaticText( this, wxID_ANY, _("Filter"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText33->Wrap( -1 );
	fgSizer10->Add( m_staticText33, 0, wxALL, 5 );
	
	wxString m_cFilterChoices[] = { _("narrow"), _("middle"), _("wide") };
	int m_cFilterNChoices = sizeof( m_cFilterChoices ) / sizeof( wxString );
	m_cFilter = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cFilterNChoices, m_cFilterChoices, 0 );
	m_cFilter->SetSelection( 1 );
	fgSizer10->Add( m_cFilter, 0, wxALL, 5 );
	
	
	fgSizer12->Add( fgSizer10, 1, wxEXPAND, 5 );
	
	m_cbSkip = new wxCheckBox( this, wxID_ANY, _("Skip start, stop and \nphasing detection"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer12->Add( m_cbSkip, 0, wxALL, 5 );
	
	m_cbInclude = new wxCheckBox( this, wxID_ANY, _("Include header data in image"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer12->Add( m_cbInclude, 0, wxALL, 5 );
	
	
	sbSizer6->Add( fgSizer12, 1, wxEXPAND, 5 );
	
	
	bSizer2->Add( sbSizer6, 1, wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	
	bSizer2->Add( m_sdbSizer1, 0, wxBOTTOM|wxEXPAND|wxTOP, 5 );
	
	
	this->SetSizer( bSizer2 );
	this->Layout();
	bSizer2->Fit( this );
	
	this->Centre( wxBOTH );
}

WeatherFaxPrefsDialog::~WeatherFaxPrefsDialog()
{
}
