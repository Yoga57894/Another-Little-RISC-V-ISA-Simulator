#include <gtest/gtest.h>
#include "ALISS.h"

TEST(MyTestSuite, MyTestCase) {
    ASSERT_TRUE(true);
}

TEST(ELFTestSuite, ELFLoaderTest_entryPoint) {
    const char* test = "test.elf";
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    ALISS::loadElf(test);
    EXPECT_EQ(ALISS::pc, 0x10116);
    free(ALISS::memory);
}

TEST(ELFTestSuite, ELFLoaderTest_FirstInst) {
    const char* test = "test.elf";
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    ALISS::loadElf(test);
    free(ALISS::memory);
}

TEST(IFTestSuite, InstFetchTest) {
    const char* test = "test.elf";
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    ALISS::loadElf(test);
    EXPECT_EQ(ALISS::IF(), 0x4197);
    free(ALISS::memory);
}

TEST(ID_EX_WB_TestSuite, PCNextTest) {
    const char* test = "test.elf";
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    ALISS::loadElf(test);
    ALISS::run_pipe();
    EXPECT_EQ(ALISS::pc, 0x1011a); //0x10116 + 4;
    free(ALISS::memory);
}

/*ISA Test*/

TEST(ISATESTSuite, ADD3_7)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 3;
    ALISS::reg[12] = 7;
    uint32_t insn = 0x00c58533; // add a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0xa); //3 + 7 = 10;
}

TEST(ISATESTSuite, ADDN1_1)
{
    ALISS::reg[11] = 0x0;
    ALISS::reg[10] = 0xffffffffffffffff;
    ALISS::reg[12] = 0x1;
    uint32_t insn = 0x00c505b3; // add a1 a0 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[11], 0); //1 + -1 = 0;
}

TEST(ISATESTSuite, ADDN1_N1)
{
    ALISS::reg[11] = 0x0;
    ALISS::reg[10] = 0xffffffffffffffff;
    ALISS::reg[12] = 0xffffffffffffffff;
    uint32_t insn = 0x00c505b3; // add a1 a0 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[11], 0xfffffffffffffffe); //-1 + -1  = -2;
}

TEST(ISATESTSuite, SUBN1_N1)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0xffffffffffffffff;
    ALISS::reg[12] = 0xffffffffffffffff;
    uint32_t insn = 0x40c58533; // sub a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x0); //-1 - -1  = 0;
}

TEST(ISATESTSuite, SLL_1_7)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x1;
    ALISS::reg[12] = 0x7;
    uint32_t insn = 0x00c59533; // sll a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x80); // (1 << 7) = 128
}

TEST(ISATESTSuite, SLT_1_f)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x1;
    ALISS::reg[12] = 0xf;
    uint32_t insn = 0x00c5a533; // slt a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x1); // (1 < f) == 1
}

TEST(ISATESTSuite, SLT_f_1)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0xf;
    ALISS::reg[12] = 0x1;
    uint32_t insn = 0x00c5a533; // slt a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x0); // (f < 1) == 0
}

TEST(ISATESTSuite, SLT_0_ffffffffffffffff)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0;
    ALISS::reg[12] = 0xffffffffffffffff;
    uint32_t insn = 0x00c5a533; // slt a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x0); // (0 < -1) == 0
}

TEST(ISATESTSuite, SLTU_0_ffffffffffffffff)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0;
    ALISS::reg[12] = 0xffffffffffffffff;
    uint32_t insn = 0x00c5b533; // slt a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x1); // (0 < (unsigned)ffffffffffffffff) = 1
}

TEST(ISATESTSuite, XOR_0f000f00_ff00ff00)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0f0f0f0f;
    ALISS::reg[12] = 0xff00ff00;
    uint32_t insn = 0x00c5c533; // xor a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0xf00ff00f ); //0x0f0f0f0f ^ 0xff00ff00 = 0xf00ff00f;
}

TEST(ISATESTSuite, SRL_ffffffffffffffff_5)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0xffffffffffffffff;
    ALISS::reg[12] = 0x5;
    uint32_t insn = 0x00c5d533; // srl a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x07ffffffffffffff ); //0xffffffffffffffff srl 5 = 07ffffffffffffff;
}

TEST(ISATESTSuite, SRA_ffffffffffffffff_5)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0xffffffffffffffff;
    ALISS::reg[12] = 0x5;
    uint32_t insn = 0x40c5d533; // srl a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0xffffffffffffffff ); //0xffffffffffffffff sra 5 = ffffffffffffffff;
}

