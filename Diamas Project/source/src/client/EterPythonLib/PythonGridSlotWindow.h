#pragma once

#include "PythonSlotWindow.h"

namespace UI
{
	class CGridSlotWindow : public CSlotWindow
	{
		public:
			static DWORD Type();

		public:
			CGridSlotWindow ();
			virtual ~CGridSlotWindow();

			void Destroy();

			void ArrangeGridSlot (DWORD dwStartIndex, DWORD dwxCount, DWORD dwyCount, int ixSlotSize, int iySlotSize, int ixTemporarySize, int iyTemporarySize);

		protected:
			void __Initialize();

			bool GetPickedSlotPointer (TSlot** ppSlot);
			bool GetPickedSlotList (int iWidth, int iHeight, std::list<TSlot*>* pSlotPointerList);
			bool GetGridSlotPointer (int ix, int iy, TSlot** ppSlot);
			bool GetSlotPointerByNumber (DWORD dwSlotNumber, TSlot** ppSlot);
			bool GetPickedGridSlotPosition (int ixLocal, int iyLocal, int* pix, int* piy);
			bool CheckMoving (DWORD dwSlotNumber, DWORD dwItemIndex, const std::list<TSlot*>& c_rSlotList);
			bool CheckSwapping (DWORD dwSlotNumber, DWORD dwItemIndex, const std::list<TSlot*>& c_rSlotList);

			bool OnIsType (DWORD dwType);

			void OnRefreshSlot();
			void OnRenderPickingSlot();

		protected:
			DWORD m_dwxCount;
			DWORD m_dwyCount;

			std::vector<TSlot*> m_SlotVector;
	};
};
