#include <base/DumpVersion.hpp>

#include <BuildInfo.hpp>
#include <Version.hpp>

#include <cstdio>



const char* kMachineVersionString =
	"@@VERSION@@" METIN2_BUILD_USER "@" METIN2_BUILD_MACHINE
	"@" METIN2_GIT_REV "@" METIN2_BUILD_NAME;

const char* kHumanVersionString =
	METIN2_BUILD_USER "@" METIN2_BUILD_MACHINE " - "
	METIN2_BUILD_NAME "@" METIN2_GIT_REV;

void DumpVersion()
{
	using namespace std;
	fprintf(stderr, "%s\nv%s\n",
	        kHumanVersionString,
	        METIN2_PRODUCTVERSION_STR);
}


