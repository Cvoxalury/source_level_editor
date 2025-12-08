//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef RENDER_H
#define RENDER_H
#pragma once

#include "Color.h"
#include "utlstack.h"
#include "hammer_mathlib.h"
#include "MaterialSystem\imesh.h"
#ifdef SLE //// for colours, etc
#include "Options.h"
#endif
class IMaterial;
struct DrawModelInfo_t;
class CCoreDispInfo;
class CMapView;
class CCamera;
class CMapAtom;
class IEditorTexture;
class CMapClass;
class CMapInstance;

typedef unsigned short MDLHandle_t;

#define CAMERA_FRONT_PLANE_DISTANCE		8.0f
#define CAMERA_HORIZONTAL_FOV			90.0f

//
// Colors for selected faces and edges. Kinda hacky; should probably be elsewhere.
//
#ifdef SLE //// SLE NEW: Custom colour scheme support.
#define SELECT_FACE_RED			GetRValue(Options.colors.clr3DSelection)
#define SELECT_FACE_GREEN		GetGValue(Options.colors.clr3DSelection)
#define SELECT_FACE_BLUE		GetBValue(Options.colors.clr3DSelection)

#define SELECT_EDGE_RED			GetRValue(Options.colors.clr3DEdges)
#define SELECT_EDGE_GREEN		GetGValue(Options.colors.clr3DEdges)
#define SELECT_EDGE_BLUE		GetBValue(Options.colors.clr3DEdges)
#else
#define SELECT_FACE_RED			220
#define SELECT_FACE_GREEN		0
#define SELECT_FACE_BLUE		0

#define SELECT_EDGE_RED			255
#define SELECT_EDGE_GREEN		255
#define SELECT_EDGE_BLUE		0
#endif
inline void SelectFaceColor( Color &pColor )
{
	pColor[0] = SELECT_FACE_RED;
	pColor[1] = SELECT_FACE_GREEN;
	pColor[2] = SELECT_FACE_BLUE;
}

inline void SelectEdgeColor( Color &pColor )
{
	pColor[0] = SELECT_EDGE_RED;
	pColor[1] = SELECT_EDGE_GREEN;
	pColor[2] = SELECT_EDGE_BLUE;
}

inline void InstanceColor( Color &pColor, bool bSelected )
{
#ifdef SLE //// SLE NEW: Custom colour scheme support.
	if ( bSelected )
	{
		pColor[ 0 ] = GetRValue(Options.colors.clrInstancesSel); //192;
		pColor[ 1 ] = GetGValue(Options.colors.clrInstancesSel); //128;
		pColor[ 2 ] = GetBValue(Options.colors.clrInstancesSel); //0;
		pColor[ 3 ] = 128; //192;
	}
	else
	{
		pColor[ 0 ] = GetRValue(Options.colors.clrInstances); //128;
		pColor[ 1 ] = GetGValue(Options.colors.clrInstances); //128;
		pColor[ 2 ] = GetBValue(Options.colors.clrInstances); //0;
		pColor[ 3 ] = 128; //192;
	}
#else
	if ( bSelected )
	{
		pColor[ 0 ] = 192;
		pColor[ 1 ] = 128;
		pColor[ 2 ] = 0;
		pColor[ 3 ] = 192;
	}
	else
	{
		pColor[ 0 ] = 128;
		pColor[ 1 ] = 128;
		pColor[ 2 ] = 0;
		pColor[ 3 ] = 192;
	}
#endif
}

enum EditorRenderMode_t
{
	RENDER_MODE_NONE = 0,				// dont render anything
	RENDER_MODE_EXTERN,					// other system is using material system
	RENDER_MODE_DEFAULT,				// select default material
	RENDER_MODE_CURRENT,				// the current render mode
	RENDER_MODE_WIREFRAME,				// wire frame mode
#ifdef SLE
	RENDER_MODE_WIREFRAME_NOZ,			// wire frame mode, ignore Z
#endif
	RENDER_MODE_FLAT,					// flat solid colors
	RENDER_MODE_FLAT_NOZ,				// flat solid colors, ignore Z
	RENDER_MODE_FLAT_NOCULL,			// flat solid colors, no backface culling
	RENDER_MODE_DOTTED,					// flat colored dotted, ignore Z
	RENDER_MODE_TRANSLUCENT_FLAT,
	RENDER_MODE_TEXTURED,
	RENDER_MODE_LIGHTMAP_GRID,
	RENDER_MODE_SELECTION_OVERLAY,
	RENDER_MODE_SMOOTHING_GROUP,
	RENDER_MODE_TEXTURED_SHADED,
	RENDER_MODE_LIGHT_PREVIEW2,
	RENDER_MODE_LIGHT_PREVIEW_RAYTRACED,
	RENDER_MODE_INSTANCE_OVERLAY,
#ifdef SLE //// SLE NEW - some extra overlays
	RENDER_MODE_CLIPPING_OVERLAY,
	RENDER_MODE_HIDDEN_WIRE,
#endif
};

