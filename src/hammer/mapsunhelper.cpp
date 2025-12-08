// under construction

#include "stdafx.h"
#include "hammer_mathlib.h"
#include "box3d.h"
#include "bspfile.h"
#include "const.h"
#include "KeyValues.h"
#include "mapdefs.h"		// dvs: For COORD_NOTINIT
#include "mapdoc.h"
#include "mapentity.h"
#include "mapsunhelper.h"
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

IMPLEMENT_MAPCLASS(CSunHelper)

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a CSunHelper from a set
//			of string parameters from the FGD file.
// Input  : *pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, NULL if an error occurs.
//-----------------------------------------------------------------------------
CMapClass* CSunHelper::CreateSun(CHelperInfo* pHelperInfo, CMapEntity* pParent)
{
	const char* key = pParent->GetKeyValue("material");

	CSunHelper* sun = new CSunHelper;
	if ( !sun ) return NULL;

	sun->m_suntexturename_string = key;
	
	key = pParent->GetKeyValue("size");
	sun->m_sunsize_float = atof(key);

//	key = pParent->GetKeyValue("use_angles");
//	if ( !key || atoi(key) == 0 )
//		sun->m_sunsize_float = 0;

	int r = 0, g = 0, b = 0;

	sscanf(pParent->GetKeyValue("rendercolor"), "%d %d %d", &r, &g, &b);
	sun->m_RenderColor.r = r;
	sun->m_RenderColor.g = g;
	sun->m_RenderColor.b = b;

	return sun;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CSunHelper::CSunHelper() : m_eRenderMode(kRenderTransAlpha)
{
	m_RenderColor.r = 255;
	m_RenderColor.g = 255;
	m_RenderColor.b = 255;
	m_RenderColor.a = 255;
	m_sunsize_float = 0.0f;
	m_suntexturename_string = "";
	Initialize();
}

//-----------------------------------------------------------------------------
// Purpose: Calculates our bounding box based on the sprite dimensions.
// Input  : bFullUpdate - Whether we should recalculate our childrens' bounds.
//-----------------------------------------------------------------------------
void CSunHelper::CalcBounds(BOOL bFullUpdate)
{
	CMapClass::CalcBounds(bFullUpdate);
	Vector vMin = m_Origin - Vector(m_sunsize_float, m_sunsize_float, m_sunsize_float);
	Vector vMax = -vMin;
	m_CullBox.UpdateBounds(vMin, vMax);
	m_BoundingBox = m_CullBox;
	m_Render2DBox.UpdateBounds(vMin, vMax);
}

//-----------------------------------------------------------------------------
// Purpose: Returns a copy of this object.
// Output : Pointer to the new object.
//-----------------------------------------------------------------------------
CMapClass* CSunHelper::Copy(bool bUpdateDependencies)
{
	CSunHelper* pCopy = new CSunHelper;

	if ( pCopy != NULL )
		pCopy->CopyFrom(this, bUpdateDependencies);

	return pCopy;
}

//-----------------------------------------------------------------------------
// Purpose: Turns this into a duplicate of the given object.
// Input  : pObject - Pointer to the object to copy from.
// Output : Returns a pointer to this object.
//-----------------------------------------------------------------------------
CMapClass* CSunHelper::CopyFrom(CMapClass* pObject, bool bUpdateDependencies)
{
	CSunHelper* pFrom = dynamic_cast<CSunHelper*>( pObject );
	Assert(pFrom != NULL);

	if ( pFrom != NULL )
	{
		CMapClass::CopyFrom(pObject, bUpdateDependencies);

		m_Angles = pFrom->m_Angles;
		m_RenderColor = pFrom->m_RenderColor;
		SetRenderColor(m_RenderColor.r, m_RenderColor.g, m_RenderColor.b);
	}

	return this;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CSunHelper::Initialize()
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
void CSunHelper::Render3D(CRender3D* pRender)
{
	if ( !GetParent() ) return;

	if ( m_sunsize_float <= 0 ) return;

	int nPasses;
	if ( ( GetSelectionState() != SELECT_NONE ))
	{
		if ( pRender->NeedsOverlay() )
			nPasses = 3;
		else
			nPasses = 2;
	} else
	{
		nPasses = 1;
	}

	//
	// If we have a sprite, render it.
	//
	bool render3d = true; //// SLE NEW: allow to hide sprites in 3d

//	if ( m_pSpriteInfo && render3d )
	{
		//
		// Only sprite icons can be clicked on, sprite preview objects cannot.
		//
	//	if ( m_bIsClickable )
		{
			pRender->BeginRenderHitTarget(this);
		}

		CSpriteModel *m_pSpriteInfo = CSpriteCache::CreateSprite(m_suntexturename_string);

		m_pSpriteInfo->SetOrigin(m_Origin);
		m_pSpriteInfo->SetAngles(m_Angles);

		m_pSpriteInfo->Bind(pRender, pRender->GetRenderFrame());
		for ( int nPass = 0; nPass < nPasses; nPass++ )
		{
			if ( nPass == 0 )
			{
				// First pass uses the default rendering mode.
				// unless that mode is texture
				if ( pRender->GetCurrentRenderMode() == RENDER_MODE_LIGHTMAP_GRID )
					pRender->PushRenderMode(RENDER_MODE_TEXTURED);
				else
					pRender->PushRenderMode(RENDER_MODE_CURRENT);
			} 
			
			else
			{
				if ( nPass == nPasses - 1 )
				{
					// last pass uses wireframe rendering mode.
					pRender->PushRenderMode(RENDER_MODE_WIREFRAME);
				} else
				{
					pRender->PushRenderMode(RENDER_MODE_SELECTION_OVERLAY);
				}
			}

			m_pSpriteInfo->SetScale(m_sunsize_float / 16);

			float fBlend;
			// dvs: lots of things contribute to blend factor. See r_blend in engine.
			if ( m_eRenderMode == kRenderWorldGlow )
			{
				fBlend = 0.5;
			} else
			{
				fBlend = 1.0;
			}

			unsigned char color[ 4 ];
		//	SpriteColor(color, m_eRenderMode, m_RenderColor, 255 * fBlend);

			MaterialPrimitiveType_t type = ( nPass > 0 ) ? MATERIAL_LINE_LOOP : MATERIAL_POLYGON;
			m_pSpriteInfo->SetMaterialPrimitiveType(type);

			m_pSpriteInfo->DrawSprite3D(pRender, color);

			pRender->PopRenderMode();
		}
	}

//	pRender->RenderWireframeSphere(m_Origin, m_sunsize_float, 18, 18, m_RenderColor.r, m_RenderColor.g, m_RenderColor.b);
}

//-----------------------------------------------------------------------------
void CSunHelper::DoTransform(const VMatrix& matrix)
{
	BaseClass::DoTransform(matrix);
}

//-----------------------------------------------------------------------------
// Purpose: Notifies that this object's parent entity has had a key value change.
// Input  : szKey - The key that changed.
//			szValue - The new value of the key.
//-----------------------------------------------------------------------------
void CSunHelper::OnParentKeyChanged(const char* szKey, const char* szValue)
{
	if ( !stricmp(szKey, "size") )
	{
		m_sunsize_float = atof(szValue);
		//	CalcBounds(true);
		PostUpdate(Notify_Changed);
	}
	if ( !stricmp(szKey, "material") )
	{
		m_suntexturename_string = szValue;
		PostUpdate(Notify_Changed);
	}
	if ( !stricmp(szKey, "use_angles") ) // mapbase uses SpotlightMaterial
	{
		PostUpdate(Notify_Changed);
	} 

	if ( !stricmp(szKey, "angles") )
	{
		sscanf(szValue, "%f %f %f", &m_Angles[ PITCH ], &m_Angles[ YAW ], &m_Angles[ ROLL ]);
		//	CalcBounds(true);
		PostUpdate(Notify_Changed);
	} 

	if ( !stricmp(szKey, "rendercolor") )
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
