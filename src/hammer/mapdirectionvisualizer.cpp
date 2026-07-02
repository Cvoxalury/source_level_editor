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
	if (pHelperInfo->GetParameterCount() > 1) // user supplied direction and length (distance)
	{
		strcpy(pBox->m_szDistKeyName, pHelperInfo->GetParameter(1));

		pBox->m_distance_value_float = V_atof(pHelperInfo->GetParameter(1));
	}
	if ( pHelperInfo->GetParameterCount() == 5 ) // user supplied color
	{
		pBox->r = V_atoi( pHelperInfo->GetParameter( 2 ) );
		pBox->g = V_atoi( pHelperInfo->GetParameter( 3 ) );
		pBox->b = V_atoi( pHelperInfo->GetParameter( 4 ) );
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
	strcpy( m_szDirKeyName, pszKey );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMapDirectionVisualizer::Initialize()
{
	m_Dir.Init( 1.f, 0.f, 0.f );
	m_szDirKeyName[0] = '\0';
	m_szDistKeyName[0] = '\0';

	r = 25;
	g = 204;
	b = 204;

	m_distance_value_float = 0;
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

	strcpy( m_szDirKeyName, pFrom->m_szDirKeyName );
	strcpy(m_szDistKeyName, pFrom->m_szDistKeyName);
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

	const float scale = 1.f; //( 1.f / pRender->GetCamera()->GetZoom() ) * 16.f;

	const float flLengthBase = (m_distance_value_float > 0 ? m_distance_value_float - 12 : 32.0f * scale); // 12 is from the tip length
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
	const float scale = min(2.0f, max(camPos.DistTo( center ) / 256.f, 0.5f));

	const float flLengthBase = (m_distance_value_float > 0 ? m_distance_value_float - 12 : 32.0f * scale); // 12 is from the tip length
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
	QAngle dir;
	if ( stricmp( szKey, m_szDirKeyName ) == 0 )
	{
		sscanf(szValue, "%f %f %f", &dir.x, &dir.y, &dir.z);

		if ( m_pitch_is_set_bool )
		{
			dir.x = m_pitch_value_float;
		}

		AngleVectors( dir, &m_Dir );
	//	CalcBounds();
	}

	// movelinears have the arrow length show the move distance
	if (m_szDirKeyName[0] && stricmp(szKey, m_szDistKeyName) == 0)
	{
		sscanf(szValue, "%f", &m_distance_value_float);
	}
	
	//// special case for lights with separate Pitch value
	else if ( stricmp(szKey, "pitch") == 0 )
	{
		VectorAngles(m_Dir, dir); // remember the non-pitch angles, else they're reset to 0
		sscanf(szValue, "%f", &dir.x);
		dir.x *= -1; // pitch is designed to be negated
		m_pitch_value_float = dir.x;
		AngleVectors( dir, &m_Dir );
		m_pitch_is_set_bool = true;
	}
	CalcBounds();
}
