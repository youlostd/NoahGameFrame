#include "constants.h"
#include "item.h"
#include "item_manager.h"
#include <game/GamePacket.hpp>
#include "desc.h"
#include "char.h"
#include <game/DragonSoulTable.hpp>
#include "log.h"
#include "DragonSoul.h"
#include "main.h"
#include <boost/algorithm/clamp.hpp>
#include <boost/math/constants/constants.hpp>

#include "game/AffectConstants.hpp"
#include "TextTagUtil.hpp"

typedef std::vector<std::string> TTokenVector;

int Gamble(std::vector<float> &vecProbs)
{
    auto range = 0.f;
    for (size_t i = 0; i < vecProbs.size(); i++) { range += vecProbs[i]; }
    const auto fProb = Random::get(0.0f, range);
    auto sum = 0.0f;
    for (size_t idx = 0; idx < vecProbs.size(); idx++)
    {
        sum += vecProbs[idx];
        if (sum >= fProb)
            return idx;
    }
    return -1;
}

// 가중치 테이블(prob_lst)을 받아 random_set.size()개의 index를 선택하여 random_set을 return
bool MakeDistinctRandomNumberSet(std::list<float> prob_lst, std::vector<int> &random_set)
{
    int size = prob_lst.size();
    int n = random_set.size();
    if (size < n)
        return false;

    std::vector<int> select_bit(size, 0);
    for (int i = 0; i < n; i++)
    {
        float range = 0.f;
        for (std::list<float>::iterator it = prob_lst.begin(); it != prob_lst.end(); ++it) { range += *it; }
        float r = Random::get(0.f, range);
        float sum = 0.f;
        int idx = 0;
        for (std::list<float>::iterator it = prob_lst.begin(); it != prob_lst.end(); ++it)
        {
            while (select_bit[idx++]);

            sum += *it;
            if (sum >= r)
            {
                select_bit[idx - 1] = 1;
                random_set[i] = idx - 1;
                prob_lst.erase(it);
                break;
            }
        }
    }
    return true;
}

/* 용혼석 Vnum에 대한 comment	
 * ITEM VNUM을 10만 자리부터, FEDCBA라고 한다면
 * FE : 용혼석 종류.	D : 등급
 * C : 단계			B : 강화		
 * A : 여벌의 번호들... 	
 */

uint8_t GetType(uint32_t dwVnum) { return uint8_t(dwVnum / 10000); }

uint8_t GetGradeIdx(uint32_t dwVnum) { return (dwVnum / 1000) % 10; }

uint8_t GetStepIdx(uint32_t dwVnum) { return (dwVnum / 100) % 10; }

uint8_t GetStrengthIdx(uint32_t dwVnum) { return (dwVnum / 10) % 10; }

bool DSManager::ReadDragonSoulTableFile(const char *c_pszFileName)
{
    return m_pTable.ReadDragonSoulTableFile(c_pszFileName);
}

void DSManager::ClearDragonSoulTable()
{
}

void DSManager::GetDragonSoulInfo(uint32_t dwVnum, uint8_t &bType, uint8_t &bGrade, uint8_t &bStep,
                                  uint8_t &bStrength) const
{
    bType = GetType(dwVnum);
    bGrade = GetGradeIdx(dwVnum);
    bStep = GetStepIdx(dwVnum);
    bStrength = GetStrengthIdx(dwVnum);
}

bool DSManager::IsValidCellForThisItem(const TItemTable *pItem, const TItemPos &Cell)
{
    if (nullptr == pItem)
        return false;

    uint16_t wBaseCell = GetBasePosition(pItem);
    if (std::numeric_limits<uint16_t>::max() == wBaseCell)
        return false;

    if (Cell.window_type != DRAGON_SOUL_INVENTORY
        || (Cell.cell < wBaseCell || Cell.cell >= wBaseCell + DRAGON_SOUL_BOX_SIZE)) { return false; }

    return true;
}

bool DSManager::IsValidCellForThisItem(const CItem *pItem, const TItemPos &Cell) const
{
    if (nullptr == pItem)
        return false;

    uint16_t wBaseCell = GetBasePosition(pItem);
    if (std::numeric_limits<uint16_t>::max() == wBaseCell)
        return false;

    if (Cell.window_type != DRAGON_SOUL_INVENTORY
        || (Cell.cell < wBaseCell || Cell.cell >= wBaseCell + DRAGON_SOUL_BOX_SIZE)) { return false; }
    else
        return true;
}

uint16_t DSManager::GetBasePosition(const CItem *pItem) const
{
    if (nullptr == pItem)
        return std::numeric_limits<uint16_t>::max();

    uint8_t type, grade_idx, step_idx, strength_idx;
    GetDragonSoulInfo(pItem->GetVnum(), type, grade_idx, step_idx, strength_idx);

    uint8_t col_type = pItem->GetSubType();
    uint8_t row_type = grade_idx;
    if (row_type > DRAGON_SOUL_GRADE_MAX)
        return std::numeric_limits<uint16_t>::max();

    return col_type * DRAGON_SOUL_GRADE_MAX * DRAGON_SOUL_BOX_SIZE + row_type * DRAGON_SOUL_BOX_SIZE;
}

