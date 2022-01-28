#include "datas/reflector.hpp"
#include "datas/stat.hpp"
#include "datas/unit_testing.hpp"

struct SimpleStruct;

struct SimpleStruct2 {
  uint32 field;
};

REFLECT(CLASS(SimpleStruct2), MEMBER(field))

int main() {
  es::SetupWinApiConsole();
  es::print::AddPrinterFunction(es::Print);
  auto rf0 = GetReflectedClass<SimpleStruct>();

  TEST_EQUAL(rf0->nTypes, 2);

  auto rf1 = GetReflectedClass<SimpleStruct2>();

  TEST_EQUAL(rf1->nTypes, 1);

  return 0;
}
