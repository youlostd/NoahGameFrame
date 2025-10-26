#include <windows/NetworkAdapters.hpp>

#include <storm/WindowsPlatform.hpp>

#include <winsock2.h>
#include <iphlpapi.h>
#include "spdlog/spdlog.h"

METIN2_BEGIN_NS

bool GetAdapters(std::vector<uint8_t>& adapters)
{
	for (uint32_t i = 0; i != 3; ++i) {
		ULONG size = adapters.size();
		IP_ADAPTER_ADDRESSES* addrs =
			reinterpret_cast<IP_ADAPTER_ADDRESSES*>(adapters.data());

		const auto ret = GetAdaptersAddresses(AF_UNSPEC, 0, NULL,
		                                      addrs,
		                                      &size);

		if (ret == ERROR_SUCCESS)
			return true;

		if (ret == ERROR_BUFFER_OVERFLOW)
			adapters.resize(size + sizeof(IP_ADAPTER_INFO));
		else
			return false;
	}

	SPDLOG_DEBUG("Couldn't capture adapter list");
	return false;
}

bool GetFirstAdapterMac(const std::vector<uint8_t>& adapters,
                        vstd::string& name,
                        std::vector<uint8_t>& mac)
{
	const IP_ADAPTER_ADDRESSES* addrs =
		reinterpret_cast<const IP_ADAPTER_ADDRESSES*>(adapters.data());

	const IP_ADAPTER_ADDRESSES* cur = nullptr;
	for (auto addr = addrs; addr; addr = addr->Next) {
		if (0 == addr->PhysicalAddressLength)
			continue;

		if (!cur ||
		    -1 == std::strcmp(addr->AdapterName, cur->AdapterName)) {
			cur = addr;
		}
	}

	if (cur) {
		SPDLOG_DEBUG("Got {0} bytes address for {1}",
		           cur->PhysicalAddressLength, cur->AdapterName);

		name = cur->AdapterName;
		mac.insert(mac.end(), cur->PhysicalAddress,
		           cur->PhysicalAddress + cur->PhysicalAddressLength);
		return true;
	}

	SPDLOG_DEBUG("Couldn't find a viable adapter");
	return false;
}

METIN2_END_NS
