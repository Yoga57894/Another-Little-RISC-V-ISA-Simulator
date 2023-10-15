#ifndef __MAIN_HEADER
#define __MAIN_HEADER

//#define BAREMETAL
//#define INSN_DEBUG

#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <elf.h>
#include <unordered_map>

namespace ALISS
{

    static bool debug_mode = false;

    static uint8_t* memory = 0;
    static uint64_t pc;
    static uint64_t next_pc;
    static uint64_t reg[32];
    static std::string insn_type;
    static bool reservation = 0;
    static std::unordered_map<uint32_t,uint64_t> csr;

    void dump_insn(uint32_t insn)
    {
        printf("pc = %016lx , insn = %08x \n",pc,insn);
        for(uint32_t i = 0 ; i < 4 ; i ++)
        {
            for(uint32_t j = 0 ; j < 8 ; j ++)
            {
                printf("reg[%02d] = %016lx ,", i * 8 + j ,reg[i*8+j]);

            }
            printf("\n");
        }
        //printf("%016lx\n",reg[10]);
        //std::cout <<  insn_type <<  std::endl;
        printf("/////////////////////////////////////////////////////\n");
        printf("/////////////////////////////////////////////////////\n");
    }


    inline int64_t sext(uint64_t insn, int len) { return int64_t(insn) << (64 - len) >> (64 - len); }

    //ref : https://github.com/riscv-software-src/riscv-isa-sim

    inline uint64_t mulhu(uint64_t a, uint64_t b)
    {
      uint64_t t;
      uint32_t y1, y2, y3;
      uint64_t a0 = (uint32_t)a, a1 = a >> 32;
      uint64_t b0 = (uint32_t)b, b1 = b >> 32;

      t = a1*b0 + ((a0*b0) >> 32);
      y1 = t;
      y2 = t >> 32;

      t = a0*b1 + y1;

      t = a1*b1 + y2 + (t >> 32);
      y2 = t;
      y3 = t >> 32;

      return ((uint64_t)y3 << 32) | y2;
    }

    inline int64_t mulh(int64_t a, int64_t b)
    {
      int negate = (a < 0) != (b < 0);
      uint64_t res = mulhu(a < 0 ? -a : a, b < 0 ? -b : b);
      return negate ? ~res + (a * b == 0) : res;
    }

    inline int64_t mulhsu(int64_t a, uint64_t b)
    {
      int negate = a < 0;
      uint64_t res = mulhu(a < 0 ? -a : a, b);
      return negate ? ~res + (a * b == 0) : res;
    }

    ///

    bool loadElf(const char* filename)
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

