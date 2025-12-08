//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef STUDIOMODEL_H
#define STUDIOMODEL_H

#ifdef _WIN32
#pragma once
#endif

#ifndef byte
typedef unsigned char byte;
#endif // byte

#include "hammer_mathlib.h"
#include "studio.h"
#include "utlvector.h"
#include "datacache/imdlcache.h"
#include "FileChangeWatcher.h"

class StudioModel;
class CMaterial;
class CRender3D;
class CRender2D;

struct ModelCache_t
{
	StudioModel *pModel;
	char *pszPath;
	int nRefCount;
};

//-----------------------------------------------------------------------------
// Purpose: Defines an interface to a cache of studio models.
//-----------------------------------------------------------------------------
class CStudioModelCache
{
	public:
		static StudioModel *FindModel(const char *pszModelPath);
		static StudioModel *CreateModel(const char *pszModelPath);
#ifdef SLE  //// SLE NEW - ported from sdk-2013-hammer
		static void ReloadModel(const char *pszModelPath);
#endif
		static void AddRef(StudioModel *pModel);
		static void Release(StudioModel *pModel);
		static void AdvanceAnimation(float flInterval);

	protected:
		static BOOL AddModel(StudioModel *pModel, const char *pszModelPath);
		static void RemoveModel(StudioModel *pModel);
#ifdef SLE
		static ModelCache_t m_Cache[65536];
#else
		static ModelCache_t m_Cache[1024];
#endif
		static int m_nItems;
};

// Calling these will monitor the filesystem for changes to model files and automatically 
// incorporate changes to the models.
void InitStudioFileChangeWatcher();
void UpdateStudioFileChangeWatcher();

class StudioModel
{
public:
	static bool				Initialize(void);
	static void				Shutdown(void); // garymcthack - need to call this.

	StudioModel(void);
	~StudioModel(void);

	void FreeModel ();
	bool LoadModel(const char *modelname);
	bool PostLoadModel (const char *modelname);
#ifdef SLE
#ifdef HAMMER2013_PORT_PROXIES
	void DrawModel3D(CRender3D *pRender, const Color &color, float flAlpha, bool bWireframe, bool bSelectionOverlay = false, CMapClass* pParent = nullptr);
#else
	void DrawModel3D(CRender3D *pRender, const Color &color, float flAlpha, bool bWireframe, bool bSelectionOverlay = false);
#endif
#else
	void DrawModel3D(CRender3D *pRender, float flAlpha, bool bWireframe);
#endif
#ifdef HAMMER2013_PORT_PROXIES
	void DrawModel2D(CRender2D *pRender, float flAlpha, bool bWireFrame, CMapClass* pParent = nullptr);
#else
	void DrawModel2D(CRender2D *pRender, float flAlpha, bool bWireFrame);
#endif
	void AdvanceFrame(float dt);

	void ExtractBbox(Vector &mins, Vector &maxs);
	void ExtractClippingBbox(Vector &mins, Vector &maxs);
	void ExtractMovementBbox(Vector &mins, Vector &maxs);
	void RotateBbox(Vector &Mins, Vector &Maxs, const QAngle &Angles);

	int SetSequence(int iSequence);
	int GetSequence(void);
	int GetSequenceCount(void);
	void GetSequenceName(int nIndex, char *szName);
	void GetSequenceInfo(float *pflFrameRate, float *pflGroundSpeed);

#ifdef SLE	//// SLE NEW: print out sequence activity and some other useful info...
	void GetActivityNameForSequence(int nSequence, char *szActivityName)
	{
		CStudioHdr *pStudioHdr = GetStudioHdr();
		if ( pStudioHdr )
		{
			if ( nSequence < pStudioHdr->GetNumSeq() )
			{
				strcpy(szActivityName, pStudioHdr->pSeqdesc(nSequence).pszActivityName());
			}
		}
	}

	void GetModelMass(float *fMass)
	{
		CStudioHdr *pStudioHdr = GetStudioHdr();
		if ( pStudioHdr && pStudioHdr->GetRenderHdr() )
		{
			*fMass = pStudioHdr->GetRenderHdr()->mass;
		}
	}

