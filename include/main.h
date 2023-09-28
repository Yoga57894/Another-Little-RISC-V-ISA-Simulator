#ifndef __MAIN_HEADER
#define __MAIN_HEADER

#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <elf.h>
#include <unordered_map>

namespace ALISS
{
    void loadElf(const char* elf_name);
    unsigned get_mem_w(unsigned long long addr);
    static uint8_t* memory = 0;
    static uint64_t pc;
}

#endif