TEST(ISATESTSuite, OR_0f000f00_ff00ff00)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0f0f0f0f;
    ALISS::reg[12] = 0xff00ff00;
    uint32_t insn = 0x00c5e533; // or a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0xff0fff0f ); //0x0f0f0f0f | 0xff00ff00 = 0xff0fff0f;
}

TEST(ISATESTSuite, AND_0f000f00_ff00ff00)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0f0f0f0f;
    ALISS::reg[12] = 0xff00ff00;
    uint32_t insn = 0x00c5f533; // and a0 a1 a2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x0f000f00 ); //0x0f0f0f0f & 0xff00ff00 = 0x0f000f00;
}

TEST(ISATESTSuite, ADDI3_7)
{
    ALISS::reg[11] = 3;
    //ALISS::reg[12] = 7;
    uint32_t insn = 0x00758513; // addi a0 a1 7
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0xa); //3 + 7 = 10;
}

TEST(ISATESTSuite, ADDI1_N800)
{
    ALISS::reg[10] = 800;
    //ALISS::reg[12] = 0x1;
    uint32_t insn = 0xce050593; // addi a1 a0 -800
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[11], 0); //800 + -800 = 0;
}

TEST(ISATESTSuite, SLLI_1_7)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x1;
    uint32_t insn = 0x00759513; // slli a0 a1 7
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x80); // (1 << 7) = 128
}

TEST(ISATESTSuite, SLTI_1_f)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x1;
    uint32_t insn = 0x00f5a513; // slti a0 a1 f
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x1); // (1 < f) == 1
}

TEST(ISATESTSuite, SLTI_f_1)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0xf;
    uint32_t insn = 0x0015a513; // slti a0 a1 1
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x0); // (f < 1) == 0
}

TEST(ISATESTSuite, SLTI_0_fff)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0;
//    ALISS::reg[12] = 0xffffffffffffffff;
    uint32_t insn = 0xfff5a513; // sltiu a0 a1 0xfff(only 12bits)
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x0); // (0 < -1) == 0
}

TEST(ISATESTSuite, SLTIU_0_fff)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0;
    uint32_t insn = 0xfff5b513; // sltiu a0 a1 0xfff
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x1); // (0 < (unsigned)ffffffffffffffff) = 1
}

TEST(ISATESTSuite, XORI_0f000f00_ff00)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0f0f0f0f;
    uint32_t insn = 0xf005c513; // xori a0 a1 (sext)0xff00
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0xfffffffff0f0f00f ); //0x0f0f0f0f ^ 0xffffffffffffff00 = 0xffffffff0f0ff00f;
}

TEST(ISATESTSuite, SRLI_ffffffffffffffff_5)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0xffffffffffffffff;
    uint32_t insn = 0x0055d513; // srli a0 a1 5
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x07ffffffffffffff ); //0xffffffffffffffff srli 5 = ffffffffffffffff;
}

TEST(ISATESTSuite, SRAI_ffffffffffffffff_5)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0xffffffffffffffff;
    uint32_t insn = 0x4055d513; // srai a0 a1 5
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0xffffffffffffffff ); //0xffffffffffffffff sra 5 = ffffffffffffffff;
}

TEST(ISATESTSuite, ORI_0f000f00_ff00)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0f0f0f0f;
    uint32_t insn = 0xf005e513; // ori a0 a1 (sext)0xff00
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0xffffffffffffff0f ); //0x0f0f0f0f | 0xffffffffffffff00 = 0xffffffffffffff0f;
}

TEST(ISATESTSuite, ANDI_0f000f00_ff00)
{
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x0f0f0f0f;
    uint32_t insn = 0xf005f513; // andi a0 a1 (sext)0xff00
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x0f0f0f00 ); //0x0f0f0f0f & 0xfffffffffffffff00 = 0x0f0f0f00;
}

TEST(ISATESTSuite, JALR_0x100_N128)
{
    ALISS::pc = 0x100;

    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x100;
    uint32_t insn = 0xf8058567; // jalr a0, -128(a1)
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x104 ); //return address = 104;
    EXPECT_EQ(ALISS::next_pc, 0x80 ); //pc = 0x80;
}

TEST(ISATESTSuite, JAL_0x100_N128)
{
    ALISS::pc = 0x100;

    ALISS::reg[10] = 0x0;
    uint32_t insn = 0xf81ff56f; // jal a0, -128
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::reg[10], 0x104 ); //return address = 104;
    EXPECT_EQ(ALISS::next_pc, 0x80 ); //pc = 0x80;
}

