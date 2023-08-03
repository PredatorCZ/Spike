#include "spike/util/unit_testing.hpp"
#include "spike/util/endian.hpp"

struct endianTest {
    uint16 item0;
    uint32 item1;
    float item2;
    uint64 item3;
    double item4;

    uint32 items0[3];

    void SwapEndian() {
        FByteswapper(item0);
        FByteswapper(item1);
        FByteswapper(item2);
        FByteswapper(item3);
        FByteswapper(item4);
        FByteswapper(items0);
    }
};

struct endianArrayTest {
    uint32 field0;
    uint32 field1;
    uint32 field2;
};

int test_endian() {
    endianTest test;
    test.item0 = 0xabcd;
    test.item1 = 0x90abcdef;
    test.item2 = 2.3509887E-38f;
    test.item3 = 0x1234567890abcdef;
    const uint64 itdbl = 0x0100000000000000;
    test.item4 = reinterpret_cast<const double&>(itdbl);
    test.items0[0] = 0x12345678;
    test.items0[1] = 0x789abcde;
    test.items0[2] = 0x98765432;

    FByteswapper(test);

    TEST_EQUAL(test.item0, 0xcdab);
    TEST_EQUAL(test.item1, 0xefcdab90);
    TEST_EQUAL(test.item2, 1.4E-45f);
    TEST_EQUAL(test.item3, 0xefcdab9078563412);
    TEST_EQUAL(reinterpret_cast<const uint64&>(test.item4), 1);
    TEST_EQUAL(test.items0[0], 0x78563412);
    TEST_EQUAL(test.items0[1], 0xdebc9a78);
    TEST_EQUAL(test.items0[2], 0x32547698);

    endianArrayTest test1;
    test1.field0 = 0x12345678;
    test1.field1 = 0x789abcde;
    test1.field2 = 0x98765432;

    FArraySwapper<uint32>(test1);

    TEST_EQUAL(test.items0[0], 0x78563412);
    TEST_EQUAL(test.items0[1], 0xdebc9a78);
    TEST_EQUAL(test.items0[2], 0x32547698);

    return 0;
}