    bool loadDTB(const char* filename, uint64_t dtb_addr )
    {
        // Open the file
        FILE* file = fopen(filename, "rb");
        if (file == NULL) {
            perror("Failed to open the file");
            exit(EXIT_FAILURE);
        }

        // Get the file size
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        // Allocate memory to store the file content
        uint8_t* file_data = (uint8_t*)malloc(file_size);
        if (file_data == NULL) {
            perror("Memory allocation failed");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Read the file content
        size_t bytes_read = fread(file_data, 1, file_size, file);
        if (bytes_read != file_size) {
            perror("Failed to read the file");
            free(file_data);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Use memcpy to copy the file content into memory
        memcpy(memory + dtb_addr, file_data, file_size);

        // Close the file and free memory
        fclose(file);
        free(file_data);

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
        uint64_t* memory64  = (uint64_t*)(ALISS::memory + addr);
        *memory64 = data;
 
        return;
    };

    void set_mem_w(uint64_t addr, uint32_t data)
    {
        uint32_t* memory32  = (uint32_t*)(ALISS::memory + addr);
        *memory32 = data;
        return;
    };

    void set_mem_h(uint64_t addr, uint16_t data)
    {
        uint16_t* memory16  = (uint16_t*)(ALISS::memory + addr);
        *memory16 = data;
        return;
    };

    void set_mem_b(uint64_t addr, uint8_t data)
    {
        uint8_t* memory8 = (uint8_t*)(ALISS::memory + addr);
        *memory8 = data;
        return;
    };

    uint32_t IF()
    {
        return get_mem_w(pc);
    };

    void ID_EX_WB(uint32_t insn)
    {
        reg[0] = 0; //ZERO always return 0
        ALISS::next_pc = ALISS::pc + 4;
        uint8_t decode = insn & 0x7f; //last 7 bit

        switch(decode)
        {
            case 0x3:  //LOAD
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t imm = ((insn >> 20) & 0xfff);

               if(reg[rs1] + sext(imm,12) == 0x10000005)
               {
                   reg[rd] = 0x60;
                   break;
               }


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
            case 0xf: //fence
            {
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
                        uint64_t shamt = imm & 0x3f; //[25:20]
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
                        reg[rd] = (reg[rs1] < sext(imm,12));
                        break;
                    }
                    case 0x4: //XORI
                    {
                        reg[rd] = (reg[rs1] ^ sext(imm, 12));
                        break;
                    }
                    case 0x5: //SRLI & SRAI
                    {
                        uint64_t shamt = imm & 0x3f; //[25:20]
                        switch ((insn >> 26) & 0x7f)
                        {
                            case 0x0 : //SRLI
                            {
                                reg[rd] =  reg[rs1] >> shamt;
                                break;
                            }
                            case 0x10 : //SRAI
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
            case 0x17: //AUIPC
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t imm = ((insn >> 12) & 0xfffff);
                reg[rd] = pc + sext((imm << 12),32);
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
                        uint64_t shamt = imm & 0x3f; //[25:20]
                        reg[rd] = sext(((reg[rs1]  & 0xffffffff) << shamt),32);
                        break;
                    }
                    case 0x5: //SRLIW & SRAIW
                    {
                        uint64_t shamt = imm & 0x3f; //[25:20]
                        switch ((insn >> 25) & 0x7f)
                        {
                            case 0x0 : //SRLIW
                            {
                                reg[rd] =  sext(((reg[rs1] & 0xffffffff) >> shamt) , 32);
                                break;
                            }
                            case 0x20 : //SRAIW
                            {
                                reg[rd] =  sext(((int32_t)(reg[rs1] & 0xffffffff) >> shamt) , 32);
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

                if(reg[rs1] + sext(imm,12) == 0x10000000)
                {
                    printf("%c",(uint8_t)reg[rs2]);
                    break;
                }

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
            case 0x2f: //A-EXTENSION
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t rs2 = ((insn  >> 20) & 0x1f);
                switch ((insn >> 12) & 7) //func3
                {
                    case 0x2: // A32
                    {
                        switch((insn >> 27 ) & 0x1f)
                        {
                            case 0x2: //LR.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                reservation = true;
                                break;
                            }
                            case 0x3: //SC.W
                            {
                                if(reservation)
                                {
                                    set_mem_w(reg[rs1], (uint32_t)reg[rs2]);
                                    reg[rd] = 0;
                                }
                                else
                                {

                                    reg[rd] = 1;
                                }
                                reservation = false;
                                break;
                            }
                            case 0x1: //AMOSWP.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                set_mem_w(reg[rs1], (uint32_t)reg[rs2]);
                                break;
                            }
                            case 0x0: //AMOADD.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                set_mem_w(reg[rs1], (uint32_t)reg[rs2] + (uint32_t)reg[rd]);
                                break;
                            }
                            case 0x4: //AMOXOR.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                set_mem_w(reg[rs1], (uint32_t)reg[rs2] ^ (uint32_t)reg[rd]);
                                break;
                            }
                            case 0xc: //AMOAND.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                set_mem_w(reg[rs1], (uint32_t)reg[rs2] & (uint32_t)reg[rd]);
                                break;
                            }
                            case 0x8: //AMOOR.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                set_mem_w(reg[rs1], (uint32_t)reg[rs2] | (uint32_t)reg[rd]);
                                break;
                            }
                            case 0x10: //AMOMIN.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                set_mem_w(reg[rs1], (int32_t)reg[rs2] < (int32_t)reg[rd] ? (uint32_t)reg[rs2] : (uint32_t)reg[rd] );
                                break;
                            }
                            case 0x14: //AMOMAX.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                set_mem_w(reg[rs1], (int32_t)reg[rs2] > (int32_t)reg[rd] ? (uint32_t)reg[rs2] : (uint32_t)reg[rd] );
                                break;
                            }
                            case 0x18: //AMOMINU.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                set_mem_w(reg[rs1], (uint32_t)reg[rs2] < (uint32_t)reg[rd] ? (uint32_t)reg[rs2] : (uint32_t)reg[rd] );
                                break;
                            }
                            case 0x1c: //AMOMAXU.W
                            {
                                reg[rd] = sext(get_mem_w(reg[rs1]),32);
                                set_mem_w(reg[rs1], (uint32_t)reg[rs2] > (uint32_t)reg[rd] ? (uint32_t)reg[rs2] : (uint32_t)reg[rd] );
                                break;
                            }
                        }
                        break;
                    }
                    case 0x3: // A64
                    {
                        switch((insn >> 27 ) & 0x1f)
                        {
                            case 0x2: //LR.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                reservation = true;
                                break;
                            }
                            case 0x3: //SC.D
                            {
                                if(reservation)
                                {
                                    set_mem_d(reg[rs1], reg[rs2]);
                                    reg[rd] = 0;
                                }
                                else
                                {

                                    reg[rd] = 1;
                                }
                                reservation = false;
                                break;
                            }
                            case 0x1: //AMOSWP.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                set_mem_d(reg[rs1], reg[rs2]);
                                break;
                            }
                            case 0x0: //AMOADD.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                set_mem_d(reg[rs1], reg[rs2] + reg[rd]);
                                break;
                            }
                            case 0x4: //AMOXOR.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                set_mem_d(reg[rs1], reg[rs2] ^ reg[rd]);
                                break;
                                }
                            case 0xc: //AMOAND.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                set_mem_d(reg[rs1], reg[rs2] & reg[rd]);
                                break;
                            }
                            case 0x8: //AMOOR.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                set_mem_d(reg[rs1], reg[rs2] | reg[rd]);
                                break;
                            }
                            case 0x10: //AMOMIN.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                set_mem_d(reg[rs1], (int64_t)reg[rs2] < (int64_t)reg[rd] ? reg[rs2] : reg[rd] );
                                break;
                            }
                            case 0x14: //AMOMAX.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                set_mem_d(reg[rs1], (int64_t)reg[rs2] > (int64_t)reg[rd] ? reg[rs2] : reg[rd] );
                                break;
                            }
                            case 0x18: //AMOMINU.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                set_mem_d(reg[rs1], reg[rs2] < reg[rd] ? reg[rs2] : reg[rd] );
                                break;
                            }
                            case 0x1c: //AMOMAXU.D
                            {
                                reg[rd] = get_mem_d(reg[rs1]);
                                set_mem_d(reg[rs1], reg[rs2] > reg[rd] ? reg[rs2] : reg[rd] );
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
    if(((insn >> 25) & 0x7f) == 0x1) //M-extension
    {
        switch ((insn >> 12) & 7)
        {
            case 0x0: //MUL
            {
                reg[rd] = (int64_t)reg[rs1] * (int64_t)reg[rs2];

                break;
            }
            case 0x1: //MULH
            {
                reg[rd] = mulh(reg[rs1],reg[rs2]);
                break;
            }
            case 0x2: //MULHSU
            {
                reg[rd] = mulhsu(reg[rs1],reg[rs2]);
                break;
            }
            case 0x3: //MULHU
            {
                reg[rd] = mulhu(reg[rs1],reg[rs2]);
                break;
            }
            case 0x4: //DIV
            {
                if(reg[rs2] == 0) //can't div 0
                {
                    reg[rd] = -1;
                }
                else if((int64_t)reg[rs1] == INT64_MIN && (int64_t)reg[rs2] == -1) // may overflow
                {
                    reg[rd] = reg[rs1];
                }
                else
                {
                    reg[rd] = (int64_t)reg[rs1] / (int64_t)reg[rs2];
                }
                break;
            }
            case 0x5: //DIVU
            {
                if(reg[rs2] == 0) //can't div 0
                {
                    reg[rd] = -1;
                }
                else
                {
                    reg[rd] = reg[rs1] / reg[rs2];
                }
                break;
            }
            case 0x6: //REM
            {
                if(reg[rs2] == 0) //can't div 0
                {
                    reg[rd] = reg[rs1];
                }
                else if((int64_t)reg[rs1] == INT64_MIN && (int64_t)reg[rs2] == -1) // may overflow
                {
                    reg[rd] = 0;
                }
                else
                {
                    reg[rd] = (int64_t)reg[rs1] % (int64_t)reg[rs2];
                }
                break;
            }
            case 0x7: //REMU
            {
                if(reg[rs2] == 0) //can't div 0
                {
                    reg[rd] = reg[rs1];
                }
                else
                {
                    reg[rd] = reg[rs1] % reg[rs2];
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

    }
    else
                {
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
                }
                break;
            }
            case 0x37: //lui
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t imm = ((insn >> 12) & 0xfffff);
                reg[rd] = sext((imm << 12),32);
                break;
            }
            case 0x3b: //OP-32bit
            {
                uint64_t rd = ((insn >> 7) & 0x1f);
                uint64_t rs1 = ((insn  >> 15) & 0x1f);
                uint64_t rs2 = ((insn  >> 20) & 0x1f);
                if(((insn >> 25) & 0x7f) == 0x1) //M-extension
                {
                    switch ((insn >> 12) & 7)
                    {
                        case 0x0: //MULW
                        {
                            reg[rd] = sext((int64_t)reg[rs1] * (int64_t)reg[rs2] & 0xffffffff,32);

                            break;
                        }
                        case 0x4: //DIVW
                        {
                            if(reg[rs2] == 0) //can't div 0
                            {
                                reg[rd] = -1;
                            }
                            else if((int64_t)reg[rs1] == INT32_MIN && (int64_t)reg[rs2] == -1) // may overflow
                            {
                                reg[rd] = reg[rs1];
                            }
                            else
                            {
                             reg[rd] = sext(((int32_t)(reg[rs1]  & 0xffffffff)) / ((int32_t)(reg[rs2] & 0xffffffff)),32);
                            }
                            break;
                        }
                        case 0x5: //DIVUW
                        {
                            if(reg[rs2] == 0) //can't div 0
                            {
                                reg[rd] = -1;
                            }
                            else
                            {
                                reg[rd] = sext((reg[rs1] & 0xffffffff) / (reg[rs2] & 0xffffffff),32);
                            }
                            break;
                        }
                        case 0x6: //REMW
                        {
                            if(reg[rs2] == 0) //can't div 0
                            {
                                reg[rd] = reg[rs1];
                            }
                            else if((int64_t)reg[rs1] == INT32_MIN && (int64_t)reg[rs2] == -1) // may overflow
                            {
                                reg[rd] = 0;
                            }
                            else
                            {
                                reg[rd] = sext(((int32_t)(reg[rs1]  & 0xffffffff)) % ((int32_t)(reg[rs2] & 0xffffffff)),32);
                            }
                            break;
                        }
                        case 0x7: //REMUW
                        {
                            if(reg[rs2] == 0) //can't div 0
                            {
                                reg[rd] = reg[rs1];
                            }
                            else
                            {
                                reg[rd] = sext((reg[rs1] & 0xffffffff) % (reg[rs2] & 0xffffffff),32);
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

                }
                else
                {
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
                            reg[rd] = sext(((reg[rs1]  & 0xffffffff) << (reg[rs2] & 0x1f)),32);
                            break;
                        }
                        case 0x5: //SRLW or SRAW
                        {
                            switch ((insn >> 25) & 0x7f)
                            {
                               case 0x0 : //SRLW
                               {
                                    reg[rd] =  sext(((reg[rs1] &  0xffffffff) >> (reg[rs2] & 0x1f)),32);
                                    break;
                               }
                               case 0x20 : //SRAW
                               {
                                    reg[rd] = sext(((int32_t)(reg[rs1] & 0xffffffff) >> (reg[rs2] & 0x1f)),32);
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

                        if(debug_mode)
                            insn_type = "BNE";
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
                uint64_t temp = next_pc;
                next_pc = reg[rs1] + sext(imm,12);
                reg[rd] = temp;

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
                    case 0: //System 
                    {
                        uint64_t systemop = csrno;
                        switch(systemop)
                        {
                            case 0x0: //ecall
                            {
#ifdef BAREMETAL
                                if(reg[17] == 93) //exit
                                {
                                    printf("return value = %d\n",(int)reg[10]);
                                    exit(reg[10]);
                                }
#endif
                                
                                csr[0x341] = pc; //epc = pc
                                next_pc = csr[0x305]; //mtvec = 0x305
                                break;
                            }
                            case 0x1: //ebreak
                            {
                                csr[0x341] = pc;
                                next_pc = csr[0x305]; // mtvec = 0x305
                                break;
                            }
                            case 0x105: // wfi
                            {
                                //implement as nop ...
                                break;
                            }
                            case 0x302: //mret
                            {
                                next_pc = csr[0x341]; //epc = 0x341
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
    };

    void run_pipe()
    {
        uint32_t insn = ALISS::IF();
        ALISS::ID_EX_WB(insn);

    if(debug_mode)
        dump_insn(insn);

        pc = next_pc;
    };
}

#endif