#include "char.h"
#include "constants.h"
#include "desc.h"
#include "item.h"
#include "item_manager.h"
#include "log.h"
#include "utils.h"

int CItem::GetAttributeSetIndex() const
{
	if (GetItemType() == ITEM_WEAPON) {
		switch (GetSubType()) {
			case WEAPON_ARROW:
			case WEAPON_QUIVER:
				return -1;

			default:
				return ATTRIBUTE_SET_WEAPON;
		}
	}

	if (GetItemType() == ITEM_COSTUME) {
		switch (GetSubType()) {
			case COSTUME_HAIR:
				return ATTRIBUTE_SET_COSTUME_HAIR;
			case COSTUME_BODY:
				return ATTRIBUTE_SET_COSTUME_BODY;
			case COSTUME_WEAPON:
				return ATTRIBUTE_SET_COSTUME_WEAPON;

			default:
				break;
		}
	}

	if (GetItemType() == ITEM_ARMOR) {
		switch (GetSubType()) {
			case ARMOR_BODY:
				//			case COSTUME_BODY: // 코스츔 갑옷은 일반 갑옷과 동일한
				//Attribute Set을 이용하여 랜덤속성 붙음 (ARMOR_BODY ==
				//COSTUME_BODY)
				return ATTRIBUTE_SET_BODY;

			case ARMOR_WRIST:
				return ATTRIBUTE_SET_WRIST;

			case ARMOR_FOOTS:
				return ATTRIBUTE_SET_FOOTS;

			case ARMOR_NECK:
				return ATTRIBUTE_SET_NECK;

			case ARMOR_HEAD:
				//			case COSTUME_HAIR: // 코스츔 헤어는 일반 투구 아이템과
				//동일한 Attribute Set을 이용하여 랜덤속성 붙음 (ARMOR_HEAD ==
				//COSTUME_HAIR)
				return ATTRIBUTE_SET_HEAD;

			case ARMOR_SHIELD:
				return ATTRIBUTE_SET_SHIELD;

			case ARMOR_EAR:
				return ATTRIBUTE_SET_EAR;
		}
	}

	if (GetItemType() == ITEM_TOGGLE) {
		switch (GetSubType()) {
			case TOGGLE_LEVEL_PET:
				return ATTRIBUTE_SET_LEVEL_PET;

			default:
				break;
		}
	}

	return -1;
}

uint8_t CItem::GetNormAttrNum() const
{
	if (GetAttributeSetIndex() != ATTRIBUTE_SET_LEVEL_PET)
		return ITEM_MAX_NORM_ATTR_NUM;

	return ITEM_ATTRIBUTE_MAX_NUM;
}

