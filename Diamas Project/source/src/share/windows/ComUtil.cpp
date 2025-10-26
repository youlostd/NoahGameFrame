#include <windows/ComUtil.hpp>
#include "spdlog/spdlog.h"

METIN2_BEGIN_NS

ScopedComInitialization::ScopedComInitialization()
	: m_initialized(S_OK == ::CoInitializeEx(0, COINIT_MULTITHREADED))
{
	// ctor
}

ScopedComInitialization::~ScopedComInitialization()
{
	if (m_initialized)
		CoUninitialize();
}

WmiQuery::WmiQuery()
	: m_loc(nullptr)
	, m_svc(nullptr)
{
	// ctor
}

WmiQuery::~WmiQuery()
{
	if (m_svc)
		m_svc->Release();

	if (m_loc)
		m_loc->Release();
}

bool WmiQuery::Setup()
{
	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer m_svc
	// to make IWbemServices calls.
	HRESULT hres = m_loc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL, // User name. NULL = current user
		NULL, // User password. NULL = current
		0, // Locale. NULL indicates current
		NULL, // Security flags.
		0, // Authority (for example, Kerberos)
		0, // Context object
		&m_svc // pointer to IWbemServices proxy
	);

	if (FAILED(hres)) {
		spdlog::error("Could not connect. Error code {0}",
		           hres);
		return false;
	}

	spdlog::error("Connected to ROOT\\CIMV2 WMI namespace");

	// Set security levels on the proxy
	hres = CoSetProxyBlanket(
		m_svc, // Indicates the proxy to set
		RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE, // RPC_C_AUTHZ_xxx
		NULL, // Server principal name
		RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL, // client identity
		EOAC_NONE // proxy capabilities
	);

	if (FAILED(hres)) {
		spdlog::error("Could not set proxy blanket. Error code {0}",
		           hres);
		CoUninitialize();
		return false;
	}

	return true;
}

IEnumWbemClassObject* WmiQuery::Execute(const char* query)
{
	// Use the IWbemServices pointer to make requests of WMI
	// For example, get the name of the operating system
	IEnumWbemClassObject* enumerator = NULL;
	HRESULT hres = m_svc->ExecQuery(
		bstr_t("WQL"),
		bstr_t(query),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&enumerator
	);

	if (FAILED(hres)) {
		spdlog::error("Query {1} failed. Error code {0}",
		           hres, query);
		return nullptr;
	}

	return enumerator;
}

METIN2_END_NS