enum InstanceRenderingState_t
{
	INSTANCE_STATE_OFF,				// normal rendering
	INSTANCE_STATE_ON,				// will be tinted as an instance
	INSTANCE_STACE_SELECTED			// will be tinted as a selected instance
};

typedef struct SInstanceState
{
	CMapInstance	*m_pInstanceClass;		// the func_instance entity
	Vector			m_InstanceOrigin;		// the origin offset of instance rendering
	QAngle			m_InstanceAngles;		// the rotation of the instance rendering
	VMatrix			m_InstanceMatrix;		// matrix of the origin and rotation of rendering
	VMatrix			m_InstanceRenderMatrix;	// matrix of the current camera transform
	bool			m_bIsEditable;
	CMapInstance	*m_pTopInstanceClass;
} TInstanceState;

#define STENCIL_AS_CALLS	1

class CRender
{
public:
	CRender(void);
	virtual ~CRender(void);

	enum
	{	TEXT_SINGLELINE           =   0x1,             // put all of the text on one line
		TEXT_MULTILINE            =   0x2,             // the text is written on multiple lines
		TEXT_JUSTIFY_BOTTOM       =   0x4,             // default
		TEXT_JUSTIFY_TOP          =   0x8,				
		TEXT_JUSTIFY_RIGHT        =  0x10,			  // default
		TEXT_JUSTIFY_LEFT         =  0x20,
		TEXT_JUSTIFY_HORZ_CENTER  =  0x40,
		TEXT_JUSTIFY_VERT_CENTER  =  0x80,
		TEXT_CLEAR_BACKGROUND     = 0x100
	};           // clear the background behind the text

	enum
	{	HANDLE_NONE			= 0,
		HANDLE_SQUARE,
		HANDLE_CIRCLE,
		HANDLE_DIAMOND,
		HANDLE_CROSS
	};

	// map view setup
	virtual bool SetView( CMapView *pView );

	CMapView	*GetView() { return m_pView; }
	CCamera		*GetCamera();// { return m_pView->GetCamera(); }
	bool		IsActiveView();

	// begin/end single render frame, sets up camera etc
	virtual void StartRenderFrame();
    virtual void EndRenderFrame();
	int  GetRenderFrame() { return m_nFrameCount; }

	// switch rendering to client space coordinates (horz,vert,ignore)
	// render is in world space mode by default
	bool BeginClientSpace(void);
	void EndClientSpace(void);
	bool IsInClientSpace() { return m_bIsClientSpace; }

	void BeginLocalTransfrom( const VMatrix &matrix, bool MultiplyCurrent = false );
	void EndLocalTransfrom();
	bool IsInLocalTransformMode();
	void GetLocalTranform( VMatrix &matrix );

	void SetTextColor( unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255 );
	void SetDrawColor( unsigned char r, unsigned char g, unsigned char b );
	void SetDrawColor( const Color &color );
	void GetDrawColor( Color &color );
	void SetHandleColor( unsigned char r, unsigned char g, unsigned char b );
	
	void SetHandleStyle( int size, int type );

	void SetDefaultRenderMode(EditorRenderMode_t eRenderMode);
#ifdef HAMMER2013_PORT_PROXIES
	void BindTexture(IEditorTexture *pTexture, CMapClass* proxyData = nullptr);
	void BindMaterial(IMaterial *pMaterial, CMapClass* proxyData = nullptr);
#else
	void BindTexture(IEditorTexture *pTexture);
	void BindMaterial( IMaterial *pMaterial );
#endif
	virtual void SetRenderMode( EditorRenderMode_t eRenderMode, bool force = false);
	inline	EditorRenderMode_t GetCurrentRenderMode() { return m_eCurrentRenderMode; }
	inline	EditorRenderMode_t GetDefaultRenderMode() { return m_eDefaultRenderMode; }

	void PushRenderMode( EditorRenderMode_t eRenderMode );
	void PopRenderMode();

