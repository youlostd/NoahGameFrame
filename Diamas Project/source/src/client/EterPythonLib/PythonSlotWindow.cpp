#include "PythonSlotWindow.h"
#include "PythonWindow.h"

#include "../EterBase/StepTimer.h"
#include "../EterLib/GrpImage.h"

constexpr bool renderSlotArea = false;

using namespace UI;

class UI::CSlotWindow::CSlotButton : public CButton
{
  public:
    enum ESlotButtonType
    {
        SLOT_BUTTON_TYPE_PLUS,
        SLOT_BUTTON_TYPE_COVER,
    };

  public:
    CSlotButton(ESlotButtonType dwType, DWORD dwSlotNumber, CSlotWindow *pParent) : CButton()
    {
        m_dwSlotButtonType = dwType;
        m_dwSlotNumber = dwSlotNumber;
        m_pParent = pParent;
    }

    bool OnMouseLeftButtonDown() override
    {
        if (!IsEnable())
            return false;

        if (CWindowManager::Instance().IsAttaching())
            return false;

        m_isPressed = true;
        Down();
        return false;
    }

    bool OnMouseLeftButtonUp() override
    {
        if (!IsEnable())
        {
            return TRUE;
        }
        if (!IsPressed())
        {
            return TRUE;
        }

        if (IsIn())
        {
            SetCurrentVisual(&m_overVisual);
        }
        else
        {
            SetCurrentVisual(&m_upVisual);
        }

        m_pParent->OnPressedSlotButton(m_dwSlotButtonType, m_dwSlotNumber);

        return TRUE;
    }

    bool OnMouseOverIn()
    {
        if (IsEnable())
        {
            SetCurrentVisual(&m_overVisual);
        }
        m_pParent->OnOverInItem(m_dwSlotNumber);
        return true;
    }

    bool OnMouseOverOut() override
    {
        if (IsEnable())
        {
            SetUp();
            SetCurrentVisual(&m_upVisual);
        }
        m_pParent->OnOverOutItem();
        return true;
    }

  protected:
    ESlotButtonType m_dwSlotButtonType;
    DWORD m_dwSlotNumber;
    CSlotWindow *m_pParent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class UI::CSlotWindow::CCoverButton : public CButton
{
  public:
    CCoverButton(CSlotWindow *pParent, uint32_t dwSlotNumber) : m_bLeftButtonEnable{true}, m_bRightButtonEnable{false}, m_pParent{pParent}
    {
        m_dwSlotNumber = dwSlotNumber;

    }

    void SetLeftButtonEnable(bool bEnable)
    {
        m_bLeftButtonEnable = bEnable;
    }

    void SetRightButtonEnable(bool bEnable)
    {
        m_bRightButtonEnable = bEnable;
    }

    void OnRender() override
    {
    }

    void RenderButton()
    {
        CButton::OnRender();
    }

    bool OnMouseLeftButtonDown() override
    {
        if (!IsEnable())
            return false;

        if (m_bLeftButtonEnable)
            CButton::OnMouseLeftButtonDown();
        else
            m_pParent->OnMouseLeftButtonDown();

        return false;
    }

    bool OnMouseLeftButtonUp() override
    {
        if (!IsEnable())
        {
            return TRUE;
        }
        if (m_bLeftButtonEnable)
        {
            CButton::OnMouseLeftButtonUp();
            m_pParent->OnMouseLeftButtonDown();
        }
        else
        {
            m_pParent->OnMouseLeftButtonUp();
        }
        return TRUE;
    }

    bool OnMouseRightButtonDown() override
    {
        if (!IsEnable())
        {
            return false;
        }
        if (m_bRightButtonEnable)
        {
            CButton::OnMouseLeftButtonDown();
        }
        return false;
    }

    bool OnMouseRightButtonUp() override
    {
        if (!IsEnable())
        {
            return TRUE;
        }
        m_pParent->OnMouseRightButtonDown();
        if (m_bRightButtonEnable)
        {
            CButton::OnMouseLeftButtonUp();
        }
        return TRUE;
    }

    bool OnMouseOverIn()
    {
        if (IsEnable())
        {
            SetCurrentVisual(&m_overVisual);
        }
        m_pParent->OnOverInItem(m_dwSlotNumber);
        return true;
    }

    bool OnMouseOverOut() override
    {
        if (IsEnable())
        {
            SetUp();
            SetCurrentVisual(&m_upVisual);
        }
        m_pParent->OnOverOutItem();
        return true;
    }

    void SetScale(float fx, float fy);

  protected:
    bool m_bLeftButtonEnable;
    bool m_bRightButtonEnable;
    uint32_t m_dwSlotNumber;
    CSlotWindow *m_pParent;
};

void CSlotWindow::CCoverButton::SetScale(float fx, float fy)
{
    if (m_pcurVisual)
    {
        m_pcurVisual->SetScale(fx, fy);
    }

    if (!m_upVisual.IsEmpty())
    {
        m_upVisual.SetScale(fx, fy);
    }

    if (!m_overVisual.IsEmpty())
    {
        m_overVisual.SetScale(fx, fy);
    }

    if (!m_downVisual.IsEmpty())
    {
        m_downVisual.SetScale(fx, fy);
    }

    if (!m_disableVisual.IsEmpty())
    {
        m_disableVisual.SetScale(fx, fy);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class UI::CSlotWindow::CCoolTimeFinishEffect : public CAniImageBox
{
  public:
    CCoolTimeFinishEffect(CSlotWindow *pParent, DWORD dwSlotIndex) : CAniImageBox()
    {
        m_parent = pParent;
        m_dwSlotIndex = dwSlotIndex;
    }

    virtual ~CCoolTimeFinishEffect()
    {
    }

    void OnEndFrame()
    {
        ((CSlotWindow *)m_parent)->ReserveDestroyCoolTimeFinishEffect(m_dwSlotIndex);
    }

  protected:
    DWORD m_dwSlotIndex;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Set & Append

void CSlotWindow::SetWindowType(int iType)
{
    m_iWindowType = iType;
}

void CSlotWindow::SetSlotType(DWORD dwType)
{
    m_dwSlotType = dwType;
}

void CSlotWindow::SetSlotStyle(DWORD dwStyle)
{
    m_dwSlotStyle = dwStyle;
}

void UI::CSlotWindow::SetDiffuseColor(DWORD dwIndex, BYTE colorType)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    switch (colorType)
    {
    case SLOT_COLOR_TYPE_ORANGE:
        pSlot->sDiffuseColor.x = (1.0f);
        pSlot->sDiffuseColor.y = 0.34509805f;
        pSlot->sDiffuseColor.z = (0.035294119f);
        pSlot->sDiffuseColor.w = (0.5f);
        break;
    case SLOT_COLOR_TYPE_RED:
        pSlot->sDiffuseColor.x = (1.0f);
        pSlot->sDiffuseColor.y = (0.0f);
        pSlot->sDiffuseColor.z = (0.0f);
        pSlot->sDiffuseColor.w = (0.5f);
        break;
    case SLOT_COLOR_TYPE_GREEN:
        pSlot->sDiffuseColor.x = (0.0f);
        pSlot->sDiffuseColor.y = (1.0f);
        pSlot->sDiffuseColor.z = (0.0f);
        pSlot->sDiffuseColor.w = (0.5f);
        break;
    case SLOT_COLOR_TYPE_YELLOW:
        pSlot->sDiffuseColor.x = (1.0f);
        pSlot->sDiffuseColor.y = (1.0f);
        pSlot->sDiffuseColor.z = (0.0f);
        pSlot->sDiffuseColor.w = (0.5f);
        break;
    case SLOT_COLOR_TYPE_SKY:
        pSlot->sDiffuseColor.x = (0.0f);
        pSlot->sDiffuseColor.y = (1.0f);
        pSlot->sDiffuseColor.z = (1.0f);
        pSlot->sDiffuseColor.w = (0.5f);
        break;
    case SLOT_COLOR_TYPE_PINK:
        pSlot->sDiffuseColor.x = (1.0f);
        pSlot->sDiffuseColor.y = (0.0f);
        pSlot->sDiffuseColor.z = (1.0f);
        pSlot->sDiffuseColor.w = (0.5f);
        break;
    case SLOT_COLOR_TYPE_WHITE:
        pSlot->sDiffuseColor.x = (1.0f);
        pSlot->sDiffuseColor.y = (1.0f);
        pSlot->sDiffuseColor.z = (1.0f);
        pSlot->sDiffuseColor.w = (0.5f);
        break;
    case SLOT_COLOR_TYPE_BLUE:
        pSlot->sDiffuseColor.x = (0.0f);
        pSlot->sDiffuseColor.y = (0.0f);
        pSlot->sDiffuseColor.z = (1.0f);
        pSlot->sDiffuseColor.w = (0.5f);
        break;
    default:
        pSlot->sDiffuseColor.x = (1.0f);
        pSlot->sDiffuseColor.y = (1.0f);
        pSlot->sDiffuseColor.z = (1.0f);
        pSlot->sDiffuseColor.w = (0.5f);
        break;
    }
}

void UI::CSlotWindow::SetItemDiffuseColor(DWORD dwIndex, float r, float g, float b, float a)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (pSlot->pInstance)
    {
        pSlot->pInstance->SetDiffuseColor(r, g, b, a);
    }
}

void CSlotWindow::SetSlotBaseImageScale(float fx, float fy)
{
    if (m_pBaseImageInstance)
    {
        m_pBaseImageInstance->SetScale(fx, fy);
    }
}

void CSlotWindow::SetScale(float fx, float fy)
{
    m_v2Scale.x = fx;
    m_v2Scale.y = fy;
}

void CSlotWindow::SetCoverButtonScale(DWORD dwIndex, float fx, float fy)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (pSlot->pCoverButton)
    {
        pSlot->pCoverButton->SetScale(fx, fy);
    }
}

void CSlotWindow::SetSlotCoverImage(int iSlotIndex, const char *c_szFilename)
{
    if (!m_pCoverSlotImage)
    {
        __CreateCoverSlotImage(c_szFilename);
    }

    TSlot *pSlot;
    if (!GetSlotPointer(iSlotIndex, &pSlot))
    {
        return;
    }

    pSlot->bEnableSlotCoverImage = 1;
}

void CSlotWindow::EnableSlotCoverImage(int iSlotIndex, bool bIsEnable)
{
    TSlot *pSlot;
    if (!GetSlotPointer(iSlotIndex, &pSlot))
    {
        return;
    }

    pSlot->bEnableSlotCoverImage = bIsEnable != 0;
}

void CSlotWindow::AppendSlot(DWORD dwIndex, int ixPosition, int iyPosition, int ixCellSize, int iyCellSize,
                             int placementX, int placementY)
{
    TSlot Slot{};
    Slot.pInstance = nullptr;
    Slot.pBackgroundInstance = nullptr;
    Slot.pNumberLine.reset();
    Slot.pText.reset();
    Slot.pCoverButton = nullptr;
    Slot.pSlotButton = nullptr;
    Slot.pSignImage = nullptr;
    Slot.pFinishCoolTimeEffect = nullptr;

    ClearSlot(&Slot);
    Slot.dwSlotNumber = dwIndex;
    Slot.dwCenterSlotNumber = dwIndex;
    Slot.dwRealSlotNumber = dwIndex;
    Slot.dwRealCenterSlotNumber = dwIndex;
    Slot.ixPosition = ixPosition;
    Slot.iyPosition = iyPosition;
    Slot.ixCellSize = ixCellSize;
    Slot.iyCellSize = iyCellSize;
    Slot.ixPlacementPosition = placementX;
    Slot.iyPlacementPosition = placementY;
    m_SlotList.push_back(std::move(Slot));
}

void CSlotWindow::SetCoverButton(DWORD dwIndex, const char *c_szUpImageName, const char *c_szOverImageName,
                                 const char *c_szDownImageName, const char *c_szDisableImageName,
                                 bool bLeftButtonEnable, bool bRightButtonEnable)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    CCoverButton *&rpCoverButton = pSlot->pCoverButton;

