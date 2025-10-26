#include <base/Clipboard.hpp>

#include <Version.hpp>

#include <storm/UnicodeUtil.hpp>
#include <storm/WindowsPlatform.hpp>

#include <boost/system/error_code.hpp>

#include <vector>



namespace
{

struct ScopedClipboard
{
	ScopedClipboard()
		: ok(OpenClipboard(GetActiveWindow()))
	{
		if (!ok)
			spdlog::error("OpenClipboard() failed with {0}",
			          GetLastError());
	}

	~ScopedClipboard()
	{
		if (ok)
			CloseClipboard();
	}

	bool ok;
};

// TODO(tim): Perhaps expose this to other code?
template <typename T>
struct ScopedGlobal
{
	ScopedGlobal(HGLOBAL global)
		: global(global)
		, ptr(static_cast<T*>(GlobalLock(global)))
	{
		if (!ptr) {
			spdlog::error("GlobalLock({0}) failed with {1}",
			          global,
			          (GetLastError()));
		}
	}

	~ScopedGlobal()
	{
		// https://msdn.microsoft.com/en-us/library/windows/desktop/aa366595%28v=vs.85%29.aspx
		if (ptr && !GlobalUnlock(global)) {
			const auto error = GetLastError();
			if (error != NO_ERROR) {
				spdlog::error("GlobalUnlock({0}) failed with {1}",
				          global, (error));
			}
		}
	}

	HGLOBAL global;
	T* ptr;
};

}

ClipboardType CF_METIN2;

storm::String GetClipboardText()
{
	ScopedClipboard clipboard;
	if (!clipboard.ok)
		return storm::String();

	HANDLE data = GetClipboardData(CF_UNICODETEXT);
	if (data == nullptr)
		return storm::String();

	ScopedGlobal<wchar_t> global(data);
	if (!global.ptr)
		return storm::String();

	size_t len = wcslen(global.ptr);
	storm::String str;
	str.resize(len * 2); // UTF-16 -> UTF-8

	bsys::error_code ec;
	len = storm::ConvertUtf16ToUtf8(global.ptr,
	                                global.ptr + len,
	                                &str[0],
	                                &str[0] + str.length(),
	                                ec);

	str.resize(len);
	return str;
}

std::pair<std::unique_ptr<uint8_t>, size_t> GetClipboardContent(ClipboardType type)
{
	ScopedClipboard clipboard;
	if (!clipboard.ok)
		return std::make_pair(nullptr, 0);

	// Try to get Metin format first
	HANDLE data = GetClipboardData(type);
	if (data) {
		ScopedGlobal<void> global(data);
		if (global.ptr) {
			size_t size = GlobalSize(data);

			if (size == 0) {
				spdlog::error("GlobalSize() failed with {0}",
				          (GetLastError()));
				return std::make_pair(nullptr,0);
			}

			std::unique_ptr<uint8_t> dst(new uint8_t[size]);
			memcpy(dst.get(), global.ptr, size);
			return std::make_pair(std::move(dst), size);
		}
	}

	return std::make_pair(nullptr, 0);
}

bool ClearClipboard()
{
	ScopedClipboard clipboard;
	if (!clipboard.ok)
		return false;

	if (!EmptyClipboard()) {
		spdlog::error("EmptyClipboard() failed with {0}",
		          (GetLastError()));
		return false;
	}

	return true;
}

bool SetClipboardContent(ClipboardType type, const void* srcdata, size_t len)
{
	ScopedClipboard clipboard;
	if (!clipboard.ok)
		return false;

	HGLOBAL data = GlobalAlloc(GMEM_MOVEABLE,
	                           len);

	if (!data) {
		spdlog::error("GlobalAlloc() failed with {0}",
		          (GetLastError()));
		return false;
	}

	ScopedGlobal<void> global(data);

	if (!global.ptr)
		return false;

	memcpy(global.ptr, srcdata, len);

	if (!SetClipboardData(type, data)) {
		spdlog::error("SetClipboardData({0}) failed with {1}",
		          data, (GetLastError()));
		return false;
	}

	return true;
}

bool SetClipboardText(const storm::StringRef& str)
{
	ScopedClipboard clipboard;
	if (!clipboard.ok)
		return false;

	size_t len = str.length();

	// The output string has to be smaller than |len|,
	// since our input is UTF-8 encoded and our output UTF-16,
	// but we can save a temporary buffer / an additional conversion
	// by simply using the string length here...
	HGLOBAL data = GlobalAlloc(GMEM_MOVEABLE,
	                           sizeof(wchar_t) * (len + 1));

	if (!data) {
		spdlog::error("GlobalAlloc() failed with {0}",
		          (GetLastError()));
		return false;
	}

	{
		ScopedGlobal<wchar_t> global(data);
		if (!global.ptr)
			return false;

		bsys::error_code ec;
		len = storm::ConvertUtf8ToUtf16(str.data(),
		                                str.data() + len,
		                                global.ptr,
		                                global.ptr + len,
		                                ec);

		if (ec) {
			spdlog::error("Failed to encode clipboard data '{0}' with {1}",
			          str, ec);
			return false;
		}

		// NUL terminator - required
		global.ptr[len] = 0;
	}

	if (!SetClipboardData(CF_UNICODETEXT, data)) {
		spdlog::error("SetClipboardData({0}) failed with {1}",
		          data, (GetLastError()));
		return false;
	}

	return true;
}

bool ClipboardInit()
{
	CF_METIN2 = RegisterClipboardFormat(METIN2_BRAND_NAME "METIN2");

	if (CF_METIN2 == 0) {
		spdlog::error("RegisterClipboardFormat failed with {0}",
		          (GetLastError()));
		return false;
	}

	return true;
}


