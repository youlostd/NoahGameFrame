#include "MarkImage.h"

#include <base/Crc32.hpp>

#include <lz4.h>

CGuildMarkImage::CGuildMarkImage()
    : m_aakBlock{}, m_apxImage{}
{
    m_uImg = INVALID_HANDLE;
}

CGuildMarkImage::~CGuildMarkImage()
{
    Destroy();
}

void CGuildMarkImage::Destroy()
{
    if (INVALID_HANDLE == m_uImg)
        return;

    ilDeleteImages(1, &m_uImg);
    m_uImg = INVALID_HANDLE;
}

void CGuildMarkImage::Create()
{
    if (INVALID_HANDLE != m_uImg)
        return;

    ilGenImages(1, &m_uImg);
}

bool CGuildMarkImage::Save(const char *c_szFileName)
{
    ilEnable(IL_FILE_OVERWRITE);
    ilBindImage(m_uImg);
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

    if (!ilSave(IL_TGA, (const ILstring)c_szFileName))
        return false;

    return true;
}

bool CGuildMarkImage::Build(const char *c_szFileName)
{
    Destroy();
    Create();

    ilBindImage(m_uImg);
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

    uint8_t *data = (uint8_t *)malloc(sizeof(Pixel) * WIDTH * HEIGHT);
    memset(data, 0, sizeof(Pixel) * WIDTH * HEIGHT);

    if (!ilTexImage(WIDTH, HEIGHT, 1, 4, IL_BGRA, IL_UNSIGNED_BYTE, data))
    {
        SPDLOG_ERROR("CGuildMarkImage: cannot initialize image");
        return false;
    }

    free(data);

    ilEnable(IL_FILE_OVERWRITE);

    if (!ilSave(IL_TGA, (const ILstring)c_szFileName))
        return false;

    return true;
}

bool CGuildMarkImage::Load(const char *c_szFileName)
{
    Destroy();
    Create();

    ilBindImage(m_uImg);
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

    if (!ilLoad(IL_TGA, (const ILstring)c_szFileName))
    {
        Build(c_szFileName);
        ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);

        if (!Load(c_szFileName))
        {
            SPDLOG_ERROR("CGuildMarkImage: cannot open file for writing {0}", c_szFileName);
            return false;
        }
    }

    if (ilGetInteger(IL_IMAGE_WIDTH) != WIDTH)
    {
        SPDLOG_ERROR("CGuildMarkImage: {0} width must be {1}", c_szFileName, WIDTH);
        return false;
    }

    if (ilGetInteger(IL_IMAGE_HEIGHT) != HEIGHT)
    {
        SPDLOG_ERROR("CGuildMarkImage: {0} height must be {1}", c_szFileName, HEIGHT);
        return false;
    }

    ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);

    BuildAllBlocks();
    return true;
}

void CGuildMarkImage::PutData(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void *data)
{
    ilBindImage(m_uImg);
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    ilSetPixels(x, y, 0, width, height, 1, IL_BGRA, IL_UNSIGNED_BYTE, data);
}

void CGuildMarkImage::GetData(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void *data)
{
    ilBindImage(m_uImg);
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    ilCopyPixels(x, y, 0, width, height, 1, IL_BGRA, IL_UNSIGNED_BYTE, data);
}

// 이미지 = 512x512
//   블럭 = 마크 4 x 4
//   마크 = 16 x 12
// 한 이미지의 블럭 = 8 x 10

bool CGuildMarkImage::SaveBlockFromCompressedData(uint32_t posBlock, const uint8_t *pbComp, uint32_t dwCompSize)
{
    if (posBlock >= BLOCK_TOTAL_COUNT)
        return false;

    Pixel apxBuf[SGuildMarkBlock::SIZE];
    int32_t sizeBuf = LZ4_decompress_safe((const char *)pbComp, (char *)apxBuf, dwCompSize, sizeof(apxBuf));
    if (sizeBuf < 0)
    {
        SPDLOG_ERROR("cannot decompress, compressed size = {0}", dwCompSize);
        return false;
    }

    if (sizeBuf != sizeof(apxBuf))
    {
        SPDLOG_ERROR("image corrupted, decompressed size = {0}", sizeBuf);
        return false;
    }

    uint32_t rowBlock = posBlock / BLOCK_COL_COUNT;
    uint32_t colBlock = posBlock % BLOCK_COL_COUNT;

    PutData(colBlock * SGuildMarkBlock::WIDTH,
            rowBlock * SGuildMarkBlock::HEIGHT,
            SGuildMarkBlock::WIDTH,
            SGuildMarkBlock::HEIGHT,
            apxBuf);

    m_aakBlock[rowBlock][colBlock].CopyFrom(pbComp, dwCompSize,
                                            ComputeCrc32(0, (const char *)apxBuf,
                                                         sizeof(Pixel) * SGuildMarkBlock::SIZE));
    return true;
}

