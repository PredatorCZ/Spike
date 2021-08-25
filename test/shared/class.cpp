#include "datas/reflector.hpp"

struct SimpleStruct {
  uint32 field0;
  uint32 field1;
};

REFLECT(CLASS(SimpleStruct), MEMBER(field0), MEMBER(field1))
