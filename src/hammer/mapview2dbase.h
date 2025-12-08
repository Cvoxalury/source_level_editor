//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef MAPVIEW2DBASE_H
#define MAPVIEW2DBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "Axes2.h"
#include "MapView.h"
#include "MapClass.h"		// For CMapObjectList
#include "utlvector.h"
#include "VGuiWnd.h"
#include "color.h"
#ifdef HAMMER2013_PORT_KEYBINDS_X
#include "Keyboard.h"
#endif
class CTitleWnd;
class CMapDoc;
class Tool3D;
#ifdef HAMMER2013_PORT_KEYBINDS_X
//
// Defines the logical keys.
//
typedef enum
{
	LOGICAL_KEY_2D_SELNEXT = 1,
	LOGICAL_KEY_2D_SELPREV,
	LOGICAL_KEY_2D_SELNEXT_ALT, // because logical keys can only have one bind
	LOGICAL_KEY_2D_SELPREV_ALT,
	LOGICAL_KEY_2D_PANNING,
	LOGICAL_KEY_2D_ZOOM_OUT,
	LOGICAL_KEY_2D_ZOOM_IN,
	LOGICAL_KEY_2D_ZOOM_OUT_ALT,
	LOGICAL_KEY_2D_ZOOM_IN_ALT,
	LOGICAL_KEY_2D_ZOOM_MIN,
} CMapView2D_LogicalKeys_t;
#endif
class CMapView2DBase : public CView, public CMapView, public Axes2, public CVGuiWnd
{
// Operations
public:
	LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

	void SetViewOrigin( float flHorz, float flVert, bool bRelative = false );
			
	void SetZoom(float flNewZoom);
	float GetZoom( void );

	void CenterView(Vector *pt3 = NULL);
	void UpdateClientView();
	void ToolScrollToPoint(const Vector2D &ptClient);
	void UpdateStatusBar();
	int  ObjectsAt( const Vector2D &vPoint, HitInfo_t *pObjects, int nMaxObjects, unsigned int nFlags = 0 );
	int  ObjectsAt( CMapWorld *pWorld, const Vector2D &vPoint, HitInfo_t *pObjects, int nMaxObjects, unsigned int nFlags = 0 );
	void GetCenterPoint(Vector& pt);
	void OnContextMenu(UINT nFlags, const Vector2D &vPoint);
	
	void EnsureVisible(Vector &vecPos, float flMargin);
	void UpdateTitleWindowPos();
	
	virtual void Render() {}

	void ZoomIn(BOOL bAllViews = FALSE);
	void ZoomOut(BOOL bAllViews = FALSE);

	//
	// Coordinate transformation functions.
	//
#ifdef HAMMER2013_PORT_KEYBINDS_X
	void InitializeKeyMap(void);
#endif
	void ProcessInput() {}
	void RenderView();
	void ActivateView(bool bActivate);
	void UpdateView(int nFlags);
	CView *GetViewWnd() { return (CView*)this; }
	CMapDoc *GetMapDoc() { return (CMapDoc*)m_pDocument; }

	void WorldToClient(Vector2D &vecClient, const Vector &vecWorld);
	void ClientToWorld(Vector &vecWorld, const Vector2D &ptClient);
	void BuildRay( const Vector2D &ptClient, Vector& vStart, Vector& vEnd );
	void GetBestTransformPlane( Vector &horzAxis, Vector &vertAxis, Vector &thirdAxis);
			
	const Vector &GetViewAxis();
	bool IsInClientView( const Vector &vecMin, const Vector &vecMax );
	bool IsInClientView( const Vector2D &vecMin, const Vector2D &vecMax );

	bool CheckDistance(const Vector2D &vecCheck, const Vector2D &vecRef, int nDist);
	bool IsBoxFullyVisible(const Vector &vecMins, const Vector &vecMaxs);
	bool CanBoxFitInView(const Vector &minsWorld, const Vector &maxsWorld);
	bool PointInClientRect( const Vector2D &point );
	bool HitTest( const Vector2D &vPoint, const Vector& mins, const Vector& maxs );
	
// Implementation
protected:
	CMapView2DBase();           // protected constructor used by dynamic creation
	virtual ~CMapView2DBase();
	DECLARE_DYNCREATE(CMapView2DBase)

