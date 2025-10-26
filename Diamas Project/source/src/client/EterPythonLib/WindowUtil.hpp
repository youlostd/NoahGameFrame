#ifndef METIN2_CLIENT_ETERPYTHONLIB_WINDOWUTIL_HPP
#define METIN2_CLIENT_ETERPYTHONLIB_WINDOWUTIL_HPP

#pragma once


namespace UI
{
	namespace {

		template <typename Function>
		CWindow* TraverseWindowChainUp(CWindow* start, Function& f)
		{
			while (start && start->IsWindow()) {
				if (f(start))
					return start;

				start = start->GetParent();
			}

			return nullptr;
		}

		template <typename Function>
		CWindow* TraverseWindowChainDown(CWindow* start, Function& f)
		{
			if (f(start))
				return start;

			for (auto child : boost::adaptors::reverse(start->GetChildren())) {
				if (!child->IsShow())
					continue;

				child = TraverseWindowChainDown(child, f);
				if (child)
					return child;
			}

			return nullptr;
		}

		// Check whether |parent| is a direct or indirect parent of |start|
		// or equivalent to |start|.
		bool IsParentOf(CWindow* start, CWindow* parent)
		{
			auto f = [parent](CWindow* win) -> bool
			{ return win == parent; };

			return !!TraverseWindowChainUp(start, f);
		}
		CWindow* FindWindowUpwardsByFlag(CWindow* start, uint32_t flags)
		{
			auto f = [flags](CWindow* win) -> bool
			{ return win->IsFlag(flags); };

			return TraverseWindowChainUp(start, f);
		}

		CWindow* FindClippingWindowUpwards(CWindow* start)
		{
			auto f = [](CWindow* win) -> bool
			{ return win->HasClippingEnabled(); };

			return TraverseWindowChainUp(start, f);
		}

		struct Rectangle {
			int32_t X;
			int32_t Y;
			int32_t Width;
			int32_t Height;
		};

		Rectangle clamp(Rectangle smaller, Rectangle larger)
		{
			Rectangle ret;
			ret.X = std::max(smaller.X, larger.X);
			ret.Y = std::max(smaller.Y, larger.Y);
			ret.Width = std::min(smaller.X + smaller.Width, larger.X + larger.Width) - ret.X;
			ret.Height = std::min(smaller.Y + smaller.Height, std::min(smaller.Y + smaller.Height, larger.Y + larger.Height) - ret.Y);
			return ret;
		}

		bool LiesEntirelyOutsideRect(const RECT& r,
			const RECT& r2)
		{
			return (r2.left > r.right
				|| r2.right < r.left
				|| r2.top > r.bottom
				|| r2.bottom < r.top
				);
		}
	}

} // namespace UI

namespace {

	class ScissorsSetter
	{
	public:
		ScissorsSetter(uint32 x, uint32 y, uint32 width, uint32 height);
		~ScissorsSetter();

		static bool isAvailable();
	private:
		RECT oldRect_;
		RECT newRect_;
		DWORD wasEnabled_;
	};
	/**
	 *	This constructor takes x and y, width, height and sets a new
	 *	temporary scissor region on the device using the passed in values
	 *
	 *	@param	x			x pixel offset of top left corner of new rect.
	 *	@param	y			y pixel offset of top left corner of new rect.
	 *	@param	width		new scissor region width.
	 *	@param	height		new scissor region height.
	 */
	ScissorsSetter::ScissorsSetter(uint32 x, uint32 y, uint32 width, uint32 height)
	{
		wasEnabled_ = STATEMANAGER.GetRenderState(D3DRS_SCISSORTESTENABLE);
		STATEMANAGER.GetDevice()->GetScissorRect(&oldRect_);
		newRect_.left = x;
		newRect_.right = x + width;
		newRect_.top = y;
		newRect_.bottom = y + height;
		STATEMANAGER.GetDevice()->SetScissorRect(&newRect_);
		STATEMANAGER.SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	}


	/**
	 *	This destructor automatically resets the saved viewport when
	 *	the class goes out of scope.
	 */
	ScissorsSetter::~ScissorsSetter()
	{
		STATEMANAGER.GetDevice()->SetScissorRect(&oldRect_);
		STATEMANAGER.GetDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, wasEnabled_);
	}


}

#endif /* METIN2_CLIENT_ETERPYTHONLIB_WINDOWUTIL_HPP */
