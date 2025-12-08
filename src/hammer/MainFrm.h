//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef MAINFRM_H
#define MAINFRM_H
#ifdef _WIN32
#pragma once
#endif

#include "MDIClientWnd.h"
#include "FilterControl.h"
#include "ObjectBar.h"
#include "Texturebar.h"
#include "MapAnimationDlg.h"
#include "SelectModeDlgBar.h"
#include "SmoothingGroupsDlg.h" // renamed for clarity
#include "ManifestDialog.h"
#ifdef SLE
#include "archdlg.h"
//#include "RenderSettingsDlg.h"
#endif
class CChildFrame;
class CObjectProperties;
class CTextureBrowser;
#ifdef SLE //// SLE NEW - backports for matsys_controls
class CModelBrowser;
#endif
class CSearchReplaceDlg;
class CFaceEditSheet;
class CMessageWnd;
class CLightingPreviewResultsWindow;

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC( CMainFrame )

public:
	CMainFrame();
	virtual ~CMainFrame();

	bool VerifyBarState(void);

	void BeginShellSession(void);
	void EndShellSession(void);
	bool IsShellSessionActive(void);

	BOOL IsUndoActive() { return m_bUndoActive; }
	void SetUndoActive(BOOL bActive);

	void UpdateAllDocViews(DWORD dwCmd);
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
	void SetBrightness(float fBrightness);
#endif
	void ConfigureMaterials();
	void Configure();

	void GlobalNotify(int nCode);
	void OnDeleteActiveDocument(void);

	void LoadWindowStates(std::fstream *pFile = NULL);

	inline CFaceEditSheet *GetFaceEditSheet( void ) { return m_pFaceEditSheet; }
	inline CStatusBar *GetStatusBar() { return &m_wndStatusBar; }

	void ShowSearchReplaceDialog(void);
	void ShowFaceEditSheetOrTextureBar( bool bShowFaceEditSheet );
	HACCEL GetAccelTable( void ) { return m_hAccelTable; }

	CFaceSmoothingVisualDlg *GetSmoothingGroupDialog( void )	{ return &m_SmoothingGroupDlg; }
#ifdef SLE //// SLE NEW - backports for matsys_controls
	CModelBrowser *GetModelBrowser();
#endif
	void ResetAutosaveTimer();

	bool IsInFaceEditMode();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void OpenURL(const char *pszURL);
	void OpenURL(UINT nID);

	//
	// Public attributes. FIXME: eliminate!
	//
	CTextureBrowser			*pTextureBrowser;
	CObjectProperties		*pObjectProperties;

	CFilterControl			m_FilterControl;
	CObjectBar				m_ObjectBar;
	CTextureBar				m_TextureBar;
	CManifestFilter			m_ManifestFilterControl;
	CFaceEditSheet			*m_pFaceEditSheet;
	CLightingPreviewResultsWindow *m_pLightingPreviewOutputWindow;
	bool					m_bLightingPreviewOutputWindowShowing;
