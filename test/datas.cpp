#include "allocator_hybrid.inl"
#include "disabler.inl"
#include "endian.inl"
#include "fileinfo.inl"
#include "flags.inl"

#include "reflector_extended.inl"
#include "reflector_io.inl"

//Compile only
#include "../formats/BlockDecoder.h"
#include "../formats/BlockDecoder.inl"
#include "../formats/DDS.hpp"
#include "../formats/FWSE.hpp"
#include "../formats/MSF.hpp"
#include "../formats/WAVE.hpp"

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
  BinReader mrd("nonexistantFile");

  TEST_CHECK(!mrd.IsValid());

  mrd.Open("nonexistantFile2");

  TEST_CHECK(!mrd.IsValid());

  mrd.Open("testFile.le");

  TEST_CHECK(mrd.IsValid());

  reflClass rClass2;

  mrd.Read(static_cast<_ReflClassData &>(rClass2));

  TEST_EQUAL(mrd.Tell(), sizeof(_ReflClassData));

  mrd.ReadContainer(rClass2.test80);

  return compare_classes(data, rClass2);
}

int main() {
#ifndef UNICODE
  printer.AddPrinterFunction(reinterpret_cast<void *>(printf));
#else
  printer.AddPrinterFunction(reinterpret_cast<void *>(wprintf));
#endif

  printline("Compiler info:\n\tLittle Endian: "
            << ES_LITTLE_ENDIAN << "\n\tX64: " << ES_X64
            << "\n\tClass padding optimalization: " << ES_REUSE_PADDING);

  printerror("I am error and I'm red.");
  printwarning("I am warning and I'm yellow.");
  SetConsoleTextColor(127, 127, 255);
  printline("I'm blue, da ri di danu da.") RestoreConsoleTextColor();

  RegisterLocalEnums();
  REGISTER_CLASSES(subrefl, reflClass, roomInfo, roomInfo01);

  reflClass rClass = {};

  TEST_CASES(
      int testResult, TEST_FUNC(test_reflector_enum00),
      TEST_FUNC(test_reflector_enum01), TEST_FUNC(test_reflector_enum02),
      TEST_FUNC(test_reflector_enum03), TEST_FUNC(test_reflector_enum04),
      TEST_FUNC(test_reflector_decl00), TEST_FUNC(test_reflector_decl01),
      TEST_FUNC(test_reflector_decl02), TEST_FUNC(test_reflector_decl03),
      TEST_FUNC(test_reflector_decl04), TEST_FUNC(test_reflector_decl05),
      TEST_FUNC(test_reflector_decl_io), TEST_FUNC(test_reflector, rClass),
      TEST_FUNC(test_reflector_bool, rClass),
      TEST_FUNC(test_reflector_int8, rClass),
      TEST_FUNC(test_reflector_uint8, rClass),
      TEST_FUNC(test_reflector_int16, rClass),
      TEST_FUNC(test_reflector_uint16, rClass),
      TEST_FUNC(test_reflector_int32, rClass),
      TEST_FUNC(test_reflector_uint32, rClass),
      TEST_FUNC(test_reflector_int64, rClass),
      TEST_FUNC(test_reflector_uint64, rClass),
      TEST_FUNC(test_reflector_float, rClass),
      TEST_FUNC(test_reflector_double, rClass),
      TEST_FUNC(test_reflector_vector, rClass),
      TEST_FUNC(test_reflector_vector2, rClass),
      TEST_FUNC(test_reflector_vector4, rClass),
      TEST_FUNC(test_reflector_vector4A16, rClass),
      TEST_FUNC(test_reflector_arrays, rClass),
      TEST_FUNC(test_reflector_enum00, rClass),
      TEST_FUNC(test_reflector_enum01, rClass),
      TEST_FUNC(test_reflector_enum02, rClass),
      TEST_FUNC(test_reflector_enum03, rClass),
      TEST_FUNC(test_reflector_enumflags00, rClass),
      TEST_FUNC(test_reflector_enumflags01, rClass),
      TEST_FUNC(test_reflector_subclass, rClass),
      TEST_FUNC(test_reflector_array_subclass, rClass),
      TEST_FUNC(test_reflector_string, rClass), TEST_FUNC(test_reflector_alias),
      TEST_FUNC(test_reflector_desc), TEST_FUNC(test_reflector_io, rClass),
      TEST_FUNC(test_alloc_hybrid), TEST_FUNC(test_disabler),
      TEST_FUNC(test_fileinfo), TEST_FUNC(test_endian),
      TEST_FUNC(test_flags_00), TEST_FUNC(test_flags_01),
      TEST_FUNC(test_binwritter, rClass), TEST_FUNC(test_binreader, rClass), );

  return testResult;
}