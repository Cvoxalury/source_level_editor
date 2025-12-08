//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: This helper is used for entities that represent a sprite beam between
//			two entities. Examples of these are: beams, lasers and ropes.
//
//			The helper factory parameters are:
//
//			<red> <green> <blue> <start key> <start key value> <end key> <end key value>
//
//			The line helper looks in the given keys in its parent entity and
//			attaches itself to the entities with those key values. If only one
//			endpoint entity is specified, the other end is assumed to be the parent
//			entity.
//
//=============================================================================//

#include "stdafx.h"
#include "Box3D.h"
#include "MapEntity.h"
#include "MapSpriteBeam.h"
#include "beamdraw.h"
#include "MapWorld.h"
#include "Render2D.h"
#include "Render3D.h"
#include "TextureSystem.h"
#include "materialsystem/imesh.h"
#include "Material.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_MAPCLASS(CMapSpriteBeam);

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a CMapSpriteBeam from a set
//			of string parameters from the FGD file.
// Input  : *pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, NULL if an error occurs.
//-----------------------------------------------------------------------------
CMapClass *CMapSpriteBeam::Create(CHelperInfo *pHelperInfo, CMapEntity *pParent)
{
	CMapSpriteBeam *pLine = NULL;
	
//	const char *pszStartKey = pHelperInfo->GetParameter(3);
	const char *pszStartValueKey = pHelperInfo->GetParameter(0);

//	const char *pszEndKey = pHelperInfo->GetParameter(5);
	const char *pszEndValueKey = pHelperInfo->GetParameter(1);

//	const char *pszMaterialKey = pHelperInfo->GetParameter(7);
	const char *pszMaterialValueKey = pHelperInfo->GetParameter(2);

	const char *pszWidthValueKey = pHelperInfo->GetParameter(3);

	//
	// Make sure we'll have at least one endpoint to work with.
	//
	if ( ( pszStartValueKey == NULL ) || ( pszEndValueKey == NULL ) )
	{
		return NULL;
	}

	pLine = new CMapSpriteBeam(pszStartValueKey, pszEndValueKey, pszMaterialValueKey, pszWidthValueKey, false);

	//
	// If they only specified a start entity, use our parent as the end entity.
	//
	if ( ( pszStartValueKey == NULL ) || ( pszEndValueKey == NULL ) )
	{
		pLine->m_pEndEntity = pParent;
	}

	return( pLine );
}

