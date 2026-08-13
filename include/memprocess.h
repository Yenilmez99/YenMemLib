#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>

using namespace std;

namespace yen {

    namespace memprocess {

        int get_pid(const wstring& processName);

        class Process
        {
        private:
            void* hProcess = nullptr; // for windows
            int target_pid = -1; // for linux
        public:
            Process(int pid);
            ~Process();

            bool init(int pid);
            void terminate();
            bool is_alive();
            bool read(const uintptr_t& address, void* buffer, const size_t& size);
            bool write(const uintptr_t& address,const void* buffer, const size_t& size);
            bool write_force(const uintptr_t& address,const void* buffer, const size_t& size);
            uintptr_t get_pointer_address(const uintptr_t& address, const vector<ptrdiff_t>& offsets) {
                if (offsets.empty()) return address;
                
                uintptr_t temp_address = address;
                
                for (size_t i = 0; i < offsets.size(); ++i)
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
            bool read(const uintptr_t& address, T& value) {
                return read(address, &value, sizeof(T));
            }
            template<typename T>
            bool write(const uintptr_t& address,const T& value) {
                return write(address, &value, sizeof(T));
            }
            template<typename T>
            bool write_force(const uintptr_t& address,const T& value) {
                return write_force(address, &value, sizeof(T));
            }

        };

    }

}