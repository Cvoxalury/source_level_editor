//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
////// SLE TODO: add switching to other types in context menu
// 4) using arrow keys to move the creation box 5) preview of objects that'll be created before creating them// 
// 7) set initial rotation of cylinders or spikes 8) type out the block type above the box in 2d
// $NoKeywords: $
//=============================================================================//

#include "stdafx.h"
#include "History.h"
#include "MainFrm.h"
#include "MapDefs.h"
#include "MapDoc.h"
#include "MapView2D.h"
#include "MapView3D.h"
#include "Options.h"
#include "StatusBarIDs.h"
#include "ToolBlock.h"
#include "ToolManager.h"
#include "vgui/Cursor.h"
#include "Selection.h"
#ifdef SLE //// SLE NEW - preview tool block in 2d/3d
#include "Render2D.h"
#include "archdlg.h"
#endif

class CToolBlockMessageWnd : public CWnd
{
	public:
		bool Create(void);
		void PreMenu2D(CToolBlock *pTool, CMapView2D *pView);
#ifdef SLE //// SLE NEW - make context available in 3d to be able to create brushes without 2d views or kb
		void PreMenu3D(CToolBlock *pTool, CMapView3D *pView);
#endif
	protected:
		//{{AFX_MSG_MAP(CToolBlockMessageWnd)
		afx_msg void OnCreateObject();
		//}}AFX_MSG
	
		DECLARE_MESSAGE_MAP()

	private:
		CToolBlock *m_pToolBlock;
		CMapView2D *m_pView2D;
#ifdef SLE //// SLE NEW - make context available in 3d to be able to create brushes without 2d views or kb
		CMapView3D *m_pView3D;
#endif
};

static CToolBlockMessageWnd s_wndToolMessage;
static const char *g_pszClassName = "ValveEditor_BlockToolWnd";

BEGIN_MESSAGE_MAP(CToolBlockMessageWnd, CWnd)
	//{{AFX_MSG_MAP(CToolMessageWnd)
	ON_COMMAND(ID_CREATEOBJECT, OnCreateObject)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// Purpose: Creates the hidden window that receives context menu commands for the
//			block tool.
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CToolBlockMessageWnd::Create(void)
{
	WNDCLASS wndcls;
	memset(&wndcls, 0, sizeof(WNDCLASS));
    wndcls.lpfnWndProc   = AfxWndProc;
    wndcls.hInstance     = AfxGetInstanceHandle();
    wndcls.lpszClassName = g_pszClassName;

	if (!AfxRegisterClass(&wndcls))
	{
		return(false);
	}

	return(CWnd::CreateEx(0, g_pszClassName, g_pszClassName, 0, CRect(0, 0, 10, 10), NULL, 0) == TRUE);
}

