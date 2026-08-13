#include "memprocess.h"

#include <cstddef>
#include <memoryapi.h>
#include <minwindef.h>
#include <ostream>
#include <windows.h>
#include <winnt.h>
#include <iostream>
#include <tlhelp32.h>

int yen::memprocess::get_pid(const std::wstring& processName) 
{
    int pid = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (hSnap != INVALID_HANDLE_VALUE) 
    {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(pe32);
        
        if (Process32FirstW(hSnap, &pe32)) 
        {
            do {
                if (_wcsicmp(processName.c_str(),pe32.szExeFile) == 0) 
                {
                    pid = pe32.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnap, &pe32));
        }
        CloseHandle(hSnap);
    }
    return pid;
}
yen::memprocess::Process::Process(int pid)
{
    init(pid);
}
yen::memprocess::Process::~Process()
{
    terminate();
}
bool yen::memprocess::Process::init(int pid)
{
    terminate(); // Clear old value

    hProcess = OpenProcess(
        PROCESS_VM_READ |
        PROCESS_VM_WRITE |
        PROCESS_VM_OPERATION |
        PROCESS_QUERY_INFORMATION,
        FALSE,
        pid
    );

    if (hProcess == nullptr)
    {
    DWORD error = GetLastError();
    std::cout<< "Init Error: " << error << std::endl;
    return false;
    }

    return true;
}
void yen::memprocess::Process::terminate()
{
    if (hProcess)
    {
        CloseHandle(hProcess);
        hProcess = nullptr;
    }
}
bool yen::memprocess::Process::is_alive()
{
    if (!hProcess) return false;

    DWORD exitCode = 0;
    if (!GetExitCodeProcess(hProcess, &exitCode)) return false;

    return exitCode == STILL_ACTIVE;
}
bool yen::memprocess::Process::read(const uintptr_t& address, void* buffer, const size_t& size)
{
    if (!hProcess) return false;

    SIZE_T bytes{};
    if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), buffer, size, &bytes) && size == bytes)
    {
        return true;
    }
        
    DWORD error = GetLastError();
    std::cout<< "Read Error: " << error << std::endl;

    return false;
}
bool yen::memprocess::Process::write(const uintptr_t& address, const void* buffer, const size_t& size)
{
    if (!hProcess) return false;

    SIZE_T bytes_written{};
    if (WriteProcessMemory(
        hProcess,
        reinterpret_cast<LPVOID>(address),
        buffer,
        size,
        &bytes_written
        ) && bytes_written == size) return true;
        
    DWORD error = GetLastError();
    std::cout<< "Write Error: " << error << std::endl;

    return false;
}
bool yen::memprocess::Process::write_force(const uintptr_t& address, const void* buffer, const size_t& size)
{
    unsigned long oldProtect;
    
    if (VirtualProtectEx(hProcess, reinterpret_cast<void*>(address), size, 0x40, &oldProtect)) 
    {
        size_t bytesWritten = 0;
        
        WriteProcessMemory(hProcess, reinterpret_cast<void*>(address), buffer, size, &bytesWritten);
        
        unsigned long tempProtect;
        VirtualProtectEx(hProcess, reinterpret_cast<void*>(address), size, oldProtect, &tempProtect);
        
        return bytesWritten == size;
    }
    
    return false;
}
