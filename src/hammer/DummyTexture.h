//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of IEditorTexture interface for placeholder textures.
//			Placeholder textures are used for textures that are referenced in
//			the map file but not found in storage.
//
//=============================================================================//

#ifndef DUMMYTEXTURE_H
#define DUMMYTEXTURE_H
#ifdef _WIN32
#pragma once
#endif

#include <afxtempl.h>
#include "IEditorTexture.h"
#ifdef SLE
#include "Options.h"
#endif
enum TEXTUREFORMAT;

class CDummyTexture : public IEditorTexture
{
	public:
		CDummyTexture(const char *pszName, TEXTUREFORMAT eFormat);
		virtual ~CDummyTexture();

		inline const char *GetName() const
		{
			return(m_szName);
		}
		int GetShortName(char *pszName) const;

		int GetKeywords(char *pszKeywords) const;

		void Draw(CDC *pDC, RECT &rect, int iFontHeight, int iIconHeight, DrawTexData_t &DrawTexData);

		const char *GetFileName(void) const;

		void GetSize(SIZE &size) const;

		inline bool IsDummy() const
		{
			return(true);
		}

		int GetImageDataRGB( void *pImageRGB );
		int GetImageDataRGBA( void *pImageRGBA );

		inline int GetImageWidth() const
		{
#ifdef HAMMER2013_PORT_MISSING_CHECKER
			return 256;
#else
			return(0);
#endif
		}

		inline int GetImageHeight() const
		{
#ifdef HAMMER2013_PORT_MISSING_CHECKER
			return 256;
#else
			return(0);
#endif
		}

		inline float GetDecalScale() const
		{
			return(1.0f);
		}

		CPalette *GetPalette() const
		{
			return(NULL);
		}

		inline int GetWidth() const
		{
#ifdef HAMMER2013_PORT_MISSING_CHECKER
			return 256;
#else
			return(0);
#endif
		}

		inline int GetHeight() const
		{
#ifdef HAMMER2013_PORT_MISSING_CHECKER
			return 256;
#else
			return(0);
#endif
		}

		inline int GetTextureID() const
		{
			return(0);
		}

		inline TEXTUREFORMAT GetTextureFormat() const
		{
			return(m_eTextureFormat);
		}

		inline int GetSurfaceAttributes() const
		{
			return(0);
		}

		inline int GetSurfaceContents() const
		{
			return(0);
		}

		inline int GetSurfaceValue() const
		{
			return(0);
		}

		inline bool HasAlpha() const
		{
			return(false);
		}

		inline bool HasData() const
		{
#ifdef HAMMER2013_PORT_MISSING_CHECKER
			return true;
#else
			return(false);
#endif
		}

		inline bool HasPalette() const
		{
			return(false);
		}

		bool Load( void );
		void Reload( bool bFullReload ) {}

		inline bool IsLoaded() const
		{
			return true;
		}

		inline void SetTextureFormat(TEXTUREFORMAT eFormat)
		{
			m_eTextureFormat = eFormat;
		}

		inline void SetTextureID( int nTextureID )
		{
		}

		bool IsWater( void ) const
		{
			return false;
		}

#ifdef SLE	//// SLE NEW - lets us filter face textures by their vmt variable
		bool HasVariable(const char* var) const
		{
			return false;
		}
		bool HasVariableWithValue(const char* var, const char* val) const
		{
			return false;
		}

#ifdef HAMMER2013_PORT_MISSING_CHECKER
		IMaterial* GetMaterial(bool bForceLoad = true)
		{
			return errorMaterial;
		}
#endif
#endif
	protected:

		char m_szName[MAX_PATH];
#ifndef HAMMER2013_PORT_MISSING_CHECKER
		char m_szFileName[MAX_PATH];
#else
		static IMaterial* errorMaterial;
#endif

		TEXTUREFORMAT m_eTextureFormat;
};

#endif // DUMMYTEXTURE_H
