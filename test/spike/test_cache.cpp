#include "spike/app/cache.hpp"
#include "spike/app/console.hpp"
#include "spike/io/binwritter.hpp"
#include "spike/io/directory_scanner.hpp"
#include "spike/io/stat.hpp"
#include "spike/util/supercore.hpp"
#include "spike/util/unit_testing.hpp"

std::string RequestTempFile() { return "wal_file"; }

int test_dirscan() {
  DirectoryScanner sc;
  CacheGenerator cGen;

  sc.Scan("");
  size_t curFile = 0;
  size_t curFile2 = sc.Files().size();

  for (auto &f : sc) {
    cGen.AddFile(f, curFile++, curFile2++);
  }

  {
    BinWritter wr("cache.spch");

    cGen.WaitAndWrite(wr);
  }

  es::MappedFile mf("cache.spch");
  Cache iCache;
  iCache.Mount(mf.data);

  curFile = 0;
  curFile2 = sc.Files().size();

  for (auto &f : sc) {
    ZipEntry entry = iCache.RequestFile(f);

    TEST_EQUAL(entry.offset, curFile);
    TEST_EQUAL(entry.size, curFile2);

    curFile++;
    curFile2++;
  }

  return 0;
}

int main() {
  setlocale(LC_ALL, "C.UTF-8");
  setlocale(LC_NUMERIC, "en-US");
  es::SetupWinApiConsole();
  es::print::AddPrinterFunction(es::Print);

  printline("Printed some line into console and logger.");

  TEST_CASES(int testResult, TEST_FUNC(test_dirscan));

  return testResult;
}