//-----------------------------------------------------------------------------
// Purpose: Attaches the block tool to this window before activating the context
//			menu.
//-----------------------------------------------------------------------------
void CToolBlockMessageWnd::PreMenu2D(CToolBlock *pToolBlock, CMapView2D *pView)
{
	Assert(pToolBlock != NULL);
	m_pToolBlock = pToolBlock;
	m_pView2D = pView;
}
#ifdef SLE //// SLE NEW - make context available in 3d to be able to create brushes without 2d views or kb
void CToolBlockMessageWnd::PreMenu3D(CToolBlock *pToolBlock, CMapView3D *pView)
{
	Assert(pToolBlock != NULL);
	m_pToolBlock = pToolBlock;
	m_pView3D = pView;
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CToolBlockMessageWnd::OnCreateObject()
{
#ifdef SLE //// SLE NEW - make context available in 3d to be able to create brushes without 2d views or kb
	CMapView *view;
	if ( m_pView3D == NULL ) view = m_pView2D;
	else view = m_pView3D;
	m_pToolBlock->CreateMapObject(view);
#else
	m_pToolBlock->CreateMapObject(m_pView3D);
#endif
}
//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CToolBlock::CToolBlock(void)
{
	// We always show our dimensions when we render.
	SetDrawFlags(GetDrawFlags() | Box3D::boundstext);
	SetDrawColors(Options.colors.clrToolHandle, Options.colors.clrToolBlock);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CToolBlock::~CToolBlock(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Handles key down events in the 2D view.
// Input  : Per CWnd::OnKeyDown.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool CToolBlock::OnKeyDown2D(CMapView2D *pView, UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
		case VK_RETURN:
		{
			if ( !IsEmpty() )
			{
				CreateMapObject(pView);
			}

			return true;
		}

		case VK_ESCAPE:
		{
			OnEscape();
			return true;
		}
#ifdef SLE //// SLE NEW - allow nudging block tool box with arrow keys
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
		{			
			bool bSnap = m_pDocument->IsSnapEnabled() && (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			NudgeBlock(pView, nChar, !bSnap);
			return true;
		}
#endif
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Handles key down events in the 3D view.
// Input  : Per CWnd::OnKeyDown.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool CToolBlock::OnKeyDown3D(CMapView3D *pView, UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch ( nChar )
	{
		case VK_RETURN:
		{
			if ( !IsEmpty() )
			{
				CreateMapObject(pView);
			}

			return true;
		}

		case VK_ESCAPE:
		{
			OnEscape();
			return true;
		}
#ifdef SLE //// SLE NEW - allow nudging block tool box with arrow keys
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
		{			
			bool bSnap = m_pDocument->IsSnapEnabled() && (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			NudgeBlock(pView, nChar, !bSnap);
			return true;
		}
#endif
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Handles the escape key in the 2D or 3D views.
//-----------------------------------------------------------------------------
void CToolBlock::OnEscape(void)
{
	//
	// If we have nothing blocked, go back to the pointer tool.
	//
	if ( IsEmpty() )
	{
		ToolManager()->SetTool(TOOL_POINTER);
	}
	//
	// We're blocking out a region, so clear it.
	//
	else
	{
		SetEmpty();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles context menu events in the 2D view.
// Input  : Per CWnd::OnContextMenu.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool CToolBlock::OnContextMenu2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint) 
{
	static CMenu menu, menuCreate;
	static bool bInit = false;

	if (!bInit)
	{
		bInit = true;

		// Create the menu.
		menu.LoadMenu(IDR_POPUPS);
		menuCreate.Attach(::GetSubMenu(menu.m_hMenu, 1));

		// Create the window that handles menu messages.
		s_wndToolMessage.Create();
	}

	if ( !pView->PointInClientRect(vPoint) )
	{
		return false;
	}

	if ( !IsEmpty() )
	{
		if ( HitTest(pView, vPoint, false) )
		{
			CPoint ptScreen( vPoint.x,vPoint.y);
			pView->ClientToScreen(&ptScreen);

			s_wndToolMessage.PreMenu2D(this, pView);
			menuCreate.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN, ptScreen.x, ptScreen.y, &s_wndToolMessage);
		}
	}

	return true;
}
#ifdef SLE //// SLE NEW - make context available in 3d to be able to create brushes without 2d views or kb
bool CToolBlock::OnRMouseUp3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint)
{
	Tool3D::OnRMouseUp3D(pView, nFlags, vPoint);
	OnContextMenu3D(pView, nFlags, vPoint);
	return true;
}
bool CToolBlock::OnContextMenu3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint)
{
	static CMenu menu, menuCreate;
	static bool bInit = false;

	if ( !bInit )
	{
		bInit = true;

		// Create the menu.
		menu.LoadMenu(IDR_POPUPS);
		menuCreate.Attach(::GetSubMenu(menu.m_hMenu, 1));

		// Create the window that handles menu messages.
		s_wndToolMessage.Create();
	}

	if ( !IsEmpty() )
	{
		if ( HitTest(pView, vPoint, false) )
		{
			CPoint ptScreen(vPoint.x, vPoint.y);
			pView->ClientToScreen(&ptScreen);

			s_wndToolMessage.PreMenu3D(this, pView);
			menuCreate.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN, ptScreen.x, ptScreen.y, &s_wndToolMessage);
			return true;
		}
	}

	return true;
}
#endif
//-----------------------------------------------------------------------------
// Purpose: Handles left mouse button down events in the 2D view.
// Input  : Per CWnd::OnLButtonDown.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool CToolBlock::OnLMouseDown3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint) 
{
	Tool3D::OnLMouseDown3D(pView, nFlags, vPoint);

	//
	// If we have a box, see if they clicked on any part of it.
	//
	if ( !IsEmpty() )
	{
		if ( HitTest( pView, vPoint, true ) )
		{
			// just update current block
			StartTranslation( pView, vPoint, m_LastHitTestHandle );
			return true;
		}
	}
#ifdef SLE //// SLE new - can create new blocks in 3d view
	else
	{
		Vector vecStart;
		pView->ClientToWorld(vecStart, m_vMouseStart[ MOUSE_LEFT ]);

		// Snap it to the grid.
		m_pDocument->Snap(vecStart, constrainIntSnap);

		// Start the new box with the extents of the last selected thing.

		m_pDocument->GetSelection()->GetLastValidBounds(bmins, bmaxs);

		StartNew(pView, vPoint, vecStart - Vector(32, 32, 32), Vector(64, 64, 64));
	}
#endif
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handles left mouse button down events in the 2D view.
// Input  : Per CWnd::OnLButtonDown.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool CToolBlock::OnLMouseDown2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint) 
{
	Tool3D::OnLMouseDown2D(pView, nFlags, vPoint);

	// If we have a box, see if they clicked on any part of it.
	if ( !IsEmpty() )
	{
		if ( HitTest( pView, vPoint, true ) )
		{
			// just update current block
            StartTranslation( pView, vPoint, m_LastHitTestHandle );
			return true;
		}
	}
		
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handles left mouse button up events in the 2D view.
// Input  : Per CWnd::OnLButtonUp.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool CToolBlock::OnLMouseUp2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint)
{
	Tool3D::OnLMouseUp2D(pView, nFlags, vPoint);

	if (IsTranslating())
	{
		FinishTranslation(true);
	}

	m_pDocument->UpdateStatusbar();
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handles left mouse button up events in the 2D view.
// Input  : Per CWnd::OnLButtonUp.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool CToolBlock::OnLMouseUp3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint)
{
	Tool3D::OnLMouseUp3D(pView, nFlags, vPoint);

	if (IsTranslating())
	{
		FinishTranslation(true);
	}

	m_pDocument->UpdateStatusbar();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handles mouse move events in the 2D view.
// Input  : Per CWnd::OnMouseMove.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool CToolBlock::OnMouseMove2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint) 
{
	vgui::HCursor hCursor = vgui::dc_arrow;

	// Snap it to the grid.
	unsigned int uConstraints = GetConstraints( nFlags );

	Tool3D::OnMouseMove2D(pView, nFlags, vPoint);
							    
	//
	//
	// Convert to world coords.
	//
	Vector vecWorld;
	pView->ClientToWorld(vecWorld, vPoint);
	
	//
	// Update status bar position display.
	//
	char szBuf[128];

	if ( uConstraints & constrainSnap )
		m_pDocument->Snap(vecWorld,uConstraints);

	sprintf(szBuf, " @%.0f, %.0f ", vecWorld[pView->axHorz], vecWorld[pView->axVert]);
	SetStatusText(SBI_COORDS, szBuf);
	
	if ( IsTranslating() )
	{
		Tool3D::UpdateTranslation( pView, vPoint, uConstraints);

		hCursor =  vgui::dc_none;
	}
	else if ( m_bMouseDragged[MOUSE_LEFT] )
	{
		// Starting a new box. Build a starting point for the drag.
#ifdef SLE //// SLE FIX: apparently the old method doesn't work anymore. The new method is copied from ToolDecal.cpp.
		static HCURSOR hcurBlock;
	
		if (!hcurBlock)
		{
			hcurBlock = LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_NEWBOX));
		}

		SetCursor(hcurBlock);
#else
		pView->SetCursor( "Resource/block.cur" );
#endif
		Vector vecStart;
		pView->ClientToWorld(vecStart, m_vMouseStart[MOUSE_LEFT] );
		
		// Snap it to the grid.
		if ( uConstraints & constrainSnap )
			m_pDocument->Snap( vecStart, uConstraints);

		// Start the new box with the extents of the last selected thing.

		m_pDocument->GetSelection()->GetLastValidBounds(bmins, bmaxs);

		vecStart[pView->axThird] = bmins[pView->axThird];

		StartNew(pView, vPoint, vecStart, pView->GetViewAxis() * (bmaxs-bmins) );
	}
	else if ( !IsEmpty() )
	{
		// If the cursor is on a handle, set it to a cross.
		if ( HitTest(pView, vPoint, true) )
		{
			hCursor = UpdateCursor( pView, m_LastHitTestHandle, m_TranslateMode );
		}
	}	

	if ( hCursor != vgui::dc_none )
		pView->SetCursor( hCursor );

	return true;
}

bool CToolBlock::OnMouseMove3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint)
{
	Tool3D::OnMouseMove3D(pView, nFlags, vPoint);

	if ( IsTranslating() )
	{
		unsigned int uConstraints = GetConstraints( nFlags );

		// If they are dragging with a valid handle, update the views.
		Tool3D::UpdateTranslation( pView, vPoint, uConstraints );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Creates a solid in the given view.
//-----------------------------------------------------------------------------
void CToolBlock::CreateMapObject(CMapView *pView)
{
	CMapWorld *pWorld = m_pDocument->GetMapWorld();

	if (!(bmaxs[0] - bmins[0]) || !(bmaxs[1] - bmins[1]) || !(bmaxs[2] - bmins[2]))
	{
		AfxMessageBox("The box is empty.");
		SetEmpty();
		return;
	}

	BoundBox NewBox = *this;
	if (Options.view2d.bOrientPrimitives)
	{
		switch (pView->GetDrawType())
		{
			case VIEW2D_XY:
			{
				break;
			}

			case VIEW2D_YZ:
			{
				NewBox.Rotate90(AXIS_Y);
				break;
			}

			case VIEW2D_XZ:
			{
				NewBox.Rotate90(AXIS_X);
				break;
			}
		}
	}

	// Create the object within the given bounding box.
	CMapClass *pObject = GetMainWnd()->m_ObjectBar.CreateInBox(&NewBox, pView);
	if (pObject == NULL)
	{
		SetEmpty();
		return;
	}

	m_pDocument->ExpandObjectKeywords(pObject, pWorld);

	GetHistory()->MarkUndoPosition(m_pDocument->GetSelection()->GetList(), "New Object");

	m_pDocument->AddObjectToWorld(pObject);

	//
	// Do we need to rotate this object based on the view we created it in?
	//
	if (Options.view2d.bOrientPrimitives)
	{
		Vector center;
		pObject->GetBoundsCenter( center );

		QAngle angles(0, 0, 0);
		switch (pView->GetDrawType())
		{
			case VIEW2D_XY:
			{
				break;
			}
			
			case VIEW2D_YZ:
			{
 				angles[1] = 90.f;
				pObject->TransRotate(center, angles);
				break;
			}

			case VIEW2D_XZ:
			{
				angles[0] = 90.f;
				pObject->TransRotate(center, angles);
				break;
			}
		}
	}

	GetHistory()->KeepNew(pObject);

	// Select the new object.
	m_pDocument->SelectObject(pObject, scClear|scSelect|scSaveChanges);

	m_pDocument->SetModifiedFlag();

	SetEmpty();
	ResetBounds();
}
#ifdef SLE //// SLE NEW - preview tool block in 2d/3d
void CToolBlock::RenderTool2D(CRender2D *pRender)
{
	Box3D::RenderTool2D(pRender);
	
	if ( GetMainWnd()->m_ObjectBar.IsBlockToolCreatingPrimitive() )
	{
		switch ( GetMainWnd()->m_ObjectBar.GetBlockToolActivePrimitive() )
		{
			case 0: // block
			{
				PreviewBlockOrCylinder(pRender, NULL, true);
				break;
			}
			case 1: // wedge
			{
				PreviewWedge(pRender, NULL);
				break;
			}
			case 2: // cylinder
			{
				PreviewBlockOrCylinder(pRender, NULL, false);
				break;
			}
			case 3: // pyramid
			{
				PreviewPyramid(pRender, NULL);
				break;
			}
			case 4: // sphere
			{
				PreviewSphere(pRender, NULL);
				break;
			}
			case 5: // arch
			{
				PreviewArch(pRender, NULL);
				break;
			}
			case 6: // torus
			{
				PreviewTorus(pRender, NULL);
				break;
			}

			default:
				break;
		}
	}
	
#ifdef SLE /// more stock solids
	if ( GetMainWnd()->m_ObjectBar.IsBlockToolCreatingExtendedPrimitive() )
	{
		switch ( GetMainWnd()->m_ObjectBar.GetBlockToolActivePrimitive() )
		{
			case 0: // bypiramid
			{
				PreviewBipyramid(pRender, NULL);
				break;
			}
			case 1: // icosahedron
			{
				PreviewIcosahedron(pRender, NULL);
				break;
			}
			case 2: // elongated bypiramid
			{
				PreviewElongatedBipyramid(pRender, NULL, false);
				break;
			}
			case 3: // gyroelongated bypiramid
			{
				PreviewElongatedBipyramid(pRender, NULL, true);
				break;
			}
			case 4: // antiprism
			{
				PreviewAntiprism(pRender, NULL);
				break;
			}

			default:
				break;
		}
	}
#endif
}

void CToolBlock::RenderTool3D(CRender3D *pRender)
{
	Box3D::RenderTool3D(pRender);

	Color wireColor;
	pRender->GetDrawColor(wireColor);
	pRender->PushRenderMode(RENDER_MODE_WIREFRAME); // todo: use noZ? or is this more useful when aligning in the world?
	pRender->SetDrawColor(255, 255, 255);

	if ( GetMainWnd()->m_ObjectBar.IsBlockToolCreatingPrimitive() )
	{
		switch ( GetMainWnd()->m_ObjectBar.GetBlockToolActivePrimitive() )
		{
			case 0: // block
			{
				PreviewBlockOrCylinder(NULL, pRender, true);
				break;
			}
			case 1: // wedge
			{
				PreviewWedge(NULL, pRender);
				break;
			}
			case 2: // cylinder
			{
				PreviewBlockOrCylinder(NULL, pRender, false);
				break;
			}
			case 3: // pyramid
			{
				PreviewPyramid(NULL, pRender);
				break;
			}
			case 4: // sphere
			{
				PreviewSphere(NULL, pRender);
				break;
			}
			case 5: // arch
			{
				PreviewArch(NULL, pRender);
				break;
			}
			case 6: // torus
			{
				PreviewTorus(NULL, pRender);
				break;
			}

			default:
				break;
		}
	}
	
#ifdef SLE /// more stock solids
	if ( GetMainWnd()->m_ObjectBar.IsBlockToolCreatingExtendedPrimitive() )
	{
		switch ( GetMainWnd()->m_ObjectBar.GetBlockToolActivePrimitive() )
		{
			case 0: // bypiramid
			{
				PreviewBipyramid(NULL, pRender);
				break;
			}
			case 1: // icosahedron
			{
				PreviewIcosahedron(NULL, pRender);
				break;
			}
			case 2: // elongated bypiramid
			{
				PreviewElongatedBipyramid(NULL, pRender, false);
				break;
			}
			case 3: // gyroelongated bypiramid
			{
				PreviewElongatedBipyramid(NULL, pRender, true);
				break;
			}
			case 4: // antiprism
			{
				PreviewAntiprism(NULL, pRender);
				break;
			}

			default:
				break;
		}
	}
#endif

	pRender->SetDrawColor(wireColor);
	pRender->PopRenderMode();
}

void MakePreviewArcCenterRadius(float xCenter, float yCenter, float xrad, float yrad, 
	int npoints, float start_ang, float fArc, Vector points[])
{
    int point;
    float angle = start_ang;
	float angle_delta;

	angle_delta = fArc / (float)npoints;

	// Add an additional points if we are not doing a full circle
	if (fArc != 360.0)
	{
		++npoints;
	}
	
    for( point = 0; point < npoints; point++ )
    {
        if ( angle > 360 )
		{
           angle -= 360;
		}

        points[point][0] = (xCenter + (float)cos(DEG2RAD(angle)) * xrad);
        points[point][1] = (yCenter + (float)sin(DEG2RAD(angle)) * yrad);

		angle += angle_delta;
    }

	// Full circle, recopy the first point as the closing point.
	if (fArc == 360.0)
	{
	    points[point][0] = points[0][0];
		points[point][1] = points[0][1];
	}
}

void MakePreviewArc(float x1, float y1, float x2, float y2, int npoints, float start_ang, float fArc, Vector points[])
{
    float xrad = (x2 - x1) / 2.0f;
	float yrad = (y2 - y1) / 2.0f;

	// make centerpoint for polygon:
    float xCenter = x1 + xrad;
    float yCenter = y1 + yrad;

	MakePreviewArcCenterRadius( xCenter, yCenter, xrad, yrad, npoints, start_ang, fArc, points );
}

void CToolBlock::PreviewBlockOrCylinder(CRender2D *pRender2D, CRender3D *pRender3D, bool isBlock)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;

	if( do3D && isBlock) return; // no need to preview it in 3d, it simply matches the bound box
		
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = isBlock ? 4 : GetMainWnd()->m_ObjectBar.GetBlockToolFaceCount();
	float startAngle = GetMainWnd()->m_ObjectBar.GetBlockToolStartAngle();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	Vector origin;
	GetBoundsCenter(origin);

	Vector pmPointsTop[ 65 ]; // (65 is max cylinder sides (64) + 1)
	Vector pmPointsBot[ 65 ]; // (65 is max cylinder sides (64) + 1)
	polyMake(origin[ 0 ] - fWidth, origin[ 1 ] - fDepth, origin[ 0 ] + fWidth, origin[ 1 ] + fDepth, nSides, startAngle, pmPointsTop);
		
	if ( isBlock )
	{
		pmPointsTop[ 0 ][ 0 ] = bmins[ 0 ];
		pmPointsTop[ 0 ][ 1 ] = bmaxs[ 1 ];
		pmPointsTop[ 0 ][ 2 ] = bmaxs[ 2 ];

		pmPointsTop[ 1 ][ 0 ] = bmaxs[ 0 ];
		pmPointsTop[ 1 ][ 1 ] = bmaxs[ 1 ];
		pmPointsTop[ 1 ][ 2 ] = bmaxs[ 2 ];

		pmPointsTop[ 2 ][ 0 ] = bmaxs[ 0 ];
		pmPointsTop[ 2 ][ 1 ] = bmins[ 1 ];
		pmPointsTop[ 2 ][ 2 ] = bmaxs[ 2 ];

		pmPointsTop[ 3 ][ 0 ] = bmins[ 0 ];
		pmPointsTop[ 3 ][ 1 ] = bmins[ 1 ];
		pmPointsTop[ 3 ][ 2 ] = bmaxs[ 2 ];
	}

	// face 0 - top face
	for ( int i = 0; i < nSides + 1; i++ )
	{
		pmPointsTop[ i ][ 2 ] = origin[ 2 ] + fHeight;
	}

	if ( do2D ) // todo: for block, render filled lines in every 2d view?
	{
		pRender2D->DrawPolyLineFilled(nSides, pmPointsTop, 0.125f);
		pRender2D->DrawPolyLine(nSides, pmPointsTop); // regular line to frame the filled poly
	}
	if( do3D)
		pRender3D->DrawPolyLine(nSides, pmPointsTop);

	// face 1 - bottom face
	for ( int i = 0; i < nSides + 1; i++ )
	{
		pmPointsBot[ i ][ 0 ] = pmPointsTop[ i ][ 0 ];
		pmPointsBot[ i ][ 1 ] = pmPointsTop[ i ][ 1 ];
		pmPointsBot[ i ][ 2 ] = origin[ 2 ] - fHeight;
	}
	
	if( do2D)
		pRender2D->DrawPolyLine(nSides, pmPointsBot); // in 2d they are the same looking, no need to draw filled twice
	if( do3D)
		pRender3D->DrawPolyLine(nSides, pmPointsBot);

	// other edges
	for ( int i = 0; i < nSides; i++ )
	{
		if(do2D)
			pRender2D->DrawLine(pmPointsTop[ i ], pmPointsBot[ i ]);
		if(do3D)
			pRender3D->DrawLine(pmPointsTop[ i ], pmPointsBot[ i ]);
	}
}

void CToolBlock::PreviewPyramid(CRender2D *pRender2D, CRender3D *pRender3D)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;
			
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = GetMainWnd()->m_ObjectBar.GetBlockToolFaceCount();

	float startAngle = GetMainWnd()->m_ObjectBar.GetBlockToolStartAngle();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	Vector origin;
	GetBoundsCenter(origin);

	Vector pmPoints[ 64 ];

	polyMake(origin[ 0 ] - fWidth, origin[ 1 ] - fDepth, origin[ 0 ] + fWidth, origin[ 1 ] + fDepth, nSides, startAngle, pmPoints);
		
	// top point
	Vector top;
	top[0] = origin[0];
	top[1] = origin[1];
	top[2] = origin[2] + fHeight;
			
	// bottom face
	for ( int i = 0; i < nSides; i++ )
	{		
		pmPoints[ i ][ 2 ] = origin[ 2 ] - fHeight;
	}

	if ( do2D ) // todo: for block, render filled lines in every 2d view?
	{
		pRender2D->DrawPolyLineFilled(nSides, pmPoints, 0.125f);
		pRender2D->DrawPolyLine(nSides, pmPoints); // regular line to frame the filled poly
	}
	if( do3D)
		pRender3D->DrawPolyLine(nSides, pmPoints);
		
	// lines connecting top point to bottom face
	for ( int i = 0; i < nSides; i++ )
	{
		if( do2D)
			pRender2D->DrawLine(top, pmPoints[i]);
		if( do3D)
			pRender3D->DrawLine(top, pmPoints[i]);
	}
}

void CToolBlock::PreviewSphere(CRender2D *pRender2D, CRender3D *pRender3D)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;
			
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = GetMainWnd()->m_ObjectBar.GetBlockToolFaceCount();

	float startAngle = GetMainWnd()->m_ObjectBar.GetBlockToolStartAngle();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	Vector origin;
	GetBoundsCenter(origin);

	Vector bottom = origin; bottom[2] -= fHeight;
		
	float fAngle = 0;
	float fAngleStep = 180.0 / nSides;
		
	for (int nSlice = 0; nSlice < nSides; nSlice++)
	{
		Vector lowerPoints[4], upperPoints[4];
		lowerPoints[0] = lowerPoints[1] = lowerPoints[2] = lowerPoints[3] = vec3_invalid;
		upperPoints[0] = upperPoints[1] = upperPoints[2] = upperPoints[3] = vec3_invalid;

		float fAngle1 = fAngle + fAngleStep;
		
		//
		// Make the upper polygon.
		//
		Vector TopPoints[64];

		float fUpperWidth = fWidth * sin(DEG2RAD(fAngle));
		float fUpperDepth = fDepth * sin(DEG2RAD(fAngle));
		polyMake(origin[0] - fUpperWidth, origin[1] - fUpperDepth, 
				 origin[0] + fUpperWidth, origin[1] + fUpperDepth, 
				 nSides, startAngle, TopPoints, false);
		//
		// Make the lower polygon.
		//
		Vector BottomPoints[64];
		float fLowerWidth = fWidth * sin(DEG2RAD(fAngle1));
		float fLowerDepth = fDepth * sin(DEG2RAD(fAngle1));
		polyMake(origin[0] - fLowerWidth, origin[1] - fLowerDepth, 
				 origin[0] + fLowerWidth, origin[1] + fLowerDepth, 
				 nSides, startAngle, BottomPoints, false);
				
		//
		// Build the faces that connect the upper and lower polygons.
		//
		Vector Points[4];
		float fUpperHeight = origin[2] + fHeight * cos(DEG2RAD(fAngle));
		float fLowerHeight = origin[2] + fHeight * cos(DEG2RAD(fAngle1));

		for (int i = 0; i < nSides; i++)
		{
			if (nSlice != 0)
			{
				Points[0][0] = TopPoints[i + 1][0];
				Points[0][1] = TopPoints[i + 1][1];
				Points[0][2] = fUpperHeight;

				upperPoints[0] = Points[0];
			}
			
			Points[1][0] = TopPoints[i][0];
			Points[1][1] = TopPoints[i][1];
			Points[1][2] = fUpperHeight;

			upperPoints[1] = Points[1];
			
			Points[2][0] = BottomPoints[i][0];
			Points[2][1] = BottomPoints[i][1];
			Points[2][2] = fLowerHeight;

			lowerPoints[2] = Points[2];

			if (nSlice != nSides - 1)
			{
				Points[3][0] = BottomPoints[i + 1][0];
				Points[3][1] = BottomPoints[i + 1][1];
				Points[3][2] = fLowerHeight;

				lowerPoints[3] = Points[3];
			}
			
			// Preview the lines	
			// Top and bottom are cones, not rings, so remove one vertex per face.
			//			
			if ( do2D )
			{
				if ( nSlice == 0 )
				{
					pRender2D->DrawPolyLine(3, &Points[1]);
				} 
				else if ( nSlice == nSides - 1 )
				{
					pRender2D->DrawPolyLine(3, Points);
				}
				else
				{
					pRender2D->DrawPolyLine(4, Points);
				}
			}
			if ( do3D )
			{
				if ( nSlice == 0 )
				{
					pRender3D->DrawPolyLine(3, &Points[1]);
				}
				else if ( nSlice == nSides - 1 )
				{
					pRender3D->DrawPolyLine(3, Points);
				}
				else
				{
					pRender3D->DrawPolyLine(4, Points);
				}
			}
		}
	
		fAngle += fAngleStep;
	}
}

void CToolBlock::PreviewArch(CRender2D *pRender2D, CRender3D *pRender3D)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;
				
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = GetMainWnd()->m_ObjectBar.GetBlockToolFaceCount();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	CString str;
	int wallWidth = AfxGetApp()->GetProfileInt("Arch", "Wall Width", 32); // divide because outer/inner arcs are pushed outward/inward by half the width
	str = AfxGetApp()->GetProfileString("Arch", "Arc_", "180");
	float arc = atof(str);
	nSides = AfxGetApp()->GetProfileInt("Arch", "Sides", 8);
	str = AfxGetApp()->GetProfileString("Arch", "Start Angle_", "0");
	float angle = atof(str);
	int addHeight = AfxGetApp()->GetProfileInt("Arch", "Add Height", 0);

	bool stretch = 0; // Options.general.bStretchArches;

	Vector origin;
	GetBoundsCenter(origin);

	// create the top outer edge
	Vector pmPointsOuterTop[ 256 ];

	MakePreviewArc(origin[0] - fWidth, origin[1] - fDepth,
		origin[0] + fWidth, origin[1] + fDepth, nSides,
		angle, arc, pmPointsOuterTop);

	for ( int i = 0; i < nSides + 1; i++ )
	{		
		pmPointsOuterTop[ i ][ 2 ] = origin[ 2 ] + fHeight;
	}

	// create the top inner edge
	Vector pmPointsInnerTop[ 256 ];
	
	//// SLE TODO: passing wall width in here (and pmPointsInnerBot) is imprecise
	// for non 360 arches, as it distorts the boundaries slightly.
	// but for now this'll work well enough...
	MakePreviewArc(origin[0] - fWidth + wallWidth, origin[1] - fDepth + wallWidth,
		origin[0] + fWidth - wallWidth, origin[1] + fDepth - wallWidth, nSides,
		angle, arc, pmPointsInnerTop);

	for ( int i = 0; i < nSides + 1; i++ )
	{		
		pmPointsInnerTop[ i ][ 2 ] = origin[ 2 ] + fHeight;
	}
		
	// create the bottom outer edge
	Vector pmPointsOuterBot[ 256 ];

	MakePreviewArc(origin[0] - fWidth, origin[1] - fDepth,
		origin[0] + fWidth, origin[1] + fDepth, nSides,
		angle, arc, pmPointsOuterBot);

	for ( int i = 0; i < nSides + 1; i++ )
	{		
		pmPointsOuterBot[ i ][ 2 ] = origin[ 2 ] - fHeight;
	}
	
	// create the bottom inner edge
	Vector pmPointsInnerBot[ 256 ];

	MakePreviewArc(origin[0] - fWidth + wallWidth, origin[1] - fDepth + wallWidth,
		origin[0] + fWidth - wallWidth, origin[1] + fDepth - wallWidth, nSides,
		angle, arc, pmPointsInnerBot);
	
	for ( int i = 0; i < nSides + 1; i++ )
	{		
		pmPointsInnerBot[ i ][ 2 ] = origin[ 2 ] - fHeight;
	}

	// stretch if enabled
	if ( stretch )
	{
		Vector boundSize;
		GetBoundsSize(boundSize);
		Vector objSize;
		objSize[0] = origin[0] - fWidth;
		objSize[1] = origin[0] - fDepth;
		objSize[2] = 1.0f;
		
		if ( boundSize[ AXIS_X ] > objSize[ AXIS_X ] ||
			boundSize[ AXIS_Y ] > objSize[ AXIS_Y ] )
		{
			float scaleX, scaleY;
			scaleX = boundSize[ AXIS_X ] / objSize[ AXIS_X ];
			scaleY = boundSize[ AXIS_Y ] / objSize[ AXIS_Y ];

			for ( int i = 0; i < nSides + 1; i++ )
			{
				pmPointsOuterTop[ i ][ 0 ] *= scaleX;
				pmPointsInnerTop[ i ][ 0 ] *= scaleX;
				pmPointsOuterBot[ i ][ 0 ] *= scaleX;
				pmPointsInnerBot[ i ][ 0 ] *= scaleX;
				pmPointsOuterTop[ i ][ 1 ] *= scaleY;
				pmPointsInnerTop[ i ][ 1 ] *= scaleY;
				pmPointsOuterBot[ i ][ 1 ] *= scaleY;
				pmPointsInnerBot[ i ][ 1 ] *= scaleY;
								
				pmPointsOuterTop[ i ][ 1 ] -= fDepth;
				pmPointsInnerTop[ i ][ 1 ] -= fDepth;
				pmPointsOuterBot[ i ][ 1 ] -= fDepth;
				pmPointsInnerBot[ i ][ 1 ] -= fDepth;
			}
		}
	}

	// create preview volumes imitating the future solids
		
	// lines connecting top point to bottom face
	for ( int i = 0; i < nSides; i++ )
	{
		Vector volume[ 8 ];
		volume[ 0 ] = pmPointsOuterTop[ i ];
		volume[ 1 ] = pmPointsInnerTop[ i ];
		volume[ 2 ] = pmPointsInnerTop[ i + 1 ];
		volume[ 3 ] = pmPointsOuterTop[ i + 1 ];

		volume[ 4 ] = pmPointsOuterBot[ i ];
		volume[ 5 ] = pmPointsInnerBot[ i ];
		volume[ 6 ] = pmPointsInnerBot[ i + 1 ];
		volume[ 7 ] = pmPointsOuterBot[ i + 1 ];

		/*if ( i == nSides - 1 )
		{
			volume[ 2 ] = pmPointsInnerTop[ 0 ];
			volume[ 3 ] = pmPointsOuterTop[ 0 ];
			volume[ 6 ] = pmPointsInnerBot[ 0 ];
			volume[ 7 ] = pmPointsOuterBot[ 0 ];
		}*/

		if ( i > 0 )
		{
			volume[ 0 ][ 2 ] += addHeight * (i);
			volume[ 1 ][ 2 ] += addHeight * (i);
			volume[ 2 ][ 2 ] += addHeight * (i);
			volume[ 3 ][ 2 ] += addHeight * (i);
			volume[ 4 ][ 2 ] += addHeight * (i);
			volume[ 5 ][ 2 ] += addHeight * (i);
			volume[ 6 ][ 2 ] += addHeight * (i);
			volume[ 7 ][ 2 ] += addHeight * (i);
		}

		if ( do2D )
		{
			pRender2D->DrawLine(volume[ 0 ], volume[ 1 ]);
			pRender2D->DrawLine(volume[ 0 ], volume[ 3 ]);
			pRender2D->DrawLine(volume[ 2 ], volume[ 1 ]);
			pRender2D->DrawLine(volume[ 2 ], volume[ 3 ]);

			pRender2D->DrawLine(volume[ 4 ], volume[ 5 ]);
			pRender2D->DrawLine(volume[ 4 ], volume[ 7 ]);
			pRender2D->DrawLine(volume[ 6 ], volume[ 5 ]);
			pRender2D->DrawLine(volume[ 6 ], volume[ 7 ]);

			pRender2D->DrawLine(volume[ 0 ], volume[ 4 ]);
			pRender2D->DrawLine(volume[ 1 ], volume[ 5 ]);
			pRender2D->DrawLine(volume[ 2 ], volume[ 6 ]);
			pRender2D->DrawLine(volume[ 3 ], volume[ 7 ]);
		}
		if ( do3D )
		{
			pRender3D->DrawLine(volume[ 0 ], volume[ 1 ]);
			pRender3D->DrawLine(volume[ 0 ], volume[ 3 ]);
			pRender3D->DrawLine(volume[ 2 ], volume[ 1 ]);
			pRender3D->DrawLine(volume[ 2 ], volume[ 3 ]);

			pRender3D->DrawLine(volume[ 4 ], volume[ 5 ]);
			pRender3D->DrawLine(volume[ 4 ], volume[ 7 ]);
			pRender3D->DrawLine(volume[ 6 ], volume[ 5 ]);
			pRender3D->DrawLine(volume[ 6 ], volume[ 7 ]);

			pRender3D->DrawLine(volume[ 0 ], volume[ 4 ]);
			pRender3D->DrawLine(volume[ 1 ], volume[ 5 ]);
			pRender3D->DrawLine(volume[ 2 ], volume[ 6 ]);
			pRender3D->DrawLine(volume[ 3 ], volume[ 7 ]);
		}
	}
}

void CToolBlock::PreviewWedge(CRender2D *pRender2D, CRender3D *pRender3D)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;
			
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = 3;

	float startAngle = GetMainWnd()->m_ObjectBar.GetBlockToolStartAngle();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	Vector origin;
	GetBoundsCenter(origin);

	Vector pmPointsTop[ 4 ];
	Vector pmPointsBot[ 4 ];
	polyMake(origin[ 0 ] - fWidth, origin[ 1 ] - fDepth, origin[ 0 ] + fWidth, origin[ 1 ] + fDepth, nSides, startAngle, pmPointsTop);
		
	pmPointsTop[0][0] = origin[0] + fWidth;
	pmPointsTop[0][1] = origin[1] + fDepth;
	pmPointsTop[0][2] = origin[2] + fHeight;

	pmPointsTop[1][0] = origin[0] + fWidth;
	pmPointsTop[1][1] = origin[1] - fDepth;
	pmPointsTop[1][2] = origin[2] + fHeight;

	pmPointsTop[2][0] = origin[0] - fWidth;
	pmPointsTop[2][1] = origin[1] - fDepth;
	pmPointsTop[2][2] = origin[2] + fHeight;
	
	// face 0 - top face
	for ( int i = 0; i < nSides; i++ )
	{		
		pmPointsTop[ i ][ 2 ] = origin[ 2 ] + fHeight;
	}

	if ( do2D ) // todo: for block, render filled lines in every 2d view?
	{
		pRender2D->DrawPolyLineFilled(nSides, pmPointsTop, 0.125f);
		pRender2D->DrawPolyLine(nSides, pmPointsTop); // to frame the filled poly
	}
	if( do3D)
		pRender3D->DrawPolyLine(nSides, pmPointsTop);
		
	// face 1 - bottom face
	for ( int i = 0; i < nSides; i++ )
	{
		pmPointsBot[i][0] = pmPointsTop[i][0];
		pmPointsBot[i][1] = pmPointsTop[i][1];
		pmPointsBot[i][2] = origin[2] - fHeight;
	}
	
	if( do2D)
		pRender2D->DrawPolyLine(nSides, pmPointsBot); // in 2d they are the same looking, no need to draw filled twice
	if( do3D)
		pRender3D->DrawPolyLine(nSides, pmPointsBot);

	// other edges
	for ( int i = 0; i < nSides; i++ )
	{
		if(do2D)
			pRender2D->DrawLine(pmPointsTop[ i ], pmPointsBot[ i ]);
		if(do3D)
			pRender3D->DrawLine(pmPointsTop[ i ], pmPointsBot[ i ]);
	}
}

typedef float PreviewTorusPointList_t[ARC_MAX_POINTS][3];

void PreviewTorusSegment(float fStartOuterPoints[][ 3 ], float fStartInnerPoints[][ 3 ],
	float fEndOuterPoints[][ 3 ], float fEndInnerPoints[][ 3 ],
	int iStart, int iEnd, BOOL bCreateSouthFace, CRender2D *pRender2D, CRender3D *pRender3D)
{
	Vector points[ 4 ];	// all sides have four vertices

	// create top face
	points[ 0 ][ 0 ] = fStartOuterPoints[ iStart ][ 0 ];
	points[ 0 ][ 1 ] = fStartOuterPoints[ iStart ][ 1 ];
	points[ 0 ][ 2 ] = fStartOuterPoints[ iStart ][ 2 ];

	points[ 1 ][ 0 ] = fStartOuterPoints[ iEnd ][ 0 ];
	points[ 1 ][ 1 ] = fStartOuterPoints[ iEnd ][ 1 ];
	points[ 1 ][ 2 ] = fStartOuterPoints[ iEnd ][ 2 ];

	points[ 2 ][ 0 ] = fStartInnerPoints[ iEnd ][ 0 ];
	points[ 2 ][ 1 ] = fStartInnerPoints[ iEnd ][ 1 ];
	points[ 2 ][ 2 ] = fStartInnerPoints[ iEnd ][ 2 ];

	points[ 3 ][ 0 ] = fStartInnerPoints[ iStart ][ 0 ];
	points[ 3 ][ 1 ] = fStartInnerPoints[ iStart ][ 1 ];
	points[ 3 ][ 2 ] = fStartInnerPoints[ iStart ][ 2 ];

	if ( pRender2D )
	{
		pRender2D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender2D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender2D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender2D->DrawLine(points[ 3 ], points[ 0 ]);
	}
	if ( pRender3D )
	{
		pRender3D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender3D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender3D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender3D->DrawLine(points[ 3 ], points[ 0 ]);
	}

// bottom face - set other z value and reverse order
	points[ 0 ][ 0 ] = fEndOuterPoints[ iStart ][ 0 ];
	points[ 0 ][ 1 ] = fEndOuterPoints[ iStart ][ 1 ];
	points[ 0 ][ 2 ] = fEndOuterPoints[ iStart ][ 2 ];

	points[ 1 ][ 0 ] = fEndOuterPoints[ iEnd ][ 0 ];
	points[ 1 ][ 1 ] = fEndOuterPoints[ iEnd ][ 1 ];
	points[ 1 ][ 2 ] = fEndOuterPoints[ iEnd ][ 2 ];

	points[ 2 ][ 0 ] = fEndInnerPoints[ iEnd ][ 0 ];
	points[ 2 ][ 1 ] = fEndInnerPoints[ iEnd ][ 1 ];
	points[ 2 ][ 2 ] = fEndInnerPoints[ iEnd ][ 2 ];

	points[ 3 ][ 0 ] = fEndInnerPoints[ iStart ][ 0 ];
	points[ 3 ][ 1 ] = fEndInnerPoints[ iStart ][ 1 ];
	points[ 3 ][ 2 ] = fEndInnerPoints[ iStart ][ 2 ];

	if ( pRender2D )
	{
		pRender2D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender2D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender2D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender2D->DrawLine(points[ 3 ], points[ 0 ]);
	}
	if ( pRender3D )
	{
		pRender3D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender3D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender3D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender3D->DrawLine(points[ 3 ], points[ 0 ]);
	}

// left side
	points[ 0 ][ 0 ] = fEndOuterPoints[ iStart ][ 0 ];
	points[ 0 ][ 1 ] = fEndOuterPoints[ iStart ][ 1 ];
	points[ 0 ][ 2 ] = fEndOuterPoints[ iStart ][ 2 ];

	points[ 1 ][ 0 ] = fStartOuterPoints[ iStart ][ 0 ];
	points[ 1 ][ 1 ] = fStartOuterPoints[ iStart ][ 1 ];
	points[ 1 ][ 2 ] = fStartOuterPoints[ iStart ][ 2 ];

	points[ 2 ][ 0 ] = fStartInnerPoints[ iStart ][ 0 ];
	points[ 2 ][ 1 ] = fStartInnerPoints[ iStart ][ 1 ];
	points[ 2 ][ 2 ] = fStartInnerPoints[ iStart ][ 2 ];

	points[ 3 ][ 0 ] = fEndInnerPoints[ iStart ][ 0 ];
	points[ 3 ][ 1 ] = fEndInnerPoints[ iStart ][ 1 ];
	points[ 3 ][ 2 ] = fEndInnerPoints[ iStart ][ 2 ];

	if ( pRender2D )
	{
		pRender2D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender2D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender2D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender2D->DrawLine(points[ 3 ], points[ 0 ]);
	}
	if ( pRender3D )
	{
		pRender3D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender3D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender3D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender3D->DrawLine(points[ 3 ], points[ 0 ]);
	}

// right side
	points[ 0 ][ 0 ] = fStartOuterPoints[ iEnd ][ 0 ];
	points[ 0 ][ 1 ] = fStartOuterPoints[ iEnd ][ 1 ];
	points[ 0 ][ 2 ] = fStartOuterPoints[ iEnd ][ 2 ];

	points[ 1 ][ 0 ] = fEndOuterPoints[ iEnd ][ 0 ];
	points[ 1 ][ 1 ] = fEndOuterPoints[ iEnd ][ 1 ];
	points[ 1 ][ 2 ] = fEndOuterPoints[ iEnd ][ 2 ];

	points[ 2 ][ 0 ] = fEndInnerPoints[ iEnd ][ 0 ];
	points[ 2 ][ 1 ] = fEndInnerPoints[ iEnd ][ 1 ];
	points[ 2 ][ 2 ] = fEndInnerPoints[ iEnd ][ 2 ];

	points[ 3 ][ 0 ] = fStartInnerPoints[ iEnd ][ 0 ];
	points[ 3 ][ 1 ] = fStartInnerPoints[ iEnd ][ 1 ];
	points[ 3 ][ 2 ] = fStartInnerPoints[ iEnd ][ 2 ];

	if ( pRender2D )
	{
		pRender2D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender2D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender2D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender2D->DrawLine(points[ 3 ], points[ 0 ]);
	}
	if ( pRender3D )
	{
		pRender3D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender3D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender3D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender3D->DrawLine(points[ 3 ], points[ 0 ]);
	}

// north face
	points[ 0 ][ 0 ] = fStartOuterPoints[ iEnd ][ 0 ];
	points[ 0 ][ 1 ] = fStartOuterPoints[ iEnd ][ 1 ];
	points[ 0 ][ 2 ] = fStartOuterPoints[ iEnd ][ 2 ];

	points[ 1 ][ 0 ] = fStartOuterPoints[ iStart ][ 0 ];
	points[ 1 ][ 1 ] = fStartOuterPoints[ iStart ][ 1 ];
	points[ 1 ][ 2 ] = fStartOuterPoints[ iStart ][ 2 ];

	points[ 2 ][ 0 ] = fEndOuterPoints[ iStart ][ 0 ];
	points[ 2 ][ 1 ] = fEndOuterPoints[ iStart ][ 1 ];
	points[ 2 ][ 2 ] = fEndOuterPoints[ iStart ][ 2 ];

	points[ 3 ][ 0 ] = fEndOuterPoints[ iEnd ][ 0 ];
	points[ 3 ][ 1 ] = fEndOuterPoints[ iEnd ][ 1 ];
	points[ 3 ][ 2 ] = fEndOuterPoints[ iEnd ][ 2 ];

	if ( pRender2D )
	{
		pRender2D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender2D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender2D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender2D->DrawLine(points[ 3 ], points[ 0 ]);
	}
	if ( pRender3D )
	{
		pRender3D->DrawLine(points[ 0 ], points[ 1 ]);
		pRender3D->DrawLine(points[ 1 ], points[ 2 ]);
		pRender3D->DrawLine(points[ 2 ], points[ 3 ]);
		pRender3D->DrawLine(points[ 3 ], points[ 0 ]);
	}

// south face
	if ( bCreateSouthFace )
	{
		points[ 0 ][ 0 ] = fStartInnerPoints[ iStart ][ 0 ];
		points[ 0 ][ 1 ] = fStartInnerPoints[ iStart ][ 1 ];
		points[ 0 ][ 2 ] = fStartInnerPoints[ iStart ][ 2 ];

		points[ 1 ][ 0 ] = fStartInnerPoints[ iEnd ][ 0 ];
		points[ 1 ][ 1 ] = fStartInnerPoints[ iEnd ][ 1 ];
		points[ 1 ][ 2 ] = fStartInnerPoints[ iEnd ][ 2 ];

		points[ 2 ][ 0 ] = fEndInnerPoints[ iEnd ][ 0 ];
		points[ 2 ][ 1 ] = fEndInnerPoints[ iEnd ][ 1 ];
		points[ 2 ][ 2 ] = fEndInnerPoints[ iEnd ][ 2 ];

		points[ 3 ][ 0 ] = fEndInnerPoints[ iStart ][ 0 ];
		points[ 3 ][ 1 ] = fEndInnerPoints[ iStart ][ 1 ];
		points[ 3 ][ 2 ] = fEndInnerPoints[ iStart ][ 2 ];

		if ( pRender2D )
		{
			pRender2D->DrawLine(points[ 0 ], points[ 1 ]);
			pRender2D->DrawLine(points[ 1 ], points[ 2 ]);
			pRender2D->DrawLine(points[ 2 ], points[ 3 ]);
			pRender2D->DrawLine(points[ 3 ], points[ 0 ]);
		}
		if ( pRender3D )
		{
			pRender3D->DrawLine(points[ 0 ], points[ 1 ]);
			pRender3D->DrawLine(points[ 1 ], points[ 2 ]);
			pRender3D->DrawLine(points[ 2 ], points[ 3 ]);
			pRender3D->DrawLine(points[ 3 ], points[ 0 ]);
		}
	}
}

void CToolBlock::PreviewTorus(CRender2D *pRender2D, CRender3D *pRender3D)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;
				
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = GetMainWnd()->m_ObjectBar.GetBlockToolFaceCount();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	CString str;
	int wallWidth = AfxGetApp()->GetProfileInt("Torus", "Wall Width", 32);
	str = AfxGetApp()->GetProfileString("Torus", "Arc_", "180");
	float arc = atof(str);
	nSides = AfxGetApp()->GetProfileInt("Torus", "Sides", 8);
	str = AfxGetApp()->GetProfileString("Torus", "Start Angle_", "0");
	float startAngle = atof(str);
	str = AfxGetApp()->GetProfileString("Torus", "Rotation Arc_", "90");
	float rotationArc = atof(str);
	str = AfxGetApp()->GetProfileString("Torus", "Rotation Start Angle_", "0");
	float rotationStartAngle = atof(str);
	str = AfxGetApp()->GetProfileString("Torus", "Cross Section Radius", "32");
	float crossSectionRadius = fHeight / 2;
	crossSectionRadius = atof(str);
	int rotationSides = AfxGetApp()->GetProfileInt("Torus", "Rotation Sides", 4);
	int addHeight = AfxGetApp()->GetProfileInt("Torus", "Add Height", 0);

	Vector origin;
	GetBoundsCenter(origin);
	
	//// torus
	float xCenter = (bmaxs[0] + bmins[0]) * 0.5f;
	float yCenter = (bmaxs[1] + bmins[1]) * 0.5f;
	float xRad = (bmaxs[0] - xCenter);
	float yRad = (bmaxs[1] - yCenter);

	if (xRad < 0.0f )
	{
		xRad = 0.0f;
	}

	if (yRad < 0.0f )
	{
		yRad = 0.0f;
	}

	if ( crossSectionRadius > (xRad * 0.5f) )
	{
		crossSectionRadius = (xRad * 0.5f);
	}
	if ( crossSectionRadius > (yRad * 0.5f) )
	{
		crossSectionRadius = (yRad * 0.5f);
	}

	if ( wallWidth < crossSectionRadius )
	{
		crossSectionRadius -= wallWidth;
	}
	else
	{
		wallWidth = crossSectionRadius;
		crossSectionRadius = 0.0f;
	}

	float crossSectionHalfWidth = crossSectionRadius + wallWidth;
	xRad -= crossSectionHalfWidth;
	yRad -= crossSectionHalfWidth;

	Vector fOuterPoints[ARC_MAX_POINTS];
	Vector fInnerPoints[ARC_MAX_POINTS];

	// create outer points (unrotated)
	MakePreviewArcCenterRadius(0.0f, 0.0f,
		crossSectionRadius + wallWidth, crossSectionRadius + wallWidth, 
		nSides, startAngle, arc, fOuterPoints);

	BOOL bCreateSouthFace = TRUE;
	if ( crossSectionRadius != 0.0f )
	{
		// create inner points (unrotated)
		MakePreviewArcCenterRadius(0.0f, 0.0f, crossSectionRadius, crossSectionRadius, 
			nSides, startAngle, arc, fInnerPoints);
	}
	else
	{
		for( int i = 0; i < nSides; i++)
		{
			fInnerPoints[i][0] = fInnerPoints[i][1] = 0.0f;
		}
		bCreateSouthFace = FALSE;
	}
	
	PreviewTorusPointList_t innerPoints[2];
	PreviewTorusPointList_t outerPoints[2];
	PreviewTorusPointList_t *pStartInnerPoints;
	PreviewTorusPointList_t *pStartOuterPoints;
	PreviewTorusPointList_t *pEndInnerPoints = &innerPoints[1];
	PreviewTorusPointList_t *pEndOuterPoints = &outerPoints[1];
	int nCurrIndex = 0;

	float flCurrentZ = bmins[2] + wallWidth + crossSectionRadius;
	float flDeltaZ = (float)addHeight / (float)(rotationSides);

	float flRotationAngle = rotationStartAngle;
	float flRotationDeltaAngle = rotationArc / rotationSides;

	bool bIsCircle = ( addHeight == 0.0f ) && ( rotationArc == 360.0f );
	++rotationSides;
	for ( int i = 0; i != rotationSides; ++i )
	{
		// This eliminates a seam in circular toruses
		if ( bIsCircle && (i == rotationSides - 1) )
		{
			flRotationAngle = rotationStartAngle;
		}

		float xCurrCenter, yCurrCenter;

		float flCosAngle = cos( DEG2RAD(flRotationAngle) );
		float flSinAngle = sin( DEG2RAD(flRotationAngle) );
		xCurrCenter = xCenter + xRad * flCosAngle;
		yCurrCenter = yCenter + yRad * flSinAngle;

		// Update buffers
		pStartInnerPoints = pEndInnerPoints;
		pStartOuterPoints = pEndOuterPoints;
		pEndInnerPoints = &innerPoints[nCurrIndex];
		pEndOuterPoints = &outerPoints[nCurrIndex];
		nCurrIndex = 1 - nCurrIndex;

		// Transform points into actual space.
		int jPrevPoint = -1;
		int j = 0;
		do
		{
			// x original is transformed into x/y based on rotation
			// y original is transformed into z
			(*pEndInnerPoints)[j][0] = xCurrCenter + fInnerPoints[j][0] * flCosAngle;
			(*pEndInnerPoints)[j][1] = yCurrCenter + fInnerPoints[j][0] * flSinAngle;
			(*pEndInnerPoints)[j][2] = flCurrentZ + fInnerPoints[j][1];

			(*pEndOuterPoints)[j][0] = xCurrCenter + fOuterPoints[j][0] * flCosAngle;
			(*pEndOuterPoints)[j][1] = yCurrCenter + fOuterPoints[j][0] * flSinAngle;
			(*pEndOuterPoints)[j][2] = flCurrentZ + fOuterPoints[j][1];

			// We'll use the j == 0 data when iNextPoint = iSides - 1
			if ( ( i != 0 ) && ( jPrevPoint != -1 ) )
			{
				PreviewTorusSegment(*pStartOuterPoints, *pStartInnerPoints,
					*pEndOuterPoints, *pEndInnerPoints,
					jPrevPoint, j, bCreateSouthFace, pRender2D, pRender3D);
			}

			jPrevPoint = j;
			++j;
		} while( jPrevPoint != nSides );

		flRotationAngle += flRotationDeltaAngle;
		flCurrentZ += flDeltaZ;

		if ( flRotationAngle >= 360.0f )
		{
			flRotationAngle -= 360.0f;
		}
	}
}

#ifdef SLE //// more stock solids
void CToolBlock::PreviewBipyramid(CRender2D *pRender2D, CRender3D *pRender3D)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;
			
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = GetMainWnd()->m_ObjectBar.GetBlockToolFaceCount();
	nSides /= 2; // bipyramid is symmetrical, face setting gets doubled on creation

	float startAngle = GetMainWnd()->m_ObjectBar.GetBlockToolStartAngle();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	Vector origin;
	GetBoundsCenter(origin);

	Vector pmPoints[ 128 ];

	polyMake(origin[ 0 ] - fWidth, origin[ 1 ] - fDepth, origin[ 0 ] + fWidth, origin[ 1 ] + fDepth, nSides, startAngle, pmPoints);
		
	// top point
	Vector top;
	top[0] = origin[0];
	top[1] = origin[1];
	top[2] = origin[2] + fHeight;

	// bottom point
	Vector bot;
	bot = top;
	bot[2] = origin[2] - fHeight;
			
	// mid "face"
	for ( int i = 0; i < nSides; i++ )
	{		
		pmPoints[ i ][ 2 ] = origin[ 2 ];
	}

	if ( do2D ) // todo: for block, render filled lines in every 2d view?
	{
		pRender2D->DrawPolyLineFilled(nSides, pmPoints, 0.125f);
		pRender2D->DrawPolyLine(nSides, pmPoints); // regular line to frame the filled poly
	}
	if( do3D)
		pRender3D->DrawPolyLine(nSides, pmPoints);
		
	// lines connecting top point to mid "face"
	for ( int i = 0; i < nSides; i++ )
	{
		if( do2D)
			pRender2D->DrawLine(top, pmPoints[i]);
		if( do3D)
			pRender3D->DrawLine(top, pmPoints[i]);
	}

	// lines connecting bottom point to mid "face"
	for ( int i = 0; i < nSides; i++ )
	{
		if( do2D)
			pRender2D->DrawLine(bot, pmPoints[i]);
		if( do3D)
			pRender3D->DrawLine(bot, pmPoints[i]);
	}
}

