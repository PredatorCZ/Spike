#include "datas/reflector.hpp"
#include "datas/unit_testing.hpp"
#include "datas/tchar.hpp"

struct SimpleStruct;

struct SimpleStruct2 {
    uint32 field;
};

REFLECTOR_CREATE(SimpleStruct2, 1, VARNAMES, field)

int main() {
    printer.AddPrinterFunction(UPrintf);
    auto rf0 = GetReflectedClass<SimpleStruct>();

    TEST_EQUAL(rf0->nTypes, 2);

    auto rf1 = GetReflectedClass<SimpleStruct2>();

    TEST_EQUAL(rf1->nTypes, 1);

    return 0;
}