uint16_t DSManager::GetBasePosition(const TItemTable *pItem)
{
    if (nullptr == pItem)
        return std::numeric_limits<uint16_t>::max();

    uint8_t type, grade_idx, step_idx, strength_idx;
    GetDragonSoulInfo(pItem->dwVnum, type, grade_idx, step_idx, strength_idx);

    uint8_t col_type = pItem->bSubType;
    uint8_t row_type = grade_idx;
    if (row_type > DRAGON_SOUL_GRADE_MAX)
        return std::numeric_limits<uint16_t>::max();

    return col_type * DRAGON_SOUL_STEP_MAX * DRAGON_SOUL_BOX_SIZE + row_type * DRAGON_SOUL_BOX_SIZE;
}

bool DSManager::RefreshItemAttributes(CItem *pDS)
{
    if (!pDS->IsDragonSoul())
    {
        SPDLOG_ERROR("This item(ID : %d) is not DragonSoul.", pDS->GetID());
        return false;
    }

    uint8_t ds_type, grade_idx, step_idx, strength_idx;
    GetDragonSoulInfo(pDS->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

    DragonSoulTable::TVecApplys vec_basic_applys;
    DragonSoulTable::TVecApplys vec_addtional_applys;

    if (!m_pTable.GetBasicApplys(ds_type, vec_basic_applys))
    {
        SPDLOG_ERROR("There is no BasicApply about {} type dragon soul.", ds_type);
        return false;
    }

    if (!m_pTable.GetAdditionalApplys(ds_type, vec_addtional_applys))
    {
        SPDLOG_ERROR("There is no AdditionalApply about {} type dragon soul.", ds_type);
        return false;
    }

    // add_min과 add_max는 더미로 읽음.
    int basic_apply_num, add_min, add_max;
    if (!m_pTable.GetApplyNumSettings(ds_type, grade_idx, basic_apply_num, add_min, add_max))
    {
        SPDLOG_ERROR("In ApplyNumSettings, INVALID VALUES Group type({}), GRADE idx({})", ds_type, grade_idx);
        return false;
    }

    float fWeight = 0.f;
    if (!m_pTable.GetWeight(ds_type, grade_idx, step_idx, strength_idx, fWeight)) { return false; }
    fWeight /= 100.f;

    int n = std::min<int>(basic_apply_num, vec_basic_applys.size());
    for (int i = 0; i < n; i++)
    {
        const SApply &basic_apply = vec_basic_applys[i];
        uint8_t bType = basic_apply.apply_type;
        ApplyValue sValue = basic_apply.apply_value * fWeight;

        pDS->SetForceAttribute(i, bType, sValue);
    }

    for (int i = DRAGON_SOUL_ADDITIONAL_ATTR_START_IDX; i < ITEM_ATTRIBUTE_MAX_NUM; i++)
    {
        uint8_t bType = pDS->GetAttributeType(i);
        ApplyValue sValue = 0;
        if (APPLY_NONE == bType)
            continue;

        for (size_t j = 0; j < vec_addtional_applys.size(); j++)
        {
            if (vec_addtional_applys[j].apply_type == bType)
            {
                sValue = vec_addtional_applys[j].apply_value;
                break;
            }
        }
        pDS->SetForceAttribute(i, bType, static_cast<ApplyValue>(sValue * fWeight));
    }
    return true;
}

bool DSManager::PutAttributes(CItem *pDS)
{
    if (!pDS->IsDragonSoul())
    {
        SPDLOG_ERROR("This item(ID : %d) is not DragonSoul.", pDS->GetID());
        return false;
    }

    uint8_t ds_type, grade_idx, step_idx, strength_idx;
    GetDragonSoulInfo(pDS->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

    DragonSoulTable::TVecApplys vec_basic_applys;
    DragonSoulTable::TVecApplys vec_addtional_applys;

    if (!m_pTable.GetBasicApplys(ds_type, vec_basic_applys))
    {
        SPDLOG_ERROR("There is no BasicApply about %d type dragon soul.", ds_type);
        return false;
    }
    if (!m_pTable.GetAdditionalApplys(ds_type, vec_addtional_applys))
    {
        SPDLOG_ERROR("There is no AdditionalApply about %d type dragon soul.", ds_type);
        return false;
    }

    int basic_apply_num, add_min, add_max;
    if (!m_pTable.GetApplyNumSettings(ds_type, grade_idx, basic_apply_num, add_min, add_max))
    {
        SPDLOG_ERROR("In ApplyNumSettings, INVALID VALUES Group type(%d), GRADE idx(%d)", ds_type, grade_idx);
        return false;
    }

    float fWeight = 0.0f;
    if (!m_pTable.GetWeight(ds_type, grade_idx, step_idx, strength_idx, fWeight)) { return false; }
    fWeight /= 100.f;

    int n = std::min<int>(basic_apply_num, vec_basic_applys.size());
    for (int i = 0; i < n; i++)
    {
        const SApply &basic_apply = vec_basic_applys[i];
        uint8_t bType = basic_apply.apply_type;
        ApplyValue sValue = static_cast<ApplyValue>(basic_apply.apply_value * fWeight);

        pDS->SetForceAttribute(i, bType, sValue);
    }

    const uint8_t additional_attr_num = std::min<uint8_t>(Random::get(add_min, add_max), 3);

    std::vector<int> random_set;
    if (additional_attr_num > 0)
    {
        random_set.resize(additional_attr_num);
        std::list<float> list_probs;
        for (auto &apply : vec_addtional_applys) { list_probs.push_back(apply.prob); }
        if (!MakeDistinctRandomNumberSet(list_probs, random_set))
        {
            SPDLOG_ERROR("MakeDistinctRandomNumberSet error.");
            return false;
        }

        for (int i = 0; i < additional_attr_num; i++)
        {
            int r = random_set[i];
            const SApply &additional_attr = vec_addtional_applys[r];
            auto bType = additional_attr.apply_type;
            ApplyValue sValue = static_cast<ApplyValue>(additional_attr.apply_value * fWeight);

            pDS->SetForceAttribute(DRAGON_SOUL_ADDITIONAL_ATTR_START_IDX + i, bType, sValue);
        }
    }

    return true;
}

bool DSManager::DragonSoulItemInitialize(CItem *pItem)
{
    if (nullptr == pItem || !pItem->IsDragonSoul())
        return false;
    PutAttributes(pItem);
    int time = DSManager::instance().GetDuration(pItem);
    if (time > 0)
        pItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, time);
    return true;
}

uint32_t DSManager::MakeDragonSoulVnum(uint8_t bType, uint8_t grade, uint8_t step, uint8_t refine)
{
    return bType * 10000 + grade * 1000 + step * 100 + refine * 10;
}

int DSManager::GetDuration(const CItem *pItem) const { return pItem->GetDuration(); }

// 용혼석을 받아서 용심을 추출하는 함수
bool DSManager::ExtractDragonHeart(CHARACTER *ch, CItem *pItem, CItem *pExtractor)
{
    if (nullptr == ch || nullptr == pItem)
        return false;
    if (pItem->IsEquipped())
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "착용 중인 용혼석은 추출할 수 없습니다.");
        return false;
    }

    uint32_t dwVnum = pItem->GetVnum();
    uint8_t ds_type, grade_idx, step_idx, strength_idx;
    GetDragonSoulInfo(dwVnum, ds_type, grade_idx, step_idx, strength_idx);

    int iBonus = 0;

    if (nullptr != pExtractor) { iBonus = pExtractor->GetValue(0); }

    std::vector<float> vec_chargings;
    std::vector<float> vec_probs;

    if (!m_pTable.GetDragonHeartExtValues(ds_type, grade_idx, vec_chargings, vec_probs)) { return false; }

    int idx = Gamble(vec_probs);

    float sum = 0.f;
    if (-1 == idx)
    {
        SPDLOG_ERROR("Gamble is failed. ds_type(%d), grade_idx(%d)", ds_type, grade_idx);
        return false;
    }

    float fCharge = vec_chargings[idx] * (100 + iBonus) / 100.f;
    fCharge = std::clamp<float>(fCharge, 0.f, 100.f);

    if (fCharge < boost::math::tools::epsilon<float>())
    {
        pItem->SetCount(pItem->GetCount() - 1);
        if (nullptr != pExtractor) { pExtractor->SetCount(pExtractor->GetCount() - 1); }
        LogManager::instance().ItemLog(ch, pItem, "DS_HEART_EXTRACT_FAIL", "");

        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "용심 추출에 실패하였습니다.");
        return false;
    }
    else
    {
        CItem *pDH = ITEM_MANAGER::instance().CreateItem(DRAGON_HEART_VNUM);

        if (nullptr == pDH)
        {
            SPDLOG_ERROR("Cannot create DRAGON_HEART(%d).", DRAGON_HEART_VNUM);
            return false;
        }

        pItem->SetCount(pItem->GetCount() - 1);
        if (nullptr != pExtractor) { pExtractor->SetCount(pExtractor->GetCount() - 1); }

        int iCharge = (int)(fCharge + 0.5f);
        pDH->SetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX, iCharge);
        pItem = ch->AutoGiveItem(pDH, true);

        std::string s = std::to_string(iCharge);
        s += "%s";
        LogManager::instance().ItemLog(ch, pItem, "DS_HEART_EXTRACT_SUCCESS", s.c_str());
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "용심 추출에 성공하였습니다.");
        return true;
    }
}

