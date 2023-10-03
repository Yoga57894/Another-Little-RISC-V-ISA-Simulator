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

    int64_t sext(uint64_t insn, int len) { return int64_t(insn) << (64 - len) >> (64 - len); }


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
        uint8_t decode = insn & 0x7f; //last 7 bit

        switch(decode)
        {
            case 0x33:
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t rs2 = ((insn  >> 20) & 0x1f);
                switch ((insn >> 12) & 7)
                {
                    case 0x0: //ADD or SUB
                    {
                        switch ((insn >> 25) & 0x7f)
                        {
                           case 0x0 :
                           {
                                reg[rd] =  reg[rs1] + reg[rs2];
                                break;
                           }
                           case 0x20 :
                           {
                                reg[rd] = reg[rs1] - reg[rs2];
                                break;
                           }
                           default:
                           {

                                printf("Illegal instruction");
                                printf("%x\n",insn);
                                break;
                           }
                        }
                        break;
                    }
                    case 0x1: //SLL
                    {
                        reg[rd] = reg[rs1] << reg[rs2];
                        break;
                    }
                    case 0x2: //SLT
                    {
                        reg[rd] = ((int64_t)reg[rs1] < (int64_t)reg[rs2]);
                        break;
                    }
                    case 0x3: //SLTU
                    {
                        reg[rd] = (reg[rs1] < reg[rs2]);
                        break;
                    }
                    case 0x4: //XOR
                    {
                        reg[rd] = reg[rs1] ^ reg[rs2];
                        break;
                    }
                    case 0x5: //SRL or SRA
                    {
                        switch ((insn >> 25) & 0x7f)
                        {
                           case 0x0 : //SRL
                           {
                                reg[rd] =  reg[rs1] >> reg[rs2];
                                break;
                           }
                           case 0x20 : //SRA
                           {
                                reg[rd] = (int64_t)reg[rs1] >> reg[rs2];
                                break;
                           }
                           default:
                           {
                                printf("Illegal instruction");
                                printf("%x\n",insn);
                                break;
                           }
                        }
                        break;
                    }
                    case 0x6: //OR
                    {
                        reg[rd] = reg[rs1] | reg[rs2];
                        break;
                    }
                    case 0x7: //AND
                    {
                        reg[rd] = reg[rs1] & reg[rs2];
                        break;
                    }
                    default:
                    {
                        printf("Illegal instruction");
                        printf("%x\n",insn);
                        break;
                    }
                }
                break;
            }
            case 0x13:
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t imm = ((insn >> 20) & 0xfff);
                switch ((insn >> 12) & 7)
                {
                    case 0x0: //ADDI
                    {
                        reg[rd] =  reg[rs1] +  sext(imm, 12);
                        break;
                    }
                    case 0x1: //SLLI
                    {
                        uint64_t shamt = imm & 0x1f; //[24:20]
                        reg[rd] = reg[rs1] << shamt;
                        break;
                    }
                    case 0x2: //SLTI
                    {
                        reg[rd] = ((int64_t)reg[rs1] < sext(imm, 12));
                        break;
                    }
                    case 0x3: //SLTUI
                    {
                        reg[rd] = (reg[rs1] < imm);
                        break;
                    }
                    case 0x4: //XORI
                    {
                        reg[rd] = (reg[rs1] ^ sext(imm, 12));
                        break;
                    }
                    case 0x5: //SRLI & SRAI
                    {
                        uint64_t shamt = imm & 0x1f; //[24:20]
                        switch ((insn >> 25) & 0x7f)
                        {
                            case 0x0 : //SRLI
                            {
                                reg[rd] =  reg[rs1] >> shamt;
                                break;
                            }
                            case 0x20 : //SRAI
                            {
                                reg[rd] =  (int64_t)reg[rs1] >> shamt;
                                break;
                            }
                           default:
                           {
                                printf("Illegal instruction");
                                printf("%x\n",insn);
                                break;
                           }
                        }
                        break;
                    }
                    case 0x6: //ORI
                    {
                        reg[rd] = (reg[rs1] | sext(imm, 12));
                        break;
                    }
                    case 0x7: //ANDI
                    {
                        reg[rd] = (reg[rs1] & sext(imm, 12));
                        break;
                    }
                    default:
                    {
                        printf("Illegal instruction");
                        printf("%x\n",insn);
                        break;
                    }
                }
                break;
            }
            default:
            {
                printf("Illegal instruction");
                printf("%x\n",insn);
                break;
            }
        }
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