#include "../datas/tchar.hpp"
#include "uni_format.inl"

int main() {
  printer.AddPrinterFunction(UPrintf);

  TEST_CASES(int testResult, TEST_FUNC(test_format_00));

  return testResult;
}