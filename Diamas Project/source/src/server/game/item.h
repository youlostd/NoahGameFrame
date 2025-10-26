#ifndef METIN2_SERVER_GAME_ITEM_H
#define METIN2_SERVER_GAME_ITEM_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "entity.h"
#include "utils.h"
#include <boost/signals2/signal.hpp>

class ITEM_MANAGER;

class CItem : public CEntity
{
	friend class ITEM_MANAGER;

	protected:
	// override methods from ENTITY class
	void EncodeInsertPacket(CEntity* entity) override;
	void EncodeRemovePacket(CEntity* entity) override;

	public:
	typedef boost::signals2::signal<void()> OnDestroy;

	CItem(ItemVnum vnum);
	~CItem() override;

	int GetLevelLimit();
	bool CheckItemUseLevel(int nLevel);

	long FindApplyValue(uint8_t bApplyType);

	bool IsStackable() const;

	void Save();

	void SetID(uint32_t id);

	uint32_t GetID() const;

	TItemTable const* GetProto() const;

	Gold GetShopBuyPrice() const;

	const char* GetName() const;

	const char* GetBaseName() const;

	uint8_t GetSize() const;

	uint64_t GetFlag() const;

	bool IsSlowQuery();

	void AddFlag(long bit);
	void RemoveFlag(long bit);

	uint32_t GetWearFlag() const;

	uint64_t GetAntiFlag() const;

	uint32_t GetImmuneFlag() const;

	void SetVID(uint32_t vid);

	uint32_t GetVID();

	bool SetCount(CountType count);
	CountType GetCount() const;

	uint32_t GetVnum() const;
	uint32_t GetTransmutationVnum() const;

	void SetTransmutationVnum(uint32_t transVnum);

	uint32_t GetOriginalVnum() const;

	uint8_t GetItemType() const;
	uint8_t GetSubType() const;

	uint8_t GetLimitType(uint32_t idx) const;
	long GetLimitValue(uint32_t idx) const;

	int32_t GetLevel() const;

	bool IsSocketModifyingItem() const;
	bool IsWeapon() const;
	bool IsSword() const;
	bool IsDagger() const;
	bool IsBow() const;
	bool IsTwoHandSword() const;
	bool IsBell() const;
	bool IsFan() const;
	bool IsArrow() const;
	bool IsMountSpear() const;
	bool IsClaw() const;
	bool IsQuiver() const;
	bool IsArmor() const;
	bool IsArmorBody() const;
	bool IsHelmet() const;
	bool IsShield() const;
	bool IsWrist() const;
	bool IsShoe() const;
	bool IsNecklace() const;
	bool IsEarRing() const;
	bool IsRing() const;
	bool IsCostume() const;
	bool IsCostumeMount() const;
	bool IsCostumeHair() const;
	bool IsCostumeBody() const;
	bool IsCostumeAcce() const;
	bool IsCostumeWeapon() const;
	bool IsOldHair() const;
	bool IsCostumeModifyItem() const;
	bool IsBelt() const;
	bool IsPolymorphItem();

	// 28/03/14 - Think - Fix immune bug
	uint32_t GetRealImmuneFlag();
	// End fix immune bug

	long GetValue(uint32_t idx) const;
	long GetApply(uint32_t idx) const;

	uint8_t GetWindow() const;

	uint16_t GetCell() const;
	uint16_t GetCell();

	TItemPos GetPosition() const;
	void SetPosition(const TItemPos& pos);

	CHARACTER* GetOwner() const;
	CHARACTER* GetOwner();

	void SetOwner(CHARACTER* ch);

	void RemoveFromCharacter();
	bool AddToCharacter(CHARACTER* ch, const TItemPos& Cell);

	int GetBelongingItemID();

	CItem* RemoveFromGround();
	bool AddToGround(long lMapIndex, const PIXEL_POSITION& pos,
	                 bool skipOwnerCheck = false);

	bool IsEquipped() const;
	bool IsEquipable() const;

	bool HaveOwnership() const;

	bool DistanceValid(CHARACTER* ch);

	void UpdatePacket();

	void SetExchanging(bool isOn = true);

	bool IsExchanging() const;

	void ModifyParts(bool bAdd);
	void ModifyPointsAcce(bool bAdd);
	void ModifyPoints(
	    bool bAdd,
	    CHARACTER* ch = nullptr); // 아이템의 효과를 캐릭터에 부여 한다. bAdd가
	// false이면 제거함

	bool CreateSocket(uint8_t bSlot, uint8_t bGold);

	const SocketValue* GetSockets();

	SocketValue GetSocket(int i) const;

	void SetSockets(const SocketValue* al);
	void HandleToggleMountSocketState(SocketValue v);
	void SetSocket(int i, SocketValue v, bool bLog = true);

