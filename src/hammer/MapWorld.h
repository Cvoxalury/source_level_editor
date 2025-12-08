//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef MAPWORLD_H
#define MAPWORLD_H
#ifdef _WIN32
#pragma once
#endif

#include "MapEntity.h"
#include "EditGameClass.h"
#include "MapClass.h"
#include "MapPath.h"

// Flags for SaveVMF.
#define SAVEFLAGS_LIGHTSONLY	(1<<0)
#define SAVEFLAGS_AUTOSAVE		(1<<1)
#ifdef HAMMER2013_PORT_SAVE_ON_CRASH
#define SAVEFLAGS_NO_UI_UPDATE	(1<<2) //// SLE NEW - ported from sdk-2013-hammer - autosave on crash
#endif

#define MAX_VISIBLE_OBJECTS		10000

#define NUM_HASHED_ENTITY_BUCKETS	200

#define HAMMER2013_MAPWORLD_FIXES

class BoundBox;
class CChunkFile;
class CVisGroup;
class CCullTreeNode;
class IEditorTexture;
class CMapGroup;
class CMapDoc;
class CMapInstance;

struct SaveLists_t;


struct UsedTexture_t
{
	IEditorTexture *pTex;
	int nUsageCount;
};


class CUsedTextureList : public CUtlVector<UsedTexture_t>
{
public:

	inline int Find(IEditorTexture *pTex)
	{
		for (int i = 0; i < Count(); i++)
		{
			if (Element(i).pTex == pTex)
			{
				return i;
			}
		}
		
		return -1;	
	}
};


class CMapWorld : public CMapClass, public CEditGameClass
{
#ifdef HAMMER2013_MAPWORLD_FIXES
	void Init();
#endif
	public:

		DECLARE_MAPCLASS(CMapWorld,CMapClass)

		CMapWorld( void );
		CMapWorld( CMapDoc *pOwningDocument );
		~CMapWorld(void);

		CMapDoc *GetOwningDocument( void ) { return m_pOwningDocument; }

		//
		// Public interface to the culling tree.
		//
		void CullTree_Build(void);
		inline CCullTreeNode *CullTree_GetCullTree(void) { return(m_pCullTree); }

		//
		// CMapClass virtual overrides.
		//
		virtual void AddChild(CMapClass *pChild);
		virtual CMapClass *Copy(bool bUpdateDependencies);
		virtual CMapClass *CopyFrom(CMapClass *pFrom, bool bUpdateDependencies);
		virtual void PresaveWorld(void);
		virtual void RemoveChild(CMapClass *pChild, bool bUpdateBounds = true);
		virtual bool IsWorld() { return true; }

		virtual CMapEntity *FindChildByKeyValue( const char* key, const char* value, bool *bIsInInstance = NULL, VMatrix *InstanceMatrix = NULL );

		void AddObjectToWorld(CMapClass *pObject, CMapClass *pParent = NULL);
		void RemoveObjectFromWorld(CMapClass *pObject, bool bRemoveChildren);

		// Groups have to be treated as logical because they potentially have logical children
		virtual bool IsLogical(void) { return true; }
		virtual bool IsVisibleLogical(void) { return IsVisible(); }
		virtual bool IsEditable( void );

		//
		// Serialization.
		//
		ChunkFileResult_t LoadVMF(CChunkFile *pFile);
		void PostloadWorld(void);
		void PostloadVisGroups(void);
		
		// saveFlags is a combination of SAVEFLAGS_ defines.
		ChunkFileResult_t SaveSolids(CChunkFile *pFile, CSaveInfo *pSaveInfo, int saveFlags);
#ifdef HAMMER2013_PORT_CORDONS
		ChunkFileResult_t SaveVMF(CChunkFile *pFile, CSaveInfo *pSaveInfo, int saveFlags, const CMapObjectList* extraSolids = nullptr);
#else
		ChunkFileResult_t SaveVMF(CChunkFile *pFile, CSaveInfo *pSaveInfo, int saveFlags);
#endif
		virtual int SerializeRMF(std::fstream &file, BOOL fIsStoring);
		virtual int SerializeMAP(std::fstream &file, BOOL fIsStoring, BoundBox *pIntersecting = NULL);
#ifdef SLE //// SLE CHANGE - allow saving the map as a commentary txt file (or potentially in the future, mapadd)
		virtual int SerializeCommentaryTXT(std::fstream &file, BOOL fIsStoring);
#endif
		virtual void UpdateChild(CMapClass *pChild);
#ifdef HAMMER2013_MAPWORLD_FIXES
		virtual void OnUndoRedo();
#endif
		void UpdateAllDependencies( CMapClass *pObject );

		//
		// Face ID management.
		//
		inline int FaceID_GetNext(void);
		inline void FaceID_SetNext(int nNextFaceID);
		CMapFace *FaceID_FaceForID(int nFaceID);
		void FaceID_StringToFaceIDLists(CMapFaceIDList *pFullFaceList, CMapFaceIDList *pPartialFaceList, const char *pszValue);
		void FaceID_StringToFaceLists(CMapFaceList *pFullFaceList, CMapFaceList *pPartialFaceList, const char *pszValue);
		static bool FaceID_FaceIDListsToString(char *pszList, int nSize, CMapFaceIDList *pFullFaceIDList, CMapFaceIDList *pPartialFaceIDList);
		static bool FaceID_FaceListsToString(char *pszValue, int nSize, CMapFaceList *pFullFaceList, CMapFaceList *pPartialFaceList);

		void GetUsedTextures(CUsedTextureList &List);
		void Subtract(CMapObjectList &Results, CMapClass *pSubtractWith, CMapClass *pSubtractFrom);

		CUtlVector<CMapPath*> m_Paths;