void CToolBlock::PreviewElongatedBipyramid(CRender2D *pRender2D, CRender3D *pRender3D, bool isGyro)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;
			
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = GetMainWnd()->m_ObjectBar.GetBlockToolFaceCount();
	
	float startAngle = GetMainWnd()->m_ObjectBar.GetBlockToolStartAngle();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	Vector origin;
	GetBoundsCenter(origin);

	// bottom of the top piramid
	Vector pmPointsTop[128];
	Vector pmPointsBot[128];

	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle, pmPointsTop );

	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsTop[i][2] = (origin[2] + fHeight * 0.5f); 
	}

	if ( do2D ) // todo: for block, render filled lines in every 2d view?
	{
		pRender2D->DrawPolyLineFilled(nSides, pmPointsTop, 0.125f);
		pRender2D->DrawPolyLine(nSides, pmPointsTop); // regular line to frame the filled poly
	}
	
	// top penta
	Vector topPoints[3];

	// get centerpoint
	topPoints[0][0] = origin[0];
	topPoints[0][1] = origin[1];

	topPoints[0][2] = (origin[2] + fHeight);

	for(int i = 0; i < nSides; i++)
	{
		topPoints[1] = pmPointsTop[i];
		topPoints[2] = pmPointsTop[i+1];
		
		if ( do2D )
			pRender2D->DrawPolyLine(3, topPoints);
		if( do3D)
			pRender3D->DrawPolyLine(3, topPoints);
	}

	// top of the bottom pyramid
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, isGyro ? startAngle + (180 / nSides) : startAngle, pmPointsBot );
	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsBot[i][2] = (origin[2] - fHeight * 0.5f);
	}

	if ( do2D )
	{
		pRender2D->DrawPolyLine(nSides, pmPointsTop);
	}

	Vector botPoints[3];

	// get centerpoint
	botPoints[0][0] = origin[0];
	botPoints[0][1] = origin[1];

	botPoints[0][2] = (origin[2] - fHeight);

	// bottom piramyd
	for(int i = 0; i < nSides; i++)
	{
		botPoints[2] = pmPointsBot[i];
		botPoints[1] = pmPointsBot[i+1];
		
		if ( do2D )
			pRender2D->DrawPolyLine(3, botPoints);
		if( do3D)
			pRender3D->DrawPolyLine(3, botPoints);
	}

	if ( isGyro )
	{

	// mid band of triangles
		Vector midPoints[ 3 ];
		midPoints[ 0 ][ 2 ] = pmPointsTop[ 0 ][ 2 ];
		midPoints[ 1 ][ 2 ] = pmPointsTop[ 0 ][ 2 ];
		midPoints[ 2 ][ 2 ] = pmPointsBot[ 0 ][ 2 ];
		for ( int i = 0; i < nSides; i++ )
		{
			midPoints[ 0 ][ 0 ] = pmPointsTop[ i + 1 ][ 0 ];
			midPoints[ 0 ][ 1 ] = pmPointsTop[ i + 1 ][ 1 ];
			midPoints[ 1 ][ 0 ] = pmPointsTop[ i ][ 0 ];
			midPoints[ 1 ][ 1 ] = pmPointsTop[ i ][ 1 ];
			midPoints[ 2 ][ 0 ] = pmPointsBot[ i ][ 0 ];
			midPoints[ 2 ][ 1 ] = pmPointsBot[ i ][ 1 ];
			
			if ( do2D )
				pRender2D->DrawPolyLine(3, midPoints);
			if ( do3D )
				pRender3D->DrawPolyLine(3, midPoints);
		}
		midPoints[ 0 ][ 2 ] = pmPointsBot[ 0 ][ 2 ];
		midPoints[ 1 ][ 2 ] = pmPointsBot[ 0 ][ 2 ];
		midPoints[ 2 ][ 2 ] = pmPointsTop[ 0 ][ 2 ];
		for ( int i = 0; i < nSides; i++ )
		{
			midPoints[ 0 ][ 0 ] = pmPointsBot[ i ][ 0 ];
			midPoints[ 0 ][ 1 ] = pmPointsBot[ i ][ 1 ];
			midPoints[ 1 ][ 0 ] = pmPointsBot[ i + 1 ][ 0 ];
			midPoints[ 1 ][ 1 ] = pmPointsBot[ i + 1 ][ 1 ];
			midPoints[ 2 ][ 0 ] = pmPointsTop[ i + 1 ][ 0 ];
			midPoints[ 2 ][ 1 ] = pmPointsTop[ i + 1 ][ 1 ];
			
			if ( do2D )
				pRender2D->DrawPolyLine(3, midPoints);
			if ( do3D )
				pRender3D->DrawPolyLine(3, midPoints);
		}
	}
	else
	{
	// mid band of rectangles
		Vector midPoints[ 4 ];
		midPoints[ 0 ][ 2 ] = pmPointsTop[ 0 ][ 2 ];
		midPoints[ 1 ][ 2 ] = pmPointsTop[ 0 ][ 2 ];
		midPoints[ 2 ][ 2 ] = pmPointsBot[ 0 ][ 2 ];
		midPoints[ 3 ][ 2 ] = pmPointsBot[ 0 ][ 2 ];
		for ( int i = 0; i < nSides; i++ )
		{
			midPoints[ 0 ][ 0 ] = pmPointsTop[ i + 1 ][ 0 ];
			midPoints[ 0 ][ 1 ] = pmPointsTop[ i + 1 ][ 1 ];
			midPoints[ 1 ][ 0 ] = pmPointsTop[ i ][ 0 ];
			midPoints[ 1 ][ 1 ] = pmPointsTop[ i ][ 1 ];
			midPoints[ 2 ][ 0 ] = pmPointsBot[ i ][ 0 ];
			midPoints[ 2 ][ 1 ] = pmPointsBot[ i ][ 1 ];
			midPoints[ 3 ][ 0 ] = pmPointsBot[ i + 1 ][ 0 ];
			midPoints[ 3 ][ 1 ] = pmPointsBot[ i + 1 ][ 1 ];

			if ( do2D )
				pRender2D->DrawPolyLine(4, midPoints);
			if ( do3D )
				pRender3D->DrawPolyLine(4, midPoints);
		}
	}
}

