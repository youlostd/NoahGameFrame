#ifndef METIN2_CLIENT_GAMELIB_INTERFACE_H
#define METIN2_CLIENT_GAMELIB_INTERFACE_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

// 2004.07.05.myevan.�ý�ź�� �ʿ� ���̴� �����ذ�
class IBackground : public CSingleton<IBackground>
{
  public:
    IBackground()
    {
    }
    virtual ~IBackground()
    {
    }

    virtual bool IsBlock(int x, int y) = 0;
};
#endif /* METIN2_CLIENT_GAMELIB_INTERFACE_H */
