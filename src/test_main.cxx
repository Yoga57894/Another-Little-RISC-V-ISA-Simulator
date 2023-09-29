#include <gtest/gtest.h>
#include "main.h"

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