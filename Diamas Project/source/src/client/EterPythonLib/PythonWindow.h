#pragma once

#include "../eterBase/Utils.h"
#include "../eterLib/GrpRenderTargetTexture.h"
#include "../EterLib/GrpImageInstance.h"
#include "../EterLib/GrpExpandedImageInstance.h"
#include "../eterLib/GrpMarkInstance.h"
#include "../eterLib/GrpTextInstance.h"
#include <boost/container/small_vector.hpp>

#include "../EterLib/KeyboardInput.hpp"
#include "../CWebBrowser/WebView.h"
#include "BandiVideoLibrary.h"
#include "easing.h"
#include "tweeny.h"

namespace UI
{


	class CWindow : public CScreen {
	public:
		typedef boost::container::small_vector<CWindow*, 10> WindowList;

		static DWORD Type();
		bool         IsType(DWORD dwType);

		enum EHorizontalAlign {
			HORIZONTAL_ALIGN_LEFT = 0,
			HORIZONTAL_ALIGN_CENTER = 1,
			HORIZONTAL_ALIGN_RIGHT = 2,
		};

		enum EVerticalAlign {
			VERTICAL_ALIGN_TOP = 0,
			VERTICAL_ALIGN_CENTER = 1,
			VERTICAL_ALIGN_BOTTOM = 2,
		};

		enum EFlags {
			FLAG_MOVABLE = (1 << 0),
			// 움직일 수 있는 창
			FLAG_SNAP = (1 << 1),
			// 스냅 될 수 있는 창
			FLAG_DRAGABLE = (1 << 2),
			FLAG_ATTACH = (1 << 3),
			// 완전히 부모에 붙어 있는 창 (For Drag / ex. ScriptWindow)
			FLAG_RESTRICT_X = (1 << 4),
			// 좌우 이동 제한
			FLAG_RESTRICT_Y = (1 << 5),
			// 상하 이동 제한
			//FLAG_NOT_CAPTURE = (1 <<  6),
			FLAG_FLOAT = (1 << 7),
			// 공중에 떠있어서 순서 재배치가 되는 창
			FLAG_NOT_PICK = (1 << 8),
			// 마우스에 의해 Pick되지 않는 창
			FLAG_IGNORE_SIZE = (1 << 9),
			FLAG_RTL = (1 << 10),
			// Right-to-left
			FLAG_FOCUSABLE = 1 << 11,
			FLAG_ANIMATED_BOARD = 1 << 12,
			FLAG_COMPONENT = 1 << 13,
		};

		CWindow();
		virtual ~CWindow();

		void AddChild(CWindow* win);

		void Clear();
		void DestroyHandle();

		void Update();
		void Render();

		void SetName(std::string name) {
			m_name = name;
		}

		const std::string& GetName() const {
			return m_name;
		}

		void SetSize(int32_t width, int32_t height);
		void SetScale(float fx, float fy);

		int32_t GetWidth() const {
			return m_width;
		}

		int32_t GetHeight() const {
			return m_height;
		}

		void SetHandler(PyObject* handler) {
			m_handler = handler;
		}

		PyObject* GetHandler() const {
			return m_handler;
		}

		void SetHorizontalAlign(DWORD dwAlign);
		void SetVerticalAlign(DWORD dwAlign);

		void SetPosition(int32_t x, int32_t y);

		int32_t GetPositionX() const {
			return m_x;
		}

		int32_t GetPositionY() const {
			return m_y;
		}

		RECT& GetRect() {
			return m_rect;
		}

		const RECT& GetClipRect() const {
			return m_clipRect;
		}

		void SetClipRect(const RECT& rect) {
			m_clipRect = rect;
		}

		void MakeLocalPosition(int32_t& rlx, int32_t& rly);

		void UpdateRect();

        void SetVisibility(bool visible);
		void Show();

		void Hide();

		bool IsShow() const;

		bool IsRendering();

		bool HasParent() const {
			return !!m_parent;
		}

		const WindowList& GetChildren() const {
			return m_children;
		}

		size_t GetChildrenCount() const {
			return m_children.size();
		}

		CWindow* GetRoot();
		CWindow* GetParent();
		void SetParentForce(CWindow* parent) { m_parent = parent; }

		bool IsChild(CWindow* win);
		void DeleteChild(CWindow* win, bool clearParent = true);
		void SetTop(CWindow* win);

		bool IsIn(int32_t x, int32_t y);
		bool IsIn();

		CWindow* PickWindow(int32_t x, int32_t y);

		bool HasClippingEnabled() const { return m_enableClipping; }
		void EnableClipping() {
			m_enableClipping = true;
		}