	int GetSocketCount() const;
	bool AddSocket();

	const TPlayerItemAttribute* GetAttributes() const;

	const TPlayerItemAttribute& GetAttribute(int i) const;

	uint8_t GetAttributeType(int i) const;

	ApplyValue GetAttributeValue(int i) const;

	void SetAttributes(const TPlayerItemAttribute* c_pAttribute);

	int FindAttribute(uint8_t bType);
	bool RemoveAttributeAt(int index);
	bool RemoveAttributeType(uint8_t bType);

	bool HasAttr(uint8_t bApply);
	bool HasRareAttr(uint8_t bApply);

	void SetDestroyEvent(LPEVENT pkEvent);
	void StartDestroyEvent(int iSec = 150);

	uint32_t GetRefinedVnum() const;

	uint32_t GetRefineFromVnum();
	int GetRefineLevel();

	void SetSkipSave(bool b);

	bool GetSkipPacket() const;

	void SetSkipPacket(bool b);

	bool GetSkipSave() const;

	bool IsOwnership(CHARACTER* ch) const;
	void SetOwnership(CHARACTER* ch, int iSec = 1243);

	uint32_t GetOwnerPid() const;

	bool WasMine() const;

	void WasMine(bool v);

	uint32_t GetLastOwnerPID() const;

	void SetLastOwnerPid(uint32_t lastOwnerPID);

	int
	GetAttributeSetIndex() const; // 속성 붙는것을 지정한 배열의 어느 인덱스를
	uint8_t GetNormAttrNum() const;
	// 사용하는지 돌려준다.
	void AlterToMagicItem(int iSecondPct = 0, int iThirdPct = 0);
	void AlterToSocketItem(int iSocketCount);

	uint16_t GetRefineSet() const;

	void StartUniqueExpireEvent();
	void SetUniqueExpireEvent(LPEVENT pkEvent);