	// drawing primitives
			void DrawPoint( const Vector &vPoint );			
			void DrawLine( const Vector &vStart, const Vector &vEnd );
	virtual void DrawBox( const Vector &vMins, const Vector &vMaxs, bool bFill = false );
			void DrawBoxExt( const Vector &vCenter, float extend, bool bFill = false );
			void DrawSphere( const Vector &vCenter, int nRadius );
			void DrawCircle( const Vector &vCenter, const Vector &vNormal, float flRadius, int nSegments );
			void DrawPolyLine( int nPoints, const Vector *Points );
#ifdef SLE  //// SLE NEW - for when we want filled 2d outlines
			void DrawPolyLineFilled(int nPoints, const Vector *Points, float opacity = 1.0f);
#endif
			void DrawText( const char *text, int x, int y, int nFlags );	// Uses pixel coordinates
			void DrawText( const char *text, const Vector2D &vPos, int nOffsetX, int nOffsetY, int nFlags );	// Uses "world" coordinates
			void DrawHandle( const Vector &vPoint, const Vector2D *vOffset = NULL );
			void DrawHandles( int nPoints, const Vector *Points );
			void DrawArrow( Vector const &vStart, Vector const &vEnd ); 
#ifdef SLE	//// SLE NEW - ported from sdk-2013-hammer
	virtual void DrawArrow(const Vector& vStart, const Vector& vDir, float flLengthBase = 16.0f, float flLengthTip = 8.0f, float flRadiusBase = 1.2f, float flRadiusTip = 4.0f);
#endif		
			void DrawPlane( const Vector &p0, const Vector &p1, const Vector &p2, const Vector &p3, bool bFill = false );

	// drawing complex objects
#ifdef SLE //// SLE CHANGED - overloaded with a few new arguments
			void DrawModel( DrawModelInfo_t* pInfo, matrix3x4_t *pBoneToWorld, const Vector &vOrigin, float fAlpha = 1, bool bWireframe = false, const Color &color = Color(255, 255, 255, 255));
#else
			void DrawModel( DrawModelInfo_t* pInfo, matrix3x4_t *pBoneToWorld, const Vector &vOrigin, float fAlpha = 1, bool bWireframe = false );
#endif
			void DrawDisplacement( CCoreDispInfo *pDisp );
			void DrawCollisionModel( MDLHandle_t mdlHandle, const VMatrix &mViewMatrix );
			
	//
	// helper funtions
	//
			void TransformPoint( Vector2D &vClient, const Vector& vWorld );
			void TransformNormal( Vector2D &vClient, const Vector& vWorld );

			void			GetViewForward( Vector &ViewForward ) const;
			void			GetViewRight( Vector &ViewRight ) const;
			void			GetViewUp( Vector &ViewUp ) const;

			void			PrepareInstanceStencil( void );
			void			DrawInstanceStencil( void );
			void			PushInstanceRendering( InstanceRenderingState_t State );
			void			PopInstanceRendering( void );
			void			SetInstanceRendering( InstanceRenderingState_t State );

			void			SetInstanceRendering( bool InstanceRendering ) { m_bInstanceRendering = InstanceRendering; }
	virtual	void			PushInstanceData( CMapInstance *pInstanceClass, Vector &InstanceOrigin, QAngle &InstanceAngles );
	virtual	void			PopInstanceData( void );
			bool			GetInstanceRendering( void ) { return m_bInstanceRendering; }
			CMapInstance	*GetInstanceClass( void ) { return m_CurrentInstanceState.m_pInstanceClass; }
			void			GetInstanceMatrix( VMatrix &Matrix ) { Matrix = m_CurrentInstanceState.m_InstanceMatrix; }
			Vector			GetInstanceOrigin( void ) { return m_CurrentInstanceState.m_InstanceOrigin; }
			QAngle			GetInstanceAngle( void ) { return m_CurrentInstanceState.m_InstanceAngles; }
			void			TransformInstanceVector( Vector &In, Vector &Out ) { m_CurrentInstanceState.m_InstanceMatrix.V3Mul( In, Out ); }
			void			RotateInstanceVector( Vector &In, Vector &Out ) { VectorRotate( In, m_CurrentInstanceState.m_InstanceMatrix.As3x4(), Out ); }
			void			TransformInstanceAABB( Vector &InMins, Vector &InMaxs, Vector &OutMins, Vector &OutMaxs ) { TransformAABB( m_CurrentInstanceState.m_InstanceMatrix.As3x4(), InMins, InMaxs, OutMins, OutMaxs );  } 
#ifdef HAMMER2013_PORT_PROXIES
	struct ProxyData
	{
		CRender* pRender;
		CMapClass* pClass;
	};
#endif
protected:

