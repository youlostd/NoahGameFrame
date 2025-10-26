#ifndef METIN2_SERVER_GAME_MARKIMAGE_HPP
#define METIN2_SERVER_GAME_MARKIMAGE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <IL/il.h>

#include <unordered_map>

typedef uint32_t Pixel;

struct SGuildMark
{
    enum
    {
        WIDTH = 16,
        HEIGHT = 12,
        SIZE = WIDTH * HEIGHT,
    };

    ///////////////////////////////////////////////////////////////////////////////
    Pixel m_apxBuf[SIZE]; // 실제 이미지

    ///////////////////////////////////////////////////////////////////////////////
    void Clear();
    bool IsEmpty();
};

struct SGuildMarkBlock
{
    enum
    {
        MARK_PER_BLOCK_WIDTH = 4,
        MARK_PER_BLOCK_HEIGHT = 4,

        WIDTH = SGuildMark::WIDTH * MARK_PER_BLOCK_WIDTH,
        HEIGHT = SGuildMark::HEIGHT * MARK_PER_BLOCK_HEIGHT,

        SIZE = WIDTH * HEIGHT,
        MAX_COMP_SIZE = (SIZE * sizeof(Pixel)) + ((SIZE * sizeof(Pixel)) >> 4) + 64 + 3
    };

    ///////////////////////////////////////////////////////////////////////////////
    Pixel m_apxBuf[SIZE]; // 실제 이미지

    std::vector<uint8_t> m_abCompBuf; // 압축된 데이터
    int32_t m_sizeCompBuf;            // 압축된 크기
    uint32_t m_crc;                   // 압축된 데이터의 CRC

    ///////////////////////////////////////////////////////////////////////////////
    uint32_t GetCRC() const;

    void CopyFrom(const uint8_t *pbCompBuf, uint32_t dwCompSize, uint32_t crc);
    void Compress(const Pixel *pxBuf);
};

class CGuildMarkImage
{
public:
    enum
    {
        WIDTH = 512,
        HEIGHT = 512,

        BLOCK_ROW_COUNT = HEIGHT / SGuildMarkBlock::HEIGHT,
        // 10
        BLOCK_COL_COUNT = WIDTH / SGuildMarkBlock::WIDTH,
        // 8

        BLOCK_TOTAL_COUNT = BLOCK_ROW_COUNT * BLOCK_COL_COUNT,
        // 80

        MARK_ROW_COUNT = BLOCK_ROW_COUNT * SGuildMarkBlock::MARK_PER_BLOCK_HEIGHT,
        // 40
        MARK_COL_COUNT = BLOCK_COL_COUNT * SGuildMarkBlock::MARK_PER_BLOCK_WIDTH,
        // 32

        MARK_TOTAL_COUNT = MARK_ROW_COUNT * MARK_COL_COUNT,
        // 1280

        INVALID_MARK_POSITION = 0xffffffff,
    };

    CGuildMarkImage();
    ~CGuildMarkImage();

    void Create();
    void Destroy();

    bool Build(const char *c_szFileName);
    bool Save(const char *c_szFileName);
    bool Load(const char *c_szFileName);

    void PutData(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void *data);
    void GetData(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void *data);

    bool SaveMark(uint32_t posMark, uint8_t *pbMarkImage);
    bool DeleteMark(uint32_t posMark);

    uint32_t GetEmptyPosition(); // 빈 마크 위치를 얻는다.

    void GetBlockCRCList(uint32_t *crcList);
    void GetDiffBlocks(const uint32_t *crcList, std::unordered_map<uint8_t, const SGuildMarkBlock *> &mapDiffBlocks);

private:
    enum
    {
        INVALID_HANDLE = 0xffffffff,
    };

    void BuildAllBlocks();

    SGuildMarkBlock m_aakBlock[BLOCK_ROW_COUNT][BLOCK_COL_COUNT];
    Pixel m_apxImage[WIDTH * HEIGHT * sizeof(Pixel)];

    ILuint m_uImg;
};

#endif
