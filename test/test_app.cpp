#include "../datas/directory_scanner.hpp"
#include "../datas/settings_manager.hpp"
#include "../datas/unit_testing.hpp"

static struct SampleSettings : SettingsManager<SampleSettings> {
  bool boolVal0 = false;
  bool boolVal1 = true;
  int intVal0 = 10;
  float floatVal0 = 50.123f;
} settings;

REFLECT(CLASS(SampleSettings),
        MEMBER(boolVal0, "setting1", ReflDesc{"This is a 1st setting."}),
        MEMBER(boolVal1, "setting2", ReflDesc{"This is a 2nd setting."}),
        MEMBER(intVal0, "number", ReflDesc{"A numerical setting."}),
        MEMBER(floatVal0, "decimal", ReflDesc{"A decimal setting."}));

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

  sc.AddFilter(".cpp");
  sc.AddFilter(".inl");

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
  settings.CreateLog("build/testingLog");

  printline("Printed some line into console and logger.");

  TEST_CASES(int testResult, TEST_FUNC(test_dirscan));

  return testResult;
}
