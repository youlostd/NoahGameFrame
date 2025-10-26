#include "StdAfx.h"
#include "MapBase.h"

CMapBase::CMapBase()
{
    Clear();
}

CMapBase::~CMapBase()
{
    Clear();
}

void CMapBase::Clear()
{
    m_strName = "";
    m_eType = MAPTYPE_INVALID;
    Leave();
}

bool CMapBase::Enter()
{
    m_bReady = true;
    return true;
}

bool CMapBase::Leave()
{
    m_bReady = false;
    return true;
}

void CMapBase::Render()
{
    if (IsReady())
        OnRender();
}

bool CMapBase::LoadProperty()
{
    std::string strFileName = GetName() + "\\MapProperty.txt";

    CTokenVectorMap stTokenVectorMap;

    if (!LoadMultipleTextData(strFileName.c_str(), stTokenVectorMap))
    {
        SPDLOG_ERROR(
            "CMapBase::LoadProperty(FileName={0}) - LoadMultipleTextData ERROR It is very likely that there are no files.",
            strFileName.c_str());
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("scripttype"))
    {
        SPDLOG_ERROR("CMapBase::LoadProperty(FileName={0}) - FIND 'scripttype' - FAILED", strFileName.c_str());
        return false;
    }

    if (stTokenVectorMap.end() == stTokenVectorMap.find("maptype"))
    {
        SPDLOG_ERROR("CMapBase::LoadProperty(FileName={0}) - FIND 'maptype' - FAILED", strFileName.c_str());
        return false;
    }

    // NOTE: �̹� �����ϴ� �� �����Ϳ� ������ �����͸� ����ϴ� ���� ���� �߰��� ��, �� ���� �뷮�� ���̱� ���� �۾�.
    // MapProperty.txt ���Ͽ� ParentMapName ���� �����Ǿ� �ִٸ�, ���� ��� �����ʹ� ParentMap���� �о�´�.
    // �������� �κа���(�κ� ��������Ʈ?) ����� �ʿ� ���뼭, Parent Map���� ��� �����͸� �о��.
    if (stTokenVectorMap.end() != stTokenVectorMap.find("parentmapname"))
    {
        m_strParentMapName = stTokenVectorMap["parentmapname"][0];
    }

    const std::string &c_rstrType = stTokenVectorMap["scripttype"][0];
    const std::string &c_rstrMapType = stTokenVectorMap["maptype"][0];

    if (0 != c_rstrType.compare("MapProperty"))
    {
        SPDLOG_ERROR("CMapBase::LoadProperty(FileName={0}) - Resourse Type ERROR", strFileName.c_str());
        return false;
    }

    if (0 == c_rstrMapType.compare("Indoor"))
        SetType(MAPTYPE_INDOOR);
    else if (0 == c_rstrMapType.compare("Outdoor"))
        SetType(MAPTYPE_OUTDOOR);
    else if (0 == c_rstrMapType.compare("Invalid"))
        SetType(MAPTYPE_OUTDOOR);

    return true;
}
