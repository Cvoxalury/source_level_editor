//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "stdafx.h"
#include "IHammer.h"
#include "tier1/strtools.h"
#include "materialsystem/IMaterialProxy.h"
#include "materialsystem/IMaterialProxyFactory.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterial.h"
#ifdef HAMMER2013_PORT_PROXIES
#include "camera.h"
#include "mapclass.h"
#include "mapview.h"
#include "mapworld.h"

#include "materialsystem/itexture.h"
#include "tier1/KeyValues.h"
#include "tier1/utldict.h"
#include "tier1/utlstring.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifdef HAMMER2013_PORT_PROXIES
extern float frameTime;

// Stringify a number
#define V_STRINGIFY_INTERNAL(x) #x
// Extra level of indirection needed when passing in a macro to avoid getting the macro name instead of value
#define V_STRINGIFY(x) V_STRINGIFY_INTERNAL(x)
// ------------------------------------------------------------------------------------- //
// The material proxy factory for WC.
// ------------------------------------------------------------------------------------- //

class CMaterialProxyFactory final : public IMaterialProxyFactory
{
public:
	IMaterialProxy* CreateProxy(const char* proxyName) override;
	void DeleteProxy(IMaterialProxy* pProxy) override;

	using InstantiateProxyFn = IMaterialProxy* (*)();
	class CMaterialProxyFactoryRegister
	{
	public:
		CMaterialProxyFactoryRegister(const char* pProxyName, InstantiateProxyFn instFunc);
	};
private:
	CUtlDict<InstantiateProxyFn> m_proxies;
};
static CMaterialProxyFactory g_MaterialProxyFactory;

#define REGISTER_PROXY( className, proxyName ) \
	static IMaterialProxy* __Create##proxyName##_proxy() { return new className; } \
	static CMaterialProxyFactory::CMaterialProxyFactoryRegister __##proxyName##_proxy_register( V_STRINGIFY( proxyName ), __Create##proxyName##_proxy )


IMaterialProxy* CMaterialProxyFactory::CreateProxy(const char* proxyName)
{
	const int find = m_proxies.Find(proxyName);
	if (m_proxies.IsValidIndex(find))
		return m_proxies[find]();
	return CreateProxy("Empty"); // silence proxy warnings
}

void CMaterialProxyFactory::DeleteProxy(IMaterialProxy* pProxy)
{
	if (pProxy)
		pProxy->Release();
}

CMaterialProxyFactory::CMaterialProxyFactoryRegister::CMaterialProxyFactoryRegister(const char* pProxyName, InstantiateProxyFn instFunc)
{
	g_MaterialProxyFactory.m_proxies.Insert(pProxyName, instFunc);
}

IMaterialProxyFactory* GetHammerMaterialProxyFactory()
{
	return &g_MaterialProxyFactory;
}


#pragma region Base
class CFloatInput
{
public:
	CFloatInput() : m_flValue(0), m_pFloatVar(nullptr), m_FloatVecComp(0) {}

	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues, const char* pKeyName, float flDefault = 0.0f)
	{
		m_pFloatVar = nullptr;
		KeyValues* pSection = pKeyValues->FindKey(pKeyName);
		if (pSection)
		{
			if (pSection->GetDataType() == KeyValues::TYPE_STRING)
			{
				const char* pVarName = pSection->GetString();

				float flValue;
				const int nCount = sscanf(pVarName, "%f", &flValue);
				if (nCount == 1)
				{
					m_flValue = flValue;
					return true;
				}

				char pTemp[256];
				if (strchr(pVarName, '['))
				{
					Q_strncpy(pTemp, pVarName, 256);
					char* pArray = strchr(pTemp, '[');
					*pArray++ = 0;

					char* pIEnd;
					m_FloatVecComp = strtol(pArray, &pIEnd, 10);

					pVarName = pTemp;
				}
				else
				{
					m_FloatVecComp = -1;
				}

				bool bFoundVar;
				m_pFloatVar = pMaterial->FindVar(pVarName, &bFoundVar, true);
				if (!bFoundVar)
					return false;
			}
			else
			{
				m_flValue = pSection->GetFloat();
			}
		}
		else
		{
			m_flValue = flDefault;
		}
		return true;
	}

	float GetFloat() const
	{
		if (!m_pFloatVar)
			return m_flValue;

		if (m_FloatVecComp < 0)
			return m_pFloatVar->GetFloatValue();

		const int iVecSize = m_pFloatVar->VectorSize();
		if (m_FloatVecComp >= iVecSize)
			return 0;

		float v[4];
		m_pFloatVar->GetVecValue(v, iVecSize);
		return v[m_FloatVecComp];
	}

private:
	float m_flValue;
	IMaterialVar *m_pFloatVar;
	int	m_FloatVecComp;
};

class CResultProxy : public IMaterialProxy
{
public:
	CResultProxy() : m_pResult(nullptr), m_ResultVecComp(0) {}
	~CResultProxy() override = default;

	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		char const* pResult = pKeyValues->GetString("resultVar");
		if (!pResult)
			return false;

		char pTemp[256];
		if (strchr(pResult, '['))
		{
			V_strcpy_safe(pTemp, pResult);
			char* pArray = strchr(pTemp, '[');
			*pArray++ = 0;

			char* pIEnd;
			m_ResultVecComp = strtol(pArray, &pIEnd, 10);

			pResult = pTemp;
		}
		else
		{
			m_ResultVecComp = -1;
		}

		bool foundVar;
		m_pResult = pMaterial->FindVar(pResult, &foundVar, true);
		return foundVar;
	}

	void Release() override { delete this; }
	IMaterial* GetMaterial() override { return m_pResult->GetOwningMaterial(); }

protected:
	void SetFloatResult(float result)
	{
		if (m_pResult->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		{
			if (m_ResultVecComp >= 0)
				m_pResult->SetVecComponentValue(result, m_ResultVecComp);
			else
			{
				float v[4];
				const int vecSize = m_pResult->VectorSize();
				for (int i = 0; i < vecSize; ++i)
					v[i] = result;

				m_pResult->SetVecValue(v, vecSize);
			}
		}
		else
		{
			m_pResult->SetFloatValue(result);
		}
	}

	IMaterialVar* m_pResult;
	int m_ResultVecComp;
};

