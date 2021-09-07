#include "datas/directory_scanner.hpp"
#include "datas/unit_testing.hpp"
#include "datas/supercore.hpp"
#include "datas/tchar.hpp"

using namespace es::string_view_literals;

int test_dirscan() {
  DirectoryScanner sc;

  sc.Scan("");
  const auto &fnd = sc.Files();

  TEST_NOT_CHECK(
      es::IsEnd(fnd, std::find(fnd.begin(), fnd.end(), "uni/skeleton.hpp")));
  TEST_NOT_CHECK(
      es::IsEnd(fnd, std::find(fnd.begin(), fnd.end(), "datas/reflector.hpp")));
  TEST_NOT_CHECK(
      es::IsEnd(fnd, std::find(fnd.begin(), fnd.end(), "test/test_app.cpp")));
  TEST_NOT_CHECK(
      es::IsEnd(fnd, std::find(fnd.begin(), fnd.end(), ".gitignore")));

  sc.AddFilter(".cpp"sv);
  sc.AddFilter(".inl"sv);

  sc.Clear();
  sc.Scan("");

  TEST_CHECK(
      es::IsEnd(fnd, std::find(fnd.begin(), fnd.end(), "uni/skeleton.hpp")));
  TEST_CHECK(
      es::IsEnd(fnd, std::find(fnd.begin(), fnd.end(), "datas/reflector.hpp")));
  TEST_NOT_CHECK(
      es::IsEnd(fnd, std::find(fnd.begin(), fnd.end(), "test/test_app.cpp")));
  TEST_CHECK(es::IsEnd(fnd, std::find(fnd.begin(), fnd.end(), ".gitignore")));
  TEST_NOT_CHECK(
      es::IsEnd(fnd, std::find(fnd.begin(), fnd.end(), "test/reflector.inl")));

  return 0;
}

int main() {
  setlocale(LC_ALL, "C.UTF-8");
  setlocale(LC_NUMERIC, "en-US");
  es::print::AddPrinterFunction(UPrintf);

  printline("Printed some line into console and logger.");

  TEST_CASES(int testResult, TEST_FUNC(test_dirscan));

  return testResult;
}
