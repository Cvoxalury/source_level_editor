//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "stdafx.h"
#include "mapdoc.h"
#include "MapWorld.h"
#include "Render2D.h"
#include "Render3D.h"
#include "StudioModel.h"
#include "bone_setup.h"
#include "IStudioRender.h"
#include "bone_accessor.h"
#include "optimize.h"
#include "HammerVGui.h"
#include "mapview2d.h"
//// not needed for compilation
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <malloc.h>
//#include "Material.h"
//#include "Render.h"
//#include "ViewerSettings.h"
//#include "materialsystem/imesh.h"
//#include "materialsystem/MaterialSystem_Config.h"
//#include "TextureSystem.h"
//#include "GlobalFunctions.h"
//#include "UtlMemory.h"
//#include "utldict.h"
//#include "filesystem.h"
//#include "Hammer.h"
//#include <VGuiMatSurface/IMatSystemSurface.h>
//#include "mapdefs.h"
//#include "camera.h"
//#include "options.h"
//#include "Render.h"
//#include <keyvalues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#pragma warning(disable : 4244) // double to float

//-----------------------------------------------------------------------------
// Purpose: Monitors the filesystem for changes to model files and flushes
// any stuff in memory for the model if necessary.
//-----------------------------------------------------------------------------
class CStudioFileChangeWatcher : private CFileChangeWatcher::ICallbacks
{
public:
	void Init();
	void Update();	// Call this periodically to update.

private:
	// CFileChangeWatcher::ICallbacks..
	virtual void OnFileChange( const char *pRelativeFilename, const char *pFullFilename );

private:
	CFileChangeWatcher m_Watcher;
	CUtlDict<int,int> m_ChangedModels;
};
static CStudioFileChangeWatcher g_StudioFileChangeWatcher;

Vector			g_lightvec;						// light vector in model reference frame
Vector			g_blightvec[MAXSTUDIOBONES];	// light vectors in bone reference frames
int				g_ambientlight;					// ambient world light
float			g_shadelight;					// direct world light
Vector			g_lightcolor;
bool			g_bUpdateBones2D = true;

//-----------------------------------------------------------------------------
// Model meshes themselves are cached to avoid redundancy. There should never be
// more than one copy of a given studio model in memory at once.
//-----------------------------------------------------------------------------
ModelCache_t CStudioModelCache::m_Cache[65536];
int CStudioModelCache::m_nItems = 0;

//-----------------------------------------------------------------------------
// Purpose: Find a model in the cache. Returns null if it's not in the cache.
//-----------------------------------------------------------------------------
StudioModel *CStudioModelCache::FindModel(const char *pszModelPath)
{
	char testPath[MAX_PATH];
	V_strncpy( testPath, pszModelPath, sizeof( testPath ) );
	V_FixSlashes( testPath );
		
	//
	// First look for the model in the cache. If it's there, increment the
	// reference count and return a pointer to the cached model.
	//
	for (int i = 0; i < m_nItems; i++)
	{
		char testPath2[MAX_PATH];
		V_strncpy( testPath2, m_Cache[i].pszPath, sizeof( testPath2 ) );
		V_FixSlashes( testPath2 );
		
		if (!stricmp(testPath, testPath2))
		{
			m_Cache[i].nRefCount++;
			return(m_Cache[i].pModel);
		}
	}
	
	return NULL;
}
#ifdef SLE //// SLE NEW - ported from sdk-2013-hammmer
//-----------------------------------------------------------------------------
// Purpose: Find all models in the cache with the same model name and reload them
//-----------------------------------------------------------------------------
void CStudioModelCache::ReloadModel(const char *pszModelPath)
{
	// Fix up our name to match what will be in the cache
	char testPath[MAX_PATH];
	V_strncpy(testPath, pszModelPath, sizeof(testPath));
	V_FixSlashes(testPath);

	// Look through all models in the cache, updating them as we find them
	for (int i = 0; i < m_nItems; i++)
	{
		char testPath2[MAX_PATH];
		V_strncpy(testPath2, m_Cache[i].pszPath, sizeof(testPath2));
		V_FixSlashes(testPath2);

		// If it's a match, reload it
		if (!stricmp(testPath, testPath2))
		{
			m_Cache[i].pModel->FreeModel();
			m_Cache[i].pModel->LoadModel(pszModelPath);
		}
	}
}
#endif
//-----------------------------------------------------------------------------
// Purpose: Returns an instance of a particular studio model. If the model is
//			in the cache, a pointer to that model is returned. If not, a new one
//			is created and added to the cache.
// Input  : pszModelPath - Full path of the .MDL file.
//-----------------------------------------------------------------------------
StudioModel *CStudioModelCache::CreateModel(const char *pszModelPath)
{
#ifndef SLE //// SLE CHANGE - make the cache always consider a model to be new, 
			//// this makes individual animations on prop dynamics work.
			//// TODO: is this the best we can come up with?..
	StudioModel *pTest = FindModel( pszModelPath );
	if ( pTest )
		return pTest;
#endif
	//
	// If it isn't there, try to create one.
	//
	StudioModel *pModel = new StudioModel;

	if (pModel != NULL)
	{
		bool bLoaded = pModel->LoadModel(pszModelPath);

		if (bLoaded)
		{
			bLoaded = pModel->PostLoadModel(pszModelPath);
		}

		if (!bLoaded)
		{
			delete pModel;
			pModel = NULL;
		}
	}

	//
	// If we successfully created it, add it to the cache.
	//
	if (pModel != NULL)
	{
		CStudioModelCache::AddModel(pModel, pszModelPath);
	}

	return(pModel);
}

//-----------------------------------------------------------------------------
// Purpose: Adds the model to the cache, setting the reference count to one.
// Input  : pModel - Model to add to the cache.
//			pszModelPath - The full path of the .MDL file, which is used as a
//				key in the model cache.
// Output : Returns TRUE if the model was successfully added, FALSE if we ran
//			out of memory trying to add the model to the cache.
//-----------------------------------------------------------------------------
BOOL CStudioModelCache::AddModel(StudioModel *pModel, const char *pszModelPath)
{
	//
	// Copy the model pointer.
	//
	m_Cache[m_nItems].pModel = pModel;

	//
	// Allocate space for and copy the model path.
	//
	m_Cache[m_nItems].pszPath = new char [strlen(pszModelPath) + 1];
	if (m_Cache[m_nItems].pszPath != NULL)
	{
		strcpy(m_Cache[m_nItems].pszPath, pszModelPath);
	}
	else
	{
		return(FALSE);
	}

	m_Cache[m_nItems].nRefCount = 1;

	m_nItems++;

	return(TRUE);
}

