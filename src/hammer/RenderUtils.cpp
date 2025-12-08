//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "stdafx.h"
#include "Render2D.h"
#include "RenderUtils.h"
#include "mapview2d.h"
#include "toolinterface.h"
#ifdef SLE //// SLE CHANGE - show extents in chosen map units
#include "mapdoc.h"
#include "Box3D.h"
//WorldUnits_t Box3D::m_eWorldUnits = Units_None;
#endif
//-----------------------------------------------------------------------------
// Purpose: Draws the measurements of a brush in the 2D view.
// Input  : pRender -
//			Mins - 
//			Maxs -
//			nFlags - 
//-----------------------------------------------------------------------------
void DrawBoundsText(CRender2D *pRender, const Vector &Mins, const Vector &Maxs, int nFlags)
{
	CMapView2D *pView = (CMapView2D*) pRender->GetView();

	// Calculate the solid's extents along our 2D view axes.
	Vector Extents = Maxs - Mins;
	Vector Center = (Mins + Maxs ) * 0.5f;

	for ( int i=0; i<3;i++ )
		Extents[i] = fabs(Extents[i]);

	// Transform the solids mins and maxs to 2D view space. These are used
	// for placing the text in the view.
	Vector2D projMins, projMaxs, projCenter;
    pRender->TransformPoint( projMins, Mins );
    pRender->TransformPoint( projMaxs, Maxs );
	pRender->TransformPoint( projCenter, Center );

    if( projMins.x > projMaxs.x )
    {
		V_swap( projMins.x, projMaxs.x );
    }

    if( projMins.y > projMaxs.y )
    {
        V_swap( projMins.y, projMaxs.y );
    }

	//
    // display the extents of this brush
    //
    char extentText[30];
    int nTextX, nTextY;
	int nTextFlags;

	pRender->SetTextColor( 255, 255, 255 );

    // horz
#ifdef SLE //// SLE CHANGE - display brush sizes in finer fractions
#ifdef SLE //// SLE CHANGE - show extents in chosen map units
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if ( pDoc )
	{
		switch ( Box3D::GetWorldUnits() )
		{
			case Units_Feet_Inches:
			{
				int nFeet = ( int )fabs(Extents[ pView->axHorz ]) / 12;
				int nInches = ( int )fabs(Extents[ pView->axHorz ]) % 12;
				
				sprintf(extentText, "%d' %d\"", nFeet, nInches);
				break;
			}

			case Units_Centimeters:
			{
				float fCM = ( int )fabs(Extents[ pView->axHorz ]) * 2.54;

				sprintf(extentText, "%.3f cm", fCM);
				break;
			}

			case Units_Meters_Centimeters:
			{
				float fMCM = ( int )fabs(Extents[ pView->axHorz ]) * 0.0254;

				sprintf(extentText, " %.4f m", fMCM);
				break;
			}

			default:
			{
				sprintf(extentText, "%.3f", Extents[ pView->axHorz ]);
				break;
			}
		}
	}
#endif
#else
     sprintf( extentText, "%.1f", Extents[pView->axHorz] );
#endif
 	nTextFlags = CRender2D::TEXT_JUSTIFY_HORZ_CENTER;
    nTextX = projCenter.x;

	if ( nFlags & DBT_TOP )
	{
		nTextY = projMins.y - (HANDLE_RADIUS*3);
		nTextFlags |= CRender2D::TEXT_JUSTIFY_TOP;
	}
	else
	{
		nTextY = projMaxs.y + (HANDLE_RADIUS*3);
		nTextFlags |= CRender2D::TEXT_JUSTIFY_BOTTOM;
	}

    pRender->DrawText( extentText, nTextX, nTextY, nTextFlags );

    // vert
#ifdef SLE //// SLE CHANGE - display brush sizes in finer fractions
#ifdef SLE //// SLE CHANGE - show extents in chosen map units
	if ( pDoc )
	{
		switch ( Box3D::GetWorldUnits() )
		{
			case Units_Feet_Inches:
			{
				int nFeet = ( int )fabs(Extents[ pView->axVert ]) / 12;
				int nInches = ( int )fabs(Extents[ pView->axVert ]) % 12;

				sprintf(extentText, "%d' %d\"", nFeet, nInches);
				break;
			}

			case Units_Centimeters:
			{
				float fCM = ( int )fabs(Extents[ pView->axVert ]) * 2.54;

				sprintf(extentText, "%.3f cm", fCM);
				break;
			}

			case Units_Meters_Centimeters:
			{
				float fMCM = ( int )fabs(Extents[ pView->axVert ]) * 0.0254;

				sprintf(extentText, " %.4f m", fMCM);
				break;
			}

			default:
			{
				sprintf(extentText, "%.3f", Extents[ pView->axVert ]);
				break;
			}
		}
	}
#endif
#else
    sprintf( extentText, "%.1f", Extents[pView->axVert] );
#endif
	nTextFlags = CRender2D::TEXT_JUSTIFY_VERT_CENTER;
	nTextY = projCenter.y;
	
	if ( nFlags & DBT_LEFT )
	{
		nTextX = projMins.x - (HANDLE_RADIUS*3);
		nTextFlags |= CRender2D::TEXT_JUSTIFY_LEFT;
	}
	else
	{
		nTextX = projMaxs.x + (HANDLE_RADIUS*3);
		nTextFlags |= CRender2D::TEXT_JUSTIFY_RIGHT;
	}
    
    pRender->DrawText( extentText, nTextX, nTextY, nTextFlags );
}
