#include "GrannyRuntime.hpp"

#include <storm/memory/AbstractAllocator.hpp>

#include <granny.h>
#include <SpdLog.hpp>



namespace
{

bool initialized = false;

GRANNY_CALLBACK(void*)
CustomAllocator(const char* SourceFile,
                const granny_int32x SourceLine,
                granny_uintaddrx Alignment,
                granny_uintaddrx Size,
                granny_int32x Intent)
{
	// TODO(tim): Add logging(?)
	/* Size == 0 is a valid allocation, if a little strange.  Granny will never pass this
	   value, but we'll imitate the behavior for new or malloc: simply return NULL. */
	if (Size == 0)
		return NULL;

	uint32_t flags = 0;

	if (Intent == GrannyAllocationTemporary)
		flags |= storm::AllocationFlags::kTemp;
	else if (Intent == GrannyAllocationLongTerm)
		flags |= storm::AllocationFlags::kPerm;

	return storm::GetDefaultAllocator()->Allocate(Size, Alignment, 0, flags);
}

GRANNY_CALLBACK(void)
CustomDeallocator(const char* SourceFile,
                  const granny_int32x SourceLine,
                  void* Memory)
{
	// TODO(tim): Add logging(?)
	storm::GetDefaultAllocator()->Deallocate(Memory);
}

GRANNY_CALLBACK(void) LogCallback(granny_log_message_type Type,
                                  granny_log_message_origin Origin,
                                  char const* File, granny_int32x Line,
                                  char const * Message,
                                  void* /*UserData*/)
{
	STORM_ASSERT(Message, "No message?");

	char const* TypeString = GrannyGetLogMessageTypeString(Type);
	char const* OriginString = GrannyGetLogMessageOriginString(Origin);

	if (Type == GrannyErrorLogMessage)
		SPDLOG_ERROR("Granny says: {0} ({1})  {2}({3}): {4}",
		          TypeString, OriginString, File, Line, Message);
	else
		SPDLOG_INFO("Granny says: {0} ({1})  {2}({3}): {4}",
		          TypeString, OriginString, File, Line, Message);
}

}

void InitializeGrannyRuntime()
{
	STORM_ASSERT(!initialized, "Already initialized");
	initialized = true;

	GrannySetAllocator(CustomAllocator, CustomDeallocator);

	granny_log_callback NewCallback;
	NewCallback.Function = LogCallback;
	NewCallback.UserData = NULL;
	GrannySetLogCallback(&NewCallback);

	GrannyFilterMessage(GrannyFileReadingLogMessage, false);
}

void DestroyGrannyRuntime()
{
	STORM_ASSERT(initialized, "Not initialized");
	// TODO(tim): What's here?
}