class CFunctionProxy : public CResultProxy
{
public:
	CFunctionProxy() : m_pSrc1(nullptr), m_pSrc2(nullptr) {}
	~CFunctionProxy() override = default;

	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CResultProxy::Init(pMaterial, pKeyValues))
			return false;

		char const* pSrcVar1 = pKeyValues->GetString("srcVar1");
		if (!pSrcVar1)
			return false;

		bool foundVar;
		m_pSrc1 = pMaterial->FindVar(pSrcVar1, &foundVar, true);
		if (!foundVar)
			return false;

		char const* pSrcVar2 = pKeyValues->GetString("srcVar2");
		if (pSrcVar2 && *pSrcVar2)
		{
			m_pSrc2 = pMaterial->FindVar(pSrcVar2, &foundVar, true);
			if (!foundVar)
				return false;
		}
		else
			m_pSrc2 = nullptr;

		return true;
	}

protected:
	void ComputeResultType(MaterialVarType_t& resultType, int& vecSize)
	{
		resultType = m_pResult->GetType();
		if (resultType == MATERIAL_VAR_TYPE_VECTOR)
		{
			if (m_ResultVecComp >= 0)
				resultType = MATERIAL_VAR_TYPE_FLOAT;
			vecSize = m_pResult->VectorSize();
		}
		else if (resultType == MATERIAL_VAR_TYPE_UNDEFINED)
		{
			resultType = m_pSrc1->GetType();
			if (resultType == MATERIAL_VAR_TYPE_VECTOR)
				vecSize = m_pSrc1->VectorSize();
			else if (resultType == MATERIAL_VAR_TYPE_UNDEFINED && m_pSrc2)
			{
				resultType = m_pSrc2->GetType();
				if (resultType == MATERIAL_VAR_TYPE_VECTOR)
					vecSize = m_pSrc2->VectorSize();
			}
		}
	}

	IMaterialVar* m_pSrc1;
	IMaterialVar* m_pSrc2;
};
#pragma endregion

// ------------------------------------------------------------------------------------- //
// Specific material proxies.
// ------------------------------------------------------------------------------------- //

class CWorldDimsProxy : public IMaterialProxy
{
public:
	CWorldDimsProxy() : m_pMinsVar(nullptr), m_pMaxsVar(nullptr) {}
	~CWorldDimsProxy() override = default;

	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		m_pMinsVar = pMaterial->FindVar("$world_mins", nullptr, false);
		m_pMaxsVar = pMaterial->FindVar("$world_maxs", nullptr, false);
		return true;
	}

	void OnBind(void*)
	{
		if (m_pMinsVar && m_pMaxsVar)
		{
			const float mins[3] = { -500, -500, -500 };
			const float maxs[3] = { +500, +500, +500 };
			m_pMinsVar->SetVecValue(mins, 3);
			m_pMaxsVar->SetVecValue(maxs, 3);
		}
	}

	void Release() override { delete this; }

	IMaterial* GetMaterial() override
	{
		if (m_pMinsVar && m_pMaxsVar)
			return m_pMinsVar->GetOwningMaterial();
		return nullptr;
	}

private:
	IMaterialVar* m_pMinsVar;
	IMaterialVar* m_pMaxsVar;
};
REGISTER_PROXY(CWorldDimsProxy, WorldDims);


class CAddProxy : public CFunctionProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		return CFunctionProxy::Init(pMaterial, pKeyValues) && m_pSrc2;
	}

	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pSrc2 && m_pResult);

		MaterialVarType_t resultType;
		int vecSize;
		ComputeResultType(resultType, vecSize);

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D a, b, c;
			m_pSrc1->GetVecValue(a.Base(), vecSize);
			m_pSrc2->GetVecValue(b.Base(), vecSize);
			Vector4DAdd(a, b, c);
			m_pResult->SetVecValue(c.Base(), vecSize);
		}
		break;

		case MATERIAL_VAR_TYPE_FLOAT:
			SetFloatResult(m_pSrc1->GetFloatValue() + m_pSrc2->GetFloatValue());
			break;

		case MATERIAL_VAR_TYPE_INT:
			m_pResult->SetIntValue(m_pSrc1->GetIntValue() + m_pSrc2->GetIntValue());
			break;

		default:
			Assert(0);
		}
	}
};
REGISTER_PROXY(CAddProxy, Add);

class CSubtractProxy : public CFunctionProxy
{
public:
	bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) override
	{
		return CFunctionProxy::Init(pMaterial, pKeyValues) && m_pSrc2;
	}

	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pSrc2 && m_pResult);

		MaterialVarType_t resultType;
		int vecSize;
		ComputeResultType(resultType, vecSize);

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D a, b, c;
			m_pSrc1->GetVecValue(a.Base(), vecSize);
			m_pSrc2->GetVecValue(b.Base(), vecSize);
			Vector4DSubtract(a, b, c);
			m_pResult->SetVecValue(c.Base(), vecSize);
			break;
		}

		case MATERIAL_VAR_TYPE_FLOAT:
			SetFloatResult(m_pSrc1->GetFloatValue() - m_pSrc2->GetFloatValue());
			break;

		case MATERIAL_VAR_TYPE_INT:
			m_pResult->SetIntValue(m_pSrc1->GetIntValue() - m_pSrc2->GetIntValue());
			break;

		default:
			Assert(0);
		}
	}
};
REGISTER_PROXY(CSubtractProxy, Subtract);

class CMultiplyProxy : public CFunctionProxy
{
public:
	bool Init(IMaterial *pMaterial, KeyValues *pKeyValues) override
	{
		return CFunctionProxy::Init(pMaterial, pKeyValues) && m_pSrc2;
	}

	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pSrc2 && m_pResult);

		MaterialVarType_t resultType;
		int vecSize;
		ComputeResultType(resultType, vecSize);

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D a, b, c;
			m_pSrc1->GetVecValue(a.Base(), vecSize);
			m_pSrc2->GetVecValue(b.Base(), vecSize);
			Vector4DMultiply(a, b, c);
			m_pResult->SetVecValue(c.Base(), vecSize);
			break;
		}

		case MATERIAL_VAR_TYPE_FLOAT:
			SetFloatResult(m_pSrc1->GetFloatValue() * m_pSrc2->GetFloatValue());
			break;

		case MATERIAL_VAR_TYPE_INT:
			m_pResult->SetIntValue(m_pSrc1->GetIntValue() * m_pSrc2->GetIntValue());
			break;

		default:
			Assert(0);
		}
	}
};
REGISTER_PROXY(CMultiplyProxy, Multiply);

