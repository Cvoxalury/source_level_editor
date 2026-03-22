//===== Copyright Valve Corporation, All rights reserved. ======//

#include "stdafx.h"
#include "mapimagepanel.h"
#include "hammer_mathlib.h"
#include "bitmap/psd.h"
#include "bitmap/tgaloader.h"
#include "box3d.h"
#include "bspfile.h"
#include "camera.h"
#include "const.h"
#include "filesystem.h"
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include "KeyValues.h"
#include "mapdefs.h"		// dvs: For COORD_NOTINIT
#include "MapEntity.h"
#include "render2d.h"
#include "render3d.h"
#include "hammer.h"
/*
#include "texturesystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imesh.h"
#include "../materialsystem/itextureinternal.h"
#include "options.h"
*/
#include "material.h"
#include "pixelwriter.h"

#ifdef SLE_IMAGEPANEL_USE_STBI
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#define STB_IMAGE_STATIC
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_MAX_DIMENSIONS 4096
#define STBI_ASSERT(x) Assert(x)
#include "stb_image.h"

#define STBIR_ASSERT(x) Assert(x)
#include "stb_image_resize.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_MAPCLASS( CImagePanelHelper )

class CBackgroundRegenerator : public ITextureRegenerator
{
public:
	CBackgroundRegenerator( unsigned char *ImageData, int Width, int Height, enum ImageFormat Format ) :
	  m_ImageData( ImageData ), 
		  m_Width( Width ), 
		  m_Height( Height ),
		  m_Format( Format )
	  {
	  }

	  virtual void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect )
	  {
		  for (int iFrame = 0; iFrame < pVTFTexture->FrameCount(); ++iFrame )
		  {
			  for (int iFace = 0; iFace < pVTFTexture->FaceCount(); ++iFace )
			  {
				  int nWidth = pVTFTexture->Width();
				  int nHeight = pVTFTexture->Height();
				  int nDepth = pVTFTexture->Depth();
				  for (int z = 0; z < nDepth; ++z)
				  {
					  // Fill mip 0 with a checkerboard
					  CPixelWriter pixelWriter;
					  pixelWriter.SetPixelMemory( pVTFTexture->Format(), pVTFTexture->ImageData( iFrame, iFace, 0, 0, 0, z ), pVTFTexture->RowSizeInBytes( 0 ) );

					  switch( m_Format )
					  {
					  case IMAGE_FORMAT_BGR888:
						  {
							  unsigned char *data = m_ImageData;

							  for (int y = 0; y < nHeight; ++y)
							  {
								  pixelWriter.Seek( 0, y );
								  for (int x = 0; x < nWidth; ++x)
								  {
									  pixelWriter.WritePixel( *( data + 2 ), *( data + 1 ), *( data ), 255 );
									  data += 3;
								  }
							  }
						  }
						  break;
					  }
				  }
			  }
		  }
	  }

	  virtual void Release()
	  {
		  delete this;
	  }

