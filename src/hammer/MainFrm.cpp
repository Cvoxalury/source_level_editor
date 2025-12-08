//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "stdafx.h"
#include <afxadv.h>
#include <oaidl.h>
#include "hammer.h"
#include "Box3D.h"				// For units
#include "FaceEditSheet.h"
#include "MainFrm.h"
#include "MessageWnd.h"
#include "ControlBarIDs.h"
#include "CustomMessages.h"
#include "DynamicDialogWnd.h"
#include "filesystem_tools.h"
#include "GlobalFunctions.h"
#include "Prefabs.h"
#include "PrefabsDlg.h"
#include "MapDoc.h"
#include "Manifest.h"
#include "StatusBarIDs.h"
#include "Splash.h"
#include "Options.h"
#include "OptionProperties.h"
#include "ObjectProperties.h"
#include "OP_Groups.h"
#include "MapView2D.h"
#include "MapViewLogical.h"
#include "MapView3D.h"
#include "ChildFrm.h"
#include "NewDocType.h"
#include "SearchReplaceDlg.h"
#include "TextureBrowser.h"
#include "TextureSystem.h"
#include "ToolManager.h"
#include "Material.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/MaterialSystem_Config.h"
#include "soundbrowser.h"
#include "lprvwindow.h"
#ifdef SLE
#include "EntityReportDlg.h" //// used for Model Browser's  new 'find asset' feature
#include "ModelBrowser.h"
#ifdef HAMMER2013_PORT_CORDONS
#include "cmdhandlers.h"
#endif
#ifdef SLE_WINTAB_ENABLE //// SLE NEW - Tablet support w/ Wintab
#include <wintab.h>
#include "tablet.h"
#endif
#endif //// SLE
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_EDIT_PROPERTIES, OnEditProperties)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PROPERTIES, OnUpdateEditFunction)
	ON_COMMAND(ID_VIEW_MESSAGES, OnViewMessages)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MESSAGES, OnUpdateViewMessages)
	ON_WM_ACTIVATEAPP()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_TIMER()
#ifdef SLE
	ON_WM_ERASEBKGND() //// SLE NEW - dark theme test
	ON_COMMAND(IDC_COLOR_3DBACKGROUND, On3DVoidColor) //// SLE NEW - selectable void colour setting
	ON_COMMAND(IDC_COLOR_3DSELECTION, OnSelectionColor) //// SLE NEW - configure the tint on selected objects
	ON_COMMAND(ID_REPORT_GITHUB, OnOpenGitHubIssues) //// SLE NEW - link directly to GitHub repo
	ON_COMMAND(ID_INSTANCES_ENABLE_LOADING, OnToggleInstancesLoading) //// SLE NEW - control to disable loading instances
	ON_UPDATE_COMMAND_UI(ID_INSTANCES_ENABLE_LOADING, OnUpdateToggleInstancesLoading) //// ditto
#endif
	ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOptions)
	ON_COMMAND(ID_TOOLS_PREFABFACTORY, OnToolsPrefabfactory)
	ON_COMMAND_EX(ID_HELP_TOPICS, OnHelpOpenURL)
	ON_COMMAND_EX(ID_HELP_EDITINGSITE, OnHelpOpenURL)
	ON_COMMAND_EX(ID_HELP_WORLDCRAFT_SUPPORT_MAIL, OnHelpOpenURL)
	ON_COMMAND(ID_EDIT_UNDOREDOACTIVE, OnEditUndoredoactive)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDOREDOACTIVE, OnUpdateEditUndoredoactive)
	ON_COMMAND_EX(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_SAVEWINDOWSTATE, OnSavewindowstate)
	ON_COMMAND(ID_LOADWINDOWSTATE, OnLoadwindowstate)
	ON_COMMAND_EX(ID_MAP_UNITS_NONE, OnUnits)
	ON_UPDATE_COMMAND_UI(ID_MAP_UNITS_NONE, OnUpdateUnits)
	ON_COMMAND_EX(ID_MAP_UNITS_INCHES, OnUnits)
	ON_UPDATE_COMMAND_UI(ID_MAP_UNITS_INCHES, OnUpdateUnits)
	ON_COMMAND_EX(ID_MAP_UNITS_FEET_INCHES, OnUnits)
#ifdef SLE
	ON_UPDATE_COMMAND_UI(ID_MAP_UNITS_FEET_INCHES, OnUpdateUnits) //// SLE NEW - show units in cm
	ON_COMMAND_EX(ID_MAP_UNITS_CENTIMETERS, OnUnits)
	ON_UPDATE_COMMAND_UI(ID_MAP_UNITS_METERS_CENTIMETERS, OnUpdateUnits) //// SLE NEW - show units in m + cm
	ON_COMMAND_EX(ID_MAP_UNITS_METERS_CENTIMETERS, OnUnits)
	ON_UPDATE_COMMAND_UI(ID_MAP_UNITS_METERS_CENTIMETERS, OnUpdateUnits)
#endif
	ON_COMMAND(ID_VIEW_OPAQUE_MATERIALS, OnOpaqueMaterials)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OPAQUE_MATERIALS, OnUpdateOpaqueMaterials)
#ifdef SLE //// SLE NEW - preview normal maps, diffuse, specular...
	ON_COMMAND(ID_VIEW_SHOW_DIFFUSE, OnShowDiffuse)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_DIFFUSE, OnUpdateShowDiffuse)
	ON_COMMAND(ID_VIEW_SHOW_NORMALMAPS, OnShowNormalMaps)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_NORMALMAPS, OnUpdateShowNormalMaps)
	ON_COMMAND(ID_VIEW_SHOW_SPECULAR, OnShowSpecular)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_SPECULAR, OnUpdateShowSpecular)
	ON_COMMAND(ID_VIEW_EDITOR_MODE, OnUseEditorMode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_EDITOR_MODE, OnUpdateUseEditorMode)
#endif
	ON_UPDATE_COMMAND_UI(ID_VIEW_2DXZ, OnUpdateView2d)
	ON_UPDATE_COMMAND_UI(ID_VIEW_2DYZ, OnUpdateView2d) 
	ON_UPDATE_COMMAND_UI(ID_VIEW_2DXY, OnUpdateView2d)
	ON_UPDATE_COMMAND_UI(ID_VIEW_3DWIREFRAME, OnUpdateView3d)
	ON_UPDATE_COMMAND_UI(ID_VIEW_3DPOLYGON, OnUpdateView3d)
	ON_UPDATE_COMMAND_UI(ID_VIEW_3DTEXTURED, OnUpdateView3d)
	//ON_UPDATE_COMMAND_UI(ID_VIEW_3DENGINE, OnUpdateView3d)
#ifdef SLE //// New bar ids
	ON_UPDATE_COMMAND_UI(ID_VIEW_UNDO_REDO_BAR, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_UNDO_REDO_BAR, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRIDSELECTION, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_GRIDSELECTION, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GROUPHIDECORDONTEX, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_GROUPHIDECORDONTEX, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VISIBILITY, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_VISIBILITY, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CHECKCOMPILE, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_CHECKCOMPILE, CFrameWnd::OnBarCheck)
#endif
#ifndef SLE //// SL REMOVE - deprecated, Quake 2
	ON_COMMAND_EX(ID_VIEW3D_BRIGHTER, OnView3dChangeBrightness)
	ON_COMMAND_EX(ID_VIEW3D_DARKER, OnView3dChangeBrightness)
#endif
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTBAR, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_OBJECTBAR, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECTION_MODE_BAR, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_SELECTION_MODE_BAR, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FILTERCONTROL, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_FILTERCONTROL, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAPVIEWBAR, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_MAPVIEWBAR, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAPTOOLSBAR, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_MAPTOOLSBAR, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTUREBAR, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_TEXTUREBAR, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MANIFEST_BAR, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_MANIFEST_BAR, CFrameWnd::OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ANIMATIONBAR, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_ANIMATIONBAR, CFrameWnd::OnBarCheck)
#ifdef SLE //// SLE TODO - separate render settings toolbar
//	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERSETTINGS, CFrameWnd::OnUpdateControlBarMenu)
//	ON_COMMAND_EX(ID_VIEW_RENDERSETTINGS, CFrameWnd::OnBarCheck)
#endif
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAPOPSBAR, CFrameWnd::OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_MAPOPSBAR, CFrameWnd::OnBarCheck)
	ON_COMMAND_EX(ID_TOOLS_POINTER, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_POINTER, OnUpdateToolUI)
	ON_COMMAND_EX(ID_TOOLS_CAMERA, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_CAMERA, OnUpdateToolUI)
	ON_COMMAND_EX(ID_TOOLS_MAGNIFY, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_MAGNIFY, OnUpdateToolUI)
	ON_COMMAND_EX(ID_TOOLS_BLOCK, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_BLOCK, OnUpdateToolUI)
	ON_COMMAND_EX(ID_TOOLS_ENTITY, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_ENTITY, OnUpdateToolUI)
	ON_COMMAND_EX(ID_TOOLS_APPLYDECALS, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_APPLYDECALS, OnUpdateToolUI)
	ON_COMMAND_EX(ID_TOOLS_MORPH, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_MORPH, OnUpdateToolUI)
	ON_COMMAND_EX(ID_TOOLS_CLIPPER, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_CLIPPER, OnUpdateToolUI)
#ifdef SLE
	ON_COMMAND_EX(ID_TOOLS_CLIPPER3POINT, OnChangeTool) //// SLE NEW - 3-point clipping tool, a subclass of normal Clipper
	ON_UPDATE_COMMAND_UI(ID_TOOLS_CLIPPER3POINT, OnUpdateToolUI)
#endif
	ON_COMMAND_EX(ID_TOOLS_EDITCORDON, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_EDITCORDON, OnUpdateToolUI)
	ON_COMMAND_EX(ID_TOOLS_PATH, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_PATH, OnUpdateToolUI)
	ON_COMMAND_EX(ID_TOOLS_OVERLAY, OnChangeTool)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_OVERLAY, OnUpdateToolUI)
	ON_COMMAND_EX(ID_MODE_APPLICATOR, OnApplicator)
	ON_COMMAND_EX(ID_TOOLS_SOUND_BROWSER, OnSoundBrowser)
	ON_COMMAND_EX(ID_FILE_RELOAD_SOUNDS, OnReloadSounds)