bool CItem::HasAttr(uint8_t bApply)
{
	auto proto = ITEM_MANAGER::instance().GetTable(m_vnum);
	if (!proto)
		return false;

	for (int i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
		if (proto->aApplies[i].bType == bApply &&
		    !ITEM_MANAGER::instance().IsDoubleApplyAllow(bApply))
			return true;

	for (int i = 0; i < GetNormAttrNum(); ++i)
		if (GetAttributeType(i) == bApply)
			return true;

	return false;
}

bool CItem::HasRareAttr(uint8_t bApply)
{
	for (int i = 0; i < ITEM_MAX_RARE_ATTR_NUM; ++i)
		if (GetAttributeType(i + 5) == bApply)
			return true;

	return false;
}

void CItem::AddAttribute(uint8_t bApply, short sValue)
{
	if (HasAttr(bApply))
		return;

	int i = GetAttributeCount();

	if (i >= GetNormAttrNum())
		SPDLOG_ERROR("item attribute overflow!");
	else {
		if (sValue)
			SetAttribute(i, bApply, sValue);
	}
}

void CItem::AddAttr(uint8_t bApply, uint8_t bLevel)
{
	if (HasAttr(bApply))
		return;

	if (bLevel <= 0)
		return;

	int i = GetAttributeCount();

	if (i == GetNormAttrNum())
		SPDLOG_ERROR("item attribute overflow!");
	else {
		const auto& r = ITEM_MANAGER::instance().GetItemAttr(bApply);
		auto val = r.values[std::min<int>(4, bLevel - 1)];

		if (val)
			SetAttribute(i, bApply, val);
	}
}

void CItem::PutAttributeWithLevel(uint8_t bLevel)
{
	int iAttributeSet = GetAttributeSetIndex();

	if (iAttributeSet < 0)
		return;

	if (bLevel > ITEM_ATTRIBUTE_MAX_LEVEL)
		return;

	std::vector<int> avail;

	int total = 0;

	auto& itemManager = ITEM_MANAGER::instance();

	// 붙일 수 있는 속성 배열을 구축
	for (int i = 0; i < MAX_APPLY_NUM; ++i) {
		const auto& r = itemManager.GetItemAttr(i);
		if (r.maxBySet[iAttributeSet] && !HasAttr(i)) {
			avail.push_back(i);
			total += r.prob;
		}
	}

	// 구축된 배열로 확률 계산을 통해 붙일 속성 선정
	unsigned int prob = Random::get(1, total);
	int attr_idx = APPLY_NONE;

	for (uint32_t i = 0; i < avail.size(); ++i) {
		const auto& r = itemManager.GetItemAttr(avail[i]);
		if (prob <= r.prob) {
			attr_idx = avail[i];
			break;
		}

		prob -= r.prob;
	}

	if (!attr_idx) {
		SPDLOG_ERROR("Cannot put item attribute %d %d", iAttributeSet, bLevel);
		return;
	}

	const auto& r = itemManager.GetItemAttr(attr_idx);

	// 종류별 속성 레벨 최대값 제한
	if (bLevel > r.maxBySet[iAttributeSet])
		bLevel = r.maxBySet[iAttributeSet];

	AddAttr(attr_idx, bLevel);
}

void CItem::PutAttribute(const int* aiAttrPercentTable)
{
	int iAttrLevelPercent = Random::get(1, 100);
	int i;

	for (i = 0; i < ITEM_ATTRIBUTE_MAX_LEVEL; ++i) {
		if (iAttrLevelPercent <= aiAttrPercentTable[i])
			break;

		iAttrLevelPercent -= aiAttrPercentTable[i];
	}

	PutAttributeWithLevel(i + 1);
}

void CItem::ChangeAttribute(const int* aiChangeProb)
{
	int iAttributeCount = GetAttributeCount();

	if (iAttributeCount == 0)
		return;

	ClearAttribute();

	TItemTable const* pProto = GetProto();

	if (pProto && pProto->sAddonType) {
		ApplyAddon(pProto->sAddonType);
	}

	static const int tmpChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = {
	    0, 10, 40, 35, 15,
	};

	for (int i = GetAttributeCount(); i < iAttributeCount; ++i) {
		if (aiChangeProb == nullptr) {
			PutAttribute(tmpChangeProb);
		} else {
			PutAttribute(aiChangeProb);
		}
	}
}

void CItem::AddAttribute()
{
	static const int aiItemAddAttributePercent[ITEM_ATTRIBUTE_MAX_LEVEL] = {
	    40, 50, 10, 0, 0};

	if (GetAttributeCount() < GetNormAttrNum())
		PutAttribute(aiItemAddAttributePercent);
}

void CItem::ClearAttribute()
{
	for (int i = 0; i < GetNormAttrNum(); ++i) {
		m_aAttr[i].bType = 0;
		m_aAttr[i].sValue = 0;
	}
}

int CItem::GetAttributeCount()
{
	int i;

	for (i = 0; i < GetNormAttrNum(); ++i) {
		if (GetAttributeType(i) == 0)
			break;
	}

	return i;
}

int CItem::FindAttribute(uint8_t bType)
{
	for (int i = 0; i < GetNormAttrNum(); ++i) {
		if (GetAttributeType(i) == bType)
			return i;
	}

	return -1;
}

bool CItem::RemoveAttributeAt(int index)
{
	if (GetAttributeCount() <= index)
		return false;

	for (int i = index; i < GetNormAttrNum() - 1; ++i) {
		SetAttribute(i, GetAttributeType(i + 1), GetAttributeValue(i + 1));
	}

	SetAttribute(GetNormAttrNum() - 1, APPLY_NONE, 0);
	return true;
}

bool CItem::RemoveAttributeType(uint8_t bType)
{
	int index = FindAttribute(bType);
	return index != -1 && RemoveAttributeType(index);
}

void CItem::SetAttributes(const TPlayerItemAttribute* c_pAttribute)
{
	std::memcpy(m_aAttr, c_pAttribute, sizeof(m_aAttr));
	Save();
}

void CItem::SetAttribute(int i, AttrType bType, AttrValue sValue)
{
	assert(i < ITEM_MAX_NORM_ATTR_NUM);

	m_aAttr[i].bType = bType;
	m_aAttr[i].sValue = sValue;
	if (!m_selfSwitchEvent)
		UpdatePacket();
	Save();
}

void CItem::SetForceAttribute(int i, AttrType bType, AttrValue sValue)
{
	assert(i < ITEM_ATTRIBUTE_MAX_NUM);

	m_aAttr[i].bType = bType;
	m_aAttr[i].sValue = sValue;
	UpdatePacket();
	Save();
}

void CItem::CopyAttributeTo(CItem* pItem)
{
	pItem->SetAttributes(m_aAttr);
}

int CItem::GetRareAttrCount()
{
	int ret = 0;

	if (m_aAttr[5].bType != 0)
		ret++;

	if (m_aAttr[6].bType != 0)
		ret++;

	return ret;
}

bool CItem::ChangeRareAttribute()
{
	if (GetRareAttrCount() == 0)
		return false;

	int cnt = GetRareAttrCount();

	for (int i = 0; i < cnt; ++i) {
		m_aAttr[i + 5].bType = 0;
		m_aAttr[i + 5].sValue = 0;
	}

	for (int i = 0; i < cnt; ++i) {
		AddRareAttribute();
	}

	return true;
}

void CItem::Lock(bool f)
{
	m_isLocked = f;
}

bool CItem::isLocked() const
{
	return m_isLocked;
}

bool CItem::AddRareAttribute()
{
	int nAttrSet = GetAttributeSetIndex();
	if (nAttrSet == -1)
		return false;

	int count = GetRareAttrCount();

	if (count >= 2)
		return false;

	int pos = count + 5;
	TPlayerItemAttribute& attr = m_aAttr[pos];

	std::vector<int> avail;

	auto& itemManager = ITEM_MANAGER::instance();

	for (int i = 0; i < MAX_APPLY_NUM; ++i) {
		const auto& r = itemManager.GetRareItemAttr(i);
		if (r.apply != 0 && r.maxBySet[nAttrSet] > 0 && HasRareAttr(i) != true)
			avail.push_back(i);
	}

	const auto& r = itemManager.GetRareItemAttr(
	    avail[Random::get<std::size_t>(0, avail.size() - 1)]);

	int nAttrLevel = 5;
	if (nAttrLevel > r.maxBySet[nAttrSet])
		nAttrLevel = r.maxBySet[nAttrSet];

	attr.bType = r.apply;
	attr.sValue = r.values[nAttrLevel - 1];

	UpdatePacket();
	Save();

	return true;
}