    if (!rpCoverButton)
    {
        rpCoverButton = new CCoverButton(this, pSlot->dwSlotNumber);
        CWindowManager::Instance().SetParent(rpCoverButton, this);
    }

    rpCoverButton->SetLeftButtonEnable(bLeftButtonEnable);
    rpCoverButton->SetRightButtonEnable(bRightButtonEnable);
    rpCoverButton->SetUpVisual(c_szUpImageName);
    rpCoverButton->SetOverVisual(c_szOverImageName);
    rpCoverButton->SetDownVisual(c_szDownImageName);
    rpCoverButton->SetDisableVisual(c_szDisableImageName);
    rpCoverButton->Enable();
    rpCoverButton->SetScale(m_v2Scale.x, m_v2Scale.y);
    rpCoverButton->Show();

    // NOTE : Cover ¹öÆ°ÀÌ Plus ¹öÆ°À» °¡·Á¹ö·Á¼­ ÀÓ½Ã ÄÚµå¸¦..
    if (pSlot->pSlotButton)
    {
        SetTop(pSlot->pSlotButton);
    }
}

void CSlotWindow::EnableCoverButton(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (!pSlot->pCoverButton)
    {
        return;
    }

    pSlot->pCoverButton->Enable();
}

void CSlotWindow::DisableCoverButton(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (!pSlot->pCoverButton)
    {
        return;
    }

    pSlot->pCoverButton->Disable();
}

void CSlotWindow::SetAlwaysRenderCoverButton(DWORD dwIndex, bool bAlwaysRender)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (bAlwaysRender)
    {
        SET_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
    }
    else
    {
        REMOVE_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
    }
}

void CSlotWindow::ShowSlotBaseImage(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->bRenderBaseSlotImage = true;
}

void CSlotWindow::HideSlotBaseImage(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->bRenderBaseSlotImage = false;
}

bool CSlotWindow::IsDisableCoverButton(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return FALSE;
    }

    if (!pSlot->pCoverButton)
    {
        return FALSE;
    }

    return pSlot->pCoverButton->IsDisable();
}

void CSlotWindow::SetSlotBaseImage(const char *c_szFileName, float fr, float fg, float fb, float fa)
{
    __CreateBaseImage(c_szFileName, fr, fg, fb, fa);
}

void CSlotWindow::SetSlotButton(uint32_t index, const char *image, const char *hoverImage, const char *pressImage)
{
    TSlot *slot;
    if (!GetSlotPointer(index, &slot))
        return;

    auto &slotButton = slot->pSlotButton;
    if (!slotButton)
    {
        slotButton = new CSlotButton(CSlotButton::SLOT_BUTTON_TYPE_PLUS, slot->dwSlotNumber, this);
        slotButton->AddFlag(FLAG_FLOAT);
        CWindowManager::Instance().SetParent(slotButton, this);
    }

    slotButton->SetUpVisual(image);
    slotButton->SetOverVisual(hoverImage);
    slotButton->SetDownVisual(pressImage);
    slotButton->SetPosition(slot->ixPosition + 1, slot->iyPosition + 19);
    slotButton->Hide();
}

void CSlotWindow::AppendSlotButton(const char *c_szUpImageName, const char *c_szOverImageName,
                                   const char *c_szDownImageName)
{
    for (auto &rSlot : m_SlotList)
    {
        auto &rpSlotButton = rSlot.pSlotButton;

        if (!rpSlotButton)
        {
            rpSlotButton = new CSlotButton(CSlotButton::SLOT_BUTTON_TYPE_PLUS, rSlot.dwSlotNumber, this);
            rpSlotButton->AddFlag(FLAG_FLOAT);
            CWindowManager::Instance().SetParent(rpSlotButton, this);
        }

        rpSlotButton->SetUpVisual(c_szUpImageName);
        rpSlotButton->SetOverVisual(c_szOverImageName);
        rpSlotButton->SetDownVisual(c_szDownImageName);
        rpSlotButton->SetPosition(rSlot.ixPosition + 1, rSlot.iyPosition + 19);
        rpSlotButton->Hide();
    }
}

void CSlotWindow::AppendRequirementSignImage(const char *c_szImageName)
{
    for (auto &rSlot : m_SlotList)
    {
        CImageBox *&rpSignImage = rSlot.pSignImage;

        if (!rpSignImage)
        {
            rpSignImage = new CImageBox();
            CWindowManager::Instance().SetParent(rpSignImage, this);
        }

        rpSignImage->LoadImage(c_szImageName);
        rpSignImage->Hide();
    }
}

