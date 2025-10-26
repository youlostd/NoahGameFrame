#include "Thing.h"
#include "Model.h"
#include "Motion.h"
#include "StdAfx.h"
#include "ThingInstance.h"

namespace
{
struct my_file_info {
    granny_uint32 Sentinel;
    granny_file_info* FileInfo;
};

// Explicitly declare them as uint32s here to avoid compiler warnings
// due to |GrannyFirstGRNStandardTag| being converted to signed int.
const granny_uint32 kGrnCurrent = GrannyCurrentGRNStandardTag;
const granny_uint32 kGrnStart = GrannyFirstGRNStandardTag;

// (current - first) gives us the current "revision", which is necessary,
// since our tag needs to change when the underlying GrannyFileInfoType
// changes.
const granny_uint32 kCurrentTag = kGrnCurrent - kGrnStart + 0x70000000;

const granny_uint32 kSentinel = 0xdeadbeef;

granny_data_type_definition MyFileInfoType[] = {
    {GrannyUInt32Member, "Sentinel"},
    {GrannyReferenceMember, "FileInfo", GrannyFileInfoType},
    {GrannyEndMember}};

my_file_info* GetMyFileInfo(granny_file* File)
{
    granny_variant Root;
    GrannyGetDataTreeFromFile(File, &Root);

    if (File->Header->TypeTag == kCurrentTag) {
        return (my_file_info*)Root.Object;
    } else {
        if (File->ConversionBuffer == nullptr) {
            // Log Warning about conversion operation
            File->ConversionBuffer = GrannyConvertTree(
                Root.Type, Root.Object, MyFileInfoType, nullptr, nullptr);
        }

        return (my_file_info*)File->ConversionBuffer;
    }
}
} // namespace

CGraphicThing::CGraphicThing()
{
    Initialize();
}

CGraphicThing::~CGraphicThing()
{
    // OnClear();
    Clear();
}

void CGraphicThing::Initialize()
{
    m_pgrnFile = NULL;
    m_pgrnFileInfo = NULL;
    m_pgrnAni = NULL;

    m_models = NULL;
    m_motions = NULL;
}

void CGraphicThing::Clear()
{
    delete[] m_motions;
    delete[] m_models;

    if (m_pgrnFile)
        GrannyFreeFile(m_pgrnFile);

    Initialize();
}

bool CGraphicThing::CreateDeviceObjects()
{
    if (!m_pgrnFileInfo)
        return true;

    for (int m = 0; m < m_pgrnFileInfo->ModelCount; ++m) {
        CGrannyModel& rModel = m_models[m];
        rModel.CreateDeviceObjects();
    }

    return true;
}

void CGraphicThing::DestroyDeviceObjects()
{
    if (!m_pgrnFileInfo)
        return;

    for (int m = 0; m < m_pgrnFileInfo->ModelCount; ++m) {
        CGrannyModel& rModel = m_models[m];
        rModel.DestroyDeviceObjects();
    }
}

bool CGraphicThing::CheckModelIndex(int iModel) const
{
    if (!m_pgrnFileInfo) {
        SPDLOG_DEBUG("m_pgrnFileInfo == NULL: {0}", GetFileName());
        return false;
    }

    assert(m_pgrnFileInfo != NULL);

    if (iModel < 0)
        return false;

    if (iModel >= m_pgrnFileInfo->ModelCount)
        return false;

    return true;
}

bool CGraphicThing::CheckMotionIndex(int iMotion) const
{
    // Temporary
    if (!m_pgrnFileInfo)
        return false;
    // Temporary

    assert(m_pgrnFileInfo != NULL);

    if (iMotion < 0)
        return false;

    if (iMotion >= m_pgrnFileInfo->AnimationCount)
        return false;

    return true;
}

CGrannyModel* CGraphicThing::GetModelPointer(int iModel)
{
    assert(CheckModelIndex(iModel));
    assert(m_models != NULL);
    return m_models + iModel;
}

CGrannyMotion* CGraphicThing::GetMotionPointer(int iMotion)
{
    assert(CheckMotionIndex(iMotion));

    if (iMotion >= m_pgrnFileInfo->AnimationCount)
        return NULL;

    assert(m_motions != NULL);
    return (m_motions + iMotion);
}

int CGraphicThing::GetModelCount() const
{
    if (!m_pgrnFileInfo)
        return 0;

    return (m_pgrnFileInfo->ModelCount);
}

int CGraphicThing::GetMotionCount() const
{
    if (!m_pgrnFileInfo)
        return 0;

    return (m_pgrnFileInfo->AnimationCount);
}

bool CGraphicThing::Load(int iSize, const void* c_pvBuf)
{
    if (!c_pvBuf)
        return false;

    m_pgrnFile = GrannyReadEntireFileFromMemory(iSize, (void*)c_pvBuf);

    if (!m_pgrnFile) {
        return false;
    }

    m_pgrnFileInfo = GrannyGetFileInfo(m_pgrnFile);

    if (!m_pgrnFileInfo) {
        SPDLOG_ERROR("Failed to read granny file info");
        return false;
    }

    return LoadModels() && LoadMotions();
}

// SUPPORT_LOCAL_TEXTURE
static std::string gs_modelLocalPath;

const std::string& GetModelLocalPath()
{
    return gs_modelLocalPath;
}

// END_OF_SUPPORT_LOCAL_TEXTURE

bool CGraphicThing::LoadModels()
{
    assert(m_pgrnFile != NULL);
    assert(m_models == NULL);

    if (m_pgrnFileInfo->ModelCount <= 0)
        return true;

    // SUPPORT_LOCAL_TEXTURE
    const std::string fileName(GetFileNameString().data(),
                               GetFileNameString().length());

    // if (fileName.length() > 2 && fileName[1] != ':')
    {
        int sepPos = fileName.rfind('\\');
        gs_modelLocalPath.assign(fileName, 0, sepPos + 1);
    }
    // END_OF_SUPPORT_LOCAL_TEXTURE

    int modelCount = m_pgrnFileInfo->ModelCount;

    m_models = new CGrannyModel[modelCount];

    for (int m = 0; m < modelCount; ++m) {
        CGrannyModel& rModel = m_models[m];
        granny_model* pgrnModel = m_pgrnFileInfo->Models[m];

        if (!rModel.CreateFromGrannyModelPointer(pgrnModel))
            return false;

        rModel.SetFromFilename(GetFileName());
    }

    GrannyFreeFileSection(m_pgrnFile, GrannyStandardRigidVertexSection);
    GrannyFreeFileSection(m_pgrnFile, GrannyStandardRigidIndexSection);
    GrannyFreeFileSection(m_pgrnFile, GrannyStandardDeformableIndexSection);
    GrannyFreeFileSection(m_pgrnFile, GrannyStandardTextureSection);
    return true;
}

bool CGraphicThing::LoadMotions()
{
    assert(m_pgrnFile != NULL);
    assert(m_motions == NULL);

    if (m_pgrnFileInfo->AnimationCount <= 0)
        return true;

    int motionCount = m_pgrnFileInfo->AnimationCount;

    m_motions = new CGrannyMotion[motionCount];

    for (int m = 0; m < motionCount; ++m) {
        if (!m_motions[m].BindGrannyAnimation(m_pgrnFileInfo->Animations[m]))
            return false;

        m_motions[m].SetFromFilename(GetFileName());
    }

    return true;
}