		// Interface to list of all the entities in the world:
		const CMapEntityList *EntityList_GetList(void) { return(&m_EntityList); }
		inline int EntityList_GetCount();
		inline CMapEntity *EntityList_GetEntity( int nIndex );

		CMapEntity *FindEntityByName( const char *pszName, bool bVisiblesOnly = false, bool bSearchInstanceParms = false );
		bool FindEntitiesByKeyValue(CMapEntityList &Found, const char *szKey, const char *szValue, bool bVisiblesOnly);
		bool FindEntitiesByName(CMapEntityList &Found, const char *szName, bool bVisiblesOnly);
		bool FindEntitiesByClassName(CMapEntityList &Found, const char *szClassName, bool bVisiblesOnly);
		bool FindEntitiesByNameOrClassName(CMapEntityList &Found, const char *pszName, bool bVisiblesOnly);
#ifdef SLE  //// SLE NEW - 3d skybox preview
		Vector m_vecSkyCameraDelta;
		Vector GetSkyCameraDelta(void)
		{
			return m_vecSkyCameraDelta;
		}
		void SetSkyCameraDelta(Vector newDelta)
		{
			m_vecSkyCameraDelta = newDelta;
		}
		float m_flSkyCameraScale = 16.0f;
		float GetSkyCameraScale(void)
		{
			return m_flSkyCameraScale;
		}
		void SetSkyCameraScale(float newScale)
		{
			m_flSkyCameraScale = newScale;
		}
#ifdef SLE_CACHE_FOG_PARAMS
		//// SLE NEW: cache fog parameters in the world data
		Vector m_fog_color1;
		Vector m_fog_color2;
		float m_fog_start;
		float m_fog_end;
		float m_fog_density;
		void GetFogParams(Vector &fog_color1, Vector &fog_color2, float &fog_start, float &fog_end, float &fog_density) const
		{
			fog_color1 = m_fog_color1;
			fog_color2 = m_fog_color2;
			fog_start = m_fog_start;
			fog_end = m_fog_end;
			fog_density = m_fog_density;
			return;
		}
		void SetFogParams(Vector fog_color1, Vector fog_color2, float fog_start, float fog_end, float fog_density)
		{
			m_fog_color1 = fog_color1;
			m_fog_color2 = fog_color2;
			m_fog_density = fog_density;
			m_fog_start = fog_start;
			m_fog_end = fog_end;
		}
#endif
#endif //// SLE
		bool GenerateNewTargetname( const char *startName, char *newName, int newNameBufferSize, bool bMakeUnique, const char *szPrefix, CMapClass *pRoot = NULL );

		// displacement management
		inline IWorldEditDispMgr *GetWorldEditDispManager( void ) { return m_pWorldDispMgr; }

		int GetGroupList(CUtlVector<CMapGroup *> &GroupList);

	protected:

		//
		// Protected entity list functions.
		//
		void AddEntity( CMapEntity *pEntity );
		void EntityList_Add(CMapClass *pObject);
		void EntityList_Remove(CMapClass *pObject, bool bRemoveChildren);

		int FindEntityBucket( CMapEntity *pEntity, int *pnIndex );

		//
		// Serialization.
		//
		static ChunkFileResult_t LoadKeyCallback(const char *szKey, const char *szValue, CMapWorld *pWorld);
		static ChunkFileResult_t LoadGroupCallback(CChunkFile *pFile, CMapWorld *pWorld);
		static ChunkFileResult_t LoadHiddenCallback(CChunkFile *pFile, CMapWorld *pWorld);
		static ChunkFileResult_t LoadHiddenSolidCallback(CChunkFile *pFile, CMapWorld *pWorld);
		static ChunkFileResult_t LoadSolidCallback(CChunkFile *pFile, CMapWorld *pWorld);

		ChunkFileResult_t LoadSolid(CChunkFile *pFile, bool bVisible);

		static BOOL BuildSaveListsCallback(CMapClass *pObject, SaveLists_t *pSaveLists);
		ChunkFileResult_t SaveObjectListVMF(CChunkFile *pFile, CSaveInfo *pSaveInfo, const CMapObjectList *pList, int saveFlags);

		//
		// Culling tree operations.
		//
		void CullTree_SplitNode(CCullTreeNode *pNode);
		void CullTree_DumpNode(CCullTreeNode *pNode, int nDepth);
		void CullTree_FreeNode(CCullTreeNode *pNode);
		void CullTree_Free(void);

		CCullTreeNode *m_pCullTree;		// This world's objects stored in a spatial hierarchy for culling.
		
		CMapEntityList m_EntityList;									// A flat list of all the entities in this world.
		CMapEntityList m_EntityListByName[NUM_HASHED_ENTITY_BUCKETS];	// A list of all the entities in the world, hashed by name checksum.

		int m_nNextFaceID;						// Used for assigning unique IDs to every solid face in this world.

		IWorldEditDispMgr	*m_pWorldDispMgr;	// world editable displacement manager

		CMapDoc				*m_pOwningDocument;
};


//-----------------------------------------------------------------------------
// Purpose: Returns the next unique face ID for this world.
//-----------------------------------------------------------------------------
inline int CMapWorld::FaceID_GetNext(void)
{
	return(m_nNextFaceID++);
}


//-----------------------------------------------------------------------------
// Purpose: Sets the unique face ID to assign to the next face that is added
//			to this world.
//-----------------------------------------------------------------------------
inline void CMapWorld::FaceID_SetNext(int nNextFaceID)
{
	m_nNextFaceID = nNextFaceID;
}


inline int CMapWorld::EntityList_GetCount()
{
	return m_EntityList.Count();
}


inline CMapEntity *CMapWorld::EntityList_GetEntity( int nIndex )
{
	return m_EntityList.Element( nIndex );
}


#endif // MAPWORLD_H