bool CSlotWindow::HasSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return FALSE;
    }

    return TRUE;
}

void CSlotWindow::SetSlot(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, CGraphicImage::Ptr pImage,
                          Color diffuseColor)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (pSlot->isItem)
        if (pSlot->dwItemIndex == dwVirtualNumber)
        {
            pSlot->dwState = 0;
            pSlot->isItem = TRUE;
            if (pImage && pSlot->pInstance)
            {
                pSlot->pInstance->SetImagePointer(pImage);
                pSlot->pInstance->SetScale(m_v2Scale.x, m_v2Scale.y);
            }
            return;
        }

    ClearSlot(pSlot);
    pSlot->dwState = 0;
    pSlot->isItem = TRUE;
    pSlot->dwItemIndex = dwVirtualNumber;

    if (pImage)
    {
        assert(nullptr == pSlot->pInstance);
        pSlot->pInstance.reset(new CGraphicImageInstance);
        pSlot->pInstance->SetDiffuseColor(diffuseColor);
        pSlot->pInstance->SetImagePointer(pImage);
        pSlot->pInstance->SetScale(m_v2Scale.x, m_v2Scale.y);
    }

    pSlot->byxPlacedItemSize = byWidth;
    pSlot->byyPlacedItemSize = byHeight;

    if (pSlot->pCoverButton)
    {
        pSlot->pCoverButton->SetScale(m_v2Scale.x, m_v2Scale.y);
        pSlot->pCoverButton->Show();
    }
}

//void CSlotWindow::SetCardSlot(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight,
//                              const char *c_szFileName, DirectX::SimpleMath::Color &diffuseColor)
//{
//    auto pImage = CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName);
//    TSlot *pSlot;
//    if (!GetSlotPointer(dwIndex, &pSlot))
//    {
//        return;
//    }
//
//    if (pSlot->isItem)
//        if (pSlot->dwItemIndex == dwVirtualNumber)
//        {
//            pSlot->dwState = 0;
//            pSlot->isItem = TRUE;
//            if (pImage && pSlot->pInstance)
//            {
//                pSlot->pInstance->SetImagePointer(pImage);
//            }
//            return;
//        }
//
//    ClearSlot(pSlot);
//    pSlot->dwState = 0;
//    pSlot->isItem = TRUE;
//    pSlot->dwItemIndex = dwVirtualNumber;
//
//    if (pImage)
//    {
//        assert(nullptr == pSlot->pInstance);
//        pSlot->pInstance.reset(new CGraphicImageInstance);
//        pSlot->pInstance->SetImagePointer(pImage);
//        pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
//    }
//
//    pSlot->byxPlacedItemSize = byWidth;
//    pSlot->byyPlacedItemSize = byHeight;
//
//    if (pSlot->pCoverButton)
//    {
//        pSlot->pCoverButton->Show();
//    }
//}

void CSlotWindow::SetSlotCount(DWORD dwIndex, DWORD dwCount)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (dwCount <= 0)
    {
        pSlot->pNumberLine.reset();
    }
    else
    {
        char szCount[16 + 1];
        _snprintf(szCount, sizeof(szCount), "%u", dwCount);

        if (!pSlot->pNumberLine)
        {
            auto pNumberLine = std::make_unique<UI::CNumberLine>(this);
            pNumberLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
            pNumberLine->Show();
            pSlot->pNumberLine = std::move(pNumberLine);
        }

        pSlot->pNumberLine->SetNumber(szCount);
    }
}

void CSlotWindow::SetSlotCountNew(DWORD dwIndex, DWORD dwGrade, DWORD dwCount)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (dwCount <= 0)
    {
        pSlot->pNumberLine.reset();
    }
    else
    {
        char szCount[16 + 1]{};

        switch (dwGrade)
        {
        case 0:
            _snprintf(szCount, sizeof(szCount), "%u", dwCount);
            break;
        case 1:
            _snprintf(szCount, sizeof(szCount), "m%u", dwCount);
            break;
        case 2:
            _snprintf(szCount, sizeof(szCount), "g%u", dwCount);
            break;
        case 3:
            _snprintf(szCount, sizeof(szCount), "p%u", dwCount);
            break;
        case 4:
            _snprintf(szCount, sizeof(szCount), "l");
            break;
        }

        if (!pSlot->pNumberLine)
        {
            auto pNumberLine = std::make_unique<UI::CNumberLine>(this);
            pNumberLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
            pNumberLine->Show();
            pSlot->pNumberLine = std::move(pNumberLine);
        }

        pSlot->pNumberLine->SetNumber(szCount);
    }
}

void CSlotWindow::SetSlotSlotNumber(DWORD dwIndex, DWORD dwGrade, DWORD dwCount)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (!pSlot->pInstance)
        return;

    if (dwCount <= 0)
    {
        if (pSlot->pText)
        {
            pSlot->pText.reset();
        }
    }
    else
    {
        char szCount[16 + 1]{};

        switch (dwGrade)
        {
        case 0:
            _snprintf(szCount, sizeof(szCount), "%u", dwCount);
            break;
        case 1:
            _snprintf(szCount, sizeof(szCount), "M%u", dwCount);
            break;
        case 2:
            _snprintf(szCount, sizeof(szCount), "G%u", dwCount);
            break;
        case 3:
            _snprintf(szCount, sizeof(szCount), "P%u", dwCount);
            break;
        case 4:
            _snprintf(szCount, sizeof(szCount), "L");
            break;
        }

        if (!pSlot->pText)
        {
            auto pText = std::make_unique<UI::CTextLine>();
            pText->SetFontName("Nunito Sans:18b");
            pText->SetFontGradient(0xff7a7a7a, 0xffe6e6e6);
            pText->SetOutline(true);
            pText->Show();
            pText->SetParentForce(this);

            pSlot->pText = std::move(pText);
        }

        pSlot->pText->SetText(szCount);
    }
}

void UI::CSlotWindow::SetSlotSlotText(DWORD dwIndex, const std::string &text)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    if (!pSlot->pInstance)
        return;

    if (text.empty())
    {
        if (pSlot->pText)
        {
            pSlot->pText.reset();
        }
    }
    else
    {
        if (!pSlot->pText)
        {
            auto pText = std::make_unique<UI::CTextLine>();
            pText->SetFontName("Nunito Sans:18b");
            pText->SetFontGradient(0xff7a7a7a, 0xffe6e6e6);
            pText->SetOutline(true);
            pText->Show();
            pText->SetParentForce(this);

            pSlot->pText = std::move(pText);
        }

        pSlot->pText->SetText(text);
    }
}

void CSlotWindow::SetRealSlotNumber(DWORD dwIndex, DWORD dwSlotRealNumber)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->dwRealSlotNumber = dwSlotRealNumber;
}

void CSlotWindow::SetSlotCoolTime(DWORD dwIndex, float fCoolTime, float fElapsedTime)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
        return;

    pSlot->fCoolTime = fCoolTime;
    pSlot->fStartCoolTime = DX::StepTimer::Instance().GetTotalSeconds() - fElapsedTime;
}

void CSlotWindow::StoreSlotCoolTime(DWORD dwKey, DWORD dwSlotIndex, float fCoolTime, float fElapsedTime)
{
    const float now = DX::StepTimer::Instance().GetTotalSeconds();
    auto it = m_CoolDownStore[dwKey].find(dwSlotIndex);
    if (it != m_CoolDownStore[dwKey].end())
    {
        it->second.fCoolTime = fCoolTime;
        it->second.fElapsedTime = now - fElapsedTime;
        it->second.bActive = false;
    }
    else
    {
        SStoreCoolDown m_storeCoolDown{};
        m_storeCoolDown.fCoolTime = fCoolTime;

        m_storeCoolDown.fElapsedTime = now - fElapsedTime;
        m_storeCoolDown.bActive = false;
        m_CoolDownStore[dwKey].emplace(dwSlotIndex, m_storeCoolDown);
    }
}