		void DisableClipping() {
			m_enableClipping = false;
		}

		void AddFlag(DWORD flag) {
			SET_BIT(m_flags, flag);
		}

		void RemoveFlag(DWORD flag) {
			REMOVE_BIT(m_flags, flag);
		}

		bool IsFlag(DWORD flag) const {
			return (m_flags & flag);
		}

		void			SetSingleAlpha(float fAlpha);
		virtual void	SetAlpha(float fAlpha)			{ m_fWindowAlpha = fAlpha; }
		virtual float	GetAlpha() const				{ return m_fWindowAlpha; }
		void			SetAllAlpha(float fAlpha);

		virtual void SetColor(DWORD dwColor) {
		}

		virtual bool OnIsType(DWORD dwType);

		virtual bool IsWindow() {
			return true;
		}

		virtual void OnRender();
		virtual void OnAfterRender();
		virtual void OnUpdate();

		virtual void OnChangePosition() {
			RunCallback("OnChangePosition");
		}

		virtual void OnSetFocus();
		virtual void OnKillFocus();

		virtual void OnMoveWindow(int32_t x, int32_t y);
		virtual void OnMouseDrag(int32_t x, int32_t y);
		virtual bool OnMouseOverIn();
		virtual bool OnMouseOverOut();
		virtual void OnMouseOver();
		virtual void OnDrop();
		virtual void OnTop();

		virtual bool OnMouseLeftButtonDown();
		virtual bool OnMouseLeftButtonUp();
		virtual bool OnMouseLeftButtonDoubleClick();
		virtual bool OnMouseRightButtonDown();
		virtual bool OnMouseRightButtonUp();
		virtual bool OnMouseRightButtonDoubleClick();
		virtual bool OnMouseMiddleButtonDown();
		virtual bool OnMouseMiddleButtonUp();
		virtual bool OnMouseWheelEvent(int nLen);

		virtual bool OnChar(uint32_t ch);
		virtual bool OnKeyDown(KeyCode code);
		virtual bool OnKeyUp(KeyCode code);
		virtual bool OnTab();
		virtual bool OnPressExitKey();
		virtual void OnDestroy();

	protected:
		bool RunCallback(const std::string& name,
		                 PyObject*          args = nullptr);

		std::string m_name;

		EHorizontalAlign m_HorizontalAlign;
		EVerticalAlign   m_VerticalAlign;

		// X,Y 상대좌표
		int32_t m_x, m_y;

		// 크기
		int32_t m_width, m_height;
		Vector2			m_v2Scale = {1.0f, 1.0f};
		Matrix m_matScaling;


		bool				m_bSingleAlpha;
		float				m_fSingleAlpha;
		float				m_fWindowAlpha;
		bool				m_bAllAlpha;
		float				m_fAllAlpha;


		// Global 좌표
		RECT m_rect;
		RECT m_clipRect = {0,0,0,0};

		bool m_enableClipping = false;
		bool m_visible;
		std::optional<tweeny::tween<float>> m_sizeAnimation;
		

		uint32_t m_flags;

		PyObject* m_handler;

		CWindow*   m_parent;

		WindowList m_children;

		// |m_children| copy for iteration
		WindowList m_childrenCopy;
	};

	class CBox : public CWindow {
	public:
		CBox();
		virtual ~CBox();

		void SetColor(DWORD dwColor);

	protected:
		void OnRender();

		DWORD m_dwColor;
	};

	class CBar : public CWindow {
	public:
		CBar();
		virtual ~CBar();

		void SetColor(DWORD dwColor);

	protected:
		void OnRender();

		DWORD m_dwColor;
	};

	class CLine : public CWindow {
	public:
		CLine();
		virtual ~CLine();

		void SetColor(DWORD dwColor);

	protected:
		void OnRender();

		DWORD m_dwColor;
	};

	class CBar3D : public CWindow {
	public:
		static DWORD Type();

	public:
		CBar3D();
		virtual ~CBar3D();

		void SetColor(DWORD dwLeft, DWORD dwRight, DWORD dwCenter);

	protected:
		void OnRender();

		DWORD m_dwLeftColor;
		DWORD m_dwRightColor;
		DWORD m_dwCenterColor;
	};

	// Text
	class CTextLine : public CWindow {
	public:
		CTextLine();
		virtual ~CTextLine();

		DWORD Type();

		bool OnIsType(DWORD dwType);

		void SetSecret(bool bFlag);
		void SetOutline(bool bFlag);
		void SetMultiLine(bool bFlag);
		void SetFontName(const std::string& font);
		void SetFontColor(DWORD dwColor);
		void SetFontGradient(DWORD dwColor, DWORD dwColor2);
		void SetLimitWidth(float fWidth);