	void StartTimerBasedOnWearExpireEvent();
	void SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent);
	void StartSelfSwitchEvent();
	void StopSelfSwitchEvent();

	void StartRealTimeExpireEvent();
	bool IsRealTimeItem();
	bool IsLimitTimeItem() const;
	bool IsUsedTimeItem();

	void StopUniqueExpireEvent();
	void StopTimerBasedOnWearExpireEvent();
	void StopAccessorySocketExpireEvent();

	//			일단 REAL_TIME과 TIMER_BASED_ON_WEAR 아이템에 대해서만 제대로
	//동작함.
	int GetDuration() const;

	int GetAttributeCount();
	void ClearAttribute();
	void ChangeAttribute(const int* aiChangeProb = nullptr);
	void AddAttribute();
	void AddAttribute(uint8_t bType, short sValue);

	void ApplyAddon(int iAddonType);

	int GetSpecialGroup() const;
	bool IsSameSpecialGroup(const CItem* item) const;

	// ACCESSORY_REFINE
	// 액세서리에 광산을 통해 소켓을 추가
	bool IsAccessoryForSocket();

	int GetAccessorySocketGrade();
	int GetAccessorySocketMaxGrade();
	int GetAccessorySocketMinGrade();
	int GetAccessorySocketDownGradeTime();

	void SetAccessorySocketGrade(int iGrade);
	void SetAccessorySocketMaxGrade(int iMaxGrade);
	void SetAccessorySocketMinGrade(int iMinGrade);

	void SetAccessorySocketDownGradeTime(uint32_t time);

	void AccessorySocketDegrade();

	// 악세사리 를 아이템에 밖았을때 타이머 돌아가는것( 구리, 등 )
	void StartAccessorySocketExpireEvent();
	void SetAccessorySocketExpireEvent(LPEVENT pkEvent);

	bool CanPutInto(CItem* item);
	// END_OF_ACCESSORY_REFINE

	void CopyAttributeTo(CItem* pItem);
	void CopySocketTo(CItem* pItem);

	int GetRareAttrCount();
	bool AddRareAttribute();
	bool ChangeRareAttribute();

	void Lock(bool f);

	bool isLocked() const;

	void OnCreate();
	void OnLoad();
	void OnUse();
	void OnEquip();
	void OnUnequip();

	private:
	void SetAttribute(int i, AttrType bType, AttrValue sValue);

	public:
	void SetForceAttribute(int i, AttrType bType, AttrValue sValue);

	boost::signals2::connection AddOnDestroy(const OnDestroy::slot_type& s);

	protected:
	void AddAttr(uint8_t bApply, uint8_t bLevel);
	void PutAttribute(const int* aiAttrPercentTable);
	void PutAttributeWithLevel(uint8_t bLevel);

	public:
	void UpdateToggleUsePulse();
	bool IsNextToggleUsePulse() const;

	bool IsRideItem();

	// 독일에서 기존 캐시 아이템과 같지만, 교환 가능한 캐시 아이템을 만든다고
	// 하여, 오리지널 아이템에, 교환 금지 플래그만 삭제한 새로운 아이템들을
	// 새로운 아이템 대역에 할당하였다. 문제는 새로운 아이템도 오리지널 아이템과
	// 같은 효과를 내야하는데, 서버건, 클라건, vnum 기반으로 되어있어 새로운
	// vnum을 죄다 서버에 새로 다 박아야하는 안타까운 상황에 맞닿았다. 그래서 새
	// vnum의 아이템이면, 서버에서 돌아갈 때는 오리지널 아이템 vnum으로 바꿔서
	// 돌고 하고, 저장할 때에 본래 vnum으로 바꿔주도록 한다.

	// Mask vnum은 어떤 이유(ex. 위의 상황)로 인해 vnum이 바뀌어 돌아가는
	// 아이템을 위해 있다.
	void SetMaskVnum(uint32_t vnum);

	uint32_t GetMaskVnum() const;

	bool IsMaskedItem() const;

	// 용혼석
	bool IsDragonSoul() const;
	int GiveMoreTime_Per(float fPercent);
	int GiveMoreTime_Fix(uint32_t dwTime);

	// seal stuff
	long m_sealDate = 0;

	long GetSealDate() const;

	void SetSealDate(long sealDate);

	bool IsSealed() const;

	bool IsUnlimitedSealDate() const;

	bool IsSealAble();
	const TItemApply* FindApply(uint8_t type) const;
	const TItemLimit* FindLimit(uint8_t type) const;

	private:
	ItemVnum m_vnum; // 프로토 타잎
	CHARACTER* m_pOwner{};
	TItemPos m_itemPos;
	bool m_pointsAdded = false;

	uint32_t m_dwID = 0;  // 고유번호
	uint32_t m_dwVID = 0; // VID

	CountType m_dwCount = 0;       // 개수
	long m_lFlag = 0;              // 추가 flag
	uint32_t m_dwLastOwnerPID = 0; // 마지막 가지고 있었던 사람의 PID
	bool m_wasMine = true;
	SocketValue m_privateShopPrice = 0;

	bool m_bExchanging = false; ///< 현재 교환중 상태

	SocketValue m_alSockets[ITEM_SOCKET_MAX_NUM]{}; // 아이템 소캣
	TPlayerItemAttribute m_aAttr[ITEM_ATTRIBUTE_MAX_NUM]{};

	LPEVENT m_pkDestroyEvent{};
	LPEVENT m_pkExpireEvent{};
	LPEVENT m_pkUniqueExpireEvent{};
	LPEVENT m_pkTimerBasedOnWearExpireEvent{};
	LPEVENT m_pkRealTimeExpireEvent{};
	LPEVENT m_pkAccessorySocketExpireEvent{};
	LPEVENT m_pkOwnershipEvent{};
	LPEVENT m_selfSwitchEvent{};

	uint32_t m_dwOwnershipPID = 0;

	bool m_bSkipSave = 0;
	bool m_bSkipPacket = false;

	bool m_isLocked = 0;

	uint32_t m_dwMaskVnum = 0;
	uint32_t m_transmutationVnum = 0;
	uint32_t m_dwSIGVnum = 0;

	int32_t m_nextTogglePulse = 0;

	OnDestroy m_onDestroy{};

	public:
	void SetSIGVnum(uint32_t dwSIG);

	uint32_t GetSIGVnum() const;

	public:
	std::string GetHyperlink();

	private:
	enum EGMOwnerTypes {
		GM_OWNER_UNSET,
		GM_OWNER_PLAYER,
		GM_OWNER_GM,
	};

	uint8_t m_bIsGMOwner = 0;

	public:
	bool IsGMOwner() const;

	void SetGMOwner(bool bGMOwner);

	private:
	bool m_bIsBlocked = 0;

	public:
	bool IsBlocked() const;

	void SetBlocked(bool bBlocked);

	public:
	SocketValue GetPrivateShopPrice() const;

	void SetPrivateShopPrice(SocketValue val);
};

EVENTINFO(item_event_info)
{
	CItem* item;
	char szOwnerName[CHARACTER_NAME_MAX_LEN]{};

	item_event_info()
	    : item(nullptr)
	    , szOwnerName{}
	{
	}
};

EVENTINFO(item_vid_event_info)
{
	uint32_t item_vid;

	item_vid_event_info()
	    : item_vid(0)
	{
	}
};

#endif /* METIN2_SERVER_GAME_ITEM_H */
