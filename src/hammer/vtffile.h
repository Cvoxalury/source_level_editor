#pragma once

#include "bitmap/imageformat.h"

typedef void* FileHandle_t;

enum VTFResourceEntryTypeFlag
{
	RSRCF__HAS_NO_DATA_CHUNK = 0x02
};

#define MAKE_VTF__RSRC_ID(a, b, c) ((unsigned int)(((unsigned char)a) | ((unsigned char)b << 8) | ((unsigned char)c << 16)))
#define MAKE_VTF__RSRC_IDF(a, b, c, d) ((unsigned int)(((unsigned char)a) | ((unsigned char)b << 8) | ((unsigned char)c << 16) | ((unsigned char)d << 24)))
enum VTFResourceEntryType
{
	VTF__LEGACY_RSRC_LOW_RES_IMAGE = MAKE_VTF__RSRC_ID( 0x01, 0, 0 ),
	VTF__LEGACY_RSRC_IMAGE = MAKE_VTF__RSRC_ID( 0x30, 0, 0 ),
	VTF__RSRC_SHEET = MAKE_VTF__RSRC_ID( 0x10, 0, 0 ),
	VTF__RSRC_CRC = MAKE_VTF__RSRC_IDF( 'C', 'R', 'C', RSRCF__HAS_NO_DATA_CHUNK ),
	VTF__RSRC_TEXTURE_LOD_SETTINGS = MAKE_VTF__RSRC_IDF( 'L', 'O', 'D', RSRCF__HAS_NO_DATA_CHUNK ),
	VTF__RSRC_TEXTURE_SETTINGS_EX = MAKE_VTF__RSRC_IDF( 'T', 'S', 'O', RSRCF__HAS_NO_DATA_CHUNK ),
	VTF__RSRC_KEY_VALUE_DATA = MAKE_VTF__RSRC_ID( 'K', 'V', 'D' ),
	VTF__RSRC_MAX_DICTIONARY_ENTRIES = 32
};

#pragma pack(1)
#pragma warning(push)
#pragma warning(disable: 4201)
struct SVTFResource
{
	union
	{
		unsigned int Type;
		struct
		{
			unsigned char ID[3];	//!< Unique resource ID
			unsigned char Flags;	//!< Resource flags
		};
	};
	unsigned int Data;	//!< Resource data (e.g. for a  CRC) or offset from start of the file
};
#pragma warning(pop)

struct SVTFResourceData
{
	unsigned int Size;	//!< Resource data buffer size
	unsigned char* Data;	//!< Resource data bufffer
};

struct SVTFFileHeader
{
	char					TypeString[4];					//!< "Magic number" identifier- "VTF\0".
	unsigned int			Version[2];						//!< Version[0].version[1] (currently 7.2)
	unsigned int			HeaderSize;						//!< Size of the header struct (currently 80 bytes)
};

struct SVTFHeader_70 : public SVTFFileHeader
{
	unsigned short			Width;							//!< Width of the largest image
	unsigned short			Height;							//!< Height of the largest image
	unsigned int			Flags;							//!< Flags for the image
	unsigned short			Frames;							//!< Number of frames if animated (1 for no animation)
	unsigned short			StartFrame;						//!< Start frame (always 0)
	unsigned char			Padding0[4];					//!< Reflectivity padding (16 byte alignment)
	float					Reflectivity[3];				//!< Reflectivity vector
	unsigned char			Padding1[4];					//!< Reflectivity padding (8 byte packing)
	float					BumpScale;						//!< Bump map scale
	ImageFormat				Format;							//!< Image format index
	unsigned char			MipCount;						//!< Number of MIP levels (including the largest image)
	ImageFormat				LowResImageFormat;				//!< Image format of the thumbnail image
	unsigned char			LowResImageWidth;				//!< Thumbnail image width
	unsigned char			LowResImageHeight;				//!< Thumbnail image height
};

struct SVTFHeader_71 : public SVTFHeader_70
{
};

struct SVTFHeader_72 : public SVTFHeader_71
{
	unsigned short		Depth;							//!< Depth of the largest image
};

