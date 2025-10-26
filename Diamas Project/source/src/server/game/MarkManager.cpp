#include "MarkManager.h"

#include <base/Crc32.hpp>

#include <iterator>
#include <cstring>

#include "game/GamePacket.hpp"

CGuildMarkManager::CGuildMarkManager()
{
    // 남은 mark id 셋을 만든다. (서버용)
    for (uint32_t i = 0; i < MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT; ++i)
        m_setFreeMarkID.insert(i);
}

CGuildMarkManager::~CGuildMarkManager() { m_mapIdx_Image.clear(); }

bool CGuildMarkManager::GetMarkImageFilename(uint32_t imgIdx, std::string &path) const
{
    if (imgIdx >= MAX_IMAGE_COUNT)
        return false;

    char buf[64];
    snprintf(buf, sizeof(buf), "mark/%s_%u.tga", m_pathPrefix.c_str(), imgIdx);
    path = buf;
    return true;
}

void CGuildMarkManager::SetMarkPathPrefix(const char *prefix) { m_pathPrefix = prefix; }

// 마크 인덱스 불러오기 (서버에서만 사용)
bool CGuildMarkManager::LoadMarkIndex()
{
    char buf[64];
    snprintf(buf, sizeof(buf), "mark/%s_index", m_pathPrefix.c_str());
    FILE *fp = fopen(buf, "r");

    if (!fp)
        return false;

    uint32_t guildID;
    uint32_t markID;

    char line[256];

    while (fgets(line, sizeof(line) - 1, fp))
    {
        sscanf(line, "%u %u", &guildID, &markID);
        line[0] = '\0';
        AddMarkIDByGuildID(guildID, markID);
    }

    LoadMarkImages();

    fclose(fp);
    return true;
}

bool CGuildMarkManager::SaveMarkIndex()
{
    char buf[64];
    snprintf(buf, sizeof(buf), "mark/%s_index", m_pathPrefix.c_str());
    FILE *fp = fopen(buf, "w");

    if (!fp)
    {
        SPDLOG_ERROR("MarkManager::SaveMarkIndex: cannot open index file.");
        return false;
    }

    for (auto it = m_mapGID_MarkID.begin(); it != m_mapGID_MarkID.end(); ++it)
        fprintf(fp, "%u %u\n", it->first, it->second);

    fclose(fp);
    SPDLOG_INFO("MarkManager::SaveMarkIndex: index count {0}", m_mapGID_MarkID.size());
    return true;
}

void CGuildMarkManager::LoadMarkImages()
{
    bool isMarkExists[MAX_IMAGE_COUNT];
    memset(isMarkExists, 0, sizeof(isMarkExists));

    for (auto it = m_mapGID_MarkID.begin(); it != m_mapGID_MarkID.end(); ++it)
    {
        uint32_t markID = it->second;

        if (markID < MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT)
            isMarkExists[markID / CGuildMarkImage::MARK_TOTAL_COUNT] = true;
    }

    for (uint32_t i = 0; i < MAX_IMAGE_COUNT; ++i)
        if (isMarkExists[i])
            __GetImage(i);
}

void CGuildMarkManager::SaveMarkImage(uint32_t imgIdx)
{
    std::string path;

    if (GetMarkImageFilename(imgIdx, path))
        if (!__GetImage(imgIdx)->Save(path.c_str()))
            SPDLOG_ERROR("{0} Save failed\n", path.c_str());
}

CGuildMarkImage *CGuildMarkManager::__GetImage(uint32_t imgIdx)
{
    auto it = m_mapIdx_Image.find(imgIdx);
    if (it == m_mapIdx_Image.end())
    {
        std::string imagePath;

        if (GetMarkImageFilename(imgIdx, imagePath))
        {
            CGuildMarkImage *pkImage = nullptr;
            auto it2 = m_mapIdx_Image.emplace(imgIdx, std::make_unique<CGuildMarkImage>());

            if (it2.first->second && !it2.first->second->Load(imagePath.c_str()))
            {
                it2.first->second->Build(imagePath.c_str());
                it2.first->second->Load(imagePath.c_str());
            }

            return it2.first->second.get();
        }
        else
            return nullptr;
    }
    else
        return it->second.get();
}

bool CGuildMarkManager::AddMarkIDByGuildID(uint32_t guildID, uint32_t markID)
{
    if (markID >= MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT)
        return false;

    //SPDLOG_INFO("MarkManager: guild_id={0} mark_id={1}", guildID, markID);
    m_mapGID_MarkID.emplace(guildID, markID);
    m_setFreeMarkID.erase(markID);
    return true;
}

uint32_t CGuildMarkManager::GetMarkID(uint32_t guildID)
{
    auto it = m_mapGID_MarkID.find(guildID);
    if (it == m_mapGID_MarkID.end())
        return INVALID_MARK_ID;

    return it->second;
}

uint32_t CGuildMarkManager::__AllocMarkID(uint32_t guildID)
{
    auto it = m_setFreeMarkID.lower_bound(0);
    if (it == m_setFreeMarkID.end())
        return INVALID_MARK_ID;

    uint32_t markID = *it;

    uint32_t imgIdx = markID / CGuildMarkImage::MARK_TOTAL_COUNT;
    CGuildMarkImage *pkImage = __GetImage(imgIdx); // 이미지가 없다면 만들기 위해

    if (pkImage && AddMarkIDByGuildID(guildID, markID))
        return markID;

    return INVALID_MARK_ID;
}

