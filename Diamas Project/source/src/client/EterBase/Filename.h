///////////////////////////////////////////////////////////////////////  
//	CFilename Class
//
//	(c) 2003 IDV, Inc.
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com
//


#pragma once
#include <string>


class CFileNameHelper
{
public:
	static void ChangeDosPath(std::string& str) {
		const auto nLength = str.length();

		for (size_t i = 0; i < nLength; ++i)
		{
			if (str.at(i) == '/') {
				str.at(i) = '\\';
}
		}
	}

	static void StringPath(std::string& str) {
		const auto nLength = str.length();		

		for (size_t i = 0; i<nLength; ++i)
		{
			if (str.at(i) == '\\') {
				str.at(i) = '/';
			} else {
				str.at(i) = static_cast<char>(tolower(str.at(i)));
}
		}
	}

	static std::string GetName(std::string& str);           // if filename is "/idv/code/file.cpp", it returns "file"
	static std::string GetExtension(const std::string& str);      // if filename is "/idv/code/file.cpp", it returns "cpp"
	static std::string GetPath(const std::string& str);           // if filename is "/idv/code/file.cpp", it returns "/idv/code"
	static std::string NoExtension(const std::string& str);       // if filename is "/idv/code/file.cpp", it returns "/idv/code/file"
	static std::string NoPath(std::string& str);            // if filename is "/idv/code/file.cpp", it returns "file.cpp"
};

///////////////////////////////////////////////////////////////////////  
//	CFileNameHelper::GetExtension

inline std::string CFileNameHelper::GetName(std::string& str)
{
	std::string strName;

	const auto nLength = str.length();

	if (nLength > 0)
	{
		auto iExtensionStartPos = nLength - 1;

		for (auto i = nLength - 1; i > 0; i--)
		{
			if (str[i] == '.')
			{
				iExtensionStartPos = i;
			}

			if (str[i] == '/')
			{
				strName = std::string(str.c_str() + i + 1);
				strName.resize(iExtensionStartPos - i - 1);
				break;
			}
		}
	}

	return strName;
}


///////////////////////////////////////////////////////////////////////  
//	CFilenameHelper::GetExtension

inline std::string CFileNameHelper::GetExtension(const std::string& str)
{
	std::string strExtension;

	const auto nLength = str.length();

	if (nLength > 0)
	{
		for (auto i = nLength - 1; i > 0 && str[i] != '/'; i--) {
			if (str[i] == '.')
			{
				strExtension = std::string(str.c_str( ) + i + 1);
				break;
			}
}
	}

	return strExtension;
}


///////////////////////////////////////////////////////////////////////  
//	CFilenameHelper::GetPath

inline std::string CFileNameHelper::GetPath(const std::string& str)
{
	char szPath[1024];
	szPath[0] = '\0';

	size_t nLength = str.length();

	if (nLength > 0)
	{
		for (size_t i = nLength - 1; i > 0; i--)
		{
			if (str[i] == '/' || str[i] == '\\')
			{
				for (size_t j = 0; j < i + 1; j++)
					szPath[j] = str[j];
				szPath[i + 1] = '\0';
				break;
			}

		}
	}
	return szPath;
}


///////////////////////////////////////////////////////////////////////  
//	CFilenameHelper::NoExtension

inline std::string CFileNameHelper::NoExtension(const std::string& str)
{
	const auto npos = str.find_last_of('.');

	if (std::string::npos != npos) {
		return std::string(str, 0, npos);
}

	return str;
}


///////////////////////////////////////////////////////////////////////  
//	CFilenameHelper::NoPath

inline std::string CFileNameHelper::NoPath(std::string& str)
{
	char szPath[1024];
	szPath[0] = '\0';

	const auto nLength = str.length();

	if (nLength > 0)
	{
		strcpy_s(szPath, str.c_str());

		for (auto i = nLength - 1; i > 0; i--)
		{
			if (str[i] == '/' || str[i] == '\\')
			{
				auto k = 0;
				for (auto j = i + 1; j < nLength; j++, k++) {
					szPath[k] = str[j];
}
				szPath[k] = '\0';
				break;
			}

		}
	}

	return szPath;
}