#pragma once


namespace UI
{
	class CWindow;
	class CTextLine;

	enum WindowLayers
	{
		kWindowLayerGame,
		kWindowLayerUiBottom,
		kWindowLayerUi,
		kWindowLayerTopMost,
		kWindowLayerCurtain,

		kWindowLayerMax,
	};

	class CWindowManager : public CSingleton<CWindowManager>
	{
	public:
		typedef std::list<CWindow*> WindowList;

		CWindowManager();
		~CWindowManager();

		void Destroy();

		int32_t GetScreenWidth() const { return m_width; }
		int32_t GetScreenHeight() const { return m_height; }
		void SetScreenSize(int32_t lWidth, int32_t lHeight);
		void SetResolution(int hres, int vres);
		float GetAspect();

		void SetMouseHandler(PyObject * poMouseHandler);

		std::tuple<int32_t, int32_t> GetMousePosition();

		bool IsDragging();

		template <typename T>
		T* RegisterWindow(PyObject* po, const std::string& layer);

		void DestroyWindow(CWindow* win);

		// Attaching Icon
		bool IsAttaching();
		DWORD GetAttachingType();
		DWORD GetAttachingIndex();
		DWORD GetAttachingSlotNumber();
		DWORD GetAttachingRealSlotNumber() const;
		void GetAttachingIconSize(BYTE * pbyWidth, BYTE * pbyHeight);
		void AttachIcon(DWORD dwType, DWORD dwIndex, DWORD dwSlotNumber, BYTE byWidth, BYTE byHeight);
		void DeattachIcon();
		void SetAttachingFlag(bool bFlag);
		void		SetAttachingRealSlotNumber(DWORD dwRealslotNumber);

		// Attaching Icon
		HWND GetAppWindow() const { return m_appWindow; }
		void SetAppWindow(HWND wnd) { m_appWindow = wnd; }

		CWindow* GetPointWindow() const { return m_pointWindow; }

		void LockWindow(CWindow* win);
		void UnlockWindow();
		CWindow* GetLockWindow() const { return m_lockWindow; }

		void SetFocus(CWindow* win);
		void KillFocus();
		CWindow* GetFocus() const { return m_focusWindow; }

		void CaptureMouse(CWindow* win);
		void ReleaseMouse(CWindow* win);
		void ReleaseMouse();
		CWindow* GetCapture() const { return m_captureWindow; }

		void PopClipRegion();
		bool CommitClipRegion();
		bool PushClipRegion(const Vector4& cr);
		bool PushClipRegion(CWindow* win);

		void SetParent(CWindow* pWin, CWindow* pParentWindow);
		void SetTop(CWindow* pWin);

		void Update();
		void Render();

		void RunMouseMove(int32_t x, int32_t y);
		void RunMouseLeftButtonDown();
		void RunMouseLeftButtonUp();
		void RunMouseLeftButtonDoubleClick();
		void RunMouseRightButtonDown();
		void RunMouseRightButtonUp();
		void RunMouseRightButtonDoubleClick();
		void RunMouseMiddleButtonDown();
		void RunMouseMiddleButtonUp();
		bool RunMouseWheelEvent(int nLen);

		void RunChar(uint32_t ch);
		void RunKeyDown(KeyCode code);
		void RunKeyUp(KeyCode code);
		void RunTab();
		void RunPressExitKey();

		void NotifyHideWindow(CWindow* win);
		
		Vector2 PixelToClip() const
		{	
			Vector2 res( GetScreenWidth(), GetScreenHeight() );

			res.x = 2.f / res.x;
			res.y = 2.f / res.y;
			return res;
		};
	private:
		CWindow* PickWindow(int32_t x, int32_t y);
		void SetPointedWindowByPos(int32_t x, int32_t y);

		void PruneLazyDeletionQueue();

		CWindow* GetLayerByName(const std::string& name);

		void SetTopRecursive(CWindow* win);


		int32_t m_width;
		int32_t m_height;

		int32_t m_vres;
		int32_t m_hres;

		int32_t m_mouseX, m_mouseY;
		int32_t m_dragX, m_dragY;
		int32_t m_pickedX, m_pickedY;

		bool m_bOnceIgnoreMouseLeftButtonUpEventFlag;
		int m_iIgnoreEndTime;

		// Attaching Icon
		PyObject* m_poMouseHandler;
		bool m_bAttachingFlag;
		DWORD m_dwAttachingType;
		DWORD m_dwAttachingIndex;
		DWORD m_dwAttachingSlotNumber;
		DWORD m_dwAttachingRealSlotNumber = 0;
		BYTE m_byAttachingIconWidth;
		BYTE m_byAttachingIconHeight;
		// Attaching Icon

		CWindow* m_focusWindow;
		WindowList m_focusWindowList;

		// Modal window list (all other windows unresponsive)
		CWindow* m_lockWindow;
		WindowList m_lockWindowList;

		// Mouse-focus capture
		CWindow* m_captureWindow;
		WindowList m_captureWindowList;

		// OnMouse*ButtonUp needs to be called on the same window
		// which received the OnMouse*ButtonDown event (except when
		// some other window captures mouse-events).
		CWindow* m_mouseLeftWindow;
		CWindow* m_mouseRightWindow;
		CWindow* m_mouseMiddleWindow;

		// OnKeyUp events need to be routed to the same window which received
		// the OnKeyDown event. This map remembers these windows.
		std::unordered_map<KeyCode, CWindow*> m_keyHandlers;

		CWindow* m_pointWindow;
		HWND	m_appWindow;
		std::unique_ptr<CWindow> m_rootWindow;

		WindowList m_lazyDeleteQueue;
		typedef std::stack< Vector4, std::vector< Vector4 > > ClipStack;
		ClipStack			clipStack_;
	};

	template <typename T>
	T* CWindowManager::RegisterWindow(PyObject* po, const std::string& layer)
	{
		auto layerWnd = GetLayerByName(layer);
		assert(layerWnd && "not null");

		T* win = new T();
		win->SetHandler(po);
		layerWnd->AddChild(win);
		return win;
	}
}

bool PyTuple_GetWindow(PyObject* poArgs, int pos, UI::CWindow** ppRetWindow);