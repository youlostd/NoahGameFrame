#ifndef __INC_METIN_II_ATTRIBUTE_H__
#define __INC_METIN_II_ATTRIBUTE_H__

#include <cstdint>

enum EDataType
{
	D_DWORD,
	D_WORD,
	D_BYTE
};

//
// 맵 속성들을 처리할 때 사용
//
class CAttribute
{
public:
	// dword 타잎으로 모두 0을 채운다.
	CAttribute(uint32_t width, uint32_t height);

	// attr을 읽어서 smart하게 속성을 읽어온다.
	CAttribute(uint32_t * attr, uint32_t width, uint32_t height);

	~CAttribute();

	int GetDataType();
	void * GetDataPtr();

	uint32_t Get(uint32_t position) const;
	uint32_t Get(uint32_t x, uint32_t y);
	void Set(uint32_t x, uint32_t y, uint32_t attr);
	void Remove(uint32_t x, uint32_t y, uint32_t attr);

private:
	void Initialize(uint32_t width, uint32_t height);
	void Alloc(bool initialize);

	int dataType;
	uint32_t defaultAttr;
	uint32_t width, height;

	void* data;

	uint8_t* bytePtr;
	uint16_t* wordPtr;
	uint32_t* dwordPtr;
};


#endif