private:
	unsigned char		*m_ImageData;
	int					m_Width;
	int					m_Height;
	enum ImageFormat	m_Format;
};

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a CImagePanelHelper from a set
//			of string parameters from the FGD file.
// Input  : *pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, NULL if an error occurs.
//-----------------------------------------------------------------------------
CMapClass* CImagePanelHelper::CreateImagePanel( CHelperInfo* pHelperInfo, CMapEntity* pParent )
{
	const char* pMsg = pParent->GetKeyValue( "image_filename" );

	CImagePanelHelper* pImage = new CImagePanelHelper;
	pImage->SetImageFilename( pMsg );
	pImage->SetRenderMode( kRenderTransAlpha );

	return pImage;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CImagePanelHelper::CImagePanelHelper() : m_eRenderMode( kRenderTransAlpha )
{
	m_RenderColor.r = 255;
	m_RenderColor.g = 255;
	m_RenderColor.b = 255;
	m_RenderColor.a = 255;

	m_show2D_bool = false; 
	m_show3D_bool = true; 
	
	m_bg.width_int = 400.0f;
	m_bg.height_int = 300.0f;
	m_bg.alpha_float = 1.0f;
	m_bg.center_x_int = 0;
	m_bg.center_y_int = 0;
	m_bg.is_enabled_bool = true;
	m_bg.is_nocull_bool = false;
	m_bg.is_flipped_x = false;
	m_bg.is_flipped_y = false;

	m_bg_mat = NULL;
	m_bg_tex = NULL;
	m_bg_pixels = NULL;

	m_bg_width_int = 0;
	m_bg_height_int = 0;
	
	SetImageFilename( "" );
	m_Angles.Init();
}

// Purpose: Destructor.
CImagePanelHelper::~CImagePanelHelper()
{
	m_bg_pixels = NULL;
	if ( m_bg_tex )
	{
		m_bg_tex->SetTextureRegenerator(NULL);
		m_bg_tex->DecrementReferenceCount();
		m_bg_tex = NULL;
	}
	if ( m_bg_mat )
	{
		m_bg_mat->DecrementReferenceCount();
		m_bg_mat = NULL;
	}
	if ( m_bg_pixels )
	{
		delete [] m_bg_pixels;
	}
}
//-----------------------------------------------------------------------------
// Sets the render mode
//-----------------------------------------------------------------------------

void CImagePanelHelper::SetRenderMode( int eRenderMode )
{
	m_eRenderMode = eRenderMode;
}

void CImagePanelHelper::ComputeCornerVertices( Vector* pVerts, float flBloat ) const
{
	Vector ViewForward( 1.0f, 0.0f, 0.0f );
	Vector ViewUp( 0.0f, 1.0f, 0.0f );
	Vector ViewRight( 0.0f, 0.0f, -1.0f );
	AngleVectors( m_Angles, &ViewForward, &ViewRight, &ViewUp );

	float imagewidth = m_bg.width_int;
	float imageheight = m_bg.height_int;
	
//	pVerts[0] = m_Origin + ViewRight * m_bg.center_x_int + ViewUp * m_bg.center_y_int;
//	pVerts[1] = pVerts[0] + ViewUp * imageheight;
//	pVerts[2] = pVerts[1] + ViewRight * imagewidth;
//	pVerts[3] = pVerts[0] + ViewRight * imagewidth;
	
	pVerts[0] = m_Origin + ViewRight * m_bg.center_x_int + ViewUp * m_bg.center_y_int;
	pVerts[1] = pVerts[0] + ViewRight * imagewidth;
	pVerts[2] = pVerts[1] - ViewUp * imageheight;
	pVerts[3] = pVerts[0] - ViewUp * imageheight;
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
void CImagePanelHelper::CalcBounds( BOOL bFullUpdate )
{
	CMapClass::CalcBounds(bFullUpdate);

	ComputeCornerVertices( m_corners );

	Vector vMin = m_corners[0].Min( m_corners[1] ).Min( m_corners[2] ).Min( m_corners[3] );
	Vector vMax = m_corners[0].Max( m_corners[1] ).Max( m_corners[2] ).Max( m_corners[3] );
		
	// bloat the bbox a little so it's not 0 width when seen from the sides
	Vector faceNormal = GetNormalFromPoints(m_corners[0], m_corners[1], m_corners[2]);		
	vMin += faceNormal * 4.0f;
	vMax -= faceNormal * 4.0f;

	m_CullBox.UpdateBounds( vMin, vMax );
	m_Render2DBox.UpdateBounds( vMin, vMax );
}

//-----------------------------------------------------------------------------
// Purpose: Returns a copy of this object.
// Output : Pointer to the new object.
//-----------------------------------------------------------------------------
CMapClass* CImagePanelHelper::Copy( bool bUpdateDependencies )
{
	CImagePanelHelper* pCopy = new CImagePanelHelper;

	if ( pCopy != NULL )
		pCopy->CopyFrom( this, bUpdateDependencies );

	return pCopy;
}

//-----------------------------------------------------------------------------
// Purpose: Turns this into a duplicate of the given object.
// Input  : pObject - Pointer to the object to copy from.
// Output : Returns a pointer to this object.
//-----------------------------------------------------------------------------
CMapClass* CImagePanelHelper::CopyFrom( CMapClass* pObject, bool bUpdateDependencies )
{
	CImagePanelHelper* pFrom = dynamic_cast<CImagePanelHelper*>( pObject );
	Assert( pFrom != NULL );

	if ( pFrom != NULL )
	{
		CMapClass::CopyFrom( pObject, bUpdateDependencies );

		m_Angles = pFrom->m_Angles;
		m_pFilename = pFrom->m_pFilename;
		SetRenderMode( pFrom->m_eRenderMode );
		m_RenderColor = pFrom->m_RenderColor;
	}

	return this;
}

//-----------------------------------------------------------------------------
// Purpose: Reads the TGA file
//-----------------------------------------------------------------------------
bool CImagePanelHelper::ReadImageData(CString &filename)
{
	if( m_bg_loaded_bool) return true; 

	enum ImageFormat	imageFormat;
	float				sourceGamma;
	CUtlBuffer			buf;

	bool				psd; // if false, dealing with TGA

	if ( !g_pFullFileSystem->ReadFile( filename, NULL, buf ) )
	{
		return false;
	}

	if (m_bg_pixels)
	{
		delete[] m_bg_pixels;
	}

	if (m_bg_tex)
	{
		m_bg_tex->DecrementReferenceCount();
	}
	if (m_bg_mat)
	{
		m_bg_mat->DecrementReferenceCount();
	}
#ifdef SLE_IMAGEPANEL_USE_STBI

	int comp;
	auto newPixels = stbi_load(filename, &m_bg_width_int, &m_bg_height_int, &comp, 0);
	if ( !newPixels)
	{
		static bool reportedFail;
		if (!reportedFail)
		{
			CString str;
			str.Format("Image failed to load (%s)", stbi_failure_reason());
			AfxMessageBox(str, MB_OK | MB_ICONEXCLAMATION);
			reportedFail = true;
		}
		return false;
	}

	if ( comp < 3 )
	{
		free(newPixels);
		static bool reportedFail;
		if (!reportedFail)
		{
			CString str;
			str.Format("Image failed to load (image has too few channels)");
			AfxMessageBox(str, MB_OK | MB_ICONEXCLAMATION);
			reportedFail = true;
		}
		return false;
	}
	
	m_bg_pixels = newPixels;

	char *pixels = reinterpret_cast<char *>(m_bg_pixels);

	static bool showedpixels;
	if (!showedpixels)
	{
		AfxMessageBox(pixels);
		showedpixels = true;
	}

#else	
	if ( IsPSDFile(buf))
	{
		psd = true;
		bool ok = PSDGetInfo( buf, &m_bg_width_int, &m_bg_height_int, &imageFormat, &sourceGamma );
		if ( !ok )
		{			
		//	AfxMessageBox("Cannot get PSD Info");
			return false;
		}

		psd = false;
	}
	else
	{
		psd = false;
		if ( !TGALoader::GetInfo(buf, &m_bg_width_int, &m_bg_height_int, &imageFormat, &sourceGamma) )
		{
			return false;
		}
	}

	if (m_bg_width_int % 4 != 0 || m_bg_height_int % 4 != 0)
	{
		AfxMessageBox("The source image's dimensions must both be a factor of 4! Aborting the load.", MB_OK | MB_ICONEXCLAMATION);

		// show the dims so the user knows they're bad
		char printvalue[32];
		CMapEntity *pEntity = (CMapEntity*)GetParent();
		if (pEntity)
		{
			sprintf(printvalue, "%ix%i", m_bg_width_int, m_bg_height_int);
			pEntity->NotifyChildKeyChanged(this, "image_sourcedims", printvalue);
			NotifyDependents(Notify_Changed);
		}

		m_bg_loaded_bool = true; // don't try again unless changed
		return false;
	}

	int memRequired = ImageLoader::GetMemRequired(m_bg_width_int, m_bg_height_int, 1, imageFormat, false);
	m_bg_pixels = new unsigned char[ memRequired ];

	buf.SeekGet(CUtlBuffer::SEEK_HEAD, 0);
	
	if ( psd )
	{
		Bitmap_t bmPsdData;
		if ( !PSDReadFileRGBA8888(buf, bmPsdData) )
		{
		//	AfxMessageBox("Cannot perform PSDReadFileRGBA8888()");
			return false;
		}
		CUtlMemory<uint8> tmpDest( 0, m_bg.image_width_int * m_bg.image_height_int * 4 );
		
		ImageLoader::ResampleInfo_t resInfo;
		resInfo.m_pSrc = bmPsdData.GetBits();
		resInfo.m_pDest = tmpDest.Base();
		resInfo.m_nSrcWidth = m_bg_width_int;
		resInfo.m_nSrcHeight = m_bg_height_int;
		resInfo.m_nDestWidth = m_bg_width_int;
		resInfo.m_nDestHeight = m_bg_height_int;
		resInfo.m_flSrcGamma = sourceGamma;
		resInfo.m_flDestGamma = sourceGamma;

		ResampleRGBA8888( resInfo );
		
		ImageLoader::ConvertImageFormat( tmpDest.Base(), IMAGE_FORMAT_RGBA8888, m_bg_pixels,
										 IMAGE_FORMAT_BGR888, m_bg.image_width_int, m_bg.image_height_int, 
										 0, 0 );
	}
	else
		TGALoader::Load(m_bg_pixels, buf, m_bg_width_int, m_bg_height_int, imageFormat, sourceGamma, false);
#endif // SLE_IMAGEPANEL_USE_STBI

	/* // debugging
	char *pixels = reinterpret_cast<char *>(m_bg_pixels);

	static bool showedpixels;
	if (!showedpixels)
	{
		AfxMessageBox(pixels);
		showedpixels = true;
	}
	*/
	m_bg_tex = dynamic_cast< ITextureInternal * >( g_pMaterialSystem->CreateProceduralTexture("BackgroundImage", TEXTURE_GROUP_OTHER, m_bg_width_int, m_bg_height_int, IMAGE_FORMAT_BGR888,
		TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_PROCEDURAL) );

	ITextureRegenerator *pRegen = new CBackgroundRegenerator(m_bg_pixels, m_bg_width_int, m_bg_height_int, imageFormat);
	m_bg_tex->SetTextureRegenerator(pRegen);
	m_bg_tex->Download();
	m_bg_tex->IncrementReferenceCount();
	m_bg_tex->AddRef();
		
	m_bg_mat = g_pMaterialSystem->CreateMaterial("editor/background2d", new KeyValues("UnlitGeneric", "$translucent", "1", "$nofog", "1"));
	m_bg_mat->FindVar("$BaseTexture", nullptr)->SetTextureValue(m_bg_tex);
	m_bg_mat->FindVar("$alpha", nullptr)->SetFloatValue(m_bg.alpha_float); // at 0 it becomes unselectable
	m_bg_mat->FindVar("$nofog", nullptr)->SetIntValue(1);
	if( m_bg.is_nocull_bool) m_bg_mat->FindVar("$nocull", nullptr)->SetIntValue(1);
	else  m_bg_mat->FindVar("$nocull", nullptr)->SetIntValue(0);
	m_bg_mat->IncrementReferenceCount();

	char printvalue[ 32 ];
	CMapEntity *pEntity = ( CMapEntity* )GetParent();
	if ( pEntity )
	{
		sprintf(printvalue, "%ix%i", m_bg_width_int, m_bg_height_int);
		pEntity->NotifyChildKeyChanged(this, "image_sourcedims", printvalue);
		NotifyDependents(Notify_Changed);
	}
	
	m_bg_loaded_bool = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Renders the helper in either 2D or 3D render.
// Input:	pRender2D - the 2D render
//			pRender3D - the 3D render
//			Should only use one or the other!
//-----------------------------------------------------------------------------
void CImagePanelHelper::RenderImagePanel(CRender2D* pRender2D, CRender3D* pRender3D, CString &filename )
{
	bool skipRender = false; 

	if (Options.general.bShowEditorObjects)
		return;

	if( !m_bg.is_enabled_bool) skipRender = true;
	if (m_bg.width_int == 0) skipRender = true;
	if (m_bg.height_int == 0) skipRender = true;

	int nNumChars = V_strlen( filename );
	if ( !nNumChars )
		return;
	
	if ( !ReadImageData(filename) )
	{
		skipRender = true;
	}

	if( !m_bg_mat)
	{
		skipRender = true;
	}

	if(!pRender2D && !pRender3D) return;
		
	if ( !skipRender )
	{
	// Draw via meshbuilder.
		if ( pRender2D )
		{
			pRender2D->PushRenderMode(RENDER_MODE_TEXTURED);
		} 
		else if ( pRender3D )
		{
			pRender3D->PushRenderMode(RENDER_MODE_TEXTURED);
		}

	//	CString str;
	//	str.Format("Width %i, height %i, alpha %.1f, offset x %i, offset y %i", m_bg.width_int, m_bg.height_int, m_bg.alpha_float, m_bg.center_x_int, m_bg.center_y_int);
	//	AfxMessageBox(str);
		CMatRenderContextPtr pRenderContext(MaterialSystemInterface());
		pRenderContext->Bind(m_bg_mat);
		IMesh* pMesh = pRenderContext->GetDynamicMesh(false, NULL, NULL, m_bg_mat);

		// 0 0 0
		// 0 1 0
		// 0 1 1
		// 0 0 1

		CMeshBuilder meshBuilder;
		meshBuilder.Begin(pMesh, MATERIAL_QUADS, 4);
		meshBuilder.Position3f(m_corners[ 0 ].x, m_corners[ 0 ].y, m_corners[ 0 ].z);
		meshBuilder.TexCoord2f(0, 0.0f, 0.0f);
		meshBuilder.Color4ub(255, 255, 255, 255);
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f(m_corners[ 1 ].x, m_corners[ 1 ].y, m_corners[ 1 ].z);
		meshBuilder.TexCoord2f(0, (m_bg.is_flipped_x ? -1.0f : 1.0f), 0.0f);
		meshBuilder.Color4ub(255, 255, 255, 255);
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f(m_corners[ 2 ].x, m_corners[ 2 ].y, m_corners[ 2 ].z);
		meshBuilder.TexCoord2f(0, (m_bg.is_flipped_x ? -1.0f : 1.0f), (m_bg.is_flipped_y ? -1.0f : 1.0f));
		meshBuilder.Color4ub(255, 255, 255, 255);
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f(m_corners[ 3 ].x, m_corners[ 3 ].y, m_corners[ 3 ].z);
		meshBuilder.TexCoord2f(0, 0.0f, (m_bg.is_flipped_y ? -1.0f : 1.0f));
		meshBuilder.Color4ub(255, 255, 255, 255);
		meshBuilder.AdvanceVertex();
		meshBuilder.End();
		pMesh->Draw();

		if ( pRender2D )
		{
			pRender2D->PopRenderMode();
		} 
		else if ( pRender3D )
		{
			pRender3D->PopRenderMode();
		}
	}
	// draw poly line around the border, even if the image is missing
	if ( pRender2D )
	{
		pRender2D->PushRenderMode(RENDER_MODE_WIREFRAME);
		pRender2D->SetDrawColor(255, 0, 0);
		pRender2D->DrawPolyLine(4, m_corners);
		pRender2D->PopRenderMode();
	}
	else if ( pRender3D )
	{
		pRender3D->BeginRenderHitTarget( this );
		pRender3D->PushRenderMode(RENDER_MODE_WIREFRAME);
		pRender3D->SetDrawColor(255, 0, 0);
		pRender3D->DrawPolyLine(4, m_corners);
		pRender3D->PopRenderMode();
		pRender3D->EndRenderHitTarget();
	}
}

//-----------------------------------------------------------------------------
void CImagePanelHelper::Render2D( CRender2D* pRender )
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

	// Draw the text itself, if it's supposed to be visible
	if ( m_show2D_bool )
	{
		CString str;
		str.Format("%s", m_pFilename);
		RenderImagePanel(pRender, NULL, str);
	}
}

//-----------------------------------------------------------------------------
void CImagePanelHelper::Render3D( CRender3D* pRender )
{
	pRender->BeginRenderHitTarget( this );
	// Draw the text itself, if it's supposed to be visible
	if ( m_show3D_bool )
	{
		CString str;
		str.Format("%s", m_pFilename);
		RenderImagePanel(NULL, pRender, str);
	}
	pRender->EndRenderHitTarget();

	if ( GetSelectionState() != SELECT_NONE )
	{
		// Selection box
		pRender->PushRenderMode( RENDER_MODE_WIREFRAME );
	//	Vector cornerVerts[4];
	//	ComputeCornerVertices( cornerVerts, 0.2f );
		pRender->SetDrawColor( 255, 255, 0 );
	//	pRender->DrawLine( cornerVerts[0], cornerVerts[1] );
	//	pRender->DrawLine( cornerVerts[1], cornerVerts[2] );
	//	pRender->DrawLine( cornerVerts[2], cornerVerts[3] );
	//	pRender->DrawLine( cornerVerts[3], cornerVerts[0] );
		pRender->DrawPolyLine(4, m_corners);

		pRender->PopRenderMode();
	}
}

//-----------------------------------------------------------------------------
void CImagePanelHelper::DoTransform( const VMatrix& matrix )
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
void CImagePanelHelper::OnParentKeyChanged( const char* szKey, const char* szValue )
{
	if ( !stricmp( szKey, "image_enabled" ) )
	{
		m_bg.is_enabled_bool = atoi(szValue) == 1;
		PostUpdate( Notify_Changed );
	}	
	else if ( !stricmp( szKey, "image_flip_x" ) )
	{
		m_bg.is_flipped_x = atoi(szValue) == 1;
		PostUpdate( Notify_Changed );
	}	
	else if ( !stricmp( szKey, "image_flip_y" ) )
	{
		m_bg.is_flipped_y = atoi(szValue) == 1;
		PostUpdate( Notify_Changed );
	}	
	else if ( !stricmp( szKey, "image_nocull" ) )
	{
		m_bg_loaded_bool = false; // force reload
		m_bg.is_nocull_bool = atoi(szValue) == 1;
		PostUpdate( Notify_Changed );
	}
	else if ( !stricmp( szKey, "image_filename" ) )
	{
		m_bg_loaded_bool = false; // force reload
		SetImageFilename( szValue );
		PostUpdate( Notify_Changed );
	}
	else if ( !stricmp( szKey, "image_dims" ) )
	{
		int w, h;
		sscanf( szValue, "%i %i", &w, &h );
		m_bg.width_int = w;
		m_bg.height_int = h;
		PostUpdate( Notify_Changed );
	}
	else if ( !stricmp( szKey, "image_alpha" ) )
	{
		m_bg_loaded_bool = false; // force reload
		float f;
		sscanf( szValue, "%f", &f );
		m_bg.alpha_float = f;
		PostUpdate( Notify_Changed );
	}
	else if ( !stricmp( szKey, "image_center" ) )
	{
		int x, y;
		sscanf( szValue, "%i %i", &x, &y );
		m_bg.center_x_int = x;
		m_bg.center_y_int = y;
		PostUpdate( Notify_Changed );
	}

	else if ( !stricmp( szKey, "angles" ) )
	{
		sscanf( szValue, "%f %f %f", &m_Angles[PITCH], &m_Angles[YAW], &m_Angles[ROLL] );
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
	else if ( !stricmp(szKey, "showin2d") )
	{
		if( atoi(szValue) == 1)
			m_show2D_bool = true;
		else
			m_show2D_bool = false;
	}
	else if ( !stricmp(szKey, "showin3d") )
	{
		if( atoi(szValue) == 1)
			m_show3D_bool = true;
		else
			m_show3D_bool = false;
	}
}

void CImagePanelHelper::SetImageFilename(const char* pNewText)
{
	m_pFilename = pNewText;
}