void CToolBlock::PreviewAntiprism(CRender2D *pRender2D, CRender3D *pRender3D)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;
			
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = GetMainWnd()->m_ObjectBar.GetBlockToolFaceCount(); // for antiprism, number of faces is the base (top/bottom polygon sides), not total of faces

	float startAngle = GetMainWnd()->m_ObjectBar.GetBlockToolStartAngle();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	Vector origin;
	GetBoundsCenter(origin);

	Vector pmPointsTop[256];
	Vector pmPointsBot[256];

	// top face
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle, pmPointsTop );

	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsTop[i][2] = (origin[2] + fHeight);
	}

	if ( nSides > 2 ) // antiprism can exist with 2D polygon at top and bottom, but the editor will need it to become a 4-sided solid
	{
		if ( do2D )
		{
			pRender2D->DrawPolyLineFilled(nSides, pmPointsTop, 0.125f);
			pRender2D->DrawPolyLine(nSides, pmPointsTop); // to frame the filled poly
		}
		if ( do3D )
			pRender3D->DrawPolyLine(nSides, pmPointsTop);

	}

	// bottom face
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle + (180 / nSides), pmPointsBot );
	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsBot[i][2] = (origin[2] - fHeight);
	}
	
	if ( nSides > 2 ) // see above
	{
		if ( do2D )
		{
			pRender2D->DrawPolyLine(nSides, pmPointsBot); // top and bottom look the same from above, no need for filled
		}
		if ( do3D )
			pRender3D->DrawPolyLine(nSides, pmPointsBot);
	}

	// middle band of triangles
	Vector midPoints[3];
	midPoints[0][2] = pmPointsTop[0][2];
	midPoints[1][2] = pmPointsTop[0][2];
	midPoints[2][2] = pmPointsBot[0][2];

	for ( int i = 0; i < nSides; i++ )
	{
		midPoints[0][0] = pmPointsTop[i+1][0];
		midPoints[0][1] = pmPointsTop[i+1][1]; 
		midPoints[1][0] = pmPointsTop[i][0];
		midPoints[1][1] = pmPointsTop[i][1];
		midPoints[2][0] = pmPointsBot[i][0];
		midPoints[2][1] = pmPointsBot[i][1];

		if( do2D)
			pRender2D->DrawPolyLine(3, midPoints);
		if( do3D)
			pRender3D->DrawPolyLine(3, midPoints);
	}	

	midPoints[0][2] = pmPointsBot[0][2];
	midPoints[1][2] = pmPointsBot[0][2];
	midPoints[2][2] = pmPointsTop[0][2];

	for ( int i = 0; i < nSides; i++ )
	{
		midPoints[0][0] = pmPointsBot[i][0];
		midPoints[0][1] = pmPointsBot[i][1];
		midPoints[1][0] = pmPointsBot[i+1][0];
		midPoints[1][1] = pmPointsBot[i+1][1]; 
		midPoints[2][0] = pmPointsTop[i+1][0];
		midPoints[2][1] = pmPointsTop[i+1][1];
		
		if( do2D)
			pRender2D->DrawPolyLine(3, midPoints);
		if( do3D)
			pRender3D->DrawPolyLine(3, midPoints);
	}
}

