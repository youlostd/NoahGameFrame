#pragma once

#include "PythonWindow.h"
#include <base/SimpleMath.h>

namespace UI
{
	enum
	{
		ITEM_WIDTH = 32,
		ITEM_HEIGHT = 32,

		SLOT_NUMBER_NONE = 0xffffffff,
	};

	enum EHighlightType
	{
		HILIGHTSLOT_ACCE,
		HILIGHTSLOT_CHANGE_LOOK
	};

	enum ESlotStyle
	{
		SLOT_STYLE_NONE,
		SLOT_STYLE_PICK_UP,
		SLOT_STYLE_SELECT,
	};

	enum ESlotState
	{
		SLOT_STATE_LOCK		= (1 << 0),
		SLOT_STATE_CANT_USE	= (1 << 1),
		SLOT_STATE_DISABLE	= (1 << 2),
		SLOT_STATE_ALWAYS_RENDER_COVER = (1 << 3),			// 현재 Cover 버튼은 슬롯에 무언가 들어와 있을 때에만 렌더링 하는데, 이 flag가 있으면 빈 슬롯이어도 커버 렌더링
		SLOT_STATE_CANT_MOUSE_EVENT = (1 << 4),			
		SLOT_STATE_UNUSABLE_ON_TOP_WND = (1 << 5),			
		SLOT_STATE_HIGHLIGHT_GREEN = (1 << 6),			
		SLOT_STATE_HIGHLIGHT_RED = (1 << 7),			
	};

	enum ESlotDiffuseColorType
	{
		SLOT_COLOR_TYPE_ORANGE,
		SLOT_COLOR_TYPE_WHITE,
		SLOT_COLOR_TYPE_RED,
		SLOT_COLOR_TYPE_GREEN,
		SLOT_COLOR_TYPE_YELLOW,
		SLOT_COLOR_TYPE_SKY,
		SLOT_COLOR_TYPE_PINK,
		SLOT_COLOR_TYPE_BLUE,
	};

	enum EWindowType
	{
		SLOT_WND_DEFAULT,
		SLOT_WND_INVENTORY
	};

	class CSlotWindow : public CWindow
	{
		public:
			static DWORD Type();

		public:
			class CSlotButton;
			class CCoverButton;
			class CCoolTimeFinishEffect;

			friend class CSlotButton;
			friend class CCoverButton;

			typedef struct SSlot
			{
				DWORD	dwState;
				DWORD	dwSlotNumber;
				DWORD	dwCenterSlotNumber;		// NOTE : 사이즈가 큰 아이템의 경우 아이템의 실제 위치 번호
				DWORD   dwRealSlotNumber;					//Unique identifier. Initially added as an aid for intentory grid. [Think]
				DWORD	dwRealCenterSlotNumber;
				DWORD	dwItemIndex;			// NOTE : 여기서 사용되는 Item이라는 단어는 좁은 개념의 것이 아닌,
				bool	isItem;					//        "슬롯의 내용물"이라는 포괄적인 개념어. 더 좋은 것이 있을까? - [levites]

				// CoolTime
				float	fCoolTime;
				float	fStartCoolTime;
				DirectX::SimpleMath::Color sCoolTimeColor;
				bool bIsInverseCoolTime;

				// Expire Time
				float fExpireTime;
				float fStartExpireTime;
				float fExpireMaxTime;

				// Toggle
				bool	bActive;
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
				BOOL	bChangeLookActive;
				BOOL	bSlotCoverImage;
#endif
				int		ixPosition;
				int		iyPosition;

				int		ixCellSize;
				int		iyCellSize;
				
				int		ixPlacementPosition=0;
				int		iyPlacementPosition=0;
					
				int		ixTextPosition=-4;
				int		iyTextPosition=8;

				int		ixSlotButtonPosition=1;
				int		iySlotButtonPosition=19;

				BYTE	byxPlacedItemSize;
				BYTE	byyPlacedItemSize;

				std::unique_ptr<CGraphicImageInstance> pInstance;
				std::unique_ptr<CGraphicImageInstance> pBackgroundInstance;
				std::unique_ptr<CNumberLine> pNumberLine;
				std::unique_ptr<CTextLine> pText;


				bool	bRenderBaseSlotImage;
				CCoverButton* pCoverButton;
				CSlotButton* pSlotButton;
				CImageBox* pSignImage;
				CAniImageBox* pFinishCoolTimeEffect;
				DirectX::SimpleMath::Color	sDiffuseColor;
				bool bEnableSlotCoverImage;
				std::vector<CSlotButton*> vevSlotButtons;
			} TSlot;
			typedef std::list<TSlot> TSlotList;
			typedef TSlotList::iterator TSlotListIterator;
			typedef struct SStoreCoolDown { float fCoolTime; float fElapsedTime; bool bActive; } TStoreCoolDown;

