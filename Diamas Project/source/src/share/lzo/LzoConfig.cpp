#include <lzo/LzoConfig.hpp>

METIN2_BEGIN_NS

namespace
{

const char* kErrorMessages[] = {
	"Error",
	"Out of memory",
	"Not compressible",
	"Input overrun",
	"Output overrun",
	"Look-behind overun",
	"EOF not found",
	"Input not consumed",
	"Not yet implemented",
	"Invalid argument"
};

}

const char* GetLzoErrorString(int code)
{
	if (code < 0) {
		uint32_t unsignedCode = -code;

		if (unsignedCode < STORM_ARRAYSIZE(kErrorMessages))
			return kErrorMessages[unsignedCode];
	}

	return "Error code out of range";
}

METIN2_END_NS