// 특정 용혼석을 장비창에서 제거할 때에 성공 여부를 결정하고, 실패시 부산물을 주는 함수.
bool DSManager::PullOut(CHARACTER *ch, TItemPos DestCell, CItem *&pItem, CItem *pExtractor)
{
    if (!ch || !pItem)
    {
        SPDLOG_ERROR("Trying to PullOut dragon stone either ch({}) or pItem({}) as nullptr", fmt::ptr(ch),
                     fmt::ptr(pItem));
        return false;
    }

    // 목표 위치가 valid한지 검사 후, valid하지 않다면 임의의 빈 공간을 찾는다.
    if (!IsValidCellForThisItem(pItem, DestCell))
    {
        int iEmptyCell = ch->GetEmptyDragonSoulInventory(pItem);
        if (iEmptyCell < 0)
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "소지품에 빈 공간이 없습니다.");
            return false;
        }
        else
        {
            DestCell.window_type = DRAGON_SOUL_INVENTORY;
            DestCell.cell = iEmptyCell;
        }
    }

    if (!pItem->IsEquipped())
        return false;

    pItem->RemoveFromCharacter();

#ifdef ENABLE_DS_SET
    if (ch->DragonSoul_IsDeckActivated())
    {
        ch->DragonSoul_HandleSetBonus();
        ch->RemoveAffect(AFFECT_DS_SET);
    }