#ifdef SLE
	ON_COMMAND(ID_TOOLS_MODEL_BROWSER, OnModelBrowser) //// SLE NEW - backports for matsys_control
	ON_COMMAND_EX(ID_FILE_RELOAD_MATERIALS, OnReloadMaterials) //// SLE NEW: option to manually uncache mats and textures
	ON_COMMAND_EX(ID_FILE_RELOAD_FGD, OnReloadFGD) //// SLE NEW: quick fgd reload button
#endif
    ON_UPDATE_COMMAND_UI(ID_MODE_APPLICATOR, OnUpdateApplicatorUI)
#ifdef SLE //// SLE CHANGE - bring back offline .chm help
	ON_COMMAND(ID_HELP_OFFLINE, OnHelpFinder)
#endif
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HDR, OnHDR)
	ON_WM_HELPINFO()
	ON_WM_SYSCOMMAND()
	ON_WM_ENTERMENULOOP()
#ifdef SLE_WINTAB_ENABLE //// SLE NEW - Tablet support w/ Wintab
	ON_MESSAGE(WT_PACKET, OnWTPacket)
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
	
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_SELECTION,
	ID_INDICATOR_COORDS,
	ID_INDICATOR_SIZE,
	ID_INDICATOR_GRIDZOOM,
	ID_INDICATOR_SNAP
};

const int NUMSTATUSPANES = 7;

const char * WINSTATETAG = "WCWINSTATE";
const int	 WINSTATEEND = -1;
const int	 WINSTATE2DVIEW = 0;
const int	 WINSTATE3DVIEW = 1;
const int	 WINSTATELOGICALVIEW = 2;
const float	 fVersion = 0.1f;

struct
{
	int nIndex;
	UINT nID;
	UINT nStyle;
	int cxWidth;
} paneinfo[NUMSTATUSPANES] = 
{
	{ SBI_PROMPT,		ID_SEPARATOR,				SBPS_STRETCH | SBPS_NOBORDERS, 0 },
	{ SBI_SELECTION,	ID_INDICATOR_SELECTION,		SBPS_NORMAL, 350 },
	{ SBI_COORDS,		ID_INDICATOR_COORDS,		SBPS_NORMAL, 100 },
	{ SBI_SIZE,			ID_INDICATOR_SIZE,			SBPS_NORMAL, 180 },
	{ SBI_GRIDZOOM,		ID_INDICATOR_GRIDZOOM,		SBPS_NORMAL, 80 },
	{ SBI_SNAP,			ID_INDICATOR_SNAP,			SBPS_NORMAL, 135 },
	{ SBI_LIGHTPROGRESS,ID_INDICATOR_LIGHTPROGRESS,	SBPS_NORMAL, 50 }
};

static GameData gd;
static CMainFrame *pMainWnd;

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CMainFrame::CMainFrame(void)
{
	pTextureBrowser = NULL;
	pObjectProperties = NULL;
	m_bUndoActive = TRUE;
	m_bShellSessionActive = false;
	m_pFaceEditSheet = NULL;
	m_bMinimized = false;
	m_pSearchReplaceDlg = NULL;
	m_pLightingPreviewOutputWindow = NULL;
	m_bLightingPreviewOutputWindowShowing = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CMainFrame::~CMainFrame(void)
{
	delete pObjectProperties;
	delete pTextureBrowser;
	delete m_pFaceEditSheet;
	delete m_pSearchReplaceDlg;
	delete m_pLightingPreviewOutputWindow;

	CPrefabLibrary::FreeAllLibraries();
}

//-----------------------------------------------------------------------------
// Purpose: Called through the shell to begin a session of editing the map
//			via the shell. The user interface is disabled to prevent a mismatched
//			versions between Hammer and the shell client.
//-----------------------------------------------------------------------------
void CMainFrame::BeginShellSession(void)
{
	m_bShellSessionActive = true;
}

//-----------------------------------------------------------------------------
// Purpose: Called through the shell to end a session of editing the map
//			via the engine. The user interface is enabled.
//-----------------------------------------------------------------------------
void CMainFrame::EndShellSession(void)
{
	m_bShellSessionActive = false;
}

//-----------------------------------------------------------------------------
// Purpose: If we get here there is no active 3D view. Uncheck the button.
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateView3d(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(FALSE);
}

//-----------------------------------------------------------------------------
// Purpose: If we get here there is no active 3D view. Uncheck the button.
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateOpaqueMaterials(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(MaterialSystemConfig().bNoTransparency);
}

void CMainFrame::OnOpaqueMaterials()
{
	MaterialSystemConfig().bNoTransparency = !MaterialSystemConfig().bNoTransparency;
	MaterialSystemInterface()->OverrideConfig( MaterialSystemConfig(), false );
}
#ifdef SLE //// SLE NEW - preview normal maps, diffuse, specular...
void CMainFrame::OnUpdateShowDiffuse(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(MaterialSystemConfig().bShowDiffuse);
}

void CMainFrame::OnShowDiffuse()
{
	MaterialSystemConfig().bShowDiffuse = !MaterialSystemConfig().bShowDiffuse;
	MaterialSystemInterface()->OverrideConfig(MaterialSystemConfig(), false);
}

void CMainFrame::OnUpdateShowNormalMaps(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(MaterialSystemConfig().bShowNormalMap);
}

void CMainFrame::OnShowNormalMaps()
{
	MaterialSystemConfig().bShowNormalMap = !MaterialSystemConfig().bShowNormalMap;
	MaterialSystemInterface()->OverrideConfig(MaterialSystemConfig(), false);
}

void CMainFrame::OnUpdateShowSpecular(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(MaterialSystemConfig().bShowSpecular);
}

void CMainFrame::OnShowSpecular()
{
	MaterialSystemConfig().bShowSpecular = !MaterialSystemConfig().bShowSpecular;
	MaterialSystemInterface()->OverrideConfig(MaterialSystemConfig(), false);
}

void CMainFrame::OnUpdateUseEditorMode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(MaterialSystemConfig().bEditMode);
}

void CMainFrame::OnUseEditorMode()
{
	MaterialSystemConfig().bEditMode = !MaterialSystemConfig().bEditMode;
	MaterialSystemInterface()->OverrideConfig(MaterialSystemConfig(), false);
}

//// SLE NEW - control to disable loading instances
void CMainFrame::OnToggleInstancesLoading()
{
	Options.general.bEnableInstancesLoading = !Options.general.bEnableInstancesLoading;
}

void CMainFrame::OnUpdateToggleInstancesLoading(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(Options.general.bEnableInstancesLoading);
}
#endif
//-----------------------------------------------------------------------------
// Purpose: If we get here there is no active 2D view. Uncheck the button.
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateView2d(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(FALSE);
}

