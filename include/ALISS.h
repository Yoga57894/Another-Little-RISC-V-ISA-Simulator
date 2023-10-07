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
    static std::unordered_map<uint32_t,uint64_t> csr;

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

    uint64_t get_mem_d(uint64_t addr)
    {
        return *(uint64_t*)(ALISS::memory + addr);
    };

   uint32_t get_mem_w(uint64_t addr)
    {
        return *(uint32_t*)(ALISS::memory + addr);
    };
    
    uint16_t get_mem_h(uint64_t addr)
    {
        return *(uint16_t*)(ALISS::memory + addr);
    };

    uint8_t get_mem_b(uint64_t addr)
    {
        return *(uint8_t*)(ALISS::memory + addr);
    };

    void set_mem_d(uint64_t addr, uint64_t data)
    {
        uint64_t* memory64  = (uint64_t*)ALISS::memory;
        memory64[addr /  8] = data;
        return;
    };

    void set_mem_w(uint64_t addr, uint32_t data)
    {
        uint32_t* memory32  = (uint32_t*)ALISS::memory;
        memory32[addr / 4] = data;
        return;
    };

    void set_mem_h(uint64_t addr, uint16_t data)
    {
        uint16_t* memory16  = (uint16_t*)ALISS::memory;
        memory16[addr / 2] = data;
        return;
    };

    void set_mem_b(uint64_t addr, uint8_t data)
    {
        memory[addr] = data;
        return;
    };

    uint32_t IF()
    {
        return get_mem_w(pc);
    };

    void ID_EX_WB(uint32_t insn)
    {

        ALISS::next_pc = ALISS::pc + 4;
        uint8_t decode = insn & 0x7f; //last 7 bit

        switch(decode)
        {
            case 0x3:  //LOAD
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t imm = ((insn >> 20) & 0xfff);
                switch ((insn >> 12) & 7)
                {
                    case 0: //LB
                    {
                        reg[rd] = sext(get_mem_b(reg[rs1] + sext(imm,12)),8);
                        break;
                    }
                    case 1:  //LH
                    {
                        reg[rd] = sext(get_mem_h(reg[rs1] + sext(imm,12)),16);
                        break;
                    }
                    case 2: //LW
                    {
                        reg[rd] = sext(get_mem_w(reg[rs1] + sext(imm,12)),32);
                        break;
                    }
                    case 3: //LD
                    {
                        reg[rd] = get_mem_d(reg[rs1] + sext(imm,12));
                        break;
                    }
                    case 4: //LBU
                    {
                        reg[rd] = get_mem_b(reg[rs1] + sext(imm,12));
                        break;
                    }
                    case 5:  //LHU
                    {
                        reg[rd] = get_mem_h(reg[rs1] + sext(imm,12));
                        break;
                    }
                    case 6:  //LWU
                    {
                        reg[rd] = get_mem_w(reg[rs1] + sext(imm,12));
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
            case 0x13: //I-type ALU
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
            case 0x1b: //OP-IMM-32
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t imm = ((insn >> 20) & 0xfff);
                switch ((insn >> 12) & 7)
                {
                    case 0x0: //ADDIW
                    {
                        reg[rd] =  sext(((reg[rs1] +  sext(imm, 12)) & 0xffffffff),32);
                        break;
                    }
                    case 0x1: //SLLIW
                    {
                        uint64_t shamt = imm & 0x1f; //[24:20]
                        reg[rd] = sext((reg[rs1] << shamt)  & 0xffffffff,32);
                        break;
                    }
                    case 0x5: //SRLIW & SRAIW
                    {
                        uint64_t shamt = imm & 0x1f; //[24:20]
                        switch ((insn >> 25) & 0x7f)
                        {
                            case 0x0 : //SRLIW
                            {
                                reg[rd] =  sext((reg[rs1] >> shamt) & 0xffffffff,32);
                                break;
                            }
                            case 0x20 : //SRAIW
                            {
                                reg[rd] =  sext(((int64_t)reg[rs1] >> shamt) & 0xffffffff,32);
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
                break;
            }
            case 0x23: //STORE
            {
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t rs2 = ((insn  >> 20) & 0x1f);

                uint64_t imm = ( ( insn & 0xfe000000 ) >> 20 ) | // [11:5]
                               ( ( insn >> 7 ) & 0x1f ); //[4:0]
                switch ((insn >> 12) & 7)
                {
                    case 0: //SB
                    {
                        set_mem_b(reg[rs1] + sext(imm,12), (uint8_t)reg[rs2]);
                        break;
                    }
                    case 1:  //SH
                    {
                        set_mem_h(reg[rs1] + sext(imm,12), (uint16_t)reg[rs2]);
                        break;
                    }
                    case 2: //SW
                    {
                        set_mem_w(reg[rs1] + sext(imm,12), (uint32_t)reg[rs2]);
                        break;
                    }
                    case 3: //SD
                    {
                        set_mem_d(reg[rs1] + sext(imm,12), (uint64_t)reg[rs2]);
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
            case 0x33: //OP
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
            case 0x3b: //OP-32bit
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t rs2 = ((insn  >> 20) & 0x1f);
                switch ((insn >> 12) & 7)
                {
                    case 0x0: //ADDW or SUBW
                    {
                        switch ((insn >> 25) & 0x7f)
                        {
                           case 0x0 : //ADDW
                           {
                                reg[rd] =  sext((reg[rs1] + reg[rs2]) & 0xffffffff,32);
                                break;
                           }
                           case 0x20 :  //SUBW
                           {
                                reg[rd] = sext((reg[rs1] - reg[rs2]) &  0xffffffff,32);
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
                    case 0x1: //SLLW
                    {
                        reg[rd] = sext((reg[rs1] << reg[rs2])  & 0xffffffff,32);
                        break;
                    }
                    case 0x5: //SRLW or SRAW
                    {
                        switch ((insn >> 25) & 0x7f)
                        {
                           case 0x0 : //SRLW
                           {
                                reg[rd] =  sext((reg[rs1] >> reg[rs2]) &  0xffffffff,32);
                                break;
                           }
                           case 0x20 : //SRAW
                           {
                                reg[rd] = sext(((int64_t)reg[rs1] >> reg[rs2]) & 0xffffffff,32);
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
                break;
            }
            case 0x63:  //Branch
            {
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t rs2 = ((insn  >> 20) & 0x1f);
                uint64_t imm = (((insn & 0xf00) >> 7) | //[4:1]
                               ((insn & 0x7e000000) >> 20) |  //[10:5]
                               ((insn & 0x80) << 4) | //[11]
                               ((insn & 0x80000000) >> 19));

                switch ((insn >> 12) & 7)
                {
                    case 0: //BEQ
                    {
                        if(reg[rs1] == reg[rs2])
                            next_pc = pc + sext(imm,13);
                        break;
                    }
                    case 1: //BNE
                    {
                        if(reg[rs1] != reg[rs2])
                            next_pc = pc + sext(imm,13);
                        break;
                    }
                    case 4: //BLT
                    {
                        if((int64_t)reg[rs1] < (int64_t)reg[rs2])
                            next_pc = pc + sext(imm,13);
                        break;
                    }
                    case 5: //BGE
                    {
                        if((int64_t)reg[rs1] >= (int64_t)reg[rs2])
                            next_pc = pc + sext(imm,13);
                        break;
                    }
                    case 6: //BLTU
                    {
                        if(reg[rs1] < reg[rs2])
                            next_pc = pc + sext(imm,13);
                        break;
                    }
                    case 7: //BGEU
                    {
                        if(reg[rs1] >= reg[rs2])
                            next_pc = pc + sext(imm,13);
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
            case 0x67:  //JALR
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t imm = ((insn >> 20) & 0xfff);
                reg[rd] = next_pc;
                next_pc = reg[rs1] + sext(imm,12);

                break;
            }
            case 0x6f:  //JAL
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t imm  = (((insn & 0x80000000) >> 11)  | // [20]
                                ((insn & 0x7fe00000)>> 20)   | // [10:1]
                                ((insn & 0x00100000) >> 9 )  | // [11]
                                ((insn & 0x000ff000)));         //[19:12]

                reg[rd] = next_pc;
                next_pc = pc + sext((imm),21);
                break;
            }
            case 0x73: //SYSTEM 
            {
                uint64_t csrno = ((insn >> 20) & 0xfff); //[31:20] = csrno
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t rd = ((insn >> 7) & 0x1f);
                switch ((insn >> 12) & 7)
                {
                     case 1: //csrrw
                    {
                        reg[rd]  = csr[csrno];
                        csr[csrno] = reg[rs1];
                        break;
                    }
                    case 2: //csrrs
                    {
                        reg[rd]  = csr[csrno];
                        csr[csrno] = reg[rd] | reg[rs1];
                        break;
                    }
                    case 3: //csrrc
                    {
                        reg[rd]  = csr[csrno];
                        csr[csrno] = reg[rd] & ~reg[rs1];
                        break;
                    }
                    case 5: //csrrwi
                    {
                        reg[rd]  = csr[csrno];
                        csr[csrno] = rs1;
                        break;
                    }
                    case 6: //csrrsi
                    {
                        reg[rd]  = csr[csrno];
                        csr[csrno] = reg[rd] | rs1;
                        break;
                    }
                    case 7: //csrrci
                    {
                        reg[rd]  = csr[csrno];
                        csr[csrno] = reg[rd] & ~rs1;
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