#endif

    bool bSuccess;
    uint32_t dwByProduct = 0;
    int iBonus = 0;
    float fProb;
    float fDice;
    // 용혼석 추출 성공 여부 결정.
    {
        uint32_t dwVnum = pItem->GetVnum();

        uint8_t ds_type, grade_idx, step_idx, strength_idx;
        GetDragonSoulInfo(pItem->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

        // 추출 정보가 없다면 일단 무조건 성공하는 것이라 생각하자.
        if (!m_pTable.GetDragonSoulExtValues(ds_type, grade_idx, fProb, dwByProduct))
        {
            pItem->AddToCharacter(ch, DestCell);
            return true;
        }

        if (nullptr != pExtractor)
        {
            iBonus = pExtractor->GetValue(ITEM_VALUE_DRAGON_SOUL_POLL_OUT_BONUS_IDX);
            pExtractor->SetCount(pExtractor->GetCount() - 1);
        }
        fDice = Random::get(0.f, 100.f);
        bSuccess = fDice <= (fProb * (100 + iBonus) / 100.f);
    }

    // 캐릭터의 용혼석 추출 및 추가 혹은 제거. 부산물 제공.
    {
        char buf[128];

        if (bSuccess)
        {
            if (pExtractor)
            {
                sprintf(buf, "dice(%d) prob(%d + %d) EXTR(VN:%d)", (int)fDice, (int)fProb, iBonus,
                        pExtractor->GetVnum());
            }
            else { sprintf(buf, "dice(%d) prob(%d)", (int)fDice, (int)fProb); }
            LogManager::instance().ItemLog(ch, pItem, "DS_PULL_OUT_SUCCESS", buf);
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "용혼석 추출에 성공하였습니다.");
            pItem->AddToCharacter(ch, DestCell);
            return true;
        }
        else
        {
            if (pExtractor)
            {
                sprintf(buf, "dice(%d) prob(%d + %d) EXTR(VN:%d) ByProd(VN:%d)", (int)fDice, (int)fProb, iBonus,
                        pExtractor->GetVnum(), dwByProduct);
            }
            else { sprintf(buf, "dice(%d) prob(%d) ByProd(VNUM:%d)", (int)fDice, (int)fProb, dwByProduct); }
            LogManager::instance().ItemLog(ch, pItem, "DS_PULL_OUT_FAILED", buf);
            M2_DESTROY_ITEM(pItem);
            pItem = nullptr;
            if (dwByProduct)
            {
                CItem *pByProduct = ch->AutoGiveItem(dwByProduct, true);
                if (pByProduct)
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "용혼석 추출에 실패하여 %s를 얻었습니다.",
                                       TextTag::itemname(pByProduct->GetVnum()).c_str());
                else
                    SendI18nChatPacket(ch, CHAT_TYPE_INFO, "용혼석 추출에 실패하였습니다.");
            }
            else
                SendI18nChatPacket(ch, CHAT_TYPE_INFO, "용혼석 추출에 실패하였습니다.");
        }
    }

    return bSuccess;
}

