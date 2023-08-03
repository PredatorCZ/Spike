#include "spike/io/stat.hpp"
#include "uni_format.inl"

int main() {
  es::SetupWinApiConsole();
  es::print::AddPrinterFunction(es::Print);

  TEST_CASES(int testResult, TEST_FUNC(test_format_00),
             TEST_FUNC(test_format_01), TEST_FUNC(test_format_02),
             TEST_FUNC(test_format_03), TEST_FUNC(test_format_04));

  return testResult;
}
