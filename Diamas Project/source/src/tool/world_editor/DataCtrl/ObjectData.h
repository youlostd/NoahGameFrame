#ifndef METIN2_TOOL_WORLDEDITOR_DATACTRL_OBJECTDATA_HPP
#define METIN2_TOOL_WORLDEDITOR_DATACTRL_OBJECTDATA_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "ObjectAnimationAccessor.h"


METIN2_BEGIN_NS

class CObjectData
{
	public:
		enum
		{
			MODEL_TYPE_CHARACTER,
			MODEL_TYPE_ITEM,
			MODEL_TYPE_OBJECT,
		};

		enum
		{
			LIGHT_MAX_NUM = 3,
		};

		// Texturing
		enum
		{
			TEXTURING_TYPE_COLOR,
			TEXTURING_TYPE_ALPHA,
		};

		enum
		{
			LIGHT_TYPE_POINT		= D3DLIGHT_POINT,
			LIGHT_TYPE_SPOT			= D3DLIGHT_SPOT,
			LIGHT_TYPE_DIRECTIONAL	= D3DLIGHT_DIRECTIONAL,
		};

		enum
		{
			TEXTURING_ARG_CURRENT,
			TEXTURING_ARG_DIFFUSE,
			TEXTURING_ARG_SELECTMASK,
			TEXTURING_ARG_SPECULAR,
			TEXTURING_ARG_TEMP,
			TEXTURING_ARG_TEXTURE,
			TEXTURING_ARG_TFACTOR,
			TEXTURING_ARG_ALPHAREPLICATE,
			TEXTURING_ARG_COMPLEMENT,
		};

		enum
		{
			// +1 To Real D3D Operation Flag
			TEXTURING_OPERATION_DISABLE,
			TEXTURING_OPERATION_SELECTARG1,
			TEXTURING_OPERATION_SELECTARG2,
			TEXTURING_OPERATION_MODULATE,
			TEXTURING_OPERATION_MODULATE2X,
			TEXTURING_OPERATION_MODULATE4X,
			TEXTURING_OPERATION_ADD,
			TEXTURING_OPERATION_ADDSIGNED,
			TEXTURING_OPERATION_ADDSIGNED2X,
			TEXTURING_OPERATION_SUBTRACT,
			TEXTURING_OPERATION_ADDSMOOTH,
			TEXTURING_OPERATION_BLENDDIFFUSEALPHA,
			TEXTURING_OPERATION_BLENDTEXTUREALPHA,
			TEXTURING_OPERATION_BLENDFACTORALPHA,
			TEXTURING_OPERATION_BLENDTEXTUREALPHAPM,
			TEXTURING_OPERATION_BLENDCURRENTALPHA,
			TEXTURING_OPERATION_PREMODULATE,
			TEXTURING_OPERATION_MODULATEALPHA_ADDCOLOR,
			TEXTURING_OPERATION_MODULATECOLOR_ADDALPHA,
			TEXTURING_OPERATION_MODULATEINVALPHA_ADDCOLOR,
			TEXTURING_OPERATION_MODULATEINVCOLOR_ADDALPHA,
			TEXTURING_OPERATION_BUMPENVMAP,
			TEXTURING_OPERATION_BUMPENVMAPLUMINANCE,
			TEXTURING_OPERATION_DOTPRODUCT3,
			TEXTURING_OPERATION_MULTIPLYADD,
			TEXTURING_OPERATION_LERP,
		};

		typedef struct STextureState
		{
			int iOperation;
			int iArgument[2];
		} TTextureState;

		typedef struct SLightElementData
		{
			std::string				strName;
			std::string				strFileName;
			bool					bLightEnable;
			D3DLIGHT9				Light;
		} TLightElementData;

		typedef struct STextureData
		{
			bool					bEnable;
			std::string				strName;
			std::string				strFileName;
			CGraphicImageInstance	Texture;
		} TTextureData;