bool DSManager::DoRefineGrade(CHARACTER *ch, TItemPos (&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE])
{
    if (nullptr == ch)
        return false;

    if (nullptr == aItemPoses) { return false; }

    if (!ch->DragonSoul_RefineWindow_CanRefine())
    {
        SPDLOG_ERROR("%s do not activate DragonSoulRefineWindow. But how can he come here?", ch->GetName());
        ch->ChatPacket(CHAT_TYPE_INFO, "[SYSTEM ERROR]You cannot upgrade dragon soul without refine window.");
        return false;
    }

    // 혹시나 모를 중복되는 item pointer 없애기 위해서 set 사용
    // 이상한 패킷을 보낼 경우, 중복된 TItemPos가 있을 수도 있고, 잘못된 TItemPos가 있을 수도 있다.
    std::unordered_set<CItem *> set_items;
    for (int i = 0; i < DRAGON_SOUL_REFINE_GRID_SIZE; i++)
    {
        if (aItemPoses[i].IsEquipPosition())
            return false;
        CItem *pItem = ch->GetItem(aItemPoses[i]);
        if (nullptr != pItem)
        {
            // 용혼석이 아닌 아이템이 개량창에 있을 수 없다.
            if (!pItem->IsDragonSoul())
            {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO, "등급 개량에 필요한 재료가 아닙니다.");
                SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
                                       TItemPos(pItem->GetWindow(), pItem->GetCell()));

                return false;
            }

            set_items.insert(pItem);
        }
    }

    if (set_items.empty())
    {
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL, NPOS);
        return false;
    }

    uint32_t count = set_items.size();
    CountType need_count = 0;
    Gold fee = 0;
    std::vector<float> vec_probs;

    uint8_t ds_type, grade_idx, step_idx, strength_idx;
    int result_grade;

    // 가장 처음 것을 강화의 기준으로 삼는다.
    std::unordered_set<CItem *>::iterator it = set_items.begin();
    {
        CItem *pItem = *it;

        GetDragonSoulInfo(pItem->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

        if (!m_pTable.GetRefineGradeValues(ds_type, grade_idx, need_count, fee, vec_probs))
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "등급 개량할 수 없는 용혼석입니다.");
            SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
                                   TItemPos(pItem->GetWindow(), pItem->GetCell()));

            return false;
        }
    }
    while (++it != set_items.end())
    {
        CItem *pItem = *it;

        // 클라 ui에서 장착한 아이템은 개량창에 올릴 수 없도록 막았기 때문에,
        // 별도의 알림 처리는 안함.
        if (pItem->IsEquipped()) { return false; }

        if (ds_type != GetType(pItem->GetVnum()) || grade_idx != GetGradeIdx(pItem->GetVnum()))
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "등급 개량에 필요한 재료가 아닙니다.");
            SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
                                   TItemPos(pItem->GetWindow(), pItem->GetCell()));

            return false;
        }
    }

    // 클라에서 한번 갯수 체크를 하기 때문에 count != need_count라면 invalid 클라일 가능성이 크다.
    if (count != need_count)
    {
        SPDLOG_ERROR("Possiblity of invalid client. Name %s", ch->GetName());
        uint8_t bSubHeader = count < need_count
                                 ? DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL
                                 : DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL;
        SendRefineResultPacket(ch, bSubHeader, NPOS);
        return false;
    }

    if (ch->GetGold() < fee)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "개량을 하기 위한 돈이 부족합니다.");
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY, NPOS);
        return false;
    }

    if (-1 == (result_grade = Gamble(vec_probs)))
    {
        SPDLOG_ERROR("Gamble failed. See RefineGardeTables' probabilities");
        return false;
    }

    CItem *pResultItem = ITEM_MANAGER::instance().CreateItem(MakeDragonSoulVnum(ds_type, (uint8_t)result_grade, 0, 0));

    if (nullptr == pResultItem)
    {
        SPDLOG_ERROR("INVALID DRAGON SOUL({})", MakeDragonSoulVnum(ds_type, (uint8_t)result_grade, 0, 0));
        return false;
    }

    ch->ChangeGold(-fee);
    CountType left_count = need_count;

    for (auto* pItem : set_items)
    {
        CountType n = pItem->GetCount();
        if (left_count > n)
        {
            pItem->RemoveFromCharacter();
            M2_DESTROY_ITEM(pItem);
            left_count -= n;
        }
        else { pItem->SetCount(n - left_count); }
    }

    pResultItem = ch->AutoGiveItem(pResultItem, true);

    if (result_grade > grade_idx)
    {
        char buf[128];
        sprintf(buf, "GRADE : %d -> %d", grade_idx, result_grade);
        LogManager::instance().ItemLog(ch, pResultItem, "DS_GRADE_REFINE_SUCCESS", buf);
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "등급 개량에 성공했습니다.");
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_SUCCEED,
                               TItemPos(pResultItem->GetWindow(), pResultItem->GetCell()));
        return true;
    }
    else
    {
        char buf[128];
        sprintf(buf, "GRADE : %d -> %d", grade_idx, result_grade);
        LogManager::instance().ItemLog(ch, pResultItem, "DS_GRADE_REFINE_FAIL", buf);
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "등급 개량에 실패했습니다.");
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL,
                               TItemPos(pResultItem->GetWindow(), pResultItem->GetCell()));
        return false;
    }
}