class CDivideProxy : public CFunctionProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		return CFunctionProxy::Init(pMaterial, pKeyValues) && m_pSrc2;
	}

	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pSrc2 && m_pResult);

		MaterialVarType_t resultType;
		int vecSize;
		ComputeResultType(resultType, vecSize);

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D a, b, c;
			m_pSrc1->GetVecValue(a.Base(), vecSize);
			m_pSrc2->GetVecValue(b.Base(), vecSize);
			Vector4DDivide(a, b, c);
			m_pResult->SetVecValue(c.Base(), vecSize);
			break;
		}

		case MATERIAL_VAR_TYPE_FLOAT:
			if (m_pSrc2->GetFloatValue() != 0)
				SetFloatResult(m_pSrc1->GetFloatValue() / m_pSrc2->GetFloatValue());
			else
				SetFloatResult(m_pSrc1->GetFloatValue());
			break;

		case MATERIAL_VAR_TYPE_INT:
			if (m_pSrc2->GetIntValue() != 0)
				m_pResult->SetIntValue(m_pSrc1->GetIntValue() / m_pSrc2->GetIntValue());
			else
				m_pResult->SetIntValue(m_pSrc1->GetIntValue());
			break;

		default:
			Assert(0);
		}
	}
};
REGISTER_PROXY(CDivideProxy, Divide);

class CClampProxy : public CFunctionProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CFunctionProxy::Init(pMaterial, pKeyValues))
			return false;

		if (!m_Min.Init(pMaterial, pKeyValues, "min", 0))
			return false;

		if (!m_Max.Init(pMaterial, pKeyValues, "max", 1))
			return false;

		return true;
	}

	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pResult);

		MaterialVarType_t resultType;
		int vecSize;
		ComputeResultType(resultType, vecSize);

		float flMin = m_Min.GetFloat();
		float flMax = m_Max.GetFloat();

		if (flMin > flMax)
			V_swap(flMin, flMax);

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D a;
			m_pSrc1->GetVecValue(a.Base(), vecSize);
			for (int i = 0; i < vecSize; ++i)
			{
				if (a[i] < flMin)
					a[i] = flMin;
				else if (a[i] > flMax)
					a[i] = flMax;
			}
			m_pResult->SetVecValue(a.Base(), vecSize);
			break;
		}

		case MATERIAL_VAR_TYPE_FLOAT:
		{
			float src = m_pSrc1->GetFloatValue();
			if (src < flMin)
				src = flMin;
			else if (src > flMax)
				src = flMax;
			SetFloatResult(src);
			break;
		}

		case MATERIAL_VAR_TYPE_INT:
		{
			int src = m_pSrc1->GetIntValue();
			if (src < flMin)
				src = flMin;
			else if (src > flMax)
				src = flMax;
			m_pResult->SetIntValue(src);
			break;
		}
		default:
			Assert(0);
		}
	}

private:
	CFloatInput m_Min;
	CFloatInput m_Max;
};
REGISTER_PROXY(CClampProxy, Clamp);

class CSineProxy : public CResultProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CResultProxy::Init(pMaterial, pKeyValues))
			return false;

		if (!m_SinePeriod.Init(pMaterial, pKeyValues, "sinePeriod", 1.0f))
			return false;
		if (!m_SineMax.Init(pMaterial, pKeyValues, "sineMax", 1.0f))
			return false;
		if (!m_SineMin.Init(pMaterial, pKeyValues, "sineMin", 0.0f))
			return false;
		if (!m_SineTimeOffset.Init(pMaterial, pKeyValues, "timeOffset", 0.0f))
			return false;

		return true;
	}

	void OnBind(void*) override
	{
		Assert(m_pResult);

		const float flSineMin = m_SineMin.GetFloat();
		float flSinePeriod = m_SinePeriod.GetFloat();
		if (flSinePeriod == 0)
			flSinePeriod = 1;

		float flValue = (sin(2.0f * M_PI * (Plat_FloatTime() - m_SineTimeOffset.GetFloat()) / flSinePeriod) * 0.5f) + 0.5f;
		flValue = (m_SineMax.GetFloat() - flSineMin) * flValue + flSineMin;

		SetFloatResult(flValue);
	}

private:
	CFloatInput m_SinePeriod;
	CFloatInput m_SineMax;
	CFloatInput m_SineMin;
	CFloatInput m_SineTimeOffset;
};
REGISTER_PROXY(CSineProxy, Sine);

class CEqualsProxy : public CFunctionProxy
{
public:
	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pResult);

		MaterialVarType_t resultType;
		int vecSize;
		ComputeResultType(resultType, vecSize);

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D a;
			m_pSrc1->GetVecValue(a.Base(), vecSize);
			m_pResult->SetVecValue(a.Base(), vecSize);
			break;
		}

		case MATERIAL_VAR_TYPE_FLOAT:
			SetFloatResult(m_pSrc1->GetFloatValue());
			break;

		case MATERIAL_VAR_TYPE_INT:
			m_pResult->SetIntValue(m_pSrc1->GetIntValue());
			break;
		default:
			Assert(0);
		}
	}
};
REGISTER_PROXY(CEqualsProxy, Equals);

class CFracProxy : public CFunctionProxy
{
public:
	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pResult);

		MaterialVarType_t resultType;
		int vecSize;
		ComputeResultType(resultType, vecSize);

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D a;
			m_pSrc1->GetVecValue(a.Base(), vecSize);
			a[0] -= static_cast<float>(static_cast<int>(a[0]));
			a[1] -= static_cast<float>(static_cast<int>(a[1]));
			a[2] -= static_cast<float>(static_cast<int>(a[2]));
			a[3] -= static_cast<float>(static_cast<int>(a[3]));
			m_pResult->SetVecValue(a.Base(), vecSize);
			break;
		}

		case MATERIAL_VAR_TYPE_FLOAT:
		{
			float a = m_pSrc1->GetFloatValue();
			a -= static_cast<int>(a);
			SetFloatResult(a);
			break;
		}

		case MATERIAL_VAR_TYPE_INT:
			m_pResult->SetIntValue(m_pSrc1->GetIntValue());
			break;

		default:
			Assert(0);
		}
	}
};
REGISTER_PROXY(CFracProxy, Frac);

class CIntProxy : public CFunctionProxy
{
public:
	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pResult);

		MaterialVarType_t resultType;
		int vecSize;
		ComputeResultType(resultType, vecSize);

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D a;
			m_pSrc1->GetVecValue(a.Base(), vecSize);
			a[0] = static_cast<float>(static_cast<int>(a[0]));
			a[1] = static_cast<float>(static_cast<int>(a[1]));
			a[2] = static_cast<float>(static_cast<int>(a[2]));
			a[3] = static_cast<float>(static_cast<int>(a[3]));
			m_pResult->SetVecValue(a.Base(), vecSize);
			break;
		}

		case MATERIAL_VAR_TYPE_FLOAT:
		{
			const float a = static_cast<float>(static_cast<int>(m_pSrc1->GetFloatValue()));
			SetFloatResult(a);
			break;
		}

		case MATERIAL_VAR_TYPE_INT:
			// don't do anything besides assignment!
			m_pResult->SetIntValue(m_pSrc1->GetIntValue());
			break;

		default:
			Assert(0);
		}
	}
};
REGISTER_PROXY(CIntProxy, Int);

