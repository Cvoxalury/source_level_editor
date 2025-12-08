
#include "stdafx.h"
#include "hammer_mathlib.h"
#include "box3d.h"
#include "bspfile.h"
#include "const.h"
#include "KeyValues.h"
#include "mapdefs.h"		// dvs: For COORD_NOTINIT
#include "mapdoc.h"
#include "mapentity.h"
#include "mapspotlight.h"
#include "render2d.h"
#include "render3d.h"
#include "hammer.h"
#include "texturesystem.h"
#include "materialsystem/imesh.h"
#include "material.h"
#include "options.h"
#include "camera.h"
#include "sprite.h"
#include "mapsprite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_MAPCLASS( CSpotlightHelper )

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a CSpotlightHelper from a set
//			of string parameters from the FGD file.
// Input  : *pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, NULL if an error occurs.
//-----------------------------------------------------------------------------
CMapClass* CSpotlightHelper::CreateSpotlight( CHelperInfo* pHelperInfo, CMapEntity* pParent )
{
	const char* key = pParent->GetKeyValue( "spotlightwidth" );

	CSpotlightHelper* spotlight = new CSpotlightHelper;
	spotlight->m_beamwidth_float = atof(key);
	key = pParent->GetKeyValue("spotlightlength");
	spotlight->m_beamlength_float = atof(key);
	key = pParent->GetKeyValue("spotlighttexture");
	if( !key)
		key = pParent->GetKeyValue("spotlightmaterial"); // mapbase uses SpotlightMaterial
	if (!key) // still nothing? default to sprites/glow_test02.vmt like the game does
		key = "sprites/glow_test02.vmt";
	spotlight->m_beamtexturename_string = key;
	int r = 0, g = 0, b = 0;
	sscanf(pParent->GetKeyValue("rendercolor"), "%d %d %d", &r, &g, &b);
	spotlight->m_RenderColor.r = r;
	spotlight->m_RenderColor.g = g;
	spotlight->m_RenderColor.b = b;
	
	return spotlight;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CSpotlightHelper::CSpotlightHelper() : m_eRenderMode(kRenderTransAlpha)
{
	m_RenderColor.r = 255;
	m_RenderColor.g = 255;
	m_RenderColor.b = 255;
	m_RenderColor.a = 255;
	m_beamlength_float = 0.0f;
	m_beamwidth_float = 0.0f;
	m_beamtexturename_string = "sprites/glow_test02.vmt"; // default to this like the games do
	Initialize();
}

// Copied from Sprite.cpp, see comments there.
void CSpotlightHelper::GetSpriteAxes(const QAngle& Angles, int type, Vector& forward, 
	Vector& right, Vector& up, const Vector& ViewUp, const Vector& ViewRight, const Vector& ViewForward)
{
	int				i;
	float			dot, angle, sr, cr;
	Vector			tvec;

	if (Angles[2] != 0 && type == SPR_VP_PARALLEL)
	{
		type = SPR_VP_PARALLEL_ORIENTED;
	}

	switch (type)
	{
	case SPR_FACING_UPRIGHT:
	{
		tvec[0] = -m_Origin[0];
		tvec[1] = -m_Origin[1];
		tvec[2] = -m_Origin[2];
		VectorNormalize(tvec);
		dot = tvec[2];
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;
		up[0] = 0;
		up[1] = 0;
		up[2] = 1;
		right[0] = tvec[1];

		right[1] = -tvec[0];

		right[2] = 0;
		VectorNormalize(right);
		forward[0] = -right[1];
		forward[1] = right[0];
		forward[2] = 0;

		break;
	}

	case SPR_VP_PARALLEL:
	{
		for (i = 0; i<3; i++)
		{
			up[i] = ViewUp[i];
			right[i] = ViewRight[i];
			forward[i] = ViewForward[i];
		}
		break;
	}

	case SPR_VP_PARALLEL_UPRIGHT:
	{
		dot = ViewForward[2];
		if ((dot > 0.999848) || (dot < -0.999848))	// cos(1 degree) = 0.999848
			return;

		up[0] = 0;
		up[1] = 0;
		up[2] = 1;

		right[0] = ViewForward[1];
		right[1] = -ViewForward[0];
		right[2] = 0;
		VectorNormalize(right);

		forward[0] = -right[1];
		forward[1] = right[0];
		forward[2] = 0;
		break;
	}

	case SPR_ORIENTED:
	{
		AngleVectors(Angles, &forward, &right, &up);
		break;
	}

	case SPR_VP_PARALLEL_ORIENTED:
	{
		angle = Angles[ROLL] * (M_PI * 2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i = 0; i<3; i++)
		{
			forward[i] = ViewForward[i];
			right[i] = ViewRight[i] * cr + ViewUp[i] * sr;
			up[i] = ViewRight[i] * -sr + ViewUp[i] * cr;
		}
		break;
	}

	default:
	{
		break;
	}
	}
}

void CSpotlightHelper::ComputeCornerVertices( Vector* pVerts, float length, float width) const
{
	Vector ViewForward( 1.0f, 0.0f, 0.0f );
	Vector ViewUp( 0.0f, 1.0f, 0.0f );
	Vector ViewRight( 0.0f, 0.0f, -1.0f );
	AngleVectors( m_Angles, &ViewForward, &ViewRight, &ViewUp );
	
	pVerts[0] = m_Origin - ViewUp * (width / 2);
	pVerts[1] = pVerts[0] + ViewUp * width;
	pVerts[2] = pVerts[1] + ViewForward * length;
	pVerts[3] = pVerts[0] + ViewForward * length;
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
void CSpotlightHelper::CalcBounds( BOOL bFullUpdate )
{
	CMapClass::CalcBounds(bFullUpdate);
	Vector cornerVerts[4];
	ComputeCornerVertices( cornerVerts, m_beamlength_float, m_beamwidth_float * 2);

	Vector vMin = cornerVerts[0].Min( cornerVerts[1] ).Min( cornerVerts[2] ).Min( cornerVerts[3] );
	Vector vMax = cornerVerts[0].Max( cornerVerts[1] ).Max( cornerVerts[2] ).Max( cornerVerts[3] );
	m_CullBox.UpdateBounds( vMin, vMax );
//	m_Render2DBox.UpdateBounds( vMin, vMax );
}

//-----------------------------------------------------------------------------
// Purpose: Returns a copy of this object.
// Output : Pointer to the new object.
//-----------------------------------------------------------------------------
CMapClass* CSpotlightHelper::Copy( bool bUpdateDependencies )
{
	CSpotlightHelper* pCopy = new CSpotlightHelper;

	if ( pCopy != NULL )
		pCopy->CopyFrom( this, bUpdateDependencies );

	return pCopy;
}

//-----------------------------------------------------------------------------
// Purpose: Turns this into a duplicate of the given object.
// Input  : pObject - Pointer to the object to copy from.
// Output : Returns a pointer to this object.
//-----------------------------------------------------------------------------
CMapClass* CSpotlightHelper::CopyFrom( CMapClass* pObject, bool bUpdateDependencies )
{
	CSpotlightHelper* pFrom = dynamic_cast<CSpotlightHelper*>( pObject );
	Assert( pFrom != NULL );

	if ( pFrom != NULL )
	{
		CMapClass::CopyFrom( pObject, bUpdateDependencies );

		m_Angles = pFrom->m_Angles;
		m_RenderColor = pFrom->m_RenderColor;
		SetRenderColor(m_RenderColor.r, m_RenderColor.g, m_RenderColor.b);
	}

	return this;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CSpotlightHelper::Initialize()
{
	m_Angles.Init();

	//m_eRenderMode = kRenderNormal;

	m_RenderColor.r = 255;
	m_RenderColor.g = 255;
	m_RenderColor.b = 255;
	m_RenderColor.a = 64;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pRender -
//-----------------------------------------------------------------------------
void CSpotlightHelper::Render3D( CRender3D* pRender )
{
	if (!GetParent()) return;

	Vector cornerVerts[4];
	ComputeCornerVertices(cornerVerts, m_beamlength_float, m_beamwidth_float * 2); // multiply width by 2 to emulate the game
		
	IMaterial *material;
	CMeshBuilder meshBuilder;

	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->MatrixMode(MATERIAL_MODEL);
	pRenderContext->PushMatrix();

	switch (pRender->GetCurrentRenderMode())
	{
	case RENDER_MODE_WIREFRAME: pRender->PushRenderMode(RENDER_MODE_WIREFRAME_NOZ);
		break;
	case RENDER_MODE_LIGHTMAP_GRID: /*pRender->PushRenderMode(RENDER_MODE_WIREFRAME_NOZ)*/ return; // don't draw these in flat mode
		break;
	case RENDER_MODE_FLAT: /*pRender->PushRenderMode(RENDER_MODE_FLAT_NOZ)*/ return; // don't draw these in flat mode
		break;
	default: pRender->PushRenderMode(RENDER_MODE_TEXTURED);
		break;
	}

	material = MaterialSystemInterface()->FindMaterial(m_beamtexturename_string, TEXTURE_GROUP_OTHER);
	if (material)
	{
		KeyValues *pVMTKeyValues = new KeyValues("Unlitgeneric");
		pVMTKeyValues->SetString("$basetexture", m_beamtexturename_string);
		char buf[16];
		sprintf(buf, "{%i %i %i}", m_RenderColor.r, m_RenderColor.g, m_RenderColor.b);
		pVMTKeyValues->SetString("$color", buf);
		material->SetShaderAndParams(pVMTKeyValues);
	}
	else
	{
		pRender->PopRenderMode();
		pRenderContext->PopMatrix();
		return;
	}
	material->AlphaModulate(64);
	material->SetMaterialVarFlag((MaterialVarFlags_t)MATERIAL_VAR_ADDITIVE, true);
	material->SetMaterialVarFlag((MaterialVarFlags_t)MATERIAL_VAR_NOCULL, true);
	m_eRenderMode = kRenderWorldGlow;
	material->RecomputeStateSnapshots();
	material->AddRef();
	pRender->BindMaterial(material);

	m_eRenderMode = kRenderTransAdd;

	Vector origin;
//	GetParent()->GetOrigin(origin);
	origin = cornerVerts[0];
	Vector ViewUp;
	Vector ViewRight;
	Vector ViewForward;
	pRender->GetViewUp(ViewUp);
	pRender->GetViewRight(ViewRight);
	pRender->GetViewForward(ViewForward);
	/*
	Vector corner, spritex, spritey, spritez;
	GetSpriteAxes(m_Angles, SPR_VP_PARALLEL, spritez, spritex, spritey, ViewUp, ViewRight, ViewForward);

	m_UL.y = origin[1];
	m_LR.y = origin[1] - m_beamlength_float;
	m_UL.x = origin[0];
	m_LR.x = m_beamwidth_float + origin[0];

	Vector2D ul, lr;
	Vector2DMultiply(m_UL, 0.1f, ul);
	Vector2DMultiply(m_LR, 0.1f, lr);

	VectorMA(origin, ul.x, spritex, corner);
	VectorMA(corner, lr.y, spritey, corner);
	spritex *= (lr.x - ul.x);
	spritey *= (ul.y - lr.y);

//	Vector2D texul, texlr;
//	texul.x = m_TexUL.x;
//	texul.y = m_TexLR.y : m_TexUL.y;
//	texlr.x = m_TexLR.x;
//	texlr.y = m_TexUL.y : m_TexLR.y;
	*/

	IMesh* pMesh = pRenderContext->GetDynamicMesh();
	meshBuilder.Begin(pMesh, MATERIAL_POLYGON, 4);

	meshBuilder.Position3f(cornerVerts[0].x, cornerVerts[0].y, cornerVerts[0].z);
	meshBuilder.TexCoord2f(0, 0, 0);
	meshBuilder.Color4ub((int)(m_RenderColor.r * (64 / 255)), (int)(m_RenderColor.g * (64 / 255)), (int)(m_RenderColor.b * (64 / 255)), m_RenderColor.a); // in-game spotlights set brightness to 64, emulate it
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f(cornerVerts[1].x, cornerVerts[1].y, cornerVerts[1].z);
	meshBuilder.TexCoord2f(0, 1, 0);
	meshBuilder.Color4ub((int)(m_RenderColor.r * (64/255)), (int)(m_RenderColor.g * (64 / 255)), (int)(m_RenderColor.b * (64 / 255)), m_RenderColor.a);
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f(cornerVerts[2].x, cornerVerts[2].y, cornerVerts[2].z);
	meshBuilder.TexCoord2f(0, 1, 1);
	meshBuilder.Color4ub(0, 0, 0, 0); // in-game spotlights have 'shade out', emulate it
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f(cornerVerts[3].x, cornerVerts[3].y, cornerVerts[3].z);
	meshBuilder.TexCoord2f(0, 0, 1);
	meshBuilder.Color4ub(0, 0, 0, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
	
	pRender->PopRenderMode();
	pRenderContext->PopMatrix();
	if (GetSelectionState() != SELECT_NONE && Options.general.bShowHelpers)
	{
		// Draw an upside-down T-like shape to show the length and end width
		pRender->PushRenderMode(RENDER_MODE_WIREFRAME);
		ComputeCornerVertices(cornerVerts, m_beamlength_float, m_beamwidth_float * 2); // full length for showing the size
		pRender->SetDrawColor(255, 255, 0);
		pRender->DrawLine((cornerVerts[1] + cornerVerts[0]) / 2, (cornerVerts[3] + cornerVerts[2]) / 2);
		pRender->DrawLine(cornerVerts[2], cornerVerts[3]);
		pRender->PopRenderMode();
	}
}

//-----------------------------------------------------------------------------
void CSpotlightHelper::DoTransform( const VMatrix& matrix )
{
	BaseClass::DoTransform( matrix );
}

//-----------------------------------------------------------------------------
// Purpose: Notifies that this object's parent entity has had a key value change.
// Input  : szKey - The key that changed.
//			szValue - The new value of the key.
//-----------------------------------------------------------------------------
void CSpotlightHelper::OnParentKeyChanged( const char* szKey, const char* szValue )
{
	if (!stricmp(szKey, "spotlightwidth"))
	{
		m_beamwidth_float = atof(szValue);
	//	CalcBounds(true);
		PostUpdate(Notify_Changed);
	}
	if (!stricmp(szKey, "spotlightlength"))
	{
		m_beamlength_float = atof(szValue);
	//	CalcBounds(true);
		PostUpdate(Notify_Changed);
	}
	if (!stricmp(szKey, "spotlighttexture"))
	{
		m_beamtexturename_string = szValue;
		PostUpdate(Notify_Changed);
	}
	if ( !stricmp(szKey, "spotlightmaterial") ) // mapbase uses SpotlightMaterial
	{
		m_beamtexturename_string = szValue;
		PostUpdate(Notify_Changed);
	}
	if ( m_beamtexturename_string.Length() == 0 )
	{
		// default to sprites/glow_test02.vmt like the game does
		m_beamtexturename_string = "sprites/glow_test02.vmt";
	}
	else if (!stricmp(szKey, "angles"))
	{
		sscanf(szValue, "%f %f %f", &m_Angles[PITCH], &m_Angles[YAW], &m_Angles[ROLL]);
	//	CalcBounds(true);
		PostUpdate(Notify_Changed);
	}
	else if (!stricmp(szKey, "rendercolor"))
	{
		int r = 0, g = 0, b = 0;
		sscanf(szValue, "%d %d %d", &r, &g, &b);
		m_RenderColor.r = r;
		m_RenderColor.g = g;
		m_RenderColor.b = b;

		SetRenderColor(r, g, b);

		PostUpdate(Notify_Changed);
	}
}

//-----------------------------------------------------------------------------
void CSpotlightHelper::Render2D( CRender2D* pRender )
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
