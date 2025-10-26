#ifndef METIN2_SERVER_DB_NETBASE_H
#define METIN2_SERVER_DB_NETBASE_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <thecore/singleton.hpp>

class CNetBase
{
    public:
	CNetBase();
	virtual ~CNetBase();

    protected:
	static LPFDWATCH	m_fdWatcher;
};

class CNetPoller : public CNetBase, public singleton<CNetPoller>
{
    public:
	CNetPoller();
	virtual ~CNetPoller();

	bool	Create();
	void	Destroy();
};

#endif /* METIN2_SERVER_DB_NETBASE_H */