	bool GetRequiredMaterial( const char *pName, IMaterial* &pMaterial );
	void UpdateStudioRenderConfig( bool bFlat, bool bWireframe );
	// client space helper functions:
#ifdef SLE //// required in ToolClipper
public:
#endif
	void DrawFilledRect( Vector2D& pt1, Vector2D& pt2, unsigned char *pColor, bool bBorder );
#ifdef SLE //// required in ToolClipper
protected:
#endif
	void DrawCross( Vector2D& pt1, Vector2D& pt2, unsigned char *pColor );
	void DrawCircle( Vector2D &vCenter, float fRadius, int nSegments, unsigned char *pColor );
	void DrawRect( Vector2D& pt1, Vector2D& pt2, unsigned char *pColor );

#ifdef SLE	//// SLE NEW: copied lighting mechanics from HLMV //// SLE TODO, unfinished
//	static Vector *m_AmbientLightColors;
#endif
protected:
	CMapView		*m_pView;
	unsigned long	m_DefaultFont;
	bool			m_bIsClientSpace;

	bool					m_bIsLocalTransform;
	CUtlVector< VMatrix >	m_LocalMatrix;

	VMatrix			m_OrthoMatrix;	
	
	// Meshbuilder used for drawing
	IMesh* m_pMesh;
	CMeshBuilder meshBuilder;

	// colors
	Color		m_DrawColor;         // current draw/fill color
	Color		m_TextColor;         // current text color
	Color		m_HandleColor;       // current text color

	// handle styles
	int			m_nHandleSize;
	int			m_nHandleType;

	// frame count
	int			m_nFrameCount;		// increases each setup camera 
	bool		m_bIsRendering;
	bool		m_bIsRenderingIntoVGUI;

	// materials
	IMaterial*	m_pCurrentMaterial;	// The currently bound material
	IMaterial*	m_pBoundMaterial;	// a material given from external caller

	int			m_nDecalMode;			// 0 or 1 
#ifdef SLE
public:
#endif
	IMaterial*	m_pWireframe[2];		// default wireframe material
#ifdef SLE //// SLE NEW - material used for see-thru solid edges
	IMaterial*	m_pWireframeNoZ[2];		// default wireframe material, ignore Z
#endif
	IMaterial*	m_pFlat[2];				// default flat material
	IMaterial*	m_pDotted[2];			// default dotted material
	IMaterial*	m_pFlatNoZ[2];			// default flat material, ignore Z
	IMaterial*	m_pFlatNoCull[2];		// default flat material, no backface cull
	IMaterial*	m_pTranslucentFlat[2];	// default translucent flat material
	IMaterial*  m_pLightmapGrid[2];		// default lightmap grid material
	IMaterial*  m_pSelectionOverlay[2];	// for selecting actual textures
#ifdef SLE //// SLE NEW - some extra overlays	
	IMaterial*  m_pClippingOverlay[2];
	IMaterial*  m_pHiddenWire[2];
#endif
#ifdef SLE
protected:
#endif
	// render modes
	EditorRenderMode_t m_eCurrentRenderMode;	// Current render mode setting - Wireframe, flat, or textured.
	EditorRenderMode_t m_eDefaultRenderMode;	// Default render mode - Wireframe, flat, or textured.
	CUtlStack<EditorRenderMode_t> m_RenderModeStack;

	// instance
	int								m_nInstanceCount;		// increases each time an instance is drawn regardless of view
	bool							m_bInstanceRendering;	// if true, we are rendering an instance
	VMatrix							m_CurrentMatrix;		// matrix of the current transforms
	int								m_InstanceSelectionDepth;
	TInstanceState					m_CurrentInstanceState;	// the current instance state ( with current transforms )
	CUtlVector< TInstanceState >	m_InstanceState;		// the instance state stack
#ifndef STENCIL_AS_CALLS
	ShaderStencilState_t					m_ShaderStencilState;
#endif // STENCIL_AS_CALLS
	int										m_nNumInstancesRendered;	// number of instances rendered that impacted the stencil buffer
	CUtlVector< InstanceRenderingState_t >	m_InstanceRenderingState;	// the instance rendering state stack
#ifdef HAMMER2013_PORT_PROXIES
	ProxyData								m_proxyData;
#endif
};

#endif // RENDER_H