bool DSManager::DoRefineStep(CHARACTER *ch, TItemPos (&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE])
{
    if (nullptr == ch)
        return false;
    if (nullptr == aItemPoses) { return false; }

    if (!ch->DragonSoul_RefineWindow_CanRefine())
    {
        SPDLOG_ERROR("%s do not activate DragonSoulRefineWindow. But how can he come here?", ch->GetName());
        ch->ChatPacket(CHAT_TYPE_INFO, "[SYSTEM ERROR]You cannot use dragon soul refine window.");
        return false;
    }

    // 혹시나 모를 중복되는 item pointer 없애기 위해서 set 사용
    // 이상한 패킷을 보낼 경우, 중복된 TItemPos가 있을 수도 있고, 잘못된 TItemPos가 있을 수도 있다.
    std::unordered_set<CItem *> set_items;
    for (int i = 0; i < DRAGON_SOUL_REFINE_GRID_SIZE; i++)
    {
        CItem *pItem = ch->GetItem(aItemPoses[i]);
        if (nullptr != pItem)
        {
            // 용혼석이 아닌 아이템이 개량창에 있을 수 없다.
            if (!pItem->IsDragonSoul())
            {
                SendI18nChatPacket(ch, CHAT_TYPE_INFO, "단계 개량에 필요한 재료가 아닙니다.");
                SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
                                       TItemPos(pItem->GetWindow(), pItem->GetCell()));
                return false;
            }
            set_items.insert(pItem);
        }
    }

    if (set_items.empty())
    {
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL, NPOS);
        return false;
    }

    int count = set_items.size();
    CountType need_count = 0;
    Gold fee = 0;
    std::vector<float> vec_probs;

    uint8_t ds_type, grade_idx, step_idx, strength_idx;
    int result_step;

    // 가장 처음 것을 강화의 기준으로 삼는다.
    auto it = set_items.begin();
    {
        CItem *pItem = *it;
        GetDragonSoulInfo(pItem->GetVnum(), ds_type, grade_idx, step_idx, strength_idx);

        if (!m_pTable.GetRefineStepValues(ds_type, step_idx, need_count, fee, vec_probs))
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "단계 개량할 수 없는 용혼석입니다.");
            SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
                                   TItemPos(pItem->GetWindow(), pItem->GetCell()));
            return false;
        }
    }

    while (++it != set_items.end())
    {
        CItem *pItem = *it;
        // 클라 ui에서 장착한 아이템은 개량창에 올릴 수 없도록 막았기 때문에,
        // 별도의 알림 처리는 안함.
        if (pItem->IsEquipped())
        {
            SPDLOG_ERROR("set_items IsEquipped");
            return false;
        }
        if (ds_type != GetType(pItem->GetVnum()) || grade_idx != GetGradeIdx(pItem->GetVnum()) || step_idx !=
            GetStepIdx(pItem->GetVnum()))
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "단계 개량에 필요한 재료가 아닙니다.");
            SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
                                   TItemPos(pItem->GetWindow(), pItem->GetCell()));
            return false;
        }
    }

    // 클라에서 한번 갯수 체크를 하기 때문에 count != need_count라면 invalid 클라일 가능성이 크다.
    if (count != need_count)
    {
        SPDLOG_ERROR("Possiblity of invalid client. Name %s", ch->GetName());
        uint8_t bSubHeader = count < need_count
                                 ? DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL
                                 : DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL;
        SendRefineResultPacket(ch, bSubHeader, NPOS);
        return false;
    }

    if (ch->GetGold() < fee)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "개량을 하기 위한 돈이 부족합니다.");
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY, NPOS);
        return false;
    }

    float sum = 0.f;

    if (-1 == (result_step = Gamble(vec_probs)))
    {
        SPDLOG_ERROR("Gamble failed. See RefineStepTables' probabilities");
        return false;
    }

    CItem *pResultItem = ITEM_MANAGER::instance().CreateItem(
        MakeDragonSoulVnum(ds_type, grade_idx, (uint8_t)result_step, 0));

    if (nullptr == pResultItem)
    {
        SPDLOG_ERROR("INVALID DRAGON SOUL({})", MakeDragonSoulVnum(ds_type, grade_idx, (uint8_t)result_step, 0));
        return false;
    }

    ch->ChangeGold(-fee);
    int left_count = need_count;
    for (auto it = set_items.begin(); it != set_items.end(); ++it)
    {
        CItem *pItem = *it;
        CountType n = pItem->GetCount();
        if (left_count > n)
        {
            pItem->RemoveFromCharacter();
            M2_DESTROY_ITEM(pItem);
            left_count -= n;
        }
        else { pItem->SetCount(n - left_count); }
    }

    pResultItem = ch->AutoGiveItem(pResultItem, true);
    if (result_step > step_idx || (result_step == DRAGON_SOUL_STEP_HIGHEST))
    {
        char buf[128];
        sprintf(buf, "STEP : %d -> %d", step_idx, result_step);
        LogManager::instance().ItemLog(ch, pResultItem, "DS_STEP_REFINE_SUCCESS", buf);
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "단계 개량에 성공했습니다.");
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_SUCCEED,
                               TItemPos(pResultItem->GetWindow(), pResultItem->GetCell()));
        return true;
    }
    else
    {
        char buf[128];
        sprintf(buf, "STEP : %d -> %d", step_idx, result_step);
        LogManager::instance().ItemLog(ch, pResultItem, "DS_STEP_REFINE_FAIL", buf);
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "단계 개량에 실패했습니다.");
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL,
                               TItemPos(pResultItem->GetWindow(), pResultItem->GetCell()));
        return false;
    }
}

bool IsDragonSoulRefineMaterial(CItem *pItem)
{
    if (pItem->GetItemType() != ITEM_MATERIAL)
        return false;
    return (pItem->GetSubType() == MATERIAL_DS_REFINE_NORMAL ||
            pItem->GetSubType() == MATERIAL_DS_REFINE_BLESSED ||
            pItem->GetSubType() == MATERIAL_DS_REFINE_HOLLY);
}