void CSlotWindow::RestoreSlotCoolTime(DWORD dwKey)
{
    for (auto it = m_CoolDownStore[dwKey].begin(); it != m_CoolDownStore[dwKey].end(); ++it)
    {
        TSlot *pSlot;
        if (!GetSlotPointer(it->first, &pSlot))
            return;

        auto &storedCoolDown = it->second;

        pSlot->fCoolTime = storedCoolDown.fCoolTime;
        pSlot->fStartCoolTime = storedCoolDown.fElapsedTime;
        if (storedCoolDown.bActive)
            ActivateSlot(it->first);
        else
            DeactivateSlot(it->first);
    }
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
void CSlotWindow::EnableSlotCoverImage(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
        return;

    pSlot->bSlotCoverImage = TRUE;

    if (!m_pSlotCoverImage)
    {
        __CreateSlotCoverImage();
    }
}

void CSlotWindow::DisableSlotCoverImage(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
        return;

    pSlot->bSlotCoverImage = FALSE;
}
#endif

void CSlotWindow::SetSlotCoolTimeColor(DWORD dwIndex, float fCoolTime, float fElapsedTime, float fColorR, float fColorG,
                                       float fColorB, float fColorA)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->fCoolTime = fCoolTime;

    pSlot->fStartCoolTime = DX::StepTimer::Instance().GetTotalSeconds() - fElapsedTime;
    pSlot->sCoolTimeColor.R(fColorR);
    pSlot->sCoolTimeColor.G(fColorG);
    pSlot->sCoolTimeColor.B(fColorB);
    pSlot->sCoolTimeColor.A(fColorA);
}

void CSlotWindow::SetSlotCoolTimeInverse(DWORD dwIndex, float fCoolTime, float fElapsedTime)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->fCoolTime = fCoolTime;

    pSlot->fStartCoolTime = DX::StepTimer::Instance().GetTotalSeconds() - fElapsedTime;
    pSlot->bIsInverseCoolTime = true;
}

void CSlotWindow::SetSlotExpireTime(DWORD dwIndex, float fExpireTime, float fMaxTime)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->fExpireTime = fExpireTime;
    pSlot->fExpireMaxTime = fMaxTime;
}

void CSlotWindow::ActivateSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->bActive = TRUE;

    int8_t itemSize = pSlot->byyPlacedItemSize - 1;
    if (itemSize < 0)
    {
        itemSize = 1;
    }

    if (!m_pSlotActiveEffect[itemSize])
    {
        __CreateSlotEnableEffect(itemSize);
    }

    auto it = m_CoolDownStore[1].find(dwIndex);
    if (it != m_CoolDownStore[1].end())
        it->second.bActive = true;
}

void CSlotWindow::DeactivateSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->bActive = FALSE;

    auto it = m_CoolDownStore[1].find(dwIndex);
    if (it != m_CoolDownStore[1].end())
        it->second.bActive = false;
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
void CSlotWindow::ActivateChangeLookSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
        return;

    pSlot->bChangeLookActive = TRUE;


}

void CSlotWindow::DeactivateChangeLookSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
        return;

    pSlot->bChangeLookActive = FALSE;
}
#endif

void CSlotWindow::ClearSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    ClearSlot(pSlot);
}

void CSlotWindow::ClearSlot(TSlot *pSlot)
{
    pSlot->bActive = FALSE;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
    pSlot->bChangeLookActive = FALSE;
    pSlot->bSlotCoverImage = FALSE;
#endif
    pSlot->byxPlacedItemSize = 1;
    pSlot->byyPlacedItemSize = 1;

    pSlot->isItem = FALSE;
    pSlot->dwState = 0;
    pSlot->fCoolTime = 0.0f;
    pSlot->fExpireTime = 0.0f;
    pSlot->fStartCoolTime = 0.0f;
    pSlot->dwCenterSlotNumber = 0xffffffff;

    pSlot->dwItemIndex = 0;
    pSlot->bRenderBaseSlotImage = true;

    pSlot->pInstance.reset();

    if (pSlot->pCoverButton)
    {
        pSlot->pCoverButton->Hide();
    }
    if (pSlot->pSlotButton)
    {
        pSlot->pSlotButton->Hide();
    }
    if (pSlot->pSignImage)
    {
        pSlot->pSignImage->Hide();
    }
    if (pSlot->pFinishCoolTimeEffect)
    {
        pSlot->pFinishCoolTimeEffect->Hide();
    }

    pSlot->sDiffuseColor = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 0.5f);
    pSlot->bIsInverseCoolTime = 0;
    pSlot->sCoolTimeColor = DirectX::SimpleMath::Color(0.0f, 0.0f, 0.0f, 0.5f);
}

void CSlotWindow::ClearAllSlot()
{
    for (auto &rSlot : m_SlotList)
    {
        ClearSlot(&rSlot);
    }
}

void CSlotWindow::RefreshSlot()
{
    OnRefreshSlot();

    // NOTE : Refresh µÉ¶§ ToolTip µµ °»½Å ÇÕ´Ï´Ù - [levites]
    if (IsRendering())
    {
        TSlot *pSlot;
        if (GetPickedSlotPointer(&pSlot))
        {
            OnOverOutItem();
            OnOverInItem(pSlot->dwSlotNumber);
        }
    }
}

void CSlotWindow::OnRefreshSlot()
{
}

DWORD CSlotWindow::GetSlotCount()
{
    return m_SlotList.size();
}

void CSlotWindow::LockSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->dwState |= SLOT_STATE_LOCK;
}

void CSlotWindow::UnlockSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->dwState ^= SLOT_STATE_LOCK;
}

void CSlotWindow::SetCantUseSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->dwState |= SLOT_STATE_CANT_USE;
}

void CSlotWindow::SetUseSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->dwState ^= SLOT_STATE_CANT_USE;
}

void CSlotWindow::SetCantMouseEventSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }
    LockSlot(dwIndex);

    SET_BIT(pSlot->dwState, SLOT_STATE_CANT_MOUSE_EVENT);
}

void CSlotWindow::SetCanMouseEventSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }
    UnlockSlot(dwIndex);

    REMOVE_BIT(pSlot->dwState, SLOT_STATE_CANT_MOUSE_EVENT);
}

void CSlotWindow::SetUnusableSlotOnTopWnd(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    SET_BIT(pSlot->dwState, SLOT_STATE_UNUSABLE_ON_TOP_WND);
}

void CSlotWindow::DisableSlotHighlightedGreen(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    REMOVE_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_GREEN);
}

void CSlotWindow::SetSlotHighlightedGreeen(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    SET_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_GREEN);
}

void CSlotWindow::SetSlotTextPosition(DWORD dwIndex, DWORD x, DWORD y)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->ixTextPosition = x;
    pSlot->iyTextPosition = y;
}

void CSlotWindow::GetSlotLocalPosition(DWORD dwIndex, DWORD *x, DWORD *y)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    *x = pSlot->ixPosition;
    *y = pSlot->iyPosition;
}

void CSlotWindow::GetSlotGlobalPosition(DWORD dwIndex, DWORD *x, DWORD *y)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    *x = m_rect.top + pSlot->ixPosition;
    *y = m_rect.right + pSlot->iyPosition;
}

void CSlotWindow::SetUsableSlotOnTopWnd(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    REMOVE_BIT(pSlot->dwState, SLOT_STATE_UNUSABLE_ON_TOP_WND);
}

void CSlotWindow::EnableSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    REMOVE_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
    // pSlot->dwState |= SLOT_STATE_DISABLE;
}

void CSlotWindow::DisableSlot(DWORD dwIndex)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }
    SET_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
    // pSlot->dwState ^= SLOT_STATE_DISABLE;
}

// Select

void CSlotWindow::SelectSlot(DWORD dwSelectingIndex)
{
    std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
    for (; itor != m_dwSelectedSlotIndexList.end();)
    {
        if (dwSelectingIndex == *itor)
        {
            itor = m_dwSelectedSlotIndexList.erase(itor);
            return;
        }
        else
        {
            ++itor;
        }
    }

    TSlot *pSlot;
    if (GetSlotPointer(dwSelectingIndex, &pSlot))
    {
        if (pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT)
            return;

        if (!pSlot->isItem)
        {
            return;
        }

        m_dwSelectedSlotIndexList.push_back(dwSelectingIndex);
    }
}