void CToolBlock::PreviewIcosahedron(CRender2D *pRender2D, CRender3D *pRender3D)
{
	bool do2D = pRender2D != NULL;
	bool do3D = pRender3D != NULL;

	if ( !do2D && !do3D ) return;
			
	float fWidth = float(bmaxs[ 0 ] - bmins[ 0 ]) / 2;
	float fDepth = float(bmaxs[ 1 ] - bmins[ 1 ]) / 2;
	float fHeight = float(bmaxs[ 2 ] - bmins[ 2 ]) / 2;

	int nSides = 12;

	float startAngle = GetMainWnd()->m_ObjectBar.GetBlockToolStartAngle();

	if( fWidth <= 0 || fDepth <= 0 || fHeight <= 0) return;

	Vector origin;
	GetBoundsCenter(origin);

	// bottom of the top penta
	Vector pmPointsTop[64];
	Vector pmPointsBot[64];

	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, 5, startAngle, pmPointsTop, false);

	for(int i = 0; i < 6; i++)
	{
		pmPointsTop[i][2] = (origin[2] + fHeight / 2); // the gap between the two pentagonal planes equals the radius of the circle around them, which is equal to the half width of the solid
	}

	// draw filled cross-section for 2d preivew
	// top and bottom pentas look the same from above, just use the top
	if(do2D)
		pRender2D->DrawPolyLineFilled(5, pmPointsTop, 0.125f);
	
	// top penta
	Vector topPoints[3];

	// get centerpoint
	topPoints[0][0] = origin[0];
	topPoints[0][1] = origin[1];

	topPoints[0][2] = (origin[2] + fHeight);

	for(int i = 0; i < 5; i++)
	{
		topPoints[1] = pmPointsTop[i];
		topPoints[2] = pmPointsTop[i+1];

		if(do2D)
			pRender2D->DrawPolyLine(3, topPoints);
		if(do3D)
			pRender3D->DrawPolyLine(3, topPoints);
	}

	// top of the bottom penta	
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, 5, startAngle + 36, pmPointsBot, false); // rotate bottom by 180 / 5 deg
	for(int i = 0; i < 6; i++)
	{
		pmPointsBot[i][2] = (origin[2] - fHeight / 2); // see above
	}

	Vector botPoints[3];

	// get centerpoint
	botPoints[0][0] = origin[0];
	botPoints[0][1] = origin[1];

	botPoints[0][2] = (origin[2] - fHeight);

	// bottom penta
	for(int i = 0; i < 5; i++)
	{
		botPoints[2] = pmPointsBot[i];
		botPoints[1] = pmPointsBot[i+1];
		
		if(do2D)
			pRender2D->DrawPolyLine(3, botPoints);
		if(do3D)
			pRender3D->DrawPolyLine(3, botPoints);
	}

	// mid band of triangles
	Vector midPoints[3];
	midPoints[0][2] = pmPointsTop[0][2];
	midPoints[1][2] = pmPointsTop[0][2];
	midPoints[2][2] = pmPointsBot[0][2];
	for ( int i = 0; i < 5; i++ )
	{
		midPoints[0][0] = pmPointsTop[i+1][0];
		midPoints[0][1] = pmPointsTop[i+1][1]; 
		midPoints[1][0] = pmPointsTop[i][0];
		midPoints[1][1] = pmPointsTop[i][1];
		midPoints[2][0] = pmPointsBot[i][0];
		midPoints[2][1] = pmPointsBot[i][1];
				
		if(do2D)
			pRender2D->DrawPolyLine(3, midPoints);
		if(do3D)
			pRender3D->DrawPolyLine(3, midPoints);
	}	
	midPoints[0][2] = pmPointsBot[0][2];
	midPoints[1][2] = pmPointsBot[0][2];
	midPoints[2][2] = pmPointsTop[0][2];
	for ( int i = 0; i < 5; i++ )
	{
		midPoints[0][0] = pmPointsBot[i][0];
		midPoints[0][1] = pmPointsBot[i][1];
		midPoints[1][0] = pmPointsBot[i+1][0];
		midPoints[1][1] = pmPointsBot[i+1][1]; 
		midPoints[2][0] = pmPointsTop[i+1][0];
		midPoints[2][1] = pmPointsTop[i+1][1];
						
		if(do2D)
			pRender2D->DrawPolyLine(3, midPoints);
		if(do3D)
			pRender3D->DrawPolyLine(3, midPoints);
	}
}
#endif