	// Derived classes must implement these
	virtual bool IsLogical() { return false; }
	virtual void OnRenderListDirty() {}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapView2DBase)
	protected:
	virtual void OnInitialUpdate();     // first time after construct
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	virtual void OnDraw(CDC *) {};
	//}}AFX_VIRTUAL

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void DrawGridLogical( CRender2D *pRender );
	void DrawGrid( CRender2D *pRender, int xAxis, int yAxis, float depth, bool bNoSmallGrid = false );
#ifdef SLE_2D_BACKGROUNDS
	void DrawBackgroundImage(CRender2D *pRender, CString &filename, int bl_x = 0, int bl_y = 0, int tr_x = 0, int tr_y = 0, int opacity = 0); //// SLE NEW - background images
#endif
	CRender2D *GetRender();
	CTitleWnd *GetTitleWnd();
	bool HasTitleWnd() const;

	// Create a title window.
	void CreateTitleWindow(void);

protected:
	// timer IDs:
	enum 
	{ 
		TIMER_SCROLLVIEW = 1, 
	};

	void DrawPointFile( CRender2D *pRender );
	bool HighlightGridLine( CRender2D *pRender, int nGridLine );

	POINT m_ptLDownClient;		// client pos at which lbutton was pressed, for dragging the view

	// TODO zoom  & forward are all camera properties, remove here

	CRender2D *m_pRender;		// Performs the 3D rendering in our window.

	float	m_flMinZoom;		// Minimum legal zoom factor (should be sufficient to display entire map in the view)

	// these vars are used often, so keep values. they just mirror Camera values
	Vector  m_vViewOrigin;
	float	m_fZoom;			// zoom factor (* map units)
	float	m_fClientWidthHalf;	
	float	m_fClientHeightHalf;
	Vector  m_vViewAxis;		// view axis, normal

	Vector	m_ViewMin;			// client view in world coordinates, same as 3D view frustum
	Vector	m_ViewMax;

	int		m_ClientWidth;
	int		m_ClientHeight;
	
	int		m_xScroll, m_yScroll;	// amount to scroll on timer
	
	bool	m_bToolShown;			// is tool currently visible?
	CTitleWnd *m_pwndTitle;			// title window

	Color	m_clrGrid;				// standard grid color
	Color	m_clrGrid1024;			// 1024 unit line color
#ifdef SLE //// for highlighting map bounds - SLE allows moving objects outside them
	Color	m_clrGrid16384;			// 16384 unit line color
#endif
	Color	m_clrGridCustom;		// custom unit color
	Color	m_clrGridDot;			// grid dot color
	Color	m_clrAxis;				// grid axis color 

	//
	// Color scheme functions.
	//
	void AdjustColorIntensity(Color &color, int nIntensity);
	void SetColorMode(bool bBlackOnWhite);
		
	// mouse drag (space + leftbutton):
	bool	m_bMouseDrag;	// status indicator

	// Generated message map functions
	//{{AFX_MSG(CMapView2DBase)
#ifdef SLE //// SLE NEW - pan the 2d view with MMB
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
#endif
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditProperties();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnUpdateEditFunction(CCmdUI *pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
#ifdef HAMMER2013_PORT_KEYBINDS_X
private:
	CKeyboard m_Keyboard;	// Handles binding of keys and mouse buttons to logical functions.
#endif
};

//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
inline bool CMapView2DBase::PointInClientRect( const Vector2D &point )
{
	return ( point.x >= 0 && point.y >= 0 && point.x < m_ClientWidth && point.y < m_ClientHeight );
}

inline float CMapView2DBase::GetZoom() 
{ 
	return m_fZoom; 
}

inline CRender2D *CMapView2DBase::GetRender()
{
	return m_pRender;
}

inline CTitleWnd* CMapView2DBase::GetTitleWnd()
{
	return m_pwndTitle;
}

inline bool CMapView2DBase::HasTitleWnd() const
{
	return m_pwndTitle != NULL;
}

#endif // MAPVIEW2DBASE_H