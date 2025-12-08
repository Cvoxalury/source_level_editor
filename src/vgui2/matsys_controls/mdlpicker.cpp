//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "matsys_controls/mdlpicker.h"
#include "tier1/KeyValues.h"
#include "tier1/utldict.h"
#include "filesystem.h"
#include "studio.h"
#include "matsys_controls/matsyscontrols.h"
#include "matsys_controls/mdlpanel.h"
#include "vgui_controls/Splitter.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/MessageBox.h"
#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/DirectorySelectDialog.h"
#include "vgui/IVGui.h"
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "vgui/Cursor.h"
#include "matsys_controls/assetpicker.h"
#include "matsys_controls/colorpickerpanel.h"
#include "dmxloader/dmxloader.h"
#include "tier1/utlbuffer.h"
#include "bitmap/tgawriter.h"
#include "tier3/tier3.h"
#include "istudiorender.h"
#include "../vgui2/src/VPanel.h"
#include "tier2/p4helpers.h"
#include "ivtex.h"
#include "bitmap/tgaloader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//#define MODELBROWSER_USES_GRID_VIEW

using namespace vgui;

//-----------------------------------------------------------------------------
//
// MDL Picker
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Sort by MDL name
//-----------------------------------------------------------------------------
static int __cdecl MDLBrowserSortFunc( vgui::ListPanel *pPanel, const ListPanelItem &item1, const ListPanelItem &item2 )
{
	const char *string1 = item1.kv->GetString("mdl");
	const char *string2 = item2.kv->GetString("mdl");
	return stricmp( string1, string2 );
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMDLPicker::CMDLPicker( vgui::Panel *pParent, int nFlags ) : 
	BaseClass( pParent, "MDL Files", "mdl", "models", "mdlName" )
{
	for( int i = 0; i < MAX_SELECTED_MODELS; i++ )
	{
		m_hSelectedMDL[ i ] = MDLHANDLE_INVALID;
	}

	m_nFlags = nFlags;	// remember what we show and what not

	m_pRenderPage = NULL;
	m_pSequencesPage = NULL;
	m_pActivitiesPage = NULL;
	m_pSkinsPage = NULL;
	m_pInfoPage = NULL;
	m_pAttachmentsPage = NULL; //// SLE NEW - attachments tab

	m_pSequencesList = NULL;
	m_pActivitiesList = NULL;
	m_pAttachmentsList = NULL;  //// SLE NEW - attachments tab

	m_hDirectorySelectDialog = NULL;

	// Horizontal splitter for mdls
	m_pFileBrowserSplitter = new Splitter( this, "FileBrowserSplitter", SPLITTER_MODE_VERTICAL, 1 );

	float flFractions[] = { 0.33f, 0.67f };

	m_pFileBrowserSplitter->RespaceSplitters( flFractions );

	vgui::Panel *pSplitterLeftSide = m_pFileBrowserSplitter->GetChild( 0 );
	vgui::Panel *pSplitterRightSide = m_pFileBrowserSplitter->GetChild( 1 );

	// Standard browser controls
	pSplitterLeftSide->RequestFocus();
	CreateStandardControls( pSplitterLeftSide, false );

	// property sheet - revisions, changes, etc.
	m_pPreviewSplitter = new Splitter( pSplitterRightSide, "PreviewSplitter", SPLITTER_MODE_HORIZONTAL, 1 );

	vgui::Panel *pSplitterTopSide = m_pPreviewSplitter->GetChild( 0 );
	vgui::Panel *pSplitterBottomSide = m_pPreviewSplitter->GetChild( 1 );

	// MDL preview
	m_pMDLPreview = new CMDLPanel( pSplitterTopSide, "MDLPreview" );
#ifdef MODELBROWSER_USES_GRID_VIEW
	m_pMDLPreview->SetAutoResize(PIN_TOPLEFT, AUTORESIZE_RIGHT, 0, 0, -16, 0);
	pSplitterTopSide->SetSizer(new vgui::CBoxSizer(vgui::ESLD_VERTICAL));
	pSplitterTopSide->GetSizer()->AddPanel(m_pMDLPreview, vgui::SizerAddArgs_t().Expand(1.0f));
#endif
	SetSkipChildDuringPainting( m_pMDLPreview );

	m_pViewsSheet = new vgui::PropertySheet( pSplitterBottomSide, "ViewsSheet" );
#ifdef MODELBROWSER_USES_GRID_VIEW
	m_pViewsSheet->SetAutoResize(PIN_TOPLEFT, AUTORESIZE_RIGHT, 0, 0, -16, 0);
	pSplitterBottomSide->SetSizer(new vgui::CBoxSizer(vgui::ESLD_VERTICAL));
	pSplitterBottomSide->GetSizer()->AddPanel(m_pViewsSheet, vgui::SizerAddArgs_t().Expand(1.0f));
#endif
	m_pViewsSheet->AddActionSignalTarget( this );

	// now add wanted features
	if ( nFlags & PAGE_RENDER )
	{
		m_pRenderPage = new vgui::PropertyPage( m_pViewsSheet, "RenderPage" );

		m_pRenderPage->AddActionSignalTarget( this );
#ifdef MODELBROWSER_USES_GRID_VIEW
		auto s = new vgui::CBoxSizer(vgui::ESLD_HORIZONTAL);
		auto s2 = new vgui::CBoxSizer(vgui::ESLD_VERTICAL);
		auto s3 = new vgui::CBoxSizer(vgui::ESLD_VERTICAL);

		s2->AddSpacer(vgui::SizerAddArgs_t().Padding(2));
		s2->AddPanel(new vgui::CheckButton(m_pRenderPage, "Wireframe", "Wireframe"), vgui::SizerAddArgs_t().Padding(0));
		s2->AddPanel(new vgui::CheckButton(m_pRenderPage, "Collision", "Collision Model"), vgui::SizerAddArgs_t().Padding(0));
		s2->AddPanel(new vgui::CheckButton(m_pRenderPage, "NoGround", "No Ground"), vgui::SizerAddArgs_t().Padding(0));
		s2->AddPanel(new vgui::CheckButton(m_pRenderPage, "LockView", "Lock View"), vgui::SizerAddArgs_t().Padding(0));

		s3->AddSpacer(vgui::SizerAddArgs_t().Padding(2));
		s3->AddPanel(new vgui::CheckButton(m_pRenderPage, "LookAtCamera", "Look At Camera"), vgui::SizerAddArgs_t().Padding(0));
		s3->AddPanel(new vgui::Button(m_pRenderPage, "ChooseLightProbe", "&Select Light Probe", this, "ChooseLightProbe"), vgui::SizerAddArgs_t().Padding(0));

		s->AddSizer(s2, vgui::SizerAddArgs_t().Padding(0));
		s->AddSizer(s3, vgui::SizerAddArgs_t().Padding(0));

		m_pRenderPage->SetSizer(s);
#endif

		m_pRenderPage->LoadUserConfig("level_editor/resource/level_editor_model_browser_render.res");
		m_pRenderPage->LoadControlSettings("level_editor/resource/level_editor_model_browser_render.res", "EXECUTABLE_PATH");
       // m_pRenderPage->LoadControlSettingsAndUserConfig( "level_editor/resource/level_editor_model_browser_render.res" ); //// SLE CHANGE - move new model browser scheme to new file

		RefreshRenderSettings();

		// ground
	//	Button *pSelectProbe = (Button*)m_pRenderPage->FindChildByName( "ChooseLightProbe" ); //// SLE REMOVE - didn't work
	//	pSelectProbe->AddActionSignalTarget( this );
	}

	if ( nFlags & PAGE_SEQUENCES )
	{
		m_pSequencesPage = new vgui::PropertyPage( m_pViewsSheet, "SequencesPage" );

		m_pSequencesList = new vgui::ListPanel( m_pSequencesPage, "SequencesList" );
		m_pSequencesList->AddColumnHeader( 0, "sequence", "sequence", 52, 0 );
		m_pSequencesList->AddActionSignalTarget( this );
		m_pSequencesList->SetSelectIndividualCells( true );
		m_pSequencesList->SetEmptyListText("No .MDL file currently selected.");
		m_pSequencesList->SetDragEnabled(false);
		m_pSequencesList->SetAutoResize( Panel::PIN_TOPLEFT, Panel::AUTORESIZE_DOWNANDRIGHT, 6, 6, -6, -6 );
	}

	if ( nFlags & PAGE_ACTIVITIES )
	{
		m_pActivitiesPage = new vgui::PropertyPage( m_pViewsSheet, "ActivitiesPage" );
		m_pActivitiesList = new vgui::ListPanel( m_pActivitiesPage, "ActivitiesList" );
		m_pActivitiesList->AddColumnHeader( 0, "activity", "activity", 52, 0 );
		m_pActivitiesList->AddActionSignalTarget( this );
		m_pActivitiesList->SetSelectIndividualCells( true );
		m_pActivitiesList->SetEmptyListText( "No .MDL file currently selected." );
		m_pActivitiesList->SetDragEnabled(false);
		m_pActivitiesList->SetAutoResize( Panel::PIN_TOPLEFT, Panel::AUTORESIZE_DOWNANDRIGHT, 6, 6, -6, -6 );
	}

	if ( nFlags & PAGE_SKINS )
	{
		m_pSkinsPage = new vgui::PropertyPage( m_pViewsSheet, "SkinsPage" );

		m_pSkinsList = new vgui::ListPanel( m_pSkinsPage, "SkinsList" );
		m_pSkinsList->AddColumnHeader( 0, "skin", "skin", 52, 0 );
		m_pSkinsList->AddActionSignalTarget( this );
		m_pSkinsList->SetSelectIndividualCells( true );
		m_pSkinsList->SetEmptyListText( "No .MDL file currently selected." );
		m_pSkinsList->SetDragEnabled(false);
		m_pSkinsList->SetAutoResize( Panel::PIN_TOPLEFT, Panel::AUTORESIZE_DOWNANDRIGHT, 6, 6, -6, -6 );		
	}

	if ( nFlags & PAGE_INFO )
	{
		m_pInfoPage = new vgui::PropertyPage( m_pViewsSheet, "InfoPage" );

		m_pInfoPage->AddActionSignalTarget( this );
#ifdef MODELBROWSER_USES_GRID_VIEW
		auto s = new CBoxSizer(vgui::ESLD_VERTICAL);
		auto s2 = new CBoxSizer(vgui::ESLD_HORIZONTAL);
		
		vgui::CheckButton* btn;
		s2->AddPanel(btn = new CheckButton(m_pInfoPage, "PhysicsObject", "physics"), vgui::SizerAddArgs_t().Padding(2));
		btn->MakeReadyForUse();
		btn->SetEnabled(false);
		btn->SetDisabledFgColor2(btn->GetFgColor());
		s2->AddPanel(btn = new CheckButton(m_pInfoPage, "DynamicObject", "dynamic"), vgui::SizerAddArgs_t().Padding(2));
		btn->MakeReadyForUse();
		btn->SetEnabled(false);
		btn->SetDisabledFgColor2(btn->GetFgColor());
		s2->AddPanel(btn = new CheckButton(m_pInfoPage, "StaticObject", "static"), vgui::SizerAddArgs_t().Padding(2));
		btn->MakeReadyForUse();
		btn->SetEnabled(false);
		btn->SetDisabledFgColor2(btn->GetFgColor());
		s2->AddSpacer(vgui::SizerAddArgs_t().Padding(2));
		s2->AddPanel(new Label(m_pInfoPage, "StaticText", "Model is missing $staticprop entry"), vgui::SizerAddArgs_t().Padding(2));
		s2->AddSpacer(vgui::SizerAddArgs_t().Padding(2));
		s2->AddSpacer(vgui::SizerAddArgs_t().Expand(1.0f).Padding(2));
		s2->AddPanel(new Label(m_pInfoPage, "Mass", "Mass"), vgui::SizerAddArgs_t().Padding(2));
		s2->AddSpacer(vgui::SizerAddArgs_t().Padding(2));
		s2->AddPanel(new Label(m_pInfoPage, "MassValue", "0"), vgui::SizerAddArgs_t().MinX(32).Padding(2));
		s2->AddSpacer(vgui::SizerAddArgs_t().Padding(2));

		s->AddSizer(s2, vgui::SizerAddArgs_t().Padding(0));
		m_pInfoPage->LoadUserConfig("level_editor/resource/level_editor_model_browser_info.res");
		m_pInfoPage->LoadControlSettings("level_editor/resource/level_editor_model_browser_info.res", "EXECUTABLE_PATH");
#else
		m_pInfoPage->LoadUserConfig("level_editor/resource/level_editor_model_browser_info.res");
		m_pInfoPage->LoadControlSettings("level_editor/resource/level_editor_model_browser_info.res", "EXECUTABLE_PATH");
	//	m_pInfoPage->LoadControlSettingsAndUserConfig( "level_editor/resource/level_editor_model_browser_info.res" ); //// SLE CHANGE - move new model browser scheme to new file

		CheckButton * pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName( "PhysicsObject" );
		pTempCheck->SetDisabledFgColor1( pTempCheck->GetFgColor());
		pTempCheck->SetDisabledFgColor2( pTempCheck->GetFgColor());
		pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName( "StaticObject" );
		pTempCheck->SetDisabledFgColor1( pTempCheck->GetFgColor());
		pTempCheck->SetDisabledFgColor2( pTempCheck->GetFgColor());
		pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName( "DynamicObject" );
		pTempCheck->SetDisabledFgColor1( pTempCheck->GetFgColor());
		pTempCheck->SetDisabledFgColor2( pTempCheck->GetFgColor());
#endif
		m_pPropDataList = new vgui::ListPanel( m_pInfoPage, "PropData" );
		m_pPropDataList->AddColumnHeader( 0, "key", "key", 250, ListPanel::COLUMN_FIXEDSIZE );		
		m_pPropDataList->AddColumnHeader( 1, "value", "value", 52, 0 );
		m_pPropDataList->AddActionSignalTarget( this );
		m_pPropDataList->SetSelectIndividualCells( false );
		m_pPropDataList->SetEmptyListText( "No prop_data available." );
		m_pPropDataList->SetDragEnabled(false);
		m_pPropDataList->SetAutoResize(Panel::PIN_TOPLEFT, Panel::AUTORESIZE_DOWNANDRIGHT, 6, 72, -6, -6);

		RefreshRenderSettings();
	}

	if (nFlags & PAGE_ATTACHMENTS) //// SLE NEW - attachments tab
	{
		m_pAttachmentsPage = new vgui::PropertyPage(m_pViewsSheet, "AttachmentsPage");

		m_pAttachmentsPage->AddActionSignalTarget(this);

		m_pAttachmentsPage->LoadUserConfig("level_editor/resource/level_editor_model_browser_attachments.res");
		m_pAttachmentsPage->LoadControlSettings("level_editor/resource/level_editor_model_browser_attachments.res", "EXECUTABLE_PATH");
		
		m_pAttachmentsList = new vgui::ListPanel(m_pAttachmentsPage, "Attachments");
		m_pAttachmentsList->AddColumnHeader(0, "parent", "Parent bone", 250, ListPanel::COLUMN_FIXEDSIZE);
		m_pAttachmentsList->AddColumnHeader(1, "attachment", "Attachment name", 250, 0);
		m_pAttachmentsList->AddActionSignalTarget(this);
		m_pAttachmentsList->SetSelectIndividualCells(false);
		m_pAttachmentsList->SetEmptyListText(".MDL file contains no attachments.");
		m_pAttachmentsList->SetDragEnabled(false);
		m_pAttachmentsList->SetAutoResize(Panel::PIN_TOPLEFT, Panel::AUTORESIZE_DOWNANDRIGHT, 6, 6, -6, -6);

		RefreshRenderSettings();
	}

	// Load layout settings; has to happen before pinning occurs in code
	LoadUserConfig("level_editor/resource/level_editor_model_browser.res");
	LoadControlSettings("level_editor/resource/level_editor_model_browser.res", "EXECUTABLE_PATH");
	//LoadControlSettingsAndUserConfig( "level_editor/resource/level_editor_model_browser.res" ); //// SLE CHANGE - move new model browser scheme to new file

	// Pages must be added after control settings are set up
	if ( m_pRenderPage )
	{
		m_pViewsSheet->AddPage( m_pRenderPage, "Render" );
	}
	if ( m_pSequencesPage )
	{
		m_pViewsSheet->AddPage( m_pSequencesPage, "Sequences" );
	}
	if ( m_pActivitiesPage )
	{
		m_pViewsSheet->AddPage( m_pActivitiesPage, "Activities" );
	}
	if ( m_pSkinsPage )
	{
		m_pViewsSheet->AddPage( m_pSkinsPage, "Skins" );
	}
	if ( m_pInfoPage )
	{
		m_pViewsSheet->AddPage( m_pInfoPage, "Info" );
	}
	if (m_pAttachmentsPage) //// SLE NEW - attachments tab
	{
		m_pViewsSheet->AddPage(m_pAttachmentsPage, "Attachments");
	}
}

void CMDLPicker::RefreshRenderSettings()
{
	vgui::CheckButton *pToggle;

	if ( !m_pRenderPage )
		return;

	// ground
	pToggle = (vgui::CheckButton*)m_pRenderPage->FindChildByName("NoGround");
	pToggle->AddActionSignalTarget( this );
	m_pMDLPreview->SetGroundGrid( !pToggle->IsSelected() );
	
	// collision
	pToggle = (vgui::CheckButton*)m_pRenderPage->FindChildByName("Collision");
	pToggle->AddActionSignalTarget( this );
	m_pMDLPreview->SetCollsionModel( pToggle->IsSelected() );

	// wireframe
	pToggle = (vgui::CheckButton*)m_pRenderPage->FindChildByName("Wireframe");
	pToggle->AddActionSignalTarget( this );
	m_pMDLPreview->SetWireFrame( pToggle->IsSelected() );

	//// SLE NEW - attachments tab 
	pToggle = (vgui::CheckButton*)m_pRenderPage->FindChildByName("ShowAttachments");
	pToggle->AddActionSignalTarget(this);
	m_pMDLPreview->SetShowAllAttachments(pToggle->IsSelected());
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMDLPicker::~CMDLPicker()
{
}


//-----------------------------------------------------------------------------
// Performs layout
//-----------------------------------------------------------------------------
void CMDLPicker::PerformLayout()
{
	// NOTE: This call should cause auto-resize to occur
	// which should fix up the width of the panels
	BaseClass::PerformLayout();

	int w, h;
	GetSize( w, h );

	// Layout the mdl splitter
	m_pFileBrowserSplitter->SetBounds( 0, 0, w, h );
}


//-----------------------------------------------------------------------------
// Buttons on various pages
//-----------------------------------------------------------------------------
void CMDLPicker::OnAssetSelected( KeyValues *pParams )
{
	const char *pAsset = pParams->GetString( "asset" );

	char pProbeBuf[MAX_PATH];
	Q_snprintf( pProbeBuf, sizeof(pProbeBuf), "materials/lightprobes/%s", pAsset );

	BeginDMXContext();
	CDmxElement *pLightProbe = NULL; 
	bool bOk = UnserializeDMX( pProbeBuf, "GAME", true, &pLightProbe );
	if ( !pLightProbe || !bOk )
	{
		char pBuf[1024];
		Q_snprintf( pBuf, sizeof(pBuf), "Error loading lightprobe file '%s'!\n", pProbeBuf ); 
		vgui::MessageBox *pMessageBox = new vgui::MessageBox( "Error Loading File!\n", pBuf, GetParent() );
		pMessageBox->DoModal( );

		EndDMXContext( true );
		return;
	}

	m_pMDLPreview->SetLightProbe( pLightProbe );
	EndDMXContext( true );
}


//-----------------------------------------------------------------------------
// Buttons on various pages
//-----------------------------------------------------------------------------
void CMDLPicker::OnCommand( const char *pCommand )
{
	/* // SLE REMOVE - making model browser leaner
	if ( !Q_stricmp( pCommand, "ChooseLightProbe" ) )
	{
		CAssetPickerFrame *pPicker = new CAssetPickerFrame( this, "Select Light Probe (.prb) File",
			"Light Probe", "prb", "materials/lightprobes", "lightprobe" );
		pPicker->DoModal();
		return;
	}
	*/
	BaseClass::OnCommand( pCommand );
}

//-----------------------------------------------------------------------------
void *VTexFilesystemFactory( const char *pName, int *pReturnCode )
{
	return g_pFullFileSystem;
}


void* MdlPickerFSFactory( const char *pName, int *pReturnCode )
{
	if ( IsX360() )
		return NULL;

	if ( Q_stricmp( pName, FILESYSTEM_INTERFACE_VERSION ) == 0 )
		return g_pFullFileSystem;

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: rebuilds the list of activities
//-----------------------------------------------------------------------------
void CMDLPicker::RefreshActivitiesAndSequencesList()
{
	m_pActivitiesList->RemoveAll();
	m_pSequencesList->RemoveAll();
	m_pMDLPreview->SetSequence( 0 );

	if ( m_hSelectedMDL[ 0 ] == MDLHANDLE_INVALID )
	{
		m_pActivitiesList->SetEmptyListText("No .MDL file currently selected");
		m_pSequencesList->SetEmptyListText("No .MDL file currently selected");
		return;
	}

	m_pActivitiesList->SetEmptyListText(".MDL file contains no activities");
	m_pSequencesList->SetEmptyListText(".MDL file contains no sequences");

	studiohdr_t *hdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL[ 0 ] );
	
	CUtlDict<int, unsigned short> activityNames( true, 0, hdr->GetNumSeq() );

	for (int j = 0; j < hdr->GetNumSeq(); j++)
	{
		if ( /*g_viewerSettings.showHidden ||*/ !(hdr->pSeqdesc(j).flags & STUDIO_HIDDEN))
		{
			const char *pActivityName = hdr->pSeqdesc(j).pszActivityName();
			if ( pActivityName && pActivityName[0] )
			{
				// Multiple sequences can have the same activity name; only add unique activity names
				if ( activityNames.Find( pActivityName ) == activityNames.InvalidIndex() )
				{
					KeyValues *pkv = new KeyValues("node", "activity", pActivityName );
					int nItemID = m_pActivitiesList->AddItem( pkv, 0, false, false );

					KeyValues *pDrag = new KeyValues( "drag", "text", pActivityName );
					pDrag->SetString( "texttype", "activityName" );
					pDrag->SetString( "mdl", vgui::MDLCache()->GetModelName( m_hSelectedMDL[ 0 ] ) );
					m_pActivitiesList->SetItemDragData( nItemID, pDrag );

					activityNames.Insert( pActivityName, j );
				}
			}

			const char *pSequenceName = hdr->pSeqdesc(j).pszLabel();
			if ( pSequenceName && pSequenceName[0] )
			{
				KeyValues *pkv = new KeyValues("node", "sequence", pSequenceName);
				int nItemID = m_pSequencesList->AddItem( pkv, 0, false, false );

				KeyValues *pDrag = new KeyValues( "drag", "text", pSequenceName );
				pDrag->SetString( "texttype", "sequenceName" );
				pDrag->SetString( "mdl", vgui::MDLCache()->GetModelName( m_hSelectedMDL[ 0 ] ) );
				m_pSequencesList->SetItemDragData( nItemID, pDrag );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// A MDL was selected
//-----------------------------------------------------------------------------
void CMDLPicker::OnSelectedAssetPicked( const char *pMDLName )
{
	char pRelativePath[MAX_PATH];

	int nSelectSecondary = -1;
	if ( input()->IsKeyDown( KEY_RCONTROL ) || input()->IsKeyDown( KEY_LCONTROL ) )
	{
		nSelectSecondary = 0;
	}
	else if ( input()->IsMouseDown(MOUSE_RIGHT) )
	{
		nSelectSecondary = 1;
	}

	if ( pMDLName )
	{
		Q_snprintf( pRelativePath, sizeof(pRelativePath), "models\\%s", pMDLName );
		SelectMDL( pRelativePath, true, nSelectSecondary );
	}
	else
	{
		SelectMDL( NULL, true, nSelectSecondary );
	}
}


//-----------------------------------------------------------------------------
// Allows external apps to select a MDL
//-----------------------------------------------------------------------------
void CMDLPicker::SelectMDL( const char *pRelativePath, bool bDoLookAt, int nSelectSecondary )
{
	MDLHandle_t hSelectedMDL = pRelativePath ? vgui::MDLCache()->FindMDL( pRelativePath ) : MDLHANDLE_INVALID;
	int			index = ( nSelectSecondary > 0 ? nSelectSecondary : 0 );

	// We didn't change models after all...
	if ( hSelectedMDL == m_hSelectedMDL[ index ] )
	{
		// vgui::MDLCache()->FindMDL adds a reference by default we don't use, release it again
		if ( hSelectedMDL != MDLHANDLE_INVALID )
		{
			vgui::MDLCache()->Release( hSelectedMDL );
		}
		return;
	}

	m_hSelectedMDL[ index ] = hSelectedMDL;

	if ( vgui::MDLCache()->IsErrorModel( m_hSelectedMDL[ index ] ) )
	{
		m_hSelectedMDL[ index ] = MDLHANDLE_INVALID;
	}
	if ( nSelectSecondary != -1 )
	{
		m_pMDLPreview->ClearMergeMDLs();
		for( int i = 1; i < MAX_SELECTED_MODELS; i++ )
		{
			if ( i != index )
			{
				m_hSelectedMDL[ i ] = MDLHANDLE_INVALID;
			}
		}
	}

	if ( index > 0 )
	{
		m_pMDLPreview->SetMergeMDL( m_hSelectedMDL[ index ] );
	}
	else
	{
		m_pMDLPreview->SetMDL( m_hSelectedMDL[ index ] );

		if ( bDoLookAt )
		{
			m_pMDLPreview->LookAtMDL();
		}

		if ( m_nFlags & ( PAGE_SKINS ) )
		{
			UpdateSkinsList();
		}

		if ( m_nFlags & ( PAGE_INFO ) )
		{
			UpdateInfoTab();
		}

		if ( m_nFlags & (PAGE_ACTIVITIES|PAGE_SEQUENCES) )
		{
			RefreshActivitiesAndSequencesList();
		}

		if (m_nFlags & (PAGE_ATTACHMENTS)) //// SLE NEW - attachments tab
		{
			UpdateAttachmentsTab();
		}
	}

	// vgui::MDLCache()->FindMDL adds a reference by default we don't use, release it again
	if ( hSelectedMDL != MDLHANDLE_INVALID )
	{
		vgui::MDLCache()->Release( hSelectedMDL );
	}

	PostActionSignal( new KeyValues( "MDLPreviewChanged", "mdl", pRelativePath ? pRelativePath : "" ) );
}


//-----------------------------------------------------------------------------
// Purpose: updates revision view on a file being selected
//-----------------------------------------------------------------------------
void CMDLPicker::OnCheckButtonChecked(KeyValues *kv)
{
//    RefreshMDLList();
	BaseClass::OnCheckButtonChecked( kv );
	RefreshRenderSettings();
}


void CMDLPicker::GetSelectedMDLName( char *pBuffer, int nMaxLen )
{
	Assert( nMaxLen > 0 );
	if ( GetSelectedAssetCount() > 0 )
	{
		Q_snprintf( pBuffer, nMaxLen, "models\\%s", GetSelectedAsset( ) );
	}
	else
	{
		pBuffer[0] = 0;
	}
}
	
//-----------------------------------------------------------------------------
// Gets the selected activity/sequence
//-----------------------------------------------------------------------------
int CMDLPicker::GetSelectedPage( )
{
	if ( m_pSequencesPage && ( m_pViewsSheet->GetActivePage() == m_pSequencesPage ) )
		return PAGE_SEQUENCES;

	if ( m_pActivitiesPage && ( m_pViewsSheet->GetActivePage() == m_pActivitiesPage ) )
		return PAGE_ACTIVITIES;

	return PAGE_NONE;
}

const char *CMDLPicker::GetSelectedSequenceName()
{
	if ( !m_pSequencesPage  )
		return NULL;

	int nIndex = m_pSequencesList->GetSelectedItem( 0 );
	if ( nIndex >= 0 )
	{
		KeyValues *pkv = m_pSequencesList->GetItem( nIndex );
		return pkv->GetString( "sequence", NULL );
	}

	return NULL;
}

const char *CMDLPicker::GetSelectedActivityName()
{
	if ( !m_pActivitiesPage  )
		return NULL;

	int nIndex = m_pActivitiesList->GetSelectedItem( 0 );
	if ( nIndex >= 0 )
	{
		KeyValues *pkv = m_pActivitiesList->GetItem( nIndex );
		return pkv->GetString( "activity", NULL );
	}
	return NULL;
}

int	CMDLPicker::GetSelectedSkin()
{
	if ( !m_pSkinsPage )
		return 0;

	int nIndex = m_pSkinsList->GetSelectedItem( 0 );
	if ( nIndex >= 0 )
	{
		return nIndex;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Plays the selected activity
//-----------------------------------------------------------------------------
void CMDLPicker::SelectActivity( const char *pActivityName )
{
	studiohdr_t *pstudiohdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL[ 0 ] );
	for ( int i = 0; i < pstudiohdr->GetNumSeq(); i++ )
	{
		mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( i );
		if ( stricmp( seqdesc.pszActivityName(), pActivityName ) == 0 )
		{
			// FIXME: Add weighted sequence selection logic?
			m_pMDLPreview->SetSequence( i );
			break;
		}
	}

	PostActionSignal( new KeyValues( "SequenceSelectionChanged", "activity", pActivityName ) );
}


//-----------------------------------------------------------------------------
// Plays the selected sequence
//-----------------------------------------------------------------------------
void CMDLPicker::SelectSequence( const char *pSequenceName )
{
	studiohdr_t *pstudiohdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL[ 0 ] );
	for (int i = 0; i < pstudiohdr->GetNumSeq(); i++)
	{
		mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( i );
		if ( !Q_stricmp( seqdesc.pszLabel(), pSequenceName ) )
		{
			m_pMDLPreview->SetSequence( i );
			break;
		}
	}

	PostActionSignal( new KeyValues( "SequenceSelectionChanged", "sequence", pSequenceName ) );
}

void CMDLPicker::SelectSkin( int nSkin )
{
	m_pMDLPreview->SetSkin( nSkin );
	PostActionSignal( new KeyValues( "SkinSelectionChanged", "skin", nSkin));
}

	
//-----------------------------------------------------------------------------
// Purpose: Updates preview when an item is selected
//-----------------------------------------------------------------------------
void CMDLPicker::OnItemSelected( KeyValues *kv )
{
	Panel *pPanel = (Panel *)kv->GetPtr("panel", NULL);
	if ( m_pSequencesList && (pPanel == m_pSequencesList ) )
	{
		const char *pSequenceName = GetSelectedSequenceName();
		if ( pSequenceName )
		{
			SelectSequence( pSequenceName );
		}
		return;
	}

	if ( m_pActivitiesList && ( pPanel == m_pActivitiesList ) )
	{
		const char *pActivityName = GetSelectedActivityName();
		if ( pActivityName )
		{
			SelectActivity( pActivityName );
		}
		return;
	}

	if ( m_pSkinsList && ( pPanel == m_pSkinsList ) )
	{
		int nSelectedSkin = GetSelectedSkin();
		SelectSkin( nSelectedSkin );
	
		return;
	}

	if (m_pAttachmentsList && (pPanel == m_pAttachmentsList))
	{
		int nIndex = m_pAttachmentsList->GetSelectedItem(0);
		if (nIndex >= 0)
		{
			m_pMDLPreview->SetSelectedAttachmentId(nIndex);
		}

		return;
	}

	BaseClass::OnItemSelected( kv );
}


//-----------------------------------------------------------------------------
// Purpose: Called when a page is shown
//-----------------------------------------------------------------------------
void CMDLPicker::OnPageChanged( )
{
	//// SLE NEW - attachments tab
	if (m_pMDLPreview) m_pMDLPreview->SetSelectedAttachmentId(-1); // reset every time we switch away or to from attachments tab

	if ( m_pSequencesPage && ( m_pViewsSheet->GetActivePage() == m_pSequencesPage ) )
	{
		m_pSequencesList->RequestFocus();

		const char *pSequenceName = GetSelectedSequenceName();

		if ( pSequenceName )
		{
			SelectSequence( pSequenceName );
		}
		return;
	}
	
	if ( m_pActivitiesPage && ( m_pViewsSheet->GetActivePage() == m_pActivitiesPage ) )
	{
		m_pActivitiesList->RequestFocus();

		const char *pActivityName = GetSelectedActivityName();

		if ( pActivityName )
		{
			SelectActivity( pActivityName );
		}
		return;
	}
}


//-----------------------------------------------------------------------------
//
// Purpose: Modal picker frame
//
//-----------------------------------------------------------------------------
CMDLPickerFrame::CMDLPickerFrame( vgui::Panel *pParent, const char *pTitle, int nFlags ) : 
	BaseClass( pParent )
{
	SetAssetPicker( new CMDLPicker( this, nFlags ) );
	LoadUserConfig("level_editor/resource/level_editor_model_browser_frame.res");
	LoadControlSettings("level_editor/resource/level_editor_model_browser_frame.res", "EXECUTABLE_PATH");
//	LoadControlSettingsAndUserConfig( "level_editor/resource/level_editor_model_browser_frame.res" ); //// SLE CHANGE - move new model browser scheme to new file
	SetTitle( pTitle, false );
}

CMDLPickerFrame::~CMDLPickerFrame()
{
}


//-----------------------------------------------------------------------------
// Allows external apps to select a MDL
//-----------------------------------------------------------------------------
void CMDLPickerFrame::SelectMDL( const char *pRelativePath )
{
	static_cast<CMDLPicker*>( GetAssetPicker() )->SelectMDL( pRelativePath );
}

int CMDLPicker::UpdateSkinsList()
{
	int nNumSkins = 0;

	if ( m_pSkinsList )
	{
		m_pSkinsList->RemoveAll();

		studiohdr_t *hdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL[ 0 ] );
		if ( hdr )
		{
			nNumSkins = hdr->numskinfamilies;
			for ( int i = 0; i < nNumSkins; i++ )
			{
				char skinText[25] = "";
				sprintf( skinText, "skin%i", i );
				KeyValues *pkv = new KeyValues("node", "skin", skinText );
				m_pSkinsList->AddItem( pkv, 0, false, false );
			}
		}
	}
		
	return nNumSkins;
}

void CMDLPicker::UpdateInfoTab()
{
	studiohdr_t *hdr = vgui::MDLCache()->GetStudioHdr( m_hSelectedMDL[ 0 ] );
	if ( !hdr )
		return;
	
	int nMass = hdr->mass;
	Panel *pTempPanel = m_pInfoPage->FindChildByName("MassValue");
	char massBuff[10];
	Q_snprintf( massBuff, 10, "%d", nMass );
	((vgui::Label *)pTempPanel)->SetText( massBuff );
	bool bIsStatic = hdr->flags & STUDIOHDR_FLAGS_STATIC_PROP;
	bool bIsPhysics = false;
	const char* buf = hdr->KeyValueText();
	Label * pTempLabel = (Label *)m_pInfoPage->FindChildByName("StaticText");
	pTempLabel->SetVisible( false );
	if( buf )
	{
		buf = Q_strstr( buf, "prop_data" );
		if ( buf )
		{
			int iPropDataCount = UpdatePropDataList( buf, bIsStatic );
			if( iPropDataCount )
			{
				bIsPhysics = true;
			}
		}
		else
		{
			m_pPropDataList->RemoveAll();
		}
	}
	else
	{
		m_pPropDataList->RemoveAll();
	}
	
	CheckButton * pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName("StaticObject");
	pTempCheck->SetCheckButtonCheckable( true );
	pTempCheck->SetSelected( bIsStatic );
	pTempCheck->SetCheckButtonCheckable( false );
	pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName("PhysicsObject");
	pTempCheck->SetCheckButtonCheckable( true );
	pTempCheck->SetSelected( bIsPhysics );
	pTempCheck->SetCheckButtonCheckable( false );
	pTempCheck = (CheckButton *)m_pInfoPage->FindChildByName("DynamicObject");
	pTempCheck->SetCheckButtonCheckable( true );
	pTempCheck->SetSelected( !bIsPhysics );
	pTempCheck->SetCheckButtonCheckable( false );
}

int CMDLPicker::UpdatePropDataList( const char* pszPropData, bool &bIsStatic )
{
	int iCount = 0;  

	if ( m_pPropDataList )
	{
		m_pPropDataList->RemoveAll();

		const char * endPropData = strchr( pszPropData, '}' );
		char keyText[255] = "";
		char valueText[255] = "";
		const char *beginChunk = strchr( pszPropData, '\"' );
		if ( !beginChunk )
		{
			return 0;
		}
		beginChunk++;
		const char *endChunk = strchr( beginChunk, '\"' );
		while( endChunk )
		{
			Q_memcpy( keyText, beginChunk, endChunk - beginChunk );
			beginChunk = endChunk + 1;
			beginChunk = strchr( beginChunk, '\"' ) + 1;
			endChunk = strchr( beginChunk, '\"' );
			Q_memcpy( valueText, beginChunk, endChunk - beginChunk );		
			if( !Q_strcmp( keyText, "allowstatic" ) && !Q_strcmp( valueText , "1" ) )
			{
				if ( !bIsStatic )
				{					
					Label * pTempLabel = (Label *)m_pInfoPage->FindChildByName("StaticText");
					pTempLabel->SetVisible( true );
				}
				bIsStatic &= true;
			}
			KeyValues *pkv = new KeyValues("node", "key", keyText, "value", valueText );
			m_pPropDataList->AddItem( pkv, 0, false, false );
			Q_memset( keyText, 0, 255 );
			Q_memset( valueText, 0, 255 );
			iCount++;
			beginChunk = endChunk + 1;
			beginChunk = strchr( beginChunk, '\"' );
			if ( !beginChunk || beginChunk > endPropData )
			{
				return iCount;
			}
			beginChunk++;
			endChunk = strchr( beginChunk, '\"' );		
		}
	}
	return iCount;
}

void CMDLPicker::UpdateAttachmentsTab() //// SLE NEW - attachments tab
{
	studiohdr_t *hdr = vgui::MDLCache()->GetStudioHdr(m_hSelectedMDL[0]);
	if (!hdr)
		return;
	
	if (m_pAttachmentsList)
	{
		m_pAttachmentsList->SetEmptyListText(".MDL file contains no attachments");

		m_pAttachmentsList->RemoveAll();

		int attachments = hdr->GetNumAttachments();
		if (attachments <= 0) return;

		CUtlDict<int, unsigned short> attachmentNames(true, 0, attachments);
		CUtlDict<int, unsigned short> boneNames(true, 0, attachments);

		for (int i = 0; i < attachments; i++)
		{
			const char *attachmentName = hdr->pAttachment(i).pszName();
			const char *boneName = hdr->pBone(hdr->GetAttachmentBone(i))->pszName();

			if (attachmentName && attachmentName[0] && boneName && boneName[0])
			{
				KeyValues *parent = new KeyValues("node", "parent", boneName, "attachment", attachmentName);
				m_pAttachmentsList->AddItem(parent, 0, false, false);

				attachmentNames.Insert(attachmentName, i);
			}
		}
	}
}