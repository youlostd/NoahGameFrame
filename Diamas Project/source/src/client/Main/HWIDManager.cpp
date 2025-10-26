#include "HWIDManager.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <sha256.h>
#include <boost/asio.hpp>
#include <windows/NetworkAdapters.hpp>

namespace HWIDMANAGER
{
    static std::string s_MachineGUID;
    static std::string s_MacAddr;
    static std::string s_CpuId;


    auto GetCPUID() -> std::string
    {
        int registers[4];
        char cpuIDHex[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        char *part1 = &cpuIDHex[0];
        char *part2 = &cpuIDHex[8];

        __cpuid(registers, 1);

        sprintf_s(part1, 9, "%08X", registers[3]); //reverse byte order (endiannes) MartPwnS
        sprintf_s(part2, 9, "%08X", registers[0]);

        return cpuIDHex;
    }

    auto GetMachineGuid() -> std::string
    {
        HKEY hKey;
        LSTATUS res = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY,
                                    &hKey);

        if (res == ERROR_SUCCESS)
        {
            std::wstring temp;
            WCHAR szBuffer[512];
            DWORD dwBufferSize = sizeof(szBuffer);
            ULONG res2 = RegQueryValueExW(hKey, L"MachineGuid", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
            if (ERROR_SUCCESS == res2)
                temp = szBuffer;

            if (res2 == ERROR_SUCCESS)
            {
                return std::string(temp.begin(), temp.end());
            }
        }
        return "";
    }
    
    const std::string& getMachineGUID()
    {
        return s_MachineGUID;
    };

    const std::string& getMacAddr()
    {
        return s_MacAddr;
    };

    const std::string& getCPUid()
    {
        return s_CpuId;
    };

    void InitHardwareId()
    {
        std::vector<uint8_t> adapters, mac;
        vstd::string serial;
        if (GetAdapters(adapters) && GetFirstAdapterMac(adapters, serial, mac))
        {
            unsigned char digest[SHA256::DIGEST_SIZE];
            memset(digest, 0, SHA256::DIGEST_SIZE);

            SHA256 ctx = SHA256();
            ctx.init();
            ctx.update((unsigned char *)mac.data(), mac.size());
            ctx.final(digest);

            
		    char macHash[HWID_MAX_LEN + 1];
		    storm::ConvertBinaryToHex(digest, sizeof(digest), macHash);

            s_MacAddr = macHash;    
        }
        

        s_MachineGUID = GetMachineGuid();
        s_CpuId = GetCPUID();
    }
}