bool CSlotWindow::isSelectedSlot(DWORD dwIndex)
{
    std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
    for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
    {
        if (dwIndex == *itor)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void CSlotWindow::ClearSelected()
{
    m_dwSelectedSlotIndexList.clear();
}

DWORD CSlotWindow::GetSelectedSlotCount()
{
    return m_dwSelectedSlotIndexList.size();
}

DWORD CSlotWindow::GetSelectedSlotNumber(DWORD dwIndex)
{
    if (dwIndex >= m_dwSelectedSlotIndexList.size())
    {
        return DWORD(-1);
    }

    DWORD dwCount = 0;
    auto itor = m_dwSelectedSlotIndexList.begin();
    for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
    {
        if (dwIndex == dwCount)
        {
            break;
        }

        ++dwCount;
    }

    return *itor;
}

void CSlotWindow::ShowSlotButton(DWORD dwSlotNumber)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwSlotNumber, &pSlot))
    {
        return;
    }

    if (pSlot->pSlotButton)
    {
        pSlot->pSlotButton->Show();
    }
}

void CSlotWindow::HideAllSlotButton()
{
    for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
    {
        TSlot &rSlot = *itor;

        if (rSlot.pSlotButton)
        {
            rSlot.pSlotButton->Hide();
        }
    }
}

void CSlotWindow::SetSlotButtonPosition(DWORD dwSlotNumber, int x, int y)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwSlotNumber, &pSlot))
    {
        return;
    }

    if (pSlot->pSlotButton)
    {
        pSlot->ixSlotButtonPosition = x;
        pSlot->iySlotButtonPosition = y;
    }
}

void CSlotWindow::ShowRequirementSign(DWORD dwSlotNumber)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwSlotNumber, &pSlot))
    {
        return;
    }

    if (!pSlot->pSignImage)
    {
        return;
    }

    pSlot->pSignImage->Show();
}

void CSlotWindow::HideRequirementSign(DWORD dwSlotNumber)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwSlotNumber, &pSlot))
    {
        return;
    }

    if (!pSlot->pSignImage)
    {
        return;
    }

    pSlot->pSignImage->Hide();
}

// Event