		void               SetText(std::string text);
		const std::string& GetText();

		std::string GetHyperlinkAtPos(int32_t x, int32_t y);


		uint32_t    GetLineCount() const;;

	protected:
		virtual void OnUpdate();
		virtual void OnRender();

		virtual void OnChangeText();
		void SetAlpha(float fAlpha);
		float GetAlpha() const;

		CGraphicTextInstance m_instance;
		std::string          m_text;
		std::string          m_fontName;
		CGraphicTextInstance m_instancePlaceholder;
		uint32_t             m_placeHolderColor;
	};

	class CNumberLine : public CWindow {
	public:

		CNumberLine();
		CNumberLine(CWindow* pParent);
		
		CNumberLine(const CNumberLine &m) = delete;
		CNumberLine & operator= (const CNumberLine &) = delete;
		
		virtual ~CNumberLine();

		void SetPath(const char* c_szPath);
		void SetHorizontalAlign(int iType);
		void SetNumber(const char* c_szNumber);

	protected:
		void ClearNumber();
		void OnRender();
		void OnChangePosition();

		std::string                                         m_strPath;
		std::string                                         m_strNumber;
		std::vector<std::unique_ptr<CGraphicImageInstance>> m_numberImages;

		int   m_iHorizontalAlign;
		DWORD m_dwWidthSummary;
	};

	class CVideoFrame : public CWindow {

	public:
		CVideoFrame();
		virtual ~CVideoFrame();
		void    Close();
		void    Open(const char* filename);

	protected:
		virtual void OnUpdate();
		virtual void OnRender();
		void         OnChangePosition();

	private:
		CBandiVideoLibrary m_bvl;
		BVL_VIDEO_INFO     m_video_info;

		CGraphicImageTexture* m_texture;

		LONGLONG    m_current_time;
		LONGLONG    m_current_frame;
		Vector2 m_v2Position{};

	};

	// WebView
	class CWebView : public CWindow {
	public:
		enum eWebBrowserMouseButton {
			BROWSER_MOUSEBUTTON_LEFT = 0,
			BROWSER_MOUSEBUTTON_MIDDLE = 1,
			BROWSER_MOUSEBUTTON_RIGHT = 2
		};

		CWebView();
		virtual ~CWebView();
		void    CloseBrowser();
		void    UpdateTexture();

		void LoadUrl(const char* url);
		void OnCreateInstance(HWND parent);

	protected:
		virtual void OnDestroyInstance();

		virtual void OnUpdate();
		virtual void OnRender();
		void         OnChangePosition();
		bool         OnMouseLeftButtonDown();
		bool         OnMouseLeftButtonUp();
		bool         OnMouseRightButtonDown();
		bool         OnMouseRightButtonUp();
		bool         OnMouseMiddleButtonDown();
		BOOL         RunMouseWheelEvent(long nLen);
		bool         OnMouseWheelEvent(int nLen);
		bool         OnMouseMiddleButtonUp();
		bool         OnMouseOverIn();
		bool         OnMouseOverOut();
		void         OnSetFocus();
		void         OnKillFocus();

	protected:

		CefRefPtr<WebView> m_pWebView;
		Vector2        m_v2Position{};
		bool               m_mouseButtonStates[3]{};
		POINT              m_vecMousePosition{};

		struct {
			bool                    changed = false;
			std::mutex              dataMutex;
			std::mutex              cvMutex;
			std::condition_variable cv;

			const void*                buffer;
			int                        width, height;
			CefRenderHandler::RectList dirtyRects;

		} m_RenderData;

		bool m_bBeingDestroyed = false;
	};

	// Image
	class CImageBox : public CWindow {
	public:
		CImageBox();
		virtual ~CImageBox();

		bool LoadImage(const char* c_szFileName);
		void SetDiffuseColor(float fr, float fg, float fb, float fa);
		void SetAlpha(float fAlpha);
		float GetAlpha() const;

		int   GetWidth();
		int   GetHeight();
		float m_coolTime      = 0.0f;
		float m_startCoolTime = 0.0f;
	protected:
		virtual void OnCreateInstance();
		virtual void OnDestroyInstance();

		virtual void OnUpdate();
		virtual void OnRender();
		void         OnChangePosition();

	protected:
		std::unique_ptr<CGraphicExpandedImageInstance> m_pImageInstance;

		D3DXCOLOR m_diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
	};

class CMoveTextLine : public CTextLine
	{
	public:
		CMoveTextLine();
		virtual ~CMoveTextLine();