		public:
			CSlotWindow ();
			virtual ~CSlotWindow();

			void Destroy();
			void SetWindowType (int iType);

			// Manage Slot
			void SetSlotType (DWORD dwType);
			void SetSlotStyle (DWORD dwStyle);

			void AppendSlot (DWORD dwIndex, int ixPosition, int iyPosition, int ixCellSize, int iyCellSize, int placementX=0, int placementY=0);
			void SetCoverButton (DWORD dwIndex, const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName, const char* c_szDisableImageName, bool bLeftButtonEnable, bool bRightButtonEnable);
			void SetSlotBaseImage (const char* c_szFileName, float fr, float fg, float fb, float fa);
			void SetSlotButton(uint32_t index, const char* image, const char* hoverImage, const char* pressImage);
			void AppendSlotButton (const char* c_szUpImageName, const char* c_szOverImageName, const char* c_szDownImageName);
			void AppendRequirementSignImage (const char* c_szImageName);

			void EnableCoverButton (DWORD dwIndex);
			void DisableCoverButton (DWORD dwIndex);
			void SetAlwaysRenderCoverButton (DWORD dwIndex, bool bAlwaysRender = false);

			void ShowSlotBaseImage (DWORD dwIndex);
			void HideSlotBaseImage (DWORD dwIndex);
			bool IsDisableCoverButton (DWORD dwIndex);
			bool HasSlot (DWORD dwIndex);

			void ClearAllSlot();
			void ClearSlot (DWORD dwIndex);
			void SetSlot (DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, CGraphicImage::Ptr pImage, Color diffuseColor);
			void SetSlotCount (DWORD dwIndex, DWORD dwCount);
			void SetSlotCountNew (DWORD dwIndex, DWORD dwGrade, DWORD dwCount);
			void SetSlotSlotNumber(DWORD dwIndex, DWORD dwGrade, DWORD dwCount);
			void SetSlotSlotText(DWORD dwIndex, const std::string& text);

			void SetRealSlotNumber (DWORD dwIndex, DWORD dwID);
			void SetSlotCoolTime (DWORD dwIndex, float fCoolTime, float fElapsedTime = 0.0f);
			void SetSlotCoolTimeColor (DWORD dwIndex, float fCoolTime, float fElapsedTime, float fColorR, float fColorG, float fColorB, float fColorA);
			void SetSlotCoolTimeInverse (DWORD dwIndex, float fCoolTime, float fElapsedTime);
			void StoreSlotCoolTime(DWORD dwKey, DWORD dwSlotIndex, float fCoolTime, float fElapsedTime = .0f);
			void RestoreSlotCoolTime(DWORD dwKey);

			void SetSlotExpireTime (DWORD dwIndex, float fExpireTime, float fMaxTime);
			void ActivateSlot (DWORD dwIndex);
			void DeactivateSlot (DWORD dwIndex);
			void RefreshSlot();
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			void EnableSlotCoverImage(DWORD dwIndex);
			void DisableSlotCoverImage(DWORD dwIndex);
			void ActivateChangeLookSlot(DWORD dwIndex);
			void DeactivateChangeLookSlot(DWORD dwIndex);
#endif
			DWORD GetSlotCount();

			void LockSlot (DWORD dwIndex);
			void UnlockSlot (DWORD dwIndex);
			bool IsLockSlot (DWORD dwIndex);
			void SetCantUseSlot (DWORD dwIndex);
			void SetUseSlot (DWORD dwIndex);
			void SetCantMouseEventSlot (DWORD dwIndex);
			bool IsCantUseSlot (DWORD dwIndex);
			void SetCanMouseEventSlot (DWORD dwIndex);
			void SetUnusableSlotOnTopWnd (DWORD dwIndex);
		void DisableSlotHighlightedGreen(DWORD dwIndex);
		void SetSlotHighlightedGreeen(DWORD dwIndex);
            void SetSlotTextPosition(DWORD dwIndex, DWORD x, DWORD y);
            void GetSlotLocalPosition(DWORD dwIndex, DWORD* x, DWORD* y);
		void GetSlotGlobalPosition(DWORD dwIndex, DWORD* x, DWORD* y);
		void SetUsableSlotOnTopWnd (DWORD dwIndex);
			void EnableSlot (DWORD dwIndex);
			void DisableSlot (DWORD dwIndex);
			bool IsEnableSlot (DWORD dwIndex);

