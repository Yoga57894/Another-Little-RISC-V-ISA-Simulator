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
    static uint8_t* memory = 0;
    static uint64_t pc;
    static uint64_t next_pc;
    static uint64_t reg[32];

    bool loadElf(const char* filename) //XXX should be refactor, not need to implement twice
    {
        // ELF loader function
        std::ifstream file(filename, std::ios::binary);

        if (!file) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return 1;
        }

        // Read the ELF header
        Elf64_Ehdr elfHeader;
        file.read(reinterpret_cast<char*>(&elfHeader), sizeof(Elf64_Ehdr));

        // Check ELF magic number
        if (std::memcmp(elfHeader.e_ident, ELFMAG, SELFMAG) != 0) {
            std::cerr << "Not a valid ELF file: " << filename << std::endl;
            return 1;
        }

        // Check ELF class (32-bit or 64-bit)
        if (elfHeader.e_ident[EI_CLASS] != ELFCLASS64) {
            std::cerr << "Only 64-bit ELF files are supported: " << filename << std::endl;
            return 1;
        }

        // Check ELF data encoding (little-endian or big-endian)
        if (elfHeader.e_ident[EI_DATA] != ELFDATA2LSB) {
            std::cerr << "Only little-endian ELF files are supported: " << filename << std::endl;
            return 1 ;
        }

        // Get the entry point address
        Elf64_Addr entryPoint = elfHeader.e_entry;
        ALISS::pc = entryPoint;

        // Add more code here to load and work with program segments, sections, etc.
        for (int i = 0; i < elfHeader.e_phnum; i++) {
            file.seekg(elfHeader.e_phoff + i * sizeof(Elf64_Phdr));

            Elf64_Phdr programHeader;
            file.read(reinterpret_cast<char*>(&programHeader), sizeof(Elf64_Phdr));

            // Check if this is a loadable segment
            if (programHeader.p_type == PT_LOAD) {
                // Save flags, address, and size
                Elf64_Word flags = programHeader.p_flags;
                Elf64_Addr addr = programHeader.p_vaddr;
                Elf64_Xword size = programHeader.p_memsz;

                // Seek to the segment's file offset
                file.seekg(programHeader.p_offset);

                // Read the segment data to memory
                file.read(reinterpret_cast<char*>(ALISS::memory + addr), size);
            }
        }


        file.close();
    	return 0;
    };

    unsigned get_mem_w(unsigned long long  int addr)
    {
        return *(uint32_t*)(ALISS::memory + addr);
    };

    uint32_t IF()
    {
        return get_mem_w(pc);
    };

    void ID_EX_WB(uint32_t insn)
    {
        ///implement insn here 
        ALISS::next_pc = ALISS::pc + 4;
    };

    void run_pipe()
    {
        uint32_t insn = ALISS::IF();
        ALISS::ID_EX_WB(insn);

    #ifdef INSN_DEBUG
        dump_insn(insn);
    #endif

        pc = next_pc;
    };
}

#endif