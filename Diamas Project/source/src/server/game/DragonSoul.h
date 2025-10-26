#ifndef METIN2_SERVER_GAME_DRAGONSOUL_H
#define METIN2_SERVER_GAME_DRAGONSOUL_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/Singleton.hpp>
#include <game/DbPackets.hpp>

class CHARACTER;
class CItem;

#include <game/DragonSoulTable.hpp>

class DSManager : public singleton<DSManager>
{
public:
    DSManager();
    ~DSManager();

    bool ReadDragonSoulTableFile(const char *c_pszFileName);

    void ClearDragonSoulTable();

    void GetDragonSoulInfo(uint32_t dwVnum, uint8_t &bType, uint8_t &bGrade, uint8_t &bStep, uint8_t &bRefine) const;
    bool IsValidCellForThisItem(const TItemTable *pItem, const TItemPos &Cell);
    // fixme : titempos로
    uint16_t GetBasePosition(const CItem *pItem) const;
    uint16_t GetBasePosition(const TItemTable *pItem);
    bool IsValidCellForThisItem(const CItem *pItem, const TItemPos &Cell) const;
    int GetDuration(const CItem *pItem) const;

    // 용혼석을 받아서 특정 용심을 추출하는 함수
    bool ExtractDragonHeart(CHARACTER *ch, CItem *pItem, CItem *pExtractor = nullptr);

    // 특정 용혼석(pItem)을 장비창에서 제거할 때에 성공 여부를 결정하고, 
    // 실패시 부산물을 주는 함수.(부산물은 dragon_soul_table.txt에 정의)
    // DestCell에 invalid한 값을 넣으면 성공 시, 용혼석을 빈 공간에 자동 추가.
    // 실패 시, 용혼석(pItem)은 delete됨.
    // 추출아이템이 있다면 추출 성공 확률이 pExtractor->GetValue(0)%만큼 증가함.
    // 부산물은 언제나 자동 추가.
    bool PullOut(CHARACTER *ch, TItemPos DestCell, CItem *&pItem, CItem *pExtractor = nullptr);

    // 용혼석 업그레이드 함수
    bool DoRefineGrade(CHARACTER *ch, TItemPos (&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE]);
    bool DoRefineStep(CHARACTER *ch, TItemPos (&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE]);
    bool DoRefineStrength(CHARACTER *ch, TItemPos (&aItemPoses)[DRAGON_SOUL_REFINE_GRID_SIZE]);

    bool DragonSoulItemInitialize(CItem *pItem);

    bool IsTimeLeftDragonSoul(CItem *pItem) const;
    int LeftTime(CItem *pItem) const;
    bool ActivateDragonSoul(CItem *pItem);
    bool DeactivateDragonSoul(CItem *pItem, bool bSkipRefreshOwnerActiveState = false);
    bool IsActiveDragonSoul(CItem *pItem) const;
#ifdef ENABLE_DS_SET
    bool GetDSSetGrade(CHARACTER *ch, uint8_t &iSetGrade);
    ApplyValue GetDSSetValue(uint8_t iAttributeIndex, uint16_t iApplyType, uint32_t iVnum, uint8_t iSetGrade);
#endif
private:
    void SendRefineResultPacket(CHARACTER *ch, uint8_t bSubHeader, const TItemPos &pos);

    // 캐릭터의 용혼석 덱을 살펴보고, 활성화 된 용혼석이 없다면, 캐릭터의 용혼석 활성 상태를 off 시키는 함수.
    void RefreshDragonSoulState(CHARACTER *ch);

    uint32_t MakeDragonSoulVnum(uint8_t bType, uint8_t grade, uint8_t step, uint8_t refine);
    bool PutAttributes(CItem *pDS);
    bool RefreshItemAttributes(CItem *pItem);

    DragonSoulTable m_pTable;
};

#endif /* METIN2_SERVER_GAME_DRAGONSOUL_H */
