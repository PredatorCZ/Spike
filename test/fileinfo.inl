#include "../datas/fileinfo.hpp"
#include "../datas/unit_testing.hpp"

int test_fileinfo() {
  AFileInfo fleinf("/home\\user/GitHub\\PreCore/build/DartConfiguration.tcl");

  TEST_EQUAL(fleinf.GetExtension(), ".tcl");
  TEST_EQUAL(fleinf.GetFolder(), "/home/user/GitHub/PreCore/build/");
  TEST_EQUAL(fleinf.GetFilename(), "DartConfiguration");
  TEST_EQUAL(fleinf.GetFilenameExt(), "DartConfiguration.tcl");
  TEST_EQUAL(fleinf.GetFullPathNoExt(),
             "/home/user/GitHub/PreCore/build/DartConfiguration");
  TEST_EQUAL(fleinf.GetFullPath(),
             "/home/user/GitHub/PreCore/build/DartConfiguration.tcl");

  auto exploded = fleinf.Explode();

  TEST_EQUAL(exploded.size(), 6);
  TEST_EQUAL(exploded[0], "home");
  TEST_EQUAL(exploded[1], "user");
  TEST_EQUAL(exploded[2], "GitHub");
  TEST_EQUAL(exploded[3], "PreCore");
  TEST_EQUAL(exploded[4], "build");
  TEST_EQUAL(exploded[5], "DartConfiguration.tcl");

  auto caughtBranch = fleinf.CatchBranch("C:/Program Data/home/user/GitHub/");

  TEST_EQUAL(caughtBranch, "C:/Program Data/home/user/GitHub/PreCore/build/DartConfiguration.tcl");

  fleinf.Load("/home\\user/GitHub\\PreCore/build/.tcl");

  TEST_EQUAL(fleinf.GetExtension(), ".tcl");
  TEST_EQUAL(fleinf.GetFolder(), "/home/user/GitHub/PreCore/build/");
  TEST_EQUAL(fleinf.GetFilename(), "");
  TEST_EQUAL(fleinf.GetFilenameExt(), ".tcl");
  TEST_EQUAL(fleinf.GetFullPathNoExt(), "/home/user/GitHub/PreCore/build/");
  TEST_EQUAL(fleinf.GetFullPath(), "/home/user/GitHub/PreCore/build/.tcl");

  exploded = fleinf.Explode();

  TEST_EQUAL(exploded.size(), 6);
  TEST_EQUAL(exploded[0], "home");
  TEST_EQUAL(exploded[1], "user");
  TEST_EQUAL(exploded[2], "GitHub");
  TEST_EQUAL(exploded[3], "PreCore");
  TEST_EQUAL(exploded[4], "build");
  TEST_EQUAL(exploded[5], ".tcl");

  fleinf.Load("/home\\user/GitHub\\PreCore/build/DartConfiguration");

  TEST_EQUAL(fleinf.GetExtension(), "");
  TEST_EQUAL(fleinf.GetFolder(), "/home/user/GitHub/PreCore/build/");
  TEST_EQUAL(fleinf.GetFilename(), "DartConfiguration");
  TEST_EQUAL(fleinf.GetFilenameExt(), "DartConfiguration");
  TEST_EQUAL(fleinf.GetFullPathNoExt(),
             "/home/user/GitHub/PreCore/build/DartConfiguration");
  TEST_EQUAL(fleinf.GetFullPath(),
             "/home/user/GitHub/PreCore/build/DartConfiguration");

  exploded = fleinf.Explode();

  TEST_EQUAL(exploded.size(), 6);
  TEST_EQUAL(exploded[0], "home");
  TEST_EQUAL(exploded[1], "user");
  TEST_EQUAL(exploded[2], "GitHub");
  TEST_EQUAL(exploded[3], "PreCore");
  TEST_EQUAL(exploded[4], "build");
  TEST_EQUAL(exploded[5], "DartConfiguration");

  fleinf.Load("/home\\user/GitHub\\PreCore/build/");

  TEST_EQUAL(fleinf.GetExtension(), "");
  TEST_EQUAL(fleinf.GetFolder(), "/home/user/GitHub/PreCore/build/");
  TEST_EQUAL(fleinf.GetFilename(), "");
  TEST_EQUAL(fleinf.GetFilenameExt(), "");
  TEST_EQUAL(fleinf.GetFullPathNoExt(), "/home/user/GitHub/PreCore/build/");
  TEST_EQUAL(fleinf.GetFullPath(), "/home/user/GitHub/PreCore/build/");

  exploded = fleinf.Explode();

  TEST_EQUAL(exploded.size(), 5);
  TEST_EQUAL(exploded[0], "home");
  TEST_EQUAL(exploded[1], "user");
  TEST_EQUAL(exploded[2], "GitHub");
  TEST_EQUAL(exploded[3], "PreCore");
  TEST_EQUAL(exploded[4], "build");

  fleinf.Load("DartConfiguration");

  TEST_EQUAL(fleinf.GetExtension(), "");
  TEST_EQUAL(fleinf.GetFolder(), "");
  TEST_EQUAL(fleinf.GetFilename(), "DartConfiguration");
  TEST_EQUAL(fleinf.GetFilenameExt(), "DartConfiguration");
  TEST_EQUAL(fleinf.GetFullPathNoExt(), "DartConfiguration");
  TEST_EQUAL(fleinf.GetFullPath(), "DartConfiguration");

  exploded = fleinf.Explode();

  TEST_EQUAL(exploded.size(), 1);
  TEST_EQUAL(exploded[0], "DartConfiguration");

  fleinf.Load("DartConfiguration.tcl");

  TEST_EQUAL(fleinf.GetExtension(), ".tcl");
  TEST_EQUAL(fleinf.GetFolder(), "");
  TEST_EQUAL(fleinf.GetFilename(), "DartConfiguration");
  TEST_EQUAL(fleinf.GetFilenameExt(), "DartConfiguration.tcl");
  TEST_EQUAL(fleinf.GetFullPathNoExt(), "DartConfiguration");
  TEST_EQUAL(fleinf.GetFullPath(), "DartConfiguration.tcl");

  exploded = fleinf.Explode();

  TEST_EQUAL(exploded.size(), 1);
  TEST_EQUAL(exploded[0], "DartConfiguration.tcl");

  return 0;
}