		typedef struct SLightData
		{
			SLightData()
			{
				Clear();
			}

			void Clear()
			{
				memset(&Material, 0, sizeof(Material));
				bSpecularEnable = false;

				memset(TextureState, 0, sizeof(TextureState));

				TextureData[0].bEnable = false;
				TextureData[0].strName.assign("");
				TextureData[0].strFileName.assign("");
				TextureData[0].Texture.Destroy();

				TextureData[1].bEnable = false;
				TextureData[1].strName.assign("");
				TextureData[1].strFileName.assign("");
				TextureData[1].Texture.Destroy();

				for (int i = 0; i < LIGHT_MAX_NUM; ++i)
				{
					LightElementData[i].strName.assign("");
					LightElementData[i].strFileName.assign("");
					LightElementData[i].bLightEnable = false;
					memset(&LightElementData[i].Light, 0, sizeof(LightElementData[i].Light));
				}
			}

			D3DMATERIAL9			Material;
			bool					bSpecularEnable;
			TTextureState			TextureState[2][2];
			TTextureData			TextureData[2];
			TLightElementData		LightElementData[LIGHT_MAX_NUM];
		} TLightData;

		typedef struct SShapeData
		{
			uint32_t		dwIndex;
			std::string	strModelFileName;			//CFilename	strModelFileName;
			std::string	strSourceSkinFileName;		//CFilename	strSourceSkinFileName;
			std::string	strTargetSkinFileName;		//CFilename	strTargetSkinFileName;
		} TShapeData;

	public:
		CObjectData();
		virtual ~CObjectData();

		void Clear();
		void Initialize();

		///////////////////////////////////////////////////////
		// About Model Data
		void ClearModelData();

		BOOL isModelThing();
		BOOL SetModelThing(const char * c_szFileName);
		BOOL GetModelThing(CGraphicThing::Ptr* ppThing);

		// Public
		int & GetModelTypeReference();
		uint32_t GetAttachingDataCount();
		BOOL GetAttachingDataPointer(uint32_t dwIndex, NRaceData::TAttachingData ** ppAttachingData);
		BOOL GetCollisionDataPointer(uint32_t dwIndex, NRaceData::TAttachingData ** ppAttachingData);
		BOOL GetObjectDataPointer(uint32_t dwIndex, NRaceData::TAttachingData ** ppAttachingData);
		uint32_t CreateAttachingCollisionData();
		uint32_t CreateAttachingEffectData();
		uint32_t CreateAttachingObjectData();
		void DeleteOneAttachingData(uint32_t dwIndex);
		void DeleteAllAttachingData();

		const char * GetModelFileName();
		// Model Type - Character
		uint32_t & GetModelPartCountReference();
		// Model Type - Equipment
		uint32_t & GetEquipmentTypeReference();
		std::string & GetEquipmentIconImageFileNameReference();
		std::string & GetEquipmentDropModelFileNameReference();

		BOOL LoadModelScript(const char * c_szFileName);
		BOOL SaveModelScript(const char * c_szFileName);
		///////////////////////////////////////////////////////

		///////////////////////////////////////////////////////
		// About Motion Data
		BOOL isMotionThing();
		BOOL SetMotionThing(const char * c_szFileName);
		BOOL GetMotionThing(CGraphicThing::Ptr* ppThing);
		CRaceMotionDataAccessor * GetMotionDataPointer();
		///////////////////////////////////////////////////////

		///////////////////////////////////////////////////////
		// About Light Data
		void SaveLightScript(const char * c_szFileName);
		bool LoadLightScript(const char * c_szFileName);
		void SaveLightElementScript(uint32_t dwIndex, const char * c_szFileName);
		bool LoadLightElementScript(uint32_t dwIndex, const char * c_szFileName);
		void ClearLightElementData(uint32_t dwIndex);
		void ClearAllLightData();

		// Texturing
		int GetTexturingOperation(int iStage, int iType);
		int GetTexturingArgument(int iStage, int iType, int iIndex);
		void SetTexturingOperation(int iStage, int iType, int iOperation);
		void SetTexturingArgument(int iStage, int iType, int iIndex, int iArgument);

