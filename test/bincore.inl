#include "../datas/binwritter.hpp"
#include "../datas/binreader.hpp"

struct BinStr00 {
  int8 v0;
  bool v1;
  int16 v2;
  uint32 v3;
};

struct BinStr00_Fc : BinStr00 {
  void Read(BinReaderRef rd) {
    rd.Read(v0);
    rd.Read(v3);
  }

  void Write(BinWritterRef wr) const {
    wr.Write(v0);
    wr.Write(v3);
  }
};

struct BinStr00_Sw : BinStr00 {
  void SwapEndian() {
    FByteswapper(v2);
    FByteswapper(v3);
  }
};

int test_bincore_00() {
  std::stringstream ss;
  BinWritterRef mwr(ss);

  BinStr00_Fc tst = {};
  tst.v0 = 15;
  tst.v1 = true;
  tst.v2 = 584;
  tst.v3 = 12418651;
  mwr.Write(tst);

  tst = {};
  tst.v0 = 1;
  tst.v1 = false;
  tst.v2 = 2;
  tst.v3 = 3;

  BinReaderRef mrd(ss);
  mrd.Read(tst);

  TEST_EQUAL(tst.v0, 15);
  TEST_EQUAL(tst.v1, false);
  TEST_EQUAL(tst.v2, 2);
  TEST_EQUAL(tst.v3, 12418651);

  mwr.SwapEndian(true);
  mwr.Write(tst);

  tst.v1 = true;
  tst.v2 = 3;

  mrd.SwapEndian(true);
  mrd.Read(tst);

  TEST_EQUAL(tst.v0, 15);
  TEST_EQUAL(tst.v1, true);
  TEST_EQUAL(tst.v2, 3);
  TEST_EQUAL(tst.v3, 12418651);

  mwr.Seek(5);
  mwr.Skip(-2);

  TEST_EQUAL(mwr.Tell(), 3);

  mrd.Seek(0);

  TEST_EQUAL(mrd.Tell(), 0);

  mrd.Skip(10);

  TEST_EQUAL(mrd.Tell(), 10);

  mrd.Skip(-3);

  TEST_EQUAL(mrd.Tell(), 7);

  return 0;
};

struct wtcounter {
  uint8 count;
  uint8 someData;

  wtcounter() = default;
  wtcounter(size_t cnt) : count(cnt) {}

  void Read(BinReaderRef rd) {
    rd.Read(count);
  }

  void Write(BinWritterRef wr) const {
    wr.Write(count);
  }

  operator size_t() const {return count;}
};

int test_bincore_01() {
  std::stringstream ss;
  BinWritterRef mwr(ss);

  std::vector<BinStr00_Fc> vec;

  BinStr00_Fc tst = {};
  tst.v0 = 15;
  tst.v1 = true;
  tst.v2 = 584;
  tst.v3 = 12418651;

  vec.push_back(tst);

  tst.v0 = 79;
  tst.v1 = false;
  tst.v2 = 2100;
  tst.v3 = 4248613;

  vec.push_back(tst);

  mwr.WriteContainerWCount<wtcounter>(vec);
  vec.clear();

  BinReaderRef mrd(ss);
  mrd.ReadContainer<wtcounter>(vec);

  TEST_EQUAL(vec.size(), 2);
  TEST_EQUAL(vec[0].v0, 15);
  TEST_EQUAL(vec[0].v1, false);
  TEST_EQUAL(vec[0].v2, 0);
  TEST_EQUAL(vec[0].v3, 12418651);

  TEST_EQUAL(vec[1].v0, 79);
  TEST_EQUAL(vec[1].v1, false);
  TEST_EQUAL(vec[1].v2, 0);
  TEST_EQUAL(vec[1].v3, 4248613);

  return 0;
};

int test_bincore_02() {
  std::stringstream ss;
  BinWritterRef mwr(ss);

  std::vector<BinStr00_Sw> vec;

  BinStr00_Sw tst = {};
  tst.v0 = 15;
  tst.v1 = true;
  tst.v2 = 584;
  tst.v3 = 12418651;

  vec.push_back(tst);

  tst.v0 = 79;
  tst.v1 = false;
  tst.v2 = 2100;
  tst.v3 = 4248613;

  vec.push_back(tst);

  mwr.WriteContainerWCount<wtcounter>(vec);
  vec.clear();

  BinReaderRef mrd(ss);
  mrd.ReadContainer<wtcounter>(vec);

  TEST_EQUAL(vec.size(), 2);
  TEST_EQUAL(vec[0].v0, 15);
  TEST_EQUAL(vec[0].v1, true);
  TEST_EQUAL(vec[0].v2, 584);
  TEST_EQUAL(vec[0].v3, 12418651);

  TEST_EQUAL(vec[1].v0, 79);
  TEST_EQUAL(vec[1].v1, false);
  TEST_EQUAL(vec[1].v2, 2100);
  TEST_EQUAL(vec[1].v3, 4248613);

  mwr.SwapEndian(true);
  mwr.WriteContainerWCount(vec);

  mrd.SwapEndian(true);
  vec.clear();
  mrd.ReadContainer(vec);

  TEST_EQUAL(vec.size(), 2);
  TEST_EQUAL(vec[0].v0, 15);
  TEST_EQUAL(vec[0].v1, true);
  TEST_EQUAL(vec[0].v2, 584);
  TEST_EQUAL(vec[0].v3, 12418651);

  TEST_EQUAL(vec[1].v0, 79);
  TEST_EQUAL(vec[1].v1, false);
  TEST_EQUAL(vec[1].v2, 2100);
  TEST_EQUAL(vec[1].v3, 4248613);

  return 0;
};