bool CSlotWindow::OnMouseLeftButtonDown()
{
    TSlot *pSlot;
    if (!GetPickedSlotPointer(&pSlot))
    {
        UI::CWindowManager::Instance().DeattachIcon();
        return TRUE;
    }

    if (!(pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
    {
        if (pSlot->isItem && !(pSlot->dwState & SLOT_STATE_LOCK))
        {
            OnSelectItemSlot(pSlot->dwSlotNumber);
        }
        else
        {
            OnSelectEmptySlot(pSlot->dwSlotNumber);
        }
    }

    return TRUE;
}

bool CSlotWindow::OnMouseLeftButtonUp()
{

    if (!CWindowManager::Instance().IsAttaching() || !CWindowManager::Instance().IsDragging())
        return false;

    if (!IsIn())
        return false;

    TSlot *pSlot;
    if (!GetPickedSlotPointer(&pSlot))
    {
        CWindowManager::Instance().DeattachIcon();
        return true;
    }

    if (pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT)
        return true;

    if (pSlot->isItem)
        OnSelectItemSlot(pSlot->dwSlotNumber);
    else
        OnSelectEmptySlot(pSlot->dwSlotNumber);

    return true;
}

bool CSlotWindow::OnMouseRightButtonDown()
{
    TSlot *pSlot;
    if (!GetPickedSlotPointer(&pSlot))
    {
        return TRUE;
    }

    if (pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT)
        return TRUE;

    if (pSlot->isItem)
    {
        OnUnselectItemSlot(pSlot->dwSlotNumber);
    }
    else
    {
        OnUnselectEmptySlot(pSlot->dwSlotNumber);
    }

    return TRUE;
}

bool CSlotWindow::OnMouseLeftButtonDoubleClick()
{
    OnUseSlot();

    return TRUE;
}

bool CSlotWindow::OnMouseOverOut()
{
    OnOverOutItem();
    return true;
}

void CSlotWindow::OnMouseOver()
{
    // FIXME : À©µµ¿ì¸¦ µå·¡±ë ÇÏ´Â µµÁß¿¡ SetTopÀÌ µÇ¾î¹ö¸®¸é Capture°¡ Ç®¾îÁ® ¹ö¸°´Ù. ±×°ÍÀÇ ¹æÁö ÄÚµå.
    //         Á» ´õ ±Ùº»ÀûÀÎ ÇØ°áÃ¥À» Ã£¾Æ¾ß ÇÒ µí - [levites]
    //	if (UI::CWindowManager::Instance().IsCapture())
    //	if (!UI::CWindowManager::Instance().IsAttaching())
    //		return;

    CWindow *pPointWindow = UI::CWindowManager::Instance().GetPointWindow();
    if (this == pPointWindow)
    {
        TSlot *pSlot;
        if (GetPickedSlotPointer(&pSlot))
        {
            if (OnOverInItem(pSlot->dwSlotNumber))
            {
                return;
            }
        }
    }

    OnOverOutItem();
}

void CSlotWindow::OnSelectEmptySlot(int iSlotNumber)
{
    RunCallback("OnSelectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}

void CSlotWindow::OnSelectItemSlot(int iSlotNumber)
{
    RunCallback("OnSelectItemSlot", Py_BuildValue("(i)", iSlotNumber));

    if (UI::CWindowManager::Instance().IsAttaching())
        OnOverOutItem();
}

void CSlotWindow::OnUnselectEmptySlot(int iSlotNumber)
{
    RunCallback("OnUnselectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}

void CSlotWindow::OnUnselectItemSlot(int iSlotNumber)
{
    RunCallback("OnUnselectItemSlot", Py_BuildValue("(i)", iSlotNumber));
}

void CSlotWindow::OnUseSlot()
{
    TSlot *pSlot;
    if (GetPickedSlotPointer(&pSlot))
        if (pSlot->isItem && !(pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
        {
            RunCallback("OnUseSlot", Py_BuildValue("(i)", pSlot->dwSlotNumber));
        }
}

bool CSlotWindow::OnOverInItem(DWORD dwSlotNumber)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwSlotNumber, &pSlot))
    {
        return FALSE;
    }

    if (!pSlot->isItem)
    {
        return FALSE;
    }

    if (pSlot->dwSlotNumber == m_dwToolTipSlotNumber)
    {
        return TRUE;
    }

    m_dwToolTipSlotNumber = dwSlotNumber;
    RunCallback("OnOverInItem", Py_BuildValue("(i)", dwSlotNumber));

    return TRUE;
}

void CSlotWindow::OnOverOutItem()
{
    if (SLOT_NUMBER_NONE == m_dwToolTipSlotNumber)
    {
        return;
    }

    m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;
    RunCallback("OnOverOutItem", Py_BuildValue("()"));
}

void CSlotWindow::OnPressedSlotButton(DWORD dwType, DWORD dwSlotNumber, bool isLeft)
{
    if (CSlotButton::SLOT_BUTTON_TYPE_PLUS == dwType)
    {
        RunCallback("OnPressedSlotButton", Py_BuildValue("(i)", dwSlotNumber));
    }
    else if (CSlotButton::SLOT_BUTTON_TYPE_COVER == dwType)
    {
        if (isLeft)
        {
            OnMouseLeftButtonDown();
        }
    }
}

void CSlotWindow::OnUpdate()
{
    for (std::deque<DWORD>::iterator itor = m_ReserveDestroyEffectDeque.begin();
         itor != m_ReserveDestroyEffectDeque.end(); ++itor)
    {
        DWORD dwSlotIndex = *itor;

        TSlot *pSlot;
        if (!GetSlotPointer(dwSlotIndex, &pSlot))
        {
            continue;
        }

        __DestroyFinishCoolTimeEffect(pSlot);
    }
    m_ReserveDestroyEffectDeque.clear();

    for (int effectSize = 0; effectSize < 3; ++effectSize)
    {
        if (m_pSlotActiveEffect[effectSize])
        {
            m_pSlotActiveEffect[effectSize]->Update();
        }
    }
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
    for (int i = 0; i < 3; ++i)
    {
        if (m_pSlotChangeLookActiveEffect[i])
            m_pSlotChangeLookActiveEffect[i]->Update();
    }

    if (m_pSlotCoverImage)
        m_pSlotCoverImage->Update();
#endif
}

void CSlotWindow::OnRender()
{
    RenderSlotBaseImage();

    switch (m_dwSlotStyle)
    {
    case SLOT_STYLE_PICK_UP:
        OnRenderPickingSlot();
        break;
    case SLOT_STYLE_SELECT:
        OnRenderSelectedSlot();
        break;
    }

    //std::for_each(m_children.rbegin(), m_children.rend(), std::bind(&CWindow::OnRender, std::placeholders::_1));

    //
    // ¸ðµç ½½·Ô »óÀÚ ±×¸®±â
    //////////////////////////////////////////////////////////////////////////
    if constexpr (renderSlotArea)
    {
        CPythonGraphic::Instance().SetDiffuseColor(0.5f, 0.5f, 0.5f);
        for (auto itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
        {
            TSlot &rSlot = *itor;
            CPythonGraphic::Instance().RenderBox2d(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
                                                   m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
                                                   m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
        }
        CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);
        CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
    }

    //////////////////////////////////////////////////////////////////////////

    for (auto itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
    {
        TSlot &rSlot = *itor;

        if (rSlot.pSlotButton)
        {
            rSlot.pSlotButton->SetPosition(rSlot.ixPosition + rSlot.ixSlotButtonPosition,
                                           rSlot.iyPosition + rSlot.iySlotButtonPosition);
        }
        if (rSlot.pSignImage)
        {
            rSlot.pSignImage->SetPosition(rSlot.ixPosition - 7, rSlot.iyPosition + 10);
        }

        if (rSlot.pInstance)
        {
            auto x = m_rect.left + rSlot.ixPosition + rSlot.ixPlacementPosition;
            auto y = m_rect.top + rSlot.iyPosition + rSlot.iyPlacementPosition;

            rSlot.pInstance->SetPosition(x, y);
            rSlot.pInstance->Render();
        }

        if (!rSlot.isItem)
        {
            if (IS_SET(rSlot.dwState, SLOT_STATE_ALWAYS_RENDER_COVER))
            {
                rSlot.pCoverButton->Show();
                rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
                rSlot.pCoverButton->RenderButton();
            }

            continue;
        }

        if (IS_SET(rSlot.dwState, SLOT_STATE_DISABLE))
        {
            CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.3f);
            CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
                                                   m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
                                                   m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
        }

        if (IS_SET(rSlot.dwState, SLOT_STATE_CANT_MOUSE_EVENT))
        {
            CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 0.3f);
            CPythonGraphic::Instance().RenderBar2d(
                m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
                m_rect.left + rSlot.ixPosition + rSlot.byxPlacedItemSize * rSlot.ixCellSize,
                m_rect.top + rSlot.iyPosition + rSlot.byyPlacedItemSize * rSlot.iyCellSize);
        }

        if (IS_SET(rSlot.dwState, SLOT_STATE_UNUSABLE_ON_TOP_WND))
        {
            CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.3f);

            CPythonGraphic::Instance().RenderBar2d(
                m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
                m_rect.left + rSlot.ixPosition + rSlot.byxPlacedItemSize * rSlot.ixCellSize,
                m_rect.top + rSlot.iyPosition + rSlot.byyPlacedItemSize * rSlot.iyCellSize);
        }

        if (IS_SET(rSlot.dwState, SLOT_STATE_HIGHLIGHT_GREEN))
        {
            CPythonGraphic::Instance().SetDiffuseColor(74.0f / 255.f, 1.0f, 74.0f / 255.0f, 0.3f);
            CPythonGraphic::Instance().RenderBar2d(
                m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
                m_rect.left + rSlot.ixPosition + rSlot.byxPlacedItemSize * rSlot.ixCellSize,
                m_rect.top + rSlot.iyPosition + rSlot.byyPlacedItemSize * rSlot.iyCellSize);
        }

        if (rSlot.fCoolTime != 0.0f)
        {
            const float fcurTime = DX::StepTimer::Instance().GetTotalSeconds();
            const float fPercentage = (fcurTime - rSlot.fStartCoolTime) / rSlot.fCoolTime;

            int32_t iCellY = rSlot.iyCellSize;
            int32_t iCellX = rSlot.ixCellSize;

            iCellX = std::min(iCellX, iCellY);

            auto fRadius = iCellX * 0.5f;
            auto fxCenter = m_rect.left + rSlot.ixPosition + iCellX * 0.5f;
            auto fyCenter = m_rect.top + rSlot.iyPosition + iCellY * 0.5f;

            if (!rSlot.bIsInverseCoolTime)
            {
                CPythonGraphic::Instance().RenderCoolTimeBoxColor(fxCenter, fyCenter, fRadius, fPercentage,
                                                                  rSlot.sCoolTimeColor.x, rSlot.sCoolTimeColor.y,
                                                                  rSlot.sCoolTimeColor.z, rSlot.sCoolTimeColor.w);
            }
            else
            {
                CPythonGraphic::Instance().RenderCoolTimeBoxInverse(fxCenter, fyCenter, fRadius, fPercentage);
            }

            if (!rSlot.bIsInverseCoolTime && fcurTime - rSlot.fStartCoolTime >= rSlot.fCoolTime)
            {
                // ÄðÅ¸ÀÓÀÌ ³¡³­Áö 1ÃÊ ÀÌ³»¶ó¸é..
                if ((fcurTime - rSlot.fStartCoolTime) - rSlot.fCoolTime < 1.0f)
                {
                    __CreateFinishCoolTimeEffect(&rSlot);
                }

                rSlot.fCoolTime = 0.0f;
                rSlot.fStartCoolTime = 0.0f;
            }
        }

        if (rSlot.fExpireTime != 0.0f)
        {
            float fPercentage = (rSlot.fExpireTime) / rSlot.fExpireMaxTime;
            CPythonGraphic::Instance().RenderCoolTimeBoxColor(
                m_rect.left + rSlot.ixPosition + 16.0f, m_rect.top + rSlot.iyPosition + 16.0f, 16.0f, fPercentage,
                rSlot.sCoolTimeColor.x, rSlot.sCoolTimeColor.y, rSlot.sCoolTimeColor.z, rSlot.sCoolTimeColor.w);
        }

        if (rSlot.pCoverButton)
        {
            if (rSlot.pCoverButton->GetPositionX() != rSlot.ixPosition ||
                rSlot.pCoverButton->GetPositionY() != rSlot.iyPosition)
                rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
            rSlot.pCoverButton->SetScale(m_v2Scale.x, m_v2Scale.y);
            rSlot.pCoverButton->UpdateRect();
            rSlot.pCoverButton->RenderButton();
        }

        if (rSlot.pText)
        {
            int ix = rSlot.ixPosition + rSlot.ixTextPosition;
            int iy = rSlot.iyPosition + rSlot.iyTextPosition;

            rSlot.pText->SetPosition(ix, iy);
            rSlot.pText->UpdateRect();
            rSlot.pText->Update();
            rSlot.pText->Render();
        }

        if (rSlot.pNumberLine)
        {
            int ix = rSlot.byxPlacedItemSize * ITEM_WIDTH + rSlot.ixPosition - 4;
            int iy = rSlot.iyPosition + rSlot.byyPlacedItemSize * ITEM_HEIGHT - 12 + 2;

            if (rSlot.pNumberLine->GetPositionX() != ix || rSlot.pNumberLine->GetPositionY() != iy)
                rSlot.pNumberLine->SetPosition(ix, iy);
            rSlot.pNumberLine->UpdateRect();
            rSlot.pNumberLine->Update();
            rSlot.pNumberLine->Render();
        }

        if (rSlot.pFinishCoolTimeEffect)
        {
            rSlot.pFinishCoolTimeEffect->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
            rSlot.pFinishCoolTimeEffect->Update();
            rSlot.pFinishCoolTimeEffect->Render();
        }

        if (rSlot.bActive)
        {
            WORD slotSize = std::max(0, rSlot.byyPlacedItemSize - 1);

            if (m_pSlotActiveEffect[slotSize])
            {
                int ix = m_rect.left + rSlot.ixPosition;
                int iy = m_rect.top + rSlot.iyPosition;
                m_pSlotActiveEffect[slotSize]->SetPosition(ix, iy);
                m_pSlotActiveEffect[slotSize]->SetDiffuseColor(rSlot.sDiffuseColor.x, rSlot.sDiffuseColor.y,
                                                               rSlot.sDiffuseColor.z, rSlot.sDiffuseColor.w);
                m_pSlotActiveEffect[slotSize]->Render();
            }
        }
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
        if (rSlot.bChangeLookActive)
        {
            int ix = m_rect.left + rSlot.ixPosition;
            int iy = m_rect.top + rSlot.iyPosition;
            int size = rSlot.byyPlacedItemSize;

            if (size == 1)
            {
                m_pSlotChangeLookActiveEffect[0]->Show();
                m_pSlotChangeLookActiveEffect[0]->SetPosition(ix, iy);
                m_pSlotChangeLookActiveEffect[0]->Render();
            }

            if (size == 2)
            {
                m_pSlotChangeLookActiveEffect[1]->Show();
                m_pSlotChangeLookActiveEffect[1]->SetPosition(ix, iy);
                m_pSlotChangeLookActiveEffect[1]->Render();
            }

            if (size == 3)
            {
                m_pSlotChangeLookActiveEffect[2]->Show();
                m_pSlotChangeLookActiveEffect[2]->SetPosition(ix, iy);
                m_pSlotChangeLookActiveEffect[2]->Render();
            }
        }

        if (rSlot.bSlotCoverImage)
        {
            if (m_pSlotCoverImage)
            {
                int ix = m_rect.left + rSlot.ixPosition;
                int iy = m_rect.top + rSlot.iyPosition;

                m_pSlotCoverImage->SetPosition(ix, iy);
                m_pSlotCoverImage->Render();
            }
        }
#endif
        if (rSlot.bEnableSlotCoverImage && m_pCoverSlotImage)
        {
            int ix = m_rect.left + rSlot.ixPosition;
            int iy = m_rect.top + rSlot.iyPosition;
            m_pCoverSlotImage->SetPosition(ix, iy);
            m_pCoverSlotImage->Render();
        }
    }

    RenderLockedSlot();
}

void CSlotWindow::RenderSlotBaseImage()
{
    if (!m_pBaseImageInstance)
    {
        return;
    }

    for (auto &rSlot : m_SlotList)
    {
        if (!rSlot.bRenderBaseSlotImage)
        {
            continue;
        }

        auto bgImageInstance = rSlot.pBackgroundInstance ? rSlot.pBackgroundInstance.get() : m_pBaseImageInstance.get();

        bgImageInstance->SetPosition(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition);
        bgImageInstance->Render();
    }
}

void CSlotWindow::OnRenderPickingSlot()
{
    if (!UI::CWindowManager::Instance().IsAttaching())
    {
        return;
    }

    TSlot *pSlot;
    if (!GetSelectedSlotPointer(&pSlot))
    {
        return;
    }

    CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
    CPythonGraphic::Instance().RenderBar2d(m_rect.left + pSlot->ixPosition, m_rect.top + pSlot->iyPosition,
                                           m_rect.left + pSlot->ixPosition + pSlot->ixCellSize,
                                           m_rect.top + pSlot->iyPosition + pSlot->iyCellSize);
}

void CSlotWindow::OnRenderSelectedSlot()
{
    std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
    for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
    {
        TSlot *pSlot;
        if (!GetSlotPointer(*itor, &pSlot))
        {
            continue;
        }

        CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f, 0.5f);
        CPythonGraphic::Instance().RenderBar2d(m_rect.left + pSlot->ixPosition, m_rect.top + pSlot->iyPosition,
                                               m_rect.left + pSlot->ixPosition + pSlot->ixCellSize,
                                               m_rect.top + pSlot->iyPosition + pSlot->iyCellSize);
    }
}

void CSlotWindow::RenderLockedSlot()
{
    CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.5f);
    for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
    {
        TSlot &rSlot = *itor;

        if (!rSlot.isItem)
        {
            continue;
        }

        if (rSlot.dwState & SLOT_STATE_LOCK)
        {
            CPythonGraphic::Instance().RenderBar2d(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition,
                                                   m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
                                                   m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
        }
    }
}

// Check Slot

bool CSlotWindow::GetSlotPointer(DWORD dwIndex, TSlot **ppSlot)
{
    for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
    {
        TSlot &rSlot = *itor;

        if (dwIndex == rSlot.dwSlotNumber)
        {
            *ppSlot = &rSlot;
            return TRUE;
        }
    }

    return FALSE;
}

bool CSlotWindow::GetSelectedSlotPointer(TSlot **ppSlot)
{
    auto [lx, ly] = CWindowManager::Instance().GetMousePosition();

    MakeLocalPosition(lx, ly);

    for (TSlot &rSlot : m_SlotList)
    {
        if (lx >= rSlot.ixPosition && ly >= rSlot.iyPosition && lx <= rSlot.ixPosition + rSlot.ixCellSize &&
            ly <= rSlot.iyPosition + rSlot.iyCellSize)
        {
            *ppSlot = &rSlot;
            return true;
        }
    }

    return FALSE;
}

bool CSlotWindow::GetPickedSlotPointer(TSlot **ppSlot)
{
    auto [lx, ly] = CWindowManager::Instance().GetMousePosition();

    int ixLocal = lx - m_rect.left;
    int iyLocal = ly - m_rect.top;

    // NOTE : 왼쪽 맨위 상단 한곳이 기준 이라는 점을 이용해 왼쪽 위에서부터 오른쪽 아래로
    //        차례로 검색해 감으로써 덮혀 있는 Slot은 자동 무시 된다는 특성을 이용한다. - [levites]
    for (TSlot &rSlot : m_SlotList)
    {
        int ixCellSize = rSlot.ixCellSize;
        int iyCellSize = rSlot.iyCellSize;

        // NOTE : Item이 Hide 되어있을 경우를 위한..
        if (rSlot.isItem)
        {
            ixCellSize = std::max(rSlot.ixCellSize, int(rSlot.byxPlacedItemSize * ITEM_WIDTH));
            iyCellSize = std::max(rSlot.iyCellSize, int(rSlot.byyPlacedItemSize * ITEM_HEIGHT));
        }

        if (ixLocal >= rSlot.ixPosition)
            if (iyLocal >= rSlot.iyPosition)
                if (ixLocal <= rSlot.ixPosition + ixCellSize)
                    if (iyLocal <= rSlot.iyPosition + iyCellSize)
                    {
                        *ppSlot = &rSlot;
                        return true;
                    }
    }

    return false;
}

void CSlotWindow::SetUseMode(bool bFlag)
{
    m_isUseMode = bFlag;
}

#ifdef ENABLE_SHOP_EFFECT_SYSTEM
void CSlotWindow::SetSellMode(bool bFlag, int iSlot)
{
    TSlot *pSlot;
    if (!GetSlotPointer(iSlot, &pSlot))
    {
        return;
    }

    pSlot->m_isSellMode = bFlag;
}
#endif
#ifdef ENABLE_TRADE_EFFECT_SYSTEM
void CSlotWindow::SetTradeMode(bool bFlag, int iSlot)
{
    TSlot *pSlot;
    if (!GetSlotPointer(iSlot, &pSlot))
    {
        return;
    }

    pSlot->m_isTradeMode = bFlag;
}
#endif
#ifdef ENABLE_SAFEBOX_EFFECT_SYSTEM
void CSlotWindow::SetSafeboxMode(bool bFlag, int iSlot)
{
    TSlot *pSlot;
    if (!GetSlotPointer(iSlot, &pSlot))
    {
        return;
    }

    pSlot->m_isSafeboxMode = bFlag;
}
#endif

void CSlotWindow::SetUsableItem(bool bFlag)
{
    m_isUsableItem = bFlag;
}

void CSlotWindow::ReserveDestroyCoolTimeFinishEffect(DWORD dwSlotIndex)
{
    m_ReserveDestroyEffectDeque.push_back(dwSlotIndex);
}

DWORD CSlotWindow::Type()
{
    static int s_Type = ComputeCrc32(0, "CSlotWindow", strlen("CSlotWindow"));
    return s_Type;
}

bool CSlotWindow::OnIsType(DWORD dwType)
{
    if (CSlotWindow::Type() == dwType)
    {
        return TRUE;
    }

    return CWindow::OnIsType(dwType);
}

void CSlotWindow::SetSlotBackground(DWORD dwIndex, const char *c_szFileName)
{
    TSlot *pSlot;
    if (!GetSlotPointer(dwIndex, &pSlot))
    {
        return;
    }

    pSlot->pBackgroundInstance.reset(new CGraphicImageInstance);
    pSlot->pBackgroundInstance->SetImagePointer(CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName));
}

void CSlotWindow::__CreateToggleSlotImage()
{
    __DestroyToggleSlotImage();

    m_pToggleSlotImage = new CImageBox();
    m_pToggleSlotImage->LoadImage("d:/ymir work/ui/public/slot_toggle.sub");
    m_pToggleSlotImage->Show();
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
void CSlotWindow::__CreateSlotCoverImage()
{
    __DestroySlotCoverImage();

    m_pSlotCoverImage = new CAniImageBox();
    m_pSlotCoverImage->AppendImage("icon/item/ingame_convert_mark.png");
    m_pSlotCoverImage->Show();
}

void CSlotWindow::__DestroySlotCoverImage()
{
    if (m_pSlotCoverImage)
    {
        delete m_pSlotCoverImage;
        m_pSlotCoverImage = NULL;
    }
}
#endif

void CSlotWindow::__CreateCoverSlotImage(const char *c_szFileName)
{
    __DestroyCoverSlotImage();

    m_pCoverSlotImage = new CImageBox();
    m_pCoverSlotImage->LoadImage(c_szFileName);
    m_pCoverSlotImage->Show();
}

void CSlotWindow::__CreateSlotEnableEffect(int activeSlot)
{
    if (activeSlot < 0 || activeSlot > 2)
    {
        return;
    }
    __DestroySlotEnableEffect(activeSlot);

#ifdef NEW_PICK_UP_ITEM
    if (activeSlot == 1) {

        m_pSlotActiveEffect[1] = new CAniImageBox();
        for (int i = 23; i <= 54; ++i) {
            std::string imagePath = "d:/ymir "
                                    "work/ui/public/newitemeffect/"
                                    "newitemeffect_" +
                                    std::to_string(i) + ".sub";
            m_pSlotActiveEffect[1]->AppendImage(imagePath.c_str());
        }
    } 
    else if (activeSlot == 2) {
        m_pSlotActiveEffect[2] = new CAniImageBox();
        for (int i = 55; i <= 93; ++i) {
            std::string imagePath = "d:/ymir "
                                    "work/ui/public/newitemeffect/" 
                                    "newitemeffect_" +
                                    std::to_string(i) + ".sub";
            m_pSlotActiveEffect[2]->AppendImage(imagePath.c_str());
        }
    }
#else
    if (activeSlot == 1)
    {
        __DestroySlotEnableEffect(activeSlot);
        m_pSlotActiveEffect[1] = new CAniImageBox();
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_000001.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_000002.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_000003.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_000004.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_000005.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_000006.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_000007.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_000008.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_000009.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_0000010.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_0000011.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_0000012.sub");
        m_pSlotActiveEffect[1]->AppendImage("d:/ymir work/ui/public/newitemeffect/newitemeffect_0000013.sub");
    }
    else if (activeSlot == 2)
    {
        __DestroySlotEnableEffect(activeSlot);

        m_pSlotActiveEffect[2] = new CAniImageBox();
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/00.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/01.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/02.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/03.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/04.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/05.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/06.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/07.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/08.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/09.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/10.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/11.sub");
        m_pSlotActiveEffect[2]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/12.sub");
    }
#endif
    else
    {

        //Tek slotlu itemler slot belirtilmediği için default
        m_pSlotActiveEffect[activeSlot] = new CAniImageBox();
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/00.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/01.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/02.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/03.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/04.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/05.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/06.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/07.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/08.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/09.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/10.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/11.sub");
        m_pSlotActiveEffect[activeSlot]->AppendImage("d:/ymir work/ui/public/slotactiveeffect/12.sub");
    }

    m_pSlotActiveEffect[activeSlot]->SetRenderingMode(
        CGraphicExpandedImageInstance::RENDERING_MODE_MODULATE);
    m_pSlotActiveEffect[activeSlot]->Show();
}

void CSlotWindow::__CreateFinishCoolTimeEffect(TSlot *pSlot)
{
    __DestroyFinishCoolTimeEffect(pSlot);

    CAniImageBox *pFinishCoolTimeEffect = new CCoolTimeFinishEffect(this, pSlot->dwSlotNumber);
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/00.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/01.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/02.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/03.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/04.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/05.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/06.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/07.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/08.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/09.sub");
    pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/10.sub");
    pFinishCoolTimeEffect->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
    pFinishCoolTimeEffect->ResetFrame();
    pFinishCoolTimeEffect->SetDelay(2);
    pFinishCoolTimeEffect->Show();

    pSlot->pFinishCoolTimeEffect = pFinishCoolTimeEffect;
}

void CSlotWindow::__CreateBaseImage(const char *c_szFileName, float fr, float fg, float fb, float fa)
{
    __DestroyBaseImage();

    m_pBaseImageInstance = std::move(std::make_unique<CGraphicImageInstance>(c_szFileName));
    m_pBaseImageInstance->SetImagePointer(CResourceManager::Instance().LoadResource<CGraphicImage>(c_szFileName));
    m_pBaseImageInstance->SetDiffuseColor(fr, fg, fb, fa);
}

void CSlotWindow::__DestroyToggleSlotImage()
{
    if (m_pToggleSlotImage)
    {
        delete m_pToggleSlotImage;
        m_pToggleSlotImage = nullptr;
    }
}

void CSlotWindow::__DestroyCoverSlotImage()
{
    if (m_pCoverSlotImage)
    {
        delete m_pCoverSlotImage;
        m_pCoverSlotImage = nullptr;
    }
}

void CSlotWindow::__DestroySlotEnableEffect(int activeSlot)
{
    if (m_pSlotActiveEffect[activeSlot])
    {
        delete m_pSlotActiveEffect[activeSlot];
        m_pSlotActiveEffect[activeSlot] = nullptr;
    }
}

void CSlotWindow::__DestroyFinishCoolTimeEffect(TSlot *pSlot)
{
    if (pSlot->pFinishCoolTimeEffect)
    {
        delete pSlot->pFinishCoolTimeEffect;
        pSlot->pFinishCoolTimeEffect = nullptr;
    }
}

void CSlotWindow::__DestroyBaseImage()
{
    m_pBaseImageInstance.reset();
}

void CSlotWindow::__Initialize()
{
    m_dwSlotType = 0;
    m_iWindowType = SLOT_WND_DEFAULT;
    m_dwSlotStyle = SLOT_STYLE_PICK_UP;
    m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;
    m_CoolDownStore.clear();
    m_isUseMode = FALSE;
    m_isUsableItem = FALSE;

    m_pToggleSlotImage = nullptr;
    m_pSlotActiveEffect[0] = nullptr;
    m_pSlotActiveEffect[1] = nullptr;
    m_pSlotActiveEffect[2] = nullptr;
    m_pBaseImageInstance = nullptr;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
    for (int i = 0; i < 3; ++i)
    {
        m_pSlotChangeLookActiveEffect[i] = nullptr;
    }

    m_pSlotCoverImage = nullptr;
#endif
    m_v2Scale.y = 1.0f;
    m_v2Scale.x = 1.0f;
    m_pCoverSlotImage = nullptr;
}

void CSlotWindow::Destroy()
{
    for (auto &rSlot : m_SlotList)
    {
        ClearSlot(&rSlot);

        rSlot.pNumberLine.reset();
        rSlot.pText.reset();

        if (rSlot.pCoverButton)
        {
            CWindowManager::Instance().DestroyWindow(rSlot.pCoverButton);
        }
        if (rSlot.pSlotButton)
        {
            CWindowManager::Instance().DestroyWindow(rSlot.pSlotButton);
        }
        if (rSlot.pSignImage)
        {
            CWindowManager::Instance().DestroyWindow(rSlot.pSignImage);
        }
        if (rSlot.pFinishCoolTimeEffect)
        {
            CWindowManager::Instance().DestroyWindow(rSlot.pFinishCoolTimeEffect);
        }
    }

    m_SlotList.clear();

    __DestroyToggleSlotImage();
    for (int slotEffect = 0; slotEffect < 3; ++slotEffect)
    {
        __DestroySlotEnableEffect(slotEffect);
    }
    __DestroyBaseImage();
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
    __DestroySlotCoverImage();
#endif
    __Initialize();
}

CSlotWindow::CSlotWindow() : CWindow()
{
    __Initialize();
}

CSlotWindow::~CSlotWindow()
{
    Destroy();
}
