#pragma once

struct DriverHelper final {
	static HANDLE OpenProcess(ULONGLONG pid);

private:
	static bool OpenDevice();

	static inline HANDLE _hDevice;
};