struct SVTFHeader_73 : public SVTFHeader_72
{
	unsigned char		Padding2[3];
	unsigned int		ResourceCount;							//!< Number of image resources
};

struct SVTFHeader_74 : public SVTFHeader_73
{
};

struct SVTFHeader_74_A : public SVTFHeader_74
{
};

struct SVTFHeader : public SVTFHeader_74_A
{
	unsigned char			Padding3[8];
	SVTFResource		Resources[VTF__RSRC_MAX_DICTIONARY_ENTRIES];
	SVTFResourceData	Data[VTF__RSRC_MAX_DICTIONARY_ENTRIES];
};

struct SImageFormatInfo
{
	const char*		lpName;					//!< Enumeration text equivalent.
	unsigned int	uiBitsPerPixel;			//!< Format bits per pixel.
	unsigned int	uiBytesPerPixel;		//!< Format bytes per pixel.
	unsigned int	uiRedBitsPerPixel;		//!< Format red bits per pixel.  0 for N/A.
	unsigned int	uiGreenBitsPerPixel;	//!< Format green bits per pixel.  0 for N/A.
	unsigned int	uiBlueBitsPerPixel;		//!< Format blue bits per pixel.  0 for N/A.
	unsigned int	uiAlphaBitsPerPixel;	//!< Format alpha bits per pixel.  0 for N/A.
	bool			bIsCompressed;			//!< Format is compressed (DXT).
	bool			bIsSupported;			//!< Format is supported by VTFLib.
};
#pragma pack()

class CVTFFile final
{
private:
	SVTFHeader* Header;

	unsigned int uiImageBufferSize;
	unsigned char* lpImageData;

	unsigned int uiThumbnailBufferSize;
	unsigned char* lpThumbnailImageData;

public:
	CVTFFile();
	~CVTFFile();

	void Destroy();
	bool IsLoaded() const;

	bool Load( FileHandle_t Reader, bool bHeaderOnly );
private:
	static bool IsPowerOfTwo( unsigned int uiSize );
	static unsigned int NextPowerOfTwo( unsigned int uiSize );

public:
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	unsigned int GetDepth() const;
	unsigned int GetMipmapCount() const;
	unsigned int GetFaceCount() const;
	unsigned int GetFrameCount() const;
	unsigned int GetFlags() const;
	ImageFormat GetFormat() const;

	unsigned char* GetData( unsigned int uiFrame = 0, unsigned int uiFace = 0, unsigned int uiSlice = 0, unsigned int uiMipmapLevel = 0 ) const;

public:
	static const SImageFormatInfo& GetImageFormatInfo( ImageFormat ImageFormat );

	static unsigned int ComputeImageSize( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth, ImageFormat ImageFormat );
	static unsigned int ComputeImageSize( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth, unsigned int uiMipmaps, ImageFormat ImageFormat );

	static unsigned int ComputeMipmapCount( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth );
	static void ComputeMipmapDimensions( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth, unsigned int uiMipmapLevel, unsigned int& uiMipmapWidth, unsigned int& uiMipmapHeight, unsigned int& uiMipmapDepth );
	static unsigned int ComputeMipmapSize( unsigned int uiWidth, unsigned int uiHeight, unsigned int uiDepth, unsigned int uiMipmapLevel, ImageFormat ImageFormat );

private:
	unsigned int ComputeDataOffset( unsigned int uiFrame, unsigned int uiFace, unsigned int uiSlice, unsigned int uiMipmapLevel, ImageFormat ImageFormat ) const;

public:
	static bool Convert( const unsigned char* lpSource, unsigned char* lpDest, unsigned int uiWidth, unsigned int uiHeight, ImageFormat SourceFormat, ImageFormat DestFormat );

private:
	static bool DecompressDXT1( const unsigned char* src, unsigned char* dst, unsigned int uiWidth, unsigned int uiHeight );
	static bool DecompressDXT3( const unsigned char* src, unsigned char* dst, unsigned int uiWidth, unsigned int uiHeight );
	static bool DecompressDXT5( const unsigned char* src, unsigned char* dst, unsigned int uiWidth, unsigned int uiHeight );
};