//-----------------------------------------------------------------------------
// Purpose: Advances the animation of all models in the cache for the given interval.
// Input  : flInterval - delta time in seconds.
//-----------------------------------------------------------------------------
void CStudioModelCache::AdvanceAnimation(float flInterval)
{
	for (int i = 0; i < m_nItems; i++)
	{
		m_Cache[i].pModel->AdvanceFrame(flInterval);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Increments the reference count on a model in the cache. Called by
//			client code when a pointer to the model is copied, making that
//			reference independent.
// Input  : pModel - Model for which to increment the reference count.
//-----------------------------------------------------------------------------
void CStudioModelCache::AddRef(StudioModel *pModel)
{
	for (int i = 0; i < m_nItems; i++)
	{
		if (m_Cache[i].pModel == pModel)
		{
			m_Cache[i].nRefCount++;
			return;
		}
	}	
}

//-----------------------------------------------------------------------------
// Purpose: Called by client code to release an instance of a model. If the
//			model's reference count is zero, the model is freed.
// Input  : pModel - Pointer to the model to release.
//-----------------------------------------------------------------------------
void CStudioModelCache::Release(StudioModel *pModel)
{
	for (int i = 0; i < m_nItems; i++)
	{
		if (m_Cache[i].pModel == pModel)
		{
			m_Cache[i].nRefCount--;
			Assert(m_Cache[i].nRefCount >= 0);

			//
			// If this model is no longer referenced, free it and remove it
			// from the cache.
			//
			if (m_Cache[i].nRefCount <= 0)
			{
				//
				// Free the path, which was allocated by AddModel.
				//
				delete [] m_Cache[i].pszPath;
				delete m_Cache[i].pModel;

				//
				// Decrement the item count and copy the last element in the cache over
				// this element.
				//
				m_nItems--;

				m_Cache[i].pModel = m_Cache[m_nItems].pModel;
				m_Cache[i].pszPath = m_Cache[m_nItems].pszPath;
				m_Cache[i].nRefCount = m_Cache[m_nItems].nRefCount;
			}

			break;
		}
	}	
}

//-----------------------------------------------------------------------------
// Purpose: Watch for changes to studio models and reload them if necessary.
//-----------------------------------------------------------------------------
void CStudioFileChangeWatcher::Init()
{
	m_Watcher.Init( this );
	
	char searchPaths[1024 * 16];
	if ( g_pFullFileSystem->GetSearchPath( "GAME", false, searchPaths, sizeof( searchPaths ) ) > 0 )
	{
		CUtlVector<char*> searchPathList;
		V_SplitString( searchPaths, ";", searchPathList );

		for ( int i=0; i < searchPathList.Count(); i++ )
		{
#ifdef SLE //// SLE NEW - no point searching in vpks
			if (V_stristr(searchPathList[i], ".vpk"))
				continue;
#endif
			m_Watcher.AddDirectory( searchPathList[i], "models", true );
		}
		
		searchPathList.PurgeAndDeleteElements();
	}
	else
	{
#ifdef SLE //// SLE CHANGE - renamed to differentiate the program
		Warning( "Error in GetSearchPath. The editor will not automatically reload modified models." );
#else
		Warning( "Error in GetSearchPath. Hammer will not automatically reload modified models." );
#endif
	}
}

void CStudioFileChangeWatcher::OnFileChange( const char *pRelativeFilename, const char *pFullFilename )
{
	char relativeFilename[MAX_PATH];
	V_ComposeFileName( "models", pRelativeFilename, relativeFilename, sizeof( relativeFilename ) );
	V_FixSlashes( relativeFilename );
	
	// Check the cache.
	const char *pExt = V_GetFileExtension( relativeFilename );
	if ( !pExt )
		return;			 
		
	if ( V_stricmp( pExt, "mdl" ) == 0 ||
		 V_stricmp( pExt, "vtx" ) == 0 ||
		 V_stricmp( pExt, "phy" ) == 0 ||
		 V_stricmp( pExt, "vvd" ) == 0 )
	{
		// Ok, it's at least related to a model. Flush out the model.
		char tempFilename[MAX_PATH];
		V_strncpy( tempFilename, relativeFilename, pExt - relativeFilename );
		
		// Now it might have a "dx80" or "dx90" or some other extension. Get rid of that too.
		const char *pTestFilename = V_UnqualifiedFileName( tempFilename );
		pExt = V_GetFileExtension( pTestFilename );
		char filename[MAX_PATH];
		if ( pExt )
			V_strncpy( filename, tempFilename, pExt - tempFilename );
		else
			V_strncpy( filename, tempFilename, sizeof( filename ) );

		// Now we've got the filename with any extension or "dx80"-type stuff at the end.
		V_strncat( filename, ".mdl", sizeof( filename ) );
		
		// Queue up the list of changes because if they copied all the files for a model,
		// we'd like to only reload it once. //// SLE TODO - see if this is responsible for sometimes not reloading
		if ( m_ChangedModels.Find( filename ) == m_ChangedModels.InvalidIndex() )
			m_ChangedModels.Insert( filename );
	}
}

void CStudioFileChangeWatcher::Update()
{
	if ( !g_pMDLCache )
		return;

	m_Watcher.Update();

	if ( m_ChangedModels.Count() > 0 )
	{
		// Reload whatever models were changed.
		for ( int i=m_ChangedModels.First(); i != m_ChangedModels.InvalidIndex(); i=m_ChangedModels.Next( i ) )
		{
			const char *pName = m_ChangedModels.GetElementName( i );
				
			MDLHandle_t hModel = g_pMDLCache->FindMDL( pName );
			g_pMDLCache->Flush( hModel );
			g_pMDLCache->ResetErrorModelStatus( hModel );

#ifdef SLE	//// SLE CHANGE - different model reloading, from sdk-2013-hammer
			// Find all studio models and refresh their data
			CStudioModelCache::ReloadModel(pName);
#else
			// If we have it in the StudioModel cache, flush its data.
			StudioModel *pTest = CStudioModelCache::FindModel( pName );
			if ( pTest )
			{
				pTest->FreeModel();
				pTest->LoadModel( pName );
			}
#endif
		}
		
		m_ChangedModels.Purge();	
	
		for ( int i=0; i < CMapDoc::GetDocumentCount(); i++ )
		{
			CMapDoc *pDoc = CMapDoc::GetDocument( i );
			pDoc->GetMapWorld()->CalcBounds( true );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Loads up the IStudioRender interface.
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool StudioModel::Initialize()
{
	return true;
}

void StudioModel::Shutdown( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
StudioModel::StudioModel(void) : m_pModelName(0)
{
	int i;

	m_origin.Init();
	m_angles.Init();
	m_sequence = 0;
	m_cycle = 0;
	m_bodynum = 0;
	m_skinnum = 0;
#ifdef SLE
	m_modelScale = 1.0f; //// SLE NEW: preview model scale
	m_is3dskybox = false; //// SLE NEW: 3d skybox preview
#endif
	for (i = 0; i < sizeof(m_controller) / sizeof(m_controller[0]); i++)
	{
		m_controller[i] = 0;
	}

	for (i = 0; i < sizeof(m_poseParameter) / sizeof(m_poseParameter[0]); i++)
	{
		m_poseParameter[i] = 0;
	}

	m_mouth = 0;

	m_MDLHandle = MDLHANDLE_INVALID;
	m_pModel = NULL;
	m_pStudioHdr = NULL;
	m_pPosePos = NULL;
	m_pPoseAng = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor. Frees dynamically allocated data.
//-----------------------------------------------------------------------------
StudioModel::~StudioModel(void)
{
	FreeModel();
	if (m_pModelName)
	{
		delete[] m_pModelName;
	}
	delete m_pStudioHdr;

	delete []m_pPosePos;
	delete []m_pPoseAng;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the Euler angles for the model.
// Input  : fAngles - A pointer to engine PITCH, YAW, and ROLL angles.
//-----------------------------------------------------------------------------
void StudioModel::SetAngles(QAngle& pfAngles)
{
	m_angles[PITCH] = pfAngles[PITCH];
	m_angles[YAW] = pfAngles[YAW];
	m_angles[ROLL] = pfAngles[ROLL];
}

void StudioModel::AdvanceFrame( float dt )
{
	if (dt > 0.1)
		dt = 0.1f;

	CStudioHdr *pStudioHdr = GetStudioHdr();
	float t = Studio_Duration( pStudioHdr, m_sequence, m_poseParameter );

	if (t > 0)
	{
		m_cycle += dt / t;

		// wrap
		m_cycle -= (int)(m_cycle);
	}
	else
	{
		m_cycle = 0;
	}
}
#ifdef SLE
//-----------------------------------------------------------------------------
// Purpose: Keeps a global clock for "realtime" overlays to run from
//-----------------------------------------------------------------------------
float GetRealtimeTime(void)
{
	// renamed static's so debugger doesn't get confused and show the wrong one
	static int g_prevTicksRT;
	static float g_timeRT;

	int ticks = GetTickCount();
	// limit delta so that float time doesn't overflow
	if (g_prevTicksRT == 0)
		g_prevTicksRT = ticks;

	g_timeRT += ((ticks - g_prevTicksRT) / 1000.0f);
	g_prevTicksRT = ticks;

	return g_timeRT;
}
#endif
void StudioModel::SetUpBones( bool bUpdatePose, matrix3x4_t *pBoneToWorld )
{
	CStudioHdr *pStudioHdr = GetStudioHdr();

	if ( m_pPosePos == NULL )
	{
		bUpdatePose = true;
		m_pPosePos = new Vector[pStudioHdr->numbones()] ;
		m_pPoseAng = new Quaternion[pStudioHdr->numbones()];
	}
	
	if ( bUpdatePose )
	{
		IBoneSetup boneSetup( pStudioHdr, BONE_USED_BY_ANYTHING, m_poseParameter );
		boneSetup.InitPose( m_pPosePos, m_pPoseAng );
		boneSetup.AccumulatePose( m_pPosePos, m_pPoseAng, m_sequence, m_cycle, 1.0f, 0.0f, NULL );
	}
	
	mstudiobone_t *pbones = pStudioHdr->pBone( 0 );

	matrix3x4_t cameraTransform;
	AngleMatrix( m_angles, cameraTransform );
	cameraTransform[0][3] = m_origin[0];
	cameraTransform[1][3] = m_origin[1];
	cameraTransform[2][3] = m_origin[2];

	for (int i = 0; i < pStudioHdr->numbones(); i++) 
	{
		if ( CalcProceduralBone( pStudioHdr, i, CBoneAccessor( pBoneToWorld ) ))
			continue;

		matrix3x4_t	bonematrix;

		QuaternionMatrix( m_pPoseAng[i], bonematrix );
		
		bonematrix[0][3] = m_pPosePos[i][0];
		bonematrix[1][3] = m_pPosePos[i][1];
		bonematrix[2][3] = m_pPosePos[i][2];
#ifdef SLE	//// SLE NEW: Jiggle bones //// SLE TODO - can't work right now
		/*
		if ((pStudioHdr->pBone(0)[i].flags & BONE_ALWAYS_PROCEDURAL) &&
			(pStudioHdr->pBone(0)[i].proctype & STUDIO_PROC_JIGGLE))
		{
			//
			// Physics-based "jiggle" bone
			// Bone is assumed to be along the Z axis
			// Pitch around X, yaw around Y
			//

			// compute desired bone orientation
			matrix3x4_t goalMX;

			if (pbones[i].parent == -1)
			{
				ConcatTransforms(cameraTransform, bonematrix, goalMX);
			}
			else
			{
				ConcatTransforms(pBoneToWorld[pbones[i].parent], bonematrix, goalMX);
			}

			// get jiggle properties from QC data
			mstudiojigglebone_t *jiggleInfo = (mstudiojigglebone_t *)pStudioHdr->pBone(0)[i].pProcedure();

			if (!m_pJiggleBones)
			{
				m_pJiggleBones = new CJiggleBones;
			}

			// do jiggle physics
			m_pJiggleBones->BuildJiggleTransformations(i, GetRealtimeTime(), jiggleInfo, goalMX, pBoneToWorld[i]);
		}
		////
		else*/ 
#endif //// SLE	
		if (pbones[i].parent == -1)
		{
			ConcatTransforms( cameraTransform, bonematrix, pBoneToWorld[ i ] );
#ifdef SLE //// SLE NEW: preview model scale
			if (pStudioHdr->numbones() == 1) continue; // for static props, otherwise they get scaled wrongly
			
			VectorScale(pBoneToWorld[i][0], m_modelScale, pBoneToWorld[i][0]);
			VectorScale(pBoneToWorld[i][1], m_modelScale, pBoneToWorld[i][1]);
			VectorScale(pBoneToWorld[i][2], m_modelScale, pBoneToWorld[i][2]);
#endif
		}
		else 
		{
			ConcatTransforms ( pBoneToWorld[ pbones[i].parent ], bonematrix, pBoneToWorld[ i ] );
#ifdef SLE //// SLE NEW: preview model scale
		/* leads to very funny results in hammer
			VectorScale(pBoneToWorld[i][0], m_modelScale, pBoneToWorld[i][0]);
			VectorScale(pBoneToWorld[i][1], m_modelScale, pBoneToWorld[i][1]);
			VectorScale(pBoneToWorld[i][2], m_modelScale, pBoneToWorld[i][2]);
		*/
#endif
		}
	}
}

/*
=================
StudioModel::SetupModel
	based on the body part, figure out which mesh it should be using.
inputs:
	currententity
outputs:
	pstudiomesh
	pmdl
=================
*/

void StudioModel::SetupModel ( int bodypart )
{
	int index;

	CStudioHdr *pStudioHdr = GetStudioHdr();
	if (bodypart > pStudioHdr->numbodyparts())
	{
		// Con_DPrintf ("StudioModel::SetupModel: no such bodypart %d\n", bodypart);
		bodypart = 0;
	}
	
	mstudiobodyparts_t   *pbodypart = pStudioHdr->pBodypart( bodypart );

	index = m_bodynum / pbodypart->base;
	index = index % pbodypart->nummodels;

	m_pModel = pbodypart->pModel( index );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef HAMMER2013_PORT_PROXIES
void StudioModel::DrawModel3D(CRender3D *pRender, const Color &color, float flAlpha, bool bWireframe, bool bSelectionOverlay, CMapClass* pParent)
#else
void StudioModel::DrawModel3D(CRender3D *pRender, const Color &color, float flAlpha, bool bWireframe, bool bSelectionOverlay)
#endif
{
	studiohdr_t *pStudioHdr = GetStudioRenderHdr();
	if (!pStudioHdr)
		return;

	if (pStudioHdr->numbodyparts == 0)
		return;

	CMatRenderContextPtr pRenderContext(g_pMaterialSystem);

	DrawModelInfo_t info;
	info.m_pStudioHdr = pStudioHdr;
	info.m_pHardwareData = GetHardwareData();
	info.m_Decals = STUDIORENDER_DECAL_INVALID;
	info.m_Skin = m_skinnum;
	info.m_Body = m_bodynum;
	info.m_HitboxSet = 0;
	info.m_pClientEntity = NULL;
#ifdef SLE //// SLE NEW - LOD control option
	if ( Options.view3d.nLOD > -1 )
	{
		info.m_Lod = pRender->IsPicking() ? 1 : Options.view3d.nLOD; //// SLE NEW - LOD control option; display at lower lod during picking to speed up selection
	}
	else
	{
		info.m_Lod = Options.view3d.nLOD;
	}
#else
	info.m_Lod = -1;
#endif
	info.m_pColorMeshes = NULL;
#ifdef HAMMER2013_PORT_PROXIES
	CRender::ProxyData proxyData{ pRender, pParent };
	info.m_pClientEntity = &proxyData;
#else
	info.m_pClientEntity = NULL;
#endif
#ifdef SLE ///// SLE NEW - BSP Lighting
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	info.m_bStaticLighting = (false); //(pDoc && pDoc->GetBSPLighting());
#endif
	if (pRender->IsInLocalTransformMode())
	{
		// WHACKY HACKY
		Vector orgOrigin = m_origin;
		QAngle orgAngles = m_angles;

		VMatrix matrix;
		pRender->GetLocalTranform(matrix);

		// baseclass rotates the origin
		matrix.V3Mul(orgOrigin, m_origin);

		matrix3x4_t fCurrentMatrix, fMatrixNew;
		AngleMatrix(m_angles, fCurrentMatrix);
		ConcatTransforms(matrix.As3x4(), fCurrentMatrix, fMatrixNew);

		QAngle newAngles;
		MatrixAngles(fMatrixNew, m_angles);

		matrix3x4_t boneToWorld[MAXSTUDIOBONES];

		SetUpBones(false, boneToWorld);

#ifdef SLE //// SLE NEW: preview model scale
		// Scale the base transform if we don't have a bone hierarchy
		CStudioHdr *boneStudioHdr = GetStudioHdr();

		if (boneStudioHdr && boneToWorld && boneStudioHdr->numbones() == 1)
		{
			// Scale the bone to world at this point
			float flScale = m_modelScale;

			//// SLE NEW: 3d skybox preview // todo, pass the real camera scale here
			if( !Options.general.bShowToolsSkyFaces && m_is3dskybox )
			{
				CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
				if ( pDoc )
				{
					CMapWorld *pWorld = pDoc->GetMapWorld();
					if ( pWorld )
					{
						flScale *= pWorld->GetSkyCameraScale();
					}
				}
			}

			VectorScale((*boneToWorld)[0], flScale, (*boneToWorld)[0]);
			VectorScale((*boneToWorld)[1], flScale, (*boneToWorld)[1]);
			VectorScale((*boneToWorld)[2], flScale, (*boneToWorld)[2]);
		}

		if (pRender->IsPicking()) // a little faster
			g_pStudioRender->DrawModel(NULL, info, boneToWorld, 0, 0, m_origin);
		else
			pRender->DrawModel(&info, boneToWorld, m_origin, flAlpha, bWireframe, color);
#else
		pRender->DrawModel( &info, boneToWorld, m_origin, flAlpha, bWireframe );	
#endif
		m_origin = orgOrigin;
		m_angles = orgAngles;
	}
	else
	{
		matrix3x4_t boneToWorld[MAXSTUDIOBONES];
		SetUpBones(true, boneToWorld);
#ifdef SLE //// SLE NEW: preview model scale
		// Scale the base transform if we don't have a bone hierarchy
		CStudioHdr *boneStudioHdr = GetStudioHdr();

		if (boneStudioHdr && boneToWorld && boneStudioHdr->numbones() == 1)
		{
			// Scale the bone to world at this point
			float flScale = m_modelScale;

			//// SLE NEW: 3d skybox preview
			if( !Options.general.bShowToolsSkyFaces && m_is3dskybox )
			{
				CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
				if ( pDoc )
				{
					CMapWorld *pWorld = pDoc->GetMapWorld();
					if ( pWorld )
					{
						flScale *= pWorld->GetSkyCameraScale();
					}
				}
			}

			VectorScale((*boneToWorld)[0], flScale, (*boneToWorld)[0]);
			VectorScale((*boneToWorld)[1], flScale, (*boneToWorld)[1]);
			VectorScale((*boneToWorld)[2], flScale, (*boneToWorld)[2]);
		}

		if (pRender->GetCurrentRenderMode() == RENDER_MODE_FLAT) //// SLE NEW - in Flat mode, draw models with flat override
		{
			if ( !pRender->IsPicking() )
			{
				pRender->PushRenderMode(RENDER_MODE_CURRENT);
				IMaterial* overlay = materials->FindMaterial("../../bin/level_editor/materials/editor/modelflatshaded", TEXTURE_GROUP_OTHER, false);
				if ( overlay )
				{
					overlay->AddRef();
				}
				g_pStudioRender->ForcedMaterialOverride(overlay, OVERRIDE_NORMAL);
				float color[ 4 ] = { 1, 1, 1, 1 };
				if ( GetTriangleCount() <= 1000 ) // tint the models in flat rendermode according to their polycount - the higher, the redder
					color[ 0 ] += 0.0;
				else if ( GetTriangleCount() <= 2000 )
				{
					color[ 0 ] = 1;
					color[ 1 ] = 0.83;
					color[ 2 ] = 0.83;
				} else if ( GetTriangleCount() <= 4000 )
				{
					color[ 0 ] = 1;
					color[ 1 ] = 0.75;
					color[ 2 ] = 0.75;
				} else if ( GetTriangleCount() <= 6000 )
				{
					color[ 0 ] = 1;
					color[ 1 ] = 0.5;
					color[ 2 ] = 0.5;
				} else if ( GetTriangleCount() <= 8000 )
				{
					color[ 0 ] = 1;
					color[ 1 ] = 0.25;
					color[ 2 ] = 0.25;
				} else if ( GetTriangleCount() <= 10000 )
				{
					color[ 0 ] = 1;
					color[ 1 ] = 0.13;
					color[ 2 ] = 0.25;
				} else if ( GetTriangleCount() <= 15000 )
				{
					color[ 0 ] = 1;
					color[ 1 ] = 0;
					color[ 2 ] = 0.5;
				} else if ( GetTriangleCount() <= 20000 )
				{
					color[ 0 ] = 0.75;
					color[ 1 ] = 0;
					color[ 2 ] = 0.75;
				} else if ( GetTriangleCount() > 20000 )
				{
					color[ 0 ] = 0.5;
					color[ 1 ] = 0;
					color[ 2 ] = 1.00;
				}
				g_pStudioRender->SetColorModulation(color);
				g_pStudioRender->DrawModel(NULL, info, boneToWorld, 0, 0, m_origin);
				float reset[ 4 ] = { 1, 1, 1, 1 };
				g_pStudioRender->SetColorModulation(reset);
				g_pStudioRender->ForcedMaterialOverride(NULL);
				pRender->PopRenderMode();
			}
			else
				g_pStudioRender->DrawModel(NULL, info, boneToWorld, 0, 0, m_origin);
		}
		else
		{
			if( pRender->IsPicking()) // a little faster
				g_pStudioRender->DrawModel(NULL, info, boneToWorld, 0, 0, m_origin);
			else
				pRender->DrawModel(&info, boneToWorld, m_origin, flAlpha, bWireframe, color);
		}

		//// SLE NEW - draw overlaying model in solid colour as selection overlay
		if (!(pRender->IsPicking()) && bSelectionOverlay)
		{			
			IMaterial* overlay = materials->FindMaterial("../../bin/level_editor/materials/editor/modelselectionoverlay", TEXTURE_GROUP_OTHER, false);
			if (overlay)
			{
				overlay->AddRef();
			}

			float color[4] = { GetRValue(Options.colors.clrModelSelection) / 100, GetGValue(Options.colors.clrModelSelection) / 100, GetBValue(Options.colors.clrModelSelection) / 100, 1 };
			g_pStudioRender->SetColorModulation(color);
			g_pStudioRender->ForcedMaterialOverride(overlay, OVERRIDE_NORMAL);
			g_pStudioRender->DrawModel(NULL, info, boneToWorld, 0, 0, m_origin);
			g_pStudioRender->ForcedMaterialOverride(NULL);
			color[0] = color[1] = color[2] = color[3] = 1;
			g_pStudioRender->SetColorModulation(color);
		}
#else
		pRender->DrawModel( &info, boneToWorld, m_origin, flAlpha, bWireframe );
#endif //// SLE
		
		if (Options.general.bShowCollisionModels && !(pRender->IsPicking()))
		{
			VMatrix mViewMatrix = SetupMatrixOrgAngles(m_origin, m_angles);
			pRender->DrawCollisionModel(m_MDLHandle, mViewMatrix);
		}
	}
}
#ifdef HAMMER2013_PORT_PROXIES
void StudioModel::DrawModel2D(CRender2D *pRender, float flAlpha, bool bWireFrame, CMapClass *pParent)
#else
void StudioModel::DrawModel2D( CRender2D *pRender, float flAlpha, bool bWireFrame  )
#endif
{
	studiohdr_t *pStudioHdr = GetStudioRenderHdr();
	if (!pStudioHdr)
		return;

	if (pStudioHdr->numbodyparts == 0)
		return;

	Vector orgOrigin = m_origin;
	QAngle orgAngles = m_angles;
#ifdef SLE //// SLE CHANGE - apply lod depending on 2d view zoom
	CMapView2D *pView = (CMapView2D*)pRender->GetView();
#endif
	DrawModelInfo_t info;
	info.m_pStudioHdr = pStudioHdr;
	info.m_pHardwareData = GetHardwareData();
	info.m_Decals = STUDIORENDER_DECAL_INVALID;
	info.m_Skin = m_skinnum;
	info.m_Body = m_bodynum;
	info.m_HitboxSet = 0;

#ifdef HAMMER2013_PORT_PROXIES
	CRender::ProxyData proxyData{ pRender, pParent };
	info.m_pClientEntity = &proxyData;
#else
	info.m_pClientEntity = NULL;
#endif
	info.m_Lod = -1;
	info.m_pColorMeshes = NULL;
#ifdef SLE //// SLE CHANGE - apply lod depending on 2d view zoom
	if (pView->GetZoom() < 0.65)
		info.m_Lod = 1;

	if (pView->GetZoom() < 0.5)
		info.m_Lod = 2;

	if (pView->GetZoom() < 0.3)
		info.m_Lod = 3;

	if (pView->GetZoom() < 0.2)
		info.m_Lod = 4;

	if (pView->GetZoom() < 0.1)
		info.m_Lod = 5;
#endif
	bool bTransform = pRender->IsInLocalTransformMode();

	if ( bTransform )
	{
		// WHACKY HACKY
		VMatrix matrix; pRender->GetLocalTranform(matrix);

		// baseclass rotates the origin
		matrix.V3Mul( orgOrigin, m_origin );

		matrix3x4_t fCurrentMatrix,fMatrixNew;
		AngleMatrix(m_angles, fCurrentMatrix);
		ConcatTransforms(matrix.As3x4(), fCurrentMatrix, fMatrixNew);

		QAngle newAngles;
		MatrixAngles(fMatrixNew, m_angles);
	}

	if ( Options.general.bShowCollisionModels )
	{
		VMatrix mViewMatrix = SetupMatrixOrgAngles( orgOrigin, orgAngles );
		pRender->DrawCollisionModel( m_MDLHandle, mViewMatrix );
	}
	else
	{
		matrix3x4_t boneToWorld[MAXSTUDIOBONES];
		SetUpBones( false, boneToWorld );

#ifdef SLE //// SLE NEW: preview model scale
		// Scale the base transform if we don't have a bone hierarchy
		CStudioHdr *boneStudioHdr = GetStudioHdr();

		if (boneStudioHdr && boneToWorld && boneStudioHdr->numbones() == 1)
		{
			// Scale the bone to world at this point
			float flScale = m_modelScale;

			VectorScale((*boneToWorld)[0], flScale, (*boneToWorld)[0]);
			VectorScale((*boneToWorld)[1], flScale, (*boneToWorld)[1]);
			VectorScale((*boneToWorld)[2], flScale, (*boneToWorld)[2]);
		}
#endif
		pRender->DrawModel( &info, boneToWorld, m_origin, flAlpha, bWireFrame );
	}	
	
	if ( bTransform )	
	{
		// restore original position and angles
		m_origin = orgOrigin;
		m_angles = orgAngles;
    }
}

//-----------------------------------------------------------------------------
// It's translucent if all its materials are translucent
//-----------------------------------------------------------------------------
bool StudioModel::IsTranslucent()
{
	// garymcthack - shouldn't crack hardwaredata
	studiohwdata_t *pHardwareData = GetHardwareData();
	if ( pHardwareData == NULL )
		return false;

	int lodID;
	for( lodID = pHardwareData->m_RootLOD; lodID < pHardwareData->m_NumLODs; lodID++ )
	{
		for (int i = 0; i < pHardwareData->m_pLODs[lodID].numMaterials; ++i)
		{
			if (pHardwareData->m_pLODs[lodID].ppMaterials[i]->IsTranslucent())
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Frees the model data and releases textures from OpenGL.
//-----------------------------------------------------------------------------
void StudioModel::FreeModel(void)
{
	/*int nRef = */g_pMDLCache->Release( m_MDLHandle );
//	Assert( nRef == 0 );
	m_MDLHandle = MDLHANDLE_INVALID;
	m_pModel = NULL;
}

CStudioHdr *StudioModel::GetStudioHdr() const
{
	// return g_pMDLCache->GetStudioHdr( m_MDLHandle );

	if (m_pStudioHdr->IsValid())
		return m_pStudioHdr;

	studiohdr_t *hdr = g_pMDLCache->GetStudioHdr( m_MDLHandle );

	m_pStudioHdr->Init( hdr );

	Assert(m_pStudioHdr->IsValid());

	return m_pStudioHdr;
}

studiohdr_t *StudioModel::GetStudioRenderHdr() const
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	
	if (pStudioHdr)
	{
		return (studiohdr_t *)pStudioHdr->GetRenderHdr();
	}
	return NULL;
}

studiohwdata_t* StudioModel::GetHardwareData()
{
	return g_pMDLCache->GetHardwareData( m_MDLHandle );
}

bool StudioModel::LoadModel( const char *modelname )
{
	// Load the MDL file data
	Assert( m_MDLHandle == MDLHANDLE_INVALID );

	// for easier fall through cleanup
	m_MDLHandle = MDLHANDLE_INVALID;

	if ( !g_pStudioRender || !modelname )
		return false;

	// In the case of restore, m_pModelName == modelname
	if (m_pModelName != modelname)
	{
		// Copy over the model name; we'll need it later...
		if (m_pModelName)
		{
			delete[] m_pModelName;
		}

		m_pModelName = new char[strlen(modelname) + 1];
		strcpy( m_pModelName, modelname );
	}

	m_MDLHandle = g_pMDLCache->FindMDL( modelname );
	if (m_MDLHandle == MDLHANDLE_INVALID)
		return false;

	// Cache a bunch of stuff into memory
	g_pMDLCache->GetStudioHdr( m_MDLHandle );
	g_pMDLCache->GetHardwareData( m_MDLHandle );

	if (m_pStudioHdr)
	{
		delete m_pStudioHdr;
		m_pStudioHdr = NULL;
	}

	m_pStudioHdr = new CStudioHdr;

	return true;
}

bool StudioModel::PostLoadModel(const char *modelname)
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	if (pStudioHdr == NULL)
	{
		return(false);
	}

	SetSequence (0);

	for (int n = 0; n < pStudioHdr->numbodyparts(); n++)
	{
		if (SetBodygroup (n, 0) < 0)
		{
			return false;
		}
	}

	SetSkin (0);
/*
	Vector mins, maxs;
	ExtractBbox (mins, maxs);
	if (mins[2] < 5.0f)
		m_origin[2] = -mins[2];
*/
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int StudioModel::GetSequenceCount( void )
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	return pStudioHdr->GetNumSeq();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nIndex - 
//			szName - 
//-----------------------------------------------------------------------------
void StudioModel::GetSequenceName( int nIndex, char *szName )
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	if (nIndex < pStudioHdr->GetNumSeq())
	{
		strcpy(szName, pStudioHdr->pSeqdesc(nIndex).pszLabel());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns the index of the current sequence.
//-----------------------------------------------------------------------------
int StudioModel::GetSequence( )
{
	return m_sequence;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the current sequence by index.
//-----------------------------------------------------------------------------
int StudioModel::SetSequence( int iSequence )
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	if (iSequence > pStudioHdr->GetNumSeq())
		return m_sequence;

	m_sequence = iSequence;
	m_cycle = 0;

	return m_sequence;
}
#ifdef SLE //// SLE NEW - expose tris count on models
//-----------------------------------------------------------------------------
// Purpose: returns the triangle count at the current LOD
//-----------------------------------------------------------------------------
int StudioModel::GetTriangleCount(void)
{
	int nTris = 0;
	CStudioHdr *pStudioHdr = GetStudioHdr();
	if (pStudioHdr != NULL)
	{
		studiohwdata_t *hardwareData = GetHardwareData();
		if (hardwareData)
		{
			int chooseLod = Options.view3d.nLOD;
			//clamp(chooseLod, 0, hardwareData->m_NumLODs - 1);
			if (chooseLod > hardwareData->m_NumLODs - 1) chooseLod = hardwareData->m_NumLODs - 1;
			if (chooseLod < 0 ) chooseLod = 0;
			studioloddata_t *pLODData = &hardwareData->m_pLODs[chooseLod];
			for (int meshID = 0; meshID < hardwareData->m_NumStudioMeshes; meshID++)
			{
				studiomeshdata_t *pMesh = &pLODData->m_pMeshData[meshID];
				for (int groupID = 0; groupID < pMesh->m_NumGroup; groupID++)
				{
					studiomeshgroup_t	*pMeshGroup = &pMesh->m_pMeshGroup[groupID];

					for (int j = 0; j < pMeshGroup->m_NumStrips; j++)
					{
						nTris += (pMeshGroup->m_pStripData[j].numIndices / 3);
					}
				}
			}
		}
	}
	return nTris;
}

//-----------------------------------------------------------------------------
// Purpose: //// SLE NEW expose attachment positions
//-----------------------------------------------------------------------------
void StudioModel::GetAttachmentPosition(Vector &attachmentPos, int attachmentIndex)
{
	CStudioHdr *hdr = GetStudioHdr();
	if (!hdr) return;

	matrix3x4_t boneToWorld[MAXSTUDIOBONES];

	SetUpBones(false, boneToWorld);

	mstudioattachment_t &pattachments = (mstudioattachment_t)hdr->pAttachment(attachmentIndex);
	matrix3x4_t m;
	ConcatTransforms(boneToWorld[hdr->GetAttachmentBone(attachmentIndex)], pattachments.local, m);
	
	attachmentPos = Vector(m[0][3], m[1][3], m[2][3]);
}

void StudioModel::GetAttachmentPosition(Vector &attachmentPos, char *attachmentName)
{
	//// SLE TODO - realise
}
#endif //// SLE
//-----------------------------------------------------------------------------
// Purpose: Rotates the given bounding box by the given angles and computes the
//			bounds of the rotated box. This is used to take the rotation angles
//			into consideration when returning the bounding box. Note that this
//			can produce a larger than optimal bounding box.
// Input  : Mins - 
//			Maxs - 
//			Angles - 
//-----------------------------------------------------------------------------
void StudioModel::RotateBbox(Vector &Mins, Vector &Maxs, const QAngle &Angles)
{
	Vector Points[8];

	PointsFromBox( Mins, Maxs, Points );
	
	//
	// Rotate the corner points by the specified angles, in the same
	// order that our Render code uses.
	//
	VMatrix mMatrix;
	mMatrix.SetupMatrixOrgAngles( vec3_origin, Angles );
	matrix3x4_t fMatrix2 = mMatrix.As3x4();
	
	Vector RotatedPoints[8];
	for (int i = 0; i < 8; i++)
	{
		VectorRotate(Points[i], fMatrix2, RotatedPoints[i]);
	}

	//
	// Calculate the new mins and maxes.
	//
	for (int i = 0; i < 8; i++)
	{
		for (int nDim = 0; nDim < 3; nDim++)
		{
			if ((i == 0) || (RotatedPoints[i][nDim] < Mins[nDim]))
			{
				Mins[nDim] = RotatedPoints[i][nDim];
			}

			if ((i == 0) || (RotatedPoints[i][nDim] > Maxs[nDim]))
			{
				Maxs[nDim] = RotatedPoints[i][nDim];
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : mins - 
//			maxs - 
//-----------------------------------------------------------------------------
void StudioModel::ExtractBbox(Vector &mins, Vector &maxs)
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	mstudioseqdesc_t	&seqdesc = pStudioHdr->pSeqdesc( m_sequence );
	
	mins = seqdesc.bbmin;

	maxs = seqdesc.bbmax;

	RotateBbox(mins, maxs, m_angles);

#ifdef SLE //// SLE NEW - preview model scale
	VectorScale(mins, (vec_t)m_modelScale, mins);
	VectorScale(maxs, (vec_t)m_modelScale, maxs);
#endif
}

void StudioModel::ExtractClippingBbox( Vector& mins, Vector& maxs )
{
	studiohdr_t *pStudioHdr = GetStudioRenderHdr();
	mins[0] = pStudioHdr->view_bbmin[0];
	mins[1] = pStudioHdr->view_bbmin[1];
	mins[2] = pStudioHdr->view_bbmin[2];

	maxs[0] = pStudioHdr->view_bbmax[0];
	maxs[1] = pStudioHdr->view_bbmax[1];
	maxs[2] = pStudioHdr->view_bbmax[2];
	
#ifdef SLE //// SLE NEW - preview model scale
	VectorScale(mins, m_modelScale, mins);
	VectorScale(maxs, m_modelScale, maxs);
#endif
}

void StudioModel::ExtractMovementBbox( Vector& mins, Vector& maxs )
{
	studiohdr_t *pStudioHdr = GetStudioRenderHdr();
	mins[0] = pStudioHdr->hull_min[0];
	mins[1] = pStudioHdr->hull_min[1];
	mins[2] = pStudioHdr->hull_min[2];

	maxs[0] = pStudioHdr->hull_max[0];
	maxs[1] = pStudioHdr->hull_max[1];
	maxs[2] = pStudioHdr->hull_max[2];
	
#ifdef SLE //// SLE NEW - preview model scale
	VectorScale(mins, m_modelScale, mins);
	VectorScale(maxs, m_modelScale, maxs);
#endif
}

void StudioModel::GetSequenceInfo( float *pflFrameRate, float *pflGroundSpeed )
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	float t = Studio_Duration( pStudioHdr, m_sequence, m_poseParameter );

	if (t > 0)
	{
		*pflFrameRate = 1.0 / t;
		*pflGroundSpeed = 0; // sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
		// *pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 1.0;
		*pflGroundSpeed = 0.0;
	}
}

void StudioModel::SetOrigin( float x, float y, float z )
{
	m_origin[0] = x;
	m_origin[1] = y;
	m_origin[2] = z;
}

void StudioModel::SetOrigin( const Vector &v )
{
	m_origin = v;
}

void StudioModel::GetOrigin( float &x, float &y, float &z )
{
	x = m_origin[0];
	y = m_origin[1];
	z = m_origin[2];
}

void StudioModel::GetOrigin( Vector &v )
{
	v = m_origin;
}
#ifdef SLE
int StudioModel::SetBodygroups(int iValue)
{
	m_bodynum = iValue;
	return m_bodynum;
}
#endif
int StudioModel::SetBodygroup( int iGroup, int iValue )
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	if (!pStudioHdr)
		return 0;

	if (iGroup > pStudioHdr->numbodyparts())
		return -1;

	mstudiobodyparts_t *pbodypart = pStudioHdr->pBodypart( iGroup );

	if ((pbodypart->base == 0) || (pbodypart->nummodels == 0))
	{
		return -1;
	}

	int iCurrent = (m_bodynum / pbodypart->base) % pbodypart->nummodels;

	if (iValue >= pbodypart->nummodels)
		return iCurrent;

	m_bodynum = (m_bodynum - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));

	return iValue;
}

int StudioModel::SetSkin( int iValue )
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	if (!pStudioHdr)
		return 0;

	if (iValue >= pStudioHdr->numskinfamilies())
	{
		iValue = 0;
	}

	m_skinnum = iValue;

	return iValue;
}
#ifdef SLE //// SLE NEW - preview model scale
//-----------------------------------------------------------------------------
// Purpose:
// Input  : float - scale value
//-----------------------------------------------------------------------------
void StudioModel::SetModelScale(float fScale)
{
	CStudioHdr *pStudioHdr = GetStudioHdr();
	if (!pStudioHdr)
		return;

	if (fScale > 0)
	{
		m_modelScale = fScale;
	}
}

//// SLE NEW: 3d skybox preview
void StudioModel::Set3dSkybox(bool is3dskybox)
{
	m_is3dskybox = is3dskybox;
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pRender - 
//-----------------------------------------------------------------------------
/*void StudioModel::DrawModel2D(CRender2D *pRender)
{
	studiohdr_t *pStudioHdr = GetStudioRenderHdr();
	CMapView2D *pView = (CMapView2D*) pRender->GetView();

	DrawModelInfo_t info;
	ZeroMemory(&info, sizeof(info));

	info.m_pStudioHdr = pStudioHdr;
	info.m_pHardwareData = GetHardwareData();

	info.m_Decals = STUDIORENDER_DECAL_INVALID;
	info.m_Skin = m_skinnum;
	info.m_Body = m_bodynum;
	info.m_HitboxSet = 0;

	info.m_pClientEntity = NULL;
	info.m_Lod = -1;
	info.m_ppColorMeshes = NULL;

	if ( pView->m_fZoom < 3 )
		info.m_Lod = 3;

	matrix3x4_t *pBoneToWorld = SetUpBones( g_bUpdateBones2D );

	GetTriangles_Output_t tris;
	g_pStudioRender->GetTriangles( info, tris );

    for ( int batchID = 0; batchID < tris.m_MaterialBatches.Count(); batchID++ )
	{
		GetTriangles_MaterialBatch_t &materialBatch = tris.m_MaterialBatches[batchID];

		int numStrips = materialBatch.m_TriListIndices.Count() / 3;
		int numVertices = materialBatch.m_Verts.Count();
	
		POINT *points = (POINT*)_alloca( sizeof(POINT) * numVertices );
		
		//	translate all vertices
		for ( int vertID = 0; vertID < numVertices; vertID++)
		{
			GetTriangles_Vertex_t &vert = materialBatch.m_Verts[vertID];
			const Vector &pos = vert.m_Position;

			Vector newPos(0,0,0);
			
			for ( int k = 0; k < vert.m_NumBones; k++ )
			{
				const matrix3x4_t &poseToWorld = tris.m_PoseToWorld[ vert.m_BoneIndex[k] ];
				Vector tmp;
				VectorTransform( pos, poseToWorld, tmp );
				newPos += vert.m_BoneWeight[k] * tmp;
			}

			pView->WorldToClient( points[vertID], newPos );
//			pRender->TransformPoint3D( points[vertID], newPos );
		}

		// Send the vertices down to the hardware.

		int stripIndex = 0;

		for ( int strip = 0; strip < numStrips; strip++ )
		{
			int ptx[3];
			int pty[3];

			int numPoints = 0;
			POINT lastPt; lastPt.x = lastPt.y = -99999;
					
 			for ( int i = 0; i<3; i++ )
			{
				POINT pt = points[ materialBatch.m_TriListIndices[stripIndex++] ];

				if ( pt.x == lastPt.x && pt.y == lastPt.y )
					continue;

				ptx[numPoints] = pt.x;
				pty[numPoints] = pt.y;
				lastPt = pt;
				numPoints++;
			}

			// for performance sake bypass the renderer interface, buuuhhh

			if ( numPoints == 2 )
			{
				g_pMatSystemSurface->DrawLine( ptx[0], pty[0], ptx[1], pty[1] );
			}
			else if ( numPoints == 3 )
			{
				g_pMatSystemSurface->DrawPolyLine( ptx, pty, 3 );
			}
		}
	}
} */

void InitStudioFileChangeWatcher()
{
	g_StudioFileChangeWatcher.Init();
}

void UpdateStudioFileChangeWatcher()
{
	g_StudioFileChangeWatcher.Update();
}
