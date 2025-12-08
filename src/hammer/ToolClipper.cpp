//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "stdafx.h"
#include "GlobalFunctions.h"
#include "History.h"
#include "MapDefs.h"
#include "MapDoc.h"
#include "MapFace.h"
#include "MapSolid.h"
#include "MapView2D.h"
#include "MapWorld.h"
#include "Material.h"
#include "Render2D.h"
#include "Render3D.h"
#include "RenderUtils.h"
#include "StatusBarIDs.h"		// dvs: remove
#include "ToolClipper.h"
#include "ToolManager.h"
#include "vgui/Cursor.h"
#include "Selection.h"
#ifdef SLE
#include "mapoverlay.h"
#include "mapview3d.h"
#include "Camera.h"
#include "MainFrm.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#pragma warning( disable:4244 )

//=============================================================================
//
// Friend Function (for MapClass->EnumChildren Callback)
//

//-----------------------------------------------------------------------------
// Purpose: This function creates a new clip group with the given solid as 
//          the original solid.
//   Input: pSolid - the original solid to put in the clip list
//          pClipper - the clipper tool
//  Output: successful?? (true/false)
//-----------------------------------------------------------------------------
BOOL AddToClipList( CMapSolid *pSolid, Clipper3D *pClipper )
{
    CClipGroup *pClipGroup = new CClipGroup;
    if( !pClipGroup )
        return false;

    pClipGroup->SetOrigSolid( pSolid );
    pClipper->m_ClipResults.AddToTail( pClipGroup );

    return true;
}

//=============================================================================
//
// CClipGroup
//

//-----------------------------------------------------------------------------
// Purpose: Destructor. Gets rid of the unnecessary clip solids.
//-----------------------------------------------------------------------------
CClipGroup::~CClipGroup()
{
	delete m_pClipSolids[0];
	delete m_pClipSolids[1];
}

#ifdef SLE //// SLE NEW - add context menu for the clipping tool
class Clipper3DMessageWnd : public CWnd
{
	public:
		bool Create(void);
		void PreMenu2D(Clipper3D *pTool, CMapView2D *pView);
		void PreMenu3D(Clipper3D *pTool, CMapView3D *pView);

	protected:
		//{{AFX_MSG_MAP(Clipper3DMessageWnd)
		afx_msg void OnCmdFinishClipping();
		afx_msg void OnCmdIterate();
		afx_msg void OnCmdCancelClipping();
		//}}AFX_MSG	
		DECLARE_MESSAGE_MAP()

	private:
		Clipper3D *m_pToolClipper;
		CMapView2D *m_pView2D;
		CMapView3D *m_pView3D;
};

static Clipper3DMessageWnd s_wndToolMessage;
static const char *g_pszClassName = "SLE_ClipperToolWnd";

BEGIN_MESSAGE_MAP(Clipper3DMessageWnd, CWnd)
	ON_COMMAND(ID_CLIPPER_FINISH, OnCmdFinishClipping)
	ON_COMMAND(ID_CLIPPER_ITERATE, OnCmdIterate)
	ON_COMMAND(ID_CLIPPER_CANCEL, OnCmdCancelClipping)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// Purpose: Creates the hidden window that receives context menu commands for the
//			block tool.
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool Clipper3DMessageWnd::Create(void)
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
void Clipper3DMessageWnd::PreMenu2D(Clipper3D *pToolClipper, CMapView2D *pView)
{
	Assert(pToolClipper != NULL);
	m_pToolClipper = pToolClipper;
	m_pView2D = pView;
}

void Clipper3DMessageWnd::PreMenu3D(Clipper3D *pToolClipper, CMapView3D *pView)
{
	Assert(pToolClipper != NULL);
	m_pToolClipper = pToolClipper;
	m_pView3D = pView;
}

void Clipper3DMessageWnd::OnCmdFinishClipping()
{
	CMapView *view;
	if ( m_pView3D == NULL ) view = m_pView2D;
	else view = m_pView3D;
	m_pToolClipper->SaveClipResults();
}

void Clipper3DMessageWnd::OnCmdIterate()
{
	CMapView *view;
	if ( m_pView3D == NULL ) view = m_pView2D;
	else view = m_pView3D;
	m_pToolClipper->IterateClipMode();
}

void Clipper3DMessageWnd::OnCmdCancelClipping()
{
	CMapView *view;
	if ( m_pView3D == NULL ) view = m_pView2D;
	else view = m_pView3D;
	m_pToolClipper->OnEscape();
}
#endif //// SLE
//-----------------------------------------------------------------------------
// Purpose: constructor - initialize the clipper variables
//-----------------------------------------------------------------------------
Clipper3D::Clipper3D(void)
{
	m_Mode = FRONT;

    m_ClipPlane.normal.Init();
    m_ClipPlane.dist = 0.0f;
    m_ClipPoints[0].Init();
    m_ClipPoints[1].Init();
#ifdef SLE /// SLE NEW - WIP 3-point clipping
	if ( Is3PointClipper() )
	{
		m_ClipPoints[ 2 ].Init();
	}
	m_leftClickCounter = 0; //// SLE NEW - add 3d operation for the clipper, both regular and 3-point
#endif
    m_ClipPointHit = -1;

    m_pOrigObjects = NULL;
#ifdef SLE //// SLE CHANGE - what a waste of time these last X years have been!
	m_bDrawMeasurements = true;
#else
	m_bDrawMeasurements = false;
#endif
	SetEmpty();
}

