#ifndef METIN2_CLIENT_ETERGRNLIB_THING_HPP
#define METIN2_CLIENT_ETERGRNLIB_THING_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../EterLib/Resource.h"

using granny_file = struct granny_file;
using granny_file_info = struct granny_file_info;
using granny_animation = struct granny_animation;

class CGrannyModel;
class CGrannyMotion;

class CGraphicThing : public ConcreteResource<CGraphicThing>
{
public:
    CGraphicThing();
    ~CGraphicThing() override;

    void Clear() override;

    bool Load(int iSize, const void *c_pvBuf) override;

    virtual bool CreateDeviceObjects();
    virtual void DestroyDeviceObjects();

    bool CheckModelIndex(int iModel) const;
    CGrannyModel *GetModelPointer(int iModel);
    int GetModelCount() const;

    bool CheckMotionIndex(int iMotion) const;
    CGrannyMotion *GetMotionPointer(int iMotion);
    int GetMotionCount() const;

protected:
    void Initialize();

    bool LoadModels();
    bool LoadMotions();

    granny_file *m_pgrnFile;
    granny_file_info *m_pgrnFileInfo;

    granny_animation *m_pgrnAni;

    CGrannyModel *m_models;
    CGrannyMotion *m_motions;
};

#endif
