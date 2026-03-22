//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "stdafx.h"
#include "maprotationdistancevisualizer.h"
#include "Material.h"
#include "fgdlib/helperinfo.h"
#include "options.h"
#include "render2d.h"
#include "render3d.h"
#include "camera.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_MAPCLASS(CMapRotationDistanceVisualizer);

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a CMapRotationDistanceVisualizer from a set
//			of string parameters from the FGD file.
// Input  : pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, nullptr if an error occurs.
//-----------------------------------------------------------------------------
CMapClass* CMapRotationDistanceVisualizer::Create( CHelperInfo* pHelperInfo, CMapEntity* pParent )
{
	static char *pszDefaultOriginKeyName = "origin";
	static char *pszDefaultDirKeyName = "angles";
	static char *pszDefaultArcKeyName = "distance";

	const char *pszOriginKey, *pszDirKey, *pszArcKey;

	if ( pHelperInfo->GetParameterCount() == 0 ) // user haven't specified any - use the defaults
	{
		pszOriginKey = pszDefaultOriginKeyName;
		pszDirKey = pszDefaultDirKeyName;
		pszArcKey = pszDefaultArcKeyName;
	}
	else
	{
		pszOriginKey = pHelperInfo->GetParameter(0);
		if ( pszOriginKey == NULL )
			return NULL;

		pszDirKey = pHelperInfo->GetParameter(1);
		if ( pszDirKey == NULL )
			return NULL;

		pszArcKey = pHelperInfo->GetParameter(2);
		if ( pszArcKey == NULL )
			return NULL;
	}

	CMapRotationDistanceVisualizer* pBox = new CMapRotationDistanceVisualizer( pszOriginKey, pszDirKey, pszArcKey );
	if ( pHelperInfo->GetParameterCount() == 6 ) // user supplied color
	{
		pBox->r = V_atoi( pHelperInfo->GetParameter( 3 ) );
		pBox->g = V_atoi( pHelperInfo->GetParameter( 4 ) );
		pBox->b = V_atoi( pHelperInfo->GetParameter( 5 ) );
	}

	return pBox;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pfMins -
//			pfMaxs -
//-----------------------------------------------------------------------------
CMapRotationDistanceVisualizer::CMapRotationDistanceVisualizer()
{
	Initialize();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pszKey -
//-----------------------------------------------------------------------------
CMapRotationDistanceVisualizer::CMapRotationDistanceVisualizer( const char *pszOriginKey, const char* pszDirKey, const char *pszArcKey )
{
	Initialize();
	strcpy( m_szOriginKeyName, pszOriginKey);
	strcpy( m_szDirKeyName, pszDirKey );
	strcpy( m_szArcKeyName, pszArcKey );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMapRotationDistanceVisualizer::Initialize()
{
	m_dir_vector.Init( 1.f, 0.f, 0.f );
	m_arc_float = 90.0f;
	m_szOriginKeyName[0] = '\0';
	m_szDirKeyName[0] = '\0';
	m_szArcKeyName[0] = '\0';

	r = 220;
	g = 255;
	b = 128;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMapRotationDistanceVisualizer::~CMapRotationDistanceVisualizer()
{
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : bFullUpdate -
//-----------------------------------------------------------------------------
void CMapRotationDistanceVisualizer::CalcBounds( BOOL bFullUpdate )
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
CMapClass* CMapRotationDistanceVisualizer::Copy( bool bUpdateDependencies )
{
	CMapRotationDistanceVisualizer* pCopy = new CMapRotationDistanceVisualizer;

	if ( pCopy != nullptr )
		pCopy->CopyFrom( this, bUpdateDependencies );

	return pCopy;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pObject -
// Output :
//-----------------------------------------------------------------------------
CMapClass* CMapRotationDistanceVisualizer::CopyFrom( CMapClass* pObject, bool bUpdateDependencies )
{
	Assert( pObject->IsMapClass( MAPCLASS_TYPE( CMapRotationDistanceVisualizer ) ) );
	CMapRotationDistanceVisualizer* pFrom = (CMapRotationDistanceVisualizer*)pObject;

	CMapClass::CopyFrom( pObject, bUpdateDependencies );

	strcpy( m_szOriginKeyName, pFrom->m_szOriginKeyName );
	strcpy( m_szDirKeyName, pFrom->m_szDirKeyName );
	strcpy( m_szArcKeyName, pFrom->m_szArcKeyName );
	m_Origin = pFrom->m_Origin;
	m_dir_vector = pFrom->m_dir_vector;
	m_arc_float = pFrom->m_arc_float;

	return this;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pRender -
//-----------------------------------------------------------------------------
void CMapRotationDistanceVisualizer::Render2D( CRender2D* pRender )
{
	if ( GetSelectionState() == SELECT_NONE )
		return;

	pRender->PushRenderMode( RENDER_MODE_WIREFRAME_NOZ );
	
	const float scale = 1.f; //( 1.f / pRender->GetCamera()->GetZoom() ) * 16.f;
	const float lineLength = 32.0f * scale;
		
	// draw the first line
	pRender->SetDrawColor( r, g, b );
	pRender->DrawLine( m_Origin, m_Origin + m_dir_vector * lineLength );

	// rotate the direction by the arc angle and draw a new line
	Vector dir = m_dir_vector;
	QAngle arcAng = QAngle(0, m_arc_float, 0);
	AngleVectors( arcAng, &dir );
	pRender->DrawLine( m_Origin, m_Origin + dir * lineLength );
	// draw the reverse for the other side
//	AngleVectors( -arcAng, &dir );
//	pRender->DrawLine( m_Origin, m_Origin + dir * lineLength );

	// calculate the arc
	int numpoints = 8;	
	Vector arcPointsInner[ 16 ]; // arc + the root
	Vector arcPointsOuter[ 16 ]; // arc + the root

//	Vector arcPointsInnerR[ 16 ]; // reverse foor the other side of the door
//	Vector arcPointsOuterR[ 16 ]; // ditto

	arcPointsInner[0] = arcPointsOuter[0] = m_Origin; // the root
//	arcPointsInnerR[0] = arcPointsOuterR[0] = m_Origin;
	
	float arcAngle = 0;
	float arcDelta = m_arc_float / (float)numpoints;

	if( arcAngle < 360.0f) ++numpoints;

	for ( int i = 1; i <= numpoints; i++ )
	{
		arcPointsInner[ i ][ 0 ] = (m_Origin.x + ( float )cos(DEG2RAD(arcAngle)) * (lineLength / 2));
		arcPointsInner[ i ][ 1 ] = (m_Origin.y + ( float )sin(DEG2RAD(arcAngle)) * (lineLength / 2));

//		arcPointsInnerR[ i ][ 0 ] = (m_Origin.x + ( float )cos(DEG2RAD(-arcAngle)) * (lineLength / 2));
//		arcPointsInnerR[ i ][ 1 ] = (m_Origin.y + ( float )sin(DEG2RAD(-arcAngle)) * (lineLength / 2));

		arcPointsOuter[ i ][ 0 ] = (m_Origin.x + ( float )cos(DEG2RAD(arcAngle)) * lineLength);
		arcPointsOuter[ i ][ 1 ] = (m_Origin.y + ( float )sin(DEG2RAD(arcAngle)) * lineLength);

//		arcPointsOuterR[ i ][ 0 ] = (m_Origin.x + ( float )cos(DEG2RAD(-arcAngle)) * lineLength);
//		arcPointsOuterR[ i ][ 1 ] = (m_Origin.y + ( float )sin(DEG2RAD(-arcAngle)) * lineLength);

		arcPointsInner[i][2] = arcPointsOuter[i][2] = m_Origin.z; // make it flat, flush with the origin

	//	pRender->DrawHandle(arcPointsOuter[ i ]);

		arcAngle += arcDelta; // start at the max angle to which the door opens, and go back

		if ( i > 1 )
		{
			pRender->DrawLine(arcPointsInner[ i ], arcPointsInner[ i - 1 ]);
			pRender->DrawLine(arcPointsOuter[ i ], arcPointsOuter[ i - 1 ]);

//			pRender->DrawLine(arcPointsInnerR[ i ], arcPointsInnerR[ i - 1 ]);
//			pRender->DrawLine(arcPointsOuterR[ i ], arcPointsOuterR[ i - 1 ]);
		}
	}

	pRender->PopRenderMode();
	
	// draw the filled sector
	pRender->PushRenderMode( RENDER_MODE_FLAT_NOCULL );
	pRender->DrawPolyLineFilled(numpoints + 1, arcPointsOuter, 0.125f);
//	pRender->DrawPolyLineFilled(numpoints + 1, arcPointsOuterR, 0.125f);
	pRender->PopRenderMode();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pRender -
//-----------------------------------------------------------------------------
void CMapRotationDistanceVisualizer::Render3D( CRender3D* pRender )
{
	if ( GetSelectionState() == SELECT_NONE )
		return;
	
	pRender->PushRenderMode( RENDER_MODE_WIREFRAME_NOZ );
	
	const float scale = 1.0f;

	const float lineLength = 32.0f * scale;
	
	// draw the first line
	pRender->SetDrawColor( r, g, b );
	if( m_x_axis_bool) pRender->SetDrawColor( 155, 50, 155);
	if( m_y_axis_bool) pRender->SetDrawColor( 50, 155, 255);
	pRender->DrawLine( m_Origin, m_Origin + m_dir_vector * lineLength );

	// rotate the direction by the arc angle and draw a new line
	Vector dir = m_dir_vector;
	QAngle arcAng = QAngle(0, m_arc_float, 0);
	AngleVectors( arcAng, &dir );
	pRender->DrawLine( m_Origin, m_Origin + dir * lineLength );
	// draw the reverse for the other side
//	AngleVectors( -arcAng, &dir );
//	pRender->DrawLine( m_Origin, m_Origin + dir * lineLength );
	
	// calculate the arc
	int numpoints = 8;	
	Vector arcPointsInner[ 16 ]; // arc + the root
	Vector arcPointsOuter[ 16 ]; // arc + the root

//	Vector arcPointsInnerR[ 16 ]; // reverse foor the other side of the door
//	Vector arcPointsOuterR[ 16 ]; // ditto

	arcPointsInner[0] = arcPointsOuter[0] = m_Origin; // the root
//	arcPointsInnerR[0] = arcPointsOuterR[0] = m_Origin;
	
	float arcAngle = 0;
	float arcDelta = m_arc_float / (float)numpoints;

	if( arcAngle < 360.0f) ++numpoints;

	for ( int i = 1; i <= numpoints; i++ )
	{
		arcPointsInner[ i ][ 0 ] = (m_Origin.x + ( float )cos(DEG2RAD(arcAngle)) * (lineLength / 2));
		arcPointsInner[ i ][ 1 ] = (m_Origin.y + ( float )sin(DEG2RAD(arcAngle)) * (lineLength / 2));

	//	arcPointsInnerR[ i ][ 0 ] = (m_Origin.x + ( float )cos(DEG2RAD(-arcAngle)) * (lineLength / 2));
	//	arcPointsInnerR[ i ][ 1 ] = (m_Origin.y + ( float )sin(DEG2RAD(-arcAngle)) * (lineLength / 2));

		arcPointsOuter[ i ][ 0 ] = (m_Origin.x + ( float )cos(DEG2RAD(arcAngle)) * lineLength);
		arcPointsOuter[ i ][ 1 ] = (m_Origin.y + ( float )sin(DEG2RAD(arcAngle)) * lineLength);

	//	arcPointsOuterR[ i ][ 0 ] = (m_Origin.x + ( float )cos(DEG2RAD(-arcAngle)) * lineLength);
	//	arcPointsOuterR[ i ][ 1 ] = (m_Origin.y + ( float )sin(DEG2RAD(-arcAngle)) * lineLength);

		arcPointsInner[i][2] = arcPointsOuter[i][2] = m_Origin.z; // make it flat, flush with the origin
	//	arcPointsInnerR[i][2] = arcPointsOuterR[i][2] = m_Origin.z;

	//	pRender->DrawHandle(arcPointsOuter[ i ]);

		arcAngle += arcDelta; // start at the max angle to which the door opens, and go back

		if ( i > 1 )
		{
			pRender->DrawLine(arcPointsInner[ i ], arcPointsInner[ i - 1 ]);
			pRender->DrawLine(arcPointsOuter[ i ], arcPointsOuter[ i - 1 ]);

		//	pRender->DrawLine(arcPointsInnerR[ i ], arcPointsInnerR[ i - 1 ]);
		//	pRender->DrawLine(arcPointsOuterR[ i ], arcPointsOuterR[ i - 1 ]);
		}
	}

	pRender->PopRenderMode();
	
	// draw the filled sector
	/*
	IMaterial *material;
	CMatRenderContextPtr pRenderContext(materials);
	material = MaterialSystemInterface()->FindMaterial("debug/debugtranslucentvertexcolor", TEXTURE_GROUP_OTHER);
	if (!material)
	{
		return;
	}
	material->AlphaModulate(64);
	material->SetMaterialVarFlag((MaterialVarFlags_t)MATERIAL_VAR_IGNOREZ, true);
	material->SetMaterialVarFlag((MaterialVarFlags_t)MATERIAL_VAR_NOCULL, true);

	material->RecomputeStateSnapshots();
	material->AddRef();
	pRender->BindMaterial(material);
	*/
	pRender->PushRenderMode( RENDER_MODE_FLAT_NOCULL );
	pRender->DrawPolyLineFilled(numpoints + 1, arcPointsOuter, 0.125f);
//	pRender->DrawPolyLineFilled(numpoints + 1, arcPointsOuterR, 0.125f);
	pRender->PopRenderMode();
}

//-----------------------------------------------------------------------------
// Purpose: Overridden because origin helpers don't take the color of their
//			parent entity.
// Input  : red, green, blue -
//-----------------------------------------------------------------------------
void CMapRotationDistanceVisualizer::SetRenderColor( unsigned char red, unsigned char green, unsigned char blue )
{
}

//-----------------------------------------------------------------------------
// Purpose: Overridden because origin helpers don't take the color of their
//			parent entity.
// Input  : red, green, blue -
//-----------------------------------------------------------------------------
void CMapRotationDistanceVisualizer::SetRenderColor(color32 rgbColor)
{
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : szKey -
//			szValue -
//-----------------------------------------------------------------------------
void CMapRotationDistanceVisualizer::OnParentKeyChanged( const char* szKey, const char* szValue )
{
	if( stricmp (szKey, m_szOriginKeyName) == 0)
	{
		sscanf( szValue, "%f %f %f", &m_Origin.x, &m_Origin.y, &m_Origin.z );
		//	CalcBounds();
		PostUpdate(Notify_Changed);
	}

	if ( stricmp( szKey, m_szArcKeyName ) == 0 )
	{
		sscanf( szValue, "%f", &m_arc_float);
		//	CalcBounds();
		PostUpdate(Notify_Changed);
	}

	if ( stricmp( szKey, m_szDirKeyName ) == 0 )
	{
		QAngle dirAng;
		sscanf( szValue, "%f %f %f", &dirAng.x, &dirAng.y, &dirAng.z );
		// hack - doors start perpendicular to their angles, so rotate it by 90 deg
		//dirAng.y += 90;
		AngleVectors( dirAng, &m_dir_vector );
		//	CalcBounds();
		PostUpdate(Notify_Changed);
	}
	
	// Check for door spawnflags
	// todo - doesn't detect them being changed on the flags tab, only via smart edit
	// todo - need to add fgd control to specify which spawnflag is X/Y, as it is 
	// different between entities
	if ( stricmp( szKey, "spawnflags" ) == 0 )
	{
		// 64 = X Axis
		// 128 = Y Axis
		int flags;
		sscanf_s( szValue, "%i", &flags );
		if ( flags & 64 )
		{
			m_x_axis_bool = true;	
		}
		else
		{
			m_x_axis_bool = false;
		}
		if ( flags & 128 )
		{
			m_y_axis_bool = true;
		}
		else
		{
			m_y_axis_bool = false;
		}
	}
}