void CGuildMarkImage::BuildAllBlocks() // 이미지 전체를 블럭화
{
    Pixel apxBuf[SGuildMarkBlock::SIZE];
    SPDLOG_INFO("CGuildMarkImage::BuildAllBlocks");

    for (uint32_t row = 0; row < BLOCK_ROW_COUNT; ++row)
    {
        for (uint32_t col = 0; col < BLOCK_COL_COUNT; ++col)
        {
            GetData(col * SGuildMarkBlock::WIDTH,
                    row * SGuildMarkBlock::HEIGHT,
                    SGuildMarkBlock::WIDTH,
                    SGuildMarkBlock::HEIGHT,
                    apxBuf);

            m_aakBlock[row][col].Compress(apxBuf);
        }
    }
}

uint32_t CGuildMarkImage::GetEmptyPosition()
{
    SGuildMark kMark{};

    for (uint32_t row = 0; row < MARK_ROW_COUNT; ++row)
    {
        for (uint32_t col = 0; col < MARK_COL_COUNT; ++col)
        {
            GetData(col * SGuildMark::WIDTH,
                    row * SGuildMark::HEIGHT,
                    SGuildMark::WIDTH,
                    SGuildMark::HEIGHT,
                    kMark.m_apxBuf);

            if (kMark.IsEmpty())
                return (row * MARK_COL_COUNT + col);
        }
    }

    return INVALID_MARK_POSITION;
}

void CGuildMarkImage::GetBlockCRCList(uint32_t *crcList)
{
    for (uint32_t row = 0; row < BLOCK_ROW_COUNT; ++row)
        for (uint32_t col = 0; col < BLOCK_COL_COUNT; ++col)
            *(crcList++) = m_aakBlock[row][col].GetCRC();
}

////////////////////////////////////////////////////////////////////////////////
void SGuildMark::Clear()
{
    for (uint32_t iPixel = 0; iPixel < SIZE; ++iPixel)
        m_apxBuf[iPixel] = 0xff000000ul;
}

bool SGuildMark::IsEmpty()
{
    for (uint32_t iPixel = 0; iPixel < SIZE; ++iPixel)
        if (m_apxBuf[iPixel] != 0x00000000ul)
            return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t SGuildMarkBlock::GetCRC() const
{
    return m_crc;
}

void SGuildMarkBlock::CopyFrom(const uint8_t *pbCompBuf, uint32_t dwCompSize, uint32_t crc)
{
    if (dwCompSize > LZ4_compressBound(sizeof(Pixel) * SGuildMarkBlock::SIZE))
        return;

    m_sizeCompBuf = dwCompSize;
    memcpy(m_abCompBuf.data(), pbCompBuf, dwCompSize);
    m_crc = crc;
}

void SGuildMarkBlock::Compress(const Pixel *pxBuf)
{
    m_sizeCompBuf = LZ4_compressBound(sizeof(Pixel) * SGuildMarkBlock::SIZE);
    m_abCompBuf.resize(m_sizeCompBuf);

    m_sizeCompBuf = LZ4_compress_default(reinterpret_cast<const char *>(pxBuf),
                                         reinterpret_cast<char *>(m_abCompBuf.data()),
                                         sizeof(Pixel) * SGuildMarkBlock::SIZE, m_sizeCompBuf);
    if (m_sizeCompBuf <= 0)
    {
        SPDLOG_ERROR("Compress: Error! {0} > {1}",
                     sizeof(Pixel) * SGuildMarkBlock::SIZE,
                     m_sizeCompBuf);
        return;
    }

    m_crc = ComputeCrc32(0, (const char *)pxBuf,
                         sizeof(Pixel) * SGuildMarkBlock::SIZE);
}
