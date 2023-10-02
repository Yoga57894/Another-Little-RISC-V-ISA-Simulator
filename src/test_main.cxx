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
}

TEST(ELFTestSuite, ELFLoaderTest_FirstInst) {
    const char* test = "test.elf";
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    ALISS::loadElf(test);
    EXPECT_EQ(ALISS::get_mem_w(0x10116), 0x4197);
}

TEST(IFTestSuite, InstFetchTest) {
    const char* test = "test.elf";
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    ALISS::loadElf(test);
    EXPECT_EQ(ALISS::IF(), 0x4197);
}

TEST(ID_EX_WB_TestSuite, PCNextTest) {
    const char* test = "test.elf";
    ALISS::memory = (uint8_t *)std::malloc(4 * 1024 * 1024); //4MB size for test
    ALISS::loadElf(test);
    ALISS::run_pipe();
    EXPECT_EQ(ALISS::pc, 0x1011a); //0x10116 + 4;
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