uint32_t CGuildMarkManager::GetMarkImageCount() const { return m_mapIdx_Image.size(); }

uint32_t CGuildMarkManager::GetMarkCount() const { return m_mapGID_MarkID.size(); }

// SERVER
std::vector<MarkIndex> CGuildMarkManager::CopyMarkIdx() const
{

    std::vector<MarkIndex> mi;

    for (auto it = m_mapGID_MarkID.begin(); it != m_mapGID_MarkID.end(); ++it)
    {
        mi.emplace_back(MarkIndex{it->first, it->second});

    }
    return mi;
}

// SERVER
uint32_t CGuildMarkManager::SaveMark(uint32_t guildID, uint8_t *pbMarkImage)
{
    uint32_t idMark;

    if ((idMark = GetMarkID(guildID)) == INVALID_MARK_ID)
    {
        if ((idMark = __AllocMarkID(guildID)) == INVALID_MARK_ID)
        {
            SPDLOG_ERROR("CGuildMarkManager: cannot alloc mark id {0}", guildID);
            return false;
        }
        else
            SPDLOG_INFO("SaveMark: mark id alloc {0}", idMark);
    }
    else
        SPDLOG_INFO("SaveMark: mark id found {0}", idMark);

    uint32_t imgIdx = (idMark / CGuildMarkImage::MARK_TOTAL_COUNT);
    CGuildMarkImage *pkImage = __GetImage(imgIdx);

    if (pkImage)
    {
        pkImage->SaveMark(idMark % CGuildMarkImage::MARK_TOTAL_COUNT, pbMarkImage);

        SaveMarkImage(imgIdx);
        SaveMarkIndex();
    }

    return idMark;
}

// SERVER
void CGuildMarkManager::DeleteMark(uint32_t guildID)
{
    auto it = m_mapGID_MarkID.find(guildID);
    if (it == m_mapGID_MarkID.end())
        return;

    CGuildMarkImage *pkImage;

    if ((pkImage = __GetImage(it->second / CGuildMarkImage::MARK_TOTAL_COUNT)) != nullptr)
        pkImage->DeleteMark(it->second % CGuildMarkImage::MARK_TOTAL_COUNT);

    m_setFreeMarkID.insert(it->second);
    m_mapGID_MarkID.erase(it);

    SaveMarkIndex();
}

// SERVER
void CGuildMarkManager::GetDiffBlocks(uint32_t imgIdx, const uint32_t *crcList,
                                      std::unordered_map<uint8_t, const SGuildMarkBlock *> &mapDiffBlocks)
{
    mapDiffBlocks.clear();

    // 클라이언트에서 서버에 없는 이미지를 요청할 수는 없다.
    if (m_mapIdx_Image.end() == m_mapIdx_Image.find(imgIdx))
    {
        SPDLOG_ERROR("invalid idx {0}", imgIdx);
        return;
    }

    CGuildMarkImage *p = __GetImage(imgIdx);

    if (p)
        p->GetDiffBlocks(crcList, mapDiffBlocks);
}

///////////////////////////////////////////////////////////////////////////////////////
// Symbol
///////////////////////////////////////////////////////////////////////////////////////
const CGuildMarkManager::TGuildSymbol *CGuildMarkManager::GetGuildSymbol(uint32_t guildID)
{
    auto it = m_mapSymbol.find(guildID);
    if (it == m_mapSymbol.end())
        return nullptr;

    return &it->second;
}

bool CGuildMarkManager::LoadSymbol(const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (!fp)
        return true;
    else
    {
        uint32_t symbolCount;
        fread(&symbolCount, 4, 1, fp);

        for (uint32_t i = 0; i < symbolCount; i++)
        {
            uint32_t guildID;
            uint32_t dwSize;
            fread(&guildID, 4, 1, fp);
            fread(&dwSize, 4, 1, fp);

            TGuildSymbol gs;
            gs.raw.resize(dwSize);
            fread(&gs.raw[0], 1, dwSize, fp);
            gs.crc = ComputeCrc32(0, &gs.raw[0], dwSize);
            m_mapSymbol.emplace(guildID, gs);
        }
    }

    fclose(fp);
    return true;
}

void CGuildMarkManager::SaveSymbol(const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        SPDLOG_ERROR("Cannot open Symbol file (name: {0})", filename);
        return;
    }

    uint32_t symbolCount = m_mapSymbol.size();
    fwrite(&symbolCount, 4, 1, fp);

    for (auto it = m_mapSymbol.begin(); it != m_mapSymbol.end(); ++it)
    {
        uint32_t guildID = it->first;
        uint32_t dwSize = it->second.raw.size();
        fwrite(&guildID, 4, 1, fp);
        fwrite(&dwSize, 4, 1, fp);
        fwrite(&it->second.raw[0], 1, dwSize, fp);
    }

    fclose(fp);
}

void CGuildMarkManager::UploadSymbol(uint32_t guildID, int iSize, const uint8_t *pbyData)
{

    if (m_mapSymbol.find(guildID) == m_mapSymbol.end())
        m_mapSymbol.emplace(guildID, TGuildSymbol());

    TGuildSymbol &rSymbol = m_mapSymbol[guildID];
    rSymbol.raw.clear();

    if (iSize > 0)
    {
        rSymbol.raw.reserve(iSize);
        std::copy(pbyData, (pbyData + iSize), std::back_inserter(rSymbol.raw));
        rSymbol.crc = ComputeCrc32(0, pbyData, iSize);
    }
}