TEST(ISATESTSuite, BEQ_0x100_N128)
{
    ALISS::pc = 0x100;

    ALISS::reg[10] = 0x1234;
    ALISS::reg[11] = 0x1234;
    uint32_t insn = 0xf8b500e3; // beq a0, a1, -128
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::next_pc, 0x80 ); //pc = 0x80;
}

TEST(ISATESTSuite, BNE_0x100_N128)
{
    ALISS::pc = 0x100;

    ALISS::reg[10] = 0x1234;
    ALISS::reg[11] = 0x5678;
    uint32_t insn = 0xf8b510e3; // beq a0, a1, -128
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::next_pc, 0x80 ); //pc = 0x80;
}

TEST(ISATESTSuite, BLT_0x100_N128)
{
    ALISS::pc = 0x100;

    ALISS::reg[10] = 0x1234;
    ALISS::reg[11] = 0x5678;
    uint32_t insn = 0xf8b540e3; // beq a0, a1, -128
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::next_pc, 0x80 ); //pc = 0x80;
}

TEST(ISATESTSuite, BGE_0x100_N128)
{
    ALISS::pc = 0x100;

    ALISS::reg[10] = 0x1234;
    ALISS::reg[11] = 0x1234;
    uint32_t insn = 0xf8b550e3; // beq a0, a1, -128
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::next_pc, 0x80 ); //pc = 0x80;
}

TEST(ISATESTSuite, BLTU_0x100_N128)
{
    ALISS::pc = 0x100;

    ALISS::reg[10] = 0x5678;
    ALISS::reg[11] = -1;
    uint32_t insn = 0xf8b560e3; // beq a0, a1, -128
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::next_pc, 0x80 ); //pc = 0x80;
}


TEST(ISATESTSuite, BGEU_0x100_N128)
{
    ALISS::pc = 0x100;

    ALISS::reg[10] = -1;
    ALISS::reg[11] = 0x1234;

    uint32_t insn = 0xf8b570e3; // beq a0, a1, -128
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::next_pc, 0x80 ); //pc = 0x80;
}

TEST(ISATESTSuite, LB_0x40000_0xffffffff88888888)
{
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    uint64_t* memory64  = (uint64_t*)ALISS::memory;

    memory64[0x40000 / 8] = 0xffffffff88888888;
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x40100;

    uint32_t insn = 0xf0058503; // LB a0 -256(a1)
    ALISS::ID_EX_WB(insn);

    EXPECT_EQ(ALISS::reg[10], -120 ); //load -120;
    free(ALISS::memory);
}

TEST(ISATESTSuite, LBU_0x40000_0xffffffff88888888)
{
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    uint64_t* memory64  = (uint64_t*)ALISS::memory;

    memory64[0x40000 / 8] = 0xffffffff88888888;
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x40100;

    uint32_t insn = 0xf005c503; // LBU a0 -256(a1)
    ALISS::ID_EX_WB(insn);

    EXPECT_EQ(ALISS::reg[10], 0x88 ); //load 0x88;
    free(ALISS::memory);
}

TEST(ISATESTSuite, LH_0x40000_0xffffffff88888888)
{
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    uint64_t* memory64  = (uint64_t*)ALISS::memory;

    memory64[0x40000 / 8] = 0xffffffff88888888;
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x40100;

    uint32_t insn = 0xf0059503; // LH a0 -256(a1)
    ALISS::ID_EX_WB(insn);

    EXPECT_EQ(ALISS::reg[10], -30584 ); //load -30584;
    free(ALISS::memory);
}

TEST(ISATESTSuite, LHU_0x40000_0xffffffff88888888)
{
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    uint64_t* memory64  = (uint64_t*)ALISS::memory;

    memory64[0x40000 / 8] = 0xffffffff88888888;
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x40100;

    uint32_t insn = 0xf005d503; // LHU a0 -256(a1)
    ALISS::ID_EX_WB(insn);

    EXPECT_EQ(ALISS::reg[10], 0x8888 ); //load 34952;
    free(ALISS::memory);
}

TEST(ISATESTSuite, LW_0x40000_0xffffffff88888888)
{
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    uint64_t* memory64  = (uint64_t*)ALISS::memory;

    memory64[0x40000 / 8] = 0xffffffff88888888;
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x40100;

    uint32_t insn = 0xf005a503; // LHU a0 -256(a1)
    ALISS::ID_EX_WB(insn);

    EXPECT_EQ(ALISS::reg[10], -2004318072 ); //load -2004318072;
    free(ALISS::memory);
}

