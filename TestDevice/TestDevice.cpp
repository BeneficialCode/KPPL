// TestDevice.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "SecurityHelper.h"
#include "DriverHelper.h"

int main(int argc,const char* argv[]) {
    SecurityHelper::EnablePrivilege(L"SeDebugPrivilege", true);
    HANDLE hFile = ::CreateFile(L"\\\\.\\PROCEXP152", GENERIC_READ | GENERIC_WRITE, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, 0, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Last Error: %d\n", GetLastError());
        system("pause");
        return 1;
    }

    ::CloseHandle(hFile);
    if(argc<1){
        printf("Usage: TestDevice.exe pid\n");
        system("pause");
        return 0;
    }
    DWORD pid = atoi(argv[1]);
    HANDLE hProcess = DriverHelper::OpenProcess(pid);
    if (hProcess != nullptr) {
        printf("hProcess: %d\n", hProcess);
        ::CloseHandle(hProcess);
    }
    system("pause");
    return 0;
}

