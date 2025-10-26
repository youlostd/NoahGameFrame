#ifndef __POLY_SYMTABLE_H__
#define __POLY_SYMTABLE_H__

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <string>

class CSymTable  
{
    public:
	CSymTable(int aTok, std::string aStr);
	virtual ~CSymTable();

	double		dVal;
	int		token;
	std::string	strlex;
};

#endif 
