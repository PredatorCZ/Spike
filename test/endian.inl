#include "../datas/unit_testing.hpp"
#include "../datas/endian.hpp"

struct endianTest {
    uint16 item0;
    uint32 item1;
    float item2;
    uint64 item3;
    double item4;

    void SwapEndian() {
        FByteswapper(item0);
        FByteswapper(item1);
        FByteswapper(item2);
        FByteswapper(item3);
        FByteswapper(item4);
    }
};

int test_endian() {
    endianTest test;
    test.item0 = 0xabcd;
    test.item1 = 0x90abcdef;
    test.item2 = 2.3509887E-38f;
    test.item3 = 0x1234567890abcdef;
    const uint64 itdbl = 0x0100000000000000;
    test.item4 = reinterpret_cast<const double&>(itdbl);

    FByteswapper(test);

    TEST_EQUAL(test.item0, 0xcdab);
    TEST_EQUAL(test.item1, 0xefcdab90);
    TEST_EQUAL(test.item2, 1.4E-45f);
    TEST_EQUAL(test.item3, 0xefcdab9078563412);
    TEST_EQUAL(reinterpret_cast<const uint64&>(test.item4), 1);

    return 0;
}