TEST(ISATESTSuite, SB_0x40000_0xffffffff88888888)
{
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test

    ALISS::reg[10] = 0xffffffff88888888;
    ALISS::reg[11] = 0x40100;

    uint64_t* memory64  = (uint64_t*)ALISS::memory;
    memory64[0x40000 / 8 ] =  0x0;

    uint32_t insn = 0xf0a58023; // SB a0 -256(a1)
    ALISS::ID_EX_WB(insn);

    EXPECT_EQ(memory64[0x40000 /  8], 0x88 ); //store 88 to memory;
    free(ALISS::memory);
}

TEST(ISATESTSuite, SH_0x40000_0xffffffff88888888)
{
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test

    ALISS::reg[10] = 0xffffffff88888888;
    ALISS::reg[11] = 0x40100;

    uint64_t* memory64  = (uint64_t*)ALISS::memory;
    memory64[0x40000 / 8 ] =  0x0;
 
    uint32_t insn = 0xf0a59023; // SH a0 -256(a1)
    ALISS::ID_EX_WB(insn);

    EXPECT_EQ(memory64[0x40000 / 8], 0x8888 ); //store 8888 to memory;
    free(ALISS::memory);
}

TEST(ISATESTSuite, SW_0x40000_0xffffffff88888888)
{
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test

    ALISS::reg[10] = 0xffffffff88888888;
    ALISS::reg[11] = 0x40100;

    uint64_t* memory64  = (uint64_t*)ALISS::memory;
    memory64[0x40000 / 8 ] =  0x0;
 
    uint32_t insn = 0xf0a5a023; // SW a0 -256(a1)
    ALISS::ID_EX_WB(insn);

    EXPECT_EQ(memory64[0x40000 / 8], 0x88888888 ); //store 88888888 to memory;
    free(ALISS::memory);
}

TEST(ISATESTSuite, csrrw_0x7cc_0x1234)
{
    ALISS::csr[0x7cc] = 0x1010101010101010;
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x1234;
    uint32_t insn = 0x7cc59573; // csrrw a0 0x7cc a1
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::csr[0x7cc], 0x1234 ); //csr write to 1234;
    EXPECT_EQ(ALISS::reg[10], 0x1010101010101010 ); //reg  is ori value;
}

TEST(ISATESTSuite, csrrs_0x7cc_0x101)
{
    ALISS::csr[0x7cc] = 0x1010101010101010;
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x101;
    uint32_t insn = 0x7cc5a573; // csrrs a0 0x7cc a1
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::csr[0x7cc], 0x1010101010101111 ); //csrset [1], [9];
    EXPECT_EQ(ALISS::reg[10], 0x1010101010101010 ); //reg is ori value;
}

TEST(ISATESTSuite, csrrc_0x7cc_0x1010)
{
    ALISS::csr[0x7cc] = 0x1010101010101010;
    ALISS::reg[10] = 0x0;
    ALISS::reg[11] = 0x1010;
    uint32_t insn = 0x7cc5b573; // csrrc a0 0x7cc a1
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::csr[0x7cc], 0x1010101010100000 ); //csr clear [4], [12];
    EXPECT_EQ(ALISS::reg[10], 0x1010101010101010 ); //reg is ori value;
}

TEST(ISATESTSuite, csrrwi_0x7cc_0x2)
{
    ALISS::csr[0x7cc] = 0x1010101010101010;
    ALISS::reg[10] = 0x0;
    uint32_t insn = 0x7cc15573; // csrrwi x10, 0x7cc, 2
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::csr[0x7cc], 0x2 ); //csr write to 2;
    EXPECT_EQ(ALISS::reg[10], 0x1010101010101010 ); //reg  is ori value;
}

TEST(ISATESTSuite, csrrsi_0x7cc_0x1)
{
    ALISS::csr[0x7cc] = 0x1010101010101010;
    ALISS::reg[10] = 0x0;
    uint32_t insn = 0x7cc0e573; // csrrsi x10, 0x7cc, 1
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::csr[0x7cc], 0x1010101010101011 ); //csrset [1];
    EXPECT_EQ(ALISS::reg[10], 0x1010101010101010 ); //reg is ori value;
}

TEST(ISATESTSuite, csrrci_0x7cc_0x10)
{
    ALISS::csr[0x7cc] = 0x1010101010101010;
    ALISS::reg[10] = 0x0;
    uint32_t insn = 0x7cc87573; // csrrci a0 0x7cc 0x10
    ALISS::ID_EX_WB(insn);
    EXPECT_EQ(ALISS::csr[0x7cc], 0x1010101010101000 ); //csr clear [4];
    EXPECT_EQ(ALISS::reg[10], 0x1010101010101010 ); //reg is ori value;
}

