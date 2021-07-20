#include "datas/reflector.hpp"

struct SimpleStruct {
    uint32 field0;
    uint32 field1;
};

REFLECTOR_CREATE(SimpleStruct, 1, VARNAMES, field0, field1)
