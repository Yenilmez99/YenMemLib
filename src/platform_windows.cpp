#include <string>
#include <cstddef>
#include <cstdint> 
#include <iostream>

#include <windows.h>
#include <tlhelp32.h>

#include "memprocess.h"

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
std::uintptr_t yen::memprocess::get_module_base_address(int pid, const std::string& module_name)
{
    std::uintptr_t base_address = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 module_entry;
        module_entry.dwSize = sizeof(module_entry);

        if (Module32First(snapshot, &module_entry)) {
            do {
                std::wstring w_module_name(module_name.begin(), module_name.end());
                if (module_name == module_entry.szModule) {

                    base_address = reinterpret_cast<std::uintptr_t>(module_entry.modBaseAddr);
                    break;
                }
            } while (Module32Next(snapshot, &module_entry));
        }

        // İşlem bitince bellek sızıntısını önlemek için handle'ı kapat
        CloseHandle(snapshot);
    }

    return base_address;
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
bool yen::memprocess::Process::read(const std::uintptr_t& address, void* buffer, const std::size_t& size)
{
    if (!hProcess) return false;

    SIZE_T bytes{};
    if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), buffer, size, &bytes) && size == bytes)
    {
        return true;
    }
        
    DWORD error = GetLastError();
    std::cout << "Read Error: " << error << std::endl;

    return false;
}
bool yen::memprocess::Process::write(const std::uintptr_t& address, const void* buffer, const std::size_t& size)
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
bool yen::memprocess::Process::write_force(const std::uintptr_t& address, const void* buffer, const std::size_t& size)
{
    DWORD oldProtect;
    
    if (VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect)) 
    {
        SIZE_T bytesWritten = 0;
        
        WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), buffer, size, &bytesWritten);
        
        DWORD tempProtect = 0;
        VirtualProtectEx(hProcess, reinterpret_cast<LPVOID>(address), size, oldProtect, &tempProtect);
        
        return bytesWritten == size;
    }
    
    return false;
}