class CLinearRampProxy : public CResultProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CResultProxy::Init(pMaterial, pKeyValues))
			return false;

		if (!m_Rate.Init(pMaterial, pKeyValues, "rate", 1))
			return false;

		if (!m_InitialValue.Init(pMaterial, pKeyValues, "initialValue", 0))
			return false;

		return true;
	}

	void OnBind(void*) override
	{
		Assert(m_pResult);

		// get a value in [0,1]
		const float flValue = m_Rate.GetFloat() * Plat_FloatTime() + m_InitialValue.GetFloat();
		SetFloatResult(flValue);
	}

private:
	CFloatInput m_Rate;
	CFloatInput m_InitialValue;
};
REGISTER_PROXY(CLinearRampProxy, LinearRamp);

static CUniformRandomStream random;
class CUniformNoiseProxy : public CResultProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CResultProxy::Init(pMaterial, pKeyValues))
			return false;

		if (!m_flMinVal.Init(pMaterial, pKeyValues, "minVal", 0))
			return false;

		if (!m_flMaxVal.Init(pMaterial, pKeyValues, "maxVal", 1))
			return false;

		static bool first = true;
		if (first)
		{
			random.SetSeed(time(nullptr) % 32768);
			first = false;
		}

		return true;
	}

	void OnBind(void*) override
	{
		SetFloatResult(random.RandomFloat(m_flMinVal.GetFloat(), m_flMaxVal.GetFloat()));
	}

private:
	CFloatInput	m_flMinVal;
	CFloatInput	m_flMaxVal;
};
REGISTER_PROXY(CUniformNoiseProxy, UniformNoise);

static CGaussianRandomStream randomgaussian;
static CUniformRandomStream randomgaussianbase;
class CGaussianNoiseProxy : public CResultProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CResultProxy::Init(pMaterial, pKeyValues))
			return false;

		if (!m_Mean.Init(pMaterial, pKeyValues, "mean", 0.0f))
			return false;

		if (!m_StdDev.Init(pMaterial, pKeyValues, "halfwidth", 1.0f))
			return false;

		if (!m_flMinVal.Init(pMaterial, pKeyValues, "minVal", -FLT_MAX))
			return false;

		if (!m_flMaxVal.Init(pMaterial, pKeyValues, "maxVal", FLT_MAX))
			return false;

		static bool first = true;
		if (first)
		{
			randomgaussianbase.SetSeed(time(nullptr) % 32768);
			randomgaussian.AttachToStream(&randomgaussianbase);
			first = false;
		}

		return true;
	}

	void OnBind(void*) override
	{
		float flVal = randomgaussian.RandomFloat(m_Mean.GetFloat(), m_StdDev.GetFloat());
		float flMaxVal = m_flMaxVal.GetFloat();
		float flMinVal = m_flMinVal.GetFloat();

		if (flMinVal > flMaxVal)
			V_swap(flMinVal, flMaxVal);

		if (flVal < flMinVal)
			flVal = flMinVal;
		else if (flVal > flMaxVal)
			flVal = flMaxVal;

		SetFloatResult(flVal);
	}

private:
	CFloatInput m_Mean;
	CFloatInput m_StdDev;
	CFloatInput	m_flMinVal;
	CFloatInput	m_flMaxVal;
};
REGISTER_PROXY(CGaussianNoiseProxy, GaussianNoise);

class CExponentialProxy : public CFunctionProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CFunctionProxy::Init(pMaterial, pKeyValues))
			return false;

		if (!m_Scale.Init(pMaterial, pKeyValues, "scale", 1.0f))
			return false;

		if (!m_Offset.Init(pMaterial, pKeyValues, "offset", 0.0f))
			return false;

		if (!m_flMinVal.Init(pMaterial, pKeyValues, "minVal", -FLT_MAX))
			return false;

		if (!m_flMaxVal.Init(pMaterial, pKeyValues, "maxVal", FLT_MAX))
			return false;

		return true;
	}

	void OnBind(void*) override
	{
		float flVal = m_Scale.GetFloat() * exp(m_pSrc1->GetFloatValue() + m_Offset.GetFloat());

		float flMaxVal = m_flMaxVal.GetFloat();
		float flMinVal = m_flMinVal.GetFloat();

		if (flMinVal > flMaxVal)
			V_swap(flMinVal, flMaxVal);

		if (flVal < flMinVal)
			flVal = flMinVal;
		else if (flVal > flMaxVal)
			flVal = flMaxVal;

		SetFloatResult(flVal);
	}

private:
	CFloatInput	m_Scale;
	CFloatInput	m_Offset;
	CFloatInput	m_flMinVal;
	CFloatInput	m_flMaxVal;
};
REGISTER_PROXY(CExponentialProxy, Exponential);

class CAbsProxy : public CFunctionProxy
{
public:
	void OnBind(void*) override
	{
		SetFloatResult(fabs(m_pSrc1->GetFloatValue()));
	}
};
REGISTER_PROXY(CAbsProxy, Abs);

class CEmptyProxy : public IMaterialProxy
{
public:
	CEmptyProxy() = default;
	~CEmptyProxy() override = default;
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override { return true; }
	void OnBind(void*) override {}
	void Release() override { delete this; }
	IMaterial* GetMaterial() { return nullptr; }
};
REGISTER_PROXY(CEmptyProxy, Empty);
REGISTER_PROXY(CEmptyProxy, Dummy);

class CLessOrEqualProxy : public CFunctionProxy
{
public:
	CLessOrEqualProxy() : m_pLessVar(nullptr), m_pGreaterVar(nullptr) {}
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		char const* pLessEqualVar = pKeyValues->GetString("lessEqualVar");
		if (!pLessEqualVar)
			return false;

		bool foundVar;
		m_pLessVar = pMaterial->FindVar(pLessEqualVar, &foundVar, true);
		if (!foundVar)
			return false;

		char const* pGreaterVar = pKeyValues->GetString("greaterVar");
		if (!pGreaterVar)
			return false;

