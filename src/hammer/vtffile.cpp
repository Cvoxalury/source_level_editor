#include "stdafx.h"

#include "vtffile.h"
#include "filesystem.h"
#include <cstring>
#include <cmath>

#include "tier0/memdbgon.h"

enum CubeMapFaceIndex_t
{
	CUBEMAP_FACE_RIGHT = 0,
	CUBEMAP_FACE_LEFT,
	CUBEMAP_FACE_BACK,	// NOTE: This face is in the +y direction?!?!?
	CUBEMAP_FACE_FRONT,	// NOTE: This face is in the -y direction!?!?
	CUBEMAP_FACE_UP,
	CUBEMAP_FACE_DOWN,

	// This is the fallback for low-end
	CUBEMAP_FACE_SPHEREMAP,

	// NOTE: Cubemaps have *7* faces; the 7th is the fallback spheremap
	CUBEMAP_FACE_COUNT
};

#define VTF_MAJOR_VERSION 7
#define VTF_MINOR_VERSION 5
#define VTF_MINOR_VERSION_DEFAULT 5
#define VTF_MINOR_VERSION_MIN_SPHERE_MAP	1
#define VTF_MINOR_VERSION_MIN_VOLUME		2
#define VTF_MINOR_VERSION_MIN_RESOURCE		3
#define VTF_MINOR_VERSION_MIN_NO_SPHERE_MAP	5

CVTFFile::CVTFFile()
{
	Header = nullptr;

	uiImageBufferSize = 0;
	lpImageData = nullptr;

	uiThumbnailBufferSize = 0;
	lpThumbnailImageData = nullptr;
}

CVTFFile::~CVTFFile()
{
	Destroy();
}

void CVTFFile::Destroy()
{
	if ( Header != nullptr )
		for ( unsigned int i = 0; i < Header->ResourceCount; i++ )
			delete[] Header->Data[i].Data;

	delete Header;
	Header = 0;

	uiImageBufferSize = 0;
	delete[] lpImageData;
	lpImageData = nullptr;

	uiThumbnailBufferSize = 0;
	delete[] lpThumbnailImageData;
	lpThumbnailImageData = nullptr;
}

bool CVTFFile::IsPowerOfTwo( unsigned int uiSize )
{
	return uiSize > 0 && ( uiSize & ( uiSize - 1 ) ) == 0;
}

unsigned int CVTFFile::NextPowerOfTwo( unsigned int uiSize )
{
	if ( uiSize == 0 )
		return 1;

	if ( IsPowerOfTwo( uiSize ) )
		return uiSize;

	uiSize--;
	for ( unsigned int i = 1; i <= sizeof( unsigned int ) * 4; i <<= 1 )
		uiSize = uiSize | ( uiSize >> i );
	uiSize++;

	return uiSize;
}

bool CVTFFile::IsLoaded() const
{
	return Header != nullptr;
}

bool CVTFFile::Load( FileHandle_t Reader, bool bHeaderOnly )
{
	Destroy();

	unsigned int uiThumbnailBufferOffset = 0, uiImageDataOffset = 0;
	unsigned int uiFileSize = g_pFullFileSystem->Size( Reader );
	SVTFFileHeader FileHeader{};
	if ( uiFileSize < sizeof( SVTFFileHeader ) )
		goto failed;

	if ( g_pFullFileSystem->Read( &FileHeader, sizeof( SVTFFileHeader ), Reader ) != (int)sizeof( SVTFFileHeader ) )
		goto failed;

	if ( memcmp( FileHeader.TypeString, "VTF\0", 4 ) != 0 )
		goto failed;

	if ( FileHeader.Version[0] != VTF_MAJOR_VERSION || ( FileHeader.Version[1] < 0 || FileHeader.Version[1] > VTF_MINOR_VERSION ) )
		goto failed;

	if ( FileHeader.HeaderSize > sizeof( SVTFHeader ) )
		goto failed;

	g_pFullFileSystem->Seek( Reader, 0, FILESYSTEM_SEEK_HEAD );

#pragma push_macro( "new" )
#undef new
	Header = new SVTFHeader;
#pragma pop_macro( "new" )
	memset( Header, 0, sizeof( SVTFHeader ) );

	if ( g_pFullFileSystem->Read( Header, FileHeader.HeaderSize, Reader ) != (int)FileHeader.HeaderSize )
		goto failed;

	if ( Header->Version[0] < VTF_MAJOR_VERSION || ( Header->Version[0] == VTF_MAJOR_VERSION && Header->Version[1] < VTF_MINOR_VERSION_MIN_VOLUME ) )
		Header->Depth = 1;

	Header->ResourceCount = 0;

	if ( bHeaderOnly )
		return true;

	{
		const auto oldFrames = Header->Frames;
		Header->Frames = 1;                    // temp override
		uiImageBufferSize = CVTFFile::ComputeImageSize( Header->Width, Header->Height, 1, 1, Header->Format ) * GetFaceCount() * GetFrameCount();
		Header->Frames = oldFrames; // revert
	}

	if ( Header->LowResImageFormat != IMAGE_FORMAT_UNKNOWN )
		uiThumbnailBufferSize = CVTFFile::ComputeImageSize( Header->LowResImageWidth, Header->LowResImageHeight, 1, Header->LowResImageFormat );
	else
		uiThumbnailBufferSize = 0;

	if ( Header->ResourceCount )
	{
		if ( Header->ResourceCount > VTF__RSRC_MAX_DICTIONARY_ENTRIES )
			goto failed;

		for ( unsigned int i = 0; i < Header->ResourceCount; i++ )
		{
			switch ( Header->Resources[i].Type )
			{
			case VTF__LEGACY_RSRC_LOW_RES_IMAGE:
				if ( Header->LowResImageFormat == IMAGE_FORMAT_UNKNOWN )
					goto failed;
				if ( uiThumbnailBufferOffset != 0 )
					goto failed;
				uiThumbnailBufferOffset = Header->Resources[i].Data;
				break;
			case VTF__LEGACY_RSRC_IMAGE:
				if ( uiImageDataOffset != 0 )
					goto failed;
				uiImageDataOffset = Header->Resources[i].Data;
				break;
			default:
				if ( ( Header->Resources[i].Flags & RSRCF__HAS_NO_DATA_CHUNK ) == 0 )
				{
					if ( Header->Resources[i].Data + sizeof( unsigned int ) > uiFileSize )
						goto failed;

					unsigned int uiSize = 0;
					g_pFullFileSystem->Seek( Reader, Header->Resources[i].Data, FILESYSTEM_SEEK_HEAD );
					if ( g_pFullFileSystem->Read( &uiSize, sizeof( unsigned int ), Reader ) != (int)sizeof( unsigned int ) )
						goto failed;

					if ( Header->Resources[i].Data + sizeof( unsigned int ) + uiSize > uiFileSize )
						goto failed;

					Header->Data[i].Size = uiSize;
					Header->Data[i].Data = new unsigned char[uiSize];
					if ( g_pFullFileSystem->Read( Header->Data[i].Data, uiSize, Reader ) != (int)uiSize )
						goto failed;
				}
				break;
			}
		}
	}
	else
	{
		uiThumbnailBufferOffset = Header->HeaderSize;
		uiImageDataOffset = uiThumbnailBufferOffset + uiThumbnailBufferSize;
	}

	if ( Header->HeaderSize > uiFileSize || uiThumbnailBufferOffset + uiThumbnailBufferSize > uiFileSize || uiImageDataOffset + uiImageBufferSize > uiFileSize )
		goto failed;

	if ( uiThumbnailBufferOffset == 0 )
		Header->LowResImageFormat = IMAGE_FORMAT_UNKNOWN;

#if 0 // do not load thumbnail
	if ( Header->LowResImageFormat != IMAGE_FORMAT_UNKNOWN )
	{
		lpThumbnailImageData = new unsigned char[uiThumbnailBufferSize];

		Reader.SeekGet( CUtlBuffer::SEEK_HEAD, uiThumbnailBufferOffset );
		if ( Reader.Get( lpThumbnailImageData, uiThumbnailBufferSize ); !Reader.IsValid() )
			goto failed;
	}
#endif

	if ( uiImageDataOffset == 0 )
		Header->Format = IMAGE_FORMAT_UNKNOWN;

	if ( Header->Format != IMAGE_FORMAT_UNKNOWN )
	{
		lpImageData = new unsigned char[uiImageBufferSize];

		g_pFullFileSystem->Seek( Reader, uiImageDataOffset + ComputeDataOffset( 0, 0, 0, 0, Header->Format ), FILESYSTEM_SEEK_HEAD );
		Header->MipCount = 1; // override
		Header->Depth = 1;    // override
		Header->Frames = 1;   // override
		if ( g_pFullFileSystem->Read( lpImageData, uiImageBufferSize, Reader ) != (int)uiImageBufferSize )
			goto failed;
	}

	g_pFullFileSystem->Close( Reader );
	return true;

failed:
	Destroy();
	g_pFullFileSystem->Close( Reader );
	return false;
}

