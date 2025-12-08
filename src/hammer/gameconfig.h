//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef GAMECONFIG_H
#define GAMECONFIG_H
#ifdef _WIN32
#pragma once
#endif

#pragma warning(push, 1)
#pragma warning(disable:4701 4702 4530)
#include <fstream>
#pragma warning(pop)
#include "fgdlib/HelperInfo.h"
#include "TokenReader.h"
#include "fgdlib/gamedata.h"
#include "IEditorTexture.h"
#include "GamePalette.h"
#include "utlvector.h"

class MDkeyvalue;
class KeyValues;

#define MAX_DIRECTORY_SIZE	32

enum MAPFORMAT
{
	mfQuake = 0,
	mfHexen2,
	mfQuake2,
	mfHalfLife,
	mfHalfLife2,
#ifdef SLE //// SLE NEW - support for modern delimeters (TF2)
	mfTF2,
#endif
};

struct MatExlcusions_s
{
	char szDirectory[MAX_PATH];		// Where we store the material exclusion directories
	bool bUserGenerated;			// If the user specified this ( default:  false -- FGD defined )
};
#ifdef SLE //// SLE NEW - Favourites for material browser
struct MatFavourites_s
{
	char szName[ MAX_PATH ];		// Where we store the material favourites names
	bool bUserGenerated;			// If the user specified this ( default:  false -- FGD defined )
};
#endif

class CGameConfig
{
public:

	CGameConfig();

		static CGameConfig *GetActiveGame(void);
		static void SetActiveGame(CGameConfig *pGame);

	inline TEXTUREFORMAT GetTextureFormat(void);
	inline void SetTextureFormat(TEXTUREFORMAT eFormat);

	inline float GetDefaultTextureScale(void);
	inline void SetDefaultTextureScale(float fScale);

	inline int GetDefaultLightmapScale(void);
	inline void SetDefaultLightmapScale(int nScale);

	inline const char *GetCordonTexture(void);
	inline void SetCordonTexture(const char *szCordonTexture);

	inline void GetSteamExe(CString &str);
	inline void GetSteamDir(CString &str);
	inline void GetSteamUserDir(CString &str);
	inline void GetSteamAppID(CString &str);

	inline MAPFORMAT GetMapFormat();

	CUtlVector< MatExlcusions_s > m_MaterialExclusions;
#ifdef SLE //// SLE NEW - Favourites for material browser
	CUtlVector< MatFavourites_s> m_MaterialFavourites;

	//// SLE CHANGE - since now default config is composed, nConfigs is never 0; detect instead if it was created from default
	inline int IsDefault(void);
#endif
	DWORD dwID;	// assigned on load

	char szName[128];
	int nGDFiles;
	MAPFORMAT mapformat;

	char szExecutable[128];
	char szDefaultPoint[128];
	char szDefaultSolid[128];
#ifdef SLE	//// SLE NEW - quick access class buttons for setting current entity's class
	char szQuickAccessClass1[128];
	char szQuickAccessClass2[128];
	char szQuickAccessClass3[128];
	char szQuickAccessClass4[128];
#endif
	char szBSP[128];
	char szLIGHT[128];
	char szVIS[128];
	char m_szGameExeDir[128];
	char szMapDir[128];
	char szBSPDir[128];
	char m_szModDir[128];
	int	 m_MaterialExcludeCount;
#ifdef SLE //// SLE NEW - Favourites for material browser
	int m_MaterialFavouritesCount;
#endif
	CStringArray GDFiles;
	GameData GD;	// gamedata files loaded

	CGamePalette Palette; // used for tfWAD3

	BOOL Import(std::fstream &, float fVersion);

	bool Load(KeyValues *pkv);
	bool Save(KeyValues *pkv);

	void Save(std::fstream &);
	bool Save(const char *pszFileName, const char *pszSection);
	void CopyFrom(CGameConfig *pConfig);
	void LoadGDFiles(void);

	void ParseGameInfo();

	// Accessor methods to get at the mod + the game (*not* full paths)
	const char *GetMod();
	const char *GetGame();

protected:
	TEXTUREFORMAT textureformat;
	float m_fDefaultTextureScale;
	int m_nDefaultLightmapScale;
	char m_szCordonTexture[MAX_PATH];

	// These settings are loaded from GameInfo.txt:
	char m_szSteamDir[MAX_PATH];			// The full path to steam.exe
	char m_szSteamUserDir[MAX_PATH];		// The full path to the users's directory under SteamApps
	char m_szSteamAppID[32];				// The app id to add to the command line when launching the game via Steam.
#ifdef SLE //// SLE CHANGE - since now default config is composed, nConfigs is never 0; detect instead if it was created from default
	int m_bIsDefault;
#endif
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
MAPFORMAT CGameConfig::GetMapFormat()
{
	return mapformat;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
TEXTUREFORMAT CGameConfig::GetTextureFormat(void)
{
	return(textureformat);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameConfig::SetTextureFormat(TEXTUREFORMAT eFormat)
{
	textureformat = eFormat;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CGameConfig::GetCordonTexture(void)
{
	return(m_szCordonTexture);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameConfig::SetCordonTexture(const char *szCordonTexture)
{
	Q_strncpy( m_szCordonTexture, szCordonTexture, sizeof( m_szCordonTexture ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CGameConfig::GetDefaultLightmapScale(void)
{
	return(m_nDefaultLightmapScale);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameConfig::SetDefaultLightmapScale(int nScale)
{
	m_nDefaultLightmapScale = nScale;
}
#ifdef SLE //// SLE CHANGE - since now default config is composed, nConfigs is never 0; detect instead if it was created from default
int CGameConfig::IsDefault(void)
{
	return m_bIsDefault;
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CGameConfig::GetDefaultTextureScale(void)
{
	return(m_fDefaultTextureScale);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameConfig::SetDefaultTextureScale(float fScale)
{
	m_fDefaultTextureScale = fScale;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the full path to steam.exe, found by searching up from
//			whatever directory hammer is in.
//-----------------------------------------------------------------------------
void CGameConfig::GetSteamDir(CString &str)
{
	str = m_szSteamDir;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the full path to steam.exe, found by searching up from
//			whatever directory hammer is in.
//-----------------------------------------------------------------------------
void CGameConfig::GetSteamExe(CString &str)
{
	GetSteamDir(str);
	str += "\\steam.exe";
}

//-----------------------------------------------------------------------------
// Purpose: Returns the full path to steam.exe, found by searching up from
//			whatever directory hammer is in.
//-----------------------------------------------------------------------------
void CGameConfig::GetSteamUserDir(CString &str)
{
	str = m_szSteamUserDir;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameConfig::GetSteamAppID(CString &str)
{
	str = m_szSteamAppID;
} 

extern GameData *pGD;
extern CGameConfig *g_pGameConfig;
extern float g_MAX_MAP_COORD;
extern float g_MIN_MAP_COORD;

#endif // GAMECONFIG_H