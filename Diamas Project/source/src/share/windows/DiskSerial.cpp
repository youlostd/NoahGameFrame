#include <windows/DiskSerial.hpp>
#include <windows/ComUtil.hpp>

#include <storm/WindowsPlatform.hpp>
#include <storm/StringUtil.hpp>

#include <vector>

#include <winioctl.h>
#include "spdlog/spdlog.h"

METIN2_BEGIN_NS

bool GetDriveSerialNumber(const char* drivePath, std::string& serialNumber)
{
	// Get a handle to physical drive
	HANDLE device = ::CreateFileA(drivePath, 0,
	                              FILE_SHARE_READ | FILE_SHARE_WRITE,
	                              NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == device)
		return false;

	// Set the input data structure
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	std::memset(&storagePropertyQuery, 0, sizeof(storagePropertyQuery));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	// Get the necessary output buffer size
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = {0};
	DWORD bytesReturned = 0;
	if (!::DeviceIoControl(device, IOCTL_STORAGE_QUERY_PROPERTY,
	                       &storagePropertyQuery, sizeof(storagePropertyQuery),
	                       &storageDescriptorHeader,
	                       sizeof(storageDescriptorHeader), &bytesReturned,
	                       nullptr)) {
		::CloseHandle(device);
		return false;
	}

	std::vector<char> buffer(storageDescriptorHeader.Size);

	// Get the storage device descriptor
	if (!::DeviceIoControl(device, IOCTL_STORAGE_QUERY_PROPERTY,
	                       &storagePropertyQuery, sizeof(storagePropertyQuery),
	                       buffer.data(), storageDescriptorHeader.Size,
	                       &bytesReturned, nullptr)) {
		::CloseHandle(device);
		return false;
	}

	::CloseHandle(device);

	// Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
	// followed by additional info like vendor ID, product ID, serial number, and so on.
	auto deviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer.data();
	const auto serialNumberOffset = deviceDescriptor->SerialNumberOffset;
	if (serialNumberOffset != 0) {
		// Finally, get the serial number
		serialNumber.assign(buffer.data() + serialNumberOffset);
		storm::TrimAndAssign(serialNumber);
		return true;
	}

	return false;
}

bool GetDriveSerialNumberWmi(const char* drivePath, std::string& serialNumber)
{
	ScopedComInitialization comInit;

	WmiQuery wmi;
	if (!wmi.Setup())
		return false;

	char query[256];
	snprintf(query, sizeof(query),
	         "SELECT SerialNumber FROM Win32_PhysicalMedia WHERE Tag=\"%s\"",
	         drivePath);

	auto enumerator = wmi.Execute(query);
	if (!enumerator)
		return false;

	IWbemClassObject* pclsObj = NULL;
	ULONG uReturn = 0;
	HRESULT hr = enumerator->Next(WBEM_INFINITE, 1,
	                              &pclsObj, &uReturn);

	if (0 == uReturn) {
		enumerator->Release();
		return false;
	}

	VARIANT vtProp;

	// Get the value of the Name property
	hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
	if (SUCCEEDED(hr) && vtProp.bstrVal) {
		serialNumber.assign(_bstr_t(vtProp.bstrVal));
		storm::TrimAndAssign(serialNumber);
	}

	VariantClear(&vtProp);
	pclsObj->Release();
	return !serialNumber.empty();
}

bool GetDriveSerial(uint32_t driveIndex, std::string& serialNumber)
{
	char drivePath[256];
	snprintf(drivePath, sizeof(drivePath), "\\\\.\\PhysicalDrive%u", driveIndex);

	if (GetDriveSerialNumber(drivePath, serialNumber)) {
		SPDLOG_DEBUG("Got disk serial {0} for {1} at attempt 1",
		           serialNumber, drivePath);
		return true;
	}

	if (GetDriveSerialNumberWmi(drivePath, serialNumber)) {
		SPDLOG_DEBUG("Got disk serial {0} for {1} at attempt 2",
		           serialNumber, drivePath);
		return true;
	}

	SPDLOG_DEBUG("Couldn't get serial for {0}", drivePath);
	return false;
}

METIN2_END_NS