	public:
		static DWORD Type();

		void SetMoveSpeed(float fSpeed);
		void SetMovePosition(float fDstX, float fDstY);
		bool GetMove();
		void MoveStart();
		void MoveStop();

	protected:
		void OnUpdate();
		void OnRender();
		void OnEndMove();
		void OnChangePosition();

		bool OnIsType(DWORD dwType);

		D3DXVECTOR2 m_v2SrcPos, m_v2DstPos, m_v2NextPos, m_v2Direction, m_v2NextDistance;
		float m_fDistance, m_fMoveSpeed;
		bool m_bIsMove;
	};
	class CMoveImageBox : public CImageBox
	{
		public:
			CMoveImageBox();
			virtual ~CMoveImageBox();

			static DWORD Type();

			void SetMoveSpeed(float fSpeed);
			void SetMovePosition(float fDstX, float fDstY);
			bool GetMove();
			void MoveStart();
			void MoveStop();

		protected:
			virtual void OnCreateInstance();
			virtual void OnDestroyInstance();

			virtual void OnUpdate();
			virtual void OnRender();
			virtual void OnEndMove();

			bool OnIsType(DWORD dwType);

			Vector2 m_v2SrcPos, m_v2DstPos, m_v2NextPos, m_v2Direction, m_v2NextDistance;
			float m_fDistance, m_fMoveSpeed;
			bool m_bIsMove;
	};
	class CMoveScaleImageBox : public CMoveImageBox
	{
		public:
			CMoveScaleImageBox();
			virtual ~CMoveScaleImageBox();

			static DWORD Type();

			void SetMaxScale(float fMaxScale);
			void SetMaxScaleRate(float fMaxScaleRate);
			void SetScalePivotCenter(bool bScalePivotCenter);

		protected:
			virtual void OnCreateInstance();
			virtual void OnDestroyInstance();

			virtual void OnUpdate();

			bool OnIsType(DWORD dwType);

			float m_fMaxScale, m_fMaxScaleRate, m_fScaleDistance, m_fAdditionalScale;
			Vector2 m_v2CurScale;
	};

	class CMarkBox : public CWindow {
	public:
		CMarkBox();
		virtual ~CMarkBox();

		void LoadImage(const char* c_szFilename);
		void SetDiffuseColor(float fr, float fg, float fb, float fa);
		void SetIndex(UINT uIndex);
		void SetScale(FLOAT fScale);

	protected:
		virtual void OnCreateInstance();
		virtual void OnDestroyInstance();

		virtual void OnUpdate();
		virtual void OnRender();
		void         OnChangePosition();
	protected:
		CGraphicMarkInstance* m_pMarkInstance;
	};

	class CExpandedImageBox : public CImageBox {
	public:
		static DWORD Type();

	public:
		CExpandedImageBox();
		virtual ~CExpandedImageBox();

		void SetScale(float fx, float fy);
		void SetOrigin(float fx, float fy);
		void SetRotation(float fRotation);
		void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
		void SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom);
		void SetRenderingMode(int iMode);
		void LeftRightReverse();

	protected:
		void OnCreateInstance();
		void OnDestroyInstance();

		virtual void OnUpdate();
		virtual void OnRender();