#ifdef SLE
//	CMapAnimationDlg		m_AnimationDlg;
//	CRenderSettingsDlg		m_RenderSettings;
#endif

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual void WinHelp(DWORD dwData, UINT nCmd);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditProperties();
	afx_msg void OnViewMessages();
	afx_msg void OnUpdateViewMessages(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
#ifdef SLE
	afx_msg void On3DVoidColor(); //// SLE NEW - selectable void colour setting - open colour picker dialogue
	afx_msg void OnSelectionColor(); //// SLE NEW - configure tint on selected objects
	afx_msg void OnOpenGitHubIssues(); //// SLE NEW - link directly to GitHub repo
#endif
	afx_msg void OnToolsOptions();
	afx_msg void OnViewShowconnections();
	afx_msg void OnToolsPrefabfactory();
	afx_msg BOOL OnHelpOpenURL(UINT nID);
	afx_msg void OnHelpFinder();
	afx_msg void OnEditUndoredoactive();
	afx_msg void OnUpdateEditUndoredoactive(CCmdUI* pCmdUI);
	afx_msg BOOL OnFileNew(UINT);
	afx_msg void OnSavewindowstate();
	afx_msg void OnLoadwindowstate();
	afx_msg void OnFileOpen();
	afx_msg BOOL OnChange3dViewType(UINT nID);
	afx_msg BOOL OnUnits(UINT nID);
	afx_msg void OnUpdateUnits(CCmdUI *pCmdUI);
	afx_msg void OnUpdateView3d(CCmdUI *pCmdUI);
	afx_msg void OnUpdateView2d(CCmdUI *pCmdUI);
	afx_msg void OnUpdateToolUI(CCmdUI *pUI);
	afx_msg void OnUpdateApplicatorUI(CCmdUI *pUI);
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
	afx_msg BOOL OnView3dChangeBrightness(UINT nID);
#endif
	afx_msg BOOL OnHelpInfo(HELPINFO*);
	afx_msg void OnEnterMenuLoop( BOOL bIsTrackPopupMenu );
#if _MSC_VER < 1300
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#else
	afx_msg void OnActivateApp(BOOL bActive, DWORD hThread);
#endif
	afx_msg void OnUpdateEditFunction(CCmdUI *pCmdUI);
	afx_msg BOOL OnApplicator(UINT nID);
	afx_msg BOOL OnSoundBrowser(UINT nID);
	afx_msg BOOL OnReloadSounds(UINT nID);
#ifdef SLE
	afx_msg void OnModelBrowser(); //// SLE NEW - backports for matsys_controls
	afx_msg BOOL OnReloadMaterials(UINT nID); //// SLE NEW: option to manually uncache mats and textures
	afx_msg BOOL OnReloadFGD(UINT nID); //// SLE NEW: quick fgd reload button
#endif
	afx_msg void OnUpdateOpaqueMaterials(CCmdUI *pCmdUI);
	afx_msg void OnOpaqueMaterials();
#ifdef SLE //// SLE NEW - preview normal maps, diffuse, specular...
	afx_msg void OnUpdateShowDiffuse(CCmdUI *pCmdUI);
	afx_msg void OnShowDiffuse();
	afx_msg void OnUpdateShowNormalMaps(CCmdUI *pCmdUI);
	afx_msg void OnShowNormalMaps();
	afx_msg void OnUpdateShowSpecular(CCmdUI *pCmdUI);
	afx_msg void OnShowSpecular();
	afx_msg void OnUpdateUseEditorMode(CCmdUI *pCmdUI);
	afx_msg void OnUseEditorMode();
	afx_msg void OnToggleInstancesLoading(); //// SLE NEW - control to disable loading instances
	afx_msg void OnUpdateToggleInstancesLoading(CCmdUI *pCmdUI);
#endif
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnChangeTool(UINT nID);
	afx_msg void OnInitMenu( CMenu *pMenu );
	afx_msg void OnHDR( void );
#ifdef SLE_WINTAB_ENABLE //// SLE NEW - Tablet support w/ Wintab
	afx_msg LRESULT OnWTPacket(WPARAM, LPARAM);
#endif
#ifdef SLE
	afx_msg BOOL OnEraseBkgnd(CDC* pDC)
	{
#ifdef SLE_DARK_THEME //// SLE NEW - dark theme test
		CBrush backBrush(SLE_DARK_THEME_CLR_BACK); // This color blends with the splash image!
											  // Save old brush
		CBrush *pOldBrush = pDC->SelectObject(&backBrush);

		CRect rect;
		pDC->GetClipBox(&rect);     // Erase the area needed

		pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);

		pDC->SelectObject(pOldBrush);
#endif
		return TRUE;
	}
#endif
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:

	void DockControlBarRightOf(CControlBar *Bar, CControlBar *RightOf); //// renamed in SLE for clarity
#ifdef SLE 
	void DockControlBarUnderOther(CControlBar *Bar, CControlBar *Other);
#endif
	void SaveWindowStates(std::fstream *pFile = NULL);

	CChildFrame *GetNextMDIChildWnd(CChildFrame *pCurChild);
	CChildFrame *GetNextMDIChildWndRecursive(CWnd *pCurChild);

	void EnableFaceEditMode(bool bEnable);
	void Autosave( void );
	void LoadOldestAutosave( void );

	CMDIClientWnd			wndMDIClient;			// dvs: what in God's name is this for?

	CSearchReplaceDlg		*m_pSearchReplaceDlg;
#ifdef SLE //// SLE NEW - backports for matsys_controls
	CModelBrowser			*m_pModelBrowser;
#endif
	BOOL					m_bUndoActive;

	CStatusBar				m_wndStatusBar;
	CToolBar				m_wndUndoRedoToolBar;
	CToolBar				m_wndMapEditToolBar;
	CToolBar				m_wndMapOps;

#ifdef SLE //// SLE NEW - revised/cleaned up tool bar
	CToolBar				m_wndGridSelectionToolBar;
	CToolBar				m_wndGroupHideCordonTexToolBar;
	CToolBar				m_wndVisibilityToolBar;
	CToolBar				m_wndCheckAndCompileToolBar;
#else
	CToolBar				m_wndMapToolBar;
#endif
	CSelectModeDlgBar		m_SelectModeDlg;

	CFaceSmoothingVisualDlg	m_SmoothingGroupDlg;

	bool					m_bMinimized;
	bool					m_bShellSessionActive;		// Whether a client has initiated a remote shell editing session.
	CBitmap					m_bmMapEditTools256; 
	
#ifdef SLE //// SLE NEW - revised/cleaned up tool bar
	CBitmap					m_bmUndoRedo;
	CBitmap					m_bmGridSelection;
	CBitmap					m_bmGroupHideCordonTex;
	CBitmap					m_bmVisibility;
	CBitmap					m_bmCheckCompile;
#endif
	enum
	{
		AUTOSAVE_TIMER,
		FIRST_TIMER,
#ifdef SLE
		MAPPINGTIP_TIMER, //// SLE NEW - random mapping tip message in the status bar
		SOUNDSCAPE_TIMER, //// SLE NEW - WIP soundscape playback
		MOUSEPOS_TIMER //// SLE NEW - disp free drag brush, used for sampling mouse pos to determine mouse movement
#endif
	};
};

CMainFrame *GetMainWnd();

#endif // MAINFRM_H