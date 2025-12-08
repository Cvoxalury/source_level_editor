//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CLIPPER3D_H
#define CLIPPER3D_H
#ifdef _WIN32
#pragma once
#endif

#include "MapClass.h"		// For CMapObjectList
#include "Tool3D.h"
#include "ToolInterface.h"
#include "Render2D.h"
#include "MapFace.h"

class CMapSolid;

//=============================================================================
//
// CClipGroup
//

class CClipGroup
{
public:    
    enum { FRONT = 0, BACK };
    
    inline CClipGroup();
    ~CClipGroup();

    inline void SetOrigSolid( CMapSolid *pSolid );
    inline CMapSolid *GetOrigSolid( void );

    inline void SetClipSolid( CMapSolid *pSolid, int side );
    inline CMapSolid *GetClipSolid( int side );

private:
    CMapSolid   *m_pOrigSolid;
    CMapSolid   *m_pClipSolids[2];      // front, back
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline CClipGroup::CClipGroup()
{
    m_pOrigSolid = NULL;
    m_pClipSolids[0] = NULL;
    m_pClipSolids[1] = NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void CClipGroup::SetOrigSolid( CMapSolid *pSolid )
{
    m_pOrigSolid = pSolid;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline CMapSolid *CClipGroup::GetOrigSolid( void )
{
    return m_pOrigSolid;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void CClipGroup::SetClipSolid( CMapSolid *pSolid, int side )
{
    m_pClipSolids[side] = pSolid;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline CMapSolid *CClipGroup::GetClipSolid( int side )
{
    return m_pClipSolids[side];
}

class Clipper3D : public Tool3D
{
    friend BOOL AddToClipList( CMapSolid *pSolid, Clipper3D *pClipper );

public:
    enum { FRONT = 0, BACK, BOTH };
    
    Clipper3D();
	~Clipper3D();

    void IterateClipMode( void );

	inline void ToggleMeasurements( void );

#ifdef SLE /// SLE NEW - WIP 3-point clipping
	// this will control the code for the 3-point version of the clipper
	virtual bool Is3PointClipper(void) {return false;}
#endif
    //
    // Tool3D implementation.
    //
	virtual int HitTest( CMapView *pView, const Vector2D &vPoint, bool bTestHandles = false );
	virtual unsigned int GetConstraints(unsigned int nKeyFlags);

    //
    // CBaseTool implementation.
    //
	virtual void OnActivate();
	virtual void OnDeactivate();
	virtual ToolID_t GetToolID(void) { return TOOL_CLIPPER; }

	virtual void RenderTool2D(CRender2D *pRender);
	virtual void RenderTool3D(CRender3D *pRender);

	virtual bool OnKeyDown2D(CMapView2D *pView, UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual bool OnKeyDown3D(CMapView3D *pView, UINT nChar, UINT nRepCnt, UINT nFlags);

	virtual bool OnLMouseDown2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnLMouseUp2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnMouseMove2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint);
#ifdef SLE //// SLE NEW - add 3d operation for the clipper, both regular and 3-point	
	virtual bool OnLMouseUp3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnLMouseDown3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnMouseMove3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint);

	//// add context menu
	virtual bool OnRMouseUp3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint);
	bool OnContextMenu2D( CMapView2D *pView, UINT nFlags, const Vector2D &vPoint );
	bool OnContextMenu3D( CMapView3D *pView, UINT nFlags, const Vector2D &vPoint );
#endif
protected:
    //
	// Tool3D implementation.
	//
	virtual bool UpdateTranslation( const Vector &vUpdate, UINT uFlags = 0 );
	virtual void FinishTranslation( bool bSave );
	
#ifdef SLE //// needs to be public for the context menu
public:
	void OnEscape(void);
    void SaveClipResults( void );
#else
private:
	void OnEscape(void);
    void SaveClipResults( void );
#endif
	void SetClipObjects( const CMapObjectList *pList );
    void SetClipPlane( PLANE *pPlane );
	void BuildClipPlane( void );
    void GetClipResults( void );
    void CalcClipResults( void );
    void ResetClipResults( void );

	void RemoveOrigSolid( CMapSolid *pOrigSolid );
	void SaveClipSolid( CMapSolid *pSolid, CMapSolid *pOrigSolid );

    void DrawBrushExtents(CRender2D *pRender, CMapSolid *pSolid, int nFlags);

    int             m_Mode;                 // current clipping mode { back, front, both }
    
    PLANE           m_ClipPlane;            // the clipping plane -- front/back is uneccesary
#ifdef SLE /// SLE NEW - WIP 3-point clipping
	Vector          m_ClipPoints[3];        // 2D clipping points -- used to create the clip plane

	int				m_leftClickCounter; // used to sequentially place the clipping points in 3d
#else
    Vector          m_ClipPoints[2];        // 2D clipping points -- used to create the clip plane
#endif
    int             m_ClipPointHit;         // the clipping that was "hit" {0, 1, -1}
	Vector			m_vOrgPos;

    const CMapObjectList  *m_pOrigObjects;        // list of the initial objects to clip
    CUtlVector<CClipGroup*> m_ClipResults;          // list of clipped objects

	bool            m_bDrawMeasurements;	// Whether to draw brush dimensions in the 2D view.

	CRender2D		m_Render2D;				// 2d renderer
};

//-----------------------------------------------------------------------------
// Purpose: Toggles the clipper's rendering of brush measurements in the 2D view.
//-----------------------------------------------------------------------------
inline void Clipper3D::ToggleMeasurements( void )
{
	m_bDrawMeasurements = !m_bDrawMeasurements;
	m_pDocument->UpdateAllViews( MAPVIEW_UPDATE_TOOL );
}

#endif // CLIPPER3D_H