bool DSManager::DoRefineStrength(CHARACTER *ch, TItemPos (&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE])
{
    if (nullptr == ch)
        return false;
    if (nullptr == aItemPoses) { return false; }

    if (!ch->DragonSoul_RefineWindow_CanRefine())
    {
        SPDLOG_ERROR("%s do not activate DragonSoulRefineWindow. But how can he come here?", ch->GetName());
        ch->ChatPacket(CHAT_TYPE_INFO, "[SYSTEM ERROR]You cannot use dragon soul refine window.");
        return false;
    }

    // 혹시나 모를 중복되는 item pointer 없애기 위해서 set 사용
    // 이상한 패킷을 보낼 경우, 중복된 TItemPos가 있을 수도 있고, 잘못된 TItemPos가 있을 수도 있다.
    std::unordered_set<CItem *> set_items;
    for (int i = 0; i < DRAGON_SOUL_REFINE_GRID_SIZE; i++)
    {
        CItem *pItem = ch->GetItem(aItemPoses[i]);
        if (pItem) { set_items.insert(pItem); }
    }
    if (set_items.empty()) { return false; }

    Gold fee;

    CItem *pRefineStone = nullptr;
    CItem *pDragonSoul = nullptr;
    for (auto pItem : set_items)
    {
        // 클라 ui에서 장착한 아이템은 개량창에 올릴 수 없도록 막았기 때문에,
        // 별도의 알림 처리는 안함.
        if (pItem->IsEquipped()) { return false; }

        // 용혼석과 강화석만이 개량창에 있을 수 있다.
        // 그리고 하나씩만 있어야한다.
        if (pItem->IsDragonSoul())
        {
            if (pDragonSoul != nullptr)
            {
                SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
                                       TItemPos(pItem->GetWindow(), pItem->GetCell()));
                return false;
            }
            pDragonSoul = pItem;
        }
        else if (IsDragonSoulRefineMaterial(pItem))
        {
            if (pRefineStone != nullptr)
            {
                SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
                                       TItemPos(pItem->GetWindow(), pItem->GetCell()));
                return false;
            }
            pRefineStone = pItem;
        }
        else
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "강화에 필요한 재료가 아닙니다.");
            SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
                                   TItemPos(pItem->GetWindow(), pItem->GetCell()));
            return false;
        }
    }

    uint8_t bType, bGrade, bStep, bStrength;

    if (!pRefineStone)
        pRefineStone = ch->FindSpecifyItem(100500);
    if (!pRefineStone)
        pRefineStone = ch->FindSpecifyItem(100400);
    if (!pRefineStone)
        pRefineStone = ch->FindSpecifyItem(100300);

    if (!pDragonSoul || !pRefineStone)
    {
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL, NPOS);

        return false;
    }

    if (nullptr != pDragonSoul)
    {
        GetDragonSoulInfo(pDragonSoul->GetVnum(), bType, bGrade, bStep, bStrength);

        float fWeight = 0.f;
        // 가중치 값이 없다면 강화할 수 없는 용혼석
        if (!m_pTable.GetWeight(bType, bGrade, bStep, bStrength + 1, fWeight))
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "강화할 수 없는 용혼석입니다.");
            SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
                                   TItemPos(pDragonSoul->GetWindow(), pDragonSoul->GetCell()));
            return false;
        }
        // 강화했을 때 가중치가 0이라면 더 이상 강화되서는 안된다.
        if (fWeight < boost::math::tools::epsilon<float>())
        {
            SendI18nChatPacket(ch, CHAT_TYPE_INFO, "강화할 수 없는 용혼석입니다.");
            SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
                                   TItemPos(pDragonSoul->GetWindow(), pDragonSoul->GetCell()));
            return false;
        }
    }

    float fProb;
    if (!m_pTable.GetRefineStrengthValues(bType, pRefineStone->GetSubType(), bStrength, fee, fProb))
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "강화할 수 없는 용혼석입니다.");
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
                               TItemPos(pDragonSoul->GetWindow(), pDragonSoul->GetCell()));

        return false;
    }

    if (ch->GetGold() < fee)
    {
        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "개량을 하기 위한 돈이 부족합니다.");
        SendRefineResultPacket(ch, DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY, NPOS);
        return false;
    }

    ch->ChangeGold(-fee);
    CItem *pResult = nullptr;
    uint8_t bSubHeader;

    if (Random::get(0.f, 100.f) <= fProb)
    {
        pResult = ITEM_MANAGER::instance().CreateItem(MakeDragonSoulVnum(bType, bGrade, bStep, bStrength + 1));
        if (nullptr == pResult)
        {
            SPDLOG_ERROR("INVALID DRAGON SOUL({})", MakeDragonSoulVnum(bType, bGrade, bStep, bStrength + 1));
            return false;
        }

        char buf[128];
        sprintf(buf, "STRENGTH : %d -> %d", bStrength, bStrength + 1);
        LogManager::instance().ItemLog(ch, pDragonSoul, "DS_STRENGTH_REFINE_SUCCESS", buf);

        pDragonSoul->RemoveFromCharacter();
        pDragonSoul->CopyAttributeTo(pResult);
        RefreshItemAttributes(pResult);
        ITEM_MANAGER::instance().RemoveItem(pDragonSoul);
        pRefineStone->SetCount(pRefineStone->GetCount() - 1);

        pResult = ch->AutoGiveItem(pResult, true);
        ITEM_MANAGER::instance().FlushDelayedSave(pResult);


        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "강화에 성공했습니다.");
        bSubHeader = DS_SUB_HEADER_REFINE_SUCCEED;
    }
    else
    {
        if (bStrength != 0)
        {
            pResult = ITEM_MANAGER::instance().CreateItem(MakeDragonSoulVnum(bType, bGrade, bStep, bStrength - 1));
            if (nullptr == pResult)
            {
                SPDLOG_ERROR("INVALID DRAGON SOUL({})", MakeDragonSoulVnum(bType, bGrade, bStep, bStrength - 1));
                return false;
            }
            pDragonSoul->CopyAttributeTo(pResult);
            RefreshItemAttributes(pResult);
        }
        bSubHeader = DS_SUB_HEADER_REFINE_FAIL;

        char buf[128];
        sprintf(buf, "STRENGTH : %d -> %d", bStrength, bStrength - 1);
        // strength강화는 실패시 깨질 수도 있어, 원본 아이템을 바탕으로 로그를 남김.
        LogManager::instance().ItemLog(ch, pDragonSoul, "DS_STRENGTH_REFINE_FAIL", buf);

        SendI18nChatPacket(ch, CHAT_TYPE_INFO, "강화에 실패했습니다.");
        pDragonSoul->SetCount(pDragonSoul->GetCount() - 1);
        pRefineStone->SetCount(pRefineStone->GetCount() - 1);
        if (nullptr != pResult)
            pResult = ch->AutoGiveItem(pResult, true);
    }

    SendRefineResultPacket(ch, bSubHeader,
                           nullptr == pResult ? NPOS : TItemPos(pResult->GetWindow(), pResult->GetCell()));

    return true;
}

