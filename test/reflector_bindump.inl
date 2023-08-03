#include "reflector_io.inl"
#include "spike/except.hpp"
#include "spike/io/binreader.hpp"
#include "spike/io/binwritter.hpp"

int test_binwritter(const reflClass &data) {
  BinWritter mwr("testFile.le");

  TEST_CHECK(mwr.IsValid());

  mwr.Write(static_cast<const _ReflClassData &>(data));

  TEST_EQUAL(mwr.Tell(), sizeof(_ReflClassData));

  mwr.WriteContainerWCount(data.test80);

  TEST_EQUAL(mwr.Tell(), sizeof(_ReflClassData) + 4 + data.test80.size());

  mwr.ApplyPadding();

  TEST_EQUAL((mwr.Tell() & 0xf), 0);

  return 0;
}

int test_binreader(const reflClass &data) {
  TEST_THROW(es::FileNotFoundError, BinReader mrd("nonexistantFile"););

  BinReader mrd{};

  TEST_THROW(es::FileNotFoundError, mrd.Open("nonexistantFile2"););

  TEST_CHECK(!mrd.IsValid());

  mrd.Open("testFile.le");

  TEST_CHECK(mrd.IsValid());

  reflClass rClass2;

  mrd.Read(static_cast<_ReflClassData &>(rClass2));

  TEST_EQUAL(mrd.Tell(), sizeof(_ReflClassData));

  mrd.ReadContainer(rClass2.test80);

  return compare_classes(data, rClass2);
}
