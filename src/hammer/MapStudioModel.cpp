//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "stdafx.h"
#include "Box3D.h"
#include "GlobalFunctions.h"
#include "MapDefs.h"		// dvs: For COORD_NOTINIT
#include "MapDoc.h"
#include "MapEntity.h"
#include "MapStudioModel.h"
#ifdef SLE //// needed for the sky camera access
#include "mapworld.h"
#include "vphysics_interface.h" // needed for dumping collision into SMD
#endif
#include "Render2D.h"
#include "Render3D.h"
#include "ViewerSettings.h"
#include "hammer.h"
#include "materialsystem/imesh.h"
#include "TextureSystem.h"
#include "Material.h"
#include "Options.h"
#include "camera.h"
#ifdef SLE_USE_HAMMER_LPREVIEW //// taken from Hammer-2013
#include "istudiorender.h"
#include "optimize.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define STUDIO_RENDER_DISTANCE		400

IMPLEMENT_MAPCLASS(CMapStudioModel)

float CMapStudioModel::m_fRenderDistance = STUDIO_RENDER_DISTANCE;
BOOL CMapStudioModel::m_bAnimateModels = TRUE;

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a CMapStudioModel from a set
//			of string parameters from the FGD file.
// Input  : pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, NULL if an error occurs.
//-----------------------------------------------------------------------------
CMapClass *CMapStudioModel::CreateMapStudioModel(CHelperInfo *pHelperInfo, CMapEntity *pParent)
{
	const char *pszModel = pHelperInfo->GetParameter(0);

	//
	// If we weren't passed a model name as an argument, get it from our parent
	// entity's "model" key.
	//
	if (pszModel == NULL)
	{
		pszModel = pParent->GetKeyValue("model");
	}

	//
	// If we have a model name, create a studio model object.
	//
	if (pszModel != NULL)
	{
		bool bLightProp = !stricmp(pHelperInfo->GetName(), "lightprop");
		bool bOrientedBounds = (bLightProp | !stricmp(pHelperInfo->GetName(), "studioprop"));
		return CreateMapStudioModel(pszModel, bOrientedBounds, bLightProp);
	}

	return(NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Factory function. Creates a CMapStudioModel object from a relative
//			path to an MDL file.
// Input  : pszModelPath - Relative path to the .MDL file. The path is appended
//				to each path in the	application search path until the model is found.
//			bOrientedBounds - Whether the bounding box should consider the orientation of the model.
// Output : Returns a pointer to the newly created CMapStudioModel object.
//-----------------------------------------------------------------------------
CMapStudioModel *CMapStudioModel::CreateMapStudioModel(const char *pszModelPath, bool bOrientedBounds, bool bReversePitch)
{
	CMapStudioModel *pModel = new CMapStudioModel;
	pModel->m_pStudioModel = CStudioModelCache::CreateModel(pszModelPath);
	if ( pModel->m_pStudioModel )
	{
		pModel->SetOrientedBounds(bOrientedBounds);
		pModel->ReversePitch(bReversePitch);
		pModel->CalcBounds();
	}
	else
	{
		delete pModel;
		pModel = NULL;
	}
	return(pModel);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CMapStudioModel::CMapStudioModel(void)
{
	Initialize();
	InitViewerSettings();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor. Releases the studio model cache reference.
//-----------------------------------------------------------------------------
CMapStudioModel::~CMapStudioModel(void)
{
	if (m_pStudioModel != NULL)
	{
		CStudioModelCache::Release(m_pStudioModel);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called by the renderer before every frame to animate the models.
//-----------------------------------------------------------------------------
void CMapStudioModel::AdvanceAnimation(float flInterval)
{
	if (m_bAnimateModels )
	{
		CStudioModelCache::AdvanceAnimation(flInterval);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bFullUpdate - 
//-----------------------------------------------------------------------------
void CMapStudioModel::CalcBounds(BOOL bFullUpdate)
{
	CMapClass::CalcBounds(bFullUpdate);

	Vector Mins(0, 0, 0);
	Vector Maxs(0, 0, 0);

	if (m_pStudioModel != NULL)
	{
		//
		// The 3D bounds are the bounds of the oriented model's first sequence, so that
		// frustum culling works properly in the 3D view.
		//
		QAngle angles;
		GetRenderAngles(angles);

		m_pStudioModel->SetAngles(angles);
		m_pStudioModel->ExtractBbox(m_CullBox.bmins, m_CullBox.bmaxs);

		if (m_bOrientedBounds)
		{
			//
			// Oriented bounds - the 2D bounds are the same as the 3D bounds.
			//
			Mins = m_CullBox.bmins;
			Maxs = m_CullBox.bmaxs;
		}
		else
		{
			//
			// The 2D bounds are the movement bounding box of the model, which is not affected
			// by the entity's orientation. This is used for character models for which we want
			// to render a meaningful collision box in the editor.
			//
			m_pStudioModel->ExtractMovementBbox(Mins, Maxs);
		}

		Mins += m_Origin;
		Maxs += m_Origin;

		m_CullBox.bmins += m_Origin;
		m_CullBox.bmaxs += m_Origin;
	}

	//
	// If we do not yet have a valid bounding box, use a default box.
	//
	if ((Maxs - Mins) == Vector(0, 0, 0))
	{
		Mins = m_CullBox.bmins = m_Origin - Vector(10, 10, 10);
		Maxs = m_CullBox.bmaxs = m_Origin + Vector(10, 10, 10);
	}

	m_BoundingBox = m_CullBox;
	m_Render2DBox.UpdateBounds(Mins, Maxs);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CMapClass
//-----------------------------------------------------------------------------
CMapClass *CMapStudioModel::Copy(bool bUpdateDependencies)
{
	CMapStudioModel *pCopy = new CMapStudioModel;

	if (pCopy != NULL)
	{
		pCopy->CopyFrom(this, bUpdateDependencies);
	}

	return(pCopy);
}

//-----------------------------------------------------------------------------
// Purpose: Makes this an exact duplicate of pObject.
// Input  : pObject - Object to copy.
// Output : Returns this.
//-----------------------------------------------------------------------------
CMapClass *CMapStudioModel::CopyFrom(CMapClass *pObject, bool bUpdateDependencies)
{
	Assert(pObject->IsMapClass(MAPCLASS_TYPE(CMapStudioModel)));
	CMapStudioModel *pFrom = (CMapStudioModel *)pObject;

	CMapClass::CopyFrom(pObject, bUpdateDependencies);

	m_pStudioModel = pFrom->m_pStudioModel;
	if (m_pStudioModel != NULL)
	{
		CStudioModelCache::AddRef(m_pStudioModel);
	}

	m_Angles = pFrom->m_Angles;
	m_Skin = pFrom->m_Skin;

	m_bOrientedBounds = pFrom->m_bOrientedBounds;
	m_bReversePitch = pFrom->m_bReversePitch;
	m_bPitchSet = pFrom->m_bPitchSet;
	m_flPitch = pFrom->m_flPitch;

	m_bScreenSpaceFade = pFrom->m_bScreenSpaceFade;
	m_flFadeScale = pFrom->m_flFadeScale;
	m_flFadeMinDist = pFrom->m_flFadeMinDist;
	m_flFadeMaxDist = pFrom->m_flFadeMaxDist;
	m_iSolid = pFrom->m_iSolid;
#ifdef SLE
	m_BodyGroup = pFrom->m_BodyGroup; //// SLE NEW: preview bodygroups
	m_Scale = pFrom->m_Scale; //// SLE NEW: preview model scale
	m_ModelRenderColor = pFrom->m_ModelRenderColor; //// SLE NEW: preview model rendercolor
	m_disableShadows = pFrom->m_disableShadows; //// SLE NEW: shadow control for light preview
#endif

	return(this);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bEnable - 
//-----------------------------------------------------------------------------
void CMapStudioModel::EnableAnimation(BOOL bEnable)
{
	m_bAnimateModels = bEnable;
}

//-----------------------------------------------------------------------------
// Purpose: Returns this object's pitch, yaw, and roll.
//-----------------------------------------------------------------------------
void CMapStudioModel::GetAngles(QAngle &Angles)
{
	Angles = m_Angles;

	if (m_bPitchSet)
	{
		Angles[PITCH] = m_flPitch;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns this object's pitch, yaw, and roll for rendering.
//-----------------------------------------------------------------------------
void CMapStudioModel::GetRenderAngles(QAngle &Angles)
{
	GetAngles(Angles);

	if (m_bReversePitch)
	{
		Angles[PITCH] *= -1;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapStudioModel::Initialize(void)
{
	m_Angles.Init();
	m_bPitchSet = false;
	m_flPitch = 0;
	m_bReversePitch = false;
	m_pStudioModel = NULL;
	m_Skin = 0;

	m_bScreenSpaceFade = false;
	m_flFadeScale = 1.0f;
	m_flFadeMinDist = 0.0f;
	m_flFadeMaxDist = 0.0f;
	m_iSolid = -1;
#ifdef SLE
	m_BodyGroup = 0; //// SLE NEW: preview bodygroups
	m_Scale = 1.0f; //// SLE NEW: preview model scale
	m_ModelRenderColor.SetColor(255, 255, 255, 255); //// SLE NEW: preview model rendercolor
	m_disableShadows = false; //// SLE NEW: shadow control for light preview
	m_sequenceFrameFromSlider = 0;
	
	if( m_pStudioModel) m_pStudioModel->SetBodygroups(m_BodyGroup);
	if( m_pStudioModel) m_pStudioModel->SetModelScale(m_Scale);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Notifies that this object's parent entity has had a key value change.
// Input  : szKey - The key that changed.
//			szValue - The new value of the key.
//-----------------------------------------------------------------------------
void CMapStudioModel::OnParentKeyChanged(const char* szKey, const char* szValue)
{
	if (!stricmp(szKey, "angles"))
	{
		sscanf(szValue, "%f %f %f", &m_Angles[PITCH], &m_Angles[YAW], &m_Angles[ROLL]);
		PostUpdate(Notify_Changed);
	}
	else if (!stricmp(szKey, "pitch"))
	{
		m_flPitch = atof(szValue);
		m_bPitchSet = true;

		PostUpdate(Notify_Changed);
	}
	else if (!stricmp(szKey, "skin"))
	{
		m_Skin = atoi(szValue);
		PostUpdate(Notify_Changed);
	}
	else if (!stricmp(szKey, "fademindist"))
	{
		m_flFadeMinDist = atoi(szValue);
	}
	else if (!stricmp(szKey, "fademaxdist"))
	{
		m_flFadeMaxDist = atoi(szValue);
	}
	else if (!stricmp(szKey, "screenspacefade"))
	{
		m_bScreenSpaceFade = (atoi(szValue) != 0);
	}
	else if (!stricmp(szKey, "fadescale"))
	{
		m_flFadeScale = atof(szValue);
	}
	else if ( !stricmp( szKey, "solid") )
	{
		m_iSolid = atof( szValue );
	}
#ifdef SLE
	//// SLE NEW: preview bodygroups
	else if (!stricmp(szKey, "body")) // default
	{
		m_BodyGroup = atoi(szValue);
		if( m_BodyGroup < 0 ) m_BodyGroup = 0; // hack to prevent studiomodel crash; this simulates game behavior as -1 means defaulting to the first 
		PostUpdate(Notify_Changed);
	}
	else if (!stricmp(szKey, "bodygroup")) 
	{
		m_BodyGroup = atoi(szValue);
		if( m_BodyGroup < 0 ) m_BodyGroup = 0; // hack to prevent studiomodel crash; this simulates game behavior as -1 means defaulting to the first 
		PostUpdate(Notify_Changed);
	}
	else if (!stricmp(szKey, "SetBodygroup")) // prop dynamics use this
	{
		m_BodyGroup = atoi(szValue);
		if( m_BodyGroup < 0 ) m_BodyGroup = 0; // hack to prevent studiomodel crash; this simulates game behavior as -1 means defaulting to the first 
		PostUpdate(Notify_Changed);
	}
	else if (!stricmp(szKey, "hardware")) // prop door rotatings use this
	{
		m_BodyGroup = atoi(szValue);
		if( m_BodyGroup < 0 ) m_BodyGroup = 0; // hack to prevent studiomodel crash; this simulates game behavior as -1 means defaulting to the first 
		PostUpdate(Notify_Changed);
	}
	else if (!stricmp(szKey, "modelscale")) //// SLE NEW: preview model scale
	{
		m_Scale = atof(szValue);
		if (m_Scale == 0) m_Scale = 1.0f;
		PostUpdate(Notify_Changed);
	}
	else if (!stricmp(szKey, "rendercolor")) //// SLE NEW: preview model rendercolor
	{
		int r, g, b;
		sscanf(szValue, "%d %d %d", &r, &g, &b);
		m_ModelRenderColor.SetColor(r, g, b, 255);
	}
	else if (!stricmp(szKey, "defaultanim")) //// SLE NEW: preview default prop dynamic anim
	{
		int nSequence = GetSequenceIndex(szValue);
		if (nSequence != -1)
		{
			m_pStudioModel->SetSequence(nSequence);
		}

		SetFrame(m_sequenceFrameFromSlider);
	}
	else if ( !stricmp(szKey, "disableshadows") ) //// SLE NEW: shadow control for light preview
	{
		if( atoi(szValue) == 0)
			m_disableShadows = false;
		else
			m_disableShadows = true;
		PostUpdate(Notify_Changed);
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pRender - 
//-----------------------------------------------------------------------------
bool CMapStudioModel::RenderPreload(CRender3D *pRender, bool bNewContext)
{
	return(m_pStudioModel != NULL);
}

//-----------------------------------------------------------------------------
// Draws basis vectors
//-----------------------------------------------------------------------------
static void DrawBasisVectors( CRender3D* pRender, const Vector &origin, const QAngle &angles)
{
	matrix3x4_t fCurrentMatrix;
	AngleMatrix(angles, fCurrentMatrix);

	pRender->PushRenderMode( RENDER_MODE_WIREFRAME );

	CMeshBuilder meshBuilder;
	CMatRenderContextPtr pRenderContext( MaterialSystemInterface() );
	IMesh* pMesh = pRenderContext->GetDynamicMesh();
	meshBuilder.Begin( pMesh, MATERIAL_LINES, 3 );

	meshBuilder.Color3ub(255, 0, 0);
	meshBuilder.Position3f(origin[0], origin[1], origin[2]);
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3ub(255, 0, 0);
	meshBuilder.Position3f(origin[0] + (100 * fCurrentMatrix[0][0]), 
		origin[1] + (100 * fCurrentMatrix[1][0]), origin[2] + (100 * fCurrentMatrix[2][0]));
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3ub(0, 255, 0);
	meshBuilder.Position3f(origin[0], origin[1], origin[2]);
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3ub(0, 255, 0);
	meshBuilder.Position3f(origin[0] + (100 * fCurrentMatrix[0][1]), 
		origin[1] + (100 * fCurrentMatrix[1][1]), origin[2] + (100 * fCurrentMatrix[2][1]));
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3ub(0, 0, 255);
	meshBuilder.Position3f(origin[0], origin[1], origin[2]);
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3ub(0, 0, 255);
	meshBuilder.Position3f(origin[0] + (100 * fCurrentMatrix[0][2]), 
		origin[1] + (100 * fCurrentMatrix[1][2]), origin[2] + (100 * fCurrentMatrix[2][2]));
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();

	pRender->PopRenderMode();
}

//-----------------------------------------------------------------------------
// It should render last if any of its materials are translucent, or if
// we are previewing model fades.
//-----------------------------------------------------------------------------
bool CMapStudioModel::ShouldRenderLast()
{
	return m_pStudioModel->IsTranslucent() || Options.view3d.bPreviewModelFade;
}

//-----------------------------------------------------------------------------
// Purpose: Renders the studio model in the 2D views.
// Input  : pRender - Interface to the 2D renderer.
//-----------------------------------------------------------------------------
void CMapStudioModel::Render2D(CRender2D *pRender)
{
	Vector vecMins;
	Vector vecMaxs;
	GetRender2DBox(vecMins, vecMaxs);

	Vector2D pt,pt2;
	pRender->TransformPoint(pt, vecMins);
	pRender->TransformPoint(pt2, vecMaxs);

	color32 rgbColor = GetRenderColor();
	bool	bIsEditable = IsEditable();
	
#ifdef SLE //// SLE NEW - Freeze/unfreeze 
	if ( IsFrozen() )
	{		
		pRender->SetDrawColor( 128, 128, 128 ); // todo - add Frozen colour later
		pRender->SetHandleColor( 128, 128, 128 );
	}
	else
#endif
	{
		if ( GetSelectionState() != SELECT_NONE )
		{
#ifdef SLE //// SLE CHANGE - separate colours
			pRender->SetDrawColor(GetRValue(Options.colors.clr2DSelection), GetGValue(Options.colors.clr2DSelection), GetBValue(Options.colors.clr2DSelection));
			pRender->SetHandleColor(GetRValue(Options.colors.clr2DSelection), GetGValue(Options.colors.clr2DSelection), GetBValue(Options.colors.clr2DSelection));
#else
			pRender->SetDrawColor( GetRValue(Options.colors.clrSelection), GetGValue(Options.colors.clrSelection), GetBValue(Options.colors.clrSelection) );
			pRender->SetHandleColor( GetRValue(Options.colors.clrSelection), GetGValue(Options.colors.clrSelection), GetBValue(Options.colors.clrSelection) );	
#endif
		} 
		else
		{
			pRender->SetDrawColor(rgbColor.r, rgbColor.g, rgbColor.b);
			pRender->SetHandleColor(rgbColor.r, rgbColor.g, rgbColor.b);
		}
	}

	int sizeX = abs(pt2.x-pt.x);
	int sizeY = abs(pt2.y-pt.y);

	//
	// Don't draw the center handle if the model is smaller than the handle cross 	
	//
	if ( bIsEditable && sizeX >= 8 && sizeY >= 8 && pRender->IsActiveView() )
	{
		pRender->SetHandleStyle( HANDLE_RADIUS, CRender::HANDLE_CROSS );

		pRender->DrawHandle( (vecMins+vecMaxs)/2 );
	}
	
	QAngle vecAngles;
	GetRenderAngles(vecAngles);
#ifdef SLE //// makes it a little less perf heavy
	bool bDrawAsModel = (Options.view2d.bDrawModels && ((sizeX+sizeY) > 100)) ||	
						IsSelected() ||	( pRender->IsInLocalTransformMode() && !pRender->GetInstanceRendering() );
#else
	bool bDrawAsModel = (Options.view2d.bDrawModels && ((sizeX+sizeY) > 50)) ||	
						IsSelected() ||	( pRender->IsInLocalTransformMode() && !pRender->GetInstanceRendering() );
#endif
	if ( !bDrawAsModel || IsSelected() )
	{
		// Draw the bounding box.
		pRender->DrawBox( vecMins, vecMaxs );
	}

	if ( bDrawAsModel )
	{
		//
		// Draw the model as wireframe.
		//

		m_pStudioModel->SetAngles(vecAngles);
		
		m_pStudioModel->SetOrigin(m_Origin[0], m_Origin[1], m_Origin[2]);
		m_pStudioModel->SetSkin(m_Skin);
#ifdef SLE //// SLE NEW: preview model scale
		m_pStudioModel->SetModelScale(m_Scale);
#endif
		if ( GetSelectionState() == SELECT_NORMAL || ( pRender->IsInLocalTransformMode() && pRender->GetInstanceRendering() == false ) )
		{
 			// draw textured model half translucent
			m_pStudioModel->DrawModel2D(pRender, 0.6, false );
		}
		else
		{
			// just draw the wireframe 
			m_pStudioModel->DrawModel2D(pRender, 1.0, true );
		}
	}

	if ( IsSelected() )
	{
		//
		// Render the forward vector if the object is selected.
		//
		
		Vector Forward;
		AngleVectors(vecAngles, &Forward, NULL, NULL);

		pRender->SetDrawColor( 255, 255, 0 );
		pRender->DrawLine(m_Origin, m_Origin + Forward * 24);
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#if 0 //def SLE //// SLE CHANGE - from SDK-2013-Hammer, fade is reworked so it can show screenspace fade 
				// needs more work, screenspace seems bugged and distance fade seems to not always apply
/*inline*/ float CMapStudioModel::ComputeDistanceFade( CRender3D *pRender ) /*const*/
{
	Vector vecViewPos;
	pRender->GetCamera()->GetViewPoint( vecViewPos );

	Vector vecDelta;		
	vecDelta = m_Origin - vecViewPos;

	float flMin = min(m_flFadeMinDist, m_flFadeMaxDist);
	float flMax = max(m_flFadeMinDist, m_flFadeMaxDist);

	if (flMin < 0)
	{
	//	flMin = 0;
		// from sdk-2013-hammer
		flMin = flMax + flMin;
		if (flMin < 0)
		{
			flMin = 0;
		}
	}

	float alpha = 1.0f;
	if (flMax > 0)
	{
		float flDist = vecDelta.Length();
		if ( flDist > flMax )
		{
			alpha = 0.0f;
		}
		else if ( flDist > flMin )
		{
			alpha = RemapValClamped( flDist, flMin, flMax, 1.0f, 0 );
		}
	}
		
	return alpha;
}
//// Ported from SDK-2013-Hammer
//-----------------------------------------------------------------------------
// Purpose: Calculate the bounding radius of the studio model.
//-----------------------------------------------------------------------------
float CMapStudioModel::GetBoundingRadius(void)
{
	Vector vecMin, vecMax;
	GetCullBox(vecMin, vecMax);
	return (vecMin.DistTo(vecMax) * 0.5f);
}
//-----------------------------------------------------------------------------
// Computes fade from screen-space fading
//-----------------------------------------------------------------------------
float CMapStudioModel::ComputeScreenFadeInternal(CRender3D *pRender, float flMinSize, float flMaxSize)
{
	float flRadius = GetBoundingRadius();
	float flPixelWidth = pRender->ComputePixelWidthOfSphere(m_Origin, flRadius);

	float flAlpha = 0.0f;
	if (flPixelWidth > flMinSize)
	{
		if ((flMaxSize >= 0) && (flPixelWidth < flMaxSize))
		{
			if (flMaxSize != flMinSize)
			{
				flAlpha = (flPixelWidth - flMinSize) / (flMaxSize - flMinSize);
				flAlpha = clamp(flAlpha, 0.0f, 1.0f);
			}
			else
			{
				flAlpha = 0.0f;
			}
		}
		else
		{
			flAlpha = 1.0f;
		}
	}

	return flAlpha;
}

//-----------------------------------------------------------------------------
// Computes fade alpha based on distance fade + screen fade
//-----------------------------------------------------------------------------
float CMapStudioModel::ComputeScreenFade(CRender3D *pRender, float flMinSize, float flMaxSize)
{
	CCamera *pCamera = pRender->GetCamera();
	if (!pCamera)
		return 1.0f;

	int nWidth, nHeight;
	pCamera->GetViewPort(nWidth, nHeight);

	float flScale = static_cast<float>(nWidth) / 1280.f;
	float flMin = flMinSize * flScale;
	float flMax = flMaxSize * flScale;

	return ComputeScreenFadeInternal(pRender, flMin, flMax);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CMapStudioModel::ComputeLevelFade(CRender3D *pRender)
{
	float flAlpha = 1.0f;
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if (pDoc)
	{
		CMapWorld *pWorld = pDoc->GetMapWorld();
		if (pWorld)
		{
			// Note this isn't a bug here - look at the fgd.cfg!
			const char *pszValueMin = pWorld->GetKeyValue("minpropscreenwidth");
			const char *pszValueMax = pWorld->GetKeyValue("maxpropscreenwidth");
			if (pszValueMin && pszValueMax)
			{
				float flMin = atof(pszValueMin);
				float flMax = atof(pszValueMax);
				flAlpha = ComputeScreenFade(pRender, flMin, flMax);
			}
		}
	}

	return flAlpha;
}
////

//-----------------------------------------------------------------------------
// Computes fade alpha based on distance fade + screen fade
//-----------------------------------------------------------------------------
/*inline*/ float CMapStudioModel::ComputeScreenFade( CRender3D *pRender ) /*const*/
{
	return 1.0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//// from sdk-2013-hammer
float CMapStudioModel::ComputeFade(CRender3D *pRender)
{
	// Don't fade no matter what!
	if (m_flFadeScale == 0.0f)
		return 1.0f;

	// Do we have any distance fade parameters?
	bool bCanFade = (m_flFadeMaxDist != 0.0f);

	// Do we need to fade?
	if (!bCanFade)
		return 1.0f;

	// Calculate the screen or distance fade.
	return ComputeDistanceFade(pRender);
}
#else
inline float CMapStudioModel::ComputeDistanceFade( CRender3D *pRender ) const
{
	Vector vecViewPos;
	pRender->GetCamera()->GetViewPoint( vecViewPos );

	Vector vecDelta;		
	vecDelta = m_Origin - vecViewPos;

	float flMin = min(m_flFadeMinDist, m_flFadeMaxDist);
	float flMax = max(m_flFadeMinDist, m_flFadeMaxDist);

	if (flMin < 0)
	{
		flMin = 0;
	}

	float alpha = 1.0f;
	if (flMax > 0)
	{
		float flDist = vecDelta.Length();
		if ( flDist > flMax )
		{
			alpha = 0.0f;
		}
		else if ( flDist > flMin )
		{
			alpha = RemapValClamped( flDist, flMin, flMax, 1.0f, 0 );
		}
	}
		
	return alpha;
}

//-----------------------------------------------------------------------------
// Computes fade alpha based on distance fade + screen fade
//-----------------------------------------------------------------------------
inline float CMapStudioModel::ComputeScreenFade( CRender3D *pRender ) const
{
	return 1.0;
}

inline float CMapStudioModel::ComputeFade( CRender3D *pRender ) const
{
	if ( m_bScreenSpaceFade )
	{
		return ComputeScreenFade( pRender );
	}
	else
	{
		return ComputeDistanceFade( pRender );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Renders the studio model in the 3D views.
// Input  : pRender - Interface to the 3D renderer.
//-----------------------------------------------------------------------------
void CMapStudioModel::Render3D(CRender3D *pRender)
{
#ifdef SLE //// SLE NEW - Freeze/unfreeze 
	if ( IsFrozen() && pRender->IsPicking() )
	{
		return; // frozen objects can't be picked at all
	}
#endif
	Color CurrentColor;
	CurrentColor.SetColor( r, g, b );
	
	//
	// Set to the default rendering mode, unless we're in lightmap mode
	//
#ifdef SLE
	if ( pRender->GetCurrentRenderMode() == RENDER_MODE_LIGHTMAP_GRID )
		pRender->PushRenderMode(RENDER_MODE_FLAT);
	else if (pRender->GetCurrentRenderMode() == RENDER_MODE_TEXTURED_SHADED)
		pRender->PushRenderMode(RENDER_MODE_TEXTURED_SHADED);
	else if (pRender->GetCurrentRenderMode() == RENDER_MODE_TEXTURED)
		pRender->PushRenderMode(RENDER_MODE_TEXTURED);
#else
	if (pRender->GetCurrentRenderMode() == RENDER_MODE_LIGHTMAP_GRID)
#endif
	else
		pRender->PushRenderMode(RENDER_MODE_CURRENT);

	//
	// Set up our angles for rendering.
	//
	QAngle vecAngles;
	GetRenderAngles(vecAngles);

	//
	// If we have a model, render it if it is close enough to the camera.
	//
	if (m_pStudioModel != NULL)
	{
		Vector ViewPoint;
		pRender->GetCamera()->GetViewPoint(ViewPoint);

		Vector	Origin( m_Origin );
		if ( pRender->GetInstanceRendering() )
		{
			pRender->TransformInstanceVector( m_Origin, Origin );
		}
#ifdef SLE //// SLE NEW - optimise; when picking, drastically reduce render distance (helps speed up selection a lot) // unused at the moment
		float fRenderDist = /*(pRender->IsPicking()) ? (fRenderDist = min(m_fRenderDistance, 2000)) :*/ m_fRenderDistance;
#else
		float fRenderDist = m_fRenderDistance;
#endif
#ifdef SLE //// SLE NEW: 3d skybox preview - always draw models in 3d skybox
		if ( Is3dSkybox() || GetParent() && GetParent()->Is3dSkybox() ) m_fRenderDistance = COORD_EXTENT;
#endif
		if ((fabs(ViewPoint[0] - Origin[0]) < fRenderDist) &&
			(fabs(ViewPoint[1] - Origin[1]) < fRenderDist) &&
			(fabs(ViewPoint[2] - Origin[2]) < fRenderDist))
		{
			color32 rgbColor = GetRenderColor();
			if (!(pRender->IsPicking()))
			{
				if (GetSelectionState() != SELECT_NONE)
				{
#ifdef SLE //// SLE CHANGE - separate colours
					pRender->SetDrawColor(GetRValue(Options.colors.clr2DSelection), GetGValue(Options.colors.clr2DSelection), GetBValue(Options.colors.clr2DSelection));
#else
					pRender->SetDrawColor( GetRValue(Options.colors.clrSelection), GetGValue(Options.colors.clrSelection), GetBValue(Options.colors.clrSelection) );
#endif
				}
				else
				{
					// If the user disabled collisions on this instance of the model, color the wireframe differently
					if (m_iSolid != -1)
					{
						if (m_iSolid == 0)
						{
							rgbColor.r = GetRValue(Options.colors.clrModelCollisionWireframeDisabled);
							rgbColor.g = GetGValue(Options.colors.clrModelCollisionWireframeDisabled);
							rgbColor.b = GetBValue(Options.colors.clrModelCollisionWireframeDisabled);
							rgbColor.a = 255;
						}
						else
						{
							rgbColor.r = GetRValue(Options.colors.clrModelCollisionWireframe);
							rgbColor.g = GetGValue(Options.colors.clrModelCollisionWireframe);
							rgbColor.b = GetBValue(Options.colors.clrModelCollisionWireframe);
							rgbColor.a = 255;
						}
					}
					pRender->SetDrawColor(rgbColor.r, rgbColor.g, rgbColor.b);
				}
			}
			//
			// Move the model to the proper place and orient it.
			//
			m_pStudioModel->SetAngles(vecAngles);
			m_pStudioModel->SetOrigin(m_Origin[0], m_Origin[1], m_Origin[2]);
			m_pStudioModel->SetSkin(m_Skin);
#ifdef SLE
			m_pStudioModel->SetBodygroups(m_BodyGroup); //// SLE NEW: preview bodygroups
			m_pStudioModel->SetModelScale(m_Scale); //// SLE NEW: preview model scale
			
			//// SLE NEW: 3d skybox preview
			if ( !Options.general.bShowToolsSkyFaces && ( Is3dSkybox() || m_pParent && m_pParent->Is3dSkybox() ) ) 
			{
				Vector skyDelta = vec3_origin;
				float skyScale = 16;

				CMapWorld *pWorld = ( CMapWorld * )GetWorldObject(this);
				if ( pWorld )
				{
					skyDelta = pWorld->GetSkyCameraDelta();
					skyScale = pWorld->GetSkyCameraScale();
				}

				m_pStudioModel->Set3dSkybox(true);
				m_pStudioModel->SetOrigin(( m_Origin[ 0 ] + skyDelta[ 0 ] ) * skyScale, ( m_Origin[ 1 ] + skyDelta[ 1 ] ) * skyScale, ( m_Origin[ 2 ] + skyDelta[ 2 ] ) * skyScale);
			} 
			else
				m_pStudioModel->Set3dSkybox(false);
			////
#endif
			float flAlpha = 1.0;
#ifdef SLE
			if (!pRender->IsPicking()) // don't perform calculations when picking, wastes cycles
			{
				if (Options.view3d.bPreviewModelFade)
				{
					flAlpha = ComputeFade(pRender);
				}
			}
			else
			{
				flAlpha = 0.0f;
			}
#else
			if ( Options.view3d.bPreviewModelFade )
			{
				flAlpha = ComputeFade( pRender );
			}
#endif
			bool bWireframe = pRender->GetCurrentRenderMode() == RENDER_MODE_WIREFRAME;
 
			if ( GetSelectionState() == SELECT_MODIFY 
#ifdef SLE
				&& !(pRender->IsPicking()) 
#endif
				)
			{
				bWireframe = true;
			}

			pRender->BeginRenderHitTarget(this);
#ifdef SLE //// SLE NEW: preview model rendercolor
			m_pStudioModel->DrawModel3D(pRender, m_ModelRenderColor, flAlpha, bWireframe);
#else
			m_pStudioModel->DrawModel3D(pRender, flAlpha, bWireframe );
#endif
			pRender->EndRenderHitTarget();

			if (IsSelected())
			{	
#ifdef SLE
				m_pStudioModel->DrawModel3D(pRender, m_ModelRenderColor, flAlpha, bWireframe, Options.general.bShowHelpers);

				if( Options.general.bShowHelpers)
					pRender->RenderWireframeBox(m_Render2DBox.bmins, m_Render2DBox.bmaxs, 200, 200, 50);
				else
					pRender->RenderWireframeBox(m_Render2DBox.bmins, m_Render2DBox.bmaxs, 100, 100, 25); // if showing helpers is disabled, make the bbox very faint
#else
				pRender->RenderWireframeBox(m_Render2DBox.bmins, m_Render2DBox.bmaxs, 255, 255, 0);
#endif
			}
#ifdef SLE //// SLE NEW: preview model rendercolor
			if (!(pRender->IsPicking()))
			{
				pRender->SetDrawColor(rgbColor.r, rgbColor.g, rgbColor.b);
			}
#endif
		}
		else
		{
			pRender->BeginRenderHitTarget(this);
			pRender->RenderBox(m_Render2DBox.bmins, m_Render2DBox.bmaxs, CurrentColor.r(), CurrentColor.g(), CurrentColor.b(), GetSelectionState());
			pRender->EndRenderHitTarget();
		}
	}
	//
	// Else no model, render as a bounding box.
	//
	else
	{
		pRender->BeginRenderHitTarget(this);
		pRender->RenderBox(m_Render2DBox.bmins, m_Render2DBox.bmaxs, CurrentColor.r(), CurrentColor.g(), CurrentColor.b(), GetSelectionState());
		pRender->EndRenderHitTarget();
	}

	//
	// Draw our basis vectors.
	//
#ifdef SLE //// SLE CHANGE, don't do it if helpers are disabled or when picking (wastes cycles)
	if (!pRender->IsPicking() && IsSelected() && Options.general.bShowHelpers)
#else
	if (IsSelected())
#endif
	{
		DrawBasisVectors( pRender, m_Origin, vecAngles );
#ifdef SLE //// SLE NEW - show illum position
		if ( Options.view3d.bShowIllumPosition )
		{
			Vector illumPos;
			m_pStudioModel->GetIllumPosition(illumPos);

			Vector vecViewPoint;
			pRender->GetCamera()->GetViewPoint(vecViewPoint);
			float flDist = ( illumPos - vecViewPoint ).Length();
			float size = flDist * 0.04f;
			if ( size < 2 ) size = 2;
			if ( size > 32 ) size = 32;
			pRender->PushRenderMode(RENDER_MODE_FLAT_NOZ);
			pRender->RenderSphere(m_Origin + illumPos, size, 16, 16, 255, 255, 50);
		}
#endif
	}

	pRender->PopRenderMode();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &File - 
//			bRMF - 
// Output : int
//-----------------------------------------------------------------------------
int CMapStudioModel::SerializeRMF(std::fstream &File, BOOL bRMF)
{
	return(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &File - 
//			bRMF - 
// Output : int
//-----------------------------------------------------------------------------
int CMapStudioModel::SerializeMAP(std::fstream &File, BOOL bRMF)
{
	return(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : Angles - 
//-----------------------------------------------------------------------------
void CMapStudioModel::SetAngles(QAngle &Angles)
{
	m_Angles = Angles;

	//
	// Round very small angles to zero.
	//
	for (int nDim = 0; nDim < 3; nDim++)
	{
		if (fabs(m_Angles[nDim]) < 0.001)
		{
			m_Angles[nDim] = 0;
		}
	}

	while (m_Angles[YAW] < 0)
	{
		m_Angles[YAW] += 360;
	}

	if (m_bPitchSet)
	{
		m_flPitch = m_Angles[PITCH];
	}

	//
	// Update the angles of our parent entity.
	//
	CMapEntity *pEntity = dynamic_cast<CMapEntity *>(m_pParent);
	if (pEntity != NULL)
	{
		char szValue[80];
#ifdef SLE //// SLE CHANGE - round up angles to one decimal point
		sprintf(szValue, "%.1f %.1f %.1f", m_Angles[0], m_Angles[1], m_Angles[2]);
		pEntity->NotifyChildKeyChanged(this, "angles", szValue);

		// Repeat to get rid of trailing zeros... how would I combine it into a single step?..
		pEntity->GetAngles(m_Angles);
		sprintf(szValue, "%g %g %g", (double)m_Angles[PITCH], (double)m_Angles[YAW], (double)m_Angles[ROLL]);
		pEntity->NotifyChildKeyChanged(this, "angles", szValue);

		if (m_bPitchSet)
		{
			sprintf(szValue, "%.1f", m_flPitch);
			pEntity->NotifyChildKeyChanged(this, "pitch", szValue);
		}
#else	
		sprintf(szValue, "%g %g %g", m_Angles[0], m_Angles[1], m_Angles[2]);
		pEntity->NotifyChildKeyChanged(this, "angles", szValue);

		if (m_bPitchSet)
		{
			sprintf(szValue, "%g", m_flPitch);
			pEntity->NotifyChildKeyChanged(this, "pitch", szValue);
		}
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the distance at which studio models become rendered as bounding
//			boxes. If this is set to zero, studio models are never rendered.
// Input  : fRenderDistance - Distance in world units.
//-----------------------------------------------------------------------------
void CMapStudioModel::SetRenderDistance(float fRenderDistance)
{
	m_fRenderDistance = fRenderDistance;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTransBox - 
//-----------------------------------------------------------------------------
void CMapStudioModel::DoTransform(const VMatrix &matrix)
{
	BaseClass::DoTransform(matrix);

	// rotate model angles

	matrix3x4_t fRotateMatrix, fCurrentMatrix, fMatrixNew;
	fRotateMatrix = matrix.As3x4();

	// Light entities negate pitch again!
	if ( m_bReversePitch )
	{
		QAngle rotAngles;
		MatrixAngles(fRotateMatrix, rotAngles);
		rotAngles[PITCH] *= -1;
		rotAngles[ROLL] *= -1;
		AngleMatrix(rotAngles, fRotateMatrix);
	}

	QAngle angles;
	GetAngles( angles );
	
	AngleMatrix( angles, fCurrentMatrix);
	ConcatTransforms(fRotateMatrix, fCurrentMatrix, fMatrixNew);
	MatrixAngles( fMatrixNew, angles );

	SetAngles( angles );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CMapStudioModel::GetFrame(void)
{
#ifdef SLE
	return m_sequenceFrameFromSlider;
#else
	return 0;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nFrame - 
//-----------------------------------------------------------------------------
void CMapStudioModel::SetFrame(int nFrame)
{
#ifdef SLE
	m_sequenceFrameFromSlider = nFrame;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Returns the current sequence being used for rendering.
//-----------------------------------------------------------------------------
int CMapStudioModel::GetSequence(void)
{
	if (!m_pStudioModel)
	{
		return 0;
	}
	return m_pStudioModel->GetSequence();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CMapStudioModel::GetSequenceCount(void)
{
	if (!m_pStudioModel)
	{
		return 0;
	}
	return m_pStudioModel->GetSequenceCount();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nIndex - 
//			szName - 
//-----------------------------------------------------------------------------
void CMapStudioModel::GetSequenceName(int nIndex, char *szName)
{
	if (m_pStudioModel)
	{
		m_pStudioModel->GetSequenceName(nIndex, szName);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nIndex - 
//-----------------------------------------------------------------------------
void CMapStudioModel::SetSequence(int nIndex)
{
	if (m_pStudioModel)
	{
		m_pStudioModel->SetSequence(nIndex);
	}
}

int CMapStudioModel::GetSequenceIndex( const char *pSequenceName ) const
{
	if ( m_pStudioModel )
	{
		int cnt = m_pStudioModel->GetSequenceCount();
		for ( int i=0; i < cnt; i++ )
		{
			char name[2048];
			m_pStudioModel->GetSequenceName( i, name );
			if ( Q_stricmp( pSequenceName, name ) == 0 )
				return i;
		}
	}

	return -1;
}

#ifdef SLE //// SLE NEW - expose tris count on models
int CMapStudioModel::GetTriangleCount(void)
{
	if (!m_pStudioModel)
	{
		return 0;
	}
	return m_pStudioModel->GetTriangleCount();
}
#endif

#ifdef SLE_USE_HAMMER_LPREVIEW //// taken from Hammer-2013 and adapted

extern IStudioDataCache* g_pStudioDataCache;
const vertexFileHeader_t* mstudiomodel_t::CacheVertexData( void *pModelData )
{
	return g_pStudioDataCache->CacheVertexData( (studiohdr_t*)pModelData );
}

void CMapStudioModel::AddShadowingTriangles( CUtlVector<Vector>& tri_list )
{
	// don't do it for entities with disabled shadows
//	if ( m_disableShadows)
//		return;

	// todo: filter by distance from camera, and perhaps use LODs
	if ( m_pStudioModel != NULL && ShouldAppearInRaytracedLightingPreview() )
	{
		Vector origin;
		QAngle angles;
		GetOrigin( origin );
		GetRenderAngles( angles );

		VMatrix transform;
		transform.SetupMatrixOrgAngles( origin, angles );
	//	if ( m_bExtraRotation )
	//		RotateAroundAxis( transform, 90, 2 );
		const studiohdr_t* pHdr = m_pStudioModel->GetStudioHdr()->GetRenderHdr();
		int shadowLod = m_pStudioModel->GetHardwareData()->m_NumLODs;
		studiomeshdata_t *pStudioMeshes = m_pStudioModel->GetHardwareData()->m_pLODs[shadowLod - 1].m_pMeshData;
		for ( int i = 0; i < pHdr->numbodyparts; ++i )
		{
			mstudiobodyparts_t* pBodypart = pHdr->pBodypart( i );
			for ( int j = 0; j < pBodypart->nummodels; ++j )
			{
				const mstudiomodel_t* pModel = pBodypart->pModel( j );
				for ( int k = 0; k < pModel->nummeshes; ++k )
				{
					mstudiomesh_t *pMesh = pModel->pMesh( k );
					const mstudio_meshvertexdata_t* vertData = pMesh->GetVertexData( const_cast<studiohdr_t*>( pHdr ) );
					studiomeshdata_t *pMeshData = &pStudioMeshes[pMesh->meshid];
					if ( pMeshData->m_NumGroup == 0 )
						continue;

					for ( int stripGroupID = 0; stripGroupID < pMeshData->m_NumGroup; stripGroupID++ )
					{
						studiomeshgroup_t *pMeshGroup = &pMeshData->m_pMeshGroup[stripGroupID];
						for ( int stripID = 0; stripID < pMeshGroup->m_NumStrips; stripID++ )
						{
							OptimizedModel::StripHeader_t *pStripData = &pMeshGroup->m_pStripData[stripID];

							if ( pStripData->flags & OptimizedModel::STRIP_IS_TRILIST )
							{
								for ( int i = 0; i < pStripData->numIndices; i += 3 )
								{
									int idx = pStripData->indexOffset + i;

									tri_list.AddToTail( transform.VMul4x3( *vertData->Position( pMeshGroup->MeshIndex( idx ) ) ) );
									tri_list.AddToTail( transform.VMul4x3( *vertData->Position( pMeshGroup->MeshIndex( idx + 1 ) ) ) );
									tri_list.AddToTail( transform.VMul4x3( *vertData->Position( pMeshGroup->MeshIndex( idx + 2 ) ) ) );
								}
							}
							else
							{
								Assert( pStripData->flags & OptimizedModel::STRIP_IS_TRISTRIP );
								for (int i = 0; i < pStripData->numIndices - 2; ++i)
								{
									int idx = pStripData->indexOffset + i;
									bool ccw = (i & 0x1) == 0;
									tri_list.AddToTail( transform.VMul4x3( *vertData->Position( pMeshGroup->MeshIndex( idx ) ) ) );
									tri_list.AddToTail( transform.VMul4x3( *vertData->Position( pMeshGroup->MeshIndex( idx + 1 + ccw ) ) ) );
									tri_list.AddToTail( transform.VMul4x3( *vertData->Position( pMeshGroup->MeshIndex( idx + 2 - ccw ) ) ) );
								}
							}
						}
					}
				}
			}
		}
	}
}
#endif
#ifdef SLE //// SLE TODO: SMD Export
bool CMapStudioModel::SaveSMD(ExportSMDInfo_s *pInfo, bool onlyCollision)
{
	Vector origin;
	QAngle angles;
	GetOrigin(origin);
	GetRenderAngles(angles);

	VMatrix transform;
	transform.SetupMatrixOrgAngles(origin, angles);

	const studiohdr_t* pHdr = m_pStudioModel->GetStudioHdr()->GetRenderHdr();
	int lod = Options.view3d.nLOD == -1 ? 0 : Options.view3d.nLOD;

	// look up materials, once per model	
	// todo - use g_pStudioRender->GetMaterialList or g_pStudioRender->GetMaterialListFromBodyAndSkin

	//this exports the collision - turn it into its own function later
	if (onlyCollision)
	{
		AfxMessageBox("exporting only collision");
		vcollide_t *pCollide = g_pMDLCache->GetVCollide(m_pStudioModel->GetMDLHandle());
		Vector *physicsVerts;
		int vertCount = g_pPhysicsCollision->CreateDebugMesh(pCollide->solids[0], &physicsVerts);
		int triCount = vertCount / 3;
		int vert = 0;
		VMatrix tmp = SetupMatrixOrgAngles(origin, angles);
		int i;
		for (i = 0; i < vertCount; i++)
		{
			physicsVerts[i] = tmp.VMul4x3(physicsVerts[i]);
		}
		for (i = 0; i < triCount; i++)
		{
			fprintf(pInfo->fp, "%s\n", "physbox");
			// 2-1-0 because need to flip the triangles
			CString str1, str2, str3;
			str1.Format("0 %f %f %f %f %f %f %f %f 0\n",
				physicsVerts[vert][0], physicsVerts[vert][1], physicsVerts[vert][2], 0.0, 0.0, 0.0, 0, 0);
			vert++;
			str2.Format("0 %f %f %f %f %f %f %f %f 0\n",
				physicsVerts[vert][0], physicsVerts[vert][1], physicsVerts[vert][2], 0.0, 0.0, 0.0, 0, 0);
			vert++;
			str3.Format("0 %f %f %f %f %f %f %f %f 0\n",
				physicsVerts[vert][0], physicsVerts[vert][1], physicsVerts[vert][2], 0.0, 0.0, 0.0, 0, 0);
			vert++;

			// need to flip it around or the model gets turned inside out
			fprintf(pInfo->fp, str3);
			fprintf(pInfo->fp, str2);
			fprintf(pInfo->fp, str1);
		}
		return true;
	}

	studiohwdata_t *pHardwareData = m_pStudioModel->GetHardwareData();
	if (!pHardwareData)
		return false; 

	studiomeshdata_t *pStudioMeshes = pHardwareData->m_pLODs[lod].m_pMeshData;
		
	for ( int i = 0; i < pHdr->numbodyparts; ++i )
	{
		mstudiobodyparts_t* pBodypart = pHdr->pBodypart(i);
		for ( int j = 0; j < pBodypart->nummodels; ++j )
		{
			const mstudiomodel_t* pModel = pBodypart->pModel(j);
			for ( int k = 0; k < pModel->nummeshes; ++k )
			{
				mstudiomesh_t *pMesh = pModel->pMesh(k);
				const mstudio_meshvertexdata_t* vertData = pMesh->GetVertexData(const_cast< studiohdr_t* >( pHdr ));
				studiomeshdata_t *pMeshData = &pStudioMeshes[ pMesh->meshid ];
				if ( pMeshData->m_NumGroup == 0 )
					continue;
				
				for ( int stripGroupID = 0; stripGroupID < pMeshData->m_NumGroup; stripGroupID++ )
				{
					studiomeshgroup_t *pMeshGroup = &pMeshData->m_pMeshGroup[ stripGroupID ];
					for ( int stripID = 0; stripID < pMeshGroup->m_NumStrips; stripID++ )
					{
						OptimizedModel::StripHeader_t *pStripData = &pMeshGroup->m_pStripData[ stripID ];

						if ( pStripData->flags & OptimizedModel::STRIP_IS_TRILIST )
						{
							for ( int i = 0; i < pStripData->numIndices; i += 3 )
							{
								int idx = pStripData->indexOffset + i;
								
								// Get the vert positions and normals
								Vector pos1, pos2, pos3, normal1, normal2, normal3;
								// the order should be reversed or the model comes out flipped inside out
								pos1 = transform.VMul4x3(*vertData->Position(pMeshGroup->MeshIndex(idx + 2)));
								pos2 = transform.VMul4x3(*vertData->Position(pMeshGroup->MeshIndex(idx + 1)));
								pos3 = transform.VMul4x3(*vertData->Position(pMeshGroup->MeshIndex(idx)));
								
								normal1 = transform.VMul4x3(*vertData->Normal(pMeshGroup->MeshIndex(idx + 2)));
								normal2 = transform.VMul4x3(*vertData->Normal(pMeshGroup->MeshIndex(idx + 1)));
								normal3 = transform.VMul4x3(*vertData->Normal(pMeshGroup->MeshIndex(idx)));
																																								
								mstudiovertex_t *pVertices = vertData->Vertex(i);
							//	normal = pVertices->m_vecNormal;		

								Vector2D uv1 = Vector2D(0,0);
								Vector2D uv2 = Vector2D(0, 0);
								Vector2D uv3 = Vector2D(0, 0);

								// get the texture coords for each vert in order
								uv1[0] = vertData->Vertex(pMeshGroup->MeshIndex(idx + 2))->m_vecTexCoord[0];
								uv1[1] = vertData->Vertex(pMeshGroup->MeshIndex(idx + 2))->m_vecTexCoord[1];
								uv2[0] = vertData->Vertex(pMeshGroup->MeshIndex(idx + 1))->m_vecTexCoord[0];
								uv2[1] = vertData->Vertex(pMeshGroup->MeshIndex(idx + 1))->m_vecTexCoord[1];
								uv3[0] = vertData->Vertex(pMeshGroup->MeshIndex(idx))->m_vecTexCoord[0];
								uv3[1] = vertData->Vertex(pMeshGroup->MeshIndex(idx))->m_vecTexCoord[1];
								
								m_pStudioModel->GetStudioHdr()->numtextures();

								// get the texture name
								// note, currently only using the current LOD. This may change in the future
								char texture[260];								
								Q_snprintf(texture, sizeof(texture), "%s", pHardwareData->m_pLODs[lod].ppMaterials[k]->GetName());
								// cut off the last part, since the full path includes models/<model dir>
								Q_FileBase(texture, texture, sizeof(texture));
																
								// Y UV is negative because it needs to be flipped
								fprintf(pInfo->fp, "%s\n", texture);
								fprintf(pInfo->fp, "0 %f %f %f %f %f %f %f %f 0\n", pos1[ 0 ], pos1[ 1 ], pos1[ 2 ], 
									normal1[0], normal1[1], normal1[2], uv1[ 0 ], -uv1[ 1 ]);
								fprintf(pInfo->fp, "0 %f %f %f %f %f %f %f %f 0\n", pos2[ 0 ], pos2[ 1 ], pos2[ 2 ], 
									normal2[0], normal2[1], normal2[2], uv2[ 0 ], -uv2[ 1 ]);
								fprintf(pInfo->fp, "0 %f %f %f %f %f %f %f %f 0\n", pos3[ 0 ], pos3[ 1 ], pos3[ 2 ], 
									normal3[0], normal3[1], normal3[2], uv3[ 0 ], -uv3[ 1 ]);
							}
						} 
						else
						{
							Assert(pStripData->flags & OptimizedModel::STRIP_IS_TRISTRIP);
							for ( int i = 0; i < pStripData->numIndices - 2; ++i )
							{
								int idx = pStripData->indexOffset + i;
								bool ccw = ( i & 0x1 ) == 0;

								Vector pos1, pos2, pos3, normal;
								pos1 = transform.VMul4x3(*vertData->Position(pMeshGroup->MeshIndex(idx)));
								pos2 = transform.VMul4x3(*vertData->Position(pMeshGroup->MeshIndex(idx + 1 + ccw)));
								pos3 = transform.VMul4x3(*vertData->Position(pMeshGroup->MeshIndex(idx + 2 - ccw)));

								Vector2D uv1 = Vector2D(0, 0);
								Vector2D uv2 = Vector2D(0, 0);
								Vector2D uv3 = Vector2D(0, 0);

								// get the texture coords for each vert in order
								uv1[0] = vertData->Vertex(pMeshGroup->MeshIndex(idx))->m_vecTexCoord[0];
								uv1[1] = vertData->Vertex(pMeshGroup->MeshIndex(idx))->m_vecTexCoord[1];
								uv2[0] = vertData->Vertex(pMeshGroup->MeshIndex(idx + 1 + ccw))->m_vecTexCoord[0];
								uv2[1] = vertData->Vertex(pMeshGroup->MeshIndex(idx + 1 + ccw))->m_vecTexCoord[1];
								uv3[0] = vertData->Vertex(pMeshGroup->MeshIndex(idx + 2 - ccw))->m_vecTexCoord[0];
								uv3[1] = vertData->Vertex(pMeshGroup->MeshIndex(idx + 2 - ccw))->m_vecTexCoord[1];

								fprintf(pInfo->fp, "%s\n", "test");
								fprintf(pInfo->fp, "0 %f %f %f %f %f %f %f %f 0\n", pos1[ 0 ], pos1[ 1 ], pos1[ 2 ], 
									0.0, 0.0, 0.0, uv1[ 0 ], -uv1[ 1 ]);
								fprintf(pInfo->fp, "0 %f %f %f %f %f %f %f %f 0\n", pos2[ 0 ], pos2[ 1 ], pos2[ 2 ], 
									0.0, 0.0, 0.0, uv2[ 0 ], -uv2[ 1 ]);
								fprintf(pInfo->fp, "0 %f %f %f %f %f %f %f %f 0\n", pos3[ 0 ], pos3[ 1 ], pos3[ 2 ], 
									0.0, 0.0, 0.0, uv3[ 0 ], -uv3[ 1 ]);
							}
						}
					}
				}
			}
		}
	}

	return true;
}
#endif