void DSManager::SendRefineResultPacket(CHARACTER *ch, uint8_t bSubHeader, const TItemPos &pos)
{
    TPacketGCDragonSoulRefine pack;
    pack.bSubType = bSubHeader;

    if (pos.IsValidItemPosition()) { pack.Pos = pos; }
    DESC *d = ch->GetDesc();
    if (nullptr == d)
        return;

    d->Send(HEADER_GC_DRAGON_SOUL_REFINE, pack);
}

int DSManager::LeftTime(CItem *pItem) const
{
    if (pItem == nullptr)
        return false;

    return INT_MAX;
}

bool DSManager::IsTimeLeftDragonSoul(CItem *pItem) const
{
    if (pItem == nullptr)
        return false;

    return true;
}

bool DSManager::IsActiveDragonSoul(CItem *pItem) const { return pItem->GetSocket(ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX); }

bool DSManager::ActivateDragonSoul(CItem *pItem)
{
    if (nullptr == pItem)
        return false;
    CHARACTER *pOwner = pItem->GetOwner();
    if (nullptr == pOwner)
        return false;

    int deck_idx = pOwner->DragonSoul_GetActiveDeck();

    if (deck_idx < 0)
        return false;

    if (DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * deck_idx <= pItem->GetCell() &&
        pItem->GetCell() < DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * (deck_idx + 1))
    {
        if (IsTimeLeftDragonSoul(pItem) && !IsActiveDragonSoul(pItem))
        {
            char buf[128];
            sprintf(buf, "LEFT TIME(%d)", LeftTime(pItem));
            LogManager::instance().ItemLog(pOwner, pItem, "DS_ACTIVATE", buf);
            pItem->ModifyPoints(true);
            pItem->SetSocket(ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX, 1);

            pItem->StartTimerBasedOnWearExpireEvent();
        }
        return true;
    }
    else
        return false;
}

bool DSManager::DeactivateDragonSoul(CItem *pItem, bool bSkipRefreshOwnerActiveState)
{
    if (nullptr == pItem)
        return false;

    CHARACTER *pOwner = pItem->GetOwner();
    if (nullptr == pOwner)
        return false;

    if (!IsActiveDragonSoul(pItem))
        return false;

    char buf[128];
    pItem->StopTimerBasedOnWearExpireEvent();
    pItem->SetSocket(ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX, 0);
    pItem->ModifyPoints(false);

    sprintf(buf, "LEFT TIME(%d)", LeftTime(pItem));
    LogManager::instance().ItemLog(pOwner, pItem, "DS_DEACTIVATE", buf);

    if (!bSkipRefreshOwnerActiveState)
        RefreshDragonSoulState(pOwner);

    return true;
}

void DSManager::RefreshDragonSoulState(CHARACTER *ch)
{
    if (nullptr == ch)
        return;
    for (int i = DRAGON_SOUL_EQUIP_SLOT_START; i < DRAGON_SOUL_EQUIP_SLOT_START + DS_SLOT_MAX * DRAGON_SOUL_DECK_MAX_NUM
         ; i++)
    {
        CItem *pItem = ch->GetWear(i);
        if (pItem != nullptr) { if (IsActiveDragonSoul(pItem)) { return; } }
    }
    ch->DragonSoul_DeactivateAll();
}

#ifdef ENABLE_DS_SET
bool DSManager::GetDSSetGrade(CHARACTER *ch, uint8_t &iSetGrade)
{
    if (nullptr == ch)
        return false;

    const auto iDeckIdx = ch->DragonSoul_GetActiveDeck();
    const auto wDragonSoulDeckAffectType = AFFECT_DRAGON_SOUL_DECK_0 + iDeckIdx;
    if (!ch->FindAffect(wDragonSoulDeckAffectType))
        return false;

    const uint8_t iStartSlotIndex = DRAGON_SOUL_EQUIP_SLOT_START + (iDeckIdx * DS_SLOT_MAX);
    const uint8_t iEndSlotIndex = iStartSlotIndex + DS_SLOT_MAX;

    iSetGrade = 0;

    for (uint8_t iSlotIndex = iStartSlotIndex; iSlotIndex < iEndSlotIndex; ++iSlotIndex)
    {
        auto *pkItem = ch->GetWear(iSlotIndex);
        if (!pkItem)
            return false;

        if (!pkItem->IsDragonSoul())
            return false;

        if (!IsTimeLeftDragonSoul(pkItem))
            return false;

        if (!IsActiveDragonSoul(pkItem))
            return false;

        const uint8_t iGrade = GetGradeIdx(pkItem->GetVnum());
        if (iGrade < DRAGON_SOUL_GRADE_ANCIENT)
            return false;

        const auto iStep = GetStepIdx(pkItem->GetVnum());
        if (iStep != DRAGON_SOUL_STEP_HIGHEST)
            return false;

        if (iSetGrade == 0) { iSetGrade = iGrade; }

        if (iSetGrade != iGrade) { return false; }
    }

    return true;
}

ApplyValue DSManager::GetDSSetValue(uint8_t iAttributeIndex, uint16_t iApplyType, uint32_t iVnum, uint8_t iSetGrade)
{
    const uint8_t iType = GetType(iVnum);

    return m_pTable.GetDSSetValue(iAttributeIndex, iApplyType, iType, iSetGrade);
}
#endif

DSManager::DSManager()
{
}

DSManager::~DSManager()
{
}
