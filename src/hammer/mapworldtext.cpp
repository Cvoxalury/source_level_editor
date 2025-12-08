//===== Copyright Valve Corporation, All rights reserved. ======//

#include "stdafx.h"
#include "hammer_mathlib.h"
#include "box3d.h"
#include "bspfile.h"
#include "const.h"
#include "mapdefs.h"		// dvs: For COORD_NOTINIT
#include "mapdoc.h"
#include "mapentity.h"
#include "mapworldtext.h"
#include "render2d.h"
#include "render3d.h"
#include "hammer.h"
#include "texturesystem.h"
#include "materialsystem/imesh.h"
#include "material.h"
#include "options.h"
#include "camera.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_MAPCLASS( CWorldTextHelper )

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a CWorldTextHelper from a set
//			of string parameters from the FGD file.
// Input  : *pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, NULL if an error occurs.
//-----------------------------------------------------------------------------
CMapClass* CWorldTextHelper::CreateWorldText( CHelperInfo* pHelperInfo, CMapEntity* pParent )
{
	const char* pMsg = pParent->GetKeyValue( "message" );

	CWorldTextHelper* pWorldText = new CWorldTextHelper;
	pWorldText->SetText( pMsg );
	pWorldText->SetRenderMode( kRenderTransAlpha );

	return pWorldText;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CWorldTextHelper::CWorldTextHelper() : m_eRenderMode( kRenderTransAlpha ), m_flTextSize( 10.f )
{
	m_RenderColor.r = 255;
	m_RenderColor.g = 255;
	m_RenderColor.b = 255;
	m_RenderColor.a = 255;

	Initialize();
}

//-----------------------------------------------------------------------------
// Sets the render mode
//-----------------------------------------------------------------------------

void CWorldTextHelper::SetRenderMode( int eRenderMode )
{
	m_eRenderMode = eRenderMode;
}

void CWorldTextHelper::ComputeCornerVertices( Vector* pVerts, float flBloat ) const
{
	Vector ViewForward( 1.0f, 0.0f, 0.0f );
	Vector ViewUp( 0.0f, 1.0f, 0.0f );
	Vector ViewRight( 0.0f, 0.0f, -1.0f );
	AngleVectors( m_Angles, &ViewForward, &ViewRight, &ViewUp );

	float flStrLength = m_pText.Length();
	flStrLength = Max( flStrLength, 1.0f );

	pVerts[0] = m_Origin - flBloat * ( ViewRight + ViewUp );
	pVerts[1] = pVerts[0] + ( m_flTextSize * ( 1.0f + ( flStrLength - 1.0f ) * 0.6f ) + 2.0f * flBloat ) * ViewRight;
	pVerts[2] = pVerts[1] + ( m_flTextSize + 2.0f * flBloat ) * ViewUp;
	pVerts[3] = pVerts[0] + ( m_flTextSize + 2.0f * flBloat ) * ViewUp;
}

inline Vector VectorMin( const Vector &a, const Vector &b )
{
	return Vector( fpmin(a.x, b.x), fpmin(a.y, b.y), fpmin(a.z, b.z) );
}

inline Vector VectorMax( const Vector &a, const Vector &b )
{
	return Vector( fpmax(a.x, b.x), fpmax(a.y, b.y), fpmax(a.z, b.z) );
}

//-----------------------------------------------------------------------------
// Purpose: Calculates our bounding box based on the sprite dimensions.
// Input  : bFullUpdate - Whether we should recalculate our childrens' bounds.
//-----------------------------------------------------------------------------
void CWorldTextHelper::CalcBounds( BOOL bFullUpdate )
{
	CMapClass::CalcBounds(bFullUpdate);

	Vector cornerVerts[4];
	ComputeCornerVertices( cornerVerts );

	Vector vMin = cornerVerts[0].Min( cornerVerts[1] ).Min( cornerVerts[2] ).Min( cornerVerts[3] );
	Vector vMax = cornerVerts[0].Max( cornerVerts[1] ).Max( cornerVerts[2] ).Max( cornerVerts[3] );
	m_CullBox.UpdateBounds( vMin, vMax );
	m_Render2DBox.UpdateBounds( vMin, vMax );
}

//-----------------------------------------------------------------------------
// Purpose: Returns a copy of this object.
// Output : Pointer to the new object.
//-----------------------------------------------------------------------------
CMapClass* CWorldTextHelper::Copy( bool bUpdateDependencies )
{
	CWorldTextHelper* pCopy = new CWorldTextHelper;

	if ( pCopy != NULL )
		pCopy->CopyFrom( this, bUpdateDependencies );

	return pCopy;
}

//-----------------------------------------------------------------------------
// Purpose: Turns this into a duplicate of the given object.
// Input  : pObject - Pointer to the object to copy from.
// Output : Returns a pointer to this object.
//-----------------------------------------------------------------------------
CMapClass* CWorldTextHelper::CopyFrom( CMapClass* pObject, bool bUpdateDependencies )
{
	CWorldTextHelper* pFrom = dynamic_cast<CWorldTextHelper*>( pObject );
	Assert( pFrom != NULL );

	if ( pFrom != NULL )
	{
		CMapClass::CopyFrom( pObject, bUpdateDependencies );

		m_Angles = pFrom->m_Angles;
		m_pText = pFrom->m_pText;
		SetRenderMode( pFrom->m_eRenderMode );
		m_RenderColor = pFrom->m_RenderColor;
	}

	return this;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWorldTextHelper::Initialize()
{
	SetText( "" );
	m_Angles.Init();

	//m_eRenderMode = kRenderNormal;

	m_RenderColor.r = 255;
	m_RenderColor.g = 255;
	m_RenderColor.b = 255;
}

#define CHAR_WIDTH 0.0625f // 1/16
#define CHAR_HEIGHT 0.0625f // 1/16
void CWorldTextHelper::Render3DText( CRender3D* pRender, const char* szText, const float flTextSize )
{
	if (Options.general.bShowEditorObjects)
		return;
	if ( !szText )
		return;
	int nNumChars = V_strlen( szText );
	if ( !nNumChars )
		return;

	Vector ViewForward( 1.0f, 0.0f, 0.0f );
	Vector ViewUp( 0.0f, 1.0f, 0.0f );
	Vector ViewRight( 0.0f, 0.0f, -1.0f );
	AngleVectors( m_Angles, &ViewForward, &ViewRight, &ViewUp );

	Vector vecStartPos;
	VectorCopy( m_Origin, vecStartPos );

	IMaterial* pDebugText = materials->FindMaterial("../../bin/level_editor/materials/editor/worldtext", TEXTURE_GROUP_OTHER, false );
	if (!pDebugText)
		return;
	else
	{
		pDebugText->AddRef();
	}

	pRender->PushRenderMode( RENDER_MODE_EXTERN );
	CMatRenderContextPtr pRenderContext( MaterialSystemInterface() );
	pRenderContext->Bind( pDebugText );

	IMesh* pMesh = pRenderContext->GetDynamicMesh();
	CMeshBuilder meshBuilder;

	float screenSize = flTextSize;

	meshBuilder.Begin( pMesh, MATERIAL_QUADS, nNumChars );

	for ( int i=0; i < nNumChars; i++ )
	{
		int nCharIdx = (int)( (char)*szText ) - 32;
		int nRow = nCharIdx / 16;
		int nCol = nCharIdx % 16;

		float flU = ( nCol * CHAR_WIDTH );
		float flV = ( nRow * CHAR_HEIGHT );

		flV += CHAR_HEIGHT;
		meshBuilder.Position3fv( vecStartPos.Base() );
		meshBuilder.TexCoord2f( 0, flU, flV );
		meshBuilder.Color4ub( m_RenderColor.r, m_RenderColor.g, m_RenderColor.b, 255 );
		meshBuilder.AdvanceVertex();

		vecStartPos += ( ViewUp * screenSize );
		flV -= CHAR_HEIGHT;
		meshBuilder.Position3fv( vecStartPos.Base() );
		meshBuilder.TexCoord2f( 0, flU, flV );
		meshBuilder.Color4ub( m_RenderColor.r, m_RenderColor.g, m_RenderColor.b, 255 );
		meshBuilder.AdvanceVertex();

		vecStartPos += ( ViewRight * screenSize );
		flU += CHAR_WIDTH;
		meshBuilder.Position3fv( vecStartPos.Base() );
		meshBuilder.TexCoord2f( 0, flU, flV );
		meshBuilder.Color4ub( m_RenderColor.r, m_RenderColor.g, m_RenderColor.b, 255 );
		meshBuilder.AdvanceVertex();

		vecStartPos -= ( ViewUp * screenSize );
		flV += CHAR_HEIGHT;
		meshBuilder.Position3fv( vecStartPos.Base() );
		meshBuilder.TexCoord2f( 0, flU, flV );
		meshBuilder.Color4ub( m_RenderColor.r, m_RenderColor.g, m_RenderColor.b, 255 );
		meshBuilder.AdvanceVertex();

		vecStartPos -= ( ViewRight * screenSize * 0.4f );

		szText++;
	}

	meshBuilder.End();
	pMesh->Draw();
	pRender->PopRenderMode();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pRender -
//-----------------------------------------------------------------------------
void CWorldTextHelper::Render3D( CRender3D* pRender )
{
	pRender->BeginRenderHitTarget( this );
	Render3DText( pRender, m_pText, m_flTextSize );
	pRender->EndRenderHitTarget();

	if ( GetSelectionState() != SELECT_NONE )
	{
		// Selection box
		pRender->PushRenderMode( RENDER_MODE_WIREFRAME );
		Vector cornerVerts[4];
		ComputeCornerVertices( cornerVerts, 0.2f );
		pRender->SetDrawColor( 255, 255, 0 );
		pRender->DrawLine( cornerVerts[0], cornerVerts[1] );
		pRender->DrawLine( cornerVerts[1], cornerVerts[2] );
		pRender->DrawLine( cornerVerts[2], cornerVerts[3] );
		pRender->DrawLine( cornerVerts[3], cornerVerts[0] );

		pRender->PopRenderMode();
	}
}

//-----------------------------------------------------------------------------
void CWorldTextHelper::DoTransform( const VMatrix& matrix )
{
	BaseClass::DoTransform( matrix );

	matrix3x4_t fCurrentMatrix, fMatrixNew;
	AngleMatrix( m_Angles, fCurrentMatrix );
	ConcatTransforms( matrix.As3x4(), fCurrentMatrix, fMatrixNew );
	MatrixAngles( fMatrixNew, m_Angles );

	//
	// Update the angles of our parent entity.
	//
	CMapEntity* pEntity = dynamic_cast<CMapEntity*>( m_pParent );
	if ( pEntity )
	{
		char szValue[80];
		sprintf( szValue, "%g %g %g", m_Angles[0], m_Angles[1], m_Angles[2] );
		pEntity->NotifyChildKeyChanged( this, "angles", szValue );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Notifies that this object's parent entity has had a key value change.
// Input  : szKey - The key that changed.
//			szValue - The new value of the key.
//-----------------------------------------------------------------------------
void CWorldTextHelper::OnParentKeyChanged( const char* szKey, const char* szValue )
{
	if ( !stricmp( szKey, "message" ) )
	{
		SetText( szValue );
		PostUpdate( Notify_Changed );
	}
	else if ( !stricmp( szKey, "angles" ) )
	{
		sscanf( szValue, "%f %f %f", &m_Angles[PITCH], &m_Angles[YAW], &m_Angles[ROLL] );
		PostUpdate( Notify_Changed );
	}
	else if ( !stricmp( szKey, "textsize" ) )
	{
		sscanf( szValue, "%f", &m_flTextSize );
		PostUpdate( Notify_Changed );
	}
	else if ( !stricmp( szKey, "color" ) )
	{
		int r = 0, g = 0, b = 0;
		sscanf( szValue, "%d %d %d", &r, &g, &b );
		m_RenderColor.r = r;
		m_RenderColor.g = g;
		m_RenderColor.b = b;

		PostUpdate( Notify_Changed );
	}
}

//-----------------------------------------------------------------------------
void CWorldTextHelper::Render2D( CRender2D* pRender )
{
	Vector vecMins;
	Vector vecMaxs;
	GetRender2DBox( vecMins, vecMaxs );

	Vector2D pt,pt2;
	pRender->TransformPoint( pt, vecMins );
	pRender->TransformPoint( pt2, vecMaxs );

	if ( !IsSelected() )
	{
		pRender->SetDrawColor( r, g, b );
		pRender->SetHandleColor( r, g, b );
	}
	else
	{
		pRender->SetDrawColor( GetRValue( Options.colors.clr2DSelection ), GetGValue( Options.colors.clr2DSelection ), GetBValue( Options.colors.clr2DSelection ) );
		pRender->SetHandleColor( GetRValue( Options.colors.clr2DSelection ), GetGValue( Options.colors.clr2DSelection ), GetBValue( Options.colors.clr2DSelection ) );
	}

	// Draw the bounding box.

	pRender->DrawBox( vecMins, vecMaxs );

	//
	// Draw center handle.
	//

	if ( pRender->IsActiveView() )
	{
		int sizex = abs( pt.x - pt2.x ) + 1;
		int sizey = abs( pt.y - pt2.y ) + 1;

		// dont draw handle if object is too small
		if ( sizex > 6 && sizey > 6 )
		{
			pRender->SetHandleStyle( HANDLE_RADIUS, CRender::HANDLE_CROSS );
			pRender->DrawHandle( ( vecMins + vecMaxs ) / 2 );
		}
	}
}

void CWorldTextHelper::SetText( const char* pNewText )
{
	m_pText = pNewText;
}
