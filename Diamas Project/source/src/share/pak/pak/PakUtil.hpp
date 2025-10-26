#ifndef METIN2_CLIENT_ETERPACK_PAKUTIL_HPP
#define METIN2_CLIENT_ETERPACK_PAKUTIL_HPP

// ptr either points to the output- or to the temp. buffer
// This class is used to abstract memory-management away from the consumer.
// It is useful when you have to dynamically decide whether you work
// directly on a user-provided buffer/need to allocate a temp. one.
struct TempOrOutputBuffer
{
	static const uint32_t kLocalSize = 8 * 1024;

	TempOrOutputBuffer(uint8_t* output)
		: ptr(output)
		, output(output)
	{
		// ctor
	}

	~TempOrOutputBuffer()
	{
		if (ptr != output && ptr != local)
			delete[] ptr;
	}

	void MakeTemporaryBuffer(std::size_t size)
	{
		if (size < sizeof(local))
			ptr = local;
		else
			ptr = new uint8_t[size];
	}

	uint8_t* ptr{};
	uint8_t* output{};
	uint8_t local[kLocalSize]{};
};



#endif
