//         Name: BoundaryShapeManager.h
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2002-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not
//  be copied or disclosed except in accordance with the terms of that
//  agreement.
//
//      Release version 1.0
#ifndef METIN2_CLIENT_SPEEDTREELIB_BOUNDARYSHAPEMANAGER_HPP
#define METIN2_CLIENT_SPEEDTREELIB_BOUNDARYSHAPEMANAGER_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <vstl/string.hpp>

#include <vector>

METIN2_BEGIN_NS

///////////////////////////////////////////////////////////////////////
//	SPoint
struct SPoint
{
		float&      operator[](int i) { return m_afData[i]; }
		float       m_afData[3];
};


///////////////////////////////////////////////////////////////////////
//	SBoundaryShape
struct SBoundaryShape
{
		std::vector< std::vector<SPoint> >	m_vContours;
		float								m_afMin[3];
		float								m_afMax[3];
};


///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager
class CBoundaryShapeManager
{
public:
		CBoundaryShapeManager( );
		virtual ~CBoundaryShapeManager( );

		bool						LoadBsfFile(const char* pFilename);

		bool						PointInside(float fX, float fY);
		bool						RandomPoint(float& fX, float& fY);

		vstd::string					GetCurrentError( )	{ return m_strCurrentError; }

private:
		bool						PointInShape(SBoundaryShape& sShape, float fX, float fY);

		std::vector<SBoundaryShape>	m_vBoundaries;
		vstd::string					m_strCurrentError;
};

METIN2_END_NS

#endif