//-----------------------------------------------------------------------------
// Purpose: deconstructor
//-----------------------------------------------------------------------------
Clipper3D::~Clipper3D(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Called when the tool is activated.
// Input  : eOldTool - The ID of the previously active tool.
//-----------------------------------------------------------------------------
void Clipper3D::OnActivate()
{
	if (IsActiveTool())
	{
		//
		// Already the active tool - toggle the mode.
		//
        IterateClipMode();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when the tool is deactivated.
// Input  : eNewTool - The ID of the tool that is being activated.
//-----------------------------------------------------------------------------
void Clipper3D::OnDeactivate()
{
	SetEmpty();
#ifdef SLE //// SLE NEW - add 3d operation for the clipper, both regular and 3-point	
	m_leftClickCounter = 0;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: (virtual imp) This function handles the "dragging" of the mouse
//          while the left mouse button is depressed.  It updates the position
//          of the clipping plane point selected in the StartTranslation
//          function.  This function rebuilds the clipping plane and updates
//          the clipping solids when necessary.
//   Input: pt - current location of the mouse in the 2DView
//          uFlags - constrained clipping plane point movement
//          *dragSize - not used in the virtual implementation
//  Output: success of translation (TRUE/FALSE)
//-----------------------------------------------------------------------------
bool Clipper3D::UpdateTranslation( const Vector &vUpdate, UINT uFlags )
{
    // sanity check
    if( IsEmpty() )
        return false;

	Vector vNewPos = m_vOrgPos + vUpdate;

    // snap point if need be
	if ( uFlags & constrainSnap ) // todo: doesn't play nice with sub-1 grids right now
		m_pDocument->Snap( vNewPos, uFlags );

    //
    // update clipping point positions
    //
	if ( m_ClipPoints[m_ClipPointHit] == vNewPos )
		return false;

    
    if( uFlags & constrainMoveAll )
    {
		//
		// calculate the point and delta - to move both clip points simultaneously
		//
		
		Vector delta = vNewPos - m_ClipPoints[m_ClipPointHit];		
#ifdef SLE /// SLE NEW - WIP 3-point clipping
		if( Is3PointClipper())
		{
			m_ClipPoints[(m_ClipPointHit+1)%2] += delta;
			m_ClipPoints[(m_ClipPointHit+1)%3] += delta;
		}
		else
#endif
		{
			m_ClipPoints[(m_ClipPointHit+1)%2] += delta;
		}
    }

	m_ClipPoints[m_ClipPointHit] = vNewPos;

    // build the new clip plane and update clip results
    BuildClipPlane();

    GetClipResults();

	m_pDocument->UpdateAllViews( MAPVIEW_UPDATE_TOOL );

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: (virtual imp) This function defines all finishing functionality 
//          necessary at the end of a clipping action.  Nothing really!!!
// Input  : bSave - passed along the the Tool finish translation call
//-----------------------------------------------------------------------------
void Clipper3D::FinishTranslation( bool bSave )
{
    // get the clip results -- in case the update is a click and not a drag
    GetClipResults();

    Tool3D::FinishTranslation( bSave );
}

//-----------------------------------------------------------------------------
// Purpose: iterate through the types of clipping modes, update after an
//          iteration takes place to visualize the new clip results
//-----------------------------------------------------------------------------
void Clipper3D::IterateClipMode( void )
{
    //
    // increment the clipping mode (wrap when necessary)
    //
    m_Mode++;

    if( m_Mode > BOTH )
    {
        m_Mode = FRONT;
    }

    // update the clipped objects based on the mode
    GetClipResults();
}

//-----------------------------------------------------------------------------
// Purpose: This resets the solids to clip (the original list) and calls the
//          CalcClipResults function to generate new "clip" solids
//-----------------------------------------------------------------------------
void Clipper3D::GetClipResults( void )
{
    // reset the clip list to the original solid list
    SetClipObjects( m_pOrigObjects );

    // calculate the clipped objects based on the current "clip plane"
    CalcClipResults();
}

//-----------------------------------------------------------------------------
// Purpose: This function allows one to specifically set the clipping plane
//          information, as opposed to building a clip plane during "translation"
//   Input: pPlane - the plane information used to create the clip plane
//-----------------------------------------------------------------------------
void Clipper3D::SetClipPlane( PLANE *pPlane )
{
    //
    // copy the clipping plane info
    //
    m_ClipPlane.normal = pPlane->normal;
    m_ClipPlane.dist = pPlane->dist;
}

//-----------------------------------------------------------------------------
// Purpose: This function builds a clipping plane based on the clip point
//          locations manipulated in the "translation" functions and the 2DView
//-----------------------------------------------------------------------------
void Clipper3D::BuildClipPlane( void )
{
#ifdef SLE /// SLE NEW - WIP 3-point clipping
	if( Is3PointClipper())
	{
		Vector vec1 = m_ClipPoints[1] - m_ClipPoints[0];
		Vector vec2 = m_ClipPoints[2] - m_ClipPoints[0];
		Vector vec3; CrossProduct(vec1, vec2, vec3);

		m_ClipPlane.normal = GetNormalFromPoints(m_ClipPoints[0], m_ClipPoints[1], m_ClipPoints[2]);	
		m_ClipPlane.planepts[0] = m_ClipPoints[0];
		m_ClipPlane.planepts[1] = m_ClipPoints[1];
		m_ClipPlane.planepts[2] = m_ClipPoints[2];	
		m_ClipPlane.dist = DotProduct(m_ClipPlane.planepts[0], m_ClipPlane.normal);
	}
	else
#endif
	{
		// calculate the up vector
		Vector upVect = m_vPlaneNormal;

		// calculate the right vector
		Vector rightVect;
		VectorSubtract(m_ClipPoints[ 1 ], m_ClipPoints[ 0 ], rightVect);
		// calculate the forward (normal) vector
		Vector forwardVect;
		CrossProduct(upVect, rightVect, forwardVect);
		VectorNormalize(forwardVect);

		//
		// save the clip plane info
		//
		m_ClipPlane.normal = forwardVect;
		m_ClipPlane.dist = DotProduct(m_ClipPoints[ 0 ], forwardVect);
	}
}

//-----------------------------------------------------------------------------
// Purpose: This functions sets up the list of objects to be clipped.  
//          Initially the list is passed in (typically a Selection set).  On 
//          subsequent "translation" updates the list is refreshed from the 
//          m_pOrigObjects list.
//   Input: pList - the list of objects (solids) to be clipped
//-----------------------------------------------------------------------------
void Clipper3D::SetClipObjects( const CMapObjectList *pList )
{
    // check for an empty list
    if( !pList )
        return;

    // save the original list
    m_pOrigObjects = pList;

    // clear the clip results list
    ResetClipResults();

    //
    // copy solids into the clip list
    //
    FOR_EACH_OBJ( *m_pOrigObjects, pos )
    {
        CMapClass *pObject = m_pOrigObjects->Element( pos );
        if( !pObject )
            continue;

        if( pObject->IsMapClass( MAPCLASS_TYPE( CMapSolid ) ) )
        {
            AddToClipList( ( CMapSolid* )pObject, this );
        }

        pObject->EnumChildren( ENUMMAPCHILDRENPROC( AddToClipList ), DWORD( this ), MAPCLASS_TYPE( CMapSolid ) );
    }

    // the clipping list is not empty anymore
    m_bEmpty = false;
}

//-----------------------------------------------------------------------------
// Purpose: This function calculates based on the defined or given clipping
//          plane and clipping mode the new clip solids.
//-----------------------------------------------------------------------------
void Clipper3D::CalcClipResults( void )
{
    // sanity check
    if( IsEmpty() )
        return;

    //
    // iterate through and clip all of the solids in the clip list
    //
    FOR_EACH_OBJ( m_ClipResults, pos )
    {
        CClipGroup *pClipGroup = m_ClipResults.Element( pos );
        CMapSolid *pOrigSolid = pClipGroup->GetOrigSolid();

#ifdef SLE //// SLE NEW - 3d skybox preview
		bool markSkybox = pOrigSolid->Is3dSkybox();
#endif
        //
        // check the modes for which solids to generate
        //
        CMapSolid *pFront = NULL;
        CMapSolid *pBack = NULL;
        if( m_Mode == FRONT )
        {
            pOrigSolid->Split( &m_ClipPlane, &pFront, NULL );
        }
        else if( m_Mode == BACK )
        {
            pOrigSolid->Split( &m_ClipPlane, NULL, &pBack );
        }
        else if( m_Mode == BOTH )
        {
            pOrigSolid->Split( &m_ClipPlane, &pFront, &pBack );
        }

        if( pFront )
        {
			pFront->SetTemporary(true);
            pClipGroup->SetClipSolid( pFront, FRONT );
#ifdef SLE //// SLE NEW - 3d skybox preview
			if( markSkybox) pFront->SetSkyboxState(SKYBOX_YES);
#endif
        }

        if( pBack )
        {
			pBack->SetTemporary(true);
            pClipGroup->SetClipSolid( pBack, BACK );
#ifdef SLE //// SLE NEW - 3d skybox preview
			if( markSkybox) pBack->SetSkyboxState(SKYBOX_YES);
#endif
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: This function handles the removal of the "original" solid when it
//          has been clipped into new solid(s) or removed from the world (group
//          or entity) entirely.  It handles this in an undo safe fashion.
//   Input: pOrigSolid - the solid to remove
//-----------------------------------------------------------------------------
void Clipper3D::RemoveOrigSolid( CMapSolid *pOrigSolid )
{
	m_pDocument->DeleteObject(pOrigSolid);

    //
    // remove the solid from the selection set if in the seleciton set and
    // its parent is the world, or set the selection state to none parent is group
    // or entity in the selection set
    //    

	CSelection *pSelection = m_pDocument->GetSelection();

    if ( pSelection->IsSelected( pOrigSolid ) )
    {
        pSelection->SelectObject( pOrigSolid, scUnselect );
    }
    else
    {
        pOrigSolid->SetSelectionState( SELECT_NONE );
    }
}

//-----------------------------------------------------------------------------
// Purpose: This function handles the saving of newly clipped solids (derived
//          from an "original" solid).  It handles them in an undo safe fashion.
//   Input: pSolid - the newly clipped solid
//          pOrigSolid - the "original" solid or solid the clipped solid was
//                       derived from
//-----------------------------------------------------------------------------
void Clipper3D::SaveClipSolid( CMapSolid *pSolid, CMapSolid *pOrigSolid )
{
    //
    // no longer a temporary solid
    //
    pSolid->SetTemporary( FALSE );

    //
    // Add the new solid to the original solid's parent (group, entity, world, etc.).
    //
	m_pDocument->AddObjectToWorld(pSolid, pOrigSolid->GetParent());
    
    //
    // handle linking solid into selection -- via selection set when parent is the world
    // and selected, or set the selection state if parent is group or entity in selection set
    //
    if( m_pDocument->GetSelection()->IsSelected( pOrigSolid ) )
    {
        m_pDocument->SelectObject( pSolid, scSelect );
    }
    else
    {
        pSolid->SetSelectionState( SELECT_NORMAL );
    }

    GetHistory()->KeepNew( pSolid );
}

//-----------------------------------------------------------------------------
// Purpose: This function saves all the clipped solid information.  If new solids
//          were generated from the original, they are saved and the original is
//          set for desctruciton.  Otherwise, the original solid is kept.
//-----------------------------------------------------------------------------
void Clipper3D::SaveClipResults( void )
{
    // sanity check!
    if( IsEmpty() )
        return;

	// mark this place in the history
    GetHistory()->MarkUndoPosition( NULL, "Clip Objects" );

    //
    // save all new objects into the selection list
    //
    FOR_EACH_OBJ( m_ClipResults, pos )
    {
        CClipGroup *pClipGroup = m_ClipResults.Element( pos );
        if( !pClipGroup )
            continue;

        CMapSolid *pOrigSolid = pClipGroup->GetOrigSolid();
        CMapSolid *pBackSolid = pClipGroup->GetClipSolid( CClipGroup::BACK );
        CMapSolid *pFrontSolid = pClipGroup->GetClipSolid( CClipGroup::FRONT );

        //
        // save the front clip solid and clear the clip results list of itself
        //
        if( pFrontSolid )
        {
            SaveClipSolid( pFrontSolid, pOrigSolid );
            pClipGroup->SetClipSolid( NULL, CClipGroup::FRONT );
        }

        //
        // save the front clip solid and clear the clip results list of itself
        //
        if( pBackSolid )
        {
            SaveClipSolid( pBackSolid, pOrigSolid );
            pClipGroup->SetClipSolid( NULL, CClipGroup::BACK );
        }
#ifdef SLE //// SLE NEW - when clipping, find overlays targeting this solid, and update their target faces
		const CMapObjectList *list = pOrigSolid->GetDependents();

		if ( list->Count() > 0 )
		{
			for ( int i = 0; i < list->Count(); i++ )
			{
				CMapOverlay *overlay = dynamic_cast< CMapOverlay* >( list->Element(i) );
				if ( !overlay ) continue;
				if ( overlay->GetFaceCount() == 0 ) continue;

				// Get the overlay's face normal. Sample the first face it uses 
				// (in majority of uses overlay's faces - if there's more than one - have the same or almost same normals)
				Vector o_normal;
				overlay->GetFace(0)->GetFaceNormal(o_normal);

				// get all faces of the back solid, if it was kept.
				if ( pBackSolid )
				{
					for ( int f = 0; f < pBackSolid->GetFaceCount(); f++ )
					{
						// iterate through the faces and find those aligned with the overlay.
						// allow a bit of leeway in angles in case the user wants to keep the overlay
						// on the edge of a clipped slope, or some such scenario.
						Vector f_normal;
						pBackSolid->GetFace(f)->GetFaceNormal(f_normal);
						if( f_normal.Dot(o_normal) >= 0.5)
							overlay->SideList_AddFace(pBackSolid->GetFace(f));
					}
				}

				// get all faces of the back solid, if it was kept.
				if ( pFrontSolid )
				{
					for ( int f = 0; f < pFrontSolid->GetFaceCount(); f++ )
					{
						Vector f_normal;
						pFrontSolid->GetFace(f)->GetFaceNormal(f_normal);
						if ( f_normal.Dot(o_normal) >= 0.5 )
						overlay->SideList_AddFace(pFrontSolid->GetFace(f));
					}
				}
			}
		}

		m_leftClickCounter = 0; //// SLE NEW - add 3d operation for the clipper, both regular and 3-point // reset the track of hit pos
#endif
		// Send the notification that this solid as been clipped.
		pOrigSolid->PostUpdate( Notify_Clipped );

        // remove the original solid
        RemoveOrigSolid( pOrigSolid );
    }

    // set the the clipping results list as empty
    ResetClipResults();

	// update world and views
	
    m_pDocument->SetModifiedFlag();
}

//-----------------------------------------------------------------------------
// Purpose: Draws the measurements of a brush in the 2D view.
// Input  : pRender - 
//			pSolid - 
//			nFlags - 
//-----------------------------------------------------------------------------
void Clipper3D::DrawBrushExtents( CRender2D *pRender, CMapSolid *pSolid, int nFlags )
{
    //
    // get the bounds of the solid
    //
	Vector Mins, Maxs;
	pSolid->GetRender2DBox( Mins, Maxs );

	//
	// Determine which side of the clipping plane this solid is on in screen
	// space. This tells us where to draw the extents.
	//
    if( ( m_ClipPlane.normal[0] == 0 ) && ( m_ClipPlane.normal[1] == 0 ) && ( m_ClipPlane.normal[2] == 0 ) )
        return;

    Vector normal = m_ClipPlane.normal;

    if( nFlags & DBT_BACK )
    {
       VectorNegate( normal );
    }

	Vector2D planeNormal;

    pRender->TransformNormal( planeNormal, normal );

	if( planeNormal.x <= 0 )
    {
        nFlags &= ~DBT_RIGHT;
        nFlags |= DBT_LEFT;
    }
    else if( planeNormal.x > 0 )
    {
        nFlags &= ~DBT_LEFT;
        nFlags |= DBT_RIGHT;
    }

    if( planeNormal.y <= 0 )
    {
        nFlags &= ~DBT_BOTTOM;
        nFlags |= DBT_TOP;
    }
    else if( planeNormal.y > 0 )
    {
        nFlags &= ~DBT_TOP;
        nFlags |= DBT_BOTTOM;
    }

	DrawBoundsText(pRender, Mins, Maxs, nFlags);

#ifdef SLE //// SLE NEW - print the angle of the clip plane
	char extentText[30];
	Vector vert;
	pRender->GetViewUp(vert);

	// make it so it always shows angle in 0-90 range
	if( planeNormal.x <= 0 && planeNormal.y <= 0 )
		sprintf(extentText, "%.2f deg", fabs(-acos(m_ClipPlane.normal.Dot(vert)) / M_PI * 180));
	else
	{
		if ( planeNormal.x <= 0 || planeNormal.y > 0 )
			sprintf(extentText, "%.2f deg", acos(-m_ClipPlane.normal.Dot(vert)) / M_PI * 180);
		else
			sprintf(extentText, "%.2f deg", acos(m_ClipPlane.normal.Dot(vert)) / M_PI * 180);
	}

	Vector2D vecCenter;
#ifdef SLE /// SLE NEW - WIP 3-point clipping
	if ( !Is3PointClipper() )
	{
		vecCenter = Vector2D(( m_ClipPoints[ 0 ].x + m_ClipPoints[ 1 ].x + m_ClipPoints[ 2 ].x ) / 3, ( m_ClipPoints[ 0 ].y + m_ClipPoints[ 1 ].y + m_ClipPoints[ 2 ].y ) / 3);
	}
	else
#endif
	{
		vecCenter = Vector2D(( m_ClipPoints[ 0 ].x + m_ClipPoints[ 1 ].x ) / 2, ( m_ClipPoints[ 0 ].y + m_ClipPoints[ 1 ].y ) / 2);
	}
	if ( nFlags & DBT_LEFT )
	{
		vecCenter.x = vecCenter.x - ( HANDLE_RADIUS * 3 );
	}	
	else if ( nFlags & DBT_RIGHT )
	{
		vecCenter.x = vecCenter.x + ( HANDLE_RADIUS * 3 );
	}

	pRender->DrawText(extentText, Vector2D(vecCenter.x, vecCenter.y), 0, 0, nFlags);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pRender - 
//-----------------------------------------------------------------------------
void Clipper3D::RenderTool2D(CRender2D *pRender)
{
	if ( IsEmpty() )
		return;

    // check flag for rendering vertices
#ifdef SLE //// SLE CHANGE - assume the vertices are always wanted
    bool bDrawVerts = TRUE;
#else
	bool bDrawVerts = ( bool )( Options.view2d.bDrawVertices == TRUE );
#endif
    // setup the line to use

	pRender->SetDrawColor( 255, 255, 255 );

    //
    // render the clipped solids
    //
    FOR_EACH_OBJ( m_ClipResults, pos )
    {
        CClipGroup *pClipGroup = m_ClipResults.Element( pos );
        CMapSolid *pClipBack = pClipGroup->GetClipSolid( CClipGroup::BACK );
        CMapSolid *pClipFront = pClipGroup->GetClipSolid( CClipGroup::FRONT );
        if( !pClipBack && !pClipFront )
            continue;

        //
        // draw clip solids with the extents
        //
        if( pClipBack )
        {
            int faceCount = pClipBack->GetFaceCount();
            for( int i = 0; i < faceCount; i++ )
            {
                CMapFace *pFace = pClipBack->GetFace( i );

				// size 4
#ifdef SLE 	//// SLE NEW - draw filled outlines to make them eaiser to see in 2d when zoomed in
				pRender->SetDrawColor(0, 225, 255);
                pRender->DrawPolyLineFilled( pFace->nPoints, pFace->Points, 0.25f );
#endif
				pRender->DrawPolyLine(pFace->nPoints, pFace->Points);

			//	if ( bDrawVerts ) //// SLE REMOVE - calls function that contains nothing but an assert
			//	{
			//		pRender->DrawHandles( pFace->nPoints, pFace->Points );
			//	}

            //    if( m_bDrawMeasurements )
                {
                    DrawBrushExtents( pRender, pClipBack, DBT_TOP | DBT_LEFT | DBT_BACK );
                }
            }
        }

        if( pClipFront )
        {
            int faceCount = pClipFront->GetFaceCount();
            for( int i = 0; i < faceCount; i++ )
            {
                CMapFace *pFace = pClipFront->GetFace( i );

#ifdef SLE 		//// SLE NEW - draw filled outlines to make them eaiser to see in 2d when zoomed in
				pRender->SetDrawColor(0, 225, 255);
				pRender->DrawPolyLineFilled(pFace->nPoints, pFace->Points, 0.25f);
#endif
				pRender->DrawPolyLine(pFace->nPoints, pFace->Points);
				
			//	if ( bDrawVerts ) //// SLE REMOVE - calls function that contains nothing but an assert
			//	{
			//		pRender->DrawHandles( pFace->nPoints, pFace->Points );
			//	}

            //    if( m_bDrawMeasurements )
                {
                    DrawBrushExtents( pRender, pClipFront, DBT_BOTTOM | DBT_RIGHT );
                }
            }
        }
	}

    //
	// draw the clip-plane
	//
	pRender->SetDrawColor( 0, 255, 255 );
	pRender->DrawLine( m_ClipPoints[0], m_ClipPoints[1] );
#ifdef SLE /// SLE NEW - WIP 3-point clipping
	if ( Is3PointClipper() )
	{
		pRender->DrawLine(m_ClipPoints[ 1 ], m_ClipPoints[ 2 ]);
		pRender->DrawLine(m_ClipPoints[ 2 ], m_ClipPoints[ 0 ]);
	}
#endif
	//
	// draw the clip-plane endpoints
	//

	pRender->SetHandleStyle( HANDLE_RADIUS, CRender::HANDLE_SQUARE );
#ifdef SLE /// SLE NEW - WIP 3-point clipping
	if ( Is3PointClipper() )
	{
		pRender->SetHandleColor(255, 0, 0);
		pRender->DrawHandle(m_ClipPoints[ 0 ]);

		pRender->SetHandleColor(0, 255, 0);
		pRender->DrawHandle(m_ClipPoints[ 1 ]);

		pRender->SetHandleColor(255, 225, 0);
		pRender->DrawHandle(m_ClipPoints[ 2 ]);
		pRender->SetHandleColor( 255, 255, 255 );
		
		/*
		if (0)
		{
			Vector2D textPos2D;
			for ( int i = 0; i < 3; i++ )
			{
				if ( pRender->GetView()->GetDrawType() == VIEW2D_XY )  textPos2D = Vector2D(m_ClipPoints[i].x, m_ClipPoints[i].y);
				else if ( pRender->GetView()->GetDrawType() == VIEW2D_XZ ) textPos2D = Vector2D(m_ClipPoints[i].x, m_ClipPoints[i].z);
				else if( pRender->GetView()->GetDrawType() == VIEW2D_YZ)  textPos2D = Vector2D(m_ClipPoints[i].y, m_ClipPoints[i].z);
				textPos2D = Vector2D(m_ClipPoints[ i ].x, m_ClipPoints[ i ].y);		
				char drawText[ 2 ];
				sprintf(drawText, "p%i", i + 1);
				pRender->DrawTextA(drawText, textPos2D, 0, 0, 0);
			}
		}
		*/

		if ( pRender->GetView()->GetDrawType() == VIEW2D_XY )
		{
			for ( int i = 0; i < 3; i++ )
			{
				Vector2D textPos2D = Vector2D(m_ClipPoints[ i ].x, m_ClipPoints[ i ].y);				
				char drawText[ 2 ];
				sprintf(drawText, "p%i", i + 1);
				pRender->DrawTextA(drawText, textPos2D, 0, HANDLE_RADIUS, 0);
			}
		}
		/* // broken
		if ( pRender->GetView()->GetDrawType() == VIEW2D_XZ )
		{
			for ( int i = 0; i < 3; i++ )
			{
				Vector2D textPos2D = Vector2D(m_ClipPoints[i].x, -m_ClipPoints[i].z);				
				char drawText[ 2 ];
				sprintf(drawText, "p%i", i + 1);
				pRender->DrawTextA(drawText, textPos2D, 0, HANDLE_RADIUS, 0);
			}
		}

		if ( pRender->GetView()->GetDrawType() == VIEW2D_YZ )
		{
			for ( int i = 0; i < 3; i++ )
			{
				Vector2D textPos2D = Vector2D(-m_ClipPoints[i].z, -m_ClipPoints[i].y);				
				char drawText[ 2 ];
				sprintf(drawText, "p%i", i + 1);
				pRender->DrawTextA(drawText, textPos2D, 0, HANDLE_RADIUS, 0);
			}
		}
		*/
	}
	else
#endif
	{
		pRender->SetHandleColor(255, 255, 255);

		pRender->DrawHandle(m_ClipPoints[ 0 ]);
		pRender->DrawHandle(m_ClipPoints[ 1 ]);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Renders the brushes that will be left by the clipper in white
//			wireframe.
// Input  : pRender - Rendering interface.
//-----------------------------------------------------------------------------
void Clipper3D::RenderTool3D(CRender3D *pRender)
{
	// is there anything to render?
	if ( m_bEmpty )
		return;

	//
	// setup the renderer
	//
	pRender->PushRenderMode(RENDER_MODE_WIREFRAME);

	FOR_EACH_OBJ(m_ClipResults, pos)
	{
		CClipGroup *pClipGroup = m_ClipResults.Element(pos);

		CMapSolid *pFrontSolid = pClipGroup->GetClipSolid(CClipGroup::FRONT);
		if ( pFrontSolid )
		{
			color32 rgbColor = pFrontSolid->GetRenderColor();
#ifdef SLE //// SLE CHANGE: overlay existing clip solids in flat colour for better visiblity
			pFrontSolid->SetRenderColor(100, 255, 255);
#else
			pFrontSolid->SetRenderColor(255, 255, 255);
#endif
			pFrontSolid->Render3D(pRender);
			pFrontSolid->SetRenderColor(rgbColor);
#ifdef SLE //// SLE CHANGE: overlay existing clip solids in flat colour for better visiblity	
		//	pRender->PopRenderMode();
#endif
		}

		CMapSolid *pBackSolid = pClipGroup->GetClipSolid(CClipGroup::BACK);
		if ( pBackSolid )
		{
			color32 rgbColor = pBackSolid->GetRenderColor();
#ifdef SLE //// SLE CHANGE: overlay existing clip solids in flat colour for better visiblity
			pBackSolid->SetRenderColor(100, 255, 255);
#else
			pBackSolid->SetRenderColor(255, 255, 255);
#endif
			pBackSolid->Render3D(pRender);
			pBackSolid->SetRenderColor(rgbColor);
		}
	}

	pRender->PopRenderMode();

#ifdef SLE /// SLE NEW - WIP 3-point clipping	
	pRender->PushRenderMode(RENDER_MODE_WIREFRAME_NOZ);
	pRender->SetDrawColor(0, 225, 255);
	pRender->DrawLine(m_ClipPoints[ 0 ], m_ClipPoints[ 1 ]);
	if ( Is3PointClipper() )
	{
		pRender->DrawLine(m_ClipPoints[ 1 ], m_ClipPoints[ 2 ]);
		pRender->DrawLine(m_ClipPoints[ 2 ], m_ClipPoints[ 0 ]);
	}
	pRender->SetDrawColor(255, 225, 255);

	pRender->PushRenderMode(RENDER_MODE_FLAT_NOZ);
	pRender->SetHandleStyle(HANDLE_RADIUS, CRender::HANDLE_SQUARE);
	if ( Is3PointClipper() )
	{
		pRender->SetHandleColor(255, 0, 0);
		pRender->DrawHandle(m_ClipPoints[ 0 ]);
		pRender->SetHandleColor(0, 255, 0);
		pRender->DrawHandle(m_ClipPoints[ 1 ]);
		pRender->SetHandleColor(255, 225, 0);
		pRender->DrawHandle(m_ClipPoints[ 2 ]);
	} else
	{
		pRender->SetHandleColor(255, 255, 255);
		pRender->DrawHandle(m_ClipPoints[ 0 ]);
		pRender->DrawHandle(m_ClipPoints[ 1 ]);
	}
	pRender->SetHandleColor(255, 255, 255);
	pRender->PopRenderMode();

	//// render clipped solids overlay in 3d
	

#endif
}

//-----------------------------------------------------------------------------
// Purpose: (virtual imp)
// Input  : pt - 
//			BOOL - 
// Output : int
//-----------------------------------------------------------------------------
int Clipper3D::HitTest(CMapView *pView, const Vector2D &ptClient, bool bTestHandles)
{
    // check points    
#ifdef SLE /// SLE NEW - WIP 3-point clipping
	int imax = Is3PointClipper() ? 3 : 2;
	for (int i = 0; i <imax; i++ )
#else
	for (int i = 0; i < 2; i++);
#endif
	{
		if ( HitRect(pView, ptClient, m_ClipPoints[i], HANDLE_RADIUS) )
		{
			return i+1; // return clip point index + 1
		}
	}
    
    // neither point hit
    return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Reset (clear) the clip results.
//-----------------------------------------------------------------------------
void Clipper3D::ResetClipResults( void )
{
    //
    // delete the clip solids held in the list -- originals are just pointers
    // to pre-existing objects
    //
    FOR_EACH_OBJ( m_ClipResults, pos )
    {
        CClipGroup *pClipGroup = m_ClipResults.Element(pos);

        if( pClipGroup )
        {
            delete pClipGroup;
        }
    }

	m_ClipResults.RemoveAll();

    // the clipping list is empty
    SetEmpty();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nChar - 
//			nRepCnt - 
//			nFlags - 
//-----------------------------------------------------------------------------
bool Clipper3D::OnKeyDown2D(CMapView2D *pView, UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
#ifndef SLE //// SLE REMOVE - it now always draws
		case 'O':
		{
			//
			// Toggle the rendering of measurements.
			//
			ToggleMeasurements();
			return true;
		}
#endif
		case VK_RETURN:
		{
			//
			// Do the clip.
			//
			if (!IsEmpty() )
			{
				SaveClipResults();
			}
			return true;
		}

		case VK_ESCAPE:
		{
			OnEscape();
			return true;
		}
	}

	return false;	
}

//-----------------------------------------------------------------------------
// Purpose: Handles left mouse button down events in the 2D view.
// Input  : Per CWnd::OnLButtonDown.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool Clipper3D::OnLMouseDown2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint)
{
	Tool3D::OnLMouseDown2D(pView, nFlags, vPoint);

	unsigned int uConstraints = GetConstraints( nFlags );

	//
	// Convert point to world coords.
	//
	Vector vecWorld;
	pView->ClientToWorld(vecWorld, vPoint);
	vecWorld[pView->axThird] = COORD_NOTINIT;

	// getvisiblepoint fills in any coord that's still set to COORD_NOTINIT:
	m_pDocument->GetBestVisiblePoint(vecWorld);

	// snap starting position to grid
	if ( uConstraints & constrainSnap )
		m_pDocument->Snap(vecWorld, uConstraints);
		
	bool bStarting = false;

	// if the tool is not empty, and shift is not held down (to
	//  start a new camera), don't do anything.
	if(!IsEmpty())
	{
		// test for clip point hit (result = {0, 1, 2}
		int hitPoint = HitTest( pView, vPoint );

		if ( hitPoint > 0 )
		{
			// test for clip point hit (result = {0, 1, -1})
			m_ClipPointHit = hitPoint-1; // convert back to index
			m_vOrgPos = m_ClipPoints[m_ClipPointHit];
			StartTranslation( pView, vPoint );
		}
		else if ( m_vPlaneNormal != pView->GetViewAxis() )
		{
			SetEmpty();
			bStarting = true;
		}
		else
		{
			if (nFlags & MK_SHIFT)
			{
				SetEmpty();
				bStarting = true;
			}
			else
			{
				return true; // do nothing;
			}
		}
	}
	else
	{
		bStarting = true;
	}

	SetClipObjects(m_pDocument->GetSelection()->GetList());

	if (bStarting)
	{
		// start the tools translation functionality
		StartTranslation( pView, vPoint );

		// set the initial clip points
		m_ClipPointHit = 0;
		m_ClipPoints[0] = vecWorld; 
		m_ClipPoints[1] = vecWorld;
#ifdef SLE /// SLE NEW - WIP 3-point clipping
		if ( Is3PointClipper() )
		{
			m_ClipPoints[ 2 ] = vecWorld;
		}
#endif
		m_vOrgPos = vecWorld;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handles left mouse button up events in the 2D view.
// Input  : Per CWnd::OnLButtonUp.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool Clipper3D::OnLMouseUp2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint)
{
	Tool3D::OnLMouseUp2D(pView, nFlags, vPoint);

	if ( IsTranslating() )
	{
		FinishTranslation(true);
	}

	m_pDocument->UpdateStatusbar();
	
	return true;
}

unsigned int Clipper3D::GetConstraints(unsigned int nKeyFlags)
{
	unsigned int uConstraints = Tool3D::GetConstraints( nKeyFlags );

	if(nKeyFlags & MK_CONTROL)
	{
		uConstraints |= constrainMoveAll;
	}

	return uConstraints;
}

//-----------------------------------------------------------------------------
// Purpose: Handles mouse move events in the 2D view.
// Input  : Per CWnd::OnMouseMove.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool Clipper3D::OnMouseMove2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint)
{
	vgui::HCursor hCursor = vgui::dc_arrow;
	unsigned int uConstraints = GetConstraints( nFlags );

	Tool3D::OnMouseMove2D(pView, nFlags, vPoint);
					    
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
	
	if (IsTranslating())
	{
		// cursor is cross here
		Tool3D::UpdateTranslation( pView, vPoint, uConstraints);

		hCursor = vgui::dc_none;
	}
	else if (!IsEmpty())
	{
		//
		// If the cursor is on a handle, set it to a cross.
		//
		if (HitTest( pView, vPoint, true))
		{
			hCursor = vgui::dc_crosshair;
		}
	}

	if ( hCursor != vgui::dc_none  )
		pView->SetCursor( hCursor );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Handles character events.
// Input  : Per CWnd::OnKeyDown.
// Output : Returns true if the message was handled, false if not.
//-----------------------------------------------------------------------------
bool Clipper3D::OnKeyDown3D(CMapView3D *pView, UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
		case VK_RETURN:
		{
			if (!IsEmpty()) // dvs: what does isempty mean for the clipper?
			{
				SaveClipResults();
			}
			return true;
		}

		case VK_ESCAPE:
		{
			OnEscape();
			return true;
		}
	}

	return false;
}
#ifdef SLE //// SLE NEW - add 3d operation for the clipper, both regular and 3-point	
bool Clipper3D::OnMouseMove3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint)
{
	vgui::HCursor hCursor = vgui::dc_arrow;
	unsigned int uConstraints = GetConstraints( nFlags );

	Tool3D::OnMouseMove3D(pView, nFlags, vPoint);

	if (IsTranslating())
	{
		// cursor is cross here
		Tool3D::UpdateTranslation( pView, vPoint, uConstraints);

		hCursor = vgui::dc_none;
	}
//	else if (!IsEmpty())
	{
		//
		// If the cursor is on a handle, set it to a cross.
		//
		if (HitTest( pView, vPoint, true))
		{
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
		}
		else
		{
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		}
	}

	return true;
}

bool Clipper3D::OnLMouseDown3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint)
{
	Tool3D::OnLMouseDown3D(pView, nFlags, vPoint);	

	unsigned int uConstraints = GetConstraints( nFlags );

	// if aiming at a handle, start moving it
	// test for clip point hit (result = {0, 1, 2}
	int hitPoint = HitTest( pView, vPoint );

	if ( hitPoint > 0 )
	{
		// test for clip point hit (result = {0, 1, -1})
		m_ClipPointHit = hitPoint-1; // convert back to index
		m_vOrgPos = m_ClipPoints[m_ClipPointHit];
		StartTranslation( pView, vPoint );
		return true;
	}	
	// otherwise, place one of the clipping points
	else
	{
	//
	// See if they clicked on a brush face. If so, place one of the clipping points there. 
	//
		CMapDoc *pDoc = pView->GetMapDoc();

		ULONG ulFace;
		CMapClass *pObject;

		if ( ( pObject = pView->NearestObjectAt(vPoint, ulFace, FLAG_OBJECTS_AT_ONLY_SOLIDS/* | FLAG_OBJECTS_AT_RESOLVE_INSTANCES*/) ) != NULL )
		{
			CMapSolid *pSolid = dynamic_cast < CMapSolid * > ( pObject );
			if ( pSolid == NULL )
			{
				return true;
			}

			//
			// Build a ray to trace against the face that they clicked on to
			// find the point of intersection.
			//			
			Vector Start, End;
			pView->GetCamera()->BuildRay(vPoint, Start, End);

			Vector HitPos, HitNormal;
			CMapFace *pFace = pSolid->GetFace(ulFace);
			if ( pFace->TraceLine(HitPos, HitNormal, Start, End) )
			{
				m_ClipPoints[ m_leftClickCounter ] = HitPos;

				if ( Is3PointClipper() )
				{
					if ( m_leftClickCounter == 1 )
						m_ClipPoints[ m_leftClickCounter + 1 ] = HitPos; // if the user placed only 2 points, set the 3rd to be the same as 2nd, working as a regular 2-point plane
				}

				m_leftClickCounter++;
				if ( Is3PointClipper() )
				{
					if ( m_leftClickCounter > 2 ) m_leftClickCounter = 2;
				} else
				{
					if ( m_leftClickCounter > 1 ) m_leftClickCounter = 1;
				}

				if ( m_leftClickCounter >= 1 )
				{
					BuildClipPlane();
					if ( !Is3PointClipper() )
					{
						Vector perpendicular;
						Vector faceNormal;
						Vector clipLine;
						VectorSubtract(m_ClipPoints[1], m_ClipPoints[0], clipLine);
						pFace->GetFaceNormal(faceNormal);

						CrossProduct(faceNormal, clipLine, perpendicular);
						m_ClipPlane.normal = perpendicular;
					}

					GetClipResults();
					SetClipObjects(m_pDocument->GetSelection()->GetList());
					FinishTranslation(true);
				}
			}
		}
		return true;
	}
}

bool Clipper3D::OnLMouseUp3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint) 
{
	Tool3D::OnLMouseUp3D(pView, nFlags, vPoint);

	// if done translating the handle, let go and return
	if ( IsTranslating() )
	{
		FinishTranslation(true);

		m_pDocument->UpdateStatusbar();	
	}

	return true;
}

//// add context menu
bool Clipper3D::OnContextMenu2D(CMapView2D *pView, UINT nFlags, const Vector2D &vPoint)
{
	static CMenu menu, menuClipper;
	static bool bInit = false;

	if ( !bInit )
	{
		// Create the menu.
		menu.LoadMenu( IDR_POPUPS );
		menuClipper.Attach( ::GetSubMenu( menu.m_hMenu,  11 ) );
		bInit = true;

		// Create the window that handles menu messages.
		s_wndToolMessage.Create();
	}

//	if (!IsEmpty())
	{
	//	if ( HitTest( pView, vPoint, false ) )
		{
			CPoint ptScreen( vPoint.x,vPoint.y);
			pView->ClientToScreen(&ptScreen);

			s_wndToolMessage.PreMenu2D(this, pView);
			menuClipper.TrackPopupMenu( TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN, ptScreen.x, ptScreen.y, &s_wndToolMessage);
		}
	}

	return true;
}

bool Clipper3D::OnRMouseUp3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint)
{
	Tool3D::OnRMouseUp3D(pView, nFlags, vPoint);
	OnContextMenu3D(pView, nFlags, vPoint);
	return true;
}

bool Clipper3D::OnContextMenu3D(CMapView3D *pView, UINT nFlags, const Vector2D &vPoint)
{
	static CMenu menu, menuClipper;
	static bool bInit = false;

	if ( !bInit )
	{
		// Create the menu.
		menu.LoadMenu( IDR_POPUPS );
		menuClipper.Attach( ::GetSubMenu( menu.m_hMenu,  11 ) );
		bInit = true;

		// Create the window that handles menu messages.
		s_wndToolMessage.Create();
	}

//	if (!IsEmpty())
	{
	//	if ( HitTest( pView, vPoint, false ) )
		{
			CPoint ptScreen( vPoint.x,vPoint.y);
			pView->ClientToScreen(&ptScreen);
			s_wndToolMessage.PreMenu3D(this, pView);

			menuClipper.TrackPopupMenu( TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_LEFTALIGN, ptScreen.x, ptScreen.y, &s_wndToolMessage);
		}
	}

	return true;
}
#endif //// SLE
//-----------------------------------------------------------------------------
// Purpose: Handles the escape key in the 2D or 3D views.
//-----------------------------------------------------------------------------
void Clipper3D::OnEscape(void)
{
	// If we're clipping, clear it
	if (!IsEmpty())
	{
		SetEmpty();
	}
	else
	{
		m_pDocument->GetTools()->SetTool(TOOL_POINTER);
	}
}
