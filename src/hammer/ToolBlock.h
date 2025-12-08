//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TOOLBLOCK_H
#define TOOLBLOCK_H
#ifdef _WIN32
#pragma once
#endif

#include "Box3D.h"
#include "ToolInterface.h"

class CToolBlockMessageWnd;

class CToolBlock : public Box3D
{

	friend class CToolBlockMessageWnd;

public:

	CToolBlock();
	~CToolBlock();

	//
	// CBaseTool implementation.
	//
	virtual ToolID_t GetToolID(void) { return TOOL_BLOCK; }

	virtual bool OnContextMenu2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint);
#ifdef SLE //// SLE NEW - make context available in 3d to be able to create brushes without 2d views or kb
	virtual bool OnRMouseUp3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnContextMenu3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint);

	//// SLE NEW - allow nudging block tool box with arrow keys
	void NudgeBlock(CMapView *pView, int nChar, bool bSnap);
#endif
	virtual bool OnKeyDown2D(CMapView2D *pView, UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual bool OnLMouseDown2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnLMouseDown3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnLMouseUp2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnMouseMove2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnMouseMove3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint);
	virtual bool OnLMouseUp3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint);

	virtual bool OnKeyDown3D(CMapView3D *pView, UINT nChar, UINT nRepCnt, UINT nFlags);
#ifdef SLE //// SLE NEW - preview tool block in 2d/3d
	virtual void RenderTool2D(CRender2D *pRender);
	virtual void RenderTool3D(CRender3D *pRender);
	
	void PreviewBlockOrCylinder(CRender2D *pRender2D, CRender3D *pRender3D, bool isBlock);
	void PreviewWedge(CRender2D *pRender2D, CRender3D *pRender3D);
	void PreviewPyramid(CRender2D *pRender2D, CRender3D *pRender3D);
	void PreviewSphere(CRender2D *pRender2D, CRender3D *pRender3D);
	void PreviewArch(CRender2D *pRender2D, CRender3D *pRender3D);
	void PreviewTorus(CRender2D *pRender2D, CRender3D *pRender3D);
#endif
#ifdef SLE /// more stock solids
	void PreviewBipyramid(CRender2D *pRender2D, CRender3D *pRender3D);
	void PreviewIcosahedron(CRender2D *pRender2D, CRender3D *pRender3D);
	void PreviewElongatedBipyramid(CRender2D *pRender2D, CRender3D *pRender3D, bool isGyro);
	void PreviewGyroElongatedBipyramid(CRender2D *pRender2D, CRender3D *pRender3D) {};
	void PreviewAntiprism(CRender2D *pRender2D, CRender3D *pRender3D);
#endif
private:
	void CreateMapObject(CMapView *pView);
	void OnEscape();

	void SetBlockCursor();
};

#endif // TOOLBLOCK_H