		m_pGreaterVar = pMaterial->FindVar(pGreaterVar, &foundVar, true);
		if (!foundVar)
			return false;

		return CFunctionProxy::Init(pMaterial, pKeyValues) && m_pSrc2;
	}

	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pSrc2 && m_pLessVar && m_pGreaterVar && m_pResult);

		IMaterialVar* pSourceVar;
		if (m_pSrc1->GetFloatValue() <= m_pSrc2->GetFloatValue())
			pSourceVar = m_pLessVar;
		else
			pSourceVar = m_pGreaterVar;

		int vecSize = 0;
		MaterialVarType_t resultType = m_pResult->GetType();
		if (resultType == MATERIAL_VAR_TYPE_VECTOR)
		{
			if (m_ResultVecComp >= 0)
				resultType = MATERIAL_VAR_TYPE_FLOAT;
			vecSize = m_pResult->VectorSize();
		}
		else if (resultType == MATERIAL_VAR_TYPE_UNDEFINED)
		{
			resultType = pSourceVar->GetType();
			if (resultType == MATERIAL_VAR_TYPE_VECTOR)
				vecSize = pSourceVar->VectorSize();
		}

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D src;
			pSourceVar->GetVecValue(src.Base(), vecSize);
			m_pResult->SetVecValue(src.Base(), vecSize);
			break;
		}

		case MATERIAL_VAR_TYPE_FLOAT:
			SetFloatResult(pSourceVar->GetFloatValue());
			break;

		case MATERIAL_VAR_TYPE_INT:
			m_pResult->SetIntValue(pSourceVar->GetIntValue());
			break;
		default:
			Assert(0);
		}
	}

private:
	IMaterialVar *m_pLessVar;
	IMaterialVar *m_pGreaterVar;
};
REGISTER_PROXY(CLessOrEqualProxy, LessOrEqual);

class CWrapMinMaxProxy : public CFunctionProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CFunctionProxy::Init(pMaterial, pKeyValues))
			return false;

		if (!m_flMinVal.Init(pMaterial, pKeyValues, "minVal", 0))
			return false;

		if (!m_flMaxVal.Init(pMaterial, pKeyValues, "maxVal", 1))
			return false;

		return true;
	}

	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pResult);

		if (m_flMaxVal.GetFloat() <= m_flMinVal.GetFloat()) // Bad input, just return the min
			SetFloatResult(m_flMinVal.GetFloat());
		else
		{
			float flResult = (m_pSrc1->GetFloatValue() - m_flMinVal.GetFloat()) / (m_flMaxVal.GetFloat() - m_flMinVal.GetFloat());

			if (flResult >= 0.0f)
				flResult -= static_cast<float>(static_cast<int>(flResult));
			else // Negative
				flResult -= static_cast<float>(static_cast<int>(flResult) - 1);

			flResult *= m_flMaxVal.GetFloat() - m_flMinVal.GetFloat();
			flResult += m_flMinVal.GetFloat();

			SetFloatResult(flResult);
		}
	}

private:
	CFloatInput	m_flMinVal;
	CFloatInput	m_flMaxVal;
};
REGISTER_PROXY(CWrapMinMaxProxy, WrapMinMax);

class CSelectFirstIfNonZeroProxy : public CFunctionProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		return CFunctionProxy::Init(pMaterial, pKeyValues) && m_pSrc2;
	}

	void OnBind(void*) override
	{
		Assert(m_pSrc1 && m_pSrc2 && m_pResult);

		MaterialVarType_t resultType;
		int vecSize;
		ComputeResultType(resultType, vecSize);

		switch (resultType)
		{
		case MATERIAL_VAR_TYPE_VECTOR:
		{
			Vector4D a(0, 0, 0, 0), b(0, 0, 0, 0);
			m_pSrc1->GetVecValue(a.Base(), vecSize);
			m_pSrc2->GetVecValue(b.Base(), vecSize);

			if (!a.IsZero())
				m_pResult->SetVecValue(a.Base(), vecSize);
			else
				m_pResult->SetVecValue(b.Base(), vecSize);
			break;
		}

		case MATERIAL_VAR_TYPE_FLOAT:
			if (m_pSrc1->GetFloatValue())
				SetFloatResult(m_pSrc1->GetFloatValue());
			else
				SetFloatResult(m_pSrc2->GetFloatValue());
			break;

		case MATERIAL_VAR_TYPE_INT:
			if (m_pSrc1->GetIntValue())
				m_pResult->SetFloatValue(m_pSrc1->GetIntValue());
			else
				m_pResult->SetFloatValue(m_pSrc2->GetIntValue());
			break;
		default:
			Assert(0);
		}
	}
};
REGISTER_PROXY(CSelectFirstIfNonZeroProxy, SelectFirstIfNonZero);

class CTextureTransformProxy : public CResultProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		// All are optional...
		m_pCenterVar = nullptr;
		m_pScaleVar = nullptr;
		m_pRotateVar = nullptr;
		m_pTranslateVar = nullptr;

		bool bFoundVar;
		char const* pVarName = pKeyValues->GetString("centerVar");
		if (pVarName && pVarName[0])
			m_pCenterVar = pMaterial->FindVar(pVarName, &bFoundVar, false);

		pVarName = pKeyValues->GetString("scaleVar");
		if (pVarName && pVarName[0])
			m_pScaleVar = pMaterial->FindVar(pVarName, &bFoundVar, false);

		pVarName = pKeyValues->GetString("rotateVar");
		if (pVarName && pVarName[0])
			m_pRotateVar = pMaterial->FindVar(pVarName, &bFoundVar, false);

		pVarName = pKeyValues->GetString("translateVar");
		if (pVarName && pVarName[0])
			m_pTranslateVar = pMaterial->FindVar(pVarName, &bFoundVar, false);

		return CResultProxy::Init(pMaterial, pKeyValues);
	}

	void OnBind(void*) override
	{
		Vector2D center(0.5, 0.5);
		Vector2D translation(0, 0);

		VMatrix mat, temp;

		if (m_pCenterVar)
		{
			m_pCenterVar->GetVecValue(center.Base(), 2);
		}
		MatrixBuildTranslation(mat, -center.x, -center.y, 0.0f);

		if (m_pScaleVar)
		{
			Vector2D scale;
			m_pScaleVar->GetVecValue(scale.Base(), 2);
			MatrixBuildScale(temp, scale.x, scale.y, 1.0f);
			MatrixMultiply(temp, mat, mat);
		}

		if (m_pRotateVar)
		{
			float angle = m_pRotateVar->GetFloatValue();
			MatrixBuildRotateZ(temp, angle);
			MatrixMultiply(temp, mat, mat);
		}
		MatrixBuildTranslation(temp, center.x, center.y, 0.0f);
		MatrixMultiply(temp, mat, mat);

		if (m_pTranslateVar)
		{
			m_pTranslateVar->GetVecValue(translation.Base(), 2);
			MatrixBuildTranslation(temp, translation.x, translation.y, 0.0f);
			MatrixMultiply(temp, mat, mat);
		}

		m_pResult->SetMatrixValue(mat);
	}

