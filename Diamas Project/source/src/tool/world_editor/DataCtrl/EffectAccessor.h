#ifndef METIN2_TOOL_WORLDEDITOR_DATACTRL_EFFECTACCESSOR_HPP
#define METIN2_TOOL_WORLDEDITOR_DATACTRL_EFFECTACCESSOR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <EffectLib/EffectElementBase.h>
#include <EffectLib/ParticleSystemData.h>
#include <EffectLib/EffectMesh.h>
#include <EffectLib/SimpleLightData.h>
#include <EffectLib/EffectData.h>
#include <EffectLib/EffectInstance.h>

#include <storm/io/File.hpp>

#include <vstl/string.hpp>

METIN2_BEGIN_NS

// Data
class CEffectElementBaseAccessor : public CEffectElementBase
{
	public:
		CEffectElementBaseAccessor(){}
		virtual ~CEffectElementBaseAccessor(){}

		void SaveScript(int iBaseTab, storm::File& File);

		void SetStartTime(float fTime);

		uint32_t GetPositionCount();
		void DeletePosition(uint32_t dwIndex);
		void InsertPosition(float fTime);
		bool GetTimePosition(uint32_t dwIndex, float * pTime);
		void SetTimePosition(uint32_t dwIndex, float fTime);
		bool GetValuePosition(uint32_t dwIndex, TEffectPosition ** ppEffectPosition);
		void SetValuePosition(uint32_t dwIndex, const D3DXVECTOR3 & c_rVector);
		void SetValueControlPoint(uint32_t dwIndex, const D3DXVECTOR3 & c_rVector);
};

class CParticleAccessor : public CParticleSystemData
{
	public:
		CParticleAccessor();
		virtual ~CParticleAccessor();

		void ClearAccessor();
		void SetDefaultData();
		void SaveScript(int iBaseTab, storm::File& File, const char * c_szGlobalPath);

		void InsertTexture(const char * c_szFileName);
		void ClearOneTexture(uint32_t dwIndex);
		void ClearAllTexture();

		// Emitter Property
		uint32_t & GetMaxEmissionCountReference();

		float & GetCycleLengthReference();
		bool & GetCycleLoopFlagReference();
		int & GetLoopCountReference();

		BYTE & GetEmitterShapeReference();
		BYTE & GetEmitterAdvancedTypeReference();
		bool & GetEmitFromEdgeFlagReference();
		D3DXVECTOR3 & GetEmittingSizeReference();
		float & GetEmittingRadiusReference();

		D3DXVECTOR3 & GetEmittingDirectionReference();

		TTimeEventTableFloat * GetEmitterTimeEventTableEmittingSize();
		TTimeEventTableFloat * GetEmitterTimeEventTableEmittingAngularVelocity();
		TTimeEventTableFloat * GetEmitterTimeEventTableEmittingDirectionX();
		TTimeEventTableFloat * GetEmitterTimeEventTableEmittingDirectionY();
		TTimeEventTableFloat * GetEmitterTimeEventTableEmittingDirectionZ();
		TTimeEventTableFloat * GetEmitterTimeEventTableEmittingVelocity();
		TTimeEventTableFloat * GetEmitterTimeEventTableEmissionCount();
		TTimeEventTableFloat * GetEmitterTimeEventTableLifeTime();
		TTimeEventTableFloat * GetEmitterTimeEventTableSizeX();
		TTimeEventTableFloat * GetEmitterTimeEventTableSizeY();

		// Particle Property
		BYTE & GetSrcBlendTypeReference();
		BYTE & GetDestBlendTypeReference();
		BYTE & GetColorOperationTypeReference();
		BYTE & GetBillboardTypeReference();

		BYTE & GetTexAniTypeReference();
		bool & GetTexAniRandomStartFrameFlagReference();
		double & GetTexAniDelayReference();

		bool & GetStretchFlagReference();
		bool & GetAttachFlagReference();

		BYTE & GetRotationTypeReference();
		float & GetRotationSpeedReference();
		WORD & GetRotationRandomStartBeginReference();
		WORD & GetRotationRandomStartEndReference();

		TTimeEventTableDouble * GetParticleTimeEventGravity();
		TTimeEventTableDouble * GetParticleTimeEventAirResistance();

