#include "attribute.h"
#include <memory>
#include <cassert>

#define SET_BIT(var,bit)                ((var) |= (bit))
#define REMOVE_BIT(var,bit)             ((var) &= ~(bit))

CAttribute::CAttribute(uint32_t width, uint32_t height)
{
	Initialize(width, height);
	Alloc(true);
}

CAttribute::CAttribute(uint32_t * attr, uint32_t width, uint32_t height)
{
	Initialize(width, height);

	int i;
	int size = width * height;

	for (i = 0; i < size; ++i)
		if (attr[0] != attr[i])
			break;

	// 속성이 전부 같으면 단지 defaultAttr만 설정한다.
	if (i == size) {
		defaultAttr = attr[0];
	}
	else {
		int allAttr = 0;

		for (i = 0; i < size; ++i)
			allAttr |= attr[i];

		// 하위 8비트만 사용할 경우 D_BYTE
		if (!(allAttr & 0xffffff00))
			dataType = D_BYTE;
		// 하위 16비트만 사용할 경우 D_WORD
		else if (!(allAttr & 0xffff0000))
			dataType = D_WORD;
		else // 그 이외에는 D_DWORD
			dataType = D_DWORD;

		Alloc(false);

		if (dataType == D_DWORD) {
			// D_DWORD일 때는 원본 속성과 같으므로 단지 복사.
			memcpy(data, attr, sizeof(uint32_t) * width * height);
		}
		else {
			if (dataType == D_BYTE) {
				for (uint32_t y = 0; y < height; ++y)
					for (uint32_t x = 0; x < width; ++x)
						bytePtr[y * width + x] = *attr++;
			}
			else if (dataType == D_WORD) {
				for (uint32_t y = 0; y < height; ++y)
					for (uint32_t x = 0; x < width; ++x)
						wordPtr[y * width + x] = *attr++;
			}
		}
	}
}

CAttribute::~CAttribute()
{
	if (data)
		free(data);
}

int CAttribute::GetDataType()
{
	return dataType;
}

void* CAttribute::GetDataPtr()
{
	return data;
}

uint32_t CAttribute::Get(uint32_t position) const
{
	if (position > width * height)
		return 0;

	if (!data)
		return defaultAttr;

	if (bytePtr)
		return bytePtr[position];

	if (wordPtr)
		return wordPtr[position];

	return dwordPtr[position];
}

uint32_t CAttribute::Get(uint32_t x, uint32_t y)
{
	if (x > width || y > height)
		return 0;

	if (!data)
		return defaultAttr;

	if (bytePtr)
		return bytePtr[y * width + x];

	if (wordPtr)
		return wordPtr[y * width + x];

	return dwordPtr[y * width + x];
}

void CAttribute::Set(uint32_t x, uint32_t y, uint32_t attr)
{
	if (x > width || y > height)
		return;

	if (!data)
		Alloc(true);

	if (bytePtr) {
		SET_BIT(bytePtr[y * width + x], attr);
		return;
	}

	if (wordPtr) {
		SET_BIT(wordPtr[y * width + x], attr);
		return;
	}

	SET_BIT(dwordPtr[y * width + x], attr);
}

void CAttribute::Remove(uint32_t x, uint32_t y, uint32_t attr)
{
	if (x > width || y > height)
		return;

	if (!data) // 속성을 삭제할 때 만약 데이터가 없으면 그냥 리턴한다.
		return;

	if (bytePtr) {
		REMOVE_BIT(bytePtr[y * width + x], attr);
		return;
	}

	if (wordPtr) {
		REMOVE_BIT(wordPtr[y * width + x], attr);
		return;
	}

	REMOVE_BIT(dwordPtr[y * width + x], attr);
}

void CAttribute::Initialize(uint32_t w, uint32_t h)
{
	dataType = D_DWORD;
	defaultAttr = 0;
	width = w;
	height = h;
	data = NULL;
	bytePtr = NULL;
	wordPtr = NULL;
	dwordPtr = NULL;
}

void CAttribute::Alloc(bool initialize)
{
	size_t memSize;

	switch (dataType) {
	case D_DWORD:
		memSize = width * height * sizeof(uint32_t);
		break;

	case D_WORD:
		memSize = width * height * sizeof(uint16_t);
		break;

	case D_BYTE:
		memSize = width * height;
		break;

	default:
		assert(false && "dataType error!");
		return;
	}

	data = malloc(memSize);

	switch (dataType) {
	case D_DWORD: {
		dwordPtr = static_cast<uint32_t*>(data);
		if (initialize)
			std::memset(dwordPtr, 0, memSize);
		break;
	}

	case D_WORD: {
		wordPtr = static_cast<uint16_t*>(data);
		if (initialize)
			std::memset(wordPtr, 0, memSize);
		break;
	}

	case D_BYTE: {
		bytePtr = static_cast<uint8_t*>(data);
		if (initialize)
			std::memset(bytePtr, 0, memSize);
		break;
	}
	}
}