		bool isEnableTexture(int iIndex);
		void SetEnableTexture(int iIndex, BOOL bFlag);
		void LoadTexture(int iIndex, const char * c_szFileName);
		bool GetTexture(int iIndex, LPDIRECT3DTEXTURE9 * plpTexture);
		const char * GetTextureName(int iIndex);

		// Light
		uint32_t GetSelectedLightIndex();
		void SelectLight(uint32_t dwIndex);

		bool GetLightName(uint32_t dwIndex, std::string ** ppString);
		void SetLightName(uint32_t dwIndex, const char * pszLightName);

		int GetLightType(uint32_t dwIndex);
		void SetLightType(uint32_t dwIndex, int iType);

		bool isLightEnable(uint32_t dwIndex);
		void SetLightEnable(uint32_t dwIndex, bool bFlag);

		bool GetLight(uint32_t dwIndex, const D3DLIGHT9 ** ppd3dLight);
		void SetLightDirection(uint32_t dwIndex, float fx, float fy, float fz);
		void SetLightDiffuse(uint32_t dwIndex, int iRed, int iGreen, int iBlue);
		void SetLightAmbient(uint32_t dwIndex, int iRed, int iGreen, int iBlue);
		void SetLightSpecular(uint32_t dwIndex, int iRed, int iGreen, int iBlue);

		// Material
		void GetMaterial(const D3DMATERIAL9 ** ppd3dMaterial);
		bool isSpecularEnable();
		void SetSpecularEnable(bool bFlag);
		void SetMaterialDiffuse(int iRed, int iGreen, int iBlue);
		void SetMaterialAmbient(int iRed, int iGreen, int iBlue);
		void SetMaterialSpecular(int iRed, int iGreen, int iBlue);
		void SetMaterialEmissive(int iRed, int iGreen, int iBlue);
		void SetMaterialDiffuseAlpha(float fAlpha);
		void SetMaterialAmbientAlpha(float fAlpha);
		void SetMaterialSpecularAlpha(float fAlpha);
		void SetMaterialEmissiveAlpha(float fAlpha);
		void SetMaterialPower(float fPower);
		///////////////////////////////////////////////////////

	protected:
		bool CheckLightIndex(uint32_t dwIndex);
		bool GetLightDataPointer(uint32_t dwIndex, TLightElementData ** ppData);

		BOOL LoadRaceData(CTextFileLoader & rTextFileLoader);
		BOOL SaveRaceData(const char * c_szFileName);

		BOOL LoadItemData(CTextFileLoader & rTextFileLoader);
		BOOL SaveItemData(const char * c_szFileName);

		BOOL LoadObjectData(CTextFileLoader & rTextFileLoader);
		BOOL SaveObjectData(const char * c_szFileName);

	protected:
		/////////////////////////////////////////////////////////////
		//// Model Data
		std::string m_strModelName;
		CGraphicThing::Ptr m_pModelThing;
		// Public Data
		int m_iModelType;
		NRaceData::TAttachingDataVector m_AttachingDataVector;

		std::string m_strShapeDataPathName;			//CFilename m_strShapeDataPathName;
		std::vector<TShapeData> m_ShapeData;

		CDynamicPool<NRaceData::TCollisionData> m_AttachingCollisionDataPool;
		CDynamicPool<NRaceData::TAttachingObjectData> m_AttachingObjectDataPool;
		CDynamicPool<NRaceData::TAttachingEffectData> m_AttachingEffectDataPool;

		// Private Data - Character
		uint32_t m_dwPartCount;
		// Private Data - Equipment
		uint32_t m_dwEquipmentType;
		std::string m_strEquipmentIconImageFileName;
		std::string m_strEquipmentDropModelFileName;

		/////////////////////////////////////////////////////////////
		//// Motion Data
		std::string m_strMotionName;
		CGraphicThing::Ptr m_pMotionThing;
		CRaceMotionDataAccessor m_MotionData;

		/////////////////////////////////////////////////////////////
		//// Light Data
		TLightData m_LightData;

		uint32_t m_dwSelectedLightIndex;
};

METIN2_END_NS

#endif