		TTimeEventTableDouble * GetParticleTimeEventScaleX();
		TTimeEventTableDouble * GetParticleTimeEventScaleY();
		TTimeEventTableDouble * GetParticleTimeEventColorRed();
		TTimeEventTableDouble * GetParticleTimeEventColorGreen();
		TTimeEventTableDouble * GetParticleTimeEventColorBlue();
		TTimeEventTableDouble * GetParticleTimeEventAlpha();
		TTimeEventTableDouble * GetParticleTimeEventRotation();
		TTimeEventTableFloat * GetParticleTimeEventStretch();

		uint32_t GetTextureCount();
		bool GetImagePointer(uint32_t dwIndex, CGraphicImage::Ptr* ppImage);
		bool GetTextureName(uint32_t dwIndex, const char ** pszTextureName);
};

class CMeshAccessor : public CEffectMeshScript
{
	public:
		CMeshAccessor();
		virtual ~CMeshAccessor();

		void ClearAccessor();
		void SaveScript(int iBaseTab, storm::File& File, const char * c_szGlobalPath);

		void SetMeshAnimationFlag(bool bFlag);
		void SetMeshAnimationLoopCount(int iNewCount);
		void SetMeshAnimationFrameDelay(float fDelay);
		void SetMeshFileName(const char * c_szFileName);

		void LoadMeshInstance();
		uint32_t GetMeshElementCount();
		bool GetMeshElementDataPointer(uint32_t dwIndex, CEffectMesh::TEffectMeshData ** ppMeshElementData);

	protected:
		CEffectMesh::Ptr m_pEffectMesh;
};

class CLightAccessor : public CLightData
{
	public:
		CLightAccessor();
		virtual ~CLightAccessor();

		void ClearAccessor();
		void SaveScript(int iBaseTab, storm::File& File);

		bool & GetLoopFlagReference();
		int & GetLoopCountReference() { return m_iLoopCount; }

		float & GetMaxRangeReference();
		float & GetDurationReference();

		float & GetAttenuation0Reference();
		float & GetAttenuation1Reference();
		float & GetAttenuation2Reference();

		TTimeEventTableFloat * GetEmitterTimeEventTableRange();

		D3DXCOLOR & GetAmbientColorReference();
		D3DXCOLOR & GetDiffuseColorReference();
		void SetAmbientColor(float fr, float fg, float fb);
		void SetDiffuseColor(float fr, float fg, float fb);
};

class CEffectAccessor : public CEffectData
{
	public:
		enum
		{
			EFFECT_ELEMENT_TYPE_PARTICLE,
			EFFECT_ELEMENT_TYPE_MESH,
			EFFECT_ELEMENT_TYPE_LIGHT,
		};

		typedef struct SEffectElement
		{
			bool bVisible;

			int iType;
			std::string strName;

			CEffectElementBaseAccessor * pBase;

			CParticleAccessor * pParticle;
			CMeshAccessor * pMesh;
			CLightAccessor * pLight;
			TSoundData * pSound;
		} TEffectElement;

		typedef std::vector<TEffectElement> TEffectElementVector;

	public:
		CEffectAccessor() : m_fLifeTime(5.0f){}
		virtual ~CEffectAccessor();

		void					Clear();

		CParticleSystemData *	AllocParticle() override;
		CEffectMeshScript *		AllocMesh() override;
		CLightData *			AllocLight() override;

		void					SetVisible(uint32_t dwIndex, bool bVisible);
		bool					GetVisible(uint32_t dwIndex);

		float					GetLifeTime();
		void					SetLifeTime(float fLifeTime);
		void					SetBoundingSphereRadius(float fRadius);
		void					SetBoundingSpherePosition(const D3DXVECTOR3 & c_rv3Pos);
		uint32_t					GetElementCount();
		bool					GetElement(uint32_t dwIndex, TEffectElement ** ppElement);
		bool					GetElementParticle(uint32_t dwIndex, CParticleAccessor ** ppParticleAccessor);
		void					DeleteElement(uint32_t dwIndex);
		void					SwapElement(uint32_t dwIndex1, uint32_t dwIndex2);

	protected:
		float					m_fLifeTime;
		TEffectElementVector	m_ElementVector;
};

class CEffectInstanceAccessor : public CEffectInstance
{
	public:
		CEffectInstanceAccessor(){}
		virtual ~CEffectInstanceAccessor(){}

		void SetEffectDataAccessorPointer(CEffectAccessor * pAccessor);
};

METIN2_END_NS

#endif
