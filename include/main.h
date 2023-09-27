#ifndef __MAIN_HEADER
#define __MAIN_HEADER

#include <iostream>
#include <fstream>
#include <vector>
#include <elf.h>
#include <unordered_map>

namespace ALISS
{
    void loadElf(const char* elf_name);
    static std::unordered_map<uint64_t,uint64_t> memory;
    static uint64_t pc;
}

#endif