private:
	IMaterialVar* m_pCenterVar = nullptr;
	IMaterialVar* m_pScaleVar = nullptr;
	IMaterialVar* m_pRotateVar = nullptr;
	IMaterialVar* m_pTranslateVar = nullptr;
};
REGISTER_PROXY(CTextureTransformProxy, TextureTransform);

class CMatrixRotateProxy : public CResultProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		// All are optional...
		m_pAxisVar = nullptr;

		bool bFoundVar;
		char const* pVarName = pKeyValues->GetString("axisVar");
		if (pVarName && pVarName[0])
		{
			m_pAxisVar = pMaterial->FindVar(pVarName, &bFoundVar, false);
		}

		if (!m_Angle.Init(pMaterial, pKeyValues, "angle", 0))
			return false;

		return CResultProxy::Init(pMaterial, pKeyValues);
	}

	void OnBind(void*) override
	{
		VMatrix mat;
		Vector axis(0, 0, 1);
		if (m_pAxisVar)
		{
			m_pAxisVar->GetVecValue(axis.Base(), 3);
			if (VectorNormalize(axis) < 1e-3)
				axis.Init(0, 0, 1);
		}

		MatrixBuildRotationAboutAxis(mat, axis, m_Angle.GetFloat());
		m_pResult->SetMatrixValue(mat);
	}

private:
	CFloatInput	m_Angle;
	IMaterialVar* m_pAxisVar = nullptr;
};
REGISTER_PROXY(CMatrixRotateProxy, MatrixRotate);

class CTimeMaterialProxy : public CResultProxy
{
public:
	void OnBind(void*) override
	{
		SetFloatResult(Plat_FloatTime());
	}
};
REGISTER_PROXY(CTimeMaterialProxy, CurrentTime);

class CTextureScrollMaterialProxy : public IMaterialProxy
{
public:
	CTextureScrollMaterialProxy()
	{
		m_pTextureScrollVar = nullptr;
	}

	~CTextureScrollMaterialProxy() override = default;

	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		char const* pScrollVarName = pKeyValues->GetString("textureScrollVar");
		if (!pScrollVarName)
			return false;

		bool foundVar;
		m_pTextureScrollVar = pMaterial->FindVar(pScrollVarName, &foundVar, false);
		if (!foundVar)
			return false;

		m_TextureScrollRate.Init(pMaterial, pKeyValues, "textureScrollRate", 1.0f);
		m_TextureScrollAngle.Init(pMaterial, pKeyValues, "textureScrollAngle", 0.0f);
		m_TextureScale.Init(pMaterial, pKeyValues, "textureScale", 1.0f);

		return true;
	}

	void OnBind(void*) override
	{
		if (!m_pTextureScrollVar)
			return;

		// set default values if these variables don't exist.
		const float rate = m_TextureScrollRate.GetFloat();
		const float angle = m_TextureScrollAngle.GetFloat();
		const float scale = m_TextureScale.GetFloat();

		float sOffset = Plat_FloatTime() * cos(angle * (M_PI / 180.0f)) * rate;
		float tOffset = Plat_FloatTime() * sin(angle * (M_PI / 180.0f)) * rate;

		// make sure that we are positive
		if (sOffset < 0.0f)
			sOffset += 1.0f - static_cast<int>(sOffset);
		if (tOffset < 0.0f)
			tOffset += 1.0f - static_cast<int>(tOffset);

		// make sure that we are in a [0,1] range
		sOffset = sOffset - static_cast<int>(sOffset);
		tOffset = tOffset - static_cast<int>(tOffset);

		if (m_pTextureScrollVar->GetType() == MATERIAL_VAR_TYPE_MATRIX)
		{
			const VMatrix mat(scale, 0.0f, 0.0f, sOffset,
				0.0f, scale, 0.0f, tOffset,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			m_pTextureScrollVar->SetMatrixValue(mat);
		}
		else
			m_pTextureScrollVar->SetVecValue(sOffset, tOffset, 0.0f);
	}

	void Release() override { delete this; }
	IMaterial* GetMaterial() override
	{
		return m_pTextureScrollVar->GetOwningMaterial();
	}

private:
	IMaterialVar* m_pTextureScrollVar;
	CFloatInput m_TextureScrollRate;
	CFloatInput m_TextureScrollAngle;
	CFloatInput m_TextureScale;
};
REGISTER_PROXY(CTextureScrollMaterialProxy, TextureScroll);

class CShieldProxy : public IMaterialProxy
{
public:
	CShieldProxy()
	{
		m_AlphaVar = nullptr;
		m_pTextureScrollVar = nullptr;
		m_ScrollRate = 0;
		m_ScrollAngle = 0;
	}

	~CShieldProxy() override = default;

	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		bool foundVar;

		m_AlphaVar = pMaterial->FindVar("$translucency", &foundVar, false);
		if (!foundVar)
			return false;

		char const* pScrollVarName = pKeyValues->GetString("textureScrollVar");
		if (!pScrollVarName)
			return false;

		m_pTextureScrollVar = pMaterial->FindVar(pScrollVarName, &foundVar, false);
		if (!foundVar)
			return false;

		m_ScrollRate = pKeyValues->GetFloat("textureScrollRate", 1);
		m_ScrollAngle = pKeyValues->GetFloat("textureScrollAngle", 0);
		return true;
	}

	void OnBind(void*) override
	{
		if (m_AlphaVar)
			m_AlphaVar->SetFloatValue(1.f);

		if (!m_pTextureScrollVar)
			return;

		float sOffset = Plat_FloatTime() * sin(m_ScrollAngle * (M_PI / 180.0f)) * m_ScrollRate;
		float tOffset = Plat_FloatTime() * cos(m_ScrollAngle * (M_PI / 180.0f)) * m_ScrollRate;

		// make sure that we are positive
		if (sOffset < 0.0f)
			sOffset += 1.0f - static_cast<int>(sOffset);
		if (tOffset < 0.0f)
			tOffset += 1.0f - static_cast<int>(tOffset);

		// make sure that we are in a [0,1] range
		sOffset = sOffset - static_cast<int>(sOffset);
		tOffset = tOffset - static_cast<int>(tOffset);

		m_pTextureScrollVar->SetVecValue(sOffset, tOffset, 0.0f);
	}

	IMaterial* GetMaterial() override
	{
		return m_AlphaVar->GetOwningMaterial();
	}

	void Release() override { delete this; }

