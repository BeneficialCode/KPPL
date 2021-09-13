#pragma once

struct RTCORE64_DATA {
	BYTE Pad0[8];
	DWORD64 Address;
	BYTE Pad1[8];
	DWORD Size;
	DWORD Value;
	BYTE Pad3[16];
};
static_assert(sizeof(RTCORE64_DATA) == 48, "sizeof control data must be 48 bytes");


struct KernelHelper final {
	KernelHelper();

	static bool GetKernelBaseAddr();

	static ULONG_PTR GetSymbolOffset(PCSTR name);

	
	WORD ReadMemoryWORD(ULONG_PTR address);
	DWORD ReadMemoryDWORD(ULONG_PTR address);
	DWORD64 ReadMemoryDWORD64(ULONG_PTR address);
	void WriteMemoryDWORD64(ULONG_PTR address, DWORD64 value);
	void WriteMemoryDWORD(ULONG_PTR address, DWORD value);
	void WriteMemoryWORD(ULONG_PTR address, WORD value);

private:
	static DWORD ReadMemory(ULONG_PTR address, DWORD size);
	static void WriteMemory(ULONG_PTR address, DWORD size, DWORD value);
	static bool OpenDevice();

private:
	
	static inline HMODULE _hNtos = NULL;
	static inline HANDLE _hDevice;
public:
	static inline ULONG_PTR _kernelBase = 0;
};