#include "pch.h"
#include "KernelHelper.h"
#include <Psapi.h>

#define IOCTL_READ_MEMORY	(0x80002048)
#define IOCTL_WRITE_MEMORY	(0x8000204c)

KernelHelper::KernelHelper() {
	GetKernelBaseAddr();
	_hNtos = LoadLibrary(L"ntoskrnl.exe");
}

bool KernelHelper::GetKernelBaseAddr() {
	DWORD size;

	if (EnumDeviceDrivers(nullptr, 0, &size)) {
		LPVOID* drivers = (LPVOID*)malloc(size);
		if (drivers) {
			if (EnumDeviceDrivers(drivers, size, &size)) {
				_kernelBase = reinterpret_cast<ULONG_PTR>(drivers[0]);
			}
		}
		if (drivers != nullptr) {
			free(drivers);
		}
	}
	return _kernelBase ? true : false;
}

ULONG_PTR KernelHelper::GetSymbolOffset(PCSTR name) {
	return reinterpret_cast<ULONG_PTR>(GetProcAddress(_hNtos, name))-(ULONG_PTR)_hNtos;
}

bool KernelHelper::OpenDevice() {
	if (!_hDevice) {
		// RTCore64
		_hDevice = ::CreateFile(L"\\\\.\\RTCore64", GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
			OPEN_EXISTING, 0, nullptr);
		if (_hDevice == INVALID_HANDLE_VALUE) {
			_hDevice = nullptr;
			printf("OpenDevice failed: %d\n", GetLastError());
			return false;
		}
	}
	return true;
}

DWORD KernelHelper::ReadMemory(ULONG_PTR address, DWORD size) {
	if (OpenDevice()) {
		RTCORE64_DATA data{ 0 };
		data.Address = address;
		data.Size = size;

		DWORD bytes;
		bool success = ::DeviceIoControl(_hDevice,
			IOCTL_READ_MEMORY, 
			&data,sizeof(data),
			&data, sizeof(data), &bytes, nullptr);
		if (!success)
			printf("ReadMemory failed! %d\n", GetLastError());
		return data.Value;
	}
	return 0;
}

void KernelHelper::WriteMemory(ULONG_PTR address, DWORD size, DWORD value) {
	if (OpenDevice()) {
		RTCORE64_DATA data{ 0 };
		data.Address = address;
		data.Size = size;
		data.Value = value;

		DWORD bytes;
		//printf("Write Address: %p value: %d\n", data.Address, value);
		bool success = ::DeviceIoControl(_hDevice, 
			IOCTL_WRITE_MEMORY,
			&data, sizeof(data),
			&data, sizeof(data),
			&bytes,
			nullptr);
		if (!success)
			printf("Write memory failed %d\n",GetLastError());
	}
}

WORD KernelHelper::ReadMemoryWORD(ULONG_PTR address) {
	return ReadMemory(address, 2);
}

DWORD KernelHelper::ReadMemoryDWORD(ULONG_PTR address) {
	return ReadMemory(address, 4);
}

DWORD64 KernelHelper::ReadMemoryDWORD64(ULONG_PTR address) {
	DWORD lowPart = ReadMemoryDWORD(address);
	DWORD64 highPart = ReadMemoryDWORD(address + 4);
	return (highPart << 32 | lowPart);
}

void KernelHelper::WriteMemoryDWORD64(ULONG_PTR address, DWORD64 value) {
	WriteMemory(address, 4, value & 0xffffffff);
	WriteMemory(address + 4, 4, value >> 32);	
}

void KernelHelper::WriteMemoryDWORD(ULONG_PTR address, DWORD value) {
	WriteMemory(address, 4, value);
}

void KernelHelper::WriteMemoryWORD(ULONG_PTR address, WORD value) {
	WriteMemory(address, 2, value);
}