void CMainFrame::OnEnterMenuLoop( BOOL bIsTrackPopupMenu )
{
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();

	// if we are translation objects with a tool right now, dont switch to Menu mode
	if ( pDoc )
	{
		CBaseTool *pTool = pDoc->GetTools()->GetActiveTool();

		if ( pTool && pTool->IsTranslating() )
		{
			SendMessage( WM_CANCELMODE );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : lpCreateStruct - 
// Output : 
//-----------------------------------------------------------------------------
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	const DWORD dwDefStyles = WS_CHILD | WS_VISIBLE | CBRS_TOP;
	lpCreateStruct->lpszClass = "VALVEWORLDCRAFT"; //// SLE TODO - see if this can be renamed safely

	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    if(!wndMDIClient.SubclassWindow(m_hWndMDIClient)) 
	{ 
		TRACE ("Failed to subclass MDI client window\n");
		return (-1);                                        
    }                 
#ifdef SLE //// SLE CHANGE - toolbars were renamed, redone, made docked by default, written more consistently
	//
	// Map editing toolbar (left, tool buttons)
	//
	m_wndMapEditToolBar.Create(this, dwDefStyles, IDCB_MAPTOOLSBAR);
	m_wndMapEditToolBar.ModifyStyle(0, TBSTYLE_FLAT);
	m_wndMapEditToolBar.LoadToolBar(IDR_MAPEDITTOOLS);
	m_bmMapEditTools256.LoadBitmap(IDB_MAPEDITTOOLS_256);
	m_wndMapEditToolBar.SetBitmap((HBITMAP)m_bmMapEditTools256);

	//
	// Undo redo toolbar
	//
	if (!m_wndUndoRedoToolBar.Create(this, dwDefStyles, IDCB_UNDO_REDO_BAR) || !m_wndUndoRedoToolBar.LoadToolBar(IDR_TOOLBAR_UNDOREDO))
	{
		Error("Failed to create the undo/redo toolbar");
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndUndoRedoToolBar.ModifyStyle(0, TBSTYLE_FLAT);
	m_bmUndoRedo.LoadBitmap(IDR_TOOLBAR_UNDOREDO);
	m_wndUndoRedoToolBar.SetBitmap((HBITMAP)m_bmUndoRedo);

	//
	// Grid settings and selection options toolbar
	//
	if (!m_wndGridSelectionToolBar.Create(this, dwDefStyles, IDCB_GRIDSELECTION_BAR) || !m_wndGridSelectionToolBar.LoadToolBar(IDR_TOOLBAR_GRIDSELECTION))
	{
		Error("Failed to create the grid settings/selection toolbar");
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndGridSelectionToolBar.ModifyStyle(0, TBSTYLE_FLAT);
	m_bmGridSelection.LoadBitmap(IDR_TOOLBAR_GRIDSELECTION);
	m_wndGridSelectionToolBar.SetBitmap((HBITMAP)m_bmGridSelection);

	//
	// Main area toolbar
	//
	if (!m_wndGroupHideCordonTexToolBar.Create(this, dwDefStyles, IDCB_GROUPHIDECORDONTEX_BAR) || !m_wndGroupHideCordonTexToolBar.LoadToolBar(IDR_TOOLBAR_GROUPHIDECORDONTEX))
	{
		Error("Failed to create the main top toolbar");
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndGroupHideCordonTexToolBar.ModifyStyle(0, TBSTYLE_FLAT);
	m_bmGroupHideCordonTex.LoadBitmap(IDR_TOOLBAR_GROUPHIDECORDONTEX);
	m_wndGroupHideCordonTexToolBar.SetBitmap((HBITMAP)m_bmGroupHideCordonTex);

	//
	// Visibility settings toolbar
	//
	if (!m_wndVisibilityToolBar.Create(this, dwDefStyles, IDCB_VISIBILITY_BAR) || !m_wndVisibilityToolBar.LoadToolBar(IDR_TOOLBAR_VISIBILITY))
	{
		Error("Failed to create the visibility toolbar");
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndVisibilityToolBar.ModifyStyle(0, TBSTYLE_FLAT);
	m_bmVisibility.LoadBitmap(IDR_TOOLBAR_VISIBILITY);
	m_wndVisibilityToolBar.SetBitmap((HBITMAP)m_bmVisibility);

	//
	// Check/compile map toolbar
	//
	if (!m_wndCheckAndCompileToolBar.Create(this, dwDefStyles, IDCB_CHECKCOMPILE_BAR) || !m_wndCheckAndCompileToolBar.LoadToolBar(IDR_TOOLBAR_CHECKCOMPILE))
	{
		Error("Failed to create the check/compile toolbar");
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndCheckAndCompileToolBar.ModifyStyle(0, TBSTYLE_FLAT);
	m_bmCheckCompile.LoadBitmap(IDR_TOOLBAR_CHECKCOMPILE);
	m_wndCheckAndCompileToolBar.SetBitmap((HBITMAP)m_bmCheckCompile);
	
	EnableDocking(CBRS_ALIGN_ANY);

	//
	// left-side tool bar
	//
	
	m_wndMapEditToolBar.SetBarStyle(m_wndMapEditToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_wndMapEditToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndMapEditToolBar, AFX_IDW_DOCKBAR_LEFT);

	// top tool bars
	m_wndUndoRedoToolBar.SetBarStyle(m_wndUndoRedoToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndUndoRedoToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndUndoRedoToolBar, AFX_IDW_DOCKBAR_TOP|AFX_IDW_DOCKBAR_LEFT);

	m_wndGridSelectionToolBar.SetBarStyle(m_wndGridSelectionToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndGridSelectionToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBarRightOf(&m_wndGridSelectionToolBar, &m_wndUndoRedoToolBar);
//	DockControlBar(&m_wndGridSelectionToolBar, AFX_IDW_DOCKBAR_TOP);

	m_wndGroupHideCordonTexToolBar.SetBarStyle(m_wndGroupHideCordonTexToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndGroupHideCordonTexToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBarRightOf(&m_wndGroupHideCordonTexToolBar, &m_wndGridSelectionToolBar);
//	DockControlBar(&m_wndGroupHideCordonTexToolBar, AFX_IDW_DOCKBAR_TOP);

	m_wndVisibilityToolBar.SetBarStyle(m_wndVisibilityToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndVisibilityToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBarRightOf(&m_wndVisibilityToolBar, &m_wndGroupHideCordonTexToolBar);
//	DockControlBar(&m_wndVisibilityToolBar, AFX_IDW_DOCKBAR_TOP);

	m_wndCheckAndCompileToolBar.SetBarStyle(m_wndCheckAndCompileToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndCheckAndCompileToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBarRightOf(&m_wndCheckAndCompileToolBar, &m_wndVisibilityToolBar);
//	DockControlBar(&m_wndCheckAndCompileToolBar, AFX_IDW_DOCKBAR_TOP);
		
	// rightside control bars
	//
	// Create the selection mode dialog bar.
	//
	m_SelectModeDlg.Create(this);
	m_SelectModeDlg.SetBarStyle(m_TextureBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
	m_SelectModeDlg.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_SelectModeDlg, AFX_IDW_DOCKBAR_RIGHT);

	m_TextureBar.Create(this);
	m_TextureBar.SetBarStyle(m_TextureBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
	m_TextureBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBarUnderOther(&m_TextureBar, &m_SelectModeDlg);

	m_FilterControl.Create(this);
	m_FilterControl.SetBarStyle(m_FilterControl.GetBarStyle() | 
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
	m_FilterControl.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBarUnderOther(&m_FilterControl, &m_TextureBar);

	m_ObjectBar.Create(this);
	m_ObjectBar.SetBarStyle(m_ObjectBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
	m_ObjectBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBarUnderOther(&m_ObjectBar, &m_FilterControl);

	m_ManifestFilterControl.Create(this);
	m_ManifestFilterControl.SetBarStyle(m_ManifestFilterControl.GetBarStyle() | 
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED );
	m_ManifestFilterControl.EnableDocking(CBRS_ALIGN_ANY);
//	DockControlBar(&m_ManifestFilterControl, AFX_IDW_DOCKBAR_RIGHT);
	m_ManifestFilterControl.ShowWindow(SW_HIDE); //// SLE NEW - make the manifest bar less obtrusive; hide by default.
#else //// old toolbars code
	//
	// Map view toolbar.
	//
	if (!m_wndMapToolBar.Create(this, dwDefStyles, IDCB_MAPVIEWBAR) || !m_wndMapToolBar.LoadToolBar(IDR_MAPDOC_VALVE))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndMapToolBar.ModifyStyle(0, TBSTYLE_FLAT); 

	//
	// Undo redo toolbar.
	//
	if (!m_wndUndoRedoToolBar.Create(this, dwDefStyles, IDCB_UNDO_REDO_BAR) || !m_wndUndoRedoToolBar.LoadToolBar(IDR_UNDOREDO))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndUndoRedoToolBar.ModifyStyle(0, TBSTYLE_FLAT); 

	//
	// Map editing toolbar.
	//
	m_wndMapEditToolBar.Create(this, dwDefStyles, IDCB_MAPTOOLSBAR);
	m_wndMapEditToolBar.ModifyStyle(0, TBSTYLE_FLAT); 
	m_wndMapEditToolBar.LoadToolBar(IDR_MAPEDITTOOLS_VALVE);
    m_bmMapEditTools256.LoadBitmap(IDB_MAPEDITTOOLS_256);
    m_wndMapEditToolBar.SetBitmap((HBITMAP)m_bmMapEditTools256);

	//
	// Map operations toolbar.
	//
	if (!m_wndMapOps.Create(this, dwDefStyles, IDCB_MAPOPERATIONS) || !m_wndMapOps.LoadToolBar(IDR_MAPOPERATIONS_VALVE))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndMapOps.ModifyStyle(0, TBSTYLE_FLAT); 

	//
	// Status bar.
	//
	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(NULL, NUMSTATUSPANES))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	for(int i = 0; i < NUMSTATUSPANES; i++)
	{
		m_wndStatusBar.SetPaneInfo(paneinfo[i].nIndex, paneinfo[i].nID,	paneinfo[i].nStyle, paneinfo[i].cxWidth);
	}

	EnableDocking(CBRS_ALIGN_ANY);

	m_wndMapToolBar.SetBarStyle(m_wndMapToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndUndoRedoToolBar.SetBarStyle(m_wndUndoRedoToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndMapEditToolBar.SetBarStyle(m_wndMapEditToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndMapOps.SetBarStyle(m_wndMapOps.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_wndMapToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndUndoRedoToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndMapEditToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndMapOps.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndMapEditToolBar, AFX_IDW_DOCKBAR_LEFT);

	// top bars
	DockControlBar(&m_wndMapToolBar, AFX_IDW_DOCKBAR_TOP);
	DockControlBarLeftOf(&m_wndUndoRedoToolBar, &m_wndMapToolBar );
	DockControlBarLeftOf(&m_wndMapOps, &m_wndUndoRedoToolBar);

	// rightside control bars
	m_ObjectBar.Create(this);
	m_ObjectBar.SetBarStyle(m_ObjectBar.GetBarStyle() | 
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
	m_ObjectBar.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	DockControlBar(&m_ObjectBar, AFX_IDW_DOCKBAR_RIGHT);

	m_FilterControl.Create(this);
	m_FilterControl.SetBarStyle(m_FilterControl.GetBarStyle() | 
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
	m_FilterControl.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	DockControlBarLeftOf(&m_FilterControl, &m_ObjectBar);

	m_TextureBar.Create(this);
	m_TextureBar.SetBarStyle(m_TextureBar.GetBarStyle() | 
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
	m_TextureBar.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	DockControlBarLeftOf(&m_TextureBar, &m_FilterControl);

	m_ManifestFilterControl.Create(this);
	m_ManifestFilterControl.SetBarStyle(m_ManifestFilterControl.GetBarStyle() | 
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
	m_ManifestFilterControl.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	DockControlBar(&m_ManifestFilterControl, AFX_IDW_DOCKBAR_RIGHT);
#endif //// SLE
	m_pFaceEditSheet = new CFaceEditSheet( "Face Edit Sheet", this );
	m_pFaceEditSheet->Setup();
	m_pFaceEditSheet->Create( this );
	m_pFaceEditSheet->SetVisibility( false );

	m_pLightingPreviewOutputWindow = NULL;

	//
	// Status bar.
	//
	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(NULL, NUMSTATUSPANES))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	for (int i = 0; i < NUMSTATUSPANES; i++)
	{
		m_wndStatusBar.SetPaneInfo(paneinfo[i].nIndex, paneinfo[i].nID, paneinfo[i].nStyle, paneinfo[i].cxWidth);
	}

#ifdef SLE //// SLE TODO - decide if it should (or could) be fixed and brought back
	//
	// Create the animation dialog bar.
	//
//	m_AnimationDlg.Create(this);
//	m_AnimationDlg.SetBarStyle(m_TextureBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
//	m_AnimationDlg.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
//	DockControlBarRightOf(&m_AnimationDlg, &m_TextureBar);

	//
	// Create the render settings bar
	//
//	m_RenderSettings.Create(this);
//	m_RenderSettings.SetBarStyle(m_TextureBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED);
//	m_RenderSettings.EnableDocking(CBRS_ALIGN_TOP );
//	DockControlBarRightOf(&m_RenderSettings, &m_TextureBar);
#endif
	//
	// Create object properties sheet - not visible yet.
	//
	pObjectProperties = new CObjectProperties;
	pObjectProperties->SetupPages();
	pObjectProperties->Create(this, WS_SYSMENU | WS_POPUP | WS_CAPTION | DS_MODALFRAME | WS_THICKFRAME);

	pMainWnd = this;

	//
	// Create the smoothing group visualization dialog.
	//
	m_SmoothingGroupDlg.Create( IDD_SMOOTHING_GROUP_VISUAL, this );

	//
	// Create message window.
	//
	CRect clientrect;
	wndMDIClient.GetClientRect(clientrect);
	g_pwndMessage->CreateMessageWindow( this, CRect( 0, clientrect.Height() - 90, clientrect.Width(), clientrect.Height() ) );

	CPrefabLibrary::LoadAllLibraries();

	ToolManager()->SetTool(TOOL_POINTER);
	
	pTextureBrowser = new CTextureBrowser(this);

	// HACK: Spackle up the maximized window position to (0, 0) to fix an intermittent bug. =(
	WINDOWPLACEMENT wp;
	ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	SetWindowPlacement(&wp);

	//
	// !!!NOTE: Always do this last to ensure that the layout does not get recalculated before the
	//			window is maximized. This prevents control bars from being incorrectly wrapped to
	//			the next column.
	//
	if (VerifyBarState())
	{
		LoadBarState("Barstate");
	}

	return 0;
}

void CMainFrame::DockControlBarRightOf(CControlBar* Bar, CControlBar* RightOf) //// renamed in SLE for clarity
{
	CRect rect;
	DWORD dw;
	UINT n;

	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout();
	RightOf->GetWindowRect(&rect);
	rect.OffsetRect(1,0);
	dw= RightOf->GetBarStyle();
	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;

	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line.  By calculating a rectangle, we in effect
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar,n,&rect);
}
#ifdef SLE 
void CMainFrame::DockControlBarUnderOther(CControlBar* Bar, CControlBar* Other)
{
	CRect rect;
	DWORD dw;
	UINT n;

	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout();
	Other->GetWindowRect(&rect);
	rect.OffsetRect(0, 1);
	dw = Other->GetBarStyle();
	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n == 0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n == 0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n == 0) ? AFX_IDW_DOCKBAR_RIGHT : n;

	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a separate line.  By calculating a rectangle, we in effect
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar, n, &rect);
}
#endif
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_MAXIMIZE;
	cs.lpszClass = "VALVEWORLDCRAFT"; //// SLE TODO - see if this can be renamed safely

	return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


//-----------------------------------------------------------------------------
// Purpose: Maps menu IDs to tool IDs.
// Input  : uMsg - Menu ID from a WM_COMMAND message.
//-----------------------------------------------------------------------------
static ToolID_t _ToolMsgToEnum(UINT uMsg)
{
	struct ToolIDMap_t
	{
		UINT uMsg;
		ToolID_t eToolID;
	};

	ToolIDMap_t nIDMap[] =
	{
		{ ID_TOOLS_POINTER,		TOOL_POINTER },
		{ ID_TOOLS_BLOCK,		TOOL_BLOCK },
		{ ID_TOOLS_ENTITY,		TOOL_ENTITY },
		{ ID_TOOLS_CAMERA,		TOOL_CAMERA },
		{ ID_TOOLS_MAGNIFY,		TOOL_MAGNIFY },
		{ ID_TOOLS_MORPH,		TOOL_MORPH },
		{ ID_TOOLS_CLIPPER,		TOOL_CLIPPER },
		{ ID_TOOLS_EDITCORDON,	TOOL_EDITCORDON },
		{ ID_TOOLS_OVERLAY,		TOOL_OVERLAY },
		{ ID_TOOLS_APPLYDECALS,	TOOL_DECAL },
		{ ID_MODE_APPLICATOR,	TOOL_FACEEDIT_MATERIAL },
#ifdef SLE
		{ ID_TOOLS_SPRINKLE,	TOOL_ENTITY_SPRINKLE },
		{ ID_TOOLS_CLIPPER3POINT, TOOL_CLIPPER_3POINT },
#endif
	};

	for (int i = 0; i < sizeof(nIDMap) / sizeof(nIDMap[0]); i++)
	{
		if (uMsg == nIDMap[i].uMsg)
		{
			return nIDMap[i].eToolID;
		}
	}

	return TOOL_POINTER;
}
#ifdef HAMMER2013_PORT_CORDONS

class CToolHandler_Disabled : public IToolHandlerInfo
{
public:
	virtual BOOL UpdateCmdUI(CCmdUI *pCmdUI);
	virtual BOOL Execute(UINT uMsg);
}
g_ToolHandlerDisabled;

BOOL CToolHandler_Disabled::UpdateCmdUI(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(FALSE);
	return TRUE;
}

BOOL CToolHandler_Disabled::Execute(UINT uMsg)
{
	( void )uMsg;
	return TRUE;
}

IToolHandlerInfo * _ToolToHandlerInfo(UINT uMsg)
{
	extern IToolHandlerInfo *g_pToolHandlerSyncMesh;

	switch ( uMsg )
	{
	//	case ID_TOOLS_SYNC_MESH: // the maya tool hasn't been ported yet
	//		return g_pToolHandlerSyncMesh;

		default:
			return NULL;

			// return &g_ToolHandlerDisabled to make a control look disabled
	}
}
#endif
//-----------------------------------------------------------------------------
// Purpose: activates the current tool toolbar button
// Input  : pUI - interface to button that has had a action happen
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateToolUI(CCmdUI *pUI)
{
	if (IsShellSessionActive())
	{
		pUI->Enable(FALSE);
	}
	else
	{
		//
		// check for button enabling
		//
		CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();

		bool	bIsEditable = ( pDoc ? pDoc->IsSelectionEditable() : false );

		if ( pUI->m_nID == ID_TOOLS_APPLYDECALS ||
			 pUI->m_nID == ID_TOOLS_OVERLAY ||
			 pUI->m_nID == ID_TOOLS_CLIPPER ||
#ifdef SLE //// SLE NEW - 3-point clipping tool, a subclass of normal Clipper
			 pUI->m_nID == ID_TOOLS_CLIPPER3POINT ||
#endif
			 pUI->m_nID == ID_TOOLS_MORPH )
		{

		}
		else
		{
			bIsEditable = ( pDoc ? true : false );
		}

#if 0
		//
		// Only enable the displacement toolbar button while editing HalfLife 2 maps.
		//
		if ( pUI->m_nID == ID_TOOLS_DISPLACE )
		{
			if ( pDoc != NULL )
			{
				pUI->Enable(pDoc->GetMapFormat() == mfHalfLife2);
			}
			else
			{
				pUI->Enable( pDoc != NULL );
			}
		}
		else
#endif
		{
			pUI->Enable( bIsEditable );
		}		

		ToolID_t eToolID = _ToolMsgToEnum(pUI->m_nID);
		pUI->Enable( bIsEditable );
		pUI->SetCheck(eToolID == ToolManager()->GetActiveToolID());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles toolbar and menu messages that change the active tool.
// Input  : nMessageID - the id of the menu item
// Output : Returns TRUE to indicate that the message was handled.
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnChangeTool(UINT nMessageID)
{
	//
	// Changing tool -- exit face edit mode if necessary.
	// This is here because face edit mode encompasses two tools: the
	// material tool and the displacement tool. Which tool we use is set
	// by the OnSetActive handler of each page of the face edit sheet.
	//
	if (IsInFaceEditMode())
	{
#ifdef SLE //// SLE NEW - safeguard against accidentally clicking and losing a lot of selected faces
	if ( Options.general.iDeselectFacesThreshold > 0 && GetMainWnd()->m_pFaceEditSheet 
		&& GetMainWnd()->m_pFaceEditSheet->GetFaceListCount() > Options.general.iDeselectFacesThreshold)
	{
		CString str;
		str.Format("This action will de-select currently selected %d faces. Proceed?", GetMainWnd()->m_pFaceEditSheet->GetFaceListCount());

		if ( AfxMessageBox(str, MB_YESNO) == IDNO )
			return FALSE;
	}
#endif
		EnableFaceEditMode(false);
	}
#ifdef HAMMER2013_PORT_CORDONS
	// Obtain custom tool handler
	IToolHandlerInfo *pHandlerInfo = _ToolToHandlerInfo(nMessageID);
	BOOL bHandled = pHandlerInfo ? pHandlerInfo->Execute(nMessageID) : FALSE;

	if ( !bHandled )
#endif
	{
	//
	// Activate the new tool.
	//
		ToolID_t eToolID = _ToolMsgToEnum(nMessageID);
		ToolManager()->SetTool(eToolID);
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
// Purpose: Brings up the 
//-----------------------------------------------------------------------------
void CMainFrame::OnViewMessages(void)
{
	g_pwndMessage->ToggleMessageWindow();
}

//-----------------------------------------------------------------------------
// Purpose: Manages the state of the view messages menu item.
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateViewMessages(CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck( g_pwndMessage->IsVisible() );
}

//-----------------------------------------------------------------------------
// Purpose: Brings up the Object Properties dialog.
//-----------------------------------------------------------------------------
void CMainFrame::OnEditProperties(void)
{
	pObjectProperties->ShowWindow(pObjectProperties->IsWindowVisible() ? SW_HIDE : SW_SHOW);
}

//-----------------------------------------------------------------------------
// Purpose: Tell all the documents to redraw all their views.
//-----------------------------------------------------------------------------
void CMainFrame::UpdateAllDocViews(DWORD dwCmd)
{
	for ( int i=0; i<CMapDoc::GetDocumentCount(); i++ )
	{
		CMapDoc *pDoc = CMapDoc::GetDocument(i);

		if (pDoc->GetGame() != NULL)
		{
			pDoc->UpdateAllViews( dwCmd );
		}
	}

	if (dwCmd & MAPVIEW_UPDATE_VISGROUP_ALL)
	{
		// This updates everything, so it takes priority.
		m_FilterControl.UpdateGroupList();
	}
	else if (dwCmd & MAPVIEW_UPDATE_VISGROUP_STATE)
	{
		// Only update the hidden/shown state of the visgroups.
		m_FilterControl.UpdateGroupListChecks();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Informs our application object when we are activated or deactivated.
// Input  : bActive - TRUE to activate, FALSE to deactivate.
//			hTask - task becoming active.
//-----------------------------------------------------------------------------
#if _MSC_VER < 1300
void CMainFrame::OnActivateApp(BOOL bActive, HTASK hTask)
#else
void CMainFrame::OnActivateApp(BOOL bActive, DWORD hTask)
#endif
{
	CMDIFrameWnd::OnActivateApp(bActive, hTask);

	// Had to change this code to not call GetWindowPlacement because for some reason
	// that prevented Hammer from maximizing properly -- it would leave space on top.
	// So I cache the minimized state instead, which fixes the problem. Sigh.
	if (!m_bMinimized)
	{
		MaterialSystemInterface()->EvictManagedResources();
		APP()->OnActivateApp(bActive == TRUE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nID - 
//			lParam - 
//-----------------------------------------------------------------------------
void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	CMDIFrameWnd::OnSysCommand(nID, lParam);

	if (nID == SC_MINIMIZE)
	{
		m_bMinimized = true;
		APP()->OnActivateApp(false);
	}
	else if ((nID == SC_MAXIMIZE) || (nID == SC_RESTORE))
	{
		m_bMinimized = false;
		APP()->OnActivateApp(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when the active document is deleted.
//-----------------------------------------------------------------------------
void CMainFrame::OnDeleteActiveDocument(void)
{
	pObjectProperties->MarkDataDirty();
}

//-----------------------------------------------------------------------------
// Purpose: Handles resize messages. Resizes any children that depend on our size.
// Input  : nType - 
//			cx - 
//			cy - 
//-----------------------------------------------------------------------------
void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIFrameWnd::OnSize(nType, cx, cy);

	//
	// Resize the message window if it exists.
	//
	if ( g_pwndMessage != NULL )
	{
		CRect clientrect;
		wndMDIClient.GetClientRect(clientrect);

		g_pwndMessage->Resize(CRect(0, clientrect.Height() - 130, clientrect.Width(), clientrect.Height()));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns a pointer to the main frame window.
//-----------------------------------------------------------------------------
CMainFrame *GetMainWnd(void)
{
	return pMainWnd;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nIndex - 
//			pszText - 
//-----------------------------------------------------------------------------
void SetStatusText(int nIndex, LPCTSTR pszText)
{
	GetMainWnd()->GetStatusBar()->SetPaneText(nIndex, pszText);
}

//-----------------------------------------------------------------------------
// Purpose: Invokes the configuration dialog, saving the options if the user
//			hits the OK button.
//-----------------------------------------------------------------------------
#ifdef SLE //// SLE NEW - used to direct the user to the Material config
void CMainFrame::ConfigureMaterials(void)
{
	COptionProperties dlg("Configure Editor", NULL, 0);
	if ( dlg.DoModal() == IDOK  )
	{
		dlg.SetActivePage(3);
		Options.Write(TRUE, TRUE);
	}
}
#endif
void CMainFrame::Configure(void)
{
#ifdef SLE //// SLE CHANGE: Renamed to differentiate. 
	COptionProperties dlg("Configure Editor", NULL, 0);
#else
	COptionProperties dlg("Configure Hammer", NULL, 0);
#endif
	if (dlg.DoModal() == IDOK)
	{
		Options.Write( TRUE, TRUE );
	}
}
#ifdef SLE //// SLE NEW: Opens colour picker dialogue to set selectable void colour setting
//-----------------------------------------------------------------------------
// Purpose: brings up the colour picker for the 3d viewport background
//-----------------------------------------------------------------------------
void CMainFrame::On3DVoidColor(void)
{
	// find current color
	COLORREF clr;
	BYTE r = 255, g = 255, b = 255;
	DWORD brightness = 0xffffffff;

	clr = Options.colors.clr3DBackground;
	
	CColorDialog dlg(clr, CC_FULLOPEN);
	if (dlg.DoModal() != IDOK)
		return;

	r = GetRValue(dlg.m_cc.rgbResult);
	g = GetGValue(dlg.m_cc.rgbResult);
	b = GetBValue(dlg.m_cc.rgbResult);

	clr = RGB(r, g, b);
	Options.colors.clr3DBackground = clr;

	char szBuff[128];       // Temp. buffer
	sprintf(szBuff, "%i %i %i", r, g, b);
	APP()->WriteProfileString("Color Scheme", "3DBackground", szBuff);

	// enable custom colours or the setting won't be applied on next launch
	Options.colors.bUseCustom = true;
	Options.PerformChanges();
//	APP()->WriteProfileInt("Color Scheme", "UseCustom", TRUE);
}

//// SLE NEW - brings up the colour picker for the 3d selection tint
//-----------------------------------------------------------------------------
// Purpose: brings up the colour picker for the 3d selection tint
//-----------------------------------------------------------------------------
void CMainFrame::OnSelectionColor(void)
{
	// find current color
	COLORREF clr;
	BYTE r = 15, g = 220, b = 0;
	DWORD brightness = 0xffffffff;

	clr = Options.colors.clr3DSelection;

	CColorDialog dlg(clr, CC_FULLOPEN);
	if (dlg.DoModal() != IDOK)
		return;

	r = GetRValue(dlg.m_cc.rgbResult);
	g = GetGValue(dlg.m_cc.rgbResult);
	b = GetBValue(dlg.m_cc.rgbResult);

	clr = RGB(r, g, b);
	Options.colors.clr3DSelection = clr;

	char szBuff[128];       // Temp. buffer
	sprintf(szBuff, "%i %i %i", r, g, b);
	APP()->WriteProfileString("Color Scheme", "FaceSelection", szBuff);

	// enable custom colours or the setting won't be applied on next launch
	Options.colors.bUseCustom = true;
	Options.PerformChanges();
//	APP()->WriteProfileInt("Color Scheme", "UseCustom", TRUE);
	
}
#endif //// SLE
//-----------------------------------------------------------------------------
// Purpose: Invokes the configuration dialog.
//-----------------------------------------------------------------------------
void CMainFrame::OnToolsOptions(void)
{
	Configure();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the main frame is closing. Cleans up the dialog bars
//			and saves the options to the registry.
//-----------------------------------------------------------------------------
void CMainFrame::OnClose() 
{
	// Copied from CFrameWnd::OnClose. We can't call APP()->BeginClosing if they
	// hit cancel in the Save Modified dialog, and CFrameWnd::OnClose doesn't have
	// a return code to let us know if we're actually closing. Preposterous.

	// Note: only queries the active document
	CDocument *pDocument = GetActiveDocument();
	if (pDocument != NULL && !pDocument->CanCloseFrame(this))
	{
		// document can't close right now -- don't close it
		return;
	}
	
	//
	// Save the splitter configuration of the first child window in our list.
	//
	CChildFrame *pChild = GetNextMDIChildWnd(NULL);
	if (pChild != NULL)
	{
		pChild->SaveOptions();
	}
	
	CWinApp *pApp = AfxGetApp();
	if (pApp != NULL && pApp->m_pMainWnd == this)
	{
		// attempt to save all documents
		if (pDocument == NULL && !pApp->SaveAllModified())
		{
			// don't close it
			return;
		}

		pApp->CloseAllDocuments( FALSE );
	}
	// End of copied stuff.
	
	APP()->BeginClosing();

	// want to save the faceeditor as hidden
	ShowFaceEditSheetOrTextureBar( false );
	
	SaveBarState("Barstate");
	//AfxGetApp()->WriteProfileInt("General", "NewBars", TRUE);

	// Remove the smoothing group dialog window.
	m_SmoothingGroupDlg.DestroyWindow();

	// save options
	Options.general.bClosedCorrectly = TRUE;
	Options.Write( TRUE, TRUE );

	CMDIFrameWnd::OnClose();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CMainFrame::OnDestroy(void)
{
	CMDIFrameWnd::OnDestroy();
	PostQuitMessage(-1);
}

//-----------------------------------------------------------------------------
// Purpose: Sets a timer for destroying the splash screen.
//-----------------------------------------------------------------------------
void CMainFrame::OnPaint(void)
{
	static bool bFirst = true;

	CPaintDC dc(this); // device context for painting

	if (bFirst)
	{
		bFirst = false;
		SetTimer(FIRST_TIMER, 1, NULL);
	}
}

//-----------------------------------------------------------------------------
// Purpose: This is called ONCE when the splash wnd is to be destroyed. OnPaint()
//			sets the timer.  This is now also called for the autosave timer.
// Input  : nIDEvent - 
//-----------------------------------------------------------------------------
void CMainFrame::OnTimer(UINT nIDEvent) 
{
	if (!::IsWindow(m_hWnd))
	{
		return;
	}
	if( nIDEvent == AUTOSAVE_TIMER )
	{
		APP()->Autosave();
	}
	if( nIDEvent == FIRST_TIMER )  //for the splash scren window destruction
	{
		// only want it once
		KillTimer(nIDEvent);
		
		// Don't continue if Hammer isn't configured yet!
		if (Options.configs.nConfigs == 0)
			return;
		
		Options.SetClosedCorrectly( FALSE );
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
		SetBrightness(Options.textures.fBrightness);
#endif
		// repaint texture window
#ifndef SLE
		m_TextureBar.Invalidate();
#endif
		//when hammer is ready, start the autosave timer.		
		if ( Options.general.iMaxAutosavesPerMap != 0 )
		{
			SetTimer( AUTOSAVE_TIMER, Options.general.iTimeBetweenSaves * 60 * 1000, NULL );  			  
		}		  
#ifdef SLE
		 //// SLE NEW - random mapping tip message in the status bar
		SetTimer(MAPPINGTIP_TIMER, 5 * 1000, NULL);
 
		//// SLE NEW - disp free drag brush, used for sampling mouse pos to determine mouse movement
		SetTimer(MOUSEPOS_TIMER, 50, NULL);
#endif
	}
#ifdef SLE
	
	//// SLE NEW - random mapping tip message in the status bar
	if (nIDEvent == MAPPINGTIP_TIMER)
	{
	//	APP()->UpdateMappingTip(); //// disabled for the public build for now
	}

	//// SLE NEW - disp free drag brush
	if (nIDEvent == MOUSEPOS_TIMER)
	{
		GetCursorPos(&Options.m_storedMousePos);
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Called when timer value is changed in the options
// Input  : void
//-----------------------------------------------------------------------------
void CMainFrame::ResetAutosaveTimer( void )
{
	if ( Options.general.iMaxAutosavesPerMap != 0 )
	{
		SetTimer( AUTOSAVE_TIMER, Options.general.iTimeBetweenSaves * 60 * 1000, NULL );
	}
	else 
	{
		KillTimer( AUTOSAVE_TIMER );
	}
}
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : fBrightness - 
//-----------------------------------------------------------------------------
void CMainFrame::SetBrightness(float fBrightness)
{
	if(fBrightness < 0.1f || fBrightness > 5.0f)
		return;

	// update options
	Options.textures.fBrightness = fBrightness;

	// update display
	for(int i = 0; i < Options.configs.nConfigs; i++)
		Options.configs.Configs[i]->Palette.SetBrightness(fBrightness);
	g_Textures.InformPaletteChanged();

	//
	// if current tool isn't the material tool, then redraw the texture bar
	//
	if ( ToolManager()->GetActiveToolID() != TOOL_FACEEDIT_MATERIAL )
	{
		m_TextureBar.RedrawWindow();
	}
	else
	{
		m_pFaceEditSheet->RedrawWindow();
	}

	// tell all the documents to redraw 3d views
	UpdateAllDocViews( MAPVIEW_UPDATE_ONLY_3D | MAPVIEW_UPDATE_COLOR );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nID - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnView3dChangeBrightness(UINT nID) 
{
	float fBrightness = Options.textures.fBrightness;
	float fModify = (nID == ID_VIEW3D_BRIGHTER) ? 0.2f : -0.2f;
	
	SetBrightness(fBrightness + fModify);

	return TRUE;
}
#endif
//-----------------------------------------------------------------------------
// Purpose: Brings up the sound browser
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnSoundBrowser(UINT nID)
{
	CSoundBrowser dlg("");
	dlg.DoModal();

	return TRUE;
}

//-----------------------------------------------------------------------------
// Purpose: Brings up the sound browser
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnReloadSounds(UINT nID)
{
	for ( int i = 0; i < SOUND_TYPE_COUNT; ++i )
	{
		g_Sounds.BuildSoundList( (SoundType_t)i );
	}

	return TRUE;
}
#ifdef SLE //// SLE NEW - more tools and features accessible from the doc
//// SLE NEW - backports for matsys_controls
//-----------------------------------------------------------------------------
// Purpose: model browser functionality
//-----------------------------------------------------------------------------
CModelBrowser *CMainFrame::GetModelBrowser()
{
#if 1 // destroy-recreate
	if ( m_pModelBrowser)
	{
		m_pModelBrowser = NULL;
		delete m_pModelBrowser;
	}
#endif
	if (m_pModelBrowser == NULL)
	{
		m_pModelBrowser = new CModelBrowser(this);
	}

	return m_pModelBrowser;
}

//-----------------------------------------------------------------------------
// Purpose: Brings up the model browser.
//-----------------------------------------------------------------------------
void CMainFrame::OnModelBrowser()
{
	CModelBrowser *pModelBrowser = GetModelBrowser();
	pModelBrowser->Show();

	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	CUtlVector<AssetUsageInfo_t> usedModels;
	pDoc->GetUsedModels(usedModels);

	pModelBrowser->SetUsedModelList(usedModels);
	int nRet = pModelBrowser->DoModal();
	pModelBrowser->Hide();

	if (nRet == ID_FIND_ASSET)
	{
		// They hit the Find button in the model browser. Invoke the Entity Report dialog
		// to find all occurences of the chosen model.
		char szModelName[1024];
		pModelBrowser->GetModelName(szModelName, sizeof(szModelName));

		EntityReportFilterParms_t filter;
		filter.FilterByKeyValue("model", szModelName);

		CEntityReportDlg::ShowEntityReport(pDoc, this, &filter);
	}
}

//// SLE NEW: option to manually uncache mats and textures
//-----------------------------------------------------------------------------
// Purpose: reloads the materials //// SLE TODO - causes sprites to lose transparency
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnReloadMaterials(UINT nID)
{
	if (g_pMaterialSystem)
	{
		g_pMaterialSystem->UncacheAllMaterials();
	}

	UpdateStudioFileChangeWatcher(); 

	return TRUE;
}

//// SLE NEW: quick fgd reload button
//-----------------------------------------------------------------------------
// Purpose: reloads FGDs without need to change the config
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnReloadFGD(UINT nID)
{
	CGameConfig *currentConfig = CGameConfig::GetActiveGame();
	if (currentConfig)
	{
		currentConfig->LoadGDFiles();

		for (int i = 0; i<CMapDoc::GetDocumentCount(); i++)
		{
			CMapDoc *pDoc = CMapDoc::GetDocument(i);
			pDoc->Update();
		}
	}

	return TRUE;
}
#endif //// SLE
//-----------------------------------------------------------------------------
// Purpose: Toggles face edit mode, which encompasses two different tools, the
//			materials editing tool and the displacement editing tool.
//
//			The tool itself is set by the OnSetActive handler for each property
//			page of the face properties sheet.
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnApplicator(UINT nID)
{
	bool bNewFaceEditMode = !IsInFaceEditMode();

	//
	// Show/hide face edit sheet/texturebar and update the selection set if need be.
	//
	EnableFaceEditMode(bNewFaceEditMode);

	if (!bNewFaceEditMode)
	{
		ToolManager()->SetTool(TOOL_POINTER);
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// Purpose: Enables or disables face edit mode, updating the UI as necessary.
//			When we are in face edit mode, the texture bar is hidden and the
//			face edit property sheet is shown. The active tool is changed
//			to either the material tool or the displacement tool based on which
//			page in the property sheet is active.
//-----------------------------------------------------------------------------
void CMainFrame::EnableFaceEditMode(bool bEnable)
{
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if (!pDoc)
	{
		return;
	}

	ShowFaceEditSheetOrTextureBar(bEnable);
	pDoc->UpdateForApplicator(bEnable);
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if we are in face edit mode, false if not.
//-----------------------------------------------------------------------------
bool CMainFrame::IsInFaceEditMode()
{
	if ((ToolManager()->GetActiveToolID() == TOOL_FACEEDIT_MATERIAL) || (ToolManager()->GetActiveToolID() == TOOL_FACEEDIT_DISP))
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Manages the state of the texture applicator toobar button and menu item.
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateApplicatorUI(CCmdUI *pUI)
{
	if (IsShellSessionActive())
	{
		pUI->Enable(FALSE);
	}
	else
	{
		pUI->SetCheck(IsInFaceEditMode());
		pUI->Enable(CMapDoc::GetActiveMapDoc() ? TRUE : FALSE);
	}
}
//// SLE TODO - there's a bunch of broken/obsolete functionality here, figure out later
//-----------------------------------------------------------------------------
// Purpose: Invokes the prefab manager dialog.
//-----------------------------------------------------------------------------
void CMainFrame::OnToolsPrefabfactory(void)
{
	CPrefabsDlg dlg;
	dlg.DoModal();
	CPrefabLibrary::LoadAllLibraries();
	m_ObjectBar.UpdateListForTool( ToolManager()->GetActiveToolID());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMainFrame::OnHelpFinder(void)
{
#ifdef SLE //// SLE CHANGE - bring back offline .chm help
	APP()->Help(NULL);
#else
	APP()->OpenURL(ID_HELP_TOPICS, GetMainWnd()->GetSafeHwnd());
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pHelpInfo - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnHelpInfo(HELPINFO *pHelpInfo)
{
	return(Default());
}

//-----------------------------------------------------------------------------
// Purpose: Opens a URL in the default web browser.
//-----------------------------------------------------------------------------
void CMainFrame::OpenURL(const char *pszURL)
{
	APP()->OpenURL(pszURL, m_hWnd);
}

//-----------------------------------------------------------------------------
// Purpose: Opens a URL in the default web browser by string ID.
//-----------------------------------------------------------------------------
void CMainFrame::OpenURL(UINT nID)
{
	APP()->OpenURL(nID, m_hWnd);
}

//-----------------------------------------------------------------------------
// Purpose: Opens the URL that corresponds to the given string ID. This is used
//			to hook menu items to URLs in the string table.
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnHelpOpenURL(UINT nID)
{
	OpenURL(nID);
	return TRUE;
}
#ifdef SLE //// SLE NEW - link directly to GitHub repo
void CMainFrame::OnOpenGitHubIssues()
{
	CString strText;
	strText.Append("This will open the following URL in your default browser:\n");
	strText.AppendFormat(ID_REPORT_GITHUB);
	strText.Append("\nProceed?");

	if ( AfxMessageBox(strText, MB_ICONQUESTION | MB_YESNO) == IDYES )
	{
		APP()->OpenURL(ID_REPORT_GITHUB, GetMainWnd()->GetSafeHwnd());
	}
}
#endif
//-----------------------------------------------------------------------------
// Purpose: Activates or deactivates Undo/Redo.
//-----------------------------------------------------------------------------
void CMainFrame::SetUndoActive(BOOL bActive)
{
	m_bUndoActive = bActive;
	CMapDoc::GetActiveMapDoc()->SetUndoActive(bActive == TRUE);
}

//-----------------------------------------------------------------------------
// Purpose: Toggles the active state of Undo/Redo.
//-----------------------------------------------------------------------------
void CMainFrame::OnEditUndoredoactive(void)
{
	SetUndoActive(!m_bUndoActive);	
}

//-----------------------------------------------------------------------------
// Purpose: Manages the state of the Enable/Disable Undo/Redo menu item.
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateEditUndoredoactive(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(IsShellSessionActive() ? FALSE : TRUE);
	pCmdUI->SetText(m_bUndoActive ? "Disable Undo/Redo" : "Enable Undo/Redo");
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nCode - 
//-----------------------------------------------------------------------------
void CMainFrame::GlobalNotify(int nCode)
{
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();

	switch (nCode)
	{
		//
		// Active document changed. Update visgroup lists. 
		//
		case WM_MAPDOC_CHANGED:
		{
			//
			// Update the visgroups.
			//
			m_FilterControl.UpdateGroupList();
#ifdef HAMMER2013_PORT_CORDONS
			m_FilterControl.UpdateCordonList();
#endif
			//
			// If the Object Properties dialog has a Groups tab, update
			// the groups tab.
			//
			if (pObjectProperties != NULL)
			{
				pObjectProperties->UpdateGrouplist();
			}

			if (pDoc != NULL)
			{
				pDoc->UpdateStatusbar();
				//m_AnimationDlg.SelectionChanged(*pDoc->Selection_GetList());
			}

			m_ManifestFilterControl.UpdateManifestList();
			break;
		}
		
		//
		// Game configuration changed. Update texture and entity lists.
		//
		case WM_GAME_CHANGED:
		{
			pTextureBrowser->SetTextureFormat(g_pGameConfig->GetTextureFormat());
			m_TextureBar.NotifyGraphicsChanged();
			m_pFaceEditSheet->NotifyGraphicsChanged();

			if (pDoc != NULL)
				m_ObjectBar.UpdateListForTool( pDoc->GetTools()->GetActiveToolID());
			break;
		}

		//
		// Lighting preview window closed
		//
		case LPRV_WINDOWCLOSED:
		{
			m_bLightingPreviewOutputWindowShowing = false;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : UINT - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnFileNew(UINT)
{
	return FALSE;

	CNewDocType dlg;
	dlg.m_iNewType = 0;

	if(dlg.DoModal() != IDOK)
		return TRUE;

	return FALSE;
}

//// SLE TODO - this is almost certainly broken. Needs to be figured out
//// and switched to using a more intuitive, human-readable format.
//-----------------------------------------------------------------------------
// Purpose: Saves the position and types of all 2D and 3D views in the active document.
// dvs: This really needs to be a text file instead of a binary file!
// Input  : *pFile - 
//-----------------------------------------------------------------------------
void CMainFrame::SaveWindowStates(std::fstream *pFile)
{
	char szRootDir[MAX_PATH];
	char szFullPath[MAX_PATH];
	APP()->GetDirectory(DIR_PROGRAM, szRootDir);
	Q_MakeAbsolutePath( szFullPath, MAX_PATH, "winstate.wc", szRootDir ); 

	std::fstream file(szFullPath, std::ios::out | std::ios::binary);

	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if (pDoc == NULL)
	{
		return;
	}

	file.write(WINSTATETAG, sizeof WINSTATETAG);
	file.write((char*) &fVersion, sizeof fVersion);

	CRect rectClient;
	::GetClientRect(m_hWndMDIClient, &rectClient);

	// write out each view
	POSITION p = pDoc->GetFirstViewPosition();
	while (p != NULL)
	{
		CView *pView = pDoc->GetNextView(p);
		
		//
		// Determine what type of view it is.
		//
		int iDrawType;
		if (pView->IsKindOf(RUNTIME_CLASS(CMapView2D)))
		{
			file.write((char*) &WINSTATE2DVIEW, sizeof WINSTATE2DVIEW);
			iDrawType = (int)((CMapView2D*)pView)->GetDrawType();
		}
		else if (pView->IsKindOf(RUNTIME_CLASS(CMapView3D)))
		{
			file.write((char*) &WINSTATE3DVIEW, sizeof WINSTATE3DVIEW);
			iDrawType = (int)((CMapView3D*)pView)->GetDrawType();
		}
		else if (pView->IsKindOf(RUNTIME_CLASS(CMapViewLogical)))
		{
			file.write((char*) &WINSTATELOGICALVIEW, sizeof WINSTATELOGICALVIEW);
			iDrawType = (int)((CMapViewLogical*)pView)->GetDrawType();
		}
		else
		{
			//
			// It's a view type whose state we do not save - skip it.
			//
			continue;
		}

		//
		// Write view's draw type.
		//
		file.write((char*) &iDrawType, sizeof iDrawType);

		//
		// Write position of view.
		//
		CRect rectView;
		pView->GetParentFrame()->GetWindowRect(&rectView);
		CPoint pt1 = rectView.TopLeft(), pt2 = rectView.BottomRight();
		::ScreenToClient(m_hWndMDIClient, &pt1);
		::ScreenToClient(m_hWndMDIClient, &pt2);
	
		double left, top, right, bottom;
		left =		double(pt1.x) / double(rectClient.right);
		top =		double(pt1.y) / double(rectClient.bottom);
		right =		double(pt2.x) / double(rectClient.right);
		bottom =	double(pt2.y) / double(rectClient.bottom);

		file.write((char*) &left, sizeof left);
		file.write((char*) &top, sizeof top);
		file.write((char*) &right, sizeof right);
		file.write((char*) &bottom, sizeof bottom);
	}

	file.write((char *)&WINSTATEEND, sizeof WINSTATEEND);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pFile - 
//-----------------------------------------------------------------------------
void CMainFrame::LoadWindowStates(std::fstream *pFile)
{
	char szRootDir[MAX_PATH];
	char szFullPath[MAX_PATH];
	APP()->GetDirectory(DIR_PROGRAM, szRootDir);
	Q_MakeAbsolutePath( szFullPath, MAX_PATH, "winstate.wc", szRootDir ); 

	std::fstream file( szFullPath, std::ios::in | std::ios::binary );

	if (!file.is_open())
	{
		return;
	}

	char tag[sizeof(WINSTATETAG)];
	file.read(tag, sizeof tag);

	if(memcmp(tag, WINSTATETAG, sizeof tag))
	{
		file.seekg(-int(sizeof(tag)));
		return;
	}

	float fThisVersion;
	file.read((char*) &fThisVersion, sizeof fThisVersion);

	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();

	if(!pDoc)
		return;

	// get client rect of MDI CHILD for relative positioning information
	CRect rectClient;
	::GetClientRect(m_hWndMDIClient, &rectClient);

	// keep list of views we've already modified, so if there
	//  are other views, we have to create them. this prevents
	//  us from having to delete all views and start over, 
	//  which is a slower process than simply moving existing
	//	views.
	CTypedPtrList<CPtrList, CView*> UsedViews;

	SetDefaultChildType(FALSE);

	while (1)
	{
		int iViewType;
		file.read((char *)&iViewType, sizeof iViewType);
		if ((file.eof()) || (iViewType == WINSTATEEND))
		{
			break;
		}

		int iDrawType;
		file.read((char *)&iDrawType, sizeof iDrawType);

		CView *pView = NULL;

		// find a view we haven't used
		POSITION p = pDoc->GetFirstViewPosition();
		while (p != NULL)
		{
			CView *pThisView = pDoc->GetNextView(p);

			// already used?
			if (UsedViews.Find(pThisView))
				continue;

			// make sure it's the right type ..
			if (iViewType == WINSTATE2DVIEW && !pThisView->IsKindOf(RUNTIME_CLASS(CMapView2D)))
				continue;

			if (iViewType == WINSTATELOGICALVIEW && !pThisView->IsKindOf(RUNTIME_CLASS(CMapViewLogical)))
				continue;

			if (iViewType == WINSTATE3DVIEW && !pThisView->IsKindOf(RUNTIME_CLASS(CMapView3D)))
				continue;

			// yes! so modify this one.
			pView = pThisView;
			UsedViews.AddTail(pView);
			break;
		}

		CChildFrame *pFrame = NULL;
		BOOL bNew = FALSE;
		CDocTemplate *pTemplate = NULL;

		if(!pView)
		{
			// if no view was created, we have to create a new one.
			CMDIChildWnd* pActiveChild = MDIGetActive();
			pTemplate = pDoc->GetDocTemplate();
			pFrame = (CChildFrame*) pTemplate->CreateNewFrame(pDoc, pActiveChild);
			pFrame->SetRedraw(FALSE);
			pTemplate->InitialUpdateFrame(pFrame, pDoc, FALSE);
			
			// find view in new frame
			pView = pFrame->GetActiveView();

			UsedViews.AddTail(pView);
			bNew = TRUE;
		}
		else
		{
			// find frame based on this view
			pFrame = (CChildFrame*) pView->GetParentFrame();

			if(pFrame->bUsingSplitter)
				pFrame->SetSplitterMode(FALSE);
		}

		// no redraws right now, please.
		pFrame->SetRedraw(FALSE);

		if (iViewType == WINSTATE3DVIEW)
		{
			//
			// Handle import of old WinState files before draw types were consolidated
			// into a single enumeration.
			//
			if ((iDrawType >= VIEW2D_XY) && (iDrawType <= VIEW2D_XZ))
			{
				iDrawType += 3;
			}
			pFrame->SetViewType((DrawType_t)iDrawType);
		}
		else if (iViewType == WINSTATE2DVIEW)
		{
			pFrame->SetViewType((DrawType_t)iDrawType);
		}
		else if (iViewType == WINSTATELOGICALVIEW)
		{
			pFrame->SetViewType( (DrawType_t)iDrawType );
		}
		
		// read positioning info
		double left, top, right, bottom;
		file.read((char*) &left, sizeof left);
		file.read((char*) &top, sizeof top);
		file.read((char*) &right, sizeof right);
		file.read((char*) &bottom, sizeof bottom);
		CRect r;
		r.left		= int(left * double(rectClient.right));
		r.top		= int(top * double(rectClient.bottom));
		r.right		= int(right * double(rectClient.right));
		r.bottom	= int(bottom * double(rectClient.bottom));

		// Set the frame's position.
		pFrame->MoveWindow(&r, FALSE);

		// Call OnInitialUpdate before any rendering takes place.
		if (bNew)
		{
			pTemplate->InitialUpdateFrame(pFrame, pDoc, TRUE);
		}

		// Enable WM_PAINT messages for the frame window.
		pFrame->SetRedraw(TRUE);

		// Update the window.
		pFrame->Invalidate();
		pFrame->UpdateWindow();
	}

	Invalidate();
	UpdateWindow();
}

void CMainFrame::OnInitMenu( CMenu *pMenu )
{
}

void CMainFrame::OnHDR()
{
#ifdef SLE	//// SLE CHANGE - albeit unused at the moment, this has also been changed because 
			//// lighting preview was replaced with BSP lighting
	g_bHDR = !g_bHDR;
	MaterialSystemInterface()->OverrideConfig(MaterialSystemConfig(), true);
	DrawMenuBar();
	SignalUpdate( EVTYPE_LIGHTING_CHANGED );
#else
	CMenu *pMenu= GetMenu();

	UINT state = pMenu->GetMenuState(ID_HDR, MF_BYCOMMAND);
	
	if (state & MF_CHECKED)
	{
		pMenu->CheckMenuItem(ID_HDR, MF_UNCHECKED | MF_BYCOMMAND);
		g_bHDR = false;
	}
	else
	{
		pMenu->CheckMenuItem(ID_HDR, MF_CHECKED | MF_BYCOMMAND);
		g_bHDR = true;
	}
	DrawMenuBar();
	SignalUpdate( EVTYPE_LIGHTING_CHANGED );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMainFrame::OnSavewindowstate(void)
{
	SaveWindowStates();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMainFrame::OnLoadwindowstate(void)
{
	LoadWindowStates();
}

//-----------------------------------------------------------------------------
// Purpose: Changes the format of the units displayed in the status bar.
// Input  : nID - Menu ID corresponding to a units format.
//-----------------------------------------------------------------------------
BOOL CMainFrame::OnUnits(UINT nID)
{
	switch (nID)
	{
		case ID_MAP_UNITS_NONE:
		{
			Box3D::SetWorldUnits(Units_None);
			break;
		}

		case ID_MAP_UNITS_INCHES:
		{
			Box3D::SetWorldUnits(Units_Inches);
			break;
		}

		case ID_MAP_UNITS_FEET_INCHES:
		{
			Box3D::SetWorldUnits(Units_Feet_Inches);
			break;
		}
#ifdef SLE //// SLE NEW - more units choices
		case ID_MAP_UNITS_CENTIMETERS: //// SLE NEW - show units in cm
		{
			Box3D::SetWorldUnits(Units_Centimeters);
			break;
		}

		case ID_MAP_UNITS_METERS_CENTIMETERS: //// SLE NEW - show units in m + cm:
		{
			Box3D::SetWorldUnits(Units_Meters_Centimeters);
			break;
		}
	}
#endif
	CMapDoc::GetActiveMapDoc()->UpdateStatusbar();

	return TRUE;
}

//-----------------------------------------------------------------------------
// Purpose: Manages the state of decal application toolbar button.
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateUnits(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(!IsShellSessionActive());

	if (pCmdUI->m_nID == ID_MAP_UNITS_NONE)
	{
		pCmdUI->SetCheck(Box3D::GetWorldUnits() == Units_None);
	}
	else if (pCmdUI->m_nID == ID_MAP_UNITS_INCHES)
	{
		pCmdUI->SetCheck(Box3D::GetWorldUnits() == Units_Inches);
	}
	else if (pCmdUI->m_nID == ID_MAP_UNITS_FEET_INCHES)
	{
		pCmdUI->SetCheck(Box3D::GetWorldUnits() == Units_Feet_Inches);
	}
#ifdef SLE //// SLE NEW - more units choices
	else if (pCmdUI->m_nID == ID_MAP_UNITS_CENTIMETERS) //// SLE NEW - show units in cm
	{
		pCmdUI->SetCheck(Box3D::GetWorldUnits() == Units_Centimeters);
	}
	else if (pCmdUI->m_nID == ID_MAP_UNITS_METERS_CENTIMETERS) //// SLE NEW - show units in m + cm
	{
		pCmdUI->SetCheck(Box3D::GetWorldUnits() == Units_Meters_Centimeters);
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pMsg - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	//
	// See if the message is a keydown and the current focus window is the 
	// ComboBox in the ObjectBar!
	//
	/*
	if (pMsg->message == WM_KEYDOWN)
	{
		if ((GetFocus() == &m_ObjectBar) || (GetFocus() == &this->m_FilterControl))
		{
			AfxMessageBox("Ok");
			return(TRUE);
		}
	}
	*/

	return(CMDIFrameWnd::PreTranslateMessage(pMsg));
}

//-----------------------------------------------------------------------------
// Purpose: Finds the next CChildFrame in the list of MDI child windows.
// Input  : pCurChild - Child to search from.
//-----------------------------------------------------------------------------
CChildFrame *CMainFrame::GetNextMDIChildWnd(CChildFrame *pCurChild)
{
	return GetNextMDIChildWndRecursive(pCurChild);
} 

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CChildFrame *CMainFrame::GetNextMDIChildWndRecursive(CWnd *pCurChild)
{
	CWnd *pNextChild = NULL;

	if (pCurChild == NULL)
	{
		// Get the first child window.
		pNextChild = wndMDIClient.GetWindow(GW_CHILD);
	}
	else
	{
		// Get the next child window in the list.
		pNextChild = pCurChild->GetWindow(GW_HWNDNEXT);
		if (!pNextChild)
		{
			// No child windows exist in the MDIClient,
			// or you are at the end of the list. This check
			// will terminate any recursion.
			return NULL;
		}
	}

	// Check the kind of window
    if (!pNextChild->GetWindow(GW_OWNER))
	{
        if (pNextChild->IsKindOf(RUNTIME_CLASS(CChildFrame)))
		{
			return (CChildFrame *)pNextChild;
		}
	}

	// Not one we are interested in. Try the next one.
	// Recurse over the window manager's list of windows.
	return GetNextMDIChildWndRecursive(pNextChild);
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if we are currently editing via the engine, false if not.
//-----------------------------------------------------------------------------
bool CMainFrame::IsShellSessionActive(void)
{
	return(m_bShellSessionActive);
}

//-----------------------------------------------------------------------------
// Purpose: Manages the state of all Edit menu items and toolbar buttons.
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateEditFunction(CCmdUI *pCmdUI) 
{
	pCmdUI->Enable(!IsShellSessionActive());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CMainFrame::ShowFaceEditSheetOrTextureBar( bool bShowFaceEditSheet )
{
	if( bShowFaceEditSheet )
	{
		m_pFaceEditSheet->SetVisibility( true );
		ShowControlBar( &m_TextureBar, FALSE, TRUE );
	}
	else
	{
		m_pFaceEditSheet->SetVisibility( false );
		m_pFaceEditSheet->CloseAllPageDialogs();
		ShowControlBar( &m_TextureBar, TRUE, TRUE );	
	}
}

//-----------------------------------------------------------------------------
// Purpose: Displays the search/replace dialog. It will be hide itself when the
//			user closes it.
//-----------------------------------------------------------------------------
void CMainFrame::ShowSearchReplaceDialog(void)
{
	if (m_pSearchReplaceDlg == NULL)
	{
		m_pSearchReplaceDlg = new CSearchReplaceDlg;
		m_pSearchReplaceDlg->Create(this);
	}

	m_pSearchReplaceDlg->ShowWindow(SW_SHOW);
	m_pSearchReplaceDlg->SetFocus();
}

//-----------------------------------------------------------------------------
// Purpose: Code found on codeproject.com.
//
//			Makes sure we don't have bogus dialog bar information in the registry.
//			This prevents a crash in the MFC code if a version of the editor with
//			different toolbars was run before us.
//
// TODO: fix the registry settings if they are bad so we can still load the bar state
//
// Output : Returns true if it is safe to load the toolbar settings, false if not.
//-----------------------------------------------------------------------------
bool CMainFrame::VerifyBarState(void)
{
    CDockState state;
    state.LoadState("BarState");

    for (int i = 0; i < state.m_arrBarInfo.GetSize(); i++)
    {
        CControlBarInfo* pInfo = (CControlBarInfo*)state.m_arrBarInfo[i];

        Assert(pInfo != NULL);

        int nDockedCount = pInfo->m_arrBarID.GetSize();
        if (nDockedCount > 0)
        {
            for (int j = 0; j < nDockedCount; j++)
            {
                UINT nID = (UINT) pInfo->m_arrBarID[j];
                if (nID == 0)
				{
					continue; // row separator
				}

                if (nID > 0xFFFF)
				{
                    nID &= 0xFFFF; // placeholder - get the ID
				}

                if (GetControlBar(nID) == NULL)
				{
                    return false;
				}
            }
        }
        
        if (!pInfo->m_bFloating) // floating dockbars can be created later
		{
            if (GetControlBar(pInfo->m_nBarID) == NULL)
			{
                return false; // invalid bar ID
			}
		}
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : dwData - 
//			nCmd - 
//-----------------------------------------------------------------------------
void CMainFrame::WinHelp(DWORD dwData, UINT nCmd)
{
	// dvs: HACK: just punt them to the main help page
	APP()->OpenURL(ID_HELP_TOPICS, m_hWnd);
}

#ifdef SLE_WINTAB_ENABLE //// SLE NEW - Tablet support w/ Wintab
LRESULT CMainFrame::OnWTPacket(WPARAM wSerial, LPARAM hCtx)
{
	WinTab_Packet(wSerial, hCtx);

	return TRUE;
}
#endif
