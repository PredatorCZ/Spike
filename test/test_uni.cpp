#include "../datas/tchar.hpp"
#include "uni_format.inl"

int main() {
  printer.AddPrinterFunction(UPrintf);

  TEST_CASES(int testResult, TEST_FUNC(test_format_00),
             TEST_FUNC(test_format_01), TEST_FUNC(test_format_02),
             TEST_FUNC(test_format_03), TEST_FUNC(test_format_04));

  return testResult;
}