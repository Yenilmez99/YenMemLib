#include "memprocess.h"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <dirent.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <sys/uio.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

int yen::memprocess::get_pid(const std::wstring& processName) 
{
    std::string targetName(processName.begin(), processName.end());
    int target_pid = 0;
    
    DIR* dp = opendir("/proc");
    if (dp != nullptr) 
    {
        struct dirent* dirp;
        while ((dirp = readdir(dp))) 
        {
            std::string dirName(dirp->d_name);
            
            if (std::all_of(dirName.begin(), dirName.end(), ::isdigit))
            {
                std::ifstream commFile("/proc/" + dirName + "/comm");
                if (commFile)
                {
                    std::string commName;
                    std::getline(commFile, commName);

                    if (commName == targetName)
                    {
                        target_pid = std::stoi(dirName);
                        break;
                    }

                }

            }

        }
        closedir(dp);
    }
    
    return target_pid;
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
    terminate();

    if (kill(pid, 0) == 0)
    {
        target_pid = pid;
        return true;
    }

    std::cerr << "Init Error: PID: " << pid << "\nPID couldnt be found or permission error" << std::endl;
    return false;
}
void yen::memprocess::Process::terminate()
{
    target_pid = -1;
}
bool yen::memprocess::Process::is_alive()
{
    // std::cout << "Target PID: " << target_pid << std::endl;
    if (target_pid == -1 || target_pid == 0) return false;
    return kill(target_pid,0) == 0;
}
bool yen::memprocess::Process::read(const std::uintptr_t& address, void* buffer, const std::size_t& size)
{
    if (!is_alive()) return false;

    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_base = buffer;
    local[0].iov_len = size;
    remote[0].iov_base = reinterpret_cast<void*>(address);
    remote[0].iov_len = size;

    ssize_t bytes_read = process_vm_readv(target_pid, local, 1, remote, 1, 0);

    if (bytes_read == static_cast<ssize_t>(size)) return true;

    std::cout << "Read Error: " << strerror(errno) << " Bytes read: " << bytes_read << std::endl;
    return false;
}
bool yen::memprocess::Process::write(const std::uintptr_t& address, const void* buffer, const std::size_t& size)
{
    if (!is_alive()) return false;

    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_base = const_cast<void*>(buffer);
    local[0].iov_len = size;
    remote[0].iov_base = reinterpret_cast<void*>(address);
    remote[0].iov_len = size;

    ssize_t bytes_written = process_vm_writev(target_pid, local, 1, remote, 1, 0);

    if (bytes_written == static_cast<ssize_t>(size)) return true;

    std::cout << "Write Error" << std::endl;
    return false;
}
bool yen::memprocess::Process::write_force(const std::uintptr_t& address, const void* buffer, const std::size_t& size)
{
    std::string memPath = "/proc/" + std::to_string(target_pid) + "/mem";
    
    int fd = open(memPath.c_str(), O_RDWR);
    if (fd == -1) {
        std::cerr << "Mem File couldnt be opened" << std::endl;
        return false;
    }

    ssize_t written = pwrite(fd, buffer, size, address);
    
    close(fd);
    
    return (written == static_cast<ssize_t>(size));

}
