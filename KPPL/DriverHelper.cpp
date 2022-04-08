#include "pch.h"
#include "DriverHelper.h"

#define IOCTL_OPEN_PPL_HANDLE (2201288764)

bool DriverHelper::OpenDevice() {
	if (!_hDevice) {
		// PROCEXP152
		_hDevice = ::CreateFile(L"\\\\.\\PROCEXP152", GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
			OPEN_EXISTING, 0, nullptr);
		if (_hDevice == INVALID_HANDLE_VALUE) {
			printf("OpenDevice failed: %d\n", GetLastError());
			return false;
		}
	}
	return true;
}

HANDLE DriverHelper::OpenProcess(ULONGLONG pid) {
	if (OpenDevice()) {
		HANDLE hProcess;
		DWORD bytes;
		bool success = ::DeviceIoControl(_hDevice, IOCTL_OPEN_PPL_HANDLE, &pid, sizeof(pid),
			&hProcess, sizeof(hProcess), &bytes, nullptr);
		if (!success)
			printf("Open process failed: %d\n", GetLastError());
		::CloseHandle(_hDevice);
		_hDevice = nullptr;
		return success ? hProcess : nullptr;
	}

	return nullptr;
}