			// Select
			void ClearSelected();
			void SelectSlot (DWORD dwSelectingIndex);
			bool isSelectedSlot (DWORD dwIndex);
			DWORD GetSelectedSlotCount();
			DWORD GetSelectedSlotNumber (DWORD dwIndex);

			// Slot Button
			void ShowSlotButton (DWORD dwSlotNumber);
			void HideAllSlotButton();
			void SetSlotButtonPosition(DWORD dwSlotNumber, int x, int y);
			void OnPressedSlotButton (DWORD dwType, DWORD dwSlotNumber, bool isLeft = TRUE);

			// Slot background
			void SetSlotBackground (DWORD dwIndex, const char* c_szFileName);

			// Requirement Sign
			void ShowRequirementSign (DWORD dwSlotNumber);
			void HideRequirementSign (DWORD dwSlotNumber);

			// ToolTip
			bool OnOverInItem (DWORD dwSlotNumber);
			void OnOverOutItem();

			// For Usable Item
			void SetUseMode (bool bFlag);
			void SetUsableItem (bool bFlag);
			// CallBack
			void ReserveDestroyCoolTimeFinishEffect (DWORD dwSlotIndex);

			// Diffuse Color
			void SetDiffuseColor (DWORD dwIndex, BYTE colorType);
			void SetItemDiffuseColor (DWORD dwIndex, float r, float g, float b, float a);
			void SetSlotBaseImageScale (float fx, float fy);
			void SetScale (float fx, float fy);
			void SetCoverButtonScale (DWORD dwIndex, float fx, float fy);
			void SetSlotCoverImage (int iSlotIndex, const char* c_szFilename);
			void EnableSlotCoverImage (int iSlotIndex, bool bIsEnable);
		protected:
			void __Initialize();
			void __CreateToggleSlotImage();
			void __CreateCoverSlotImage (const char* c_szFileName);
			void __CreateSlotEnableEffect (int activeSlotCount);
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			void __CreateSlotCoverImage();
			void __DestroySlotCoverImage();
#endif
			void __CreateFinishCoolTimeEffect (TSlot* pSlot);
			void __CreateBaseImage (const char* c_szFileName, float fr, float fg, float fb, float fa);

			void __DestroyToggleSlotImage();
			void __DestroyCoverSlotImage();
			void __DestroySlotEnableEffect (int activeSlot);
			void __DestroyFinishCoolTimeEffect (TSlot* pSlot);
			void __DestroyBaseImage();

			// Event
			void OnUpdate();
			void OnRender();
			bool OnMouseLeftButtonDown();
			bool OnMouseLeftButtonUp();
			bool OnMouseRightButtonDown();
			bool OnMouseLeftButtonDoubleClick();
			bool OnMouseOverOut();
			void OnMouseOver();
			void RenderSlotBaseImage();
			void RenderLockedSlot();
			virtual void OnRenderPickingSlot();
			virtual void OnRenderSelectedSlot();

			// Select
			void OnSelectEmptySlot (int iSlotNumber);
			void OnSelectItemSlot (int iSlotNumber);
			void OnUnselectEmptySlot (int iSlotNumber);
			void OnUnselectItemSlot (int iSlotNumber);
			void OnUseSlot();

			// Manage Slot
			bool GetSlotPointer (DWORD dwIndex, TSlot** ppSlot);
			bool GetSelectedSlotPointer (TSlot** ppSlot);
			virtual bool GetPickedSlotPointer (TSlot** ppSlot);
			void ClearSlot (TSlot* pSlot);
			virtual void OnRefreshSlot();

			// ETC
			bool OnIsType (DWORD dwType);

		protected:
			int m_iWindowType;
			DWORD m_dwSlotType;
			DWORD m_dwSlotStyle;
			std::list<DWORD> m_dwSelectedSlotIndexList;
			TSlotList m_SlotList;
			DWORD m_dwToolTipSlotNumber;
			std::map<DWORD, std::map<DWORD, SStoreCoolDown>>	m_CoolDownStore;
			bool m_isUseMode;
			bool m_isUsableItem;

			std::unique_ptr<CGraphicImageInstance> m_pBaseImageInstance;
			CImageBox* m_pToggleSlotImage;
			CAniImageBox* m_pSlotActiveEffect[3];
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
			CAniImageBox * m_pSlotChangeLookActiveEffect[3];
			CAniImageBox * m_pSlotCoverImage;
#endif
			std::deque<DWORD> m_ReserveDestroyEffectDeque;
			DirectX::SimpleMath::Vector2 m_v2Scale;
			CImageBox* m_pCoverSlotImage;

	};
};