private:
	IMaterialVar* m_AlphaVar;
	IMaterialVar* m_pTextureScrollVar;
	float m_ScrollRate;
	float m_ScrollAngle;
};
REGISTER_PROXY(CShieldProxy, Shield);

class CBaseAnimatedTextureProxy : public IMaterialProxy
{
public:
	CBaseAnimatedTextureProxy()
	{
		Cleanup();
	}

	~CBaseAnimatedTextureProxy() override
	{
		Cleanup();
	}

	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (char const* pAnimatedTextureVarName = pKeyValues->GetString("animatedTextureVar"))
		{
			bool foundVar;
			m_AnimatedTextureVar = pMaterial->FindVar(pAnimatedTextureVarName, &foundVar, false);
			if (foundVar)
			{
				if (char const* pAnimatedTextureFrameNumVarName = pKeyValues->GetString("animatedTextureFrameNumVar"))
				{
					m_AnimatedTextureFrameNumVar = pMaterial->FindVar(pAnimatedTextureFrameNumVarName, &foundVar, false);
					if (foundVar)
					{
						m_FrameRate = pKeyValues->GetFloat("animatedTextureFrameRate", 15);
						m_WrapAnimation = pKeyValues->GetBool("animationNoWrap");
						return true;
					}
				}
			}
		}

		// Error - null out pointers.
		Cleanup();
		return false;
	}

	void OnBind(void* pEntity) override
	{
		Assert(m_AnimatedTextureVar);

		if (m_AnimatedTextureVar->GetType() != MATERIAL_VAR_TYPE_TEXTURE)
		{
			return;
		}
		ITexture *pTexture;
		pTexture = m_AnimatedTextureVar->GetTextureValue();
		int numFrames = pTexture->GetNumAnimationFrames();

		if (numFrames <= 0)
		{
			Assert(!"0 frames in material calling animated texture proxy");
			return;
		}

		// NOTE: Must not use relative time based methods here
		// because the bind proxy can be called many times per frame.
		// Prevent multiple Wrap callbacks to be sent for no wrap mode
		float startTime = GetAnimationStartTime(pEntity);
		float deltaTime = Plat_FloatTime() - startTime;
		float prevTime = deltaTime - frameTime;

		// Clamp..
		if (deltaTime < 0.0f)
			deltaTime = 0.0f;
		if (prevTime < 0.0f)
			prevTime = 0.0f;

		float frame = m_FrameRate * deltaTime;
		float prevFrame = m_FrameRate * prevTime;

		int intFrame = ((int)frame) % numFrames;
		int intPrevFrame = ((int)prevFrame) % numFrames;

		// Report wrap situation...
		if (intPrevFrame > intFrame)
		{
			if (m_WrapAnimation)
				AnimationWrapped(pEntity);
			else
			{
				// Only sent the wrapped message once.
				// when we're in non-wrapping mode
				if (prevFrame < numFrames)
					AnimationWrapped(pEntity);
				intFrame = numFrames - 1;
			}
		}

		m_AnimatedTextureFrameNumVar->SetIntValue(intFrame);
	}

	void Release() override
	{
		delete this;
	}

	IMaterial* GetMaterial() override
	{
		return m_AnimatedTextureVar->GetOwningMaterial();
	}

protected:
	// derived classes must implement this; it returns the time
	// that the animation began
	virtual float  GetAnimationStartTime(void* pBaseEntity) = 0;

	// Derived classes may implement this if they choose;
	// this method is called whenever the animation wraps...
	virtual void   AnimationWrapped(void* pBaseEntity) {}

protected:
	void Cleanup()
	{
		m_AnimatedTextureVar = NULL;
		m_AnimatedTextureFrameNumVar = NULL;
	}

	IMaterialVar *m_AnimatedTextureVar;
	IMaterialVar *m_AnimatedTextureFrameNumVar;
	float m_FrameRate;
	bool m_WrapAnimation;
};
class CAnimatedTextureProxy : public CBaseAnimatedTextureProxy
{
public:
	CAnimatedTextureProxy() {}
	~CAnimatedTextureProxy() override = default;
	float GetAnimationStartTime(void* pBaseEntity) override { return 0.f; }
};
REGISTER_PROXY(CAnimatedTextureProxy, AnimatedTexture);
REGISTER_PROXY(CAnimatedTextureProxy, AnimatedOffsetTexture);

#if 0
class CAnimateSpecificTexture : public CBaseAnimatedTextureProxy
{
private:
	CUtlString m_OnlyAnimateOnTexture;
public:
	float GetAnimationStartTime(void* pBaseEntity) override { return 0; }
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		char const* pszAnimateOnTexture = pKeyValues->GetString("onlyAnimateOnTexture");
		if (!pszAnimateOnTexture)
			return false;

		m_OnlyAnimateOnTexture.Set(pszAnimateOnTexture);

		return CBaseAnimatedTextureProxy::Init(pMaterial, pKeyValues);
	}

	void OnBind(void* pEntity) override
	{
		if (m_OnlyAnimateOnTexture.IsEqual_CaseInsensitive(m_AnimatedTextureVar->GetTextureValue()->GetName()))
			CBaseAnimatedTextureProxy::OnBind(pEntity);
	}

	void Release() override
	{
		delete this;
	}
};
REGISTER_PROXY(CAnimateSpecificTexture, AnimateSpecificTexture);
#endif
abstract_class CEntityMaterialProxy : public IMaterialProxy
{
public:
	void Release() override
	{
		delete this;
	}

	void OnBind(void* pEntity) override
	{
		if (!pEntity)
			return;

		OnBind(static_cast<CRender::ProxyData*>(pEntity));
	}

protected:
	// base classes should implement these
	virtual void OnBind(CRender::ProxyData* pEntity) = 0;
};

