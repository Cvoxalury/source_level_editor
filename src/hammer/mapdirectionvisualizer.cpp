//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "stdafx.h"
#include "mapdirectionvisualizer.h"
#include "fgdlib/helperinfo.h"
#include "options.h"
#include "render2d.h"
#include "render3d.h"
#include "camera.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_MAPCLASS(CMapDirectionVisualizer);

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a CMapDirectionVisualizer from a set
//			of string parameters from the FGD file.
// Input  : pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, nullptr if an error occurs.
//-----------------------------------------------------------------------------
CMapClass* CMapDirectionVisualizer::Create( CHelperInfo* pHelperInfo, CMapEntity* pParent )
{
	const char* pszKey = pHelperInfo->GetParameter( 0 );
	if ( pszKey == nullptr )
		return nullptr;

	CMapDirectionVisualizer* pBox = new CMapDirectionVisualizer( pszKey );
	if ( pHelperInfo->GetParameterCount() == 4 ) // user supplied color
	{
		pBox->r = V_atoi( pHelperInfo->GetParameter( 1 ) );
		pBox->g = V_atoi( pHelperInfo->GetParameter( 2 ) );
		pBox->b = V_atoi( pHelperInfo->GetParameter( 3 ) );
	}

	return pBox;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pfMins -
//			pfMaxs -
//-----------------------------------------------------------------------------
CMapDirectionVisualizer::CMapDirectionVisualizer()
{
	Initialize();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pszKey -
//-----------------------------------------------------------------------------
CMapDirectionVisualizer::CMapDirectionVisualizer( const char* pszKey )
{
	Initialize();
	strcpy( m_szKeyName, pszKey );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMapDirectionVisualizer::Initialize()
{
	m_Dir.Init( 1.f, 0.f, 0.f );
	m_szKeyName[0] = '\0';

	r = 25;
	g = 204;
	b = 204;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMapDirectionVisualizer::~CMapDirectionVisualizer()
{
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : bFullUpdate -
//-----------------------------------------------------------------------------
void CMapDirectionVisualizer::CalcBounds( BOOL bFullUpdate )
{
	// Calculate 3D culling box.
	Vector Mins = m_Origin + Vector( 2, 2, 2 );
	Vector Maxs = m_Origin + Vector( 2, 2, 2 );
	m_CullBox.SetBounds( Mins, Maxs );
	m_BoundingBox = m_CullBox;
	m_Render2DBox = m_CullBox;
}

//-----------------------------------------------------------------------------
// Purpose:
// Output :
//-----------------------------------------------------------------------------
CMapClass* CMapDirectionVisualizer::Copy( bool bUpdateDependencies )
{
	CMapDirectionVisualizer* pCopy = new CMapDirectionVisualizer;

	if ( pCopy != nullptr )
		pCopy->CopyFrom( this, bUpdateDependencies );

	return pCopy;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pObject -
// Output :
//-----------------------------------------------------------------------------
CMapClass* CMapDirectionVisualizer::CopyFrom( CMapClass* pObject, bool bUpdateDependencies )
{
	Assert( pObject->IsMapClass( MAPCLASS_TYPE( CMapDirectionVisualizer ) ) );
	CMapDirectionVisualizer* pFrom = (CMapDirectionVisualizer*)pObject;

	CMapClass::CopyFrom( pObject, bUpdateDependencies );

	strcpy( m_szKeyName, pFrom->m_szKeyName );
	m_Dir = pFrom->m_Dir;

	return this;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pRender -
//-----------------------------------------------------------------------------
void CMapDirectionVisualizer::Render2D( CRender2D* pRender )
{
	if ( GetSelectionState() == SELECT_NONE )
		return;

	pRender->PushRenderMode( RENDER_MODE_WIREFRAME );

	Vector center;
	GetParent()->GetOrigin( center );

	const float scale = 4.f; //( 1.f / pRender->GetCamera()->GetZoom() ) * 16.f;

	const float flLengthBase = 32.0f * scale;
	const float flLengthTip = 12.0f * scale;
	const float flRadiusBase = 2.2f * scale;
	const float flRadiusTip = 6.0f * scale;

	pRender->SetDrawColor( r, g, b );
	pRender->DrawArrow( center, m_Dir, flLengthBase, flLengthTip, flRadiusBase, flRadiusTip );

	pRender->PopRenderMode();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pRender -
//-----------------------------------------------------------------------------
void CMapDirectionVisualizer::Render3D( CRender3D* pRender )
{
	if ( GetSelectionState() == SELECT_NONE )
		return;

	const bool change = pRender->GetCurrentRenderMode() >= RENDER_MODE_TEXTURED;
	if ( change )
		pRender->PushRenderMode( RENDER_MODE_FLAT );

	Vector center;
	GetParent()->GetOrigin( center );

	Vector camPos;
	pRender->GetCamera()->GetViewPoint( camPos );
	const float scale = camPos.DistTo( center ) / 256.f;

	const float flLengthBase = 32.0f * scale;
	const float flLengthTip = 12.0f * scale;
	const float flRadiusBase = 2.2f * scale;
	const float flRadiusTip = 6.0f * scale;

	pRender->SetDrawColor( r, g, b );
	pRender->DrawArrow( center, m_Dir, flLengthBase, flLengthTip, flRadiusBase, flRadiusTip );

	if ( change )
		pRender->PopRenderMode();
}

//-----------------------------------------------------------------------------
// Purpose: Overridden because origin helpers don't take the color of their
//			parent entity.
// Input  : red, green, blue -
//-----------------------------------------------------------------------------
void CMapDirectionVisualizer::SetRenderColor( unsigned char red, unsigned char green, unsigned char blue )
{
}

//-----------------------------------------------------------------------------
// Purpose: Overridden because origin helpers don't take the color of their
//			parent entity.
// Input  : red, green, blue -
//-----------------------------------------------------------------------------
void CMapDirectionVisualizer::SetRenderColor(color32 rgbColor)
{
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : szKey -
//			szValue -
//-----------------------------------------------------------------------------
void CMapDirectionVisualizer::OnParentKeyChanged( const char* szKey, const char* szValue )
{
	if ( stricmp( szKey, m_szKeyName ) == 0 )
	{
		QAngle dir;
		sscanf( szValue, "%f %f %f", &dir.x, &dir.y, &dir.z );
		AngleVectors( dir, &m_Dir );
		CalcBounds();
	}
}