unsigned int CVTFFile::GetWidth() const
{
	if ( !IsLoaded() )
		return 0;

	return Header->Width;
}

unsigned int CVTFFile::GetHeight() const
{
	if ( !IsLoaded() )
		return 0;

	return Header->Height;
}

unsigned int CVTFFile::GetDepth() const
{
	if ( !IsLoaded() )
		return 0;

	return Header->Depth;
}

unsigned int CVTFFile::GetFrameCount() const
{
	if ( !IsLoaded() )
		return 0;

	return Header->Frames;
}

unsigned int CVTFFile::GetFlags() const
{
	if ( !IsLoaded() )
		return 0;

	return Header->Flags;
}

unsigned int CVTFFile::GetFaceCount() const
{
	const unsigned int TEXTUREFLAGS_ENVMAP = 0x00004000;
	if ( !IsLoaded() )
		return 0;

	return Header->Flags & TEXTUREFLAGS_ENVMAP ? ( Header->StartFrame != 0xffff && Header->Version[1] < VTF_MINOR_VERSION_MIN_NO_SPHERE_MAP ? CUBEMAP_FACE_COUNT : CUBEMAP_FACE_COUNT - 1 ) : 1;
}

unsigned int CVTFFile::GetMipmapCount() const
{
	if ( !IsLoaded() )
		return 0;

	return Header->MipCount;
}

unsigned char* CVTFFile::GetData( unsigned int uiFrame, unsigned int uiFace, unsigned int uiSlice, unsigned int uiMipmapLevel ) const
{
	if ( !IsLoaded() )
		return 0;

	return lpImageData + ComputeDataOffset( uiFrame, uiFace, uiSlice, uiMipmapLevel, Header->Format );
}

ImageFormat CVTFFile::GetFormat() const
{
	if ( !IsLoaded() )
		return IMAGE_FORMAT_UNKNOWN;

	return Header->Format;
}