		bool OnIsType(DWORD dwType);
	};

	class CAniImageBox : public CWindow {
	public:
		static DWORD Type();

	public:
		CAniImageBox();
		virtual ~CAniImageBox();

		CAniImageBox(const CAniImageBox &m) = delete;
		CAniImageBox & operator= (const CAniImageBox &) = delete;

		void SetDelay(int iDelay);
		void AppendImage(const char* c_szFileName);

		void         SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
		void         SetRenderingRectWithScale(float fLeft, float fTop, float fRight, float fBottom);
		void         SetRenderingMode(int iMode);
		void         SetDiffuseColor(float r, float g, float b, float a);
                void SetOrigin(float x, float y);
                void SetRotation(float rot);
        void         SetScale(float scale);
        void         SetAlpha(float fAlpha);
		float        GetAlpha() const;
		void         Stop();
		void         Start();

		void SetAniImgScale(float x, float y);

		void ResetFrame();

	protected:
		void         OnUpdate();
		void         OnRender();
		void         OnChangePosition();
		virtual void OnEndFrame();

		bool OnIsType(DWORD dwType);

	protected:
		BYTE                                                        m_bycurDelay;
		BYTE                                                        m_byDelay;
		BYTE                                                        m_bycurIndex;
		bool                                                        m_isRunning;
		std::vector<std::unique_ptr<CGraphicExpandedImageInstance>> m_ImageVector;
		D3DXCOLOR m_diffuse;
	};

	class CRenderTarget : public CWindow {
	public:
		static DWORD Type();
		bool         OnIsType(DWORD dwType) override;

	public:
		CRenderTarget();
		virtual ~CRenderTarget();

		void SetRenderTarget(DWORD index);
		bool SetWikiRenderTarget(int iRenderTargetInex);
		void OnUpdate();

	protected:
		void OnRender() override;
	private:
		CGraphicRenderTargetTexture* m_renderTarget;
		int32_t	m_iRenderTargetIndex;
	};

	class ColorPicker : public CWindow {
	public:
		static DWORD Type();
		bool         OnIsType(DWORD dwType) override;

		void Create();
		void OnChangePosition();
	public:
		ColorPicker();
		virtual ~ColorPicker();

	protected:
		void OnRender() override;
		void OnMouseDrag(int32_t x, int32_t y);

	private:
		Vector2 m_v2Position{};
	};

	// Button
	class CButton : public CWindow {
	public:
		CButton();
		virtual ~CButton() = default;
		void    SetButtonScale(float xScale, float yScale);
		int     GetButtonImageWidth() const;
		int     GetButtonImageHeight() const;

		bool SetUpVisual(const char* c_szFileName);
		bool SetOverVisual(const char* c_szFileName);
		bool SetDownVisual(const char* c_szFileName);
		bool SetDisableVisual(const char* c_szFileName);

		const char* GetUpVisualFileName();
		const char* GetOverVisualFileName();
		const char* GetDownVisualFileName();

		void Flash();
		void EnableFlash();
		void DisableFlash();

		void Enable();
		void Disable();

		void SetUp();
		void Up();
		void Over();
		void Down();
		void LeftRightReverse();
		bool IsDisable() const;
		bool IsPressed() const;
		bool IsEnable() const;

		void SetDiffuseColor(float fr, float fg, float fb, float fa);
		void SetAlpha(float fAlpha);
		float GetAlpha() const;
		void SetAlwaysTooltip(bool val);
	protected:
		void OnUpdate();
		void OnRender();
		void OnChangePosition();

		bool OnMouseLeftButtonDown();
		bool OnMouseLeftButtonDoubleClick();
		bool OnMouseLeftButtonUp();
		bool OnMouseOverIn();
		bool OnMouseOverOut();

		void SetCurrentVisual(CGraphicImageInstance* pVisual);
		void SetFlashVisual(CGraphicImageInstance* visual);
	protected:
		bool                   m_bEnable;
		bool                   m_isPressed;
		bool                   m_isFlash;
		CGraphicImageInstance* m_pcurVisual;
		CGraphicImageInstance  m_upVisual;
		CGraphicImageInstance  m_overVisual;
		CGraphicImageInstance  m_downVisual;
		CGraphicImageInstance  m_disableVisual;
		CGraphicImageInstance* m_pFlashVisual;
		Color m_diffuse = { 1.0f, 1.0f, 1.0f, 1.0f};
	public:
		bool m_bIsAlwaysShowTooltip = false;
	};

	class CRadioButton : public CButton {
	public:
		CRadioButton();
		virtual ~CRadioButton();

	protected:
		bool OnMouseLeftButtonDown();
		bool OnMouseLeftButtonUp();
		bool OnMouseOverIn();
		bool OnMouseOverOut();
	};

	class CToggleButton : public CButton {
	public:
		CToggleButton();
		virtual ~CToggleButton();

	protected:
		bool OnMouseLeftButtonDown();
		bool OnMouseLeftButtonUp();
		bool OnMouseOverIn();
		bool OnMouseOverOut();
	};

	class CDragButton : public CButton {
	public:
		CDragButton();
		virtual ~CDragButton();

		
		static DWORD Type();
		bool OnIsType(DWORD dwType);

		void SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight);

	protected:
		void OnChangePosition();
		bool OnMouseOverIn();
		bool OnMouseOverOut();

		RECT m_restrictArea;
	};

	class CDragBar : public CBar {
	public:
		CDragBar();
		virtual ~CDragBar();

		static DWORD Type();
		bool OnIsType(DWORD dwType);

		void SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight);
		bool IsPressed() const { return m_isPressed; }
	protected:
		void OnChangePosition();
		bool OnMouseOverIn();
		bool OnMouseOverOut();
		bool OnMouseLeftButtonDown();
		bool OnMouseLeftButtonDoubleClick();
		bool OnMouseLeftButtonUp();

		RECT m_restrictArea;
		bool m_isPressed = false;
	};
};



