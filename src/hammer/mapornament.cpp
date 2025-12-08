#include "stdafx.h"
#include "hammer_mathlib.h"
#include "box3d.h"
#include "bspfile.h"
#include "const.h"
#include "KeyValues.h"
#include "mapdefs.h"		// dvs: For COORD_NOTINIT
#include "mapdoc.h"
#include "mapentity.h"
#include "mapornament.h"
#include "render2d.h"
#include "render3d.h"
#include "hammer.h"
#include "texturesystem.h"
#include "materialsystem/imesh.h"
#include "material.h"
#include "options.h"
#include "camera.h"
#include "sprite.h"
#include "MapStudioModel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

IMPLEMENT_MAPCLASS(COrnamentHelper)

//-----------------------------------------------------------------------------
// Purpose: Factory function. Used for creating a COrnamentHelper from a set
//			of string parameters from the FGD file.
// Input  : *pInfo - Pointer to helper info class which gives us information
//				about how to create the class.
// Output : Returns a pointer to the class, NULL if an error occurs.
//-----------------------------------------------------------------------------
CMapClass* COrnamentHelper::CreateOrnament(CHelperInfo* pHelperInfo, CMapEntity* pParent)
{
	if (!pParent) return NULL;

	COrnamentHelper* ornament = new COrnamentHelper;

	if (ornament)
	{
		ornament->m_parent_handle = pParent;

		const char* key = pParent->GetKeyValue("ornamentModel");
		ornament->m_model_name_string = key;

		ornament->m_model_handle = CStudioModelCache::CreateModel(ornament->m_model_name_string);
		if (ornament->m_model_handle)
		{
			ornament->CalcBounds();
		}
		/*
		key = pParent->GetKeyValue("ornamentAttachment");
		ornament->m_model_attachment_string = key;
		key = pParent->GetKeyValue("ornamentScale");
		ornament->m_model_scale_float = atof(key);
		key = pParent->GetKeyValue("ornamentSkin");
		ornament->m_model_skin_int = atoi(key);
		float x, y, z;
		sscanf(pParent->GetKeyValue("ornamentOffset"), "%d %d %d", &x, &y, &z);
		ornament->m_model_offset_vec.x = x;
		ornament->m_model_offset_vec.y = y;
		ornament->m_model_offset_vec.z = z;
		*/
		if (ornament->m_model_handle)
		{
			ornament->m_model_handle->SetSkin(ornament->m_model_skin_int);
			ornament->m_model_handle->SetModelScale(ornament->m_model_scale_float);
			Vector placement;
			pParent->GetOrigin(placement); // default to the model's origin
			CMapStudioModel *parentmodel = pParent->GetChildOfType((CMapStudioModel *)NULL);
			if (parentmodel && parentmodel->m_pStudioModel)
			{
				parentmodel->m_pStudioModel->GetAttachmentPosition(placement, 1);
			}
			ornament->m_model_pos_vec = placement;
			ornament->SetOrigin(ornament->m_model_pos_vec);
			ornament->m_model_handle->SetOrigin(ornament->m_model_pos_vec);
		}
	}
	return ornament;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
COrnamentHelper::COrnamentHelper()
{
	Initialize();
}

COrnamentHelper::~COrnamentHelper(void)
{
	if (m_model_handle != NULL)
	{
		CStudioModelCache::Release(m_model_handle);
	}
}

void COrnamentHelper::GetAngles(QAngle &Angles)
{
	Angles = m_Angles;
}

void COrnamentHelper::GetRenderAngles(QAngle &Angles)
{
	GetAngles(Angles);
}

//-----------------------------------------------------------------------------
// Purpose: Calculates our bounding box based on the sprite dimensions.
// Input  : bFullUpdate - Whether we should recalculate our childrens' bounds.
//-----------------------------------------------------------------------------
void COrnamentHelper::CalcBounds(BOOL bFullUpdate)
{
	CMapClass::CalcBounds(bFullUpdate);

	Vector Mins(0, 0, 0);
	Vector Maxs(0, 0, 0);

	if (m_model_handle != NULL && Q_strcmp(m_model_name_string, ""))
	{
		//
		// The 3D bounds are the bounds of the oriented model's first sequence, so that
		// frustum culling works properly in the 3D view.
		//
		QAngle angles;
		GetRenderAngles(angles);

		m_model_handle->SetAngles(angles);
		m_model_handle->ExtractBbox(m_CullBox.bmins, m_CullBox.bmaxs);
		m_model_handle->ExtractMovementBbox(Mins, Maxs);

		Mins += m_Origin;
		Maxs += m_Origin;

		m_CullBox.bmins += m_Origin;
		m_CullBox.bmaxs += m_Origin;
	}

	//
	// If we do not yet have a valid bounding box, use a default box.
	//
	if ((Maxs - Mins) == Vector(0, 0, 0))
	{
		Mins = m_CullBox.bmins = m_Origin - Vector(10, 10, 10);
		Maxs = m_CullBox.bmaxs = m_Origin + Vector(10, 10, 10);
	}

	m_BoundingBox = m_CullBox;
//	m_Render2DBox.UpdateBounds(Mins, Maxs);
}

//-----------------------------------------------------------------------------
// Purpose: Returns a copy of this object.
// Output : Pointer to the new object.
//-----------------------------------------------------------------------------
CMapClass* COrnamentHelper::Copy(bool bUpdateDependencies)
{
	COrnamentHelper* pCopy = new COrnamentHelper;

	if (pCopy != NULL)
		pCopy->CopyFrom(this, bUpdateDependencies);

	return pCopy;
}

//-----------------------------------------------------------------------------
// Purpose: Turns this into a duplicate of the given object.
// Input  : pObject - Pointer to the object to copy from.
// Output : Returns a pointer to this object.
//-----------------------------------------------------------------------------
CMapClass* COrnamentHelper::CopyFrom(CMapClass* pObject, bool bUpdateDependencies)
{
	COrnamentHelper* pFrom = dynamic_cast<COrnamentHelper*>(pObject);
	Assert(pFrom != NULL);

	if (pFrom != NULL)
	{
		CMapClass::CopyFrom(pObject, bUpdateDependencies);

		m_Angles = pFrom->m_Angles;
		m_model_attachment_int = pFrom->m_model_attachment_int;
		m_model_attachment_string = pFrom->m_model_attachment_string;
		m_model_name_string = pFrom->m_model_name_string;
		m_model_scale_float = pFrom->m_model_scale_float;
		m_model_skin_int = pFrom->m_model_skin_int;
		m_model_pos_vec = pFrom->m_model_pos_vec;

	}

	return this;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void COrnamentHelper::Initialize()
{
	m_Angles.Init();
	m_model_pos_vec = Vector(0, 0, 0);
	m_model_skin_int = 0;
	m_model_attachment_int = 0;
	m_model_scale_float = 1.0f;
	m_model_attachment_string = "eyes";
	m_model_name_string = "";
	m_model_handle = NULL;
	m_parent_handle = NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : pRender -
//-----------------------------------------------------------------------------
bool COrnamentHelper::RenderPreload(CRender3D *pRender, bool bNewContext)
{
	return(m_model_handle != NULL);
}

void COrnamentHelper::Render3D(CRender3D* pRender)
{
	if (!GetParent()) return;

	if (!m_model_handle) return;
	
	m_model_handle->SetSkin(m_model_skin_int);
	m_model_handle->SetModelScale(m_model_scale_float);
	m_model_handle->SetOrigin(m_model_pos_vec);
	SetOrigin(m_model_pos_vec);

	if( Q_strcmp(m_model_name_string, ""))
		m_model_handle->DrawModel3D(pRender, Color(255, 255, 255), 1.0f, false);
}

//-----------------------------------------------------------------------------
void COrnamentHelper::DoTransform(const VMatrix& matrix)
{
	BaseClass::DoTransform(matrix);
}

//-----------------------------------------------------------------------------
// Purpose: Notifies that this object's parent entity has had a key value change.
// Input  : szKey - The key that changed.
//			szValue - The new value of the key.
//-----------------------------------------------------------------------------
void COrnamentHelper::OnParentKeyChanged(const char* szKey, const char* szValue)
{
	if (!m_parent_handle) return;

	if (!stricmp(szKey, "ornamentModel"))
	{
		m_model_name_string = szValue;

		if(m_model_handle)CStudioModelCache::Release(m_model_handle);
		m_model_handle = CStudioModelCache::CreateModel(m_model_name_string);
		if (m_model_handle)
		{
			CalcBounds(true);
		}

		PostUpdate(Notify_Changed);
	}
	if (!stricmp(szKey, "ornamentAttachment"))
	{
		m_model_attachment_int = atoi(szValue);
		PostUpdate(Notify_Changed);
	}
	if (!stricmp(szKey, "ornamentScale"))
	{
		m_model_scale_float = atof(szValue);
		if (m_model_handle) m_model_handle->SetModelScale(m_model_scale_float);
		PostUpdate(Notify_Changed);
	}
	if (!stricmp(szKey, "ornamentSkin"))
	{
		m_model_skin_int = atoi(szValue);
		if (m_model_handle) m_model_handle->SetSkin(m_model_skin_int);
		PostUpdate(Notify_Changed);
	}
	if (!stricmp(szKey, "ornamentOffset"))
	{
		//m_model_pos_vec
		Vector placement, offset;
		sscanf(szValue, "%f %f %f", &offset[0], &offset[1], &offset[2]);
		GetParent()->GetOrigin(placement); // default to the model's origin
		CMapStudioModel *parentmodel = m_parent_handle->GetChildOfType((CMapStudioModel *)NULL);
		if (parentmodel && parentmodel->m_pStudioModel)
		{
			parentmodel->m_pStudioModel->GetAttachmentPosition(placement, m_model_attachment_int);
		}
		m_model_pos_vec = placement + offset;
		SetOrigin(m_model_pos_vec);
		if (m_model_handle) m_model_handle->SetOrigin(m_model_pos_vec);
		CalcBounds(true);
		PostUpdate(Notify_Changed);
	}
	if (!stricmp(szKey, "angles"))
	{
		sscanf(szValue, "%f %f %f", &m_Angles[PITCH], &m_Angles[YAW], &m_Angles[ROLL]);
		//	CalcBounds(true);
		PostUpdate(Notify_Changed);
	}
}

//-----------------------------------------------------------------------------
void COrnamentHelper::Render2D(CRender2D* pRender)
{
	Vector vecMins;
	Vector vecMaxs;
	GetRender2DBox(vecMins, vecMaxs);

	Vector2D pt, pt2;
	pRender->TransformPoint(pt, vecMins);
	pRender->TransformPoint(pt2, vecMaxs);

	if (!IsSelected())
	{
		pRender->SetDrawColor(r, g, b);
		pRender->SetHandleColor(r, g, b);
	}
	else
	{
		pRender->SetDrawColor(GetRValue(Options.colors.clr2DSelection), GetGValue(Options.colors.clr2DSelection), GetBValue(Options.colors.clr2DSelection));
		pRender->SetHandleColor(GetRValue(Options.colors.clr2DSelection), GetGValue(Options.colors.clr2DSelection), GetBValue(Options.colors.clr2DSelection));
	}

	// Draw the bounding box.

	pRender->DrawBox(vecMins, vecMaxs);

	//
	// Draw center handle.
	//

	if (pRender->IsActiveView())
	{
		int sizex = abs(pt.x - pt2.x) + 1;
		int sizey = abs(pt.y - pt2.y) + 1;

		// dont draw handle if object is too small
		if (sizex > 6 && sizey > 6)
		{
			pRender->SetHandleStyle(HANDLE_RADIUS, CRender::HANDLE_CROSS);
			pRender->DrawHandle((vecMins + vecMaxs) / 2);
		}
	}
}
