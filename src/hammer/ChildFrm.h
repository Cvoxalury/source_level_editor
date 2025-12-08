//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#ifndef CHILDFRM_H
#define CHILDFRM_H
#ifdef _WIN32
#pragma once
#endif

enum DrawType_t;

class CMySplitterWnd : public CSplitterWnd
{
public:
	CMySplitterWnd(void) { pMaxPrev = NULL; }

	int sizePrev[2][2];
	CWnd *pMaxPrev;

	void ToggleMax(CWnd *pWnd);
};

class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
		
		public:
	
	CChildFrame(void);
	
	CView *ReplaceView(CRuntimeClass *pViewClass);
	void SetSplitterMode(BOOL bSplitter);
	CView *GetActiveView(void);
	void SaveOptions(void);
	void SetViewType(DrawType_t eViewType);
	void CenterViews(void);
#ifdef SLE 
	void Switch3dViewToSpecified(DrawType_t eViewType);
#endif
	BOOL bAutosize4, bFirstPaint;
	BOOL bUsingSplitter;
	BOOL m_bReady;
	BOOL m_bNeedsCentered;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildFrame)
public:
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:
	virtual ~CChildFrame(void);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	//{{AFX_MSG(CChildFrame)
	afx_msg void OnSetFocus( CWnd* pOldWnd ); 
	afx_msg void OnClose();
	afx_msg void OnView2dxy();
	afx_msg void OnView2dyz();
	afx_msg void OnView2dxz();
	afx_msg void OnViewLogical();
	afx_msg void OnView3dPolygon();
	afx_msg void OnView3dTextured();
	afx_msg void OnView3dTexturedShaded();
	afx_msg void OnView3dLightingPreview();
	afx_msg void OnView3dLightingPreviewRayTraced();
	afx_msg void OnView3dLightmapGrid();
	afx_msg void OnView3dWireframe();
	afx_msg void OnView3dSmooth(void);
	afx_msg void OnView3dEngine(void);
	//afx_msg void OnUpdateView2d(CCmdUI* pCmdUI);
	//afx_msg void OnUpdateView3d(CCmdUI* pCmdUI);
	afx_msg void OnViewAutosize4();
	afx_msg void OnUpdateViewAutosize4(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnViewMaximizepane();
	afx_msg void OnUpdateViewMaximizepane(CCmdUI* pCmdUI);
	afx_msg void OnWindowToggle();
#ifdef SLE
	afx_msg void OnMaximize3DView(); //// SLE NEW - space for maximising/restoring 3d viewport exclusively
	afx_msg void OnUpdateMaximize3DView(CCmdUI* pCmdUI);
	afx_msg void OnView3dBSPLightingPreview(); //// SLE NEW - separate view for BSP lighting
	afx_msg void OnSwitch3dToWireframe(void); //// SLE NEW - separate commands for switching 3d view modes through F1-4 hotkeys.
	afx_msg void OnSwitch3dToFlatPolys(void);
	afx_msg void OnSwitch3dToTextured(void);
	afx_msg void OnSwitch3dToShaded(void);
	afx_msg void OnSwitch3dToLightmaps(void);
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

		CMySplitterWnd *m_wndSplitter;
};

void SetDefaultChildType(BOOL b4Views);

#endif // CHILDFRM_H