class CEntityOriginMaterialProxy : public CEntityMaterialProxy
{
public:
	CEntityOriginMaterialProxy()
	{
		m_pMaterial = NULL;
		m_pOriginVar = NULL;
	}
	~CEntityOriginMaterialProxy() override = default;

	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		m_pMaterial = pMaterial;
		bool found;
		m_pOriginVar = m_pMaterial->FindVar("$entityorigin", &found);
		if (!found)
		{
			m_pOriginVar = NULL;
			return false;
		}
		return true;
	}

	void OnBind(CRender::ProxyData* pEntity) override
	{
		if (!pEntity->pClass)
			return;
		Vector origin;
		pEntity->pClass->GetOrigin(origin);
		m_pOriginVar->SetVecValue(origin.x, origin.y, origin.z);
	}

	IMaterial *GetMaterial() override
	{
		return m_pMaterial;
	}

protected:
	IMaterial *m_pMaterial;
	IMaterialVar *m_pOriginVar;
};
REGISTER_PROXY(CEntityOriginMaterialProxy, EntityOrigin);

class CPlayerProximityProxy : public CResultProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CResultProxy::Init(pMaterial, pKeyValues))
			return false;

		m_Factor = pKeyValues->GetFloat("scale", 0.002f);
		return true;
	}

	void OnBind(void* pEntity) override
	{
		if (!pEntity)
			return;

		// Find the distance between the player and this entity....
		auto entity = static_cast<CRender::ProxyData*>(pEntity);
		if (!entity->pClass)
			return;
		Vector camOrigin, entOrigin;
		entity->pClass->GetOrigin(entOrigin);
		entity->pRender->GetCamera()->GetViewPoint(camOrigin);

		Vector delta;
		VectorSubtract(entOrigin, camOrigin, delta);

		Assert(m_pResult);
		SetFloatResult(delta.Length() * m_Factor);
	}

private:
	float	m_Factor;
};
REGISTER_PROXY(CPlayerProximityProxy, PlayerProximity);

class CPlayerViewProxy : public CResultProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CResultProxy::Init(pMaterial, pKeyValues))
			return false;

		m_Factor = pKeyValues->GetFloat("scale", 2.0f);
		return true;
	}

	void OnBind(void* pEntity) override
	{
		if (!pEntity)
			return;

		// Find the view angle between the player and this entity....
		auto entity = static_cast<CRender::ProxyData*>(pEntity);
		if (!entity->pClass)
			return;
		Vector camOrigin, entOrigin, forward;
		entity->pClass->GetOrigin(entOrigin);
		auto cam = entity->pRender->GetCamera();
		cam->GetViewPoint(camOrigin);
		cam->GetViewForward(forward);

		Vector delta;
		VectorSubtract(entOrigin, camOrigin, delta);
		VectorNormalize(delta);

		Assert(m_pResult);
		SetFloatResult(DotProduct(forward, delta) * m_Factor);
	}

private:
	float	m_Factor;
};
REGISTER_PROXY(CPlayerViewProxy, PlayerView);

class CPlayerPositionProxy : public CResultProxy
{
public:
	bool Init(IMaterial* pMaterial, KeyValues* pKeyValues) override
	{
		if (!CResultProxy::Init(pMaterial, pKeyValues))
			return false;

		m_Factor = pKeyValues->GetFloat("scale", 0.005f);
		return true;
	}

	void OnBind(void* pEntity) override
	{
		if (!pEntity)
			return;

		// Find the view pos....
		auto entity = static_cast<CRender::ProxyData*>(pEntity);
		if (!entity->pClass)
			return;
		Vector camOrigin;
		auto cam = entity->pRender->GetCamera();
		cam->GetViewPoint(camOrigin);

		Assert(m_pResult);
		Vector res;
		VectorMultiply(camOrigin, m_Factor, res);
		m_pResult->SetVecValue(res.Base(), 3);
	}

private:
	float	m_Factor;
};
REGISTER_PROXY(CPlayerPositionProxy, PlayerPosition);
#else
// ------------------------------------------------------------------------------------- //
// The material proxy factory for WC.
// ------------------------------------------------------------------------------------- //

class CMaterialProxyFactory : public IMaterialProxyFactory
{
public:
	IMaterialProxy *CreateProxy( const char *proxyName );
	void DeleteProxy( IMaterialProxy *pProxy );
};
CMaterialProxyFactory g_MaterialProxyFactory;


IMaterialProxy *CMaterialProxyFactory::CreateProxy( const char *proxyName )
{
	// assumes that the client.dll is already LoadLibraried
	CreateInterfaceFn clientFactory = Sys_GetFactoryThis();

	// allocate exactly enough memory for the versioned name on the stack.
	char proxyVersionedName[512];
	Q_snprintf( proxyVersionedName, sizeof( proxyVersionedName ), "%s%s", proxyName, IMATERIAL_PROXY_INTERFACE_VERSION );
	return ( IMaterialProxy * )clientFactory( proxyVersionedName, NULL );
}

void CMaterialProxyFactory::DeleteProxy( IMaterialProxy *pProxy )
{
	if ( pProxy )
	{
		pProxy->Release();
	}
}


IMaterialProxyFactory* GetHammerMaterialProxyFactory()
{
	return &g_MaterialProxyFactory;
}



// ------------------------------------------------------------------------------------- //
// Specific material proxies.
// ------------------------------------------------------------------------------------- //

class CWorldDimsProxy : public IMaterialProxy
{
public:
	CWorldDimsProxy();
	virtual ~CWorldDimsProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
	virtual void Release( void ) { delete this; }
	virtual IMaterial *GetMaterial();

public:
	IMaterialVar *m_pMinsVar;
	IMaterialVar *m_pMaxsVar;
};


CWorldDimsProxy::CWorldDimsProxy()
{
	m_pMinsVar = m_pMaxsVar = NULL;
}

CWorldDimsProxy::~CWorldDimsProxy()
{
}

bool CWorldDimsProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	m_pMinsVar = pMaterial->FindVar( "$world_mins", NULL, false );
	m_pMaxsVar = pMaterial->FindVar( "$world_maxs", NULL, false );
	return true;
}

void CWorldDimsProxy::OnBind( void *pC_BaseEntity )
{
	if ( m_pMinsVar && m_pMaxsVar )
	{
		float mins[3] = {-500,-500,-500};
		float maxs[3] = {+500,+500,+500};
		m_pMinsVar->SetVecValue( (const float*)mins, 3 );
		m_pMaxsVar->SetVecValue( (const float*)maxs, 3 );
	}
}

IMaterial *CWorldDimsProxy::GetMaterial()
{
	if ( m_pMinsVar && m_pMaxsVar )
		return m_pMinsVar->GetOwningMaterial();
	return NULL;
}

EXPOSE_INTERFACE( CWorldDimsProxy, IMaterialProxy, "WorldDims" IMATERIAL_PROXY_INTERFACE_VERSION );
#endif
