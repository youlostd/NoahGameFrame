#ifndef METIN2_CLIENT_GAMELIB_ITEMDATA_H
#define METIN2_CLIENT_GAMELIB_ITEMDATA_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


// NOTE : ItemÀÇ ÅëÇÕ °ü¸® Å¬·¡½º´Ù.
//        Icon, Model (droped on ground), Game Data

#include "../eterGrnLib/Thing.h"

#include "../EterLib/GrpImage.h"
#include "game/ItemTypes.hpp"

#include <optional>
#include <game/Types.hpp>
#include <game/ItemConstants.hpp>

class CItemData
{
public:



#pragma pack(push)
#pragma pack(1)
	typedef struct SItemScaleTable
	{
		Vector3 scalePos[2][5];
		Vector3 scale[2][5];
	} TItemScaleTable;

	//		typedef struct SItemTable
	//		{
	//			uint32_t       dwVnum;
	//			char        szItemName[ITEM_NAME_MAX_LEN + 1];
	//			uint8_t        bType;
	//			uint8_t        bSubType;
	//			uint8_t        bSize;
	//			uint32_t       dwAntiFlags;
	//			uint32_t       dwFlags;
	//			uint32_t       dwWearFlags;
	//			uint32_t       dwIBuyItemPrice;
	//			uint32_t		dwISellItemPrice;
	//			TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
	//			TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
	//			long        alValues[ITEM_VALUES_MAX_NUM];
	//			long        alSockets[ITEM_SOCKET_MAX_NUM];
	//			uint32_t       dwRefinedVnum;
	//			uint8_t		bSpecular;
	//			uint32_t		dwIconNumber;
	//		} TItemTable;
#pragma pack(pop)

public:
	CItemData();
	virtual ~CItemData();

	void Clear();
	void SetSummary(const std::string& c_rstSumm);
	void SetDescription(const std::string& c_rstDesc);

	CGraphicThing::Ptr GetModelThing();
	CGraphicThing::Ptr GetDropModelThing();
	CGraphicImage::Ptr GetIconImage();
	std::string        GetIconImageFileName();
	void               EnsureAssetsLoaded();

	uint32_t GetAttachingDataCount();
	bool GetCollisionDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData** c_ppAttachingData);
	bool GetAttachingDataPointer(uint32_t dwIndex, const NRaceData::TAttachingData** c_ppAttachingData);
	void SetName(const std::string& c_rstName);

	static bool IsWeddingDress(uint32_t vnum)
	{
		return vnum >= 11901 && vnum <= 11904;
	}
	/////
	const TItemTable* GetTable() const;
	uint32_t GetIndex() const;
	const char* GetName() const;
	const char* GetDescription() const;
	const char* GetSummary() const;
	uint8_t GetType() const;
	uint8_t GetSubType() const;
	int GetAttributeSetIndex();
	bool IsAttrChangeableCostume() const;
	UINT GetRefine() const;
	const char* GetUseTypeString() const;
	uint32_t GetWeaponType() const;
	uint8_t GetSize() const;
	bool IsAntiFlag(uint32_t dwFlag) const;
	bool IsFlag(uint32_t dwFlag) const;
	bool IsWearableFlag(uint32_t dwFlag) const;
	bool HasNextGrade() const;
	uint32_t GetWearFlags() const;
	uint32_t GetIBuyItemPrice() const;
	uint32_t GetISellItemPrice() const;
	uint32_t GetAntiFlags() const;
	uint32_t GetFlags() const;

	std::optional<TItemLimit> GetLimit(uint8_t byIndex) const;
    std::optional<TItemApply> GetApply(uint8_t byIndex) const;

    long GetValue(uint8_t byIndex) const;

	int GetSocketCount() const;
    int GetAddonType() const;

    uint32_t GetIconNumber() const;
	uint32_t GetRefinedVnum() const;
	uint32_t GetRefineSet() const;
	const char* GetModelThingPath() const;
	UINT GetSpecularPoweru() const;
	float GetSpecularPowerf() const;

	/////

	bool IsEquipment() const;


	bool IsSealAbleItem() const;

	void SetDefaultItemData(const std::string& iconName, const std::string& modelFileName = "");
	void SetItemTableData(const TItemTable& pItemTable);


#ifdef ENABLE_SHINING_SYSTEM
	struct ShiningEffectData
	{


	};
	typedef struct SItemShiningTable
	{
		bool persistent = false;
		std::string effects[WEAPON_NONE + 1] = {};
		std::optional<std::string> boneName;
		std::optional<float> scale;
		bool Any() const
		{
			for (auto effect : effects) {
				if (!effect.empty())
					return true;
			}
			return false;
		}
	} TItemShiningTable;

	void SetItemShiningTableData(const TItemShiningTable& data);
	const TItemShiningTable& GetItemShiningTable() const { return m_ItemShiningTable; }

#endif

	void SetItemTableScaleData(int dwJob, int dwSex, float fScaleX, float fScaleY, float fScaleZ, float fPosX, float fPosY, float fPosZ);
	Vector3& GetItemScalePosition(int dwJob, int dwSex);
	Vector3& GetItemScale(int dwJob, int dwSex);
	bool HasScaleData() const;

#ifdef INGAME_WIKI
protected:
	bool m_isValidImage;
	bool m_isBlacklisted;

public:


	bool IsValidImage() { return m_isValidImage; }

	void ValidateImage(bool isValidImage) { m_isValidImage = isValidImage; }

	std::string GetIconFileName() { return m_strIconFileName; }

	TWikiItemInfo* GetWikiTable() { return &m_wikiInfo; }

	void SetBlacklisted(bool val) { m_isBlacklisted = val; }
	bool IsWhitelisted() { return m_isBlacklisted; }

private:
	TWikiItemInfo m_wikiInfo;
#endif

protected:
	void LoadModels();
	void LoadIcon();

protected:
	std::string m_strModelFileName;
	std::string m_strDropModelFileName;
	std::string m_strIconFileName;
	std::string m_strDescription;
	std::string m_strSummary;
	std::string m_strName;
	std::vector<std::string> m_strLODModelFileNameVector;

	CGraphicThing::Ptr m_modelThing;
	CGraphicThing::Ptr m_dropModelThing;
	CGraphicImage::Ptr m_iconImage;

	NRaceData::TAttachingDataVector m_AttachingDataVector;
	uint32_t m_dwVnum{};
	TItemTable m_ItemTable{};

#ifdef ENABLE_SHINING_SYSTEM

	TItemShiningTable m_ItemShiningTable{};

#endif
	bool hasScaleData = false;
	TItemScaleTable m_ItemScaleTable = {};
	bool m_loaded{};

};
#endif /* METIN2_CLIENT_GAMELIB_ITEMDATA_H */
