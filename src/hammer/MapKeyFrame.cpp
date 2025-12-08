//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "stdafx.h"
#include "Box3D.h"
#include "StockSolids.h"
#include "GlobalFunctions.h"
#include "hammer_mathlib.h"
#include "MapDoc.h"
#include "MapEntity.h"
#include "MapWorld.h"
#include "KeyFrame/KeyFrame.h"
#include "MapKeyFrame.h"
#include "MapAnimator.h"
#include "Render3D.h"
#include "TextureSystem.h"
#include "materialsystem/imesh.h"
#include "Material.h"
#ifdef SLE //// SLE NEW - draw wires with textures and keyvalues
#include "KeyValues.h"
#include "beamdraw.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_MAPCLASS( CMapKeyFrame );

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a CMapKeyFrame from a set
//			of string parameters from the FGD file.
// Input  : *pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, NULL if an error occurs.
//-----------------------------------------------------------------------------
CMapClass *CMapKeyFrame::CreateMapKeyFrame(CHelperInfo *pHelperInfo, CMapEntity *pParent)
{
	return(new CMapKeyFrame);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CMapKeyFrame::CMapKeyFrame()
{
	m_pAnimator = NULL;
	m_pNextKeyFrame = NULL;
	m_flMoveTime = 0;
	m_flSpeed = 0;
	m_bRebuildPath = false;
	m_Angles.Init();
	
	// setup the quaternion identity
	m_qAngles[0] = m_qAngles[1] = m_qAngles[2] = 0;
	m_qAngles[3] = 1;
	
	m_pPositionInterpolator = NULL;
	m_iPositionInterpolator = -1;
	m_iChangeFrame = -1;
	
#ifdef SLE //// SLE NEW - draw wires with textures and keyvalues
	if (m_pParent)
	{
		CMapEntity *pEntity = dynamic_cast<CMapEntity*>(m_pParent);
		if (pEntity)
		{
			cacheTextureName = pEntity->GetKeyValue("RopeMaterial");
			cacheRopeWidth = atof(pEntity->GetKeyValue("Width"));
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMapKeyFrame::~CMapKeyFrame()
{
	if( m_pPositionInterpolator )
		m_pPositionInterpolator->Release();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bFullUpdate - 
//-----------------------------------------------------------------------------
void CMapKeyFrame::CalcBounds(BOOL bFullUpdate)
{
	CMapClass::CalcBounds(bFullUpdate);

	//
	// Calculate the 3D bounds to include all points on our line.
	//
	m_CullBox.ResetBounds();
	m_CullBox.UpdateBounds(m_Origin);

	if( m_pNextKeyFrame )
	{
		// Expand the bbox by the target entity's origin.
		Vector vNextOrigin;
		m_pNextKeyFrame->GetOrigin( vNextOrigin );
		m_CullBox.UpdateBounds(vNextOrigin);

		// Expand the bbox by the points on our line.
		for ( int i=0; i < MAX_LINE_POINTS; i++ )
		{
			m_CullBox.UpdateBounds(m_LinePoints[i]);
		}
	}

	m_BoundingBox = m_CullBox;

	//
	// Our 2D bounds are just a point, because we don't render in 2D.
	//
	m_Render2DBox.ResetBounds();
	m_Render2DBox.UpdateBounds(m_Origin, m_Origin);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CMapClass *
//-----------------------------------------------------------------------------
CMapClass *CMapKeyFrame::Copy(bool bUpdateDependencies)
{
	CMapKeyFrame *pNew = new CMapKeyFrame;
	pNew->CopyFrom(this, bUpdateDependencies);
	return pNew;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pObj - 
// Output : CMapClass *
//-----------------------------------------------------------------------------
CMapClass *CMapKeyFrame::CopyFrom(CMapClass *pObj, bool bUpdateDependencies)
{
	CMapClass::CopyFrom(pObj, bUpdateDependencies);

	CMapKeyFrame *pFrom = dynamic_cast<CMapKeyFrame*>( pObj );
	Assert( pFrom != NULL );

	m_qAngles = pFrom->m_qAngles;
	m_Angles = pFrom->m_Angles;
	m_flSpeed = pFrom->m_flSpeed;
	m_flMoveTime = pFrom->m_flMoveTime;

	if (bUpdateDependencies)
	{
		m_pNextKeyFrame = (CMapKeyFrame *)UpdateDependency(m_pNextKeyFrame, pFrom->m_pNextKeyFrame);
	}
	else
	{
		m_pNextKeyFrame = pFrom->m_pNextKeyFrame;
	}

	m_bRebuildPath = true;

	return this;
}

//-----------------------------------------------------------------------------
// Purpose: notifies the keyframe that it has been cloned
//			inserts the clone into the correct place in the keyframe list
// Input  : *pClone - 
//-----------------------------------------------------------------------------
void CMapKeyFrame::OnClone( CMapClass *pClone, CMapWorld *pWorld, const CMapObjectList &OriginalList, CMapObjectList &NewList )
{
	CMapClass::OnClone( pClone, pWorld, OriginalList, NewList );

	CMapKeyFrame *pNewKey = dynamic_cast<CMapKeyFrame*>( pClone );
	Assert( pNewKey != NULL );
	if ( !pNewKey )
		return;

	CMapEntity *pEntity = dynamic_cast<CMapEntity*>( m_pParent );
	CMapEntity *pNewEntity = dynamic_cast<CMapEntity*>( pClone->GetParent() );

	// insert the newly created keyframe into the sequence

	// point the clone's next at what we were pointing at
	const char *nextKey = pEntity->GetKeyValue( "NextKey" );
	if ( nextKey )
	{
		pNewEntity->SetKeyValue( "NextKey", nextKey );
	}

	// create a new targetname for the clone
	char newName[128];
	const char *oldName = pEntity->GetKeyValue( "targetname" );
	if ( !oldName || oldName[0] == 0 )
		oldName = "keyframe";

	pWorld->GenerateNewTargetname( oldName, newName, sizeof( newName ), true, NULL );
	pNewEntity->SetKeyValue( "targetname", newName );

	// point the current keyframe at the clone
	pEntity->SetKeyValue("NextKey", newName);
}

//-----------------------------------------------------------------------------
// Purpose: Called just after this object has been removed from the world so
//			that it can unlink itself from other objects in the world.
// Input  : pWorld - The world that we were just removed from.
//			bNotifyChildren - Whether we should forward notification to our children.
//-----------------------------------------------------------------------------
void CMapKeyFrame::OnRemoveFromWorld(CMapWorld *pWorld, bool bNotifyChildren)
{
	CMapClass::OnRemoveFromWorld(pWorld, bNotifyChildren);

	//
	// Detach ourselves from the next keyframe in the path.
	//
	m_pNextKeyFrame = (CMapKeyFrame *)UpdateDependency(m_pNextKeyFrame, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *outQuat - 
//-----------------------------------------------------------------------------
void CMapKeyFrame::GetQuatAngles( Quaternion &outQuat )
{
	outQuat = m_qAngles;
}

//-----------------------------------------------------------------------------
// Purpose: Recalulates timings based on the new position
// Input  : *pfOrigin - 
//-----------------------------------------------------------------------------
void CMapKeyFrame::SetOrigin( Vector& pfOrigin )
{
	CMapClass::SetOrigin(pfOrigin);
	m_bRebuildPath = true;
}

//-----------------------------------------------------------------------------
// Purpose: Renders the connecting lines between the keyframes
// Input  : pRender - 
//-----------------------------------------------------------------------------
void CMapKeyFrame::Render3D( CRender3D *pRender )
{
	if ( m_bRebuildPath )
	{
		if (GetAnimator() != NULL)
		{
			GetAnimator()->RebuildPath();
		}
	}
#ifdef SLE //// SLE NEW - draw wires with textures and keyvalues
	// only draw if we have a valid connection
	if ( m_pNextKeyFrame )
	{
		// only draw if we haven't already been drawn this frame
		if ( GetRenderFrame() != pRender->GetRenderFrame() )
		{
		//	pRender->PushRenderMode( RENDER_MODE_WIREFRAME );
			pRender->PushRenderMode(RENDER_MODE_FLAT_NOCULL);

			SetRenderFrame( pRender->GetRenderFrame() );

			Vector o1, o2;
			GetOrigin( o1 );
			m_pNextKeyFrame->GetOrigin( o2 );
			
			IMaterial *ropeMaterial;
			CMeshBuilder meshBuilder;
			CMatRenderContextPtr pRenderContext( materials );

			if (Q_strlen(cacheTextureName) > 0)
			{
				ropeMaterial = MaterialSystemInterface()->FindMaterial(cacheTextureName, TEXTURE_GROUP_OTHER);
			}
			else
			{
				if (m_pParent)
				{
					CMapEntity *pEntity = dynamic_cast<CMapEntity*>(m_pParent);
					if (pEntity)
					{
						cacheTextureName = pEntity->GetKeyValue("RopeMaterial");
						if( Q_strlen(cacheTextureName) > 0)
							ropeMaterial = MaterialSystemInterface()->FindMaterial(cacheTextureName, TEXTURE_GROUP_OTHER);
					}
				}
			}
			if (!ropeMaterial)
			{
				ropeMaterial = materials->FindMaterial("cable/cable", TEXTURE_GROUP_OTHER);
				AssertMsg(ropeMaterial != NULL, "Couldn't find the default rope material cable/cable!");
			}
			// construct a fake vmt, because 'Cable' doesn't work correctly in-editor
			if ( ropeMaterial )
			{
				bool found;
				IMaterialVar *ropeTex = ropeMaterial->FindVar("$basetexture", &found, false);
				if ( found )
				{
					KeyValues *pVMTKeyValues = new KeyValues("Unlitgeneric");
					pVMTKeyValues->SetString("$basetexture", ropeTex->GetStringValue());
					pVMTKeyValues->SetString("$translucent", "1");
					pVMTKeyValues->SetString("$nocull", "1");
					ropeMaterial->SetShaderAndParams(pVMTKeyValues);
					pVMTKeyValues->deleteThis();
					ropeMaterial->AddRef();
				}
			}

			// deal with losing the material if it happens
			if ( ropeMaterial->IsErrorMaterial() )
			{
				if ( m_pParent )
				{
					CMapEntity *pEntity = dynamic_cast<CMapEntity*>( m_pParent );
					if ( pEntity )
					{
						cacheTextureName = pEntity->GetKeyValue("RopeMaterial");
						if ( Q_strlen(cacheTextureName) > 0 )
							ropeMaterial = MaterialSystemInterface()->FindMaterial(cacheTextureName, TEXTURE_GROUP_OTHER);
						m_bRebuildPath = true;
					}
				}
			}
			//
			IMesh *pMesh = pRenderContext->GetDynamicMesh(true, NULL, NULL, ropeMaterial);

#if 1 ///// old method of rope-drawing
			// draw connecting line going from green to red
			meshBuilder.Begin(pMesh, MATERIAL_QUADS, MAX_LINE_POINTS);
			meshBuilder.Color3ub(255, 255, 255);

			// start point //// SLE NEW: changed red-green wires to black.
			if (IsSelected())
			{
		//		meshBuilder.Color3f(0.9f, 0.9f, 0.05f); //( 0, 1.0f, 0 ); //// SLE NEW: selected keyframes display the rope to next KF in yellow.
			}
			else
			{
		//		meshBuilder.Color3f(0.05f, 0.05f, 0.05f); //( 0, 1.0f, 0 );
			}
			for ( int i = 0; i < MAX_LINE_POINTS; i++ )
			{
				if (i == 0)
				{
					meshBuilder.Position3f(o1[0], o1[1], o1[2] + 1);
					meshBuilder.TexCoord2f(0, 0, 0);
					meshBuilder.AdvanceVertex();
					meshBuilder.Position3f(m_LinePoints[i][0], m_LinePoints[i][1], m_LinePoints[i][2] + cacheRopeWidth / 2);
					meshBuilder.TexCoord2f(0, 0, 1);
					meshBuilder.AdvanceVertex();
					meshBuilder.Position3f(m_LinePoints[i][0], m_LinePoints[i][1], m_LinePoints[i][2] - cacheRopeWidth / 2);
					meshBuilder.TexCoord2f(0, 1, 1);
					meshBuilder.AdvanceVertex();
					meshBuilder.Position3f(o1[0], o1[1], o1[2] - 1);
					meshBuilder.TexCoord2f(0, 1, 0);
					meshBuilder.AdvanceVertex();
				}
				else if (i == MAX_LINE_POINTS && o2 != Vector(0,0,0))
				{
					meshBuilder.Position3f(m_LinePoints[i][0], m_LinePoints[i][1], m_LinePoints[i][2] + cacheRopeWidth / 2);
					meshBuilder.TexCoord2f(0, 0, 1);
					meshBuilder.AdvanceVertex();
					meshBuilder.Position3f(o2[0], o2[1], o2[2] + 1);
					meshBuilder.TexCoord2f(0, 0, 0);
					meshBuilder.AdvanceVertex();
					meshBuilder.Position3f(o2[0], o2[1], o2[2] - 1);
					meshBuilder.TexCoord2f(0, 0, 0);
					meshBuilder.AdvanceVertex();
					meshBuilder.Position3f(m_LinePoints[i][0], m_LinePoints[i][1], m_LinePoints[i][2] - cacheRopeWidth / 2);
					meshBuilder.TexCoord2f(0, 1, 1);
					meshBuilder.AdvanceVertex();
				}
				else
				{
					meshBuilder.Position3f(m_LinePoints[i-1][0], m_LinePoints[i-1][1], m_LinePoints[i-1][2] + cacheRopeWidth / 2);
					meshBuilder.TexCoord2f(0, 0, 0);
					meshBuilder.AdvanceVertex();
					meshBuilder.Position3f(m_LinePoints[i][0], m_LinePoints[i][1], m_LinePoints[i][2] + cacheRopeWidth / 2);
					meshBuilder.TexCoord2f(0, 0, 1);
					meshBuilder.AdvanceVertex();
					meshBuilder.Position3f(m_LinePoints[i][0], m_LinePoints[i][1], m_LinePoints[i][2] - cacheRopeWidth / 2);
					meshBuilder.TexCoord2f(0, 1, 1);
					meshBuilder.AdvanceVertex();
					meshBuilder.Position3f(m_LinePoints[i-1][0], m_LinePoints[i-1][1], m_LinePoints[i-1][2] - cacheRopeWidth / 2);
					meshBuilder.TexCoord2f(0, 1, 0);
					meshBuilder.AdvanceVertex();
				}
			}

			meshBuilder.End();
			pMesh->Draw();

			pRender->PopRenderMode();

#else //// new method is currently bugged in terms of shader and drawing
			
			pRender->BeginRenderHitTarget(this);
			pRender->PushRenderMode(RENDER_MODE_WIREFRAME);

			float *noise;
			float fcolor[ 3 ];
			fcolor[ 0 ] = 1;
			fcolor[ 1 ] = 1;
			fcolor[ 2 ] = 1;

			for ( int i = 0; i < MAX_LINE_POINTS; i++ )
			{
				if ( i == 0 ) // o1 to m_LinePoints[i]
				{
					DrawSegs(pRender, 10, noise, cacheTextureName, 1, kRenderNormal, 
						o1, ( m_LinePoints[ i ] - o1 ), cacheRopeWidth, cacheRopeWidth, 
						1, 1, 30, 2, FBEAM_SOLID, fcolor, 0);
				}
				else if ( i == MAX_LINE_POINTS && o2 != Vector(0, 0, 0) ) // m_LinePoints[i] to o2
				{
					DrawSegs(pRender, 10, noise, cacheTextureName, 1, kRenderNormal, 
						m_LinePoints[ i ], ( o2 - m_LinePoints[ i ] ), cacheRopeWidth, cacheRopeWidth, 
						1, 1, 30, 2, FBEAM_SOLID, fcolor, 0);
				}
				else // m_LinePoints[i-1] to m_LinePoints[i]
				{
					DrawSegs(pRender, 10, noise, cacheTextureName, 1, kRenderNormal, 
						m_LinePoints[ i-1 ], ( m_LinePoints[ i ] - m_LinePoints[ i-1 ] ), cacheRopeWidth, cacheRopeWidth, 
						1, 1, 30, 2, FBEAM_SOLID, fcolor, 0);
				}
			}
			pRender->EndRenderHitTarget();
			pRender->PopRenderMode();
#endif			
			//// SLE NEW - if selected, then also draw a yellow wireframe version on top of textured version
			if ( IsSelected() )
			{
				CMeshBuilder meshBuilder;
				CMatRenderContextPtr pRenderContext(MaterialSystemInterface());
				pRender->PushRenderMode(RENDER_MODE_WIREFRAME);
				IMesh *pMesh = pRenderContext->GetDynamicMesh();

				// draw connecting line going from green to red
				meshBuilder.Begin(pMesh, MATERIAL_LINE_STRIP, MAX_LINE_POINTS);

				// start point
				meshBuilder.Color3f(0.9f, 0.9f, 0.05f);
				meshBuilder.Position3f(o1[ 0 ], o1[ 1 ], o1[ 2 ]);
				meshBuilder.AdvanceVertex();

				for ( int i = 0; i < MAX_LINE_POINTS; i++ )
				{
					meshBuilder.Color3f(0.9f, 0.9f, 0.05f);
					meshBuilder.Position3f(m_LinePoints[ i ][ 0 ], m_LinePoints[ i ][ 1 ], m_LinePoints[ i ][ 2 ]);
					meshBuilder.AdvanceVertex();
				}

				meshBuilder.End();
				pMesh->Draw();
				pRender->PopRenderMode();
			}
		}
	}	
#else //// stock, non-textured wires
	if (m_pNextKeyFrame && m_flSpeed > 0)
	{
		// only draw if we haven't already been drawn this frame
		if (GetRenderFrame() != pRender->GetRenderFrame())
		{
			pRender->PushRenderMode(RENDER_MODE_WIREFRAME);

			SetRenderFrame(pRender->GetRenderFrame());

			Vector o1, o2;
			GetOrigin(o1);
			m_pNextKeyFrame->GetOrigin(o2);

			CMeshBuilder meshBuilder;
			CMatRenderContextPtr pRenderContext(MaterialSystemInterface());
			IMesh *pMesh = pRenderContext->GetDynamicMesh();

			// draw connecting line going from green to red
			meshBuilder.Begin(pMesh, MATERIAL_LINE_STRIP, MAX_LINE_POINTS);

			// start point
			meshBuilder.Color3f(0, 1.0f, 0);
			meshBuilder.Position3f(o1[0], o1[1], o1[2]);
			meshBuilder.AdvanceVertex();

			for (int i = 0; i < MAX_LINE_POINTS; i++)
			{
				float red = (float)(i + 1) / (float)MAX_LINE_POINTS;
				meshBuilder.Color3f(red, 1.0f - red, 0);
				meshBuilder.Position3f(m_LinePoints[i][0], m_LinePoints[i][1], m_LinePoints[i][2]);
				meshBuilder.AdvanceVertex();
			}

			meshBuilder.End();
			pMesh->Draw();

			pRender->PopRenderMode();
		}
	}
#endif //// SLE
}

//-----------------------------------------------------------------------------
// Purpose: Returns the total time remaining in the animation sequence in seconds.
//-----------------------------------------------------------------------------
float CMapKeyFrame::GetRemainingTime( CMapObjectList *pVisited )
{
	CMapObjectList Visited;
	if ( pVisited == NULL )
	{
		pVisited = &Visited;
	}

	//
	// Check for circularities.
	//
	if ( pVisited->Find( this ) != -1 )
	{
		return 0.0f;
	}

	pVisited->AddToTail( this );

	if ( m_pNextKeyFrame )
	{
		return m_flMoveTime + m_pNextKeyFrame->GetRemainingTime( pVisited );
	}

	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CMapKeyFrame
//-----------------------------------------------------------------------------
CMapKeyFrame *CMapKeyFrame::NextKeyFrame( void )
{
	if ( !m_pNextKeyFrame )
		return this;

	return m_pNextKeyFrame;
}

//-----------------------------------------------------------------------------
// Purpose: Notifies that the entity this is attached to has had a key change
// Input  : key - 
//			value - 
//-----------------------------------------------------------------------------
void CMapKeyFrame::OnParentKeyChanged( const char* key, const char* value )
{
	if ( !stricmp(key, "NextKey") )
	{
		m_bRebuildPath = true;
	}
	else if (!stricmp(key, "MoveSpeed"))
	{
		m_flSpeed = atof(value);
		m_bRebuildPath = true;
	}
	
	else if ( !stricmp(key, "NextTime") )
	{
		m_flMoveTime = atof( value );
	}

#ifndef SLE //// ropes don't use angles
	else if (!stricmp(key, "angles"))
	{
		sscanf(value, "%f %f %f", &m_Angles[PITCH], &m_Angles[YAW], &m_Angles[ROLL]);
		AngleQuaternion(m_Angles, m_qAngles);
	}
#endif
	
#ifdef SLE //// SLE NEW - draw wires with textures and keyvalues
	else if (!stricmp(key, "Width"))
	{
		cacheRopeWidth = atof(value);
		m_bRebuildPath = true;
	}
	
	else if (!stricmp(key, "RopeMaterial"))
	{
		cacheTextureName = (value);
		m_bRebuildPath = true;
	}
#endif
	if( m_pPositionInterpolator )
	{
		if( m_pPositionInterpolator->ProcessKey( key, value ) )
			m_bRebuildPath = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: calculates the time the current key frame should take, given
//			the movement speed, and the distance to the next keyframe
//-----------------------------------------------------------------------------
void CMapKeyFrame::RecalculateTimeFromSpeed( void )
{
	if ( m_flSpeed <= 0 )
		return;

	if ( !m_pNextKeyFrame )
		return;

	// calculate the distance to the next key
	Vector o1;
	m_pNextKeyFrame->GetOrigin( o1 );

	Vector o2 = o1 - m_Origin;
	float dist = VectorLength( o2 );

	// couldn't get time from distance, get it from rotation instead
	if ( !dist )
	{
		// speed is in degrees per second
		// find the largest rotation component and use that
		QAngle ang = m_Angles - m_pNextKeyFrame->m_Angles;
		dist = 0;
		for ( int i = 0; i < 3; i++ )
		{
			fixang( ang[i] );
			if ( ang[i] > 180 )
				ang[i] = ang[i] - 360;

			if ( abs(ang[i]) > dist )
			{
				dist = abs(ang[i]);
			}
		}
	}

	// time = distance / speed
	float newTime = dist / m_flSpeed;

	// set the new speed (99.99% of the time this is the same so don't
	// bother forcing it to rebuild the path).
	if( m_flMoveTime != newTime )
	{
		m_flMoveTime = newTime;

		// rebuild the path before we next render
		m_bRebuildPath = true;
	}

	// "NextTime" key removed until we get a real-time updating entity properties dialog		
	/*
	CMapEntity *ent = dynamic_cast<CMapEntity*>(m_pParent);
	if ( ent )
	{
		char buf[16];
		sprintf( buf, "%.2f", newTime );
		ent->SetKeyValue( "NextTime", buf );
		ent->OnParentKeyChanged( "NextTime", buf );
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: Builds the spline points between this keyframe and the previous
//			keyframe.
// Input  : pPrev - 
//-----------------------------------------------------------------------------
void CMapKeyFrame::BuildPathSegment( CMapKeyFrame *pPrev )
{
	RecalculateTimeFromSpeed();

	CMapAnimator *pAnim = GetAnimator();

	Quaternion qAngles;
	for ( int i = 0; i < MAX_LINE_POINTS; i++ )
	{
		if (pAnim != NULL)
		{
			CMapAnimator::GetAnimationAtTime( this, pPrev, MoveTime() * ( float )( i + 1 ) / (float)MAX_LINE_POINTS, m_LinePoints[i], qAngles, pAnim->m_iPositionInterpolator, pAnim->m_iRotationInterpolator );
		}
		else
		{
			// FIXME: If we aren't connected to an animator yet, just draw straight lines. This code is never hit, because
			//		 BuildPathSegment is only called from CMapAnimator. To make matters worse, we can only reliably find
			//		 pPrev through an animator.
			CMapAnimator::GetAnimationAtTime( this, pPrev, MoveTime() * (float)( i + 1) / (float)MAX_LINE_POINTS, m_LinePoints[i], qAngles, 0, 0 );
		}
	}

	// HACK: we shouldn't need to do this. CalcBounds alone should work (but it doesn't because of where we
	// call RebuildPath from). Make this work more like other objects.
	if ( m_pParent )
	{
		GetParent()->CalcBounds( true );
	}
	else
	{
		CalcBounds();
	}

	m_bRebuildPath = false;
}

//-----------------------------------------------------------------------------
// Purpose: Called when an object that we depend on has changed.
//-----------------------------------------------------------------------------
void CMapKeyFrame::OnNotifyDependent(CMapClass *pObject, Notify_Dependent_t eNotifyType)
{
	CMapClass::OnNotifyDependent(pObject, eNotifyType);

	if ((pObject == m_pAnimator) && (eNotifyType == Notify_Removed))
	{
		SetAnimator(NULL);
	}

	//
	// If our next keyframe was deleted, try to link to the one after it.
	//
	if ((pObject == m_pNextKeyFrame) && (eNotifyType == Notify_Removed))
	{
		CMapEntity *pNextParent = m_pNextKeyFrame->GetParentEntity();
		CMapEntity *pParent = GetParentEntity();

		if ( pNextParent && pParent )
		{
			const char *szNext = pNextParent->GetKeyValue("NextKey");
			pParent->SetKeyValue("NextKey", szNext);
		}
	}

	m_bRebuildPath = true;
}

//-----------------------------------------------------------------------------
// Purpose: returns a pointer to our parent entity
// Output : CMapEntity
//-----------------------------------------------------------------------------
CMapEntity *CMapKeyFrame::GetParentEntity( void )
{
	return dynamic_cast<CMapEntity*>( m_pParent );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CMapKeyFrame::IsAnyKeyInSequenceSelected( void )
{
	if ( m_pParent && m_pParent->IsSelected() )
	{
		return true;
	}

	// search forward
	for ( CMapKeyFrame *find = m_pAnimator; find != NULL; find = find->m_pNextKeyFrame )
	{
		if ( find->m_pParent && find->m_pParent->IsSelected() )
		{
			return true;
		}
	}

	// no selected items found
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iInterpolator - 
// Output : IPositionInterpolator
//-----------------------------------------------------------------------------
IPositionInterpolator* CMapKeyFrame::SetupPositionInterpolator( int iInterpolator )
{
	if( iInterpolator != m_iPositionInterpolator )
	{
		if( m_pPositionInterpolator )
			m_pPositionInterpolator->Release();

		m_pPositionInterpolator = Motion_GetPositionInterpolator( iInterpolator );
		m_iPositionInterpolator = iInterpolator;

		// Feed keys..
		CMapEntity *pEnt = GetParentEntity();
		if( pEnt )
		{
			for ( int i=pEnt->GetFirstKeyValue(); i != pEnt->GetInvalidKeyValue(); i=pEnt->GetNextKeyValue( i ) )
			{
				m_pPositionInterpolator->ProcessKey( 
					pEnt->GetKey( i ),
					pEnt->GetKeyValue( i ) );
			}
		}
	}


	return m_pPositionInterpolator;
}

//-----------------------------------------------------------------------------
// Purpose: Marks that we need to relink any pointers defined by target/targetname pairs
//-----------------------------------------------------------------------------
void CMapKeyFrame::UpdateDependencies(CMapWorld *pWorld, CMapClass *pObject)
{
	CMapClass::UpdateDependencies(pWorld, pObject);
	m_bRebuildPath = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapKeyFrame::SetAnimator(CMapAnimator *pAnimator)
{
	m_pAnimator = (CMapAnimator *)UpdateDependency(m_pAnimator, pAnimator);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapKeyFrame::SetNextKeyFrame(CMapKeyFrame *pNext)
{
	m_pNextKeyFrame = (CMapKeyFrame *)UpdateDependency(m_pNextKeyFrame, pNext);
}