// laser is a custom case of beam where the start entity is always the parent.
CMapClass *CMapSpriteBeam::CreateLaser(CHelperInfo *pHelperInfo, CMapEntity *pParent)
{
	CMapSpriteBeam *pLine = NULL;

	//	const char *pszStartKey = pHelperInfo->GetParameter(3);
	const char *pszStartValueKey = pHelperInfo->GetParameter(0);

	//	const char *pszEndKey = pHelperInfo->GetParameter(5);
	const char *pszEndValueKey = pHelperInfo->GetParameter(1);

	//	const char *pszMaterialKey = pHelperInfo->GetParameter(7);
	const char *pszMaterialValueKey = pHelperInfo->GetParameter(2);

	const char *pszWidthValueKey = pHelperInfo->GetParameter(3);

	//
	// Make sure we'll have the endpoint to work with.
	//
	if (pszEndValueKey == NULL)
	{
		return NULL;
	}

	pLine = new CMapSpriteBeam(pszStartValueKey, pszEndValueKey, pszMaterialValueKey, pszWidthValueKey, true);

	pLine->m_pStartEntity = pParent;

	pLine->m_islaser_boolean = true;

	return(pLine);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CMapSpriteBeam::CMapSpriteBeam(void)
{
	Initialize();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor. Initializes data members.
// Input  : pszStartKey - The key to search in other entities for a match against the value of pszStartValueKey, ex 'targetname'.
//			pszStartValueKey - The key in our parent entity from which to get a search term for the start entity ex 'beamstart01'.
//			pszEndKey - The key to search in other entities for a match against the value of pszEndValueKey ex 'targetname'.
//			pszEndValueKey - The key in our parent entity from which to get a search term for the end entity ex 'beamend01'.
//-----------------------------------------------------------------------------
CMapSpriteBeam::CMapSpriteBeam(const char *pszStartValueKey, 
	const char *pszEndValueKey, 
	const char *pszMaterialValueKey, 
	const char *pszWidthValueKey,
	bool isLaser)
{
	Initialize();

	m_islaser_boolean = isLaser;

	strcpy(m_szStartValueKey, pszStartValueKey);

	if ( ( pszEndValueKey != NULL ) )
	{
		strcpy(m_szEndValueKey, pszEndValueKey);
	}
	if ( ( pszMaterialValueKey != NULL ) )
	{
		strcpy(m_szMaterialValueKey, pszMaterialValueKey);
	}
	else
	{
		strcpy(m_szMaterialValueKey, "texture");
	}
	if ( ( pszWidthValueKey != NULL ) )
	{
		strcpy(m_szWidthValueKey, pszWidthValueKey);
	}
	else
	{
		if (!m_islaser_boolean)
			strcpy(m_szWidthValueKey, "BoltWidth");
		else
			strcpy(m_szWidthValueKey, "width");
	}
}


//-----------------------------------------------------------------------------
// Purpose: Sets data members to initial values.
//-----------------------------------------------------------------------------
void CMapSpriteBeam::Initialize(void)
{
	m_szStartValueKey[ 0 ] = '\0';

	m_szEndValueKey[ 0 ] = '\0';

	m_szMaterialKey[ 0 ] = '\0';
	m_szMaterialValueKey[ 0 ] = '\0';

	m_szWidthKey[ 0 ] = '\0';
	m_szWidthValueKey[ 0 ] = '\0';

	m_colorR = m_colorG = m_colorB = 1.0f;

	m_pStartEntity = NULL;
	m_pEndEntity = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CMapSpriteBeam::~CMapSpriteBeam(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Calculates the midpoint of the line and sets our origin there.
//-----------------------------------------------------------------------------
void CMapSpriteBeam::BuildLine(void)
{
	if (m_islaser_boolean)
	{
		CMapEntity *pEntity = dynamic_cast <CMapEntity *> (m_pParent);
		if (pEntity) m_pStartEntity = pEntity;
	}
	if ( ( m_pStartEntity != NULL ) && ( m_pEndEntity != NULL ) )
	{
		//
		// Set our origin to our midpoint. This moves our selection handle box to the
		// midpoint.
		//
		Vector Start;
		Vector End;

		m_pStartEntity->GetOrigin(Start);
		m_pEndEntity->GetOrigin(End);

		SetOrigin(( Start + End ) / 2);
	}

	CalcBounds();
}

//-----------------------------------------------------------------------------
// Purpose: Recalculates our bounding box.
// Input  : bFullUpdate - Whether to force our children to recalculate or not.
//-----------------------------------------------------------------------------
void CMapSpriteBeam::CalcBounds(BOOL bFullUpdate)
{
	CMapClass::CalcBounds(bFullUpdate);

	//
	// Don't calculate 2D bounds - we don't occupy any space in 2D. This keeps our
	// parent entity's bounds from expanding to encompass our endpoints.
	//

	//
	// Update our 3D culling box and possibly our origin.
	//
	// If our start and end entities are resolved, calcuate our bounds
	// based on the positions of the start and end entities.
	//
	if ( m_pStartEntity && m_pEndEntity )
	{
		//
		// Update the 3D bounds.
		//
		Vector Start;
		Vector End;

		m_pStartEntity->GetOrigin(Start);
		m_CullBox.UpdateBounds(Start);

		m_pEndEntity->GetOrigin(End);
		m_CullBox.UpdateBounds(End);
	}

	m_BoundingBox = m_CullBox;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bUpdateDependencies - 
// Output : CMapClass
//-----------------------------------------------------------------------------
CMapClass *CMapSpriteBeam::Copy(bool bUpdateDependencies)
{
	CMapSpriteBeam *pCopy = new CMapSpriteBeam;

	if ( pCopy != NULL )
	{
		pCopy->CopyFrom(this, bUpdateDependencies);
	}

	return( pCopy );
}

//-----------------------------------------------------------------------------
// Purpose: Turns 'this' into an exact replica of 'pObject'.
// Input  : pObject - Object to replicate.
//			bUpdateDependencies - 
// Output : 
//-----------------------------------------------------------------------------
CMapClass *CMapSpriteBeam::CopyFrom(CMapClass *pObject, bool bUpdateDependencies)
{
	CMapSpriteBeam *pFrom = dynamic_cast <CMapSpriteBeam *>( pObject );

	if ( pFrom != NULL )
	{
		CMapClass::CopyFrom(pObject, bUpdateDependencies);

		if ( bUpdateDependencies )
		{
			m_pStartEntity = ( CMapEntity * )UpdateDependency(m_pStartEntity, pFrom->m_pStartEntity);
			m_pEndEntity = ( CMapEntity * )UpdateDependency(m_pEndEntity, pFrom->m_pEndEntity);
		} 
		else
		{
			m_pStartEntity = pFrom->m_pStartEntity;
			m_pEndEntity = pFrom->m_pEndEntity;
		}

		m_islaser_boolean = pFrom->m_islaser_boolean;

		strcpy(m_szStartValueKey, pFrom->m_szStartValueKey);
		strcpy(m_szEndValueKey, pFrom->m_szEndValueKey);
		strcpy(m_szMaterialValueKey, pFrom->m_szMaterialValueKey);
		strcpy(m_szWidthValueKey, pFrom->m_szWidthValueKey);
	}

	return( this );
}

//-----------------------------------------------------------------------------
// Purpose: Called after this object is added to the world.
//
//			NOTE: This function is NOT called during serialization. Use PostloadWorld
//				  to do similar bookkeeping after map load.
//
// Input  : pWorld - The world that we have been added to.
//-----------------------------------------------------------------------------
void CMapSpriteBeam::OnAddToWorld(CMapWorld *pWorld)
{
	CMapClass::OnAddToWorld(pWorld);

	//
	// Updates our start and end entity pointers since we are being added
	// into the world.
	//
	UpdateDependencies(pWorld, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Called just after this object has been removed from the world so
//			that it can unlink itself from other objects in the world.
// Input  : pWorld - The world that we were just removed from.
//			bNotifyChildren - Whether we should forward notification to our children.
//-----------------------------------------------------------------------------
void CMapSpriteBeam::OnRemoveFromWorld(CMapWorld *pWorld, bool bNotifyChildren)
{
	CMapClass::OnRemoveFromWorld(pWorld, bNotifyChildren);

	//
	// Detach ourselves from the endpoint entities.
	//
	m_pStartEntity = ( CMapEntity * )UpdateDependency(m_pStartEntity, NULL);
	m_pEndEntity = ( CMapEntity * )UpdateDependency(m_pEndEntity, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Our start or end entity has changed; recalculate our bounds and midpoint.
// Input  : pObject - Entity that changed.
//-----------------------------------------------------------------------------
void CMapSpriteBeam::OnNotifyDependent(CMapClass *pObject, Notify_Dependent_t eNotifyType)
{
	CMapClass::OnNotifyDependent(pObject, eNotifyType);

	CMapWorld *pWorld = ( CMapWorld * )GetWorldObject(this);
	UpdateDependencies(pWorld, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : key - 
//			value - 
//-----------------------------------------------------------------------------
void CMapSpriteBeam::OnParentKeyChanged(const char* key, const char* value)
{
	CMapWorld *pWorld = ( CMapWorld * )GetWorldObject(this);
	if ( pWorld != NULL )
	{
		if ( stricmp(key, m_szStartValueKey) == 0 )
		{
			if( !m_islaser_boolean)
				m_pStartEntity = ( CMapEntity * )UpdateDependency(m_pStartEntity, pWorld->FindChildByKeyValue("targetname", value));

			BuildLine();
			PostUpdate(Notify_Changed);
		} 
		else if ( stricmp(key, m_szEndValueKey) == 0 )
		{
			m_pEndEntity = ( CMapEntity * )UpdateDependency(m_pEndEntity, pWorld->FindChildByKeyValue("tagetname", value));
			BuildLine();
			PostUpdate(Notify_Changed);
		}
		else if ( stricmp(key, m_szMaterialValueKey) == 0 )
		{
		//	UpdateDependencies(pWorld, NULL);
			PostUpdate(Notify_Changed);
		}
		else if ( stricmp(key, m_szWidthValueKey) == 0 )
		{
		//	UpdateDependencies(pWorld, NULL);
			PostUpdate(Notify_Changed);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Renders the line helper in the 2D view.
// Input  : pRender - 2D rendering interface.
//-----------------------------------------------------------------------------
void CMapSpriteBeam::Render2D(CRender2D *pRender)
{
	if ( ( m_pStartEntity != NULL ) && ( m_pEndEntity != NULL ) || (m_islaser_boolean && m_pEndEntity != NULL))
	{
		Vector Start;
		Vector End;

		m_pStartEntity->GetOrigin(Start);
		m_pEndEntity->GetOrigin(End);

		if ( IsSelected() )
		{
			pRender->SetDrawColor(220, 0, 0 /*SELECT_FACE_RED, SELECT_FACE_GREEN, SELECT_FACE_BLUE*/);
		} else
		{
			pRender->SetDrawColor(r, g, b);
		}

		pRender->DrawLine(Start, End);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pRender - 
//-----------------------------------------------------------------------------
void CMapSpriteBeam::Render3D(CRender3D *pRender)
{
	if (m_islaser_boolean)
	{
		CMapEntity *pEntity = dynamic_cast <CMapEntity *> (m_pParent);
		if (pEntity) m_pStartEntity = pEntity;
	}

	if ( ( m_pStartEntity == NULL ) || ( m_pEndEntity == NULL ) )
		return;

	if ( m_szMaterialKey[ 0 ] == '\0' )
		return;

	pRender->BeginRenderHitTarget(this);
	pRender->PushRenderMode(RENDER_MODE_WIREFRAME);

	Vector Start, End;

	m_pStartEntity->GetOrigin(Start);
	m_pEndEntity->GetOrigin(End);
	
	float noisefloat = RandomFloat(0.5f, 2.5f);
	float *noise;
	noise = &noisefloat;

	float fcolor[3];

	fcolor[0] = m_colorR;
	fcolor[1] = m_colorG;
	fcolor[2] = m_colorB;
		
	DrawSegs(pRender, 10, noise, m_szMaterialKey, 1, 
		kRenderTransAdd, Start, ( End - Start ), atof(m_szWidthKey), atof(m_szWidthKey), 
		1, 1, 30, 6, FBEAM_FADEIN, fcolor, 1);

	pRender->EndRenderHitTarget();
	pRender->PopRenderMode();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : File - 
//			bRMF - 
// Output : int
//-----------------------------------------------------------------------------
int CMapSpriteBeam::SerializeRMF(std::fstream &File, BOOL bRMF)
{
	return( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : File - 
//			bRMF - 
// Output : int
//-----------------------------------------------------------------------------
int CMapSpriteBeam::SerializeMAP(std::fstream &File, BOOL bRMF)
{
	return( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTransBox - 
//-----------------------------------------------------------------------------
void CMapSpriteBeam::DoTransform(const VMatrix &matrix)
{
	BaseClass::DoTransform(matrix);
	BuildLine();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the cached pointers to our start and end entities by looking
//			for them in the given world.
// Input  : pWorld - World to search.
//-----------------------------------------------------------------------------
void CMapSpriteBeam::UpdateDependencies(CMapWorld *pWorld, CMapClass *pObject)
{
	CMapClass::UpdateDependencies(pWorld, pObject);

	if ( pWorld == NULL )
	{
		return;
	}

	CMapEntity *pEntity = dynamic_cast <CMapEntity *> ( m_pParent );
	Assert(pEntity != NULL);
	
	if ( pEntity != NULL )
	{
		const char *pszValue = pEntity->GetKeyValue(m_szStartValueKey);
		if (!m_islaser_boolean)
			m_pStartEntity = (CMapEntity *)UpdateDependency(m_pStartEntity, pWorld->FindChildByKeyValue("targetname", pszValue));
		else // lasers always start at the parent entity
		{
			m_pStartEntity = (CMapEntity *)UpdateDependency(m_pStartEntity, GetParent());
		}

		if ( m_szEndValueKey[ 0 ] != '\0' )
		{
			pszValue = pEntity->GetKeyValue(m_szEndValueKey);
			m_pEndEntity = ( CMapEntity * )UpdateDependency(m_pEndEntity, pWorld->FindChildByKeyValue("targetname", pszValue));
		}
		else
		{
			// We don't have an end entity specified, use our parent as the end point.
			m_pEndEntity = ( CMapEntity * )UpdateDependency(m_pEndEntity, GetParent());
		}

		if ( m_szMaterialValueKey[ 0 ] != '\0' )
		{
			pszValue = pEntity->GetKeyValue(m_szMaterialValueKey);
			if ( pszValue )
			{
			// strip "materials" from the string, as it is automatically appended.
				if ( pszValue && strlen(pszValue) >= 10 && !strncmp(pszValue, "materials/", 10) )
				{
					strcpy(m_szMaterialKey, &pszValue[ 10 ]);
				} else
				{
					strcpy(m_szMaterialKey, pszValue);
				}
			}
		}

		if ( m_szWidthValueKey[ 0 ] != '\0' )
		{
			pszValue = pEntity->GetKeyValue(m_szWidthValueKey);
			strcpy(m_szWidthKey, pszValue);
		}

		{
			pszValue = pEntity->GetKeyValue("rendercolor");
			if ( pszValue[ 0 ] != '\0' )
			{
				int r, g, b;
				sscanf(pszValue, "%d %d %d", &r, &g, &b);
				m_colorR = ( float )r / 255;
				m_colorG = ( float )g / 255;
				m_colorB = ( float )b / 255;
			}
		}

		BuildLine();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Never select anything because of this helper.
//-----------------------------------------------------------------------------
CMapClass *CMapSpriteBeam::PrepareSelection(SelectMode_t eSelectMode)
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Tesla preview
//-----------------------------------------------------------------------------
IMPLEMENT_MAPCLASS(CMapSpriteTesla);

CMapClass *CMapSpriteTesla::Create(CHelperInfo *pHelperInfo, CMapEntity *pParent)
{
	CMapSpriteTesla *pLine = NULL;

	//	const char *pszStartKey = pHelperInfo->GetParameter(3);
	const char *pszStartValueKey = pHelperInfo->GetParameter(0);

	//	const char *pszEndKey = pHelperInfo->GetParameter(5);
	const char *pszEndValueKey = pHelperInfo->GetParameter(1);

	//	const char *pszMaterialKey = pHelperInfo->GetParameter(7);
	const char *pszMaterialValueKey = pHelperInfo->GetParameter(2);

	const char *pszWidthValueKey = pHelperInfo->GetParameter(3);

	//
	// Make sure we'll have at least one endpoint to work with.
	//
	if ( ( pszStartValueKey == NULL ) || ( pszEndValueKey == NULL ) )
	{
		return NULL;
	}

	pLine = new CMapSpriteTesla(pszStartValueKey, pszEndValueKey, pszMaterialValueKey, pszWidthValueKey);

	//
	// If they only specified a start entity, use our parent as the end entity.
	//
	if ( ( pszStartValueKey == NULL ) || ( pszEndValueKey == NULL ) )
	{
		pLine->m_pEndEntity = pParent;
	}

	return( pLine );
}

CMapSpriteTesla::CMapSpriteTesla(void)
{
	Initialize();
}

CMapSpriteTesla::CMapSpriteTesla(const char *pszStartValueKey,
	const char *pszEndValueKey,
	const char *pszMaterialValueKey,
	const char *pszWidthValueKey)
{
	Initialize();

	strcpy(m_szStartValueKey, pszStartValueKey);

	if ( ( pszEndValueKey != NULL ) )
	{
		strcpy(m_szEndValueKey, pszEndValueKey);
	}
	if ( ( pszMaterialValueKey != NULL ) )
	{
		strcpy(m_szMaterialValueKey, pszMaterialValueKey);
	} else
	{
		strcpy(m_szMaterialValueKey, "texture");
	}
	if ( ( pszWidthValueKey != NULL ) )
	{
		strcpy(m_szWidthValueKey, pszWidthValueKey);
	} else
	{
		strcpy(m_szWidthValueKey, "BoltWidth");
	}
}

CMapSpriteTesla::~CMapSpriteTesla(void)
{
}
