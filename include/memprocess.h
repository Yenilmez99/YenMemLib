#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace yen {

    namespace memprocess {

        int get_pid(const std::wstring& processName);
        std::uintptr_t get_module_base_address(int pid, const std::string& module_name);

        class Process
        {
        private:
        public:
            void* hProcess = nullptr; // for windows
            int target_pid = -1; // for linux

            Process(int pid);
            ~Process();

            bool init(int pid);
            void terminate();
            bool is_alive();
            bool read(const std::uintptr_t& address, void* buffer, const std::size_t& size);
            bool write(const std::uintptr_t& address,const void* buffer, const std::size_t& size);
            bool write_force(const std::uintptr_t& address,const void* buffer, const std::size_t& size);
            std::uintptr_t get_pointer_address(const std::uintptr_t& address, const std::vector<ptrdiff_t>& offsets) {
                if (offsets.empty()) return address;
                
                std::uintptr_t temp_address = address;
                
                for (std::size_t i = 0; i < offsets.size(); ++i)
                {
                    temp_address += offsets[i];

                    if (i < offsets.size() - 1) {

                        if (!read(temp_address, &temp_address, sizeof(temp_address))) {
                            return 0;
                        }

                    }
                    
                }

                return temp_address;
            }

            template<typename T>
            bool read(const std::uintptr_t& address, T& value) {
                return read(address, &value, sizeof(T));
            }
            template<typename T>
            bool write(const std::uintptr_t& address,const T& value) {
                return write(address, &value, sizeof(T));
            }
            template<typename T>
            bool write_force(const std::uintptr_t& address,const T& value) {
                return write_force(address, &value, sizeof(T));
            }

        };

    }

}