static const SImageFormatInfo ImageFormatInfo[] =
{
	{ "RGBA8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_RGBA8888,
	{ "ABGR8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_ABGR8888,
	{ "RGB888",				 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_RGB888,
	{ "BGR888",				 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_BGR888,
	{ "RGB565",				 16,  2,  5,  6,  5,  0, false,  true },		// IMAGE_FORMAT_RGB565,
	{ "I8",					  8,  1,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_I8,
	{ "IA88",				 16,  2,  0,  0,  0,  8, false,  true },		// IMAGE_FORMAT_IA88
	{ "P8",					  8,  1,  0,  0,  0,  0, false, false },		// IMAGE_FORMAT_P8
	{ "A8",					  8,  1,  0,  0,  0,  8, false,  true },		// IMAGE_FORMAT_A8
	{ "RGB888 Bluescreen",	 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_RGB888_BLUESCREEN
	{ "BGR888 Bluescreen",	 24,  3,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_BGR888_BLUESCREEN
	{ "ARGB8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_ARGB8888
	{ "BGRA8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_BGRA8888
	{ "DXT1",				  4,  0,  0,  0,  0,  0,  true,  true },		// IMAGE_FORMAT_DXT1
	{ "DXT3",				  8,  0,  0,  0,  0,  8,  true,  true },		// IMAGE_FORMAT_DXT3
	{ "DXT5",				  8,  0,  0,  0,  0,  8,  true,  true },		// IMAGE_FORMAT_DXT5
	{ "BGRX8888",			 32,  4,  8,  8,  8,  0, false,  true },		// IMAGE_FORMAT_BGRX8888
	{ "BGR565",				 16,  2,  5,  6,  5,  0, false,  true },		// IMAGE_FORMAT_BGR565
	{ "BGRX5551",			 16,  2,  5,  5,  5,  0, false,  true },		// IMAGE_FORMAT_BGRX5551
	{ "BGRA4444",			 16,  2,  4,  4,  4,  4, false,  true },		// IMAGE_FORMAT_BGRA4444
	{ "DXT1 One Bit Alpha",	  4,  0,  0,  0,  0,  1,  true,  true },		// IMAGE_FORMAT_DXT1_ONEBITALPHA
	{ "BGRA5551",			 16,  2,  5,  5,  5,  1, false,  true },		// IMAGE_FORMAT_BGRA5551
	{ "UV88",				 16,  2,  8,  8,  0,  0, false,  true },		// IMAGE_FORMAT_UV88
	{ "UVWQ8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_UVWQ8899
	{ "RGBA16161616F",		 64,  8, 16, 16, 16, 16, false,  true },		// IMAGE_FORMAT_RGBA16161616F
	{ "RGBA16161616",		 64,  8, 16, 16, 16, 16, false,  true },		// IMAGE_FORMAT_RGBA16161616
	{ "UVLX8888",			 32,  4,  8,  8,  8,  8, false,  true },		// IMAGE_FORMAT_UVLX8888
	{ "R32F",				 32,  4, 32,  0,  0,  0, false,  true },		// IMAGE_FORMAT_R32F
	{ "RGB323232F",			 96, 12, 32, 32, 32,  0, false,  true },		// IMAGE_FORMAT_RGB323232F
	{ "RGBA32323232F",		128, 16, 32, 32, 32, 32, false,  true },		// IMAGE_FORMAT_RGBA32323232F
	{ "nVidia DST16",		 16,  2,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_DST16
	{ "nVidia DST24",		 24,  3,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_DST24
	{ "nVidia INTZ",		 32,  4,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_INTZ
	{ "nVidia RAWZ",		 32,  4,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_RAWZ
	{ "ATI DST16",			 16,  2,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_ATI_DST16
	{ "ATI DST24",			 24,  3,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_ATI_DST24
	{ "nVidia NULL",		 32,  4,  0,  0,  0,  0, false,  true },		// IMAGE_FORMAT_NV_NULL
	{ "ATI1N",				  4,  0,  0,  0,  0,  0,  true,  true },		// IMAGE_FORMAT_ATI1N
	{ "ATI2N",				  8,  0,  0,  0,  0,  0,  true,  true }			// IMAGE_FORMAT_ATI2N
};

SImageFormatInfo const &CVTFFile::GetImageFormatInfo( ImageFormat ImageFormat )
{
	return ImageFormatInfo[ImageFormat];
}

unsigned int CVTFFile::ComputeImageSize( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth, ImageFormat ImageFormat )
{
	switch ( ImageFormat )
	{
	case IMAGE_FORMAT_DXT1:
	case IMAGE_FORMAT_DXT1_ONEBITALPHA:
		if ( uiWidth < 4 && uiWidth > 0 )
			uiWidth = 4;

		if ( uiHeight < 4 && uiHeight > 0 )
			uiHeight = 4;

		return ( ( uiWidth + 3 ) / 4 ) * ( ( uiHeight + 3 ) / 4 ) * 8 * uiDepth;
	case IMAGE_FORMAT_DXT3:
	case IMAGE_FORMAT_DXT5:
		if ( uiWidth < 4 && uiWidth > 0 )
			uiWidth = 4;

		if ( uiHeight < 4 && uiHeight > 0 )
			uiHeight = 4;

		return ( ( uiWidth + 3 ) / 4 ) * ( ( uiHeight + 3 ) / 4 ) * 16 * uiDepth;
	default:
		return uiWidth * uiHeight * uiDepth * CVTFFile::GetImageFormatInfo( ImageFormat ).uiBytesPerPixel;
	}
}

unsigned int CVTFFile::ComputeImageSize( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth, unsigned int uiMipmaps, ImageFormat ImageFormat )
{
	unsigned int uiImageSize = 0;

	for ( unsigned int i = 0; i < uiMipmaps; i++ )
	{
		uiImageSize += CVTFFile::ComputeImageSize( uiWidth, uiHeight, uiDepth, ImageFormat );

		uiWidth >>= 1;
		uiHeight >>= 1;
		uiDepth >>= 1;

		if ( uiWidth < 1 )
			uiWidth = 1;

		if ( uiHeight < 1 )
			uiHeight = 1;

		if ( uiDepth < 1 )
			uiDepth = 1;
	}

	return uiImageSize;
}

unsigned int CVTFFile::ComputeMipmapCount( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth )
{
	unsigned int uiCount = 0;

	while ( true )
	{
		uiCount++;

		uiWidth >>= 1;
		uiHeight >>= 1;
		uiDepth >>= 1;

		if ( uiWidth == 0 && uiHeight == 0 && uiDepth == 0 )
			break;
	}

	return uiCount;
}

void CVTFFile::ComputeMipmapDimensions( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth, unsigned int uiMipmapLevel, unsigned int& uiMipmapWidth, unsigned int& uiMipmapHeight, unsigned int& uiMipmapDepth )
{
	uiMipmapWidth = uiWidth >> uiMipmapLevel;
	uiMipmapHeight = uiHeight >> uiMipmapLevel;
	uiMipmapDepth = uiDepth >> uiMipmapLevel;

	if ( uiMipmapWidth < 1 )
		uiMipmapWidth = 1;

	if ( uiMipmapHeight < 1 )
		uiMipmapHeight = 1;

	if ( uiMipmapDepth < 1 )
		uiMipmapDepth = 1;
}

unsigned int CVTFFile::ComputeMipmapSize( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth, unsigned int uiMipmapLevel, ImageFormat ImageFormat )
{
	unsigned int uiMipmapWidth, uiMipmapHeight, uiMipmapDepth;
	CVTFFile::ComputeMipmapDimensions( uiWidth, uiHeight, uiDepth, uiMipmapLevel, uiMipmapWidth, uiMipmapHeight, uiMipmapDepth );

	return CVTFFile::ComputeImageSize( uiMipmapWidth, uiMipmapHeight, uiMipmapDepth, ImageFormat );
}

unsigned int CVTFFile::ComputeDataOffset( unsigned int uiFrame, unsigned int uiFace, unsigned int uiSlice, unsigned int uiMipLevel, ImageFormat ImageFormat ) const
{
	unsigned int uiOffset = 0;

	unsigned int uiFrameCount = GetFrameCount();
	unsigned int uiFaceCount = GetFaceCount();
	unsigned int uiSliceCount = GetDepth();
	unsigned int uiMipCount = GetMipmapCount();

	if ( uiFrame >= uiFrameCount )
		uiFrame = uiFrameCount - 1;

	if ( uiFace >= uiFaceCount )
		uiFace = uiFaceCount - 1;

	if ( uiSlice >= uiSliceCount )
		uiSlice = uiSliceCount - 1;

	if ( uiMipLevel >= uiMipCount )
		uiMipLevel = uiMipCount - 1;

	for ( unsigned int i = uiMipCount - 1; i > uiMipLevel; --i )
		uiOffset += ComputeMipmapSize( Header->Width, Header->Height, Header->Depth, i, ImageFormat ) * uiFrameCount * uiFaceCount;

	unsigned int uiTemp1 = ComputeMipmapSize( Header->Width, Header->Height, Header->Depth, uiMipLevel, ImageFormat );
	unsigned int uiTemp2 = ComputeMipmapSize( Header->Width, Header->Height, 1, uiMipLevel, ImageFormat );

	uiOffset += uiTemp1 * uiFrame * uiFaceCount * uiSliceCount;
	uiOffset += uiTemp1 * uiFace * uiSliceCount;
	uiOffset += uiTemp2 * uiSlice;

	return uiOffset;
}

struct Colour8888
{
	unsigned char r;		// change the order of names to change the
	unsigned char g;		// order of the output ARGB or BGRA, etc...
	unsigned char b;		// Last one is MSB, 1st is LSB.
	unsigned char a;
};

struct Colour565
{
	unsigned int nBlue : 5;		// order of names changes
	unsigned int nGreen : 6;		// byte order of output to 32 bit
	unsigned int nRed : 5;
};

struct DXTAlphaBlockExplicit
{
	short row[4];
};

bool CVTFFile::DecompressDXT1( const unsigned char* src, unsigned char* dst, unsigned int uiWidth, unsigned int uiHeight )
{
	unsigned int		x, y, i, j, k, Select;
	const unsigned char* Temp;
	Colour565			*color_0, *color_1;
	Colour8888			colours[4], *col;
	unsigned int		bitmask, Offset;

	const unsigned char nBpp = 4;
	const unsigned char nBpc = 1;
	unsigned int iBps = nBpp * nBpc * uiWidth;

	Temp = src;

	for ( y = 0; y < uiHeight; y += 4 )
	{
		for ( x = 0; x < uiWidth; x += 4 )
		{
			color_0 = ( ( Colour565* )Temp );
			color_1 = ( ( Colour565* )( Temp + 2 ) );
			bitmask = ( ( unsigned int* )Temp )[1];
			Temp += 8;

			colours[0].r = color_0->nRed << 3;
			colours[0].g = color_0->nGreen << 2;
			colours[0].b = color_0->nBlue << 3;
			colours[0].a = 0xFF;

			colours[1].r = color_1->nRed << 3;
			colours[1].g = color_1->nGreen << 2;
			colours[1].b = color_1->nBlue << 3;
			colours[1].a = 0xFF;

			if ( *( ( unsigned short* )color_0 ) > *( ( unsigned short* )color_1 ) )
			{
				colours[2].b = ( 2 * colours[0].b + colours[1].b + 1 ) / 3;
				colours[2].g = ( 2 * colours[0].g + colours[1].g + 1 ) / 3;
				colours[2].r = ( 2 * colours[0].r + colours[1].r + 1 ) / 3;
				colours[2].a = 0xFF;

				colours[3].b = ( colours[0].b + 2 * colours[1].b + 1 ) / 3;
				colours[3].g = ( colours[0].g + 2 * colours[1].g + 1 ) / 3;
				colours[3].r = ( colours[0].r + 2 * colours[1].r + 1 ) / 3;
				colours[3].a = 0xFF;
			}
			else
			{
				colours[2].b = ( colours[0].b + colours[1].b ) / 2;
				colours[2].g = ( colours[0].g + colours[1].g ) / 2;
				colours[2].r = ( colours[0].r + colours[1].r ) / 2;
				colours[2].a = 0xFF;

				colours[3].b = ( colours[0].b + 2 * colours[1].b + 1 ) / 3;
				colours[3].g = ( colours[0].g + 2 * colours[1].g + 1 ) / 3;
				colours[3].r = ( colours[0].r + 2 * colours[1].r + 1 ) / 3;
				colours[3].a = 0x00;
			}

			for ( j = 0, k = 0; j < 4; j++ )
			{
				for ( i = 0; i < 4; i++, k++ )
				{
					Select = ( bitmask & ( 0x03 << k * 2 ) ) >> k * 2;
					col = &colours[Select];

					if ( ( ( x + i ) < uiWidth ) && ( ( y + j ) < uiHeight ) )
					{
						Offset = ( y + j ) * iBps + ( x + i ) * nBpp;
						dst[Offset + 0] = col->r;
						dst[Offset + 1] = col->g;
						dst[Offset + 2] = col->b;
						dst[Offset + 3] = col->a;
					}
				}
			}
		}
	}
	return true;
}

bool CVTFFile::DecompressDXT3( const unsigned char* src, unsigned char* dst, unsigned int uiWidth, unsigned int uiHeight )
{
	unsigned int		x, y, i, j, k, Select;
	const unsigned char* Temp;
	Colour565			*color_0, *color_1;
	Colour8888			colours[4], *col;
	unsigned int		bitmask, Offset;
	unsigned short		word;
	DXTAlphaBlockExplicit* alpha;

	const unsigned char nBpp = 4;
	const unsigned char nBpc = 1;
	unsigned int iBps = nBpp * nBpc * uiWidth;

	Temp = src;

	for ( y = 0; y < uiHeight; y += 4 )
	{
		for ( x = 0; x < uiWidth; x += 4 )
		{
			alpha = ( DXTAlphaBlockExplicit* )Temp;
			Temp += 8;
			color_0 = ( ( Colour565* )Temp );
			color_1 = ( ( Colour565* )( Temp + 2 ) );
			bitmask = ( ( unsigned int* )Temp )[1];
			Temp += 8;

			colours[0].r = color_0->nRed << 3;
			colours[0].g = color_0->nGreen << 2;
			colours[0].b = color_0->nBlue << 3;
			colours[0].a = 0xFF;

			colours[1].r = color_1->nRed << 3;
			colours[1].g = color_1->nGreen << 2;
			colours[1].b = color_1->nBlue << 3;
			colours[1].a = 0xFF;

			colours[2].b = ( 2 * colours[0].b + colours[1].b + 1 ) / 3;
			colours[2].g = ( 2 * colours[0].g + colours[1].g + 1 ) / 3;
			colours[2].r = ( 2 * colours[0].r + colours[1].r + 1 ) / 3;
			colours[2].a = 0xFF;

			colours[3].b = ( colours[0].b + 2 * colours[1].b + 1 ) / 3;
			colours[3].g = ( colours[0].g + 2 * colours[1].g + 1 ) / 3;
			colours[3].r = ( colours[0].r + 2 * colours[1].r + 1 ) / 3;
			colours[3].a = 0xFF;

			k = 0;
			for ( j = 0; j < 4; j++ )
			{
				for ( i = 0; i < 4; i++, k++ )
				{
					Select = ( bitmask & ( 0x03 << k * 2 ) ) >> k * 2;
					col = &colours[Select];

					if ( ( ( x + i ) < uiWidth ) && ( ( y + j ) < uiHeight ) )
					{
						Offset = ( y + j ) * iBps + ( x + i ) * nBpp;
						dst[Offset + 0] = col->r;
						dst[Offset + 1] = col->g;
						dst[Offset + 2] = col->b;
					}
				}
			}

			for ( j = 0; j < 4; j++ )
			{
				word = alpha->row[j];
				for ( i = 0; i < 4; i++ )
				{
					if ( ( ( x + i ) < uiWidth ) && ( ( y + j ) < uiHeight ) )
					{
						Offset = ( y + j ) * iBps + ( x + i ) * nBpp + 3;
						dst[Offset] = word & 0x0F;
						dst[Offset] = dst[Offset] | ( dst[Offset] << 4 );
					}

					word >>= 4;
				}
			}
		}
	}
	return true;
}

bool CVTFFile::DecompressDXT5( const unsigned char* src, unsigned char* dst, unsigned int uiWidth, unsigned int uiHeight )
{
	unsigned int		x, y, i, j, k, Select;
	const unsigned char* Temp;
	Colour565			*color_0, *color_1;
	Colour8888			colours[4], *col;
	unsigned int		bitmask, Offset;
	unsigned char		alphas[8];
	const unsigned char* alphamask;
	unsigned int		bits;

	const unsigned char nBpp = 4;
	const unsigned char nBpc = 1;
	unsigned int iBps = nBpp * nBpc * uiWidth;

	Temp = src;

	for ( y = 0; y < uiHeight; y += 4 )
	{
		for ( x = 0; x < uiWidth; x += 4 )
		{
			alphas[0] = Temp[0];
			alphas[1] = Temp[1];
			alphamask = Temp + 2;
			Temp += 8;
			color_0 = ( ( Colour565* )Temp );
			color_1 = ( ( Colour565* )( Temp + 2 ) );
			bitmask = ( ( unsigned int* )Temp )[1];
			Temp += 8;

			colours[0].r = color_0->nRed << 3;
			colours[0].g = color_0->nGreen << 2;
			colours[0].b = color_0->nBlue << 3;
			colours[0].a = 0xFF;

			colours[1].r = color_1->nRed << 3;
			colours[1].g = color_1->nGreen << 2;
			colours[1].b = color_1->nBlue << 3;
			colours[1].a = 0xFF;

			colours[2].b = ( 2 * colours[0].b + colours[1].b + 1 ) / 3;
			colours[2].g = ( 2 * colours[0].g + colours[1].g + 1 ) / 3;
			colours[2].r = ( 2 * colours[0].r + colours[1].r + 1 ) / 3;
			colours[2].a = 0xFF;

			colours[3].b = ( colours[0].b + 2 * colours[1].b + 1 ) / 3;
			colours[3].g = ( colours[0].g + 2 * colours[1].g + 1 ) / 3;
			colours[3].r = ( colours[0].r + 2 * colours[1].r + 1 ) / 3;
			colours[3].a = 0xFF;

			k = 0;
			for ( j = 0; j < 4; j++ )
			{
				for ( i = 0; i < 4; i++, k++ )
				{
					Select = ( bitmask & ( 0x03 << k * 2 ) ) >> k * 2;
					col = &colours[Select];

					if ( ( ( x + i ) < uiWidth ) && ( ( y + j ) < uiHeight ) )
					{
						Offset = ( y + j ) * iBps + ( x + i ) * nBpp;
						dst[Offset + 0] = col->r;
						dst[Offset + 1] = col->g;
						dst[Offset + 2] = col->b;
					}
				}
			}

			if ( alphas[0] > alphas[1] )
			{
				alphas[2] = ( 6 * alphas[0] + 1 * alphas[1] + 3 ) / 7;
				alphas[3] = ( 5 * alphas[0] + 2 * alphas[1] + 3 ) / 7;
				alphas[4] = ( 4 * alphas[0] + 3 * alphas[1] + 3 ) / 7;
				alphas[5] = ( 3 * alphas[0] + 4 * alphas[1] + 3 ) / 7;
				alphas[6] = ( 2 * alphas[0] + 5 * alphas[1] + 3 ) / 7;
				alphas[7] = ( 1 * alphas[0] + 6 * alphas[1] + 3 ) / 7;
			}
			else
			{
				alphas[2] = ( 4 * alphas[0] + 1 * alphas[1] + 2 ) / 5;
				alphas[3] = ( 3 * alphas[0] + 2 * alphas[1] + 2 ) / 5;
				alphas[4] = ( 2 * alphas[0] + 3 * alphas[1] + 2 ) / 5;
				alphas[5] = ( 1 * alphas[0] + 4 * alphas[1] + 2 ) / 5;
				alphas[6] = 0x00;
				alphas[7] = 0xFF;
			}

			bits = *( ( const int* )alphamask );
			for ( j = 0; j < 2; j++ )
			{
				for ( i = 0; i < 4; i++ )
				{
					if ( ( ( x + i ) < uiWidth ) && ( ( y + j ) < uiHeight ) )
					{
						Offset = ( y + j ) * iBps + ( x + i ) * nBpp + 3;
						dst[Offset] = alphas[bits & 0x07];
					}
					bits >>= 3;
				}
			}

			bits = *( ( const int* )&alphamask[3] );
			for ( j = 2; j < 4; j++ )
			{
				for ( i = 0; i < 4; i++ )
				{
					if ( ( ( x + i ) < uiWidth ) && ( ( y + j ) < uiHeight ) )
					{
						Offset = ( y + j ) * iBps + ( x + i ) * nBpp + 3;
						dst[Offset] = alphas[bits & 0x07];
					}
					bits >>= 3;
				}
			}
		}
	}
	return true;
}

float ClampFP16( const float& sValue )
{
	if ( sValue < 0.0f )
		return 0.0f;
	if ( sValue > 65335.0f )
		return 65335.0f;
	return sValue;
}

static float sHDRLogAverageLuminance;
void FromFP16( uint16& R, uint16& G, uint16& B, uint16& A )
{
	float sR = (float)R, sG = (float)G, sB = (float)B;//, sA = (float)A;

	float sY = sR * 0.299f + sG * 0.587f + sB * 0.114f;

	float sU = (sB - sY) * 0.565f;
	float sV = (sR - sY) * 0.713f;

	float sTemp = sY;

	sTemp = 4.0f * sTemp / sHDRLogAverageLuminance;
	sTemp = sTemp / (1.0f + sTemp);

	sTemp = sTemp / sY;

	R = (uint16)ClampFP16( pow( ( sY + 1.403f * sV ) * sTemp, 2.25f ) * 65535.0f );
	G = (uint16)ClampFP16( pow( ( sY - 0.344f * sU - 0.714f * sV ) * sTemp, 2.25f ) * 65535.0f );
	B = (uint16)ClampFP16( pow( ( sY + 1.770f * sU ) * sTemp, 2.25f ) * 65535.0f );
}

void ToLuminance( uint16& R, uint16& G, uint16& B, uint16& A )
{
	const float sLuminanceWeightR = 0.299f;
	const float sLuminanceWeightG = 0.587f;
	const float sLuminanceWeightB = 0.114f;
	R = G = B = ( uint16 )( sLuminanceWeightR * (float)R + sLuminanceWeightG * (float)G + sLuminanceWeightB * (float)B );
}

void FromLuminance( uint16& R, uint16& G, uint16& B, uint16& A )
{
	B = G = R;
}

void ToBlueScreen( uint16& R, uint16& G, uint16& B, uint16& A )
{
	if ( A == 0x0000 )
	{
		R = 0;
		G = 0;
		B = 0xffff;
	}
	A = 0xffff;
}

void FromBlueScreen( uint16& R, uint16& G, uint16& B, uint16& A )
{
	if ( R == 0 && G == 0 && B == 0xffff )
		R = G = B = A = 0x0000;
	else
		A = 0xffff;
}

using TransformProc = void (*)( uint16& R, uint16& G, uint16& B, uint16& A );
struct SVTFImageConvertInfo
{
	unsigned int	uiBitsPerPixel;
	unsigned int	uiBytesPerPixel;
	unsigned int	uiRBitsPerPixel;
	unsigned int	uiGBitsPerPixel;
	unsigned int	uiBBitsPerPixel;
	unsigned int	uiABitsPerPixel;
	int				iR;
	int				iG;
	int				iB;
	int				iA;
	bool			bIsCompressed;
	bool			bIsSupported;
	TransformProc	pToTransform;
	TransformProc	pFromTransform;
	ImageFormat		Format;
};

static const SVTFImageConvertInfo VTFImageConvertInfo[] =
{
	{ 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	 3,	false,  true, nullptr, nullptr,				IMAGE_FORMAT_RGBA8888 },
	{ 32,  4,  8,  8,  8,  8,	 3,	 2,	 1,	 0, false,  true, nullptr, nullptr,				IMAGE_FORMAT_ABGR8888 },
	{ 24,  3,  8,  8,  8,  0,	 0,	 1,	 2,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_RGB888 },
	{ 24,  3,  8,  8,  8,  0,	 2,	 1,	 0,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_BGR888 },
	{ 16,  2,  5,  6,  5,  0,	 0,	 1,	 2,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_RGB565 },
	{ 8,  1,  8,  8,  8,  0,	 0,	-1,	-1,	-1, false,  true, ToLuminance, FromLuminance,	IMAGE_FORMAT_I8 },
	{ 16,  2,  8,  8,  8,  8,	 0,	-1,	-1,	 1, false,  true, ToLuminance, FromLuminance,	IMAGE_FORMAT_IA88 },
	{ 8,  1,  0,  0,  0,  0,	-1,	-1,	-1,	-1, false, false, nullptr, nullptr,				IMAGE_FORMAT_P8 },
	{ 8,  1,  0,  0,  0,  8,	-1,	-1,	-1,	 0, false,  true, nullptr, nullptr,				IMAGE_FORMAT_A8 },
	{ 24,  3,  8,  8,  8,  8,	 0,	 1,	 2,	-1, false,  true, ToBlueScreen, FromBlueScreen,	IMAGE_FORMAT_RGB888_BLUESCREEN },
	{ 24,  3,  8,  8,  8,  8,	 2,	 1,	 0,	-1, false,  true, ToBlueScreen, FromBlueScreen,	IMAGE_FORMAT_BGR888_BLUESCREEN },
	{ 32,  4,  8,  8,  8,  8,	 3,	 0,	 1,	 2, false,  true, nullptr, nullptr,				IMAGE_FORMAT_ARGB8888 },
	{ 32,  4,  8,  8,  8,  8,	 2,	 1,	 0,	 3, false,  true, nullptr, nullptr,				IMAGE_FORMAT_BGRA8888 },
	{ 4,  0,  0,  0,  0,  0,	-1,	-1,	-1,	-1,	 true,  true, nullptr, nullptr,				IMAGE_FORMAT_DXT1 },
	{ 8,  0,  0,  0,  0,  8,	-1,	-1,	-1,	-1,	 true,  true, nullptr, nullptr,				IMAGE_FORMAT_DXT3 },
	{ 8,  0,  0,  0,  0,  8,	-1,	-1,	-1,	-1,	 true,  true, nullptr, nullptr,				IMAGE_FORMAT_DXT5 },
	{ 32,  4,  8,  8,  8,  0,	 2,	 1,	 0,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_BGRX8888 },
	{ 16,  2,  5,  6,  5,  0,	 2,	 1,	 0,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_BGR565 },
	{ 16,  2,  5,  5,  5,  0,	 2,	 1,	 0,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_BGRX5551 },
	{ 16,  2,  4,  4,  4,  4,	 2,	 1,	 0,	 3, false,  true, nullptr, nullptr,				IMAGE_FORMAT_BGRA4444 },
	{ 4,  0,  0,  0,  0,  1,	-1,	-1,	-1,	-1,	 true,  true, nullptr, nullptr,				IMAGE_FORMAT_DXT1_ONEBITALPHA },
	{ 16,  2,  5,  5,  5,  1,	 2,	 1,	 0,	 3, false,  true, nullptr, nullptr,				IMAGE_FORMAT_BGRA5551 },
	{ 16,  2,  8,  8,  0,  0,	 0,	 1,	-1,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_UV88 },
	{ 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	 3, false,  true, nullptr, nullptr,				IMAGE_FORMAT_UVWQ8888 },
	{ 64,  8, 16, 16, 16, 16,	 0,	 1,	 2,	 3, false,  true, nullptr, FromFP16,			IMAGE_FORMAT_RGBA16161616F },
	{ 64,  8, 16, 16, 16, 16,	 0,	 1,	 2,	 3, false,  true, nullptr, nullptr,				IMAGE_FORMAT_RGBA16161616 },
	{ 32,  4,  8,  8,  8,  8,	 0,	 1,	 2,	 3, false,  true, nullptr, nullptr,				IMAGE_FORMAT_UVLX8888 },
	{ 32,  4, 32,  0,  0,  0,	 0,	-1,	-1,	-1, false, false, nullptr, nullptr,				IMAGE_FORMAT_R32F },
	{ 96, 12, 32, 32, 32,  0,	 0,	 1,	 2,	-1, false, false, nullptr, nullptr,				IMAGE_FORMAT_RGB323232F },
	{ 128, 16, 32, 32, 32, 32,	 0,	 1,	 2,	 3, false, false, nullptr, nullptr,				IMAGE_FORMAT_RGBA32323232F },
	{ 16,  2, 16,  0,  0,  0,	 0,	-1,	-1,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_NV_DST16 },
	{ 24,  3, 24,  0,  0,  0,	 0,	-1,	-1,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_NV_DST24 },
	{ 32,  4,  0,  0,  0,  0,	-1,	-1,	-1,	-1, false, false, nullptr, nullptr,				IMAGE_FORMAT_NV_INTZ },
	{ 24,  3,  0,  0,  0,  0,	-1,	-1,	-1,	-1, false, false, nullptr, nullptr,				IMAGE_FORMAT_NV_RAWZ },
	{ 16,  2, 16,  0,  0,  0,	 0,	-1,	-1,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_ATI_DST16 },
	{ 24,  3, 24,  0,  0,  0,	 0,	-1,	-1,	-1, false,  true, nullptr, nullptr,				IMAGE_FORMAT_ATI_DST24 },
	{ 32,  4,  0,  0,  0,  0,	-1,	-1,	-1,	-1, false, false, nullptr, nullptr,				IMAGE_FORMAT_NV_NULL },
	{ 4,  0,  0,  0,  0,  0,	-1, -1, -1, -1,	 true, false, nullptr, nullptr,				IMAGE_FORMAT_ATI1N },
	{ 8,  0,  0,  0,  0,  0,	-1, -1, -1, -1,	 true, false, nullptr, nullptr,				IMAGE_FORMAT_ATI2N }
};

template <typename T>
void GetShiftAndMask( const SVTFImageConvertInfo& Info, T& uiRShift, T& uiGShift, T& uiBShift, T& uiAShift, T& uiRMask, T& uiGMask, T& uiBMask, T& uiAMask )
{
	if ( Info.iR >= 0 )
	{
		if ( Info.iG >= 0 && Info.iG < Info.iR )
			uiRShift += ( T )Info.uiGBitsPerPixel;

		if ( Info.iB >= 0 && Info.iB < Info.iR )
			uiRShift += ( T )Info.uiBBitsPerPixel;

		if ( Info.iA >= 0 && Info.iA < Info.iR )
			uiRShift += ( T )Info.uiABitsPerPixel;

		uiRMask = ( T )( ~0 ) >> ( T )( ( sizeof( T ) * 8 ) - Info.uiRBitsPerPixel ); // Mask is for down shifted values.
	}

	if ( Info.iG >= 0 )
	{
		if ( Info.iR >= 0 && Info.iR < Info.iG )
			uiGShift += ( T )Info.uiRBitsPerPixel;

		if ( Info.iB >= 0 && Info.iB < Info.iG )
			uiGShift += ( T )Info.uiBBitsPerPixel;

		if ( Info.iA >= 0 && Info.iA < Info.iG )
			uiGShift += ( T )Info.uiABitsPerPixel;

		uiGMask = ( T )( ~0 ) >> ( T )( ( sizeof( T ) * 8 ) - Info.uiGBitsPerPixel );
	}

	if ( Info.iB >= 0 )
	{
		if ( Info.iR >= 0 && Info.iR < Info.iB )
			uiBShift += ( T )Info.uiRBitsPerPixel;

		if ( Info.iG >= 0 && Info.iG < Info.iB )
			uiBShift += ( T )Info.uiGBitsPerPixel;

		if ( Info.iA >= 0 && Info.iA < Info.iB )
			uiBShift += ( T )Info.uiABitsPerPixel;

		uiBMask = ( T )( ~0 ) >> ( T )( ( sizeof( T ) * 8 ) - Info.uiBBitsPerPixel );
	}

	if ( Info.iA >= 0 )
	{
		if ( Info.iR >= 0 && Info.iR < Info.iA )
			uiAShift += ( T )Info.uiRBitsPerPixel;

		if ( Info.iG >= 0 && Info.iG < Info.iA )
			uiAShift += ( T )Info.uiGBitsPerPixel;

		if ( Info.iB >= 0 && Info.iB < Info.iA )
			uiAShift += ( T )Info.uiBBitsPerPixel;

		uiAMask = ( T )( ~0 ) >> ( T )( ( sizeof( T ) * 8 ) - Info.uiABitsPerPixel );
	}
}

template <typename T>
T Shrink( const T& S, const T& SourceBits, const T& DestBits )
{
	if ( SourceBits == 0 || DestBits == 0 )
		return 0;

	return S >> ( SourceBits - DestBits );
}

template <typename T>
T Expand( T S, T SourceBits, T DestBits )
{
	if ( SourceBits == 0 || DestBits == 0 )
		return 0;

	T D = 0;

	while ( DestBits >= SourceBits )
	{
		D <<= SourceBits;
		D |= S;
		DestBits -= SourceBits;
	}

	if ( DestBits )
	{
		S >>= SourceBits - DestBits;
		D <<= DestBits;
		D |= S;
	}

	return D;
}

template <typename T, typename U>
void Transform( TransformProc pTransform1, TransformProc pTransform2, T SR, T SG, T SB, T SA, T SRBits, T SGBits, T SBBits, T SABits, U& DR, U& DG, U& DB, U& DA, U DRBits, U DGBits, U DBBits, U DABits )
{
	uint16 TR, TG, TB, TA;

	// Expand from source to 16 bits for transform functions.
	TR = SRBits && SRBits < 16 ? (uint16)Expand<T>( SR, SRBits, 16 ) : (uint16)SR;
	TG = SGBits && SGBits < 16 ? (uint16)Expand<T>( SG, SGBits, 16 ) : (uint16)SG;
	TB = SBBits && SBBits < 16 ? (uint16)Expand<T>( SB, SBBits, 16 ) : (uint16)SB;
	TA = SABits && SABits < 16 ? (uint16)Expand<T>( SA, SABits, 16 ) : (uint16)SA;

	// Source transform then dest transform.
	if ( pTransform1 )
		pTransform1( TR, TG, TB, TA );
	if ( pTransform2 )
		pTransform2( TR, TG, TB, TA );

	// Shrink to dest from 16 bits.
	DR = DRBits && DRBits < 16 ? (U)Shrink<uint16>( TR, 16, (uint16)DRBits ) : (U)TR;
	DG = DGBits && DGBits < 16 ? (U)Shrink<uint16>( TG, 16, (uint16)DGBits ) : (U)TG;
	DB = DBBits && DBBits < 16 ? (U)Shrink<uint16>( TB, 16, (uint16)DBBits ) : (U)TB;
	DA = DABits && DABits < 16 ? (U)Shrink<uint16>( TA, 16, (uint16)DABits ) : (U)TA;
}

template<typename T, typename U>
bool ConvertTemplated( const unsigned char* lpSource, unsigned char* lpDest, unsigned int uiWidth, unsigned int uiHeight, const SVTFImageConvertInfo& SourceInfo, const SVTFImageConvertInfo& DestInfo )
{
	unsigned short uiSourceRShift = 0, uiSourceGShift = 0, uiSourceBShift = 0, uiSourceAShift = 0;
	unsigned short uiSourceRMask = 0, uiSourceGMask = 0, uiSourceBMask = 0, uiSourceAMask = 0;

	unsigned short uiDestRShift = 0, uiDestGShift = 0, uiDestBShift = 0, uiDestAShift = 0;
	unsigned short uiDestRMask = 0, uiDestGMask = 0, uiDestBMask = 0, uiDestAMask = 0;

	GetShiftAndMask<unsigned short>( SourceInfo, uiSourceRShift, uiSourceGShift, uiSourceBShift, uiSourceAShift, uiSourceRMask, uiSourceGMask, uiSourceBMask, uiSourceAMask );
	GetShiftAndMask<unsigned short>( DestInfo, uiDestRShift, uiDestGShift, uiDestBShift, uiDestAShift, uiDestRMask, uiDestGMask, uiDestBMask, uiDestAMask );

	if ( SourceInfo.Format == IMAGE_FORMAT_RGBA16161616F )
	{
		const unsigned char* lpFPSource = lpSource;

		sHDRLogAverageLuminance = 0.0f;

		const unsigned char* lpFPSourceEnd = lpFPSource + ( uiWidth * uiHeight * SourceInfo.uiBytesPerPixel );
		for ( ; lpFPSource < lpFPSourceEnd; lpFPSource += SourceInfo.uiBytesPerPixel )
		{
			const unsigned short* p = ( const unsigned short* )lpFPSource;

			float sLuminance = ( float )p[0] * 0.299f + ( float )p[1] * 0.587f + ( float )p[2] * 0.114f;

			sHDRLogAverageLuminance += log( 0.0000000001f + sLuminance );
		}

		sHDRLogAverageLuminance = exp( sHDRLogAverageLuminance / ( float )( uiWidth * uiHeight ) );
	}

	const unsigned char* lpSourceEnd = lpSource + ( uiWidth * uiHeight * SourceInfo.uiBytesPerPixel );
	for ( ; lpSource < lpSourceEnd; lpSource += SourceInfo.uiBytesPerPixel, lpDest += DestInfo.uiBytesPerPixel )
	{
		unsigned int i;
		T Source = 0;
		for ( i = 0; i < SourceInfo.uiBytesPerPixel; i++ )
			Source |= ( T )lpSource[i] << ( ( T )i * 8 );

		unsigned short SR = 0, SG = 0, SB = 0, SA = (unsigned short)~0;
		unsigned short DR = 0, DG = 0, DB = 0, DA = (unsigned short)~0;

		if ( uiSourceRMask )
			SR = ( unsigned short )( Source >> ( T )uiSourceRShift ) & uiSourceRMask;

		if ( uiSourceGMask )
			SG = ( unsigned short )( Source >> ( T )uiSourceGShift ) & uiSourceGMask;

		if ( uiSourceBMask )
			SB = ( unsigned short )( Source >> ( T )uiSourceBShift ) & uiSourceBMask;

		if ( uiSourceAMask )
			SA = ( unsigned short )( Source >> ( T )uiSourceAShift ) & uiSourceAMask;

		if ( SourceInfo.pFromTransform || DestInfo.pToTransform )
		{
			// transform values
			Transform<uint16, uint16>(SourceInfo.pFromTransform, DestInfo.pToTransform, SR, SG, SB, SA, SourceInfo.uiRBitsPerPixel, SourceInfo.uiGBitsPerPixel, SourceInfo.uiBBitsPerPixel, SourceInfo.uiABitsPerPixel, DR, DG, DB, DA, DestInfo.uiRBitsPerPixel, DestInfo.uiGBitsPerPixel, DestInfo.uiBBitsPerPixel, DestInfo.uiABitsPerPixel);
		}
		else
		{
			if ( uiSourceRMask && uiDestRMask )
			{
				if ( DestInfo.uiRBitsPerPixel < SourceInfo.uiRBitsPerPixel )
					DR = Shrink<unsigned short>( SR, SourceInfo.uiRBitsPerPixel, DestInfo.uiRBitsPerPixel );
				else if ( DestInfo.uiRBitsPerPixel > SourceInfo.uiRBitsPerPixel )
					DR = Expand<unsigned short>( SR, SourceInfo.uiRBitsPerPixel, DestInfo.uiRBitsPerPixel );
				else
					DR = SR;
			}

			if ( uiSourceGMask && uiDestGMask )
			{
				if ( DestInfo.uiGBitsPerPixel < SourceInfo.uiGBitsPerPixel )
					DG = Shrink<unsigned short>( SG, SourceInfo.uiGBitsPerPixel, DestInfo.uiGBitsPerPixel );
				else if ( DestInfo.uiGBitsPerPixel > SourceInfo.uiGBitsPerPixel )
					DG = Expand<unsigned short>( SG, SourceInfo.uiGBitsPerPixel, DestInfo.uiGBitsPerPixel );
				else
					DG = SG;
			}

			if ( uiSourceBMask && uiDestBMask )
			{
				if ( DestInfo.uiBBitsPerPixel < SourceInfo.uiBBitsPerPixel )
					DB = Shrink<unsigned short>( SB, SourceInfo.uiBBitsPerPixel, DestInfo.uiBBitsPerPixel );
				else if ( DestInfo.uiBBitsPerPixel > SourceInfo.uiBBitsPerPixel )
					DB = Expand<unsigned short>( SB, SourceInfo.uiBBitsPerPixel, DestInfo.uiBBitsPerPixel );
				else
					DB = SB;
			}

			if ( uiSourceAMask && uiDestAMask )
			{
				if ( DestInfo.uiABitsPerPixel < SourceInfo.uiABitsPerPixel )
					DA = Shrink<unsigned short>( SA, SourceInfo.uiABitsPerPixel, DestInfo.uiABitsPerPixel );
				else if ( DestInfo.uiABitsPerPixel > SourceInfo.uiABitsPerPixel )
					DA = Expand<unsigned short>( SA, SourceInfo.uiABitsPerPixel, DestInfo.uiABitsPerPixel );
				else
					DA = SA;
			}
		}

		U Dest = ( ( U )( DR & uiDestRMask ) << ( U )uiDestRShift ) | ( ( U )( DG & uiDestGMask ) << ( U )uiDestGShift ) | ( ( U )( DB & uiDestBMask ) << ( U )uiDestBShift ) | ( ( U )( DA & uiDestAMask ) << ( U )uiDestAShift );
		for ( i = 0; i < DestInfo.uiBytesPerPixel; i++ )
		{
			lpDest[i] = ( unsigned char )( ( Dest >> ( ( T )i * 8 ) ) & 0xff );
		}
	}

	return true;
}

bool CVTFFile::Convert( const unsigned char* lpSource, unsigned char* lpDest, unsigned int uiWidth, unsigned int uiHeight, ImageFormat SourceFormat, ImageFormat DestFormat )
{
	const SVTFImageConvertInfo& SourceInfo = VTFImageConvertInfo[SourceFormat];
	const SVTFImageConvertInfo& DestInfo = VTFImageConvertInfo[DestFormat];

	if ( !SourceInfo.bIsSupported || !DestInfo.bIsSupported )
	{
		return false;
	}

	if ( SourceFormat == DestFormat )
	{
		memcpy( lpDest, lpSource, CVTFFile::ComputeImageSize( uiWidth, uiHeight, 1, DestFormat ) );
		return true;
	}

	if ( SourceFormat == IMAGE_FORMAT_RGB888 && DestFormat == IMAGE_FORMAT_RGBA8888 )
	{
		const unsigned char* lpLast = lpSource + CVTFFile::ComputeImageSize( uiWidth, uiHeight, 1, SourceFormat );
		for ( ; lpSource < lpLast; lpSource += 3, lpDest += 4 )
		{
			lpDest[0] = lpSource[0];
			lpDest[1] = lpSource[1];
			lpDest[2] = lpSource[2];
			lpDest[3] = 255;
		}
		return true;
	}

	if ( SourceFormat == IMAGE_FORMAT_RGBA8888 && DestFormat == IMAGE_FORMAT_RGB888 )
	{
		const unsigned char *lpLast = lpSource + CVTFFile::ComputeImageSize( uiWidth, uiHeight, 1, SourceFormat );
		for ( ; lpSource < lpLast; lpSource += 4, lpDest += 3 )
		{
			lpDest[0] = lpSource[0];
			lpDest[1] = lpSource[1];
			lpDest[2] = lpSource[2];
		}
		return true;
	}

	if ( SourceInfo.bIsCompressed || DestInfo.bIsCompressed )
	{
		unsigned char* lpSourceRGBA = nullptr;
		bool bResult = true;

		if ( SourceFormat != IMAGE_FORMAT_RGBA8888 )
			lpSourceRGBA = new unsigned char[CVTFFile::ComputeImageSize( uiWidth, uiHeight, 1, IMAGE_FORMAT_RGBA8888 )];

		switch ( SourceFormat )
		{
		case IMAGE_FORMAT_RGBA8888:
			lpSourceRGBA = const_cast<unsigned char*>( lpSource );
			break;
		case IMAGE_FORMAT_DXT1:
		case IMAGE_FORMAT_DXT1_ONEBITALPHA:
			bResult = CVTFFile::DecompressDXT1( lpSource, lpSourceRGBA, uiWidth, uiHeight );
			break;
		case IMAGE_FORMAT_DXT3:
			bResult = CVTFFile::DecompressDXT3( lpSource, lpSourceRGBA, uiWidth, uiHeight );
			break;
		case IMAGE_FORMAT_DXT5:
			bResult = CVTFFile::DecompressDXT5( lpSource, lpSourceRGBA, uiWidth, uiHeight );
			break;
		default:
			bResult = CVTFFile::Convert( lpSource, lpSourceRGBA, uiWidth, uiHeight, SourceFormat, IMAGE_FORMAT_RGBA8888 );
			break;
		}

		if ( bResult )
		{
			switch ( DestFormat )
			{
			case IMAGE_FORMAT_DXT1:
			case IMAGE_FORMAT_DXT1_ONEBITALPHA:
			case IMAGE_FORMAT_DXT3:
			case IMAGE_FORMAT_DXT5:
				break;
			default:
				bResult = CVTFFile::Convert( lpSourceRGBA, lpDest, uiWidth, uiHeight, IMAGE_FORMAT_RGBA8888, DestFormat );
				break;
			}
		}

		if ( lpSourceRGBA != lpSource )
			delete[] lpSourceRGBA;

		return bResult;
	}
	else
	{
		if ( SourceInfo.uiBytesPerPixel <= 1 )
		{
			if ( DestInfo.uiBytesPerPixel <= 1 )
				return ConvertTemplated<unsigned char, unsigned char>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 2 )
				return ConvertTemplated<unsigned char, unsigned short>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 4 )
				return ConvertTemplated<unsigned char, unsigned int>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 8 )
				return ConvertTemplated<unsigned char, unsigned long long>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
		}
		else if ( SourceInfo.uiBytesPerPixel <= 2 )
		{
			if ( DestInfo.uiBytesPerPixel <= 1 )
				return ConvertTemplated<unsigned short, unsigned char>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 2 )
				return ConvertTemplated<unsigned short, unsigned short>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 4 )
				return ConvertTemplated<unsigned short, unsigned int>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 8 )
				return ConvertTemplated<unsigned short, unsigned long long>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
		}
		else if ( SourceInfo.uiBytesPerPixel <= 4 )
		{
			if ( DestInfo.uiBytesPerPixel <= 1 )
				return ConvertTemplated<unsigned int, unsigned char>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 2 )
				return ConvertTemplated<unsigned int, unsigned short>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 4 )
				return ConvertTemplated<unsigned int, unsigned int>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 8 )
				return ConvertTemplated<unsigned int, unsigned long long>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
		}
		else if ( SourceInfo.uiBytesPerPixel <= 8 )
		{
			if ( DestInfo.uiBytesPerPixel <= 1 )
				return ConvertTemplated<unsigned long long, unsigned char>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 2 )
				return ConvertTemplated<unsigned long long, unsigned short>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 4 )
				return ConvertTemplated<unsigned long long, unsigned int>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
			else if ( DestInfo.uiBytesPerPixel <= 8 )
				return ConvertTemplated<unsigned long long, unsigned long long>( lpSource, lpDest, uiWidth, uiHeight, SourceInfo, DestInfo );
		}
		return false;
	}
}