//// SLE NEW - allow nudging block tool box with arrow keys
void CToolBlock::NudgeBlock(CMapView *pView, int nChar, bool bSnap)
{
	Vector vecDelta, vVert, vHorz, vThrd;

	pView->GetBestTransformPlane( vHorz, vVert, vThrd );

    m_pDocument->GetNudgeVector( vHorz, vVert,  nChar, bSnap, vecDelta);

	BoundBox Box = *this;

	Vector mins, maxs;
	GetBounds(mins, maxs);
	SetBounds(mins + vecDelta, maxs + vecDelta);

	m_pDocument->UpdateAllViews(MAPVIEW_UPDATE_TOOL);
	m_pDocument->RenderAllViews();

	CMapView2DBase *pView2D = dynamic_cast<CMapView2DBase*>(pView);

	if ( !pView2D )
		return;
	
	// Try to keep the selection fully in the view if it started that way.
	bool bFullyVisible = pView2D->IsBoxFullyVisible(bmins, bmaxs);

	// Make sure it can still fit entirely in the view after nudging and don't scroll the
	// view if it can't. This second check is probably unnecessary, but it can't hurt,
	// and there might be cases where the selection changes size after a nudge operation.
	if (bFullyVisible && pView2D->CanBoxFitInView(bmins, bmaxs))
	{
		pView2D->LockWindowUpdate();
		pView2D->EnsureVisible(bmins, 25);
		pView2D->EnsureVisible(bmaxs, 25);
		pView2D->UnlockWindowUpdate();
	}
}
#endif