	void GetModelSurfaceProp(char *surfaceprop)
	{
		CStudioHdr *pStudioHdr = GetStudioHdr();
		if ( pStudioHdr )
		{
			strcpy(surfaceprop, pStudioHdr->pszSurfaceProp());
		}
	}

	int GetModelContents(void)
	{
		CStudioHdr *pStudioHdr = GetStudioHdr();
		if ( pStudioHdr )
		{
			return pStudioHdr->contents();
		}
		return 0;
	}

	const char *GetModelName(void)
	{
		CStudioHdr *pStudioHdr = GetStudioHdr();
		if ( pStudioHdr && pStudioHdr->GetRenderHdr() )
		{
			return pStudioHdr->GetRenderHdr()->pszName();
		}

		return NULL;
	}

	int GetModelVersion(void)
	{
		CStudioHdr *pStudioHdr = GetStudioHdr();
		if ( pStudioHdr && pStudioHdr->GetRenderHdr() )
		{
			return pStudioHdr->GetRenderHdr()->version;
		}
		return 0;
	}

	int GetModelAnimLength(int sequence)
	{
		CStudioHdr *pStudioHdr = GetStudioHdr();
		if ( pStudioHdr && pStudioHdr->pAnimStudioHdr(sequence) )
		{
			return pStudioHdr->pAnimStudioHdr(sequence)->length;
		}
		return 0;
	}
#endif
#ifdef SLE	//// SLE NEW - add SetCycle, make frame slider in Object Properties work
	void SetCycle(float fValue) { m_cycle = fValue; }
	float GetCycle(void) { return m_cycle; }

	//// SLE NEW - expose tris count on models
	int	 GetTriangleCount(void); 

	//// SLE NEW - expose attachment positions
	void GetAttachmentPosition(Vector &attachmentPos, int attachmentIndex);
	void GetAttachmentPosition(Vector &attachmentPos, char *attachmentName);

	int SetBodygroups(int iValue);
#endif
	int SetBodygroup( int iGroup, int iValue );
	int SetSkin( int iValue );
	void SetOrigin( float x, float y, float z );
	void GetOrigin( float &x, float &y, float &z );
	void SetOrigin( const Vector &v );
	void GetOrigin( Vector &v );
	void SetAngles( QAngle& pfAngles );
#ifdef SLE
	void SetModelScale(float fScale); //// SLE NEW - model scale preview
	void Set3dSkybox(bool is3dskybox); //// SLE NEW: 3d skybox preview
#endif
	bool IsTranslucent();
	
private:
	CStudioHdr				*m_pStudioHdr;
	CStudioHdr				*GetStudioHdr() const;
	studiohdr_t*			GetStudioRenderHdr() const;
	studiohwdata_t*			GetHardwareData();

	// entity settings
	Vector					m_origin;
	QAngle					m_angles;	
	int						m_sequence;			// sequence index
	float					m_cycle;			// pos in animation cycle
	int						m_bodynum;			// bodypart selection	
	int						m_skinnum;			// skin group selection
	byte					m_controller[MAXSTUDIOBONECTRLS];	// bone controllers
	float					m_poseParameter[MAXSTUDIOPOSEPARAM];		// animation blending
	byte					m_mouth;			// mouth position
	char*					m_pModelName;		// model file name
#ifdef SLE
	float					m_modelScale;		//// SLE NEW - preview model scale
	bool					m_is3dskybox;		//// SLE NEW: 3d skybox preview
#endif
	Vector					*m_pPosePos;
	Quaternion				*m_pPoseAng;

	// internal data
	MDLHandle_t				m_MDLHandle;
	mstudiomodel_t			*m_pModel;	
	
#ifdef SLE
public:
#endif
	void					SetUpBones ( bool bUpdatePose, matrix3x4_t*	pBoneToWorld );
#ifdef SLE
private:
#endif
	void					SetupModel ( int bodypart );

	void					LoadStudioRender( void );

	bool					LoadVVDFile( const char *modelname );
	bool					LoadVTXFile( const char *modelname );
};

extern Vector g_vright;		// needs to be set to viewer's right in order for chrome to work
extern float g_lambert;		// modifier for pseudo-hemispherical lighting

